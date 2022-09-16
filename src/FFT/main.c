/*
 * Copyright (C) 2022, Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 */

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

// XRT includes
#include "experimental/xrt_bo.h"
#include "experimental/xrt_device.h"
#include "experimental/xrt_kernel.h"

#define SIZE_IN_BYTES       0x40000

int InitializeMapRMs(int slot);
int FinaliseUnmapRMs(int slot);
void mapBuffer(xrt::bo boa);
int DataToAccel(int slot, uint64_t data, uint64_t size, uint8_t tid);
int DataFromAccel(int slot, uint64_t data, uint64_t size);
int DataToAccelDone(int slot);
int DataFromAccelDone(int slot);
//char * dfxmgr_uio_by_name(char *obuf, int slot, const char *name);

uint32_t config[] = {0x0000000c,0x0000000c,0x0000000c,0x0000000c};

uint32_t fft_data_in[] = {
	0xcca5a729, 0x4b276e90, 0x9a57a7e7, 0xd0bfe1c7
};
uint32_t fft_data_out[] = {
	0x88009C00, 0x5C006800, 0x5800E400, 0x1400A800
};

uint32_t resultbuff[] = {
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000
};

void isTestPassed(uint32_t *vptr)
{
	for (int i=0; i < 16; i++)					//Copying out data to resultbuff for comparison with golden data
	{
		resultbuff[i] = vptr[(i*1024)+17408];
	}
	int same_flag = 1;
		for (int i=0; i< 16; i++)
		{
			if(fft_data_out[i%4] != resultbuff[i])
			{
				same_flag=0;
				break;
			}
		}
		if(same_flag)
			printf("\t === TEST PASSED ===\n");
		else
			printf("\t === TEST FAILED ===\n");
}
		
int main(int argc, char *argv[])
{
	int slot =0;
	if(argc>1)
		slot = atoi (argv[1]);

	InitializeMapRMs(slot);
	auto device = xrt::device(0);
	auto bo_a = xrt::bo(device, SIZE_IN_BYTES, 0);
	uint32_t *vptr = (uint32_t *)bo_a.map<int*>();
	mapBuffer(bo_a);

    	std::memcpy(vptr, &config, sizeof(config));
   	for (int i=0;i<16384;i=i+4)
	    std::memcpy(vptr+1024+i, &fft_data_in, sizeof(fft_data_in));
    
	printf("FFT TEST :\n");
	//Configure FFT Ch0
	//Arguments: (int slot, uint64_t offset, uint64_t size, uint8_t tid)
	DataToAccel(slot,0x0,0x1,0x1);
	int status = DataToAccelDone(slot);

	//Configure FFT Ch1
	//Arguments: (int slot, uint64_t offset, uint64_t size, uint8_t tid)
	if(status)
	{
		printf("\t Configure FFT Ch0 done.\n");
		DataToAccel(slot,0x0,0x1,0x2);
		status = DataToAccelDone(slot);
	}
	
	//Configure FFT Ch2
	//Arguments: (int slot, uint64_t offset, uint64_t size, uint8_t tid)
	if(status)
	{
		printf("\t Configure FFT Ch1 done.\n");
		DataToAccel(slot,0x0,0x1,0x3);
		status = DataToAccelDone(slot);
	}
	//Configure FFT Ch3
	//Arguments: (int slot, uint64_t offset, uint64_t size, uint8_t tid)
	if(status)
	{
		printf("\t Configure FFT Ch2 done.\n");
		DataToAccel(slot,0x0,0x1,0x4);
		status = DataToAccelDone(slot);
	}
	//Config MM2S data
	//Arguments: (int slot, uint64_t offset, uint64_t size, uint8_t tid)
	if(status)
	{
		printf("\t Configure FFT Ch3 done.\n");
		DataToAccel(slot,0x1000,0x1000,0x0);
 	//Config S2MM data
	//Arguments: (int slot, uint64_t offset, uint64_t size)
		printf("\t Configure FFT data done.\n");
		DataFromAccel(slot, 0x11000, 0x1000);
		status = DataFromAccelDone(slot);
	}
  
  	sleep(2);							//Time added for completion of FFT before reading the out data
	
	if(status)
	{
		printf("\t Received Data From Accel.\n");
		isTestPassed(vptr);
		FinaliseUnmapRMs(slot);
	}
	printf("FFT TEST done.\n");
	return 0;

}
