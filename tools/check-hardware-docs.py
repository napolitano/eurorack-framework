#!/usr/bin/env python3
"""Validate concrete IC-driver documentation, metadata, examples, and test mappings."""
from __future__ import annotations
import json
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
REQUIRED = {"schemaVersion","library","publicHeader","hardware","category","documentation","libraryDocumentation","examples","tests","manufacturer","references","status"}
CATEGORIES = {"adc","dac","display","gpio","led","multiplexer","shift-register"}
STATUSES = {"experimental","alpha","beta","stable","deprecated"}
HEADINGS = ("## Overview","## Hardware function","## Key electrical and protocol characteristics","## Pin and signal integration","## Framework operation sequence","## Framework support","## Dependencies and ownership","## Minimal code example","## Integration notes","## Example and tests","## Manufacturer references")
errors=[]; seen=set()

def fail(msg:str)->None: errors.append(msg)
def rel_exists(value:str)->bool: return (ROOT/value).exists()

for lib in sorted((ROOT/'libraries').glob('eurorack-driver-*')):
 p=lib/'hardware-doc.json'
 if not p.is_file(): fail(f'{lib.name}: missing hardware-doc.json'); continue
 try: m=json.loads(p.read_text(encoding='utf-8'))
 except Exception as e: fail(f'{lib.name}: invalid JSON: {e}'); continue
 missing=sorted(REQUIRED-set(m))
 if missing: fail(f'{lib.name}: missing fields: {", ".join(missing)}')
 if set(m)-REQUIRED: fail(f'{lib.name}: unknown fields: {", ".join(sorted(set(m)-REQUIRED))}')
 if m.get('schemaVersion')!=1: fail(f'{lib.name}: unsupported schemaVersion')
 if m.get('library')!=lib.name: fail(f'{lib.name}: library field mismatch')
 if m.get('category') not in CATEGORIES: fail(f'{lib.name}: invalid category')
 if m.get('status') not in STATUSES: fail(f'{lib.name}: invalid status')
 header=m.get('publicHeader')
 header_file=lib/'include'/str(header)
 if not isinstance(header,str) or not header_file.is_file(): fail(f'{lib.name}: publicHeader does not exist: {header}')
 doc_value=m.get('documentation'); doc=ROOT/str(doc_value)
 if not isinstance(doc_value,str) or not doc.is_file(): fail(f'{lib.name}: documentation does not exist: {doc_value}'); continue
 if doc.resolve() in seen: fail(f'{lib.name}: documentation page reused')
 seen.add(doc.resolve()); text=doc.read_text(encoding='utf-8')
 for heading in HEADINGS:
  if heading not in text: fail(f'{doc_value}: missing heading {heading}')
 if '```cpp' not in text: fail(f'{doc_value}: missing C++ example')
 refs=m.get('references')
 if not isinstance(refs,dict): fail(f'{lib.name}: references must be an object'); refs={}
 for key in ('product','datasheet'):
  url=refs.get(key)
  if not isinstance(url,str) or not url.startswith('https://'): fail(f'{lib.name}: invalid {key} URL')
  elif url not in text: fail(f'{doc_value}: {key} URL absent from page')
 for example in m.get('examples',[]):
  path=ROOT/example
  if not path.is_dir(): fail(f'{lib.name}: example directory missing: {example}'); continue
  main=path/'main.cpp'
  if not main.is_file(): fail(f'{lib.name}: example has no main.cpp: {example}'); continue
  main_text = main.read_text(encoding='utf-8')
  if isinstance(header,str) and f'#include <{header}>' not in main_text:
   fail(f'{lib.name}: example does not include publicHeader: {example}')
  if main_text.count(';') < 4:
   fail(f'{lib.name}: example appears to be an include-only placeholder: {example}')
  if 'IoResult::Success' not in main_text and 'sample.result' not in main_text:
   fail(f'{lib.name}: example does not demonstrate result handling: {example}')
  readme = path/'README.md'
  if not readme.is_file():
   fail(f'{lib.name}: example has no README.md: {example}')
 tests=m.get('tests',[])
 if not isinstance(tests,list) or len(tests)!=1:
  fail(f'{lib.name}: exactly one dedicated native test suite is required')
 for test in tests:
  test_path=ROOT/test
  if not test_path.is_dir(): fail(f'{lib.name}: test mapping missing: {test}'); continue
  if not test_path.name.startswith('test_driver_'): fail(f'{lib.name}: test suite is not dedicated: {test}')
  test_main=test_path/'test_main.cpp'
  if not test_main.is_file(): fail(f'{lib.name}: test suite has no test_main.cpp: {test}'); continue
  test_text=test_main.read_text(encoding='utf-8')
  if isinstance(header,str) and f'#include <{header}>' not in test_text:
   fail(f'{lib.name}: dedicated test does not include publicHeader: {test}')
  if test_text.count('RUN_TEST(') < 2:
   fail(f'{lib.name}: dedicated test must contain at least two cases: {test}')
 if not isinstance(m.get('hardware'),list) or not m['hardware']: fail(f'{lib.name}: hardware must be a non-empty list')

if errors:
 print('Hardware documentation validation failed:')
 for e in errors: print(f'- {e}')
 raise SystemExit(1)
print(f'Hardware documentation validation passed for {len(seen)} concrete driver libraries.')
