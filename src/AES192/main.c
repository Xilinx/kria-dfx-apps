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
int StartAccel(int slot);
int FinaliseUnmapRMs(int slot);
void mapBuffer(xrt::bo boa);
int DataToAccel(int slot, uint64_t data, uint64_t size, uint8_t tid);
int DataFromAccel(int slot, uint64_t data, uint64_t size);
int DataToAccelDone(int slot);
int DataFromAccelDone(int slot);
//char * dfxmgr_uio_by_name(char *obuf, int slot, const char *name);

uint32_t decryptedbuff[] = {
	0x33221100, 0x77665544, 0xbbaa9988, 0xffeeddcc,
	0x33221100, 0x77665544, 0xbbaa9988, 0xffeeddcc,
	0x33221100, 0x77665544, 0xbbaa9988, 0xffeeddcc,
	0x33221100, 0x77665544, 0xbbaa9988, 0xffeeddcc,
	0x33221100, 0x77665544, 0xbbaa9988, 0xffeeddcc,
	0x33221100, 0x77665544, 0xbbaa9988, 0xffeeddcc,
	0x33221100, 0x77665544, 0xbbaa9988, 0xffeeddcc,
	0x33221100, 0x77665544, 0xbbaa9988, 0xffeeddcc,
	0x33221100, 0x77665544, 0xbbaa9988, 0xffeeddcc,
	0x33221100, 0x77665544, 0xbbaa9988, 0xffeeddcc,
	0x33221100, 0x77665544, 0xbbaa9988, 0xffeeddcc,
	0x33221100, 0x77665544, 0xbbaa9988, 0xffeeddcc,
	0x33221100, 0x77665544, 0xbbaa9988, 0xffeeddcc,
	0x33221100, 0x77665544, 0xbbaa9988, 0xffeeddcc,
	0x33221100, 0x77665544, 0xbbaa9988, 0xffeeddcc,
	0x33221100, 0x77665544, 0xbbaa9988, 0xffeeddcc
};


uint32_t encryptedbuff[] = {
	0xa47ca9dd, 0xe0df4c86, 0xa070af6e, 0x91710dec,
	0xa47ca9dd, 0xe0df4c86, 0xa070af6e, 0x91710dec,
	0xa47ca9dd, 0xe0df4c86, 0xa070af6e, 0x91710dec,
	0xa47ca9dd, 0xe0df4c86, 0xa070af6e, 0x91710dec,
	0xa47ca9dd, 0xe0df4c86, 0xa070af6e, 0x91710dec,
	0xa47ca9dd, 0xe0df4c86, 0xa070af6e, 0x91710dec,
	0xa47ca9dd, 0xe0df4c86, 0xa070af6e, 0x91710dec,
	0xa47ca9dd, 0xe0df4c86, 0xa070af6e, 0x91710dec,
	0xa47ca9dd, 0xe0df4c86, 0xa070af6e, 0x91710dec,
	0xa47ca9dd, 0xe0df4c86, 0xa070af6e, 0x91710dec,
	0xa47ca9dd, 0xe0df4c86, 0xa070af6e, 0x91710dec,
	0xa47ca9dd, 0xe0df4c86, 0xa070af6e, 0x91710dec,
	0xa47ca9dd, 0xe0df4c86, 0xa070af6e, 0x91710dec,
	0xa47ca9dd, 0xe0df4c86, 0xa070af6e, 0x91710dec,
	0xa47ca9dd, 0xe0df4c86, 0xa070af6e, 0x91710dec,
	0xa47ca9dd, 0xe0df4c86, 0xa070af6e, 0x91710dec
};

uint32_t encryptionkeybuff[] = {
	0x03020100, 0x07060504, 0x0b0a0908, 0x0f0e0d0c,
	0x13121110, 0x17161514, 0x00000001, 0x00000000
};

uint32_t decryptionkeybuff[] = {
	0x03020100, 0x07060504, 0x0b0a0908, 0x0f0e0d0c,
	0x13121110, 0x17161514, 0x00000000, 0x00000000
};

int compare(int counts, uint32_t *address, uint32_t *reference){
	int i;
	int error = 0;
	uint32_t value;
	for (i = 0; i < counts >> 2; i++){
		value = *(address + 192 + i);
		if (value != reference[i]){
			error = 1;
			break;
		}
	}
	if (error==0){
		return 1;
	}
	return 0;
}

void isTestPassed(const char* testName,uint32_t *vptr)
{
	if(!strcmp(testName, "ENCR"))
	{
		if(compare(0x100, vptr, encryptedbuff))
		{	
			printf("\t Success: AES192 ENCRYPTED DATA MATCHED WITH REFERENCE DATA !\n");
			return;
		}
		else
			printf("\t Error: AES192 ENCRYPTED DATA DOES NOT MATCH WITH REFERENCE DATA !\n");
	} else if(!strcmp(testName, "DECR"))
	{
		if(compare(0x100, vptr, decryptedbuff))
		{	
			printf("\t Success: AES192 DECRYPTED DATA MATCHED WITH REFERENCE DATA !\n");
			return;
		}
		else
			printf("\t Error: AES192 DECRYPTED DATA DOES NOT MATCH WITH REFERENCE DATA !\n");
	}
	return;
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

	// Write to the memory that was mapped, use devmem from the command line of Linux to verify it worked
    std::memcpy(vptr, &decryptionkeybuff, sizeof(decryptionkeybuff));
    std::memcpy(vptr+8, &encryptionkeybuff, sizeof(encryptionkeybuff));
    std::memcpy(vptr+64, &encryptedbuff, sizeof(encryptedbuff));
    std::memcpy(vptr+128, &decryptedbuff, sizeof(decryptedbuff));

	//Initialize AES128
	StartAccel(slot);
	printf("- AES192 DECRYPTION -\n");
	//Program key to Accelerator
	DataToAccel(slot,0x0,0x2,0x1);
	printf("\t Slot configured for DECRYPTION.\n");
	//Config DataToAccel	TID 0
	DataToAccel(slot,0x100,0x10,0x0);
	int status = DataToAccelDone(slot);
	
	if(status)
	{
		DataFromAccel(slot, 0x300, 0x10);
		status = DataFromAccelDone(slot);
		printf("\t AES192 DECRYPTION done.\n");
	}
	if(status)
	{
		isTestPassed("DECR",vptr);
	}

	StartAccel(slot);
	printf("- AES192 ENCRYPTION -\n");
	//Program key to Accelerator
	DataToAccel(slot,0x20,0x2,0x1);
	printf("\t Slot configured for ENCRYPTION.\n");
	DataToAccel(slot,0x200,0x10,0x0);
	status = DataToAccelDone(slot);
	if(status)
	{
		DataFromAccel(slot, 0x300, 0x10);
		status = DataFromAccelDone(slot);
		printf("\t AES192 ENCRYPTION done.\n");
	}
	if(status)
	{
		isTestPassed("ENCR",vptr);
		FinaliseUnmapRMs(slot);
	}

	return 0;

}
