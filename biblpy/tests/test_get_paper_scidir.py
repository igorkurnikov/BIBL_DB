import biblpy
doi = "10.1016/j.sbi.2017.02.006"
sdir = biblpy.scidir()
url_pdf = sdir.get_pdf_url_by_doi(doi,"c:\\TMP\\test2.pdf")
