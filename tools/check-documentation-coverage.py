#!/usr/bin/env python3
import sys
from pathlib import Path
from library_layout import discover_libraries
def main():
 root=Path(__file__).resolve().parents[1];libs=discover_libraries(root);text='\n'.join(p.read_text(encoding='utf-8',errors='replace') for p in [root/'README.md',*root.glob('docs/**/*.md')]);missing=[]
 for name,lib in libs.items():
  if name not in text: missing.append(name)
  if not lib.headers() and lib.sources(): missing.append(name+' (implementation without public header)')
 if missing:
  print('Documentation coverage failed:',file=sys.stderr);[print('- '+m,file=sys.stderr) for m in missing];return 1
 print(f'Documentation names all {len(libs)} libraries; implementation libraries expose public headers.');return 0
if __name__=='__main__': raise SystemExit(main())
