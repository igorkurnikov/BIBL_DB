import sys
import os
from selenium import webdriver
from selenium.webdriver.common.by import By
import biblpy
import time
import shutil

class GScholar:
    """Class to interact with Google Scholar site"""

    GSCHOLAR_URL = "https://scholar.google.com/"

    def __init__( self, driver_p = None, use_user_data = False ):

        if( driver_p != None ):
            self.driver = driver_p
        else:
            self.driver = biblpy.get_webdriver( use_user_data = True )
 

    def open_main_page( self ):
        self.driver.get(self.GSCHOLAR_URL)

    def find_ref_by_title( self, title : str): 
        """ Find paper by paper title """ 
        self.open_main_page()
        try:
            elem = self.driver.find_element(By.ID,"gs_hdr_tsi")
            if(elem):
              elem.send_keys(title)
            elem = self.driver.find_element(By.ID,"gs_hdr_tsb")
            if(elem):
              elem.click()
        except:
            print("Reference was not found on Google Scholar ")

    def find_ref_by_doi( self, doi : str): 
        """ Find paper by paper doi """ 
        self.open_main_page()
        try:
            elem = self.driver.find_element(By.ID,"gs_hdr_tsi")
            if(elem):
              elem.send_keys(doi)
            elem = self.driver.find_element(By.ID,"gs_hdr_tsb")
            if(elem):
              elem.click()
        except:
            print("Reference was not found on Google Scholar ")

    def find_info_from_refs( self ): 
        """ get information from the list of references """

        elem_ccl_mid = self.driver.find_element(By.ID,"gs_res_ccl_mid")
        print( elem_ccl_mid )
        if( elem_ccl_mid ):
            refs_elems = elem_ccl_mid.find_elements(By.CLASS_NAME,"gs_scl")
            print("num refs = " + str(len(refs_elems)))
            for ref_el in refs_elems:
                print(ref_el)
                cid = ref_el.get_attribute("data-cid")
                did = ref_el.get_attribute("data-did")
                lid = ref_el.get_attribute("data-lid")
                if( cid ): print(" cid = " + cid )
                if( did ): print(" did = " + did )
                if( lid ): print(" lid = " + lid )
                elem_cited = None
                try:
                    elem_cited = ref_el.find_element(By.XPATH,"//a[contains(text(),'Cited by')]")
                except:
                    pass
                if( elem_cited ):
                    href = elem_cited.get_attribute("href")
                    print(f"Cited By href = {href}")
                elem_versions = None
                try:
                    elem_versions = ref_el.find_element(By.XPATH,"//a[contains(text(),'versions')]")
                except:
                    pass
                if( elem_versions ):
                    href = elem_versions.get_attribute("href")
                    print(f"Versions href = {href}")
                authors = []
                try:
                    elem_ref_sh = ref_el.find_elements(By.CLASS_NAME,"gs_a")
                except:
                    pass


    def find_citing_refs( self ):
        
        cmd = "https://scholar.google.com/scholar?cites=18321766744038277522"





