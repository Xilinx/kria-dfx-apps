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

// Encrypted Buffer to be used for Decryption
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

// Key to be used for Encryption
uint32_t encryptionkeybuff[] = {
	0x0c0d0e0f, 0x08090a0b, 0x04050607, 0x00010203,
	0x00000001, 0x00000000, 0x00000000, 0x00000000
};

// Key to be used for Decryption
uint32_t decryptionkeybuff[] = {
	0x0c0d0e0f, 0x08090a0b, 0x04050607, 0x00010203,
	0x00000000, 0x00000000, 0x00000000, 0x00000000
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

int
internal_test(int slot)
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
	
	printf("AES128 TEST on Slot %d:\n",slot);
	//Initialize AES128
	StartAccel(slot);
	printf("- AES128 DECRYPTION -\n");
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
		printf("\t AES128 DECRYPTION done.\n");
	}
	if(status)
	{
		isTestPassed("DECR",vptr);
	}

	StartAccel(slot);
	printf("- AES128 ENCRYPTION -\n");
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
		printf("\t AES128 ENCRYPTION done.\n");
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
	{ "help",             no_argument, NULL, 'h'},
	{ "algorithm",  required_argument, NULL, 'a'},
	{ "decrypt",          no_argument, NULL, 'd'},
	{ "key",        required_argument, NULL, 'k'},
	{ "out",        required_argument, NULL, 'o'},
	{ "slot",       required_argument, NULL, 's'},
	{ NULL, 0, NULL, 0}
};

static const char help_usage[] =
  " AES_PROG (0|1)\n"
  "   preform a quick internal test for slot 0 or 1\n\n"
  " AES_PROG [<options>] --key passphrase --out out_file in_file\n"
  "Options are:\n"
  "  --help\n"
  "  --algorithm ALG   Use algorithm ALG. Not implelemented yet\n"
  "  --decrypt         Decrypt the file given on the command line\n"
  "  --slot rm_slot    Set slot to rm_slot: 0 or 1. Default 0\n"
  "  --out out_file    Write output to file\n"
  "  --key passphrase  Use passphrase or passphrase file\n";

void
usage(const char *msg)
{
	fprintf(stderr, "%s\n%s", msg, help_usage);
}

struct key_buf
{
	char kb_key[16];	/* AES128 : 16 * 8 = 192 */
	int  kb_enc;	/* 1 - encrypt; 0 - decrypt */
	int  kb_pad[3];	/* must be 0 ? */
};

int
main(int argc, char *argv[])
{
	struct key_buf kbuf;
	char *key_file = NULL;
	char *in_file  = NULL;
	char *out_file = NULL;
	struct stat statbuf;
	char *line;
	int slot = 0, infd = -1, outfd = -1, pwfd = -1;
	int rc, opt;

	if (argc == 1)
		return internal_test(0);
	else if (argc == 2 && argv[1][0] != '-')
		return internal_test(argv[1][0] - '0');

	memset(&kbuf, 0, sizeof (struct key_buf));
	kbuf.kb_enc = 1; 	// 1 - to encrypt. Default
	while ((opt = getopt_long(argc, argv, "ha:dk:o:s:",
				  long_opt, NULL)) != -1)
		switch (opt) {
		case 'a':
			// combine AES128, AES192, etc.
			break;
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
		case 'h':
		default:
			usage("getopt");
		}

	if (!key_file || !out_file)
		die("missing passphrase and/or output file");

	if (optind >= argc)
		die("Expected filename after options");
	in_file = argv[argc-1];

	infd = open(in_file, O_RDONLY, 0);
	if (infd == -1)
		die("open(%s)", in_file);

	if (fstat(infd, &statbuf))
		die("fstat(%s)", in_file);

	pwfd = open(key_file, O_RDONLY, 0);
	if (pwfd == -1)
		die("open(%s)", key_file);

	rc = read(pwfd, kbuf.kb_key, sizeof (kbuf.kb_key));
	if (rc < 0)
		die("read(%s)", key_file);

	close(pwfd);

	// Align to 16 byte: len = (statbuf.st_size + 0xFU) & (~0xFU);
	size_t len = statbuf.st_size;

	if ((len < 16) || (len > (SIZE_IN_BYTES - DB_OFFSET_MEM)))
		die("file size %lu is out of demo range [16, %u]", len,
			SIZE_IN_BYTES - DB_OFFSET_MEM);

	outfd = open(out_file, O_WRONLY | O_CREAT | O_TRUNC,
		     S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (outfd == -1)
		die("open(%s)", out_file);

	printf("in_file, sz, out_file = %s, %ld %s\n",
		in_file, statbuf.st_size, out_file);
	printf("slot, passphrase = %d, %s\n", slot, kbuf.kb_key);

	char *in_mm = (char *)mmap(NULL, len, PROT_READ, MAP_SHARED, infd, 0);

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
	memcpy(vptr + EKB_OFFSET, &kbuf, sizeof(kbuf));
	memcpy(vptr +  DB_OFFSET, in_mm, len);

	printf("AES192 in slot %d to %s file %s\n\tout: %s\n", slot,
		kbuf.kb_enc ? "Encrypt" : "Decrypt", in_file, out_file);
	//Initialize AES192
	StartAccel(slot);

	// key to Accelerator - 32b (16b x 2) to Offset 32
	DataToAccel(slot, EKB_OFFSET_MEM, KEYBUFF_SIZE, TID_1);
	DataToAccel(slot,  DB_OFFSET_MEM,       len>>4, TID_0);
	status = DataToAccelDone(slot);
	if (!status)
		die("DataToAccelDone(%d)", slot);

	DataFromAccel(slot, DB_OFFSET_MEM, len>>4);
	status = DataFromAccelDone(slot);
	if (!status)
		die("DataFromAccelDone(%d)", slot);

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
