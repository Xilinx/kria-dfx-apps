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

#define SIZE_IN_BYTES       0x4000000	//64MB
#define ONE_KB				1024		//Number of Bytes
#define SIXTEEN_KB			16384		//Number of Bytes

#define CONFIG_OFFSET		0		//Config Buffer Offset
#define FFT_RESULT 			17408	//Result of FFT is stored at offset 0x11000 (69632d = 17408*4) as address increments by 32bit or 4 bytes at a time
#define CONFIG_OFFSET_MEM	0x0 	//Config Mem Offset in Hex
#define CONFIG_SIZE			0x1		//Size of Config Buffer in multiples of 16 bytes. (1d*16 bytes = 16 bytes)
#define TID_0				0x0		//TID 0
#define TID_1				0x1		//TID 1
#define TID_2				0x2		//TID 2
#define TID_3				0x3		//TID 3
#define TID_4				0x4		//TID 4
#define FFT_DATA_IN_OFFSET_MEM	0x1000 	//Config Mem Offset in Hex
#define RESULT_OFFSET_MEM	0x11000	//Result Buffer Mem Offset in Hex
#define FFT_DATA_IN_SIZE	0x1000	//Size of Config Buffer in multiples of 16 bytes. (4096d*16 bytes = 65536 bytes)
#define RESULT_SIZE			0x1000	//Result Buffer Mem Offset in Hex


int InitializeMapRMs(int slot);
int FinaliseUnmapRMs(int slot);
void mapBuffer(xrt::bo boa);
int DataToAccel(int slot, uint64_t data, uint64_t size, uint8_t tid);
int DataFromAccel(int slot, uint64_t data, uint64_t size);
int DataToAccelDone(int slot);
int DataFromAccelDone(int slot);

//Input to config port of FFT IP
uint32_t config[] = {0x0000000c,0x0000000c,0x0000000c,0x0000000c};

//FFT Input Data
uint32_t fft_data_in[] = {
	0xcca5a729, 0x4b276e90, 0x9a57a7e7, 0xd0bfe1c7
};

//FFT Reference output Data
uint32_t fft_data_out[] = {
	0x88009C00, 0x5C006800, 0x5800E400, 0x1400A800
};

//Placeholder buffer for actual output from FFT
uint32_t resultbuff[] = {
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000
};

//Tests if FFT test has passed by comparing the result buffer pointer by 'vptr' against the reference fir_data_out.
void isTestPassed(uint32_t *vptr)
{
	for (int i=0; i < 16; i++)					//Copying out data to resultbuff for comparison with golden data
	{
		resultbuff[i] = vptr[(i*ONE_KB)+FFT_RESULT];
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
			printf("\t Success: FFT TEST PASSED !\n");
		else
			printf("\t Error: FFT TEST FAILED !\n");
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
	//Write Config Buffer to memory at CONFIG_OFFSET of allocated XRT Buffer
    std::memcpy(vptr+CONFIG_OFFSET, &config, sizeof(config));
   	for (int i=0;i<SIXTEEN_KB;i=i+4)
	    std::memcpy(vptr+ONE_KB+i, &fft_data_in, sizeof(fft_data_in));
    
	printf("FFT TEST on Slot %d:\n",slot);
	//Configure FFT Ch0
	//Arguments: (int slot, uint64_t offset, uint64_t size, uint8_t tid)
	DataToAccel(slot,CONFIG_OFFSET_MEM,CONFIG_SIZE,TID_1);
	int status = DataToAccelDone(slot);

	//Configure FFT Ch1
	//Arguments: (int slot, uint64_t offset, uint64_t size, uint8_t tid)
	if(status)
	{
		printf("\t Configure FFT Ch0 done.\n");
		DataToAccel(slot,CONFIG_OFFSET_MEM,CONFIG_SIZE,TID_2);
		status = DataToAccelDone(slot);
	}
	
	//Configure FFT Ch2
	//Arguments: (int slot, uint64_t offset, uint64_t size, uint8_t tid)
	if(status)
	{
		printf("\t Configure FFT Ch1 done.\n");
		DataToAccel(slot,CONFIG_OFFSET_MEM,CONFIG_SIZE,TID_3);
		status = DataToAccelDone(slot);
	}
	//Configure FFT Ch3
	//Arguments: (int slot, uint64_t offset, uint64_t size, uint8_t tid)
	if(status)
	{
		printf("\t Configure FFT Ch2 done.\n");
		DataToAccel(slot,CONFIG_OFFSET_MEM,CONFIG_SIZE,TID_4);
		status = DataToAccelDone(slot);
	}
	//Config FFT Data In
	if(status)
	{
		printf("\t Configure FFT Ch3 done.\n");
		DataToAccel(slot,FFT_DATA_IN_OFFSET_MEM,FFT_DATA_IN_SIZE,TID_0);
 	//Config S2MM data
	//Arguments: (int slot, uint64_t offset, uint64_t size)
		printf("\t Configure FFT data done.\n");
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
