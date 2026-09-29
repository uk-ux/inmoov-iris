"""Separate disconnected print pieces without changing their triangles."""
import pathlib,struct,json,collections,hashlib
ROOT=pathlib.Path(__file__).resolve().parents[1]
ASSETS=ROOT/'control_ui/twin/assets'
output=ASSETS/'components';output.mkdir(exist_ok=True)
manifest=json.loads((ASSETS/'manifest.json').read_text())
result=[]
for meta in manifest['parts']:
 # All files: even skull covers can contain both halves in print orientation.
 data=(ASSETS/meta['file']).read_bytes();n=struct.unpack_from('<I',data,80)[0]
 parent=list(range(n));owners={}
 def find(i):
  while parent[i]!=i:parent[i]=parent[parent[i]];i=parent[i]
  return i
 def union(a,b):
  a,b=find(a),find(b)
  if a!=b:parent[b]=a
 for i,face in enumerate(struct.iter_unpack('<12fH',data[84:])):
  for k in (3,6,9):
   v=tuple(round(face[k+j],4) for j in range(3))
   if v in owners:union(i,owners[v])
   else:owners[v]=i
 groups=collections.defaultdict(list)
 for i in range(n):groups[find(i)].append(i)
 groups=sorted(groups.values(),key=len,reverse=True)
 print(meta['file'],len(groups),'components',flush=True)
 for index,indices in enumerate(groups):
  lo=[float('inf')]*3;hi=[-float('inf')]*3
  chunks=[]
  for i in indices:
   chunk=data[84+i*50:134+i*50];chunks.append(chunk);face=struct.unpack('<12fH',chunk)
   for k in (3,6,9):
    for j in range(3):lo[j]=min(lo[j],face[k+j]);hi[j]=max(hi[j],face[k+j])
  name=meta['file'].replace('.stl',f'-part{index}.stl')
  derived=data[:80]+struct.pack('<I',len(indices))+b''.join(chunks)
  (output/name).write_bytes(derived)
  record={**meta,'parent':meta['file'],'parent_sha256':meta['sha256'],'sha256':hashlib.sha256(derived).hexdigest(),'file':'components/'+name,'component':index,'triangles':len(indices),'bytes':84+len(indices)*50,'min':lo,'max':hi}
  result.append(record)
  print(index,len(indices),'center',[round((lo[j]+hi[j])/2,2) for j in range(3)],'size',[round(hi[j]-lo[j],2) for j in range(3)])
(ASSETS/'components.json').write_text(json.dumps(result,indent=2))
