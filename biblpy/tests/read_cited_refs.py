from selenium.webdriver.common.by import By
print(driver)
elem = driver.find_element(By.ID,"cited-refs-full-record")
print(elem)
cited_refs_list = elem.find_elements_by_class_name('search-results-item')
print(len(cited_refs_list))
