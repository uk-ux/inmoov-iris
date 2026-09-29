"""Download official head meshes and build a provenance manifest (no CAD edits)."""
import concurrent.futures, hashlib, json, pathlib, re, struct, urllib.request
ROOT=pathlib.Path(__file__).resolve().parents[1]
DEST=ROOT/'control_ui'/'twin'/'assets'
DEST.mkdir(parents=True,exist_ok=True)
items=[]
for group,source in [('i2Head','i2head-source.html'),('i2Eyes','i2eyes-source.html')]:
    page=(ROOT/'hardware'/source).read_text(encoding='utf-8')
    names=sorted(set(re.findall(r'value="([^"<>]+\.stl)"',page)))
    for name in names:
        if 'Mold' in name: continue
        items.append((group,name))
def download(item):
    group,name=item
    url=f'https://inmoov.fr/wp-content/uploads/stl/{group}/{name}'
    path=DEST/name
    if not path.exists():
        req=urllib.request.Request(url,headers={'User-Agent':'InMoov local assembly viewer'})
        with urllib.request.urlopen(req,timeout=90) as r: data=r.read()
        if len(data)<84 or data[:30].lower().startswith(b'<!doctype'): raise ValueError('Invalid STL '+name)
        path.write_bytes(data)
    data=path.read_bytes();count=struct.unpack_from('<I',data,80)[0]
    lo=[float('inf')]*3;hi=[-float('inf')]*3
    if 84+count*50==len(data):
        for face in struct.iter_unpack('<12fH',data[84:]):
            for i in (3,6,9):
                for j in range(3):lo[j]=min(lo[j],face[i+j]);hi[j]=max(hi[j],face[i+j])
    else:
        vertices=re.findall(rb'vertex\s+([-+\deE.]+)\s+([-+\deE.]+)\s+([-+\deE.]+)',data)
        count=len(vertices)//3
        if not vertices:raise ValueError('Unrecognized STL '+name)
        for vertex in vertices:
            for j in range(3):lo[j]=min(lo[j],float(vertex[j]));hi[j]=max(hi[j],float(vertex[j]))
    result={'file':name,'group':group,'url':url,'triangles':count,'bytes':len(data),'min':lo,'max':hi,'sha256':hashlib.sha256(data).hexdigest()}
    print(name,count,'triangles',flush=True)
    return result
with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool: results=list(pool.map(download,items))
(DEST/'manifest.json').write_text(json.dumps({'author':'Gael Langevin / InMoov','license':'CC BY-NC 3.0','licenseUrl':'https://creativecommons.org/licenses/by-nc/3.0/','sources':['https://inmoov.fr/inmoov-stl-parts-viewer/?bodyparts=i2Head','https://inmoov.fr/inmoov-stl-parts-viewer/?bodyparts=i2Eyes'],'parts':results},indent=2),encoding='utf-8')
print('Saved',len(results),'parts; MB',round(sum(r['bytes'] for r in results)/1e6,1))
