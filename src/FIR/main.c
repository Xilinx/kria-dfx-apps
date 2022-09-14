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

//int dfx_cfg_load(char* packageName);
//int load_accelerator(const char *accel_name);
int InitializeMapRMs(int slot);
int FinaliseUnmapRMs(int slot);
void mapBuffer(xrt::bo boa);
int DataToAccel(int slot, uint64_t data, uint64_t size, uint8_t tid);
int DataFromAccel(int slot, uint64_t data, uint64_t size);
int DataToAccelDone(int slot);
int DataFromAccelDone(int slot);
//char * dfxmgr_uio_by_name(char *obuf, int slot, const char *name);

uint32_t config[] = {0x0000000c,0x0000000c,0x0000000c,0x0000000c};

int16_t reload[] = {142,-266,-59,32,66,70,61,44,23,1,-21,-40,-54,-59,-56,-43,-22,5,33,58,75,81,74,53,21,-17,-56,-90,-111,-114,-98,-64,-16,39,93,135,159,157,128,75,4,-74,-147,-201,-226,-215,-166,-84,20,130,230,300,326,300,219,92,-64,-227,-370,-468,-497,-444,-308,-99,159,429,670,836,887,790,528,102,-468,-1145,-1875,-2596,-3244,-3757,-4086,28568};

uint32_t fir_data_in[] = {
	0xcca5a729, 0x4b276e90, 0x9a57a7e7, 0xd0bfe1c7
};
uint32_t fir_data_out[] = {
	0x0000cf4, 0x000082e5, 0x0000eef3, 0x00008163
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
	{ 	resultbuff[i] = vptr[i+17820];
	}
	int same_flag = 1;
		for (int i=0; i< 16; i++)
		{
			if(fir_data_out[i%4] != resultbuff[i])
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
//	char uio_path[64];
//	char *get_accel_uio_by_name(int, const char *);
//	dfxmgr_uio_by_name(uio_path,0,"SIHA");
    
	for (int i=0;i<40;i++)
		vptr[i]= (uint16_t)reload[2*i] | reload[2*i+1]<<16;
    std::memcpy(vptr+128, &config, sizeof(config));
   	for (int i=0;i<16384;i=i+4)
	    std::memcpy(vptr+256+i, &fir_data_in, sizeof(fir_data_in));

	printf("FIR TEST :\n");
	//Config reload data with TID 1
	//Arguments: (int slot, uint64_t offset, uint64_t size, uint8_t tid)
	DataToAccel(slot,0x0,0xA,0x1);
	int status = DataToAccelDone(slot);

	//Config Data MM2S with TID 0 
	//Arguments: (int slot, uint64_t offset, uint64_t size, uint8_t tid)
	if(status)
	{
		printf("\t Configure Reload data done.\n");
		DataToAccel(slot,0x200,0x1,0x2);
		status = DataToAccelDone(slot);
	}

	//Config MM2S fir input data
	//Arguments: (int slot, uint64_t offset, uint64_t size, uint8_t tid)
	if(status)
	{
		printf("\t Configure Data done.\n");
		DataToAccel(slot,0x400,0x1000,0x0);

	//Config S2MM output data
		printf("\t Configure FIR Input data done.\n");
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
	return 0;
}
