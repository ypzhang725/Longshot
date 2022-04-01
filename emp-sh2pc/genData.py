import pandas as pd  
scr = [1, 1, 1, 1] 

for i in range(20):
    scr.extend(scr)
# dictionary of lists  
dict = {'payment_type': scr}  
       
df = pd.DataFrame(dict) 
    
# saving the dataframe 
df.to_csv('bin1.csv', index=False) 
