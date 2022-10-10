from ctypes import *
import ctypes
import numpy as np
import os
import time

libsiha = CDLL(os.getcwd()+"/lib/"+"libsiha.so")

libsiha.configDMSlots.argtypes = [c_int, c_uint32]
libsiha.configDMSlots.restypes = c_int

libsiha.InitializeMapRMs.argtypes = [c_int]
libsiha.InitializeMapRMs.restypes = c_int

libsiha.initRMs.argtypes = [c_int]
libsiha.initRMs.restypes = c_int

libsiha.mapRMs.argtypes = [c_int]
libsiha.mapRMs.restypes = c_int

libsiha.unmapRMs.argtypes = [c_int]
libsiha.unmapRMs.restypes = c_int

libsiha.StartAccel.argtypes = [c_int]
libsiha.StartAccel.restypes = c_int

libsiha.StopAccel.argtypes = [c_int]
libsiha.StopAccel.restypes = c_int

libsiha.FinaliseUnmapRMs.argtypes = [c_int]
libsiha.FinaliseUnmapRMs.restypes = c_int

libsiha.DataToAccel.argtypes = [c_int, c_uint64, c_uint64, c_uint8]
libsiha.DataToAccel.restypes = c_int

libsiha.DataFromAccel.argtypes = [c_int, c_uint64, c_uint64]
libsiha.DataFromAccel.restypes = c_int

libsiha.DataToAccelDone.argtypes = [c_int]
libsiha.DataToAccelDone.restypes = c_int

libsiha.DataFromAccelDone.argtypes = [c_int]
libsiha.DataFromAccelDone.restypes = c_int

libsiha.closeFDs.argtypes = [c_int]
libsiha.closeFDs.restypes = None

libsiha.loadData.argtypes = [np.ctypeslib.ndpointer(dtype=np.uint32, ndim=1, flags='C_CONTIGUOUS'), c_int, c_int]
libsiha.loadData.restypes = None

libsiha.retrieveData.argtypes = [np.ctypeslib.ndpointer(dtype=np.uint32, ndim=1, flags='C_CONTIGUOUS'), c_int, c_int]
libsiha.retrieveData.restypes = None

class AcapdAccel:
    def __init__(self, accelName, slot):
        self.slot = slot
        self.accelName = accelName
        self.open = libsiha.InitializeMapRMs(self.slot)
        self.inThroughput = 0
        self.outThroughput = 0
        self.timeTaken = 0
    
    def loadData(self, inbuff, offset):
        return libsiha.loadData(inbuff, offset, len(inbuff))
    
    def retrieve(self, offset, size):
        outbuff = np.zeros(size, dtype=np.uint32)
        libsiha.retrieveData(outbuff, offset, size*4)
        return outbuff
    
    def startAccel(self):
        return libsiha.StartAccel(self.slot)
    
    def stopAccel(self):
        return libsiha.StopAccel(self.slot)
    
    def config(self, offset, size, tid):
        offset*=4
        size//=4
        return libsiha.DataToAccel(self.slot, offset, size, tid)
    
    def config_fft(self, offset, size):
        print("FFT TEST :\n")
        status = 1
        if status : 
            libsiha.DataToAccel(self.slot,offset,size,1)
            status = libsiha.DataToAccelDone(self.slot)
        if status :
            print("\t Configure FFT Ch0 done.\n") 
            libsiha.DataToAccel(self.slot,offset,size,2)
            status = libsiha.DataToAccelDone(self.slot)
        if status : 
            print("\t Configure FFT Ch1 done.\n")
            libsiha.DataToAccel(self.slot,offset,size,3)
            status = libsiha.DataToAccelDone(self.slot)
        if status : 
            print("\t Configure FFT Ch2 done.\n")
            libsiha.DataToAccel(self.slot,offset,size,4)
            status = libsiha.DataToAccelDone(self.slot)
        print("\t Configure FFT Ch3 done.\n")

    def run(self, offset, size, tid, offset2):
        offset*=4
        size//=4
        offset2*=4
        start = time.time()
        libsiha.DataToAccel(self.slot,offset,size,tid)
        print("\t Configure Input data done.\n")
        libsiha.DataFromAccel(self.slot, offset2, size)
        status = libsiha.DataFromAccelDone(self.slot)
        if status :
            print("\t Received Data From Accel.\n")
        stop = time.time()
        size*=16
        self.timeTaken = stop-start
        self.inThroughput = size/(stop - start)
        self.outThroughput = size/(stop - start)

    def close(self):
        return libsiha.FinaliseUnmapRMs(self.slot)
    
    def getTimeTaken(self):
        return self.timeTaken
    
    def getInThroughput(self):
        return 8*self.inThroughput/1000000000
    
    def getOutThroughput(self):
        return 8*self.outThroughput/1000000000
    
    