import biblpy
import time
url = "http://pubs.acs.org/doi/pdf/10.1021/acs.jpca.2c04349"
driver = biblpy.InitWebDriver()
driver.get(url)
time.sleep(10)

#driver.quit()

