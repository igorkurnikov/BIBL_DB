import biblpy

driver = biblpy.InitWebDriver()
wos = biblpy.Wos( driver )
wos.open_main_page()

