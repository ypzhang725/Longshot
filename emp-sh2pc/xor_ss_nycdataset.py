#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import random
import pandas as pd
# Num of shares, define as global for debug purpose
M = 2
# generate additive shares 
def additive_share(secret):
    randoms  = random.randrange(1000)
    shares = randoms ^ secret 
    return shares, randoms
    
def test_stub():
 #   df = pd.read_csv('nycTaxiData_payment_type.csv')
    df = pd.read_csv('bin40.csv')
    #print(df)
    lines1 = []
    lines2 = []

    for d in df['payment_type']:
        shares, randoms = additive_share(d)
        lines1.append(shares)
        lines2.append(randoms)

    with open('bin40_ss1.txt', 'w') as f:
        for line in lines1:
            f.write(str(line))
            f.write('\n')

    with open('bin40_ss2.txt', 'w') as f:
        for line in lines2:
            f.write(str(line))
            f.write('\n')
            
test_stub()
