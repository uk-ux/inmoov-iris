import pathlib,re,urllib.request,concurrent.futures
base=pathlib.Path(__file__).parent;dest=base/'assembly-references';dest.mkdir(exist_ok=True)
page=(base/'i2head-assembly-source.html').read_text(encoding='utf-8')
urls=sorted(set(re.findall(r'src="(https://inmoov.fr/wp-content/uploads/[^" ]+/head_[^" ]+\.png)"',page)))
def get(url):
 p=dest/url.split('/')[-1]
 if not p.exists():
  try:
   req=urllib.request.Request(url,headers={'User-Agent':'Mozilla/5.0','Referer':'https://inmoov.fr/headi2/'})
   with urllib.request.urlopen(req,timeout=60) as r:p.write_bytes(r.read())
  except Exception as e:return f'{p.name}: {e}'
 return p.name
with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:print(list(pool.map(get,urls)))
