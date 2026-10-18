"""Acquire the stock game's foreground without attaching input queues.

Call from a hidden worker before an input batch so its tool console does not
immediately take focus back. Verify the returned foreground PID before input.
The optional fallback sends a balanced ALT press/release to allow an ordinary
Windows foreground request. It sends no game action and changes no settings.
"""
import argparse
import ctypes as c
from ctypes import wintypes as w
import json
import time
from lrinput import LRInput

STOCK = r"C:\progs\Lemmings Revolution\Lemmings Revolution.exe"
u = c.WinDLL("user32", use_last_error=True)
k = c.WinDLL("kernel32", use_last_error=True)
k.OpenProcess.argtypes = [w.DWORD, w.BOOL, w.DWORD]
k.OpenProcess.restype = w.HANDLE
k.QueryFullProcessImageNameW.argtypes = [w.HANDLE, w.DWORD, w.LPWSTR, c.POINTER(w.DWORD)]
k.CloseHandle.argtypes = [w.HANDLE]
u.GetForegroundWindow.restype = w.HWND
u.GetWindowThreadProcessId.argtypes = [w.HWND, c.POINTER(w.DWORD)]
u.GetClassNameW.argtypes = [w.HWND, w.LPWSTR, c.c_int]
u.IsWindowVisible.argtypes = [w.HWND]
u.ShowWindowAsync.argtypes = [w.HWND, c.c_int]
u.SetForegroundWindow.argtypes = [w.HWND]
u.SetForegroundWindow.restype = w.BOOL
CALLBACK = c.WINFUNCTYPE(w.BOOL, w.HWND, w.LPARAM)
u.EnumWindows.argtypes = [CALLBACK, w.LPARAM]


def foreground_pid():
    owner = w.DWORD()
    u.GetWindowThreadProcessId(u.GetForegroundWindow(), c.byref(owner))
    return owner.value


def _alt_pair():
    class Key(c.Structure):
        _fields_ = [("vk", w.WORD), ("scan", w.WORD), ("flags", w.DWORD),
                    ("time", w.DWORD), ("extra", c.c_size_t)]
    class Mouse(c.Structure):
        _fields_ = [("x", w.LONG), ("y", w.LONG), ("data", w.DWORD),
                    ("flags", w.DWORD), ("time", w.DWORD), ("extra", c.c_size_t)]
    class Data(c.Union):
        _fields_ = [("key", Key), ("mouse", Mouse)]
    class Input(c.Structure):
        _anonymous_ = ("data",)
        _fields_ = [("kind", w.DWORD), ("data", Data)]
    pair = (Input * 2)()
    for item in pair:
        item.kind = 1
        item.key.vk = 0x12
    pair[1].key.flags = 2
    u.SendInput.argtypes = [w.UINT, c.POINTER(Input), c.c_int]
    u.SendInput.restype = w.UINT
    sent = u.SendInput(2, pair, c.sizeof(Input))
    if sent != 2:
        u.SendInput(1, c.byref(pair[1]), c.sizeof(Input))
        raise OSError("Windows did not accept the balanced ALT pair")
    return sent


def _acquire_window(pid, timeout=3.0):
    handle = k.OpenProcess(0x1000, False, pid)
    if not handle:
        raise OSError(f"Cannot inspect game PID {pid}: {c.get_last_error()}")
    path = c.create_unicode_buffer(32768)
    length = w.DWORD(len(path))
    try:
        if not k.QueryFullProcessImageNameW(handle, 0, path, c.byref(length)):
            raise OSError("Cannot verify executable identity")
    finally:
        k.CloseHandle(handle)
    if path.value.casefold() != STOCK.casefold():
        raise ValueError("Foreground target is not the selected stock executable")
    windows = []
    @CALLBACK
    def visit(hwnd, unused):
        owner = w.DWORD()
        u.GetWindowThreadProcessId(hwnd, c.byref(owner))
        if owner.value == pid:
            name = c.create_unicode_buffer(256)
            u.GetClassNameW(hwnd, name, len(name))
            if name.value == "Lemmings Revolution":
                windows.append(hwnd)
        return True
    u.EnumWindows(visit, 0)
    if len(windows) != 1:
        raise RuntimeError(f"Expected one stock game window, found {len(windows)}")
    hwnd = windows[0]
    before = foreground_pid()
    method = "already foreground"
    alt_events = 0
    if before != pid:
        u.ShowWindowAsync(hwnd, 9)
        u.SetForegroundWindow(hwnd)
        time.sleep(.1)
        method = "ordinary request"
        if foreground_pid() != pid:
            alt_events = _alt_pair()
            u.SetForegroundWindow(hwnd)
            method = "ALT-assisted ordinary request"
    deadline = time.monotonic() + timeout
    while foreground_pid() != pid:
        if time.monotonic() >= deadline:
            raise TimeoutError(f"Foreground acquisition refused for PID {pid}; no game input should follow")
        time.sleep(.05)
    return {"pid": pid, "hwnd": int(hwnd), "foreground_before_pid": before,
            "foreground_after_pid": foreground_pid(), "method": method,
            "alt_events_sent": alt_events, "input_queues_attached": False}


def acquire(pid, timeout=3.0):
    """Call between input batches; mask activation keystrokes from the game."""
    with LRInput() as guard:
        saved_enable = guard._u32(4)
        saved_input = c.string_at(guard.base + 16, 276)
        guard.clear()
        guard.enable()
        try:
            result = _acquire_window(pid, timeout)
            guard.clear()
            if not guard.wait_frames(2, timeout=1.0):
                raise TimeoutError("Game did not poll neutral input after foreground acquisition")
            result["game_input_masked_during_activation"] = True
            return result
        finally:
            c.memmove(guard.base + 16, saved_input, len(saved_input))
            guard._set_u32(4, saved_enable)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("pid", type=int)
    args = parser.parse_args()
    print(json.dumps(acquire(args.pid), indent=2))
