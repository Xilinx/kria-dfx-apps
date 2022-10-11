# kria-dfx-apps
Repository consists of applications to test the accelerators and Jupyter notebooks to test accelertor orchestration.

# Prerequisite for native compilation
* Ethernet connection to the board.
* Balena Etcher to flash Micro-SD card.

## Steps
* Program classic-22.04-kr06 image from Ubuntu on the SD card
  1. Download "iot-limerick-kria-classic-desktop-2204-x06-20220614-78.img.xz" by clicking "Download 64-bit" button under "Ubuntu Desktop 22.04 LTS" from https://ubuntu.com/download/amd-xilinx.
  2. Program the iot-limerick-kria-classic-desktop-2204-x06-20220614-78.img.xz on SD Card using Balena Etcher. Steps for flashing the Micro-SD card can be found [here](https://www.xilinx.com/products/som/kria/kv260-vision-starter-kit/kv260-getting-started-ubuntu/setting-up-the-sd-card-image.html)


* Insert the Programmed SD Card in KV260/KR260 board, connect the ethernet cable to the board and power ON the board. Connect to the serial output of the board with baud rate set to 115200.

* Once linux boots up, use the below credentials to login and change password when prompted.
```
username: ubuntu
password: ubuntu
```

* Update the time settings.
```
sudo vi /etc/systemd/timesyncd.conf
```
Update the line 16 #NTP to the following:
```
NTP="ntp1 ntp2 ntp3 ntp.ubuntu.com"
```
Save and exit.

Run the following commands and check that system date is updated.
```
sudo systemctl stop systemd-timesyncd
sudo systemctl start systemd-timesyncd
date
```

Update the kernel version using the below steps.
```
sudo snap install xlnx-config --classic --channel=2.x
sudo xlnx-config.sysinit
sudo reboot
sudo apt install xrt-dkms
```
### ***Update /etc/apt/sources.list with limerick-updates.*** - Note: Steps will be simpler for production.
```
sudo vim /etc/apt/sources.list
- Add the below line to the sources.list file
deb https://saikira:7k1ZTN6JvBL09jtDzwKp@private-ppa.launchpadcontent.net/limerick-team/limerick-updates/ubuntu jammy main
- Add PPA Public Key by running the below command. This only needs to be done once with a new SD card image.
sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 7BBD4B7288690887C53E2372F6565ECB47853AFB
```
Run Upgrade
```
sudo apt update
sudo apt upgrade
```
# Steps for Native compilation of applications on target
```cpp
sudo apt install cmake                                             //Install cmake
sudo apt install uuid-dev libdfx-dev libdfx-mgr-dev                //Install necessary libraries
sudo git clone https://gitenterprise.xilinx.com/SOM/kria-dfx-apps  //Clone Application git
cd kria-dfx-apps
sudo mkdir bld
cd bld
sudo cmake ..
sudo cmake --build .

All four applications will be built at 
kria-dfx-apps/bld/src/AES128/aes128
kria-dfx-apps/bld/src/AES192/aes192
kria-dfx-apps/bld/src/FFT/fft
kria-dfx-apps/bld/src/FIR/fir
```

### ***Steps to install firmware on the target*** - Note: Steps will be simpler for production.
```
- Run xmutil listapps to look at the installed firmware on the target. You will see the default k26-starter-kits firmware installed.
ubuntu@kria:~/kria-dfx-apps/bld$ cd ~
ubuntu@kria:~$ sudo xmutil listapps
                k26-starter-kits            XRT_FLAT                k26-starter-kits            XRT_FLAT               (0+0)                  0,

- Clone the git repository kria-apps-firmware. This repository has pre-built firmware for DFX example design. Also, install bootgen on the target.
sudo git clone https://gitenterprise.xilinx.com/SOM/kria-apps-firmware.git
sudo apt install bootgen-xlnx

- Navigate to kria-apps-firmware directory and run the make file. This installs the firmware on the target at the location /lib/firmware/xilinx.
cd kria-apps-firmware
sudo git checkout dev-bash
sudo make -C k26-dfx/2rp_design/ install

- Verify that firmware is installed on target by running xmutil listapps command. You should see the newly installed firmware with base_type PL_DFX.
ubuntu@kria:~/kria-apps-firmware$ sudo xmutil listapps
                     Accelerator          Accel_type                            Base           Base_type      #slots(PL+AIE)         Active_slot

                          AES128         SIHA_PL_DFX                    k26_2rp_1003              PL_DFX               (2+0)                  -1
                             FIR         SIHA_PL_DFX                    k26_2rp_1003              PL_DFX               (2+0)                  -1
                          AES192         SIHA_PL_DFX                    k26_2rp_1003              PL_DFX               (2+0)                  -1
                             FFT         SIHA_PL_DFX                    k26_2rp_1003              PL_DFX               (2+0)                  -1
                k26-starter-kits            XRT_FLAT                k26-starter-kits            XRT_FLAT               (0+0)                  0,
```

# Load accelerator and Run the Application

* On boot, k26-starter-kits accelerator is loaded on slot 0. Unload the default app using "sudo xmutil unloadpp" to later load the desired DFX accelerator. 
* By default, Running Loadapp first time will load the accelerator to slot 0 and running loadapp second time will load to slot 1. 
```
ubuntu@kria:~/kria-apps-firmware$ sudo xmutil unloadapp
remove from slot 0 returns: 0 (Ok)
ubuntu@kria:~/kria-apps-firmware$ sudo xmutil listapps
                     Accelerator          Accel_type                            Base           Base_type      #slots(PL+AIE)         Active_slot

                          AES128         SIHA_PL_DFX                    k26_2rp_1003              PL_DFX               (2+0)                  -1
                             FIR         SIHA_PL_DFX                    k26_2rp_1003              PL_DFX               (2+0)                  -1
                          AES192         SIHA_PL_DFX                    k26_2rp_1003              PL_DFX               (2+0)                  -1
                             FFT         SIHA_PL_DFX                    k26_2rp_1003              PL_DFX               (2+0)                  -1
                k26-starter-kits            XRT_FLAT                k26-starter-kits            XRT_FLAT               (0+0)                  -1
```
- User can now load the intended accelerator to be tested using "sudo xmutil loadapp"
```
ubuntu@kria:~/kria-apps-firmware$ sudo xmutil loadapp AES128
[  146.337693] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /fpga-full/firmware-name
[  146.347829] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /fpga-full/resets
[  146.357800] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /__symbols__/overlay0
[  146.367687] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /__symbols__/fpga_PR0
[  146.377538] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /__symbols__/fpga_PR1
[  146.387408] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /__symbols__/overlay1
[  146.397252] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /__symbols__/afi0
[  146.406749] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /__symbols__/clocking0
[  146.416684] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /__symbols__/clocking1
[  146.426615] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /__symbols__/clocking2
[  146.436544] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /__symbols__/clocking3
[  146.446520] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /__symbols__/overlay2
[  146.456394] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /__symbols__/RP_0_AccelConfig_0
[  146.467119] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /__symbols__/RP_0_rm_comm_box_0
[  146.477915] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /__symbols__/RP_1_AccelConfig_0
[  146.488643] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /__symbols__/RP_1_rm_comm_box_0
[  146.499358] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /__symbols__/static_shell_VCU_vcu_0
[  146.510423] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /__symbols__/encoder
[  146.520178] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /__symbols__/decoder
[  146.529934] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /__symbols__/static_shell_dfx_slot_manager_siha_manager_0
[  146.936778] zocl-drm axi:zyxclmm_drm: IRQ index 0 not found
AES128: loaded to slot 0
```
- Verify that the accelerator is loaded using "sudo xmutil listapps"
```
ubuntu@kria:~/kria-dfx-apps-firmware$ cd ~/kria-dfx-apps/bld/
ubuntu@kria:~/kria-dfx-apps/bld$ sudo xmutil listapps
                     Accelerator          Accel_type                            Base           Base_type      #slots(PL+AIE)         Active_slot

                          AES128         SIHA_PL_DFX                    k26_2rp_1003              PL_DFX               (2+0)                  0,
                             FIR         SIHA_PL_DFX                    k26_2rp_1003              PL_DFX               (2+0)                  -1
                          AES192         SIHA_PL_DFX                    k26_2rp_1003              PL_DFX               (2+0)                  -1
                             FFT         SIHA_PL_DFX                    k26_2rp_1003              PL_DFX               (2+0)                  -1
                k26-starter-kits            XRT_FLAT                k26-starter-kits            XRT_FLAT               (0+0)                  -1
```
- User can now run the application and test the accelerator functionality by running the application built in "Steps for Native compilation of applications on target" section.
```
ubuntu@kria:~/kria-dfx-apps/bld$ sudo ./src/AES128/aes128 0
AES128 TEST on Slot 0:
- AES128 DECRYPTION -
         Slot configured for DECRYPTION.
         AES128 DECRYPTION done.
         Success: DECRYPTED DATA MATCHED WITH REFERENCE DATA !
- AES128 ENCRYPTION -
         Slot configured for ENCRYPTION.
         AES128 ENCRYPTION done.
         Success: ENCRYPTED DATA MATCHED WITH REFERENCE DATA !
```
To run the accelerator on Slot 1:
```
ubuntu@kria:~/kria-dfx-apps/bld$ sudo xmutil loadapp AES192
```
```
ubuntu@kria:~/kria-dfx-apps/bld$ sudo ./src/AES192/aes192 1 
AES192 TEST on Slot 1:
- AES192 DECRYPTION -
         Slot configured for DECRYPTION.
         AES192 DECRYPTION done.
         Success: DECRYPTED DATA MATCHED WITH REFERENCE DATA !
- AES192 ENCRYPTION -
         Slot configured for ENCRYPTION.
         AES192 ENCRYPTION done.
         Success: ENCRYPTED DATA MATCHED WITH REFERENCE DATA !
```

# Setup Jupyter Lab on target
- Install Jupyter
```
ubuntu@kria:~$ sudo apt install jupyter
```
- Install Jupyter Lab
```
ubuntu@kria:~$ sudo pip install --trusted-host pypi.org --trusted-host pypi.python.org --trusted-host files.pythonhosted.org jupyterlab
ubuntu@kria:~$ sudo git clone https://gitenterprise.xilinx.com/SOM/kria-dfx-apps
cd kria-dfx-apps/notebook
```
- Launch Jupyter Lab
```
ubuntu@kria:~/kria-dfx-apps/notebook$ ifconfig
ubuntu@kria:~/kria-dfx-apps/notebook$ sudo jupyter-lab --no-browser --allow-root --ip=10.140.38.140  --> Use ip addrress from the previous step
[I 2022-10-07 11:33:42.380 ServerApp] jupyterlab | extension was successfully linked.
[I 2022-10-07 11:33:42.438 ServerApp] nbclassic | extension was successfully linked.
[I 2022-10-07 11:33:42.444 ServerApp] Writing Jupyter server cookie secret to /root/.local/share/jupyter/runtime/jupyter_cookie_secret
[I 2022-10-07 11:33:44.064 ServerApp] notebook_shim | extension was successfully linked.
[I 2022-10-07 11:33:44.152 ServerApp] notebook_shim | extension was successfully loaded.
[I 2022-10-07 11:33:44.157 LabApp] JupyterLab extension loaded from /usr/local/lib/python3.10/dist-packages/jupyterlab
[I 2022-10-07 11:33:44.158 LabApp] JupyterLab application directory is /usr/local/share/jupyter/lab
[I 2022-10-07 11:33:44.176 ServerApp] jupyterlab | extension was successfully loaded.
[I 2022-10-07 11:33:44.194 ServerApp] nbclassic | extension was successfully loaded.
[I 2022-10-07 11:33:44.196 ServerApp] Serving notebooks from local directory: /home/ubuntu/kria-dfx-apps/notebook
[I 2022-10-07 11:33:44.196 ServerApp] Jupyter Server 1.19.1 is running at:
[I 2022-10-07 11:33:44.196 ServerApp] http://10.140.38.140:8888/lab?token=60fc84050fe702904b8f434bef439c38c612af989513e6e8
[I 2022-10-07 11:33:44.196 ServerApp]  or http://127.0.0.1:8888/lab?token=60fc84050fe702904b8f434bef439c38c612af989513e6e8
[I 2022-10-07 11:33:44.196 ServerApp] Use Control-C to stop this server and shut down all kernels (twice to skip confirmation).
[C 2022-10-07 11:33:44.212 ServerApp]

    To access the server, open this file in a browser:
        file:///root/.local/share/jupyter/runtime/jpserver-3769-open.html
    Or copy and paste one of these URLs:
        http://10.140.38.140:8888/lab?token=60fc84050fe702904b8f434bef439c38c612af989513e6e8
     or http://127.0.0.1:8888/lab?token=60fc84050fe702904b8f434bef439c38c612af989513e6e8
```
From the above output: Copy the http:// link that contains the ip address provided while invoking jupyter lab and run it in Browser to use the jupyter notebooks.
Eg: http://10.140.38.140:8888/lab?token=60fc84050fe702904b8f434bef439c38c612af989513e6e8

# Steps for running Jupyter Notebooks in browser

Open a jupyter launcher(Ctrl+Shift+L). Go to file -> New and click on terminal.

Notebooks are available in the left side panel and there are five example notebooks available.

1. AES128 image encryption and decryption-

   Open AES128.ipynb. 

    Pre-requisite:
    ```
    Need AES128 RM to be loaded on Slot 0
    On Linux terminal previously opened:
      "xmutil listapps" -> to check available RMs
      Unload any RM present on slot 0  "xmutil unloadapp "
      Load AES128 RM on slot 0         "xmutil loadapp AES128"
    ```      
   Go to run and click on Run All Cells.
      
2. AES192 image encryption and decryption-

   Open AES192.ipynb and run all cells.

    Pre-requisite:
    ```
    Need AES192 RM to be loaded on Slot 0
    On Linux terminal previously opened:
      "xmutil listapps" -> to check available RMs
      Unload any RMs present on slot 0  "xmutil unloadapp "
      Load AES192 RM on slot 0          "xmutil loadapp AES192"
    ```
 
3. AES-FFT

    Open AES-FFT.ipynb and run all cells.
    
    Pre-requisite:
    ```
    Need AES128 RM to be loaded on Slot 0 and FFT RM on Slot 1
    On Linux terminal previously opened:
      "xmutil listapps" -> to check available RMs
      Unload any RMs present on slot 0  "xmutil unloadapp "
      Unload any RMs present on slot 1  "xmutil unloadapp 1"
      Load AES128 RM on slot 0          "xmutil loadapp AES128"
      Load FFT RM on slot 1             "xmutil loadapp FFT"
     ```
     
4. AES-FIR-FFT
    
    Open AES-FIR-FFT.ipynb. Use shift + enter to run individual cell.

    Pre-requisite:
    ```
    Need AES128 RM to be loaded on Slot 0 until decryption is done
    On Linux terminal previously opened:
      "xmutil listapps" -> to check available RMs
      Unload any RMs present on slot 0 "xmutil unloadapp "
      Load AES128 RM on slot 0 "xmutil loadapp AES128"
   ```
   Run 1,2 and 3 cells in AES-FIR-FFT.ipynb to run AES128 application.
      ```
      Unload any RMs present on slot 0 "xmutil unloadapp "
      Load FIR RM on slot 0            "xmutil loadapp FIR"
      Unload any RMs present on slot 1 "xmutil unloadapp 1"
      Load FFT RM on slot 1            "xmutil loadapp FFT"
      ```
   Run remaining cells in AES-FIR-FFT.ipynb.
      
5. AES_On_HW_vs_SW

    Open AES_On_HW_vs_SW.ipynb and run all cells. 

    Pre-requisite:
    
    AES_On_HW_vs_SW uses 'pycryptodome' python library. Installation :
    ```
    ubuntu@kria:~$ sudo pip install pycryptodome

    Need AES128 Rm to be loaded on Slot 0
    On Linux terminal previously opened:
      "xmutil listapps" -> to check available RMs
      Unload any RMs present on slot 0  "xmutil unloadapp "
      Load AES128 RM on slot 0          "xmutil loadapp AES128"
    ```  
