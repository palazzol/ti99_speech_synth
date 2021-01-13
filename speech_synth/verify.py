# -*- coding: utf-8 -*-
"""
Created on Tue Jan 12 16:05:02 2021

@author: frank
"""

with open('dump.txt','r') as infile:
    with open('test.bin','rb') as good:
        for line in infile:
            for i in range(0,16):
                s = line[i*2:i*2+2]
                v = int(s,16)
                b = good.read(1)[0]
                if v != b:
                    print(v,b)
                