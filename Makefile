P= /group/siv2/work/saikira/cortexa72-cortexa53
LIB_DFX=  -ldfx-mgr -L $P/dfx-mgr/usr/lib -ldfx -L $P/libdfx/usr/lib -lwebsockets -L $P/libwebsockets/usr/lib
LIB= -lrt -luuid -lxrt_coreutil -lSIHA -Llib
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
	@echo '  make SIHA'
	@echo '      Command used to generate libSIHA.so '
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


all:	SIHA AES128 AES192 FFT FIR

SIHA:
	aarch64-linux-gnu-g++ -o ./lib/libSIHA.so -fpic -shared ./lib/siha.c $(INC) $(SYSROOT) $(LIB_DFX) -Wall -O0 -g -std=c++1y 
AES128:	SIHA
	aarch64-linux-gnu-g++  $(SYSROOT) $(INC) -Wall -g -std=c++1y $(LIB) $(LIB_DFX) -o testAES128 src/AES128/main.c 
AES192:	SIHA
	aarch64-linux-gnu-g++  $(SYSROOT) $(INC) -Wall -g -std=c++1y $(LIB) $(LIB_DFX) -o testAES192 src/AES192/main.c 
FFT: 	SIHA
	aarch64-linux-gnu-g++  $(SYSROOT) $(INC) -Wall -g -std=c++1y $(LIB) $(LIB_DFX) -o testFFT src/FFT/main.c 
FIR:	SIHA
	aarch64-linux-gnu-g++  $(SYSROOT) $(INC) -Wall -g -std=c++1y $(LIB) $(LIB_DFX) -o testFIR src/FIR/main.c 
clean:
	rm -f ./lib/libSIHA.so
	rm -f test*
