#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import random
# Num of shares, define as global for debug purpose
M = 2
# generate additive shares 
def additive_share(secret):
    randoms  = random.randrange(1000)
    shares = randoms ^ secret 
    return shares, randoms

#reconstruct addtive shares
def additive_reconstruct(shares, randoms):
    return shares^randoms

def test_stub():
    lines1 = []
    lines2 = []
    data_real = [5, 3, 4, 1, 2]
    data_dummy = [0, 0, 0, 0, 0]
    dummy1 = []
    dummy2 = []

    for d in data_real:
        shares, randoms = additive_share(d)
        lines1.append(shares)
        lines2.append(randoms)

    for d in data_dummy:
        shares, randoms = additive_share(d)
        lines1.append(shares)
        lines2.append(randoms)

    for	d in data_real:
        shares, randoms = additive_share(d)
        lines1.append(shares)
        lines2.append(randoms)

    for d in data_dummy:
        shares, randoms = additive_share(d)
        lines1.append(shares)
        lines2.append(randoms)

    for i in range(5):
        shares, randoms = additive_share(1)
        dummy1.append(shares)
        dummy2.append(randoms)
    for	i in range(5):
        shares, randoms = additive_share(0)
        dummy1.append(shares)
        dummy2.append(randoms)
    for	i in range(5):
        shares, randoms = additive_share(1)
        dummy1.append(shares)
        dummy2.append(randoms)
    for i in range(5):
        shares, randoms = additive_share(0)
        dummy1.append(shares)
        dummy2.append(randoms)

    with open('ss1.txt', 'w') as f:
        for line in lines1:
            f.write(str(line))
            f.write('\n')

    with open('ss2.txt', 'w') as f:
        for line in lines2:
            f.write(str(line))
            f.write('\n')

    with open('dd1.txt', 'w') as f:
        for line in dummy1:
            f.write(str(line))
            f.write('\n')

    with open('dd2.txt', 'w') as f:
        for line in dummy2:
            f.write(str(line))
            f.write('\n')
            
test_stub()
