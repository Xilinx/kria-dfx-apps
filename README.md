<h1 align="center">KRIA DYNAMIC FUNCTION EXCHANGE APPLICATION</h1>

## Introduction
This repository contains applications that demostrate accelerator functionality and Jupyter notebooks to demonstrate accelerator orchestration on multiple slots.

The repository structure is outlined below. 

* lib - Directory containing SIHA helper library that has APIs to manage the Reconfigurable Modules used by applications in src directory.
	* siha.c
	* siha.h
* notebook - Directory containing Jupyter Notebooks and corresponding input files and SIHA helper library
	* data - Directory containing input data files that are fed to the Jupyter notebooks.
	  * inputSignal.bin
	  * reload_bpf.bin
	  * reload_hpf.bin
	  * reload_lpf.bin
	* images - Directory containing input data file that is fed to Jupyter notebooks that demonstrate encryption and decryption.
		* andromeda.jpg
	* lib - Directory containing SIHA helper library used by Jupyter notebooks only.
	  * libsiha.so
	  * siha.c
	  * siha.h
	* AES-FFT.ipynb
	* AES-FIR-FFT.ipynb
	* AES128.ipynb
	* AES192.ipynb
	* AES_On_HW_vs_SW.ipynb
	* pad.py  - helper python script to pad data to align the data to byte boundary
	* siha.py - wrapper python script to use the helper functionlity provided by libsiha.so from lib directory.
* src - Directory containing example applications
	* AES128 - Directory containing AES128 application source.
	  * main.c
	* AES192 - Directory containing AES192 application source.
	  * main.c
	* FFT - Directory containing FFT application source.
	  * main.c
	* FIR - Directory containing FIR application source.
	  * main.c


## About DFX

The AMD-Xilinx Dynamic Function eXchange (DFX) is a technology that enables an incremental and partitioned hardware configuration of a programmable device. Details on DFX capabilities within the Vivado tool are available in [UG909](https://docs.xilinx.com/r/en-US/ug909-vivado-partial-reconfiguration/Introduction-to-Dynamic-Function-eXchange). In PL/FPGA-based DFX this enables a concept of reconfigurable partitions (RP) and reconfigurable modules (RM). The RP provides a pre-provisioned HW space allocation within which different RMs can be loaded dynamically. The system and solution capabilities of the DFX incremental HW configuration enable:

• Partitioned designs allowing one part of the system to be changed (reconfigured) while another part of the system remains running.

• “Slotted” architecture allowing for dynamically composable HW systems without having to recompile a monolithic configuration bitstream file.

• Decoupled life cycles for PL-based functions, facilitating a reconfigurable module (RM) to be deployed to the field later without having to update or rebuild the base platform design.


## Try DFX Application

Detailed documentation and how to try this applcation is well documented [here](https://xilinx.github.io/kria-apps-docs/dfx/build/html/docs/DFX_Landing_Page.html#getting-started-with-pre-built-dfx-examples-for-kr260-and-kv260). Please go through the [DFX documentation page](https://xilinx.github.io/kria-apps-docs/dfx/build/html/docs/DFX_Landing_Page.html#) for more details.
