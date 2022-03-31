import pandas as pd  
scr = [1, 2, 3, 4] 

for i in range(20):
    scr.extend(scr)
# dictionary of lists  
dict = {'payment_type': scr}  
       
df = pd.DataFrame(dict) 
    
# saving the dataframe 
df.to_csv('bin4.csv') 
