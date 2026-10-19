#!/usr/bin/env python3
"""READ-capture the paused stock A1 cell geometry into an explicit build/tmp directory.

Precondition: launch the unmodified stock game through the established windowed
fixture, load `Compiled\\MM_CLIMB_002.lvl`, pause it at the exact retained
camera state, and supply that process ID. This script sends no input and makes
no process writes. Raw payloads must remain under ignored build/tmp.
"""
from __future__ import annotations

import argparse
import ctypes
import ctypes.wintypes as wt
import hashlib
import json
import struct
from pathlib import Path

ROOT=Path(__file__).resolve().parents[2]
sys_path=ROOT/"tools/harness"
import sys
sys.path.insert(0,str(sys_path))
from goto_level import loaded_level  # noqa: E402
from lrstate import ui_state  # noqa: E402
from memdump import PROCESS_QUERY_INFORMATION, PROCESS_VM_READ, k32, read_mem  # noqa: E402

STOCK_EXE=Path(r"C:\progs\Lemmings Revolution\Lemmings Revolution.exe")
GAME_OBJ=0x004C6A08
WORLD=GAME_OBJ+8
CAMERA=0x004CBFC8
LEVEL_EXPECTED="Compiled\\MM_CLIMB_002.lvl"


def args_parse():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--pid",type=int,required=True,help="explicit PID of the selected stock game")
    ap.add_argument("--output",type=Path,required=True,help="new output directory, required to resolve under repository build/tmp")
    return ap.parse_args()


def main():
    args=args_parse()
    out=args.output.resolve()
    tmp=(ROOT/"build/tmp").resolve()
    if tmp not in out.parents:
        raise SystemExit(f"refusing output outside {tmp}: {out}")
    if out.exists() and any(out.iterdir()):
        raise SystemExit(f"refusing to overwrite non-empty capture directory: {out}")
    out.mkdir(parents=True,exist_ok=True)
    access=PROCESS_VM_READ|PROCESS_QUERY_INFORMATION
    handle=k32.OpenProcess(access,False,args.pid)
    if not handle:
        raise OSError(f"OpenProcess READ failed for PID {args.pid}: {ctypes.get_last_error()}")
    k32.QueryFullProcessImageNameW.argtypes=[wt.HANDLE,wt.DWORD,wt.LPWSTR,ctypes.POINTER(wt.DWORD)]
    k32.QueryFullProcessImageNameW.restype=wt.BOOL
    user=ctypes.WinDLL("user32",use_last_error=True)
    user.GetForegroundWindow.restype=wt.HWND
    user.GetWindowThreadProcessId.argtypes=[wt.HWND,ctypes.POINTER(wt.DWORD)]

    def rd(address,size):
        data=read_mem(handle,address,size)
        if data is None or len(data)!=size:
            raise RuntimeError(f"incomplete READ at {address:#x}/{size:#x}")
        return data

    def u32(address):
        return struct.unpack("<I",rd(address,4))[0]

    def header(address):
        raw=rd(address,0x10)
        return {"count":struct.unpack_from("<I",raw,0)[0],
                "capacity":struct.unpack_from("<I",raw,4)[0],
                "data":struct.unpack_from("<I",raw,0xC)[0]}

    try:
        path_buf=ctypes.create_unicode_buffer(32768)
        path_len=wt.DWORD(len(path_buf))
        if not k32.QueryFullProcessImageNameW(handle,0,path_buf,ctypes.byref(path_len)):
            raise OSError("cannot verify process image path")
        if Path(path_buf.value).resolve()!=STOCK_EXE.resolve():
            raise RuntimeError(f"wrong process image: {path_buf.value}")
        foreground_owner=wt.DWORD()
        hwnd=user.GetForegroundWindow()
        user.GetWindowThreadProcessId(hwnd,ctypes.byref(foreground_owner))
        if foreground_owner.value!=args.pid:
            raise RuntimeError(f"selected PID {args.pid} is not foreground (foreground PID {foreground_owner.value})")
        if ui_state(handle)!=1:
            raise RuntimeError(f"expected paused game UI state 1; observed {ui_state(handle)}")
        level=loaded_level(handle)
        if level.lower()!=LEVEL_EXPECTED.lower():
            raise RuntimeError(f"expected {LEVEL_EXPECTED}, observed {level}")
        reference=json.loads((ROOT/"tools/harness/fixtures/camera_a1/reference-state.json").read_text())
        expected_camera=bytes.fromhex(reference["camera"]["raw_hex"])
        camera=rd(CAMERA,len(expected_camera))
        if camera!=expected_camera:
            raise RuntimeError("exact retained initial camera bytes did not match; stopped before cell-array reads")

        groups=[];entries=[]
        for group in range(32):
            gh=header(WORLD+0x208+group*0x10)
            if gh["count"]>gh["capacity"] or gh["count"]>10000:
                raise RuntimeError(("invalid group header",group,gh))
            if gh["count"] and not (0x10000<=gh["data"]<0x7fff0000):
                raise RuntimeError(("invalid group data",group,gh))
            groups.append(gh)
            for index in range(gh["count"]):
                address=gh["data"]+index*0x100
                record=rd(address,0x100)
                entry={"group":group,"entry":index,"address":address,"record_hex":record.hex(),
                       "serialized_material_runtime":struct.unpack_from("<I",record,0xC8)[0]}
                for name,offset in (("children_88",0x88),("children_98",0x98),("children_b8",0xB8)):
                    child=header(address+offset)
                    if child["count"]>child["capacity"] or child["count"]>50000:
                        raise RuntimeError(("invalid child header",group,index,name,child))
                    entry[name]=child
                    if child["count"]:
                        if not (0x10000<=child["data"]<0x7fff0000):
                            raise RuntimeError(("invalid child data",group,index,name,child))
                        blob=rd(child["data"],child["count"]*4)
                        filename=f"g{group:02d}-e{index:03d}-{name}.bin"
                        (out/filename).write_bytes(blob)
                        child.update({"raw_file":filename,"raw_sha256":hashlib.sha256(blob).hexdigest()})
                # Both executable-selected nested mesh pointers are retained;
                # the complete 0x100 record also contains the gated +8 inline quad.
                for route,field_offset in (("mesh",0),("mesh4",4)):
                    pointer=struct.unpack_from("<I",record,field_offset)[0]
                    entry[route+"_pointer"]=pointer
                    if not (0x10000<=pointer<0x7fff0000):
                        entry[route]={"count":0,"data":0,"raw_file":None}
                        continue
                    mesh_header=rd(pointer,0x10)
                    count=struct.unpack_from("<I",mesh_header,4)[0]
                    data=struct.unpack_from("<I",mesh_header,0xC)[0]
                    if count>50000:
                        raise RuntimeError(("invalid mesh count",group,index,route,count))
                    payload={"count":count,"data":data,"stride":0x80}
                    if count:
                        if not (0x10000<=data<0x7fff0000):
                            raise RuntimeError(("invalid mesh data",group,index,route,data))
                        blob=rd(data,count*0x80)
                        filename=f"g{group:02d}-e{index:03d}-{route}-quads.bin"
                        (out/filename).write_bytes(blob)
                        payload.update({"raw_file":filename,"raw_sha256":hashlib.sha256(blob).hexdigest()})
                    entry[route]=payload
                # Preserve the old mesh key used by the offline auditor.
                entries.append(entry)

        src=header(WORLD+0x148)
        if src["count"]!=897:
            raise RuntimeError(("unexpected source vertex count",src["count"]))
        source=rd(src["data"],src["count"]*24)
        (out/"source-vertices.bin").write_bytes(source)
        materials_header=header(WORLD+0x168)
        materials=rd(materials_header["data"],materials_header["count"]*24)
        (out/"materials.bin").write_bytes(materials)
        manifest={"pid":args.pid,"image_path":path_buf.value,"foreground_pid":foreground_owner.value,
            "loaded_level":level,"ui_state":ui_state(handle),"camera":{"address":hex(CAMERA),"raw_hex":camera.hex(),
            "sha256":hashlib.sha256(camera).hexdigest(),"reference":"tools/harness/fixtures/camera_a1/reference-state.json",
            "exact_reference_match":True},"world_address":hex(WORLD),"group_header_base":hex(WORLD+0x208),
            "group_headers":groups,"entry_count":len(entries),"entries":entries,
            "source_pool":{"count":src["count"],"stride":24,"file":"source-vertices.bin","sha256":hashlib.sha256(source).hexdigest()},
            "materials":{"count":materials_header["count"],"stride":24,"file":"materials.bin","sha256":hashlib.sha256(materials).hexdigest()},
            "capture_kind":"READ-only bounded process-memory capture while exact reference world is paused",
            "raw_payload_policy":"ignored build/tmp only; do not commit"}
        (out/"manifest.json").write_text(json.dumps(manifest,indent=2),encoding="utf-8")
        print(json.dumps({"manifest":str(out/"manifest.json"),"pid":args.pid,"loaded_level":level,
            "ui_state":manifest["ui_state"],"camera_exact_reference_match":True,
            "group_counts":[group["count"] for group in groups],"entry_count":len(entries),
            "mesh0_quads":sum(entry["mesh"]["count"] for entry in entries),
            "mesh4_quads":sum(entry["mesh4"]["count"] for entry in entries),
            "children":{"88":sum(entry["children_88"]["count"] for entry in entries),
                         "98":sum(entry["children_98"]["count"] for entry in entries),
                         "b8":sum(entry["children_b8"]["count"] for entry in entries)},
            "source_pool_sha256":manifest["source_pool"]["sha256"],"materials_sha256":manifest["materials"]["sha256"]},indent=2))
    finally:
        k32.CloseHandle(handle)


if __name__=="__main__":
    main()
