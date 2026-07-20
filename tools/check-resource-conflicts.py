#!/usr/bin/env python3
import argparse,json,sys
from pathlib import Path
from library_layout import discover_libraries,dependency_closure
def main():
 p=argparse.ArgumentParser();p.add_argument('profile');a=p.parse_args();root=Path(__file__).resolve().parents[1];libs=discover_libraries(root);profile=json.loads(Path(a.profile).read_text());selected=dependency_closure(profile['libraries'],libs);reg=json.loads((root/'docs/architecture/framework-resources.json').read_text());owners={};errors=[]
 for lib in selected:
  data=reg.get(lib,{})
  share=set(data.get('shareable',[]))
  for resource in data.get('resources',[]):
   if resource in owners and resource not in share and resource not in owners[resource][1]: errors.append(f'{resource}: {owners[resource][0]} conflicts with {lib}')
   else: owners[resource]=(lib,share)
 if errors:
  print('Resource conflicts:',file=sys.stderr);[print('- '+x,file=sys.stderr) for x in errors];return 1
 print(f'No exclusive-resource conflicts in {a.profile}; checked {len(selected)} libraries.');return 0
if __name__=='__main__': raise SystemExit(main())
