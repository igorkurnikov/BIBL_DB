import sys
import os
import requests
from selenium import webdriver
from selenium.webdriver.common.by import By
from .webofscience import * 
from .gscholar import *
from .scidir import *
from .gui import *

bibldbc_loaded = 0
try:
  from .bibldbc import *
  bibldbc_loaded = 1
except:
  print(" bibldbc module is not loaded - some functionality will be switched off")

name = "biblpy"

__all__= ["webofscience","gscholar","scidir"]

driver = None

def init_webdriver():
    chrome_options = webdriver.ChromeOptions() 
    chrome_options.add_experimental_option("excludeSwitches", ['enable-automation'])  # not to show controlled by automation software banner
    #chrome_options.add_argument("disable-web-security" )  # allow to login removing non-secure browser error
    #chrome_options.add_argument('user-data-dir' )
    #chrome_options.add_argument('allow-running-insecure-content' )
    driver = webdriver.Chrome(options=chrome_options)
    return driver

def save_url_to_file( url, fname ):
        r = requests.get(url, allow_redirects=True)
        fout = open(fname,"wb")
        fout.write(r.content)
        fout.close()

def get_download_path():
    """Returns the default downloads path for linux or windows"""
    if os.name == 'nt':
        import winreg
        sub_key = r'SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders'
        downloads_guid = '{374DE290-123F-4565-9164-39C4925E467B}'
        with winreg.OpenKey(winreg.HKEY_CURRENT_USER, sub_key) as key:
            location = winreg.QueryValueEx(key, downloads_guid)[0]
        return location
    else:
        return os.path.join(os.path.expanduser('~'), 'downloads')


# print("biblpy.__init__()")
# driver = init_webdriver()


