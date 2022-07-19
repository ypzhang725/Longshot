import pandas as pd  
#scr = [1, 1, 1, 1]
# change here!!! and then use xor_ss_nycdataset.py to generate secret shares
scr = [i+1 for i in range(40)]  

for i in range(15):
    scr.extend(scr)
# dictionary of lists  
dict = {'payment_type': scr}  
       
df = pd.DataFrame(dict) 
    
# saving the dataframe 
df.to_csv('bin40.csv', index=False) 
