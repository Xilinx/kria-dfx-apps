# kria-dfx-apps
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
