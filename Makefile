# Copyright (C) 2022, Advanced Micro Devices, Inc. All rights reserved.
# SPDX-License-Identifier: MIT

P= /group/siv2/work/saikira/0905/cortexa72-cortexa53
LIB_DFX=  -ldfx-mgr -L $P/dfx-mgr/usr/lib -ldfx -L $P/libdfx/usr/lib
LIB= -lrt -luuid -lxrt_coreutil
INC= -I/proj/xbuilds/2022.1_released/internal_platforms/sw/zynqmp/xilinx-zynqmp/sysroots/cortexa72-cortexa53-xilinx-linux/usr/include/xrt -I/proj/xbuilds/SWIP/2022.1_0420_0327/installs/lin64/Vivado/2022.1/include  -I $P/dfx-mgr/usr/include
SYSROOT= --sysroot=/proj/xbuilds/2022.1_released/internal_platforms/sw/zynqmp/xilinx-zynqmp/sysroots/cortexa72-cortexa53-xilinx-linux

############################## Help Section ##############################
.PHONY: help

help::
	@echo 'Makefile Usage:'
	@echo ''
	@echo '  make all'
	@echo '      Command used to generate standalone RM projects.'
	@echo ''
	@echo '  make AES128'
	@echo '      Command used to generate AES128 elf '
	@echo ''
	@echo '  make AES192'
	@echo '      Command used to generate AES192 elf '
	@echo ''
	@echo '  make FFT'
	@echo '      Command used to generate FFT elf '
	@echo ''
	@echo '  make FIR'
	@echo '      Command used to generate FIR elf '
	@echo ''
	@echo '  make clean'
	@echo '      Command to remove all the generated files.'
	@echo ''

.PHONY: all AES128 AES192 FIR FFT clean 


all:	AES128 AES192 FFT FIR

AES128:	
	aarch64-linux-gnu-g++  $(SYSROOT) $(INC) -Wall -g -std=c++1y $(LIB) $(LIB_DFX) -o testAES128 src/AES128/main.c  lib/siha.c
AES192:	
	aarch64-linux-gnu-g++  $(SYSROOT) $(INC) -Wall -g -std=c++1y $(LIB) $(LIB_DFX) -o testAES192 src/AES192/main.c  lib/siha.c
FFT: 	
	aarch64-linux-gnu-g++  $(SYSROOT) $(INC) -Wall -g -std=c++1y $(LIB) $(LIB_DFX) -o testFFT src/FFT/main.c  lib/siha.c
FIR:	
	aarch64-linux-gnu-g++  $(SYSROOT) $(INC) -Wall -g -std=c++1y $(LIB) $(LIB_DFX) -o testFIR src/FIR/main.c  lib/siha.c
clean:
	rm -f test*
