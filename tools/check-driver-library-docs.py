#!/usr/bin/env python3
"""Validate software-oriented documentation for concrete IC driver libraries."""
from __future__ import annotations
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
HEADINGS=("## Purpose","## Public API","## Lifecycle and configuration","## Ownership and memory","## Error handling","## ISR, reentrancy, and concurrency","## Dependencies and resources","## Usage example","## Tests","## Limitations","## Related documentation")
errors=[]; seen=set(); count=0
def fail(x): errors.append(x)
for lib in sorted((ROOT/'libraries').glob('eurorack-driver-*')):
 p=lib/'hardware-doc.json'
 if not p.is_file(): continue
 m=json.loads(p.read_text(encoding='utf-8')); value=m.get('libraryDocumentation')
 if not isinstance(value,str): fail(f'{lib.name}: missing libraryDocumentation'); continue
 doc=ROOT/value
 if not doc.is_file(): fail(f'{lib.name}: library documentation missing: {value}'); continue
 if doc.resolve() in seen: fail(f'{lib.name}: library documentation page reused')
 seen.add(doc.resolve()); count+=1; text=doc.read_text(encoding='utf-8')
 for h in HEADINGS:
  if h not in text: fail(f'{value}: missing heading {h}')
 for required in (m.get('publicHeader',''),m.get('documentation','')):
  if required and required not in text: fail(f'{value}: missing reference {required}')
 for example in m.get('examples',[]):
  if example not in text: fail(f'{value}: missing example reference {example}')
 for test in m.get('tests',[]):
  if test not in text: fail(f'{value}: missing test reference {test}')
 if 'IoResult' not in text and 'AnalogSample' not in text: fail(f'{value}: missing explicit result-handling contract')
 if 'not reentrant' not in text.lower(): fail(f'{value}: missing explicit reentrancy statement')
if errors:
 print('Driver library documentation validation failed:')
 for e in errors: print(f'- {e}')
 raise SystemExit(1)
print(f'Driver library documentation validation passed for {count} libraries.')
