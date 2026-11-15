from pathlib import Path
import hashlib,zipfile
parts=sorted(Path('source').glob('IMPERIO_NEXION_SOURCE_part-*.bin'))
if len(parts)!=7: raise SystemExit(f'Expected seven source chunks, got {len(parts)}')
archive=Path('source/IMPERIO_NEXION_SOURCE_109.zip')
with archive.open('wb') as out:
 for part in parts:
  with part.open('rb') as src:
   while chunk:=src.read(1024*1024): out.write(chunk)
digest=hashlib.md5(archive.read_bytes()).hexdigest()
if digest!='a6387ab3ac7ee28ed33679be564a5e18': raise SystemExit(f'MD5 mismatch: {digest}')
root=Path('.').resolve();prefix='IMPERIO_NEXION_SOURCE/'
with zipfile.ZipFile(archive) as zf:
 bad=zf.testzip()
 if bad: raise SystemExit(f'ZIP CRC failure: {bad}')
 for info in zf.infolist():
  n=info.filename
  if not n.startswith(prefix):continue
  rel=n[len(prefix):]
  if not rel or rel.startswith('.github/') or rel in ('.gitignore','LEIA_IMPERIO_SOURCE_ATUALIZADA.md'):continue
  if rel in ('app/src/main/cpp/samp/main.cpp','app/src/main/java/com/lurion/launcher/activity/MainActivity.java'):continue
  target=(root/rel).resolve()
  if root not in target.parents and target!=root:raise SystemExit(f'Unsafe path: {rel}')
  if info.is_dir():target.mkdir(parents=True,exist_ok=True)
  else:
   target.parent.mkdir(parents=True,exist_ok=True)
   with zf.open(info) as src,target.open('wb') as dst:
    while chunk:=src.read(1024*1024):dst.write(chunk)
print(f'Source reconstructed and verified; MD5={digest}, ZIP CRC passed; Urbano Java/C++ preserved.')
