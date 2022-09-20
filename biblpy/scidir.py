import sys
import os
from selenium import webdriver
from selenium.webdriver.common.by import By
import biblpy
import time
import shutil

class scidir:
    """Class to interact with Science Direct site"""

    SCIDIR_URL = "https://www.sciencedirect.com/"
    DOWNLOAD_MAIN_BT = (By.XPATH,  "//button[@id='pdfLink']")
    DOWNLOAD_THIS_ARTICLE = (By.XPATH,  "//div[contains(@class,'PdfDropDownMenu')]//a[contains(@aria-label,'Download single PDF')]")
    SAVE_PDF_ICON_BT = (By.XPATH,  "//nav[@id='toolbar']//a[@id='save-pdf-icon-button']")
    PDF_PAGE_TOOLBAR = (By.XPATH,  "//nav[@id='toolbar']")

    DOI_SITE_URL = "http://dx.doi.org/"

    def __init__( self, driver_p = None):

        if( driver_p != None ):
            self.driver = driver_p
        else:
            self.init_webdriver()

    def init_webdriver( self ):
        chrome_options = webdriver.ChromeOptions() 
        chrome_options.add_experimental_option("excludeSwitches", ['enable-automation'])  # not to show controlled by automation software banner
        self.driver = webdriver.Chrome(options=chrome_options)  

    def get_pdf_url_by_doi( self, doi: str, pdf_path_db: str ):
        
        doi_url = self.DOI_SITE_URL + doi            
        self.driver.get(doi_url)
        try:
            elem = self.driver.find_element(*self.DOWNLOAD_MAIN_BT)
            elem.click()
            elem = self.driver.find_element(*self.DOWNLOAD_THIS_ARTICLE)
            url = elem.get_attribute("href")
            tokens = url.split("=")
            pdf_name_download = tokens[len(tokens)-1]
            download_path = biblpy.get_download_path()
            pdf_name_download_path =  os.path.join(download_path,pdf_name_download)
            url += "&download=true"
            print(url)
            self.driver.get(url)
            time.sleep(3)
            print("copy " + pdf_name_download_path + " -> " + pdf_path_db )
            shutil.copy(pdf_name_download_path, pdf_path_db)

            # elem.click()
            # elem = elf.driver.find_element(*self.PDF_PAGE_TOOLBAR)
            # elem = self.driver.find_element(*self.SAVE_PDF_ICON_BT)
            # elem.click()
        except:
            print("PDF Download Button is not working")
        return ""

    #   https://www.sciencedirect.com/science/article/pii/S0959440X17300301/pdfft?isDTMRedir=true&download=true
    #   https://www.sciencedirect.com/science/article/pii/S0959440X17300301/pdfft?md5=f4631c6c345c5e4946a02e08bd181a93&pid=1-s2.0-S0959440X17300301-main.pdf
    #   https://www.sciencedirect.com/science/article/pii/S0959440X17300301/pdfft?md5=f4631c6c345c5e4946a02e08bd181a93&pid=1-s2.0-S0959440X17300301-main.pdf&download=true




