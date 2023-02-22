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

Applications

1. AES128 - an application for encrypting and decrypting data
	requirements (to be passed to the application as command line arguments)
	1. slot ( -s or --slot ) - slot_no in which the accelerator is loaded ( 0 or 1 ) 
	2. key ( -k or --key ) - key or passphrase to be used by AES128 algorithm ( 32 bytes )
	3. input file ( -i or --in ) - data that need to be encrypted or decrypted ( > 16 bytes )
	4. output file ( -o or --out ) - file name to store the output data recieved from the algorithm
	5. -d or --decrypt flag is required for decrpytion. No flag is required for encrpytion

	Usage example - sudo ./aes128 -s 0 -k AES128_dec_key.bin -i AES128_in_data.bin -o AES128_res_data.bin -d


2. AES192 - an application for encrypting and decrypting data
	requirements (to be passed to the application as command line arguments) 
	1. slot ( -s or --slot ) - slot_no in which the accelerator is loaded ( 0 or 1 ) 
	2. key ( -k or --key ) - key or passphrase to be used by AES192 algorithm ( 32 bytes )
	3. input file ( -i or --in ) - data that need to be encrypted or decrypted ( > 16 bytes )
	4. output file ( -o or --out ) - file name to store the output data recieved from the algorithm
	5. -d or --decrypt flag is required for decrpytion. No flag is required for encrpytion

	Usage example - sudo ./aes192 -s 0 -k AES192_dec_key.bin -i AES192_in_data.bin -o AES192_res_data.bin -d

 
3. FFT - an application performing 4-channel Fast Fourier Transform
	requirements (to be passed to the application as command line arguments)
	1. slot ( -s or --slot ) - slot_no in which the accelerator is loaded ( 0 or 1 ) 
	2. FFT configuration file ( -c or --config ) - data that is needed to configure the FFT accelerator ( 16 bytes )
	3. input file ( -i or --in ) - input data for FFT accelerator 
	4. output file ( -o or --out ) - file name to store the output data recieved from the FFT accelerator

	Usage example - sudo ./fft -s 0 -c FFT_config.bin -i FFT_in_data.bin -o FFT_res_data.bin

4. FIR - an application for filtering signals using FIR
	requirements (to be passed to the application as command line arguments)
	1. slot ( -s or --slot ) - slot_no in which the accelerator is loaded ( 0 or 1 ) 
	2. FIR configuration file ( -c or --config ) - data that is needed to configure the FIR accelerator ( 16 bytes )
	3. FIR reload file ( -r or --reload ) ( 160 bytes )
	4. input file ( -i or --in ) - input data for FIR accelerator
	5. output file ( -o or --out ) - file name to store the output data recieved from the FIR accelerator

	Usage example - sudo ./fir -s 1 -c FIR_config.bin -r FIR_reload.bin -i FIR_in_data.bin -o FIR_res_data.bin


Steps to test and run the applications
```
git clone -b main https://gitenterprise.xilinx.com/SOM/kria-dfx-apps.git
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

Please use -h flag with the applications for usage and more details
```
./aes128 -h
```
