"""Run a Windows process tree in a kill-on-close Job Object.

Create suspended, assign to job, then resume: children cannot race assignment.
This bounds lifetime, not the commands or files a worker may access.
"""
import ctypes as C
from ctypes import wintypes as W
import os
from pathlib import Path
import subprocess
import time


class STARTUPINFO(C.Structure):
    _fields_ = [('cb', W.DWORD), ('lpReserved', W.LPWSTR),
                ('lpDesktop', W.LPWSTR), ('lpTitle', W.LPWSTR),
                ('dwX', W.DWORD), ('dwY', W.DWORD), ('dwXSize', W.DWORD),
                ('dwYSize', W.DWORD), ('dwXCountChars', W.DWORD),
                ('dwYCountChars', W.DWORD), ('dwFillAttribute', W.DWORD),
                ('dwFlags', W.DWORD), ('wShowWindow', W.WORD),
                ('cbReserved2', W.WORD), ('lpReserved2', C.POINTER(W.BYTE)),
                ('hStdInput', W.HANDLE), ('hStdOutput', W.HANDLE),
                ('hStdError', W.HANDLE)]


class PROCESS_INFORMATION(C.Structure):
    _fields_ = [('hProcess', W.HANDLE), ('hThread', W.HANDLE),
                ('dwProcessId', W.DWORD), ('dwThreadId', W.DWORD)]


class BASIC_LIMIT(C.Structure):
    _fields_ = [('PerProcessUserTimeLimit', C.c_int64),
                ('PerJobUserTimeLimit', C.c_int64), ('LimitFlags', W.DWORD),
                ('MinimumWorkingSetSize', C.c_size_t),
                ('MaximumWorkingSetSize', C.c_size_t),
                ('ActiveProcessLimit', W.DWORD), ('Affinity', C.c_size_t),
                ('PriorityClass', W.DWORD), ('SchedulingClass', W.DWORD)]


class IO_COUNTERS(C.Structure):
    _fields_ = [(n, C.c_uint64) for n in (
        'ReadOperationCount', 'WriteOperationCount', 'OtherOperationCount',
        'ReadTransferCount', 'WriteTransferCount', 'OtherTransferCount')]


class EXTENDED_LIMIT(C.Structure):
    _fields_ = [('BasicLimitInformation', BASIC_LIMIT), ('IoInfo', IO_COUNTERS),
                ('ProcessMemoryLimit', C.c_size_t), ('JobMemoryLimit', C.c_size_t),
                ('PeakProcessMemoryUsed', C.c_size_t),
                ('PeakJobMemoryUsed', C.c_size_t)]


def run_tree(command, *, cwd, stdin_path, stdout_path, stderr_path,
             deadline, stop_path=None):
    """Return exit/reason; always close the job, including on Ctrl+C/error."""
    if os.name != 'nt':
        raise RuntimeError('This supervisor requires Windows Job Objects')
    import msvcrt
    k = C.WinDLL('kernel32', use_last_error=True)
    signatures = {
        'CreateJobObjectW': ([C.c_void_p, W.LPCWSTR], W.HANDLE),
        'SetInformationJobObject': ([W.HANDLE, C.c_int, C.c_void_p, W.DWORD], W.BOOL),
        'AssignProcessToJobObject': ([W.HANDLE, W.HANDLE], W.BOOL),
        'CreateProcessW': ([W.LPCWSTR, W.LPWSTR, C.c_void_p, C.c_void_p,
                            W.BOOL, W.DWORD, C.c_void_p, W.LPCWSTR,
                            C.POINTER(STARTUPINFO), C.POINTER(PROCESS_INFORMATION)], W.BOOL),
        'ResumeThread': ([W.HANDLE], W.DWORD),
        'WaitForSingleObject': ([W.HANDLE, W.DWORD], W.DWORD),
        'GetExitCodeProcess': ([W.HANDLE, C.POINTER(W.DWORD)], W.BOOL),
        'TerminateProcess': ([W.HANDLE, W.UINT], W.BOOL),
        'CloseHandle': ([W.HANDLE], W.BOOL),
    }
    for name, (args, result) in signatures.items():
        getattr(k, name).argtypes = args
        getattr(k, name).restype = result

    def check(ok):
        if not ok:
            raise C.WinError(C.get_last_error())
        return ok

    if time.monotonic() >= deadline:
        return {'reason': 'timeout', 'exit_code': None}
    if stop_path and Path(stop_path).exists():
        return {'reason': 'cancelled', 'exit_code': None}
    job = check(k.CreateJobObjectW(None, None))
    pi = PROCESS_INFORMATION()
    assigned = False
    try:
        limit = EXTENDED_LIMIT()
        limit.BasicLimitInformation.LimitFlags = 0x2000  # KILL_ON_JOB_CLOSE
        check(k.SetInformationJobObject(job, 9, C.byref(limit), C.sizeof(limit)))
        with open(stdin_path or os.devnull, 'rb') as inp, \
                open(stdout_path, 'ab', buffering=0) as out, \
                open(stderr_path, 'ab', buffering=0) as err:
            handles = [msvcrt.get_osfhandle(f.fileno()) for f in (inp, out, err)]
            for handle in handles:
                os.set_handle_inheritable(handle, True)
            si = STARTUPINFO()
            si.cb = C.sizeof(si)
            si.dwFlags = 0x100  # STARTF_USESTDHANDLES
            si.hStdInput, si.hStdOutput, si.hStdError = handles
            commandline = C.create_unicode_buffer(subprocess.list2cmdline(command))
            try:
                check(k.CreateProcessW(str(command[0]), commandline, None, None,
                                      True, 0x4 | 0x08000000, None, str(cwd),
                                      C.byref(si), C.byref(pi)))
            finally:
                for handle in handles:
                    os.set_handle_inheritable(handle, False)
            check(k.AssignProcessToJobObject(job, pi.hProcess))
            assigned = True
            if k.ResumeThread(pi.hThread) == 0xFFFFFFFF:
                raise C.WinError(C.get_last_error())
            while True:
                if stop_path and Path(stop_path).exists():
                    return {'reason': 'cancelled', 'exit_code': None, 'pid': pi.dwProcessId}
                if time.monotonic() >= deadline:
                    return {'reason': 'timeout', 'exit_code': None, 'pid': pi.dwProcessId}
                wait = k.WaitForSingleObject(pi.hProcess, 100)
                if wait == 0:
                    code = W.DWORD()
                    check(k.GetExitCodeProcess(pi.hProcess, C.byref(code)))
                    return {'reason': 'exited', 'exit_code': code.value, 'pid': pi.dwProcessId}
                if wait != 0x102:
                    raise C.WinError(C.get_last_error())
    finally:
        # Failure to assign must terminate the still-suspended process too.
        if pi.hProcess and not assigned:
            k.TerminateProcess(pi.hProcess, 125)
        k.CloseHandle(job)
        if pi.hProcess:
            k.WaitForSingleObject(pi.hProcess, 5000)
            k.CloseHandle(pi.hProcess)
        if pi.hThread:
            k.CloseHandle(pi.hThread)
