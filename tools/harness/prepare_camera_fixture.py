"""Prepare local retail inputs for the bounded camera fixture; commits no game assets."""
import argparse
import hashlib
import json
import os
import shlex
import subprocess
from pathlib import Path

ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'build/tmp/a1-facilitator'
ASSETS=ROOT/'build/tmp/a1-camera/assets'

def msys(path):
    path=Path(path).resolve()
    if len(path.drive)!=2 or path.drive[1]!=':':
        raise ValueError('This Windows fixture expects a local drive path')
    return '/'+path.drive[0].lower()+path.as_posix()[2:]

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--game-dir',type=Path,default=Path(r'C:\progs\Lemmings Revolution'))
    args=parser.parse_args()
    assert args.game_dir.is_dir()
    executable=ROOT/'build/cmake/dev-debug/lrtool.exe'
    assert executable.is_file(), 'Build dev-debug first'
    OUT.mkdir(parents=True,exist_ok=True)
    level=ASSETS/'COMPILED/MM_CLIMB_002.LVL';level.parent.mkdir(parents=True,exist_ok=True)
    jobs=[('extract','COMPILED/MM_CLIMB_002.LVL',level)]
    jobs.extend(('decode',f'TEXTURES/CYLINDER/MM_CLIMB_002/OUT_MM_CLIMB_002_{col}_{row}HI.BMP',OUT/f'tile-{col}-{row}HI.bmp') for col in range(8) for row in range(3))
    powershell=Path(os.environ['SystemRoot'])/'System32/WindowsPowerShell/v1.0/powershell.exe'
    for operation,asset,destination in jobs:
        command=' '.join(shlex.quote(value) for value in (msys(executable),operation,msys(args.game_dir),asset,msys(destination)))
        result=subprocess.run([str(powershell),'-NoProfile','-File',str(ROOT/'scripts/Invoke-Ucrt64.ps1'),command],cwd=ROOT,capture_output=True,text=True)
        if result.returncode:
            raise RuntimeError(result.stdout+'\n'+result.stderr)
    digest=hashlib.sha256(level.read_bytes()).hexdigest()
    expected='95f84f86fb53ab29ce2ce3aed794cf69a06e5754d42ef78657f3af1a5db4c567'
    assert digest==expected, 'Retail level differs from the captured fixture authority'
    print(json.dumps({'prepared_assets':len(jobs),'level_sha256':digest,'output_directory':str(OUT)}))

if __name__=='__main__':main()
