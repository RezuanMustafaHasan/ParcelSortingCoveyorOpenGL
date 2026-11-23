#!/usr/bin/env python3
"""Reproduce full-resolution captures from the inspected macOS arm64 project."""
import json, os, subprocess, sys
from pathlib import Path
from PIL import Image

here=Path(__file__).resolve().parent
project=Path(sys.argv[1]).resolve()
out=here/'regenerated'
out.mkdir(exist_ok=True)
includes=[project/'include',project/'build/_deps/glfw-src/include',project/'build/gladsources/glad_gl_core_33/include',project/'build/_deps/glm-src']
flags=['/usr/bin/c++','-O3','-DNDEBUG','-std=c++17','-arch','arm64']+['-I'+str(p) for p in includes]
for stem in ['CaptureMain','CaptureCamera']:
    subprocess.run(flags+['-c',str(here/(stem+'.cpp')),'-o',str(out/(stem+'.o'))],check=True)
objects=[str(project/('build/CMakeFiles/conveyor_factory.dir/src/'+s+'.cpp.o')) for s in ['Shader','Geometry','FactoryScene','FactorySimulation']]
exe=out/'report_capture'
subprocess.run(['/usr/bin/c++','-O3','-arch','arm64',str(out/'CaptureMain.o'),str(out/'CaptureCamera.o'),*objects,str(project/'build/_deps/glfw-build/src/libglfw3.a'),str(project/'build/libglad_gl_core_33.a'),'-framework','Cocoa','-framework','IOKit','-framework','CoreFoundation','-o',str(exe)],check=True)
metadata=json.loads((here.parent/'assets/capture_metadata.json').read_text())
for frame in metadata:
    env=os.environ.copy()
    for flag in ['REPORT_CAMERA','REPORT_PAUSED','REPORT_LIGHT_OFF']:env.pop(flag,None)
    env['REPORT_CAMERA']=frame['camera']
    env.update({k:v for k,v in frame['options'].items() if k!='gouraud'})
    ppm=out/(frame['name']+'.ppm')
    command=[str(exe),'--capture',str(ppm),'--time',str(frame['time']),'--view','1']
    if frame['options'].get('gouraud'):command.append('--gouraud')
    result=subprocess.run(command,cwd=project,env=env,text=True,capture_output=True,check=True)
    Image.open(ppm).save(out/(frame['name']+'.png'))
    (out/(frame['name']+'.log')).write_text(result.stdout+result.stderr)
    print(frame['name'])
for crop in json.loads((here.parent/'assets/crop_manifest.json').read_text()):
    Image.open(out/crop['source']).crop(tuple(crop['crop_pixels'])).save(out/crop['output'])
