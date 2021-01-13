# -*- coding: utf-8 -*-
"""
Created on Sat Jan  9 23:42:54 2021

@author: frank
"""

rom = []
with open('cd2325a.u2a','rb') as f:
    rom = f.read(0x4000)
with open('cd2326a.u2b','rb') as f:
    rom = rom + f.read(0x4000)

bitreverse = {}
for i in range(0,0x100):
    b = 0
    for j in range(0,8):
        b = b | (((i>>j) & 0x01) << (7-j))
    bitreverse[i] = b
#print(bitreverse)

with open('test.bin','wb') as f:
    for i in range(0,0x8000):
        f.write(bitreverse[rom[i]].to_bytes(1,byteorder='little'))
        
def read(addr):
    return addr+1, bitreverse[int(rom[addr])]

def readaddress(addr):
    addr, high = read(addr)
    addr, low = read(addr)
    return addr, high*256+low

def readnode(addr):
    addr, length = read(addr)
    name = ''
    for i in range(0,length):
        addr, c = read(addr)
        name = name + chr(c)
    addr, prevAddr = readaddress(addr)
    addr, nextAddr = readaddress(addr)
    addr, unknown = read(addr)
    addr, dataAddr = readaddress(addr)
    addr, dataLength = read(addr)
#    print(name, hex(prevAddr), hex(nextAddr), unknown, hex(dataAddr), hex(dataLength))
    print(name, hex(dataAddr), hex(dataLength))
    return addr, name, prevAddr, nextAddr, unknown, dataAddr, dataLength

def readnoderecursive(addr):
    addr, name, prevAddr, nextAddr, unknown, dataAddr, dataLength = readnode(addr)
    if prevAddr != 0:
        readnoderecursive(prevAddr)
    if nextAddr != 0:
        readnoderecursive(nextAddr)
         
        
def readAll():
    addr = 0
    addr, data = read(addr)
    readnoderecursive(addr)

readAll()
