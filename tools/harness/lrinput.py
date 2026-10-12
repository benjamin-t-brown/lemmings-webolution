"""Drive the original Lemmings Revolution through the dinput.dll shim.

The game reads all input through two IDirectInputDevice::GetDeviceState calls
and ignores Windows input injection entirely.  tools/dinput-shim intercepts
those two calls and serves state from a named shared-memory block instead.
This module is the writer for that block.

Layout must stay in sync with LRInputBlock in dinput_shim.c:

    u32   magic            'LRIN'
    u32   enable           0 = pass through to the real device
    u32   mouse_frame      ++ by the shim on every mouse read
    u32   key_frame        ++ by the shim on every keyboard read
    i32   mouse_dx, mouse_dy, mouse_dz
    u8[4] mouse_buttons
    u32   consume_deltas   1 = shim zeroes the deltas after each read
    u8[256] keys           DIK scan code -> 0x80 when held

Typical use:

    with LRInput() as inp:
        inp.enable()
        inp.move(40, 0)          # one mouse-tick of relative motion
        inp.click()              # press and release button 0
        inp.tap(DIK['RETURN'])
"""

import ctypes
import ctypes.wintypes as wt
import struct
import time

MAGIC = 0x4E49524C
MAPPING_NAME = "Local\\LemmingsRevolutionInput"
BLOCK_SIZE = 4 * 4 + 4 * 3 + 4 + 4 + 256   # 36 + 4 + 256 = 296

OFF_MAGIC = 0
OFF_ENABLE = 4
OFF_MOUSE_FRAME = 8
OFF_KEY_FRAME = 12
OFF_DX = 16
OFF_DY = 20
OFF_DZ = 24
OFF_BUTTONS = 28
OFF_CONSUME = 32
OFF_KEYS = 36

PAGE_READWRITE = 0x04
FILE_MAP_ALL_ACCESS = 0xF001F
INVALID_HANDLE_VALUE = wt.HANDLE(-1)

k32 = ctypes.WinDLL('kernel32', use_last_error=True)
k32.CreateFileMappingA.restype = wt.HANDLE
k32.CreateFileMappingA.argtypes = [wt.HANDLE, ctypes.c_void_p, wt.DWORD,
                                   wt.DWORD, wt.DWORD, wt.LPCSTR]
k32.MapViewOfFile.restype = ctypes.c_void_p
k32.MapViewOfFile.argtypes = [wt.HANDLE, wt.DWORD, wt.DWORD, wt.DWORD, ctypes.c_size_t]
k32.UnmapViewOfFile.argtypes = [ctypes.c_void_p]
k32.CloseHandle.argtypes = [wt.HANDLE]

# DirectInput scan codes (DIK_*), the subset worth having.
DIK = {
    'ESCAPE': 0x01, '1': 0x02, '2': 0x03, '3': 0x04, '4': 0x05, '5': 0x06,
    '6': 0x07, '7': 0x08, '8': 0x09, '9': 0x0A, '0': 0x0B,
    'RETURN': 0x1C, 'LCONTROL': 0x1D, 'LSHIFT': 0x2A, 'SPACE': 0x39,
    'F1': 0x3B, 'F2': 0x3C, 'F3': 0x3D, 'F4': 0x3E, 'F5': 0x3F,
    'F6': 0x40, 'F7': 0x41, 'F8': 0x42, 'F9': 0x43, 'F10': 0x44,
    'P': 0x19, 'R': 0x13, 'N': 0x31,
    'UP': 0xC8, 'LEFT': 0xCB, 'RIGHT': 0xCD, 'DOWN': 0xD0,
    'PAUSE': 0xC5, 'PGUP': 0xC9, 'PGDN': 0xD1,
}


class LRInput:
    def __init__(self):
        self.h = None
        self.base = None

    def __enter__(self):
        self.open()
        return self

    def __exit__(self, *exc):
        self.close()

    def open(self):
        self.h = k32.CreateFileMappingA(INVALID_HANDLE_VALUE, None, PAGE_READWRITE,
                                        0, BLOCK_SIZE, MAPPING_NAME.encode())
        if not self.h:
            raise OSError('CreateFileMapping failed: %d' % ctypes.get_last_error())
        self.base = k32.MapViewOfFile(self.h, FILE_MAP_ALL_ACCESS, 0, 0, BLOCK_SIZE)
        if not self.base:
            raise OSError('MapViewOfFile failed: %d' % ctypes.get_last_error())
        if self._u32(OFF_MAGIC) != MAGIC:
            ctypes.memset(self.base, 0, BLOCK_SIZE)
            self._set_u32(OFF_MAGIC, MAGIC)
            self._set_u32(OFF_CONSUME, 1)
        return self

    def close(self):
        if self.base:
            k32.UnmapViewOfFile(ctypes.c_void_p(self.base)); self.base = None
        if self.h:
            k32.CloseHandle(self.h); self.h = None

    # -- raw accessors ----------------------------------------------------
    def _u32(self, off):
        return struct.unpack_from('<I', ctypes.string_at(self.base + off, 4))[0]

    def _set_u32(self, off, v):
        ctypes.memmove(self.base + off, struct.pack('<I', v & 0xFFFFFFFF), 4)

    def _set_i32(self, off, v):
        ctypes.memmove(self.base + off, struct.pack('<i', v), 4)

    # -- control ----------------------------------------------------------
    def enable(self, on=True):
        """Take over input.  While disabled the game reads the real devices."""
        self._set_u32(OFF_ENABLE, 1 if on else 0)

    def release(self):
        """Hand input back to the physical mouse and keyboard."""
        self.clear()
        self.enable(False)

    def clear(self):
        self._set_i32(OFF_DX, 0); self._set_i32(OFF_DY, 0); self._set_i32(OFF_DZ, 0)
        ctypes.memset(self.base + OFF_BUTTONS, 0, 4)
        ctypes.memset(self.base + OFF_KEYS, 0, 256)

    @property
    def mouse_frame(self):
        return self._u32(OFF_MOUSE_FRAME)

    @property
    def key_frame(self):
        return self._u32(OFF_KEY_FRAME)

    def wait_frames(self, n=1, timeout=2.0):
        """Block until the game has consumed n more mouse reads."""
        target = self.mouse_frame + n
        deadline = time.time() + timeout
        while self.mouse_frame < target:
            if time.time() > deadline:
                return False
            time.sleep(0.001)
        return True

    # -- mouse ------------------------------------------------------------
    def move(self, dx, dy, dz=0, settle=1):
        """Queue one tick of relative motion; the shim clears it once read."""
        self._set_i32(OFF_DX, dx); self._set_i32(OFF_DY, dy); self._set_i32(OFF_DZ, dz)
        if settle:
            self.wait_frames(settle)

    def glide(self, dx, dy, steps=10, per_step_frames=1):
        """Spread motion over several ticks, as a real mouse would deliver it."""
        for i in range(steps):
            sx = dx // steps + (1 if i < dx % steps else 0)
            sy = dy // steps + (1 if i < dy % steps else 0)
            self.move(sx, sy, settle=per_step_frames)

    def button(self, index, down):
        ctypes.memmove(self.base + OFF_BUTTONS + index,
                       bytes([0x80 if down else 0x00]), 1)

    def click(self, index=0, hold_frames=2):
        self.button(index, True)
        self.wait_frames(hold_frames)
        self.button(index, False)
        self.wait_frames(1)

    # -- keyboard ---------------------------------------------------------
    def key(self, dik, down):
        if isinstance(dik, str):
            dik = DIK[dik.upper()]
        ctypes.memmove(self.base + OFF_KEYS + dik,
                       bytes([0x80 if down else 0x00]), 1)

    def tap(self, dik, hold_frames=2):
        self.key(dik, True)
        self.wait_frames(hold_frames)
        self.key(dik, False)
        self.wait_frames(1)


if __name__ == '__main__':
    import sys
    with LRInput() as inp:
        print('magic       : %08X' % inp._u32(OFF_MAGIC))
        print('enable      : %d' % inp._u32(OFF_ENABLE))
        f0m, f0k = inp.mouse_frame, inp.key_frame
        time.sleep(1.0)
        print('mouse reads/s: %d' % (inp.mouse_frame - f0m))
        print('key   reads/s: %d' % (inp.key_frame - f0k))
        if inp.mouse_frame == f0m:
            print('\nThe shim is not being called. Check that dinput.dll is in the')
            print('game folder and that the game is running.')
            sys.exit(1)
        print('\nShim is live and the game is polling it.')
