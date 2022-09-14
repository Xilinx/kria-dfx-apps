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
int StartAccel(int slot);
int FinaliseUnmapRMs(int slot);
void mapBuffer(xrt::bo boa);
int DataToAccel(int slot, uint64_t data, uint64_t size, uint8_t tid);
int DataFromAccel(int slot, uint64_t data, uint64_t size);
int DataToAccelDone(int slot);
int DataFromAccelDone(int slot);

uint32_t decryptedbuff[] = {
	0xccddeeff, 0x8899aabb, 0x44556677, 0x00112233,
	0xccddeeff, 0x8899aabb, 0x44556677, 0x00112233,
	0xccddeeff, 0x8899aabb, 0x44556677, 0x00112233,
	0xccddeeff, 0x8899aabb, 0x44556677, 0x00112233,
	0xccddeeff, 0x8899aabb, 0x44556677, 0x00112233,
	0xccddeeff, 0x8899aabb, 0x44556677, 0x00112233,
	0xccddeeff, 0x8899aabb, 0x44556677, 0x00112233,
	0xccddeeff, 0x8899aabb, 0x44556677, 0x00112233,
	0xccddeeff, 0x8899aabb, 0x44556677, 0x00112233,
	0xccddeeff, 0x8899aabb, 0x44556677, 0x00112233,
	0xccddeeff, 0x8899aabb, 0x44556677, 0x00112233,
	0xccddeeff, 0x8899aabb, 0x44556677, 0x00112233,
	0xccddeeff, 0x8899aabb, 0x44556677, 0x00112233,
	0xccddeeff, 0x8899aabb, 0x44556677, 0x00112233,
	0xccddeeff, 0x8899aabb, 0x44556677, 0x00112233,
	0xccddeeff, 0x8899aabb, 0x44556677, 0x00112233
};

uint32_t encryptedbuff[] = {
	0xcca5a729, 0x4b276e90, 0x9a57a7e7, 0xd0bfe1c7,
	0xcca5a729, 0x4b276e90, 0x9a57a7e7, 0xd0bfe1c7,
	0xcca5a729, 0x4b276e90, 0x9a57a7e7, 0xd0bfe1c7,
	0xcca5a729, 0x4b276e90, 0x9a57a7e7, 0xd0bfe1c7,
	0xcca5a729, 0x4b276e90, 0x9a57a7e7, 0xd0bfe1c7,
	0xcca5a729, 0x4b276e90, 0x9a57a7e7, 0xd0bfe1c7,
	0xcca5a729, 0x4b276e90, 0x9a57a7e7, 0xd0bfe1c7,
	0xcca5a729, 0x4b276e90, 0x9a57a7e7, 0xd0bfe1c7,
	0xcca5a729, 0x4b276e90, 0x9a57a7e7, 0xd0bfe1c7,
	0xcca5a729, 0x4b276e90, 0x9a57a7e7, 0xd0bfe1c7,
	0xcca5a729, 0x4b276e90, 0x9a57a7e7, 0xd0bfe1c7,
	0xcca5a729, 0x4b276e90, 0x9a57a7e7, 0xd0bfe1c7,
	0xcca5a729, 0x4b276e90, 0x9a57a7e7, 0xd0bfe1c7,
	0xcca5a729, 0x4b276e90, 0x9a57a7e7, 0xd0bfe1c7,
	0xcca5a729, 0x4b276e90, 0x9a57a7e7, 0xd0bfe1c7,
	0xcca5a729, 0x4b276e90, 0x9a57a7e7, 0xd0bfe1c7
};

uint32_t encryptionkeybuff[] = {
	0x0c0d0e0f, 0x08090a0b, 0x04050607, 0x00010203,
	0x00000001, 0x00000000, 0x00000000, 0x00000000
};

uint32_t decryptionkeybuff[] = {
	0x0c0d0e0f, 0x08090a0b, 0x04050607, 0x00010203,
	0x00000000, 0x00000000, 0x00000000, 0x00000000
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
			printf("\t Success: AES128 ENCRYPTED DATA MATCHED WITH REFERENCE DATA !\n");
			return;
		}
		else
			printf("\t Error: AES128 ENCRYPTED DATA DOES NOT MATCH WITH REFERENCE DATA !\n");
	} else if(!strcmp(testName, "DECR"))
	{
		if(compare(0x100, vptr, decryptedbuff))
		{	
			printf("\t Success: AES128 DECRYPTED DATA MATCHED WITH REFERENCE DATA !\n");
			return;
		}
		else
			printf("\t Error: AES128 DECRYPTED DATA DOES NOT MATCH WITH REFERENCE DATA !\n");
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
	auto bufferObject = xrt::bo(device, SIZE_IN_BYTES, 0);
	uint32_t *vptr = (uint32_t *)bufferObject.map<int*>();
	mapBuffer(bufferObject);

	// Write to the memory that was mapped, use devmem from the command line of Linux to verify it worked
    	std::memcpy(vptr, &decryptionkeybuff, sizeof(decryptionkeybuff));
    	std::memcpy(vptr+8, &encryptionkeybuff, sizeof(encryptionkeybuff));
    	std::memcpy(vptr+64, &encryptedbuff, sizeof(encryptedbuff));
    	std::memcpy(vptr+128, &decryptedbuff, sizeof(decryptedbuff));

	//Initialize AES128
	StartAccel(slot);
	printf("- AES128 DECRYPTION -\n");
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
		printf("\t AES128 DECRYPTION done.\n");
	}
	if(status)
	{
		isTestPassed("DECR",vptr);
	}

	StartAccel(slot);
	printf("- AES128 ENCRYPTION -\n");
	//Program key to Accelerator
	DataToAccel(slot,0x20,0x2,0x1);
	printf("\t Slot configured for ENCRYPTION.\n");
	DataToAccel(slot,0x200,0x10,0x0);
	status = DataToAccelDone(slot);
	if(status)
	{
		DataFromAccel(slot, 0x300, 0x10);
		status = DataFromAccelDone(slot);
		printf("\t AES128 ENCRYPTION done.\n");
	}
	if(status)
	{
		isTestPassed("ENCR",vptr);
		FinaliseUnmapRMs(slot);
	}

	return 0;

}
