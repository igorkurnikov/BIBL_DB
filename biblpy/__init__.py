import sys
import os
import requests
import time
import shutil
from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.common.exceptions import NoSuchElementException
from selenium.common.exceptions import WebDriverException

from .webofscience import * 
from .scidir import *
from .gs     import *
from .gui import *

bibldbc_loaded = 0
try:
  from .bibldbc import *
  bibldbc_loaded = 1
except:
  print(" bibldbc module is not loaded - some functionality will be switched off")

name = "biblpy"

__all__= ["webofscience","gscholar","scidir","gs"]

driver = None

def InitWebDriver( use_user_data = False ):
    options = webdriver.ChromeOptions() 
    prefs = {"plugins.always_open_pdf_externally": True}
    #prefs = {"plugins.always_open_pdf_externally": True, "download.default_directory": download_dir }
    #options.add_experimental_option("excludeSwitches", ['enable-automation'])  # not to show controlled by automation software banner
    options.add_experimental_option("prefs", prefs)
    if use_user_data : 
        options.add_argument('user-data-dir=C:/Users/igor/AppData/Local/Google/Chrome/User Data')
    #options.add_argument("disable-web-security" )  # allow to login removing non-secure browser error
    #chrome_options.add_argument('user-data-dir' )
    #chrome_options.add_argument('allow-running-insecure-content' )
    driver = webdriver.Chrome(options=options)
    return driver

def get_webdriver( use_user_data = False ):
    if( not driver or not is_webdriver_alive(driver) ): biblpy.driver = InitWebDriver( use_user_data )
    return biblpy.driver

def StopWebDriver():
    if( driver ): driver.quit()
    driver = None

def is_webdriver_alive(driver):

    print('Checking whether the driver is alive')
    try:
        assert(driver.service.process.poll() == None) #Returns an int if dead and None if alive
        driver.service.assert_process_still_running() #Throws a WebDriverException if dead
        driver.find_element(By.TAG_NAME,'html') #Throws a NoSuchElementException if dead
        print('The driver appears to be alive')
        return True
    except (NoSuchElementException, WebDriverException, AssertionError):
        print('The driver appears to be dead')
        return False
    except Exception as ex:
        print('Encountered an unexpected exception type ({}) while checking the driver status'.format(type(ex)))
        return False

def getDownLoadedFileName(driver, waitTime):
    #driver.execute_script("window.open()")
    # switch to new tab
    #driver.switch_to.window(driver.window_handles[-1])
    # navigate to chrome downloads
    driver.get('chrome://downloads')
    # define the endTime
    endTime = time.time()+waitTime
    while True:
        try:
            # get downloaded percentage
            downloadPercentage = driver.execute_script(
                "return document.querySelector('downloads-manager').shadowRoot.querySelector('#downloadsList downloads-item').shadowRoot.querySelector('#progress').value")
            # check if downloadPercentage is 100 (otherwise the script will keep waiting)
            if downloadPercentage == 100:
                # return the file name once the download is completed
                return driver.execute_script("return document.querySelector('downloads-manager').shadowRoot.querySelector('#downloadsList downloads-item').shadowRoot.querySelector('div#content  #file-link').text")
        except:
            return driver.execute_script("return document.querySelector('downloads-manager').shadowRoot.querySelector('#downloadsList downloads-item').shadowRoot.querySelector('div#content  #file-link').text")
        time.sleep(1)
        if time.time() > endTime:
            break

def save_url_to_file_requests( url, fname ):
        """ Copy a file downloaded from url to fname using requests library """
        r = requests.get(url, allow_redirects=True)
        fout = open(fname,"wb")
        fout.write(r.content)
        fout.close()

def save_url_to_file( url, fname ):
    """ Copy a file downloaded from url to fname using selenium driver """
    if( not biblpy.driver or not is_webdriver_alive(biblpy.driver) ): biblpy.driver = InitWebDriver()
    biblpy.driver.get(url)
    fn_d = getDownLoadedFileName(biblpy.driver,15)
    d_dir = get_download_path()
    fn_d_full = os.path.join(d_dir,fn_d)
    try:
        shutil.copyfile(fn_d_full, fname)
    except OSError as error:
        print(error)

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
    #  from pathlib import Path
    #  return str(Path.home() / "Downloads")



# print("biblpy.__init__()")
# driver = InitWebDriver()


