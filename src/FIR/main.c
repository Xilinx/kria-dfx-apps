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
#define SIXTEEN_KB			16384	//Number of Bytes

#define CONFIG_OFFSET		128		//Config Buffer Offset
#define CONFIG_OFFSET_MEM	0x200 	//Config Mem Offset in Hex
#define CONFIG_SIZE			0x1		//Size of Config Buffer in multiples of 16 bytes. (1d*16 bytes = 16 bytes)

#define INPUT_OFFSET		256		//FIR Data In Offset
#define INPUT_OFFSET_MEM	0x400 	//Config Mem Offset in Hex
#define INPUT_SIZE			0x1000	//Size of Config Buffer in multiples of 16 bytes. (4096d*16 bytes = 65536 bytes)

#define RELOAD_OFFSET		0 		//Reloaf Offset
#define RELOAD_OFFSET_MEM	0x0 	//Reload Mem Offset in Hex
#define RELOAD_SIZE			0xA		//Size of Reload Buffer in multiples of 16 bytes. (10d*16 bytes = 160 bytes)

#define RESULT_OFFSET_MEM	0x11000	//Result Buffer Mem Offset in Hex
#define RESULT_SIZE			0x1000	//Result Buffer Mem Offset in Hex

#define TID_0				0x0		//TID 0
#define TID_1				0x1		//TID 1
#define TID_2				0x2		//TID 2

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
	//Write Reload Buffer to memory at offset 0 of allocated XRT Buffer
	for (int i=0;i<40;i++)
		vptr[i]= (uint16_t)reload[2*i] | reload[2*i+1]<<16;
	//Write Config Buffer to memory at CONFIG_OFFSET of allocated XRT Buffer
	std::memcpy(vptr+CONFIG_OFFSET, &config_fir, sizeof(config_fir));
	//Write FIR Input Data of 64KB from fir_data_in buffer
   	for (int i=0;i<SIXTEEN_KB;i=i+4)
	    std::memcpy(vptr+INPUT_OFFSET+i, &fir_data_in, sizeof(fir_data_in));

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
		DataToAccel(slot,INPUT_OFFSET_MEM,INPUT_SIZE,TID_0);

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

static struct option const long_opt[] =
{
	{ "help",	no_argument, 		NULL, 'h'},
	{ "slot",	required_argument, 	NULL, 's'},
	{ "config",	required_argument, 	NULL, 'c'},
	{ "reload",	required_argument, 	NULL, 'r'},
	{ "in",		required_argument, 	NULL, 'i'},
	{ "out",	required_argument, 	NULL, 'o'},
	{ NULL, 0, NULL, 0}
};

static const char help_usage[] =
	" FIR_PROG (0|1) preform a quick internal test for slot 0 or 1\n"
	" FIR_PROG [<options>] --slot rm_slot --config filename --reload filename --out filename --in filename\n"
	"Options :\n"
	"  -h, --help\n"
	"  -s, --slot rm_slot		Set slot to rm_slot: 0 or 1. Default 0\n"
	"  -c, --config filename	Use config file\n"
	"  -r, --reload filename	Use reload file\n"
	"  -i, --in filename		Input file to the program\n"
	"  -o, --out filename		Write output to file\n"
	"Example : fir -s 0 -c config.bin -r reload.bin -o output.bin -i input.bin\n\n";

void usage(const char *msg)
{
	fprintf(stderr, "%s\n%s", msg, help_usage);
	exit(0);
}

int main(int argc, char *argv[])
{
	char *config_file = NULL;
	char *reload_file = NULL;
	char *in_file  = NULL;
	char *out_file = NULL;

	struct stat statbuf;
	struct stat configbuf;
	struct stat reloadbuf;
	
	int slot = 0, infd = -1, outfd = -1, configfd = -1, reloadfd = -1;
	int rc, opt;

	if (argc == 1)
		return internal_test(0);
	else if (argc == 2 && argv[1][0] != '-')
		return internal_test(argv[1][0] - '0');

	while ((opt = getopt_long(argc, argv, "hc:r:o:s:i:",
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
		case 'r':
			reload_file = optarg;
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

	if(!reload_file)
		die("Missing reload file. Use \"-h\" for usage and more information");

	if(!in_file)
		die("Missing input file. Use \"-h\" for usage and more information");

	if(!out_file)
		die("Missing output file. Use \"-h\" for usage and more information");
	
	infd = open(in_file, O_RDONLY, 0);
	if (infd == -1)
		die("open(%s)", in_file);
	if (fstat(infd, &statbuf))
		die("fstat(%s)", in_file);
	size_t in_len = statbuf.st_size;
	if ((in_len < 16) || (in_len > (SIZE_IN_BYTES - RESULT_OFFSET_MEM)))
		die("file size %lu is out of demo range [16, %u]", in_len,
			SIZE_IN_BYTES - RESULT_OFFSET_MEM);
	char *in_mm = (char *)mmap(NULL, in_len, PROT_READ, MAP_SHARED, infd, 0);

	configfd = open(config_file, O_RDONLY, 0);
	if (configfd == -1)
		die("open(%s)", config_file);
	if (fstat(configfd, &configbuf))
		die("fstat(%s)", config_file);
	size_t config_len = configbuf.st_size;
	char *config_mm = (char *)mmap(NULL, config_len, PROT_READ, MAP_SHARED, configfd, 0);

	reloadfd = open(reload_file, O_RDONLY, 0);
	if (reloadfd == -1)
		die("open(%s)", reload_file);
	if (fstat(reloadfd, &reloadbuf))
		die("fstat(%s)", reload_file);
	size_t reload_len = reloadbuf.st_size;
	char *reload_mm = (char *)mmap(NULL, reload_len, PROT_READ, MAP_SHARED, reloadfd, 0);

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

	printf("FIR application running on Slot %d:\n",slot);

	// Write the config and the data
	memcpy(vptr + RELOAD_OFFSET, reload_mm, reload_len);
	memcpy(vptr + CONFIG_OFFSET, config_mm, config_len);
	memcpy(vptr + INPUT_OFFSET,	 in_mm, in_len);

	//Config reload data with TID 1
	DataToAccel(slot, RELOAD_OFFSET_MEM, reload_len>>4, TID_1);
	status = DataToAccelDone(slot);
	if (!status)
		die("DataToAccelDone(%d)", slot);
	printf("Configure Reload data done.\n");

	//Config data with TID 2
	DataToAccel(slot, CONFIG_OFFSET_MEM, config_len>>4, TID_2);
	status = DataToAccelDone(slot);
	if (!status)
		die("DataToAccelDone(%d)", slot);
	printf("Configure Config data done.\n");

	//Config fir input data with TID 0
	DataToAccel(slot, INPUT_OFFSET_MEM, in_len>>4, TID_0);
	printf("Configure FIR Input data done.\n");
	
	//Config fir output data
	DataFromAccel(slot, RESULT_OFFSET_MEM, in_len>>4);
	status = DataFromAccelDone(slot);
	if (!status)
		die("DataToAccelDone(%d)", slot);
	printf("Data from Accel done.\n");
	printf("FIR operation done\n");

	const void *cvp = (const void *)(uvp + RESULT_OFFSET_MEM);
	// write only the original size
	status = write(outfd, cvp, statbuf.st_size);
	FinaliseUnmapRMs(slot);
	close(infd);
	close(configfd);
	close(reloadfd);
	close(outfd);
	if (!status)
		die("write(%s)", out_file);

	return 0;
}
