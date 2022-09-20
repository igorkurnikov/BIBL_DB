import biblpy

driver = biblpy.init_webdriver()
wos = biblpy.Wos( driver )
wos.open_main_page()

