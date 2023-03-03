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
#define ONE_KB				1024		//Number of Bytes
#define SIXTEEN_KB			16384		//Number of Bytes

#define CONFIG_OFFSET		0		//Config Buffer Offset
#define CONFIG_OFFSET_MEM	0x0 	//Config Mem Offset in Hex
#define CONFIG_SIZE			0x1		//Size of Config Buffer in multiples of 16 bytes. (1d*16 bytes = 16 bytes)

#define FFT_DATA_IN_OFFSET_MEM	0x1000 	//Config Mem Offset in Hex
#define FFT_DATA_IN_SIZE	0x1000	//Size of Config Buffer in multiples of 16 bytes. (4096d*16 bytes = 65536 bytes)

#define FFT_RESULT 			17408	//Result of FFT is stored at offset 0x11000 (69632d = 17408*4) as address increments by 32bit or 4 bytes at a time
#define RESULT_OFFSET_MEM	0x11000	//Result Buffer Mem Offset in Hex
#define RESULT_SIZE			0x1000	//Result Buffer Mem Offset in Hex

#define TID_0				0x0		//TID 0
#define TID_1				0x1		//TID 1
#define TID_2				0x2		//TID 2
#define TID_3				0x3		//TID 3
#define TID_4				0x4		//TID 4


#define die(fmt, args ...) do { fprintf(stderr, \
		"ERROR:%s():%u " fmt ": %s\n", \
		__func__, __LINE__, ##args, errno ? strerror(errno) : ""); \
		exit(EXIT_FAILURE); \
	} while (0)

int InitializeMapRMs(int slot);
int FinaliseUnmapRMs(int slot);
void mapBuffer(xrt::bo boa);
int DataToAccel(int slot, uint64_t data, uint64_t size, uint8_t tid);
int DataFromAccel(int slot, uint64_t data, uint64_t size);
int DataToAccelDone(int slot);
int DataFromAccelDone(int slot);

//Input to config port of FFT IP
uint32_t config_fft[] = {0x0000000c,0x0000000c,0x0000000c,0x0000000c};

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

	//Write to the memory that was mapped, use devmem from the command line of Linux to verify it worked
	//Write Config Buffer to memory at CONFIG_OFFSET of allocated XRT Buffer
    std::memcpy(vptr+CONFIG_OFFSET, &config_fft, sizeof(config_fft));
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

static struct option const long_opt[] =
{
	{ "help",	no_argument, 		NULL, 'h'},
	{ "slot",	required_argument, 	NULL, 's'},
	{ "config",	required_argument, 	NULL, 'c'},
	{ "in",		required_argument, 	NULL, 'i'},
	{ "out",	required_argument, 	NULL, 'o'},
	{ NULL, 0, NULL, 0}
};

static const char help_usage[] =
	" fft (0|1) perform a quick internal test for slot 0 or 1\n"
	" fft [<options>] --slot rm_slot --config filename --out filename --in filename\n"
	" Options :\n"
	"  -h, --help\n"
	"  -s, --slot rm_slot		Set slot to rm_slot: 0 or 1. (Optional) Default slot is 0 if this flag is not provided\n"
	"  -c, --config filename	Use config file (Required)\n"
	"  -i, --in filename		Input file to the program (Required)\n"
	"  -o, --out filename		Write output to file (Required)\n"
	" Example : \n"
	"	fft -s 0 -c config.bin -i input.bin -o output.bin\n\n";

void usage(const char *msg)
{
	fprintf(stderr, "%s\n%s", msg, help_usage);
	exit(0);
}

int main(int argc, char *argv[])
{
	char *config_file = NULL;
	char *in_file  = NULL;
	char *out_file = NULL;

	struct stat statbuf, configbuf;
	
	int slot = 0, infd = -1, outfd = -1, configfd = -1;
	int rc, opt;

	if (argc == 1)
		return internal_test(0);
	else if (argc == 2 && argv[1][0] != '-')
		return internal_test(argv[1][0] - '0');

	while ((opt = getopt_long(argc, argv, "hc:o:s:i:",
				  long_opt, NULL)) != -1)
		switch (opt) {
		case 'c':
			config_file = optarg;
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

	if (!config_file)
		die("Missing config file. Use \"-h\" for usage and more information");

	if(!in_file)
		die("Missing input file. Use \"-h\" for usage and more information");

	if(!out_file)
		die("Missing output file. Use \"-h\" for usage and more information");
	
	configfd = open(config_file, O_RDONLY, 0);
	if (configfd == -1)
		die("open(%s)", config_file);
	if (fstat(configfd, &configbuf))
		die("fstat(%s)", config_file);
	size_t config_len = configbuf.st_size;
	if (config_len < 16)
		die("Config file size is less than 16 bytes");
	char *config_mm = (char *)mmap(NULL, config_len, PROT_READ, MAP_SHARED, configfd, 0);
	
	infd = open(in_file, O_RDONLY, 0);
	if (infd == -1)
		die("open(%s)", in_file);
	if (fstat(infd, &statbuf))
		die("fstat(%s)", in_file);
	size_t in_len = statbuf.st_size;
	if ((in_len < 65536) || (in_len > (SIZE_IN_BYTES - RESULT_OFFSET_MEM)))
	die("Input file size %lu is out of demo range [65536, %u] bytes", in_len,
		SIZE_IN_BYTES - RESULT_OFFSET_MEM);
	char *in_mm = (char *)mmap(NULL, in_len, PROT_READ, MAP_SHARED, infd, 0);


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
	mapBuffer(bufferObject);

	int status;
	// Write to the memory that was mapped, use devmem from
	// the command line of Linux to verify it worked

	printf("FFT application running on Slot %d:\n",slot);

	// Write the config and input data
	memcpy(vptr + CONFIG_OFFSET, config_mm, config_len);
	memcpy(vptr + ONE_KB, in_mm, in_len);

	// Configure four FFT channels
	for (int i=1; i<=4; i++)
	{
		//Arguments: (int slot, uint64_t offset, uint64_t size, uint8_t tid)
		DataToAccel(slot, CONFIG_OFFSET_MEM, config_len>>4, i);
		status = DataToAccelDone(slot);
		if (!status)
			die("DataToAccelDone(%d)", slot);
		printf("Configure FFT Ch%d done.\n", i);
	}
	
	//Config FFT Data In
 	//Config S2MM data
	//Arguments: (int slot, uint64_t offset, uint64_t size)
	DataToAccel(slot, FFT_DATA_IN_OFFSET_MEM, in_len>>4, TID_0);
	printf("Configure FFT data done.\n");

	DataFromAccel(slot, RESULT_OFFSET_MEM, in_len>>4);
	status = DataFromAccelDone(slot);
	if (!status)
		die("DataFromAccelDone(%d)", slot);
	printf("Data from Accel done.\n");
	printf("FFT operation done\n");

	const void *cvp = (const void *)(uvp + RESULT_OFFSET_MEM);
	// write only the original size
	status = write(outfd, cvp, statbuf.st_size);
	FinaliseUnmapRMs(slot);
	close(infd);
	close(configfd);
	close(outfd);
	if (!status)
		die("write(%s)", out_file);

	return 0;
}
