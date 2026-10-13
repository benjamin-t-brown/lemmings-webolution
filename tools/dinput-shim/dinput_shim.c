/*
 * dinput.dll proxy shim for Lemmings Revolution.
 *
 * Why this exists
 * ---------------
 * The game reads mouse and keyboard exclusively through DirectInput, and it
 * ignores every form of Windows input injection (SendInput, mouse_event,
 * keybd_event).  That blocks any attempt to drive the original automatically,
 * which the differential oracle in PLAN.md depends on.
 *
 * The game's whole input surface turns out to be two IDirectInputDevice::
 * GetDeviceState calls:
 *
 *     .text:00483B5D   GetDeviceState(0x10,  &DIMOUSESTATE)   device at this+0x28
 *     .text:00483E1A   GetDeviceState(0x100, keys[256])       device at this+0x100
 *
 * So rather than implement wrapper COM objects, we let the real dinput.dll do
 * all the work and simply patch two vtable slots:
 *
 *     IDirectInputA      vtable[3]  CreateDevice     -> hook, to catch devices
 *     IDirectInputDeviceA vtable[9] GetDeviceState   -> hook, to supply state
 *
 * State comes from a named shared-memory block that an external harness writes.
 * When `enable` is 0 every call passes straight through, so the game behaves
 * completely normally and a human can still play it.
 *
 * Build: 32-bit, see build.sh.  Drop the result in the game folder as
 * dinput.dll (dinput.dll is not a KnownDLL, so the application directory wins).
 */

#include <windows.h>
#include <stdint.h>

#define LRIN_MAGIC   0x4E49524Cu   /* 'LRIN' */
#define LRIN_MAPPING "Local\\LemmingsRevolutionInput"

/* Shared control block.  Keep this layout in sync with tools/harness/lrinput.py. */
#pragma pack(push, 1)
typedef struct {
    volatile uint32_t magic;
    volatile uint32_t enable;        /* 0 = pass through to the real device   */
    volatile uint32_t mouse_frame;   /* ++ on every mouse GetDeviceState      */
    volatile uint32_t key_frame;     /* ++ on every keyboard GetDeviceState   */
    volatile int32_t  mouse_dx;      /* relative, consumed once if consume=1  */
    volatile int32_t  mouse_dy;
    volatile int32_t  mouse_dz;
    volatile uint8_t  mouse_buttons[4];
    volatile uint32_t consume_deltas;/* 1 = zero dx/dy/dz after each read     */
    volatile uint8_t  keys[256];     /* DIK_* scan code -> 0x80 if held       */
    /* Diagnostics, appended so every offset above stays where lrinput.py
       expects it. Added to answer one question: clicks do not reach the game
       while motion does, and these say whether that is because the game asks
       for its buttons somewhere this shim is not looking. */
    volatile uint32_t data_frame;    /* ++ on every GetDeviceData call        */
    volatile uint32_t data_items;    /* items the last GetDeviceData returned */
    volatile uint32_t last_cb;       /* cb of the last GetDeviceState call    */
    volatile uint32_t state_calls;   /* ++ on every GetDeviceState call       */
} LRInputBlock;
#pragma pack(pop)

static LRInputBlock *g_block;
static HANDLE        g_mapping;
static HMODULE       g_real;

/* ---- real dinput.dll ---------------------------------------------------- */

typedef HRESULT (WINAPI *PFN_DirectInputCreateA)(HINSTANCE, DWORD, void **, void *);
typedef HRESULT (WINAPI *PFN_DirectInputCreateW)(HINSTANCE, DWORD, void **, void *);
typedef HRESULT (WINAPI *PFN_DirectInputCreateEx)(HINSTANCE, DWORD, REFIID, void **, void *);
typedef HRESULT (WINAPI *PFN_DllGetClassObject)(REFCLSID, REFIID, void **);
typedef HRESULT (WINAPI *PFN_DllCanUnloadNow)(void);

static PFN_DirectInputCreateA  real_DirectInputCreateA;
static PFN_DirectInputCreateW  real_DirectInputCreateW;
static PFN_DirectInputCreateEx real_DirectInputCreateEx;
static PFN_DllGetClassObject   real_DllGetClassObject;
static PFN_DllCanUnloadNow     real_DllCanUnloadNow;

/* ---- hooked method pointers --------------------------------------------- */

typedef HRESULT (STDMETHODCALLTYPE *PFN_CreateDevice)(void *, const GUID *, void **, void *);
typedef HRESULT (STDMETHODCALLTYPE *PFN_GetDeviceState)(void *, DWORD, void *);
typedef HRESULT (STDMETHODCALLTYPE *PFN_GetDeviceData)(void *, DWORD, void *, DWORD *, DWORD);

static PFN_CreateDevice   orig_CreateDevice;
static PFN_GetDeviceState orig_GetDeviceState;
static PFN_GetDeviceData  orig_GetDeviceData;

static void open_block(void)
{
    if (g_block)
        return;
    g_mapping = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
                                   0, sizeof(LRInputBlock), LRIN_MAPPING);
    if (!g_mapping)
        return;
    g_block = (LRInputBlock *)MapViewOfFile(g_mapping, FILE_MAP_ALL_ACCESS,
                                            0, 0, sizeof(LRInputBlock));
    if (g_block && g_block->magic != LRIN_MAGIC) {
        /* First creator initialises; harness may have got here first. */
        ZeroMemory((void *)g_block, sizeof(*g_block));
        g_block->magic = LRIN_MAGIC;
    }
}

static int load_real(void)
{
    char path[MAX_PATH];
    UINT n;

    if (g_real)
        return 1;
    /* A 32-bit process gets SysWOW64 here, which is what we want. */
    n = GetSystemDirectoryA(path, MAX_PATH);
    if (n == 0 || n > MAX_PATH - 16)
        return 0;
    lstrcatA(path, "\\dinput.dll");
    g_real = LoadLibraryA(path);
    if (!g_real)
        return 0;

    /* Cast through void* -- GetProcAddress returns a generic FARPROC. */
    real_DirectInputCreateA  = (PFN_DirectInputCreateA) (void *)GetProcAddress(g_real, "DirectInputCreateA");
    real_DirectInputCreateW  = (PFN_DirectInputCreateW) (void *)GetProcAddress(g_real, "DirectInputCreateW");
    real_DirectInputCreateEx = (PFN_DirectInputCreateEx)(void *)GetProcAddress(g_real, "DirectInputCreateEx");
    real_DllGetClassObject   = (PFN_DllGetClassObject)  (void *)GetProcAddress(g_real, "DllGetClassObject");
    real_DllCanUnloadNow     = (PFN_DllCanUnloadNow)    (void *)GetProcAddress(g_real, "DllCanUnloadNow");
    return 1;
}

/* Overwrite one slot of a COM vtable, returning the previous value. */
static void *patch_slot(void *iface, int index, void *replacement)
{
    void **vtbl = *(void ***)iface;
    DWORD old;
    void *previous;

    if (!VirtualProtect(&vtbl[index], sizeof(void *), PAGE_READWRITE, &old))
        return NULL;
    previous = vtbl[index];
    if (previous != replacement)
        vtbl[index] = replacement;
    VirtualProtect(&vtbl[index], sizeof(void *), old, &old);
    return previous;
}

/* ---- IDirectInputDevice::GetDeviceState (vtable index 9) ---------------- */

static HRESULT STDMETHODCALLTYPE hook_GetDeviceState(void *self, DWORD cb, void *data)
{
    LRInputBlock *b = g_block;

    /* Count every read, including pass-through ones, so the harness can tell
       the shim is live and the game is polling before taking control. */
    if (b) {
        b->state_calls++;
        b->last_cb = cb;
        if (cb == 0x10 || cb == 0x14)      /* DIMOUSESTATE / DIMOUSESTATE2 */
            b->mouse_frame++;
        else if (cb == 0x100)
            b->key_frame++;
    }

    if (!b || !b->enable || !data)
        return orig_GetDeviceState(self, cb, data);

    if (cb == 0x10 || cb == 0x14) {        /* DIMOUSESTATE / DIMOUSESTATE2 */
        int32_t *axis = (int32_t *)data;
        uint8_t *btn  = (uint8_t *)data + 12;
        axis[0] = b->mouse_dx;
        axis[1] = b->mouse_dy;
        axis[2] = b->mouse_dz;
        btn[0] = b->mouse_buttons[0];
        btn[1] = b->mouse_buttons[1];
        btn[2] = b->mouse_buttons[2];
        btn[3] = b->mouse_buttons[3];
        if (cb == 0x14) {
            /* DIMOUSESTATE2 has four extra button bytes after the first four. */
            btn[4] = 0;
            btn[5] = 0;
            btn[6] = 0;
            btn[7] = 0;
        }
        if (b->consume_deltas) {
            b->mouse_dx = 0;
            b->mouse_dy = 0;
            b->mouse_dz = 0;
        }
        return S_OK;
    }

    if (cb == 0x100) {                     /* 256-byte DIK keyboard array */
        CopyMemory(data, (const void *)b->keys, 256);
        return S_OK;
    }

    return orig_GetDeviceState(self, cb, data);
}

/* ---- IDirectInputDevice::GetDeviceData (vtable index 10) ----------------
   Not hooked to inject anything yet, only to observe. If the game takes its
   clicks from buffered data rather than immediate state, this counter moves
   while a click is held and GetDeviceState's button bytes are ignored --
   which is exactly the ambiguity blocking the live terrain dump. */

static HRESULT STDMETHODCALLTYPE hook_GetDeviceData(void *self, DWORD cbObjectData,
                                                    void *rgdod, DWORD *pdwInOut,
                                                    DWORD dwFlags)
{
    LRInputBlock *b = g_block;
    HRESULT hr = orig_GetDeviceData(self, cbObjectData, rgdod, pdwInOut, dwFlags);
    if (b) {
        b->data_frame++;
        if (pdwInOut)
            b->data_items = *pdwInOut;
    }
    return hr;
}

/* ---- IDirectInput::CreateDevice (vtable index 3) ------------------------ */

static HRESULT STDMETHODCALLTYPE hook_CreateDevice(void *self, const GUID *guid,
                                                   void **out, void *outer)
{
    HRESULT hr = orig_CreateDevice(self, guid, out, outer);
    if (SUCCEEDED(hr) && out && *out) {
        void *prev10 = patch_slot(*out, 10, (void *)hook_GetDeviceData);
        if (prev10 && prev10 != (void *)hook_GetDeviceData)
            orig_GetDeviceData = (PFN_GetDeviceData)prev10;
        void *prev = patch_slot(*out, 9, (void *)hook_GetDeviceState);
        if (prev && prev != (void *)hook_GetDeviceState)
            orig_GetDeviceState = (PFN_GetDeviceState)prev;
    }
    return hr;
}

/* ---- exports ------------------------------------------------------------ */

HRESULT WINAPI DirectInputCreateA(HINSTANCE hinst, DWORD ver, void **ppDI, void *outer)
{
    HRESULT hr;

    if (!load_real() || !real_DirectInputCreateA)
        return E_FAIL;
    open_block();

    hr = real_DirectInputCreateA(hinst, ver, ppDI, outer);
    if (SUCCEEDED(hr) && ppDI && *ppDI) {
        void *prev = patch_slot(*ppDI, 3, (void *)hook_CreateDevice);
        if (prev && prev != (void *)hook_CreateDevice)
            orig_CreateDevice = (PFN_CreateDevice)prev;
    }
    return hr;
}

HRESULT WINAPI DirectInputCreateW(HINSTANCE hinst, DWORD ver, void **ppDI, void *outer)
{
    if (!load_real() || !real_DirectInputCreateW)
        return E_FAIL;
    return real_DirectInputCreateW(hinst, ver, ppDI, outer);
}

HRESULT WINAPI DirectInputCreateEx(HINSTANCE hinst, DWORD ver, REFIID riid,
                                   void **ppv, void *outer)
{
    if (!load_real() || !real_DirectInputCreateEx)
        return E_FAIL;
    return real_DirectInputCreateEx(hinst, ver, riid, ppv, outer);
}

HRESULT WINAPI DllGetClassObject(REFCLSID clsid, REFIID iid, void **ppv)
{
    if (!load_real() || !real_DllGetClassObject)
        return E_FAIL;
    return real_DllGetClassObject(clsid, iid, ppv);
}

HRESULT WINAPI DllCanUnloadNow(void)
{
    if (!load_real() || !real_DllCanUnloadNow)
        return S_FALSE;
    return real_DllCanUnloadNow();
}

BOOL WINAPI DllMain(HINSTANCE inst, DWORD reason, LPVOID reserved)
{
    (void)reserved;
    if (reason == DLL_PROCESS_ATTACH)
        DisableThreadLibraryCalls(inst);
    return TRUE;
}
