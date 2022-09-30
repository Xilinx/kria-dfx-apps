import numpy as np

def addpad(data):
    padcount = 16 - data.size%16
    if padcount < 4:
        padcount = padcount + 16
    pad = np.zeros(padcount, dtype=np.uint8)
    pad[-4] = padcount       & 0xFF
    pad[-3] = (padcount >> 8)  & 0xFF
    pad[-2] = (padcount >> 16) & 0xFF
    pad[-1] = (padcount >> 32) & 0xFF
    data = np.append(data, pad)
    return data, padcount

def removePad(data):
    padcount = data[-4] ^ (data[-3] << 8) ^ (data[-2] << 16) ^ (data[-1] << 24)
    return data[:-padcount]