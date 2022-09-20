import biblpy
import requests

url = "http://pubs.acs.org/doi/pdf/"
url += "10.1021/acs.jctc.8b00174"

r = requests.get(url, allow_redirects=True)
fout = open("c:\\TMP\\test.pdf","wb")
fout.write(r.content)
fout.close()
