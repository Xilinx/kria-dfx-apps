# kria-dfx-apps
# Prerequisite for native compilation
- Program classic-22.04-kr06 image from Ubuntu on the sd-card
- sudo snap install xlnx-config --classic --channel=2.x
- sudo xlnx-config.sysinit
- <Reboot>
- sudo apt install xrt-dkms
- Updated /etc/apt/sources.list with limerick-updates link
- Add PPA Public Key. This only needs to be done once with a new SD card image.
- apt update
- apt upgrade

# Steps for Native compilation of apps on target
```cpp
apt install cmake                                             //Install cmake
apt install uuid-dev libdfx-dev libdfx-mgr-dev                //Install necessary libraries
git clone https://gitenterprise.xilinx.com/SOM/kria-dfx-apps  //Clone Application git
cd kria-dfx-apps
mkdir bld
cd bld
cmake ..
cmake --build .
```
All four applications will be built at 
kria-dfx-apps/bld/src/AES128/aes128
kria-dfx-apps/bld/src/AES192/aes192
kria-dfx-apps/bld/src/FFT/fft
kria-dfx-apps/bld/src/FIR/fir
