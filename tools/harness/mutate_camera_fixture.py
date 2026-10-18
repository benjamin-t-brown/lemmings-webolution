"""Facilitator-owned one-scalar-at-a-time original-pixel audit with restoration."""
import hashlib
import json
import subprocess
import sys
from pathlib import Path
from PIL import Image,ImageChops

ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'build/tmp/a1-facilitator'
FIXTURES=Path(__file__).resolve().parent/'fixtures/camera_a1'
OUT.mkdir(parents=True,exist_ok=True)
params=OUT/'camera-fixture-params.json'
if not params.exists():params.write_bytes((FIXTURES/'camera-fixture-params.json').read_bytes())
saved=params.read_bytes();saved_hash=hashlib.sha256(saved).hexdigest()
base=json.loads(saved)
roi=json.loads((FIXTURES/'reference-roi.json').read_text())['roi']
reference=Image.open(FIXTURES/'original-0.png').convert('RGB').crop(roi)
denominator=255*3*reference.width*reference.height
def render(label):
    command=[sys.executable,str(ROOT/'tools/harness/render_camera_skin.py'),'--hi','--linear','--parameters',str(params),'--label',label]
    result=subprocess.run(command,check=True,capture_output=True,text=True)
    path=OUT/f'skin-candidate-{label}.png'
    image=Image.open(path).convert('RGB')
    assert image.size==(640,480)
    hist=ImageChops.difference(reference,image.crop(roi)).histogram()
    error=sum((i%256)*count for i,count in enumerate(hist))/denominator
    return {'error':error,'png_sha256':hashlib.sha256(path.read_bytes()).hexdigest()}

baseline=render('mutation-baseline')
assert baseline['error']<=.02
assert baseline['png_sha256']==json.loads((FIXTURES/'camera-mutations.json').read_text())['baseline']['png_sha256']
print('baseline',json.dumps(baseline),flush=True)
policy=json.loads((FIXTURES/'camera-mutation-policy.json').read_text())
results=[]
for field,value in policy['fields'].items():
    assert params.read_bytes()==saved, 'Prior mutation not restored'
    changed=json.loads(saved)
    if field.startswith('center['):
        component=int(field[7]);changed['center'][component]=value
    else:
        changed[field]=value
    def scalars(node,prefix=''):
        if isinstance(node,dict):
            return {k:v for key,val in node.items() for k,v in scalars(val,prefix+'/'+key).items()}
        if isinstance(node,list):
            return {k:v for i,val in enumerate(node) for k,v in scalars(val,prefix+f'[{i}]').items()}
        return {prefix:node}
    original_values=scalars(base);new_values=scalars(changed)
    difference=[key for key in original_values if original_values[key]!=new_values[key]]
    assert len(difference)==1,(field,difference)
    try:
        params.write_text(json.dumps(changed,indent=2),encoding='utf-8')
        mutation=render('mutation-'+field.replace('[','-').replace(']',''))
        mutation['strictly_worse']=mutation['error']>baseline['error']
        mutation['fails_threshold']=mutation['error']>.02
    finally:
        params.write_bytes(saved)
    assert hashlib.sha256(params.read_bytes()).hexdigest()==saved_hash
    restored=render('restored')
    assert restored==baseline,('Restored render differs',restored,baseline)
    row={'field':field,'changed_scalar_path':difference[0],'new_value':value,**mutation,
         'restored_input_sha256':saved_hash,'restored_png_sha256':restored['png_sha256']}
    results.append(row)
    print(json.dumps(row),flush=True)
    (OUT/'camera-mutations.json').write_text(json.dumps({'baseline':baseline,'roi':roi,'threshold':.02,'input_sha256':saved_hash,'results':results},indent=2))
    assert mutation['strictly_worse'],('Value is not protected by original-pixel fixture',field)
print('All individual pixel mutations worsened error and restored independently.',flush=True)
