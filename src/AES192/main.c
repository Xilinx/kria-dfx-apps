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
#include <getopt.h>
#include <sys/stat.h>

// XRT includes
#include "experimental/xrt_bo.h"
#include "experimental/xrt_device.h"
#include "experimental/xrt_kernel.h"

#define SIZE_IN_BYTES		0x4000000	//64MB

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

#define die(fmt, args ...) do { fprintf(stderr, \
		"ERROR:%s():%u " fmt ": %s\n", \
		__func__, __LINE__, ##args, errno ? strerror(errno) : ""); \
		exit(EXIT_FAILURE); \
	} while (0)

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

int internal_test(int slot)
{
	if (slot != 1 && slot != 0)
		die("Invalid slot: %d - must be 0 or 1", slot);

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

static struct option const long_opt[] =
{
	{ "help",		no_argument, NULL, 'h'},
	{ "decrypt",	no_argument, NULL, 'd'},
	{ "key",		required_argument, NULL, 'k'},
	{ "in",			required_argument, NULL, 'i'},
	{ "out",		required_argument, NULL, 'o'},
	{ "slot",		required_argument, NULL, 's'},
	{ NULL, 0, NULL, 0}
};

static const char help_usage[] =
	" aes192 (0|1) perform a quick internal test for slot 0 or 1\n"
	" aes192 [<options>] --key passphrase --out out_file --in in_file\n"
	" Options :\n"
	"	-h, --help\n"
	"	-d, --decrypt			Decrypt the file given on the command line. (Optional) Default operation is encryption if this flag is not provided\n"
	"	-s, --slot rm_slot		Set slot to rm_slot: 0 or 1. (Optional) Default slot is 0 if this flag is not provided\n"
	"	-i, --in in_file		Input file for the application (Required)\n"
	"	-o, --out out_file		Write output to file (Required)\n"
	"	-k, --key passphrase	Use passphrase or passphrase file (Required)\n"
	" Example : \n"
	"	sudo ./aes192 -s 0 -k encryption_key.bin -i input.bin -o output.bin (to encrypt a file)\n"
	"	sudo ./aes192 -d -s 0 -k decryption_key.bin -i input.bin -o output.bin (to decrypt a file)\n\n";

void usage(const char *msg)
{
	fprintf(stderr, "%s\n%s", msg, help_usage);
	exit(0);
}

struct key_buf
{
	char kb_key[24];	/* AES192 : 24 * 8 = 192 */
	int  kb_enc;	/* 1 - encrypt; 0 - decrypt */
};

int main(int argc, char *argv[])
{
	struct key_buf kbuf;
	char *key_file = NULL;
	char *in_file  = NULL;
	char *out_file = NULL;
	struct stat statbuf, statkey;
	char *line;
	int slot = 0, infd = -1, outfd = -1, pwfd = -1;
	int rc, opt;

	if (argc == 1)
		return internal_test(0);
	else if (argc == 2 && argv[1][0] != '-')
		return internal_test(argv[1][0] - '0');

	memset(&kbuf, 0, sizeof (struct key_buf));
	kbuf.kb_enc = 1; 	// 1 - to encrypt. Default
	while ((opt = getopt_long(argc, argv, "hdk:o:s:i:",
				  long_opt, NULL)) != -1)
		switch (opt) {
		case 'd':
			// 0 - to Decrypt
			kbuf.kb_enc = 0;
			break;
		case 'k':
			key_file = optarg;
			break;
		case 'o':
			out_file = optarg;
			break;
		case 's':
			slot = atoi(optarg);
			break;
		case 'i':
			in_file = optarg;
			break;
		case 'h':
		default:
			usage(" Usage:");
		}

	if (!key_file)
		die("Missing key_file or passphrase. Use \"-h\" for usage and more information");

	if(!in_file)
		die("Missing input file. Use \"-h\" for usage and more information");

	if(!out_file)
		die("Missing output file. Use \"-h\" for usage and more information");

	pwfd = open(key_file, O_RDONLY, 0);
	if (pwfd == -1)
		die("open(%s)", key_file);
	if (fstat(pwfd, &statkey))
		die("open(%s)", key_file);
	size_t key_len = statkey.st_size;
	if (key_len < 24)
		die("Passphrase file size is less than 192 bits");
	rc = read(pwfd, kbuf.kb_key, sizeof (kbuf.kb_key));
	if (rc < 0)
		die("read(%s)", key_file);
	close(pwfd);

	infd = open(in_file, O_RDONLY, 0);
	if (infd == -1)
		die("open(%s)", in_file);
	if (fstat(infd, &statbuf))
		die("fstat(%s)", in_file);
	size_t len = statbuf.st_size;
	// Align to 16 byte: len = (statbuf.st_size + 0xFU) & (~0xFU);
	if ((len < 16) || (len > (SIZE_IN_BYTES - DB_OFFSET_MEM)))
		die("Input file size %lu is out of demo range [16, %u]", len,
			SIZE_IN_BYTES - DB_OFFSET_MEM);
	char *in_mm = (char *)mmap(NULL, len, PROT_READ, MAP_SHARED, infd, 0);

	outfd = open(out_file, O_WRONLY | O_CREAT | O_TRUNC,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (outfd == -1)
		die("open(%s)", out_file);

	//Initialize and memory map RMs
	if(InitializeMapRMs(slot) == -1)
		die("Use --slot option in this test; see xmutil listapps");

	//Allocate XRT buffer to be used for input and output of application
	auto device = xrt::device(0);
	auto bufferObject = xrt::bo(device, SIZE_IN_BYTES, 0);
	uint32_t *vptr = (uint32_t *)bufferObject.map<int*>();
	uint64_t uvp = (uint64_t)vptr;
	int status;

	mapBuffer(bufferObject);
	// Write to the memory that was mapped, use devmem from
	// the command line of Linux to verify it worked

	// Write the Key (passphrase) and the data
	memcpy(vptr + DKB_OFFSET, &kbuf, sizeof(kbuf));
	memcpy(vptr +  DB_OFFSET, in_mm, len);

	printf("AES192 application running on Slot %d:\n",slot);
	//Initialize AES192
	StartAccel(slot);

	// key to Accelerator - 32b (16b x 2) to Offset 32
	DataToAccel(slot, DKB_OFFSET_MEM , KEYBUFF_SIZE, TID_1);
	DataToAccel(slot,  DB_OFFSET_MEM,       len>>4, TID_0);
	status = DataToAccelDone(slot);
	if (!status)
		die("DataToAccelDone(%d)", slot);

	DataFromAccel(slot, DB_OFFSET_MEM, len>>4);
	status = DataFromAccelDone(slot);
	if (!status)
		die("DataFromAccelDone(%d)", slot);

	if (kbuf.kb_enc)
		printf("AES192 encryption done\n");
	else 
		printf("AES192 decryption done\n");

	const void *cvp = (const void *)(uvp + DB_OFFSET_MEM);
	// write only the original size
	status = write(outfd, cvp, statbuf.st_size);
	FinaliseUnmapRMs(slot);
	close(infd);
	close(outfd);
	if (!status)
		die("write(%s)", out_file);

	return 0;
}
