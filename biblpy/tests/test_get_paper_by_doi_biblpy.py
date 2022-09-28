import biblpy

#url = "http://pubs.acs.org/doi/pdf/"
#url += "10.1021/acs.jctc.8b00174"

url="http://aip.scitation.org/doi/pdf/10.1063/5.0083060"
#fname = "c:\\TMP\\test1.pdf"
fname = "c:\\users\\igor\\Google Drive\\bibl\\pdf\\jcp\\jcp_22_156_144103\\jcp_22_156_144103.pdf"

biblpy.save_url_to_file(url,fname)

