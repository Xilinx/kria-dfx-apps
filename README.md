# kria-dfx-apps
# Prerequisite for native compilation
- Program classic-22.04-kr06 image from Ubuntu on the sd-card
- sudo snap install xlnx-config --classic --channel=2.x
- sudo xlnx-config.sysinit
- Reboot the board
- sudo apt install xrt-dkms
- Update /etc/apt/sources.list with limerick-updates link
- Add PPA Public Key. This only needs to be done once with a new SD card image.
- sudo apt update
- sudo apt upgrade

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

# Steps for running compiled applications on target
1. Copy firmware to target
- USB
- SCP
  sudo scp -r saikira@172.23.81.238:/group/siv2/work/username/.../k26_2rp /lib/firmware/xilinx


2. Load accelerator and Run the Application
On boot, k26-starter-kits accelerator is loaded on slot 0 which can be verified by running "sudo xmutil listapps"
User needs to unload the default app using "sudo xmutil unloadpp" to later load the accelerator that is intended to be used.
User can now load the intended accelerator to be tested using "sudo xmutil loadapp"
Verify that the accelerator is loaded using "sudo xmutil listapps"

User can now run the application and test the accelerator funtionality by running the application built using Steps mentioned above in "Steps for Native compilation of applications on target" section





