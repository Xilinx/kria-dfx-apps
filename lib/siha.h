#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <time.h>

#include "experimental/xrt_bo.h"
#include "experimental/xrt_device.h"
#include "experimental/xrt_kernel.h"

#include "dfxmgr_client.h"

#define S2MM      0x0000
#define MM2S      0x10000
#define SIHAMGR_LEN 0x4000
#define ACCELCONFIG_LEN 0x1000
#define RMCOMMBOX_LEN 0x1000000

#define UIO_SIHA_Manager "/dev/uio4"
#define UIO_AccelConfig_SLOT0 "/dev/uio5"
#define UIO_RMCommBox_SLOT0 "/dev/uio6"
#define UIO_AccelConfig_SLOT1 "/dev/uio7"
#define UIO_RMCommBox_SLOT1 "/dev/uio8"

#define APCR       0x00
#define GIER       0x04
#define IIER       0x08
#define IISR       0x0c
#define ADDR_LOW   0x10
#define ADDR_HIGH  0x14
#define SIZE_LOW   0x1c
#define SIZE_HIGH  0x20
#define TID        0x24


typedef struct{
	int SIHAMgr_fd;
	int AccelConfig_0_fd;
	int AccelConfig_1_fd;
	int RMCommBox_0_fd;
	int RMCommBox_1_fd;
	uint8_t* SIHAMgr;
	uint8_t* AccelConfig_0;
	uint8_t* AccelConfig_1;
	uint8_t* RMCommBox_0;
	uint8_t* RMCommBox_1;
}RMs;

RMs * m_rms;
char SIHA_uio_path[64];
char AccelConfig_uio_path[64];
char rm_comm_box_uio_path[64];

uint64_t m_addr;

int configDMSlots(int slot, volatile uint8_t* RMComm);
int InitializeMapRMs(int slot);
int initRMs(int slot);
int mapRMs(int slot);
void mapBuffer(xrt::bo bo_a);
int unmapRMs(int slot);
int StartAccel(int slot);
int StopAccel(int slot);
int FinaliseUnmapRMs(int slot);
int DataToAccel(int slot, uint64_t offset, uint64_t size, uint8_t tid);
int DataFromAccel(int slot, uint64_t offset, uint64_t size);
int DataToAccelDone(int slot);
int DataFromAccelDone(int slot);
void closeFDs(int slot);
	