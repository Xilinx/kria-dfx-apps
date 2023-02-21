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
  	* data - Directory containing data files for testing the above applications.
	  * AES128_dec_key.bin 
	  * AES128_enc_key.bin 
	  * AES128_in_data.bin 
	  * AES128_out_data.bin 
	  * AES192_dec_key.bin 
	  * AES192_enc_key.bin 
	  * AES192_in_data.bin 
	  * AES192_out_data.bin 
	  * FFT_config.bin 
	  * FFT_in_data.bin 
	  * FFT_out_data.bin 
	  * FIR_config.bin 
	  * FIR_in_data.bin 
	  * FIR_out_data.bin 
	  * FIR_reload.bin

Please use -h flag for the applications for usage and more details
```
./src/AES128/aes128 -h
```

Steps to test and run the applications
```
git clone https://gitenterprise.xilinx.com/SOM/kria-dfx-apps.git
cd kria-dfx-apps
mkdir bld; cd bld
cmake ..; make
sudo xmutil unloadapp
sudo xmutil loadapp AES128
sudo ./src/AES128/aes128 -s 0 -k ../src/data/AES128_dec_key.bin -i ../src/data/AES128_in_data.bin -o ../src/data/AES128_res_data.bin -d
sudo xmutil loadapp FIR
sudo ./src/FIR/fir -s 1 -c ../src/data/FIR_config.bin -r ../src/data/FIR_reload.bin -i ../src/data/FIR_in_data.bin -o ../src/data/FIR_res_data.bin
sudo xmutil unloadapp 1
sudo xmutil loadapp AES192
sudo ./src/AES192/aes192 -s 1 -k ../src/data/AES192_dec_key.bin -i ../src/data/AES192_in_data.bin -o ../src/data/AES192_res_data.bin -d
sudo xmutil unloadapp 
sudo xmutil loadapp FFT
sudo ./src/FFT/fft -c ../src/data/FFT_config.bin -i ../src/data/FFT_in_data.bin -o ../src/data/FFT_res_data.bin
cd ../src/data
diff AES128_out_data.bin AES128_res_data.bin
diff AES192_out_data.bin AES192_res_data.bin
diff FIR_out_data.bin FIR_res_data.bin
diff FFT_out_data.bin FFT_res_data.bin
```
