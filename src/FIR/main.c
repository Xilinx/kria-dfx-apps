/*
 * Copyright (C) 2022, Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 */

#include <string.h>
#include <cstring>
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

#define SIZE_IN_BYTES       0x4000000	//64MB
#define SIXTEEN_KB			16384		//Number of Bytes

#define CONFIG_OFFSET		128		//Config Buffer Offset
#define FIR_DATA_IN_OFFSET	256		//FIR Data In Offset
#define RELOAD_OFFSET_MEM	0x0 	//Reload Mem Offset in Hex
#define CONFIG_OFFSET_MEM	0x200 	//Config Mem Offset in Hex
#define FIR_DATA_IN_OFFSET_MEM	0x400 	//Config Mem Offset in Hex
#define RESULT_OFFSET_MEM	0x11000	//Result Buffer Mem Offset in Hex
#define RELOAD_SIZE			0xA		//Size of Reload Buffer in multiples of 16 bytes. (10d*16 bytes = 160 bytes)
#define CONFIG_SIZE			0x1		//Size of Config Buffer in multiples of 16 bytes. (1d*16 bytes = 16 bytes)
#define FIR_DATA_IN_SIZE	0x1000	//Size of Config Buffer in multiples of 16 bytes. (4096d*16 bytes = 65536 bytes)
#define RESULT_SIZE			0x1000	//Result Buffer Mem Offset in Hex
#define TID_0				0x0		//TID 0
#define TID_1				0x1		//TID 1
#define TID_2				0x2		//TID 2

int InitializeMapRMs(int slot);
int FinaliseUnmapRMs(int slot);
void mapBuffer(xrt::bo boa);
int DataToAccel(int slot, uint64_t data, uint64_t size, uint8_t tid);
int DataFromAccel(int slot, uint64_t data, uint64_t size);
int DataToAccelDone(int slot);
int DataFromAccelDone(int slot);


//Input to config port of FIR IP	
uint32_t config_fir[] = {0x0000000c,0x0000000c,0x0000000c,0x0000000c};

//Filter co-efficients - Input to reload port of FIR IP
int16_t reload[] = {142,-266,-59,32,66,70,61,44,23,1,-21,-40,-54,-59,-56,-43,-22,5,33,58,75,81,74,53,21,-17,-56,-90,-111,-114,-98,-64,-16,39,93,135,159,157,128,75,4,-74,-147,-201,-226,-215,-166,-84,20,130,230,300,326,300,219,92,-64,-227,-370,-468,-497,-444,-308,-99,159,429,670,836,887,790,528,102,-468,-1145,-1875,-2596,-3244,-3757,-4086,28568};

//FIR Input Data
uint32_t fir_data_in[] = {
	0xcca5a729, 0x4b276e90, 0x9a57a7e7, 0xd0bfe1c7
};

//FIR Reference output Data
uint32_t fir_data_out[] = {
	0x0000cf4, 0x000082e5, 0x0000eef3, 0x00008163
};

//Placeholder buffer for actual output from FIR
uint32_t resultbuff[] = {
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000
};

//Tests if FIR test has passed by comparing the result buffer pointer by 'vptr' against the reference fir_data_out.
void isTestPassed(uint32_t *vptr)
{
	for (int i=0; i < 16; i++)					//Copying out data to resultbuff for comparison with golden data
	{ 	resultbuff[i] = vptr[i+17820];
	}
	int same_flag = 1;
	for (int i=0; i< 16; i++)					//Comparing result buffer with refernece fir_data_out
	{
		if(fir_data_out[i%4] != resultbuff[i])
		{
			same_flag=0;
			break;
		}
	}
	if(same_flag)
		printf("\t Success: FIR TEST PASSED !\n");
	else
		printf("\t Error: FIR TEST FAILED !\n");
}

int main(int argc, char *argv[])
{
	int slot =0;
	if(argc>1)
	{
		//Updating slot number provided as command line argument
		slot = atoi (argv[1]);
		if (slot != 1 && slot != 0)
		{
			printf("- Invalid slot number provided %s. Valid values : 0 or 1\n",argv[1]);
			return 0;
		}
	}
	
	//Initialize and memory map RMs
	if(InitializeMapRMs(slot) == -1)
	{
		printf("- Check the slot number where the accelerator is loaded and run the test on the specific slot.\n");
		return 0;
	}
	
	//Allocate XRT buffer to be used for input and output of application
	auto device = xrt::device(0);
	auto bufferObject = xrt::bo(device, SIZE_IN_BYTES, 0);
	uint32_t *vptr = (uint32_t *)bufferObject.map<int*>();
	mapBuffer(bufferObject);

	//Write to the memory that was mapped, use devmem from the command line of Linux to verify it worked
	//Write Reload Buffer to memory at offset 0 of allocated XRT Buffer
	for (int i=0;i<40;i++)
		vptr[i]= (uint16_t)reload[2*i] | reload[2*i+1]<<16;
	//Write Config Buffer to memory at CONFIG_OFFSET of allocated XRT Buffer
	std::memcpy(vptr+CONFIG_OFFSET, &config_fir, sizeof(config_fir));
	//Write FIR Input Data of 64KB from fir_data_in buffer
   	for (int i=0;i<SIXTEEN_KB;i=i+4)
	    std::memcpy(vptr+FIR_DATA_IN_OFFSET+i, &fir_data_in, sizeof(fir_data_in));

	printf("FIR TEST on Slot %d:\n",slot);
	//Config reload data with TID 1
	DataToAccel(slot,RELOAD_OFFSET_MEM,RELOAD_SIZE,TID_1);
	int status = DataToAccelDone(slot);

	//Config Data with TID 0 
	if(status)
	{
		printf("\t Configure Reload data done.\n");
		DataToAccel(slot,CONFIG_OFFSET_MEM,CONFIG_SIZE,TID_2);
		status = DataToAccelDone(slot);
	}

	//Config fir input data
	if(status)
	{
		printf("\t Configure Data done.\n");
		DataToAccel(slot,FIR_DATA_IN_OFFSET_MEM,FIR_DATA_IN_SIZE,TID_0);

		printf("\t Configure FIR Input data done.\n");
		//Config fir output data
		DataFromAccel(slot, RESULT_OFFSET_MEM, RESULT_SIZE);
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
