from selenium import webdriver
from selenium.webdriver.common.by import By
driver = webdriver.Chrome()
url = "http://pubs.acs.org/doi/pdf/10.1021/acs.jpca.2c04349"
res = driver.get(url)
btn = driver.find_element(By.ID,"download")


download_dir = "C:\\Users\\omprakashpk\\Documents" # for linux/*nix, download_dir="/usr/Public"
options = webdriver.ChromeOptions()

profile = {"plugins.plugins_list": [{"enabled": False, "name": "Chrome PDF Viewer"}], # Disable Chrome's PDF Viewer
               "download.default_directory": download_dir , "download.extensions_to_open": "applications/pdf"}
options.add_experimental_option("prefs", profile)
driver = webdriver.Chrome('C:\\chromedriver\\chromedriver_2_32.exe', chrome_options=options)  # Optional argument, if not specified will search path


download_dir = "c:\\TMP"
options = webdriver.ChromeOptions()
#options.add_argument("--headless")
prefs = {"plugins.always_open_pdf_externally": True, "download.default_directory": download_dir }
options.add_experimental_option("prefs", prefs)
driver = webdriver.Chrome(options=options)
driver.get(url)
driver.quit()
