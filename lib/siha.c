/*
 * Copyright (C) 2022, Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 */
#include "siha.h"

static volatile uint8_t* RMCOMMBOX[] = {
		NULL,
		NULL
};

int configDMSlots(int slot, volatile uint8_t* RMComm){
	RMCOMMBOX[slot] = RMComm;
	return 0;
}

int InitializeMapRMs(int slot)
{
	int status;
	m_rms = new RMs();
	status = initRMs(slot);
	if (status < 0){
		printf("initPlDevices Failed !!\n");
		return -1;
	}
	status = mapRMs(slot);
	if (status < 0){
		printf("mapPlDevices Failed !!\n");
		return -1;
	}
	if(0 == slot)
		configDMSlots(0, m_rms->RMCommBox_0);
	else if(1 == slot)
		configDMSlots(1, m_rms->RMCommBox_1);
	return 0;
}

int initRMs(int slot){
	dfxmgr_uio_by_name(SIHA_uio_path, slot, "SIHA");
	m_rms->SIHAMgr_fd = open(SIHA_uio_path, O_RDWR | O_SYNC);
	if(m_rms->SIHAMgr_fd < 0) 
	{
		printf("Failed to open UIO_SIHA_Manager device\n");
		return -1;
	}
	if(0 == slot)
	{
		dfxmgr_uio_by_name(AccelConfig_uio_path, slot, "AccelConfig");
		m_rms->AccelConfig_0_fd = open(AccelConfig_uio_path, O_RDWR | O_SYNC);
		if(m_rms->AccelConfig_0_fd < 0) 
		{
			printf("Failed to open UIO_AccelConfig_SLOT0 device\n");
			return -1;
		}
		dfxmgr_uio_by_name(rm_comm_box_uio_path, slot, "rm_comm_box");
		m_rms->RMCommBox_0_fd = open(rm_comm_box_uio_path, O_RDWR | O_SYNC);
		if(m_rms->RMCommBox_0_fd < 0)
		{
			printf("Failed to open UIO_RMCommBox_SLOT0 device\n");
			return -1;
		}
	} else if(1 == slot)
	{
		dfxmgr_uio_by_name(AccelConfig_uio_path, slot, "AccelConfig");
		m_rms->AccelConfig_1_fd = open(AccelConfig_uio_path, O_RDWR | O_SYNC);
		if(m_rms->AccelConfig_1_fd < 0) 
		{
			printf("Failed to open UIO_AccelConfig_SLOT1 device\n");
			return -1;
		}
		dfxmgr_uio_by_name(rm_comm_box_uio_path, slot, "rm_comm_box");
		m_rms->RMCommBox_1_fd = open(rm_comm_box_uio_path, O_RDWR | O_SYNC);
		if(m_rms->RMCommBox_1_fd < 0)
		{
			printf("Failed to open UIO_RMCommBox_SLOT1 device\n");
			return -1;
		}
	}
	return 0;
}

int mapRMs(int slot){
	m_rms->SIHAMgr = (uint8_t*) mmap(NULL, SIHAMGR_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, m_rms->SIHAMgr_fd, 0);
	if(m_rms->SIHAMgr == MAP_FAILED){
		printf("Mmap SIHAMgr failed !!\n");
		return -1;
	}
	if(0 == slot)
	{
		m_rms->AccelConfig_0 = (uint8_t*) mmap(NULL, ACCELCONFIG_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, m_rms->AccelConfig_0_fd, 0);
		if(m_rms->AccelConfig_0 == MAP_FAILED){
			printf("Mmap AccelConfig_0 failed !!\n");
			return -1;
		}

		m_rms->RMCommBox_0 = (uint8_t*) mmap(NULL, RMCOMMBOX_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, m_rms->RMCommBox_0_fd, 0);
		if(m_rms->RMCommBox_0 == MAP_FAILED){
			printf("Mmap RMCommBox_0 failed !!\n");
			return -1;
		}
	} else if(1 == slot)
	{
		m_rms->AccelConfig_1 = (uint8_t*) mmap(NULL, ACCELCONFIG_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, m_rms->AccelConfig_1_fd, 0);
		if(m_rms->AccelConfig_1 == MAP_FAILED){
			printf("Mmap AccelConfig_1 failed !!\n");
			return -1;
		}

		m_rms->RMCommBox_1 = (uint8_t*) mmap(NULL, RMCOMMBOX_LEN, PROT_READ | PROT_WRITE, MAP_SHARED,	m_rms->RMCommBox_1_fd, 0);
		if(m_rms->RMCommBox_1 == MAP_FAILED){
			printf("Mmap RMCommBox_1 failed !!\n");
			return -1;
		}
	}
	return 0;
}

void mapBuffer(xrt::bo bo_a){
	auto boa = bo_a;
	m_addr = boa.address();
}

int unmapRMs(int slot){
	if(0 == slot)
	{
		munmap(m_rms->AccelConfig_0, ACCELCONFIG_LEN);
		munmap(m_rms->RMCommBox_0, RMCOMMBOX_LEN);
	} else if(1 == slot)
	{
		munmap(m_rms->AccelConfig_1, ACCELCONFIG_LEN);
		munmap(m_rms->RMCommBox_1, RMCOMMBOX_LEN);
	}
	return 0;
}

int StartAccel(int slot){
	uint8_t* AccelConfig;
	if(0 == slot)
		AccelConfig = m_rms->AccelConfig_0;
	else if(1 == slot)
		AccelConfig = m_rms->AccelConfig_1;
	*(uint32_t*)(AccelConfig) = 0x81;
	return 0;
}

int StopAccel(int slot){
	uint8_t* AccelConfig;
	if(0 == slot)
		AccelConfig = m_rms->AccelConfig_0;
	else if(1 == slot)
		AccelConfig = m_rms->AccelConfig_1;
	*(uint32_t*)(AccelConfig) = 0x0;
	return 0;
}
		
int FinaliseUnmapRMs(int slot){
	unmapRMs(slot);
	closeFDs(slot);
	return 0;
}
	
int DataToAccel(int slot, uint64_t offset, uint64_t size, uint8_t tid){
	uint64_t data = m_addr + offset;
	*(uint32_t*)(RMCOMMBOX[slot] + MM2S + ADDR_LOW)  = data & 0xFFFFFFFF;
	*(uint32_t*)(RMCOMMBOX[slot] + MM2S + ADDR_HIGH) = (data >> 32) & 0xFFFFFFFF;
	*(uint32_t*)(RMCOMMBOX[slot] + MM2S + SIZE_LOW)  = size & 0xFFFFFFFF;
	*(uint32_t*)(RMCOMMBOX[slot] + MM2S + TID)       = tid & 0xFFFFFFFF;
	*(uint32_t*)(RMCOMMBOX[slot] + MM2S + GIER)      = 0x1;
	*(uint32_t*)(RMCOMMBOX[slot] + MM2S + IIER)      = 0x3;
	*(uint32_t*)(RMCOMMBOX[slot] + MM2S + APCR)      = 0x1;
	
	return 0;
}

int DataFromAccel(int slot, uint64_t offset, uint64_t size){
	uint64_t data = m_addr + offset;
	*(uint32_t*)(RMCOMMBOX[slot] + S2MM + ADDR_LOW)  = data & 0xFFFFFFFF;
	*(uint32_t*)(RMCOMMBOX[slot] + S2MM + ADDR_HIGH) = (data >> 32) & 0xFFFFFFFF;
	*(uint32_t*)(RMCOMMBOX[slot] + S2MM + SIZE_LOW)  = size & 0xFFFFFFFF;
	*(uint32_t*)(RMCOMMBOX[slot] + S2MM + GIER)      = 0x1;
	*(uint32_t*)(RMCOMMBOX[slot] + S2MM + IIER)      = 0x3;
	*(uint32_t*)(RMCOMMBOX[slot] + S2MM + APCR)      = 0x1;
	
	return 0;
}

int DataToAccelDone(int slot)
{
	u_int32_t status = *((volatile unsigned *)(RMCOMMBOX[slot] + MM2S));
	while (! ((status >> 1) & 0x1))
	{
		status = *((volatile unsigned *)(RMCOMMBOX[slot] + MM2S));
	}
	return 1;
}

int DataFromAccelDone(int slot)
{
	int status = *((volatile unsigned *)(RMCOMMBOX[slot] + S2MM));
	while (! ((status >> 1) & 0x1))
		status = *((volatile unsigned *)(RMCOMMBOX[slot] + S2MM));

	return status;
}

void closeFDs(int slot) {
	close(m_rms->SIHAMgr_fd);
	if(0 == slot)
	{
		close(m_rms->AccelConfig_0_fd);
		close(m_rms->RMCommBox_0_fd);
	} else if(1 == slot)
	{
		close(m_rms->AccelConfig_1_fd);
		close(m_rms->RMCommBox_1_fd);
	}
}
