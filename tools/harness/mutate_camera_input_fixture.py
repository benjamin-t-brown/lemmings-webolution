"""Per-field input-to-pixel audit. Failed baselines remain failed."""
import hashlib
import json
import subprocess
import sys
from pathlib import Path
from PIL import Image,ImageChops
ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'build/tmp/a1-facilitator'
OUT.mkdir(parents=True,exist_ok=True)
FIXTURES=Path(__file__).resolve().parent/'fixtures/camera_a1'
policy=json.loads((FIXTURES/'input-mutation-policy.json').read_text())
roi=policy['roi'];records=[]
def render(case,label):
    spec=OUT/(case['name']+'-params.json')
    state=FIXTURES/('reference-'+case['name']+'-state.json')
    subprocess.run([sys.executable,str(ROOT/'tools/harness/render_camera_skin.py'),'--hi','--linear','--parameters',str(spec),'--state',str(state),'--label',label],check=True,capture_output=True,text=True)
    path=OUT/f'skin-candidate-{label}.png'
    reference=Image.open(FIXTURES/f"original-{case['name']}-0.png").convert('RGB').crop(roi)
    candidate=Image.open(path).convert('RGB');assert candidate.size==(640,480)
    histogram=ImageChops.difference(reference,candidate.crop(roi)).histogram()
    error=sum((i%256)*n for i,n in enumerate(histogram))/(255*3*reference.width*reference.height)
    return {'error':error,'passes_threshold':error<=policy['threshold'],'png_sha256':hashlib.sha256(path.read_bytes()).hexdigest()}
for case in policy['cases']:
    spec=OUT/(case['name']+'-params.json')
    if not spec.exists():spec.write_bytes((FIXTURES/spec.name).read_bytes())
    saved=spec.read_bytes();original=json.loads(saved)
    baseline=render(case,case['label']+'-audit-base')
    changed=original.copy();changed[case['field']]=case['mutation']
    assert [k for k in changed if changed[k]!=original[k]]==[case['field']]
    try:
        spec.write_text(json.dumps(changed,indent=2))
        mutation=render(case,case['label']+'-mutated-'+case['field'])
    finally:spec.write_bytes(saved)
    assert spec.read_bytes()==saved
    restored=render(case,case['label']+'-restored')
    assert restored==baseline
    worse=mutation['error']>baseline['error']
    record={'field':case['field'],'reference_case':case['name'],'baseline':baseline,'mutation':mutation,
            'mutation_strictly_worse':worse,'accepted_for_this_fixture':baseline['passes_threshold'] and worse,
            'restored_input_sha256':hashlib.sha256(saved).hexdigest(),'restored_png_sha256':restored['png_sha256']}
    records.append(record);print(json.dumps(record),flush=True)
    (OUT/'input-mutations.json').write_text(json.dumps(records,indent=2))
    assert worse,('Input value unprotected by selected fixture',case['field'])
rejected=[record['field'] for record in records if not record['accepted_for_this_fixture']]
if rejected:
    print(json.dumps({'audit_complete':True,'rejected_baselines':rejected}),flush=True)
    raise SystemExit(1)
