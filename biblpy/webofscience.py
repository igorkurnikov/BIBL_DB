import sys
#import requests
from selenium import webdriver
from selenium.webdriver.common.by import By

class Wos:

    wos_cgi   = "http://apps.webofknowledge.com/InboundService.do"
    wos_cgi_2 = "http://apps.isiknowledge.com/"

    CITED_REFS = (By.XPATH,  "//a[@title='View this record’s bibliography']")
    CITING_REFS = (By.XPATH, "//div[@id='sidebar-container']//a[@title='View all of the articles that cite this one']")
    MARKED_LIST = (By.XPATH, "//a[@title='Marked List']")

    def __init__( self, driver_p = None):

        if( driver_p != None ):
            self.driver = driver_p
        else:
            self.InitWebDriver()

    def InitWebDriver( self ):
        chrome_options = webdriver.ChromeOptions() 
        chrome_options.add_experimental_option("excludeSwitches", ['enable-automation'])  # not to show controlled by automation software banner
        self.driver = webdriver.Chrome(options=chrome_options)  

    def open_main_page( self ):
        self.driver.get("https://www.isiknowledge.com")

    def open_ref( self, wosid : str):
        url = self.wos_cgi
        url += "?product=WOS&UT="
        url += wosid
        url += "&SrcApp=EndNote&Init=Yes&action=retrieve&SrcAuth=ResearchSoft&Func=Frame&customersID=ResearchSoft&IsProductCode=Yes&mode=FullRecord"
        self.driver.get(url)

    def open_ref_cited( self, wosid : str):
        self.open_ref(wosid)
        try:
            elem = self.driver.find_element(*self.CITED_REFS)
            elem.click()
        except:
            print("Cited References not found ")

    def open_ref_citing( self, wosid : str):
        self.open_ref(wosid)
        try:
            elem = self.driver.find_element(*self.CITING_REFS)
            elem.click()
        except:
            print("Cited References not found ")

    def open_marked_list( self ):
        self.open_main_page()
        try:
            elem = self.driver.find_element(*self.MARKED_LIST)
            elem.click()
        except:
            print("Marked List not found ")


    
        





