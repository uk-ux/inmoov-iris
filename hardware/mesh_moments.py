import pathlib,json,struct,math
a=pathlib.Path('control_ui/twin/assets')
for p in json.loads((a/'components.json').read_text()):
 if p['parent'].startswith(('TopskullV','TopBack')) and p['component']<2:
  f=list(struct.iter_unpack('<12fH',(a/p['file']).read_bytes()[84:]));v=[t[k:k+3] for t in f for k in (3,6,9)]
  n=len(v);c=[sum(t[k] for t in v)/n for k in range(3)]
  xx=sum((t[0]-c[0])**2 for t in v);yy=sum((t[1]-c[1])**2 for t in v);xy=sum((t[0]-c[0])*(t[1]-c[1]) for t in v)
  print(p['parent'],p['component'],'centroid',c,'angle',math.degrees(math.atan2(2*xy,xx-yy)/2))
