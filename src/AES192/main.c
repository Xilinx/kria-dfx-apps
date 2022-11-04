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

#define ENCR_DECR_RESULT 	192		//Result of encryption/decryption is stored at offset 0x300 (768d = 192*4) as address increments by 32bit or 4 bytes at a time
#define COMPARE_SIZE		0x100 	//Compare 0x100 (256d) bytes between result and golden data. 
#define DKB_OFFSET			0 		//Decryption Key Offset at 0
#define EKB_OFFSET			8 		//Encryption Key Offset at 32
#define EB_OFFSET			64 		//Encrypted Buffer Offset at 256
#define DB_OFFSET			128		//Decryped Buffer Offset at 512
#define DKB_OFFSET_MEM		0x0 	//Decryption Key Mem Offset in Hex
#define EKB_OFFSET_MEM		0x20	//Encryption Key Mem Offset in Hex
#define EB_OFFSET_MEM		0x100 	//Encrypted Buffer Mem Offset in Hex
#define DB_OFFSET_MEM		0x200	//Decryped Buffer Mem Offset in Hex
#define RESULT_OFFSET_MEM	0x300	//Result Buffer Mem Offset in Hex
#define KEYBUFF_SIZE		0x2		//Size of Encryption/Decription Key Buffer in multiples of 16 bytes. (2d*16 bytes = 32 bytes)
#define BUFF_SIZE			0x10	//Size of Encryption/Decription Key Buffer in multiples of 16 bytes. (16d*16 bytes = 256 bytes)
#define TID_0				0x0		//TID 0
#define TID_1				0x1		//TID 1


int InitializeMapRMs(int slot);
int StartAccel(int slot);
int FinaliseUnmapRMs(int slot);
void mapBuffer(xrt::bo boa);
int DataToAccel(int slot, uint64_t data, uint64_t size, uint8_t tid);
int DataFromAccel(int slot, uint64_t data, uint64_t size);
int DataToAccelDone(int slot);
int DataFromAccelDone(int slot);

// Decrypted Buffer to be used for Encryption
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

// Encrypted Buffer to be used for Decryption
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

// Key to be used for Encryption
uint32_t encryptionkeybuff[] = {
	0x03020100, 0x07060504, 0x0b0a0908, 0x0f0e0d0c,
	0x13121110, 0x17161514, 0x00000001, 0x00000000
};

// Key to be used for Decryption
uint32_t decryptionkeybuff[] = {
	0x03020100, 0x07060504, 0x0b0a0908, 0x0f0e0d0c,
	0x13121110, 0x17161514, 0x00000000, 0x00000000
};

//compare 'counts' number of bytes starting at locations pointed by 'address' and 'reference'
int compare(int counts, uint32_t *address, uint32_t *reference){
	int i;
	int error = 0;
	uint32_t value;
	for (i = 0; i < counts >> 2; i++){
		value = *(address + ENCR_DECR_RESULT + i);
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

//Tests if 'testName' i.e., encryption/decryption has passed by comparing the result buffer pointer by 'vptr' against the encryptedbuff/decryptedbuff respectively.
void isTestPassed(const char* testName,uint32_t *vptr)
{
	if(!strcmp(testName, "ENCR"))
	{
		if(compare(COMPARE_SIZE, vptr, encryptedbuff))
		{	
			printf("\t Success: ENCRYPTED DATA MATCHED WITH REFERENCE DATA !\n");
			return;
		}
		else
			printf("\t Error: ENCRYPTED DATA DOES NOT MATCH WITH REFERENCE DATA !\n");
	} else if(!strcmp(testName, "DECR"))
	{
		if(compare(COMPARE_SIZE, vptr, decryptedbuff))
		{	
			printf("\t Success: DECRYPTED DATA MATCHED WITH REFERENCE DATA !\n");
			return;
		}
		else
			printf("\t Error: DECRYPTED DATA DOES NOT MATCH WITH REFERENCE DATA !\n");
	}
	return;
}

		
int main(int argc, char *argv[])
{
	//Default slot Set to 0 unless plassed as an argument
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

	// Write to the memory that was mapped, use devmem from the command line of Linux to verify it worked
	// Write Decryption Key of Size 32 bytes (4bytes x 8 )
	std::memcpy(vptr+DKB_OFFSET, &decryptionkeybuff, sizeof(decryptionkeybuff));
	// Write Encryption Key of Size 32 bytes (4bytes x 8 )
    std::memcpy(vptr+EKB_OFFSET, &encryptionkeybuff, sizeof(encryptionkeybuff));
	// Write Encrypted Buffer of Size 256 bytes (4bytes x 64 )
    std::memcpy(vptr+EB_OFFSET, &encryptedbuff, sizeof(encryptedbuff));
	// Write Decrypted Buffer of Size 256 bytes (4bytes x 64 )
    std::memcpy(vptr+DB_OFFSET, &decryptedbuff, sizeof(decryptedbuff));
	
	printf("AES192 TEST on Slot %d:\n",slot);
	//Initialize AES192
	StartAccel(slot);
	printf("- AES192 DECRYPTION -\n");
	//Program key to Accelerator - Size 32 bytes (16bytes x 2 ) to Offset 0 (0x0)
	DataToAccel(slot,DKB_OFFSET_MEM,KEYBUFF_SIZE,TID_1);
	printf("\t Slot configured for DECRYPTION.\n");
	//DataToAccel	TID 0 - Size 256 bytes (16bytes x 16 ) to Offset 256 (0x100)
	DataToAccel(slot,EB_OFFSET_MEM,BUFF_SIZE,TID_0);
	int status = DataToAccelDone(slot);
	
	if(status)
	{
		//DataFromAccel	TID 0 - Size 256 bytes (16bytes x 16 ) to Offset 768 (0x300)
		DataFromAccel(slot, RESULT_OFFSET_MEM, BUFF_SIZE);
		status = DataFromAccelDone(slot);
		printf("\t AES192 DECRYPTION done.\n");
	}
	if(status)
	{
		isTestPassed("DECR",vptr);
	}

	StartAccel(slot);
	printf("- AES192 ENCRYPTION -\n");
	//Program key to Accelerator - Size 32 bytes (16bytes x 2 ) to Offset 32 (0x20)
	DataToAccel(slot,EKB_OFFSET_MEM,KEYBUFF_SIZE,TID_1);
	printf("\t Slot configured for ENCRYPTION.\n");
	//DataToAccel	TID 0 - Size 256 bytes (16bytes x 16 ) to Offset 512 (0x200)
	DataToAccel(slot,DB_OFFSET_MEM,BUFF_SIZE,TID_0);
	status = DataToAccelDone(slot);
	if(status)
	{
		//DataFromAccel	TID 0 - Size 256 bytes (16bytes x 16 ) to Offset 768 (0x300)
		DataFromAccel(slot, RESULT_OFFSET_MEM, BUFF_SIZE);
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
