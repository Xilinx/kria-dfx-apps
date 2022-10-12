# kria-dfx-apps
Repository consists of applications to test the accelerators and Jupyter notebooks to test accelertor orchestration.

# Prerequisite for native compilation
* Ethernet connection to the board.
* Balena Etcher to flash Micro-SD card.

## Steps
### Steps to boot and update Ubuntu on target
* Program classic-22.04-kr06 image from Ubuntu on the SD card
  1. Download "iot-limerick-kria-classic-desktop-2204-x06-20220614-78.img.xz" by clicking "Download 64-bit" button under "Ubuntu Desktop 22.04 LTS" from https://ubuntu.com/download/amd-xilinx.
  2. Program the iot-limerick-kria-classic-desktop-2204-x06-20220614-78.img.xz on SD Card using Balena Etcher. Steps for flashing the Micro-SD card can be found [here](https://www.xilinx.com/products/som/kria/kv260-vision-starter-kit/kv260-getting-started-ubuntu/setting-up-the-sd-card-image.html)


* Insert the Programmed SD Card in KV260/KR260 board, connect the ethernet cable to the board and power ON the board. On your computer, connect to the serial output of the board with baud rate set to 115200 bauds.

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
#### ***Update /etc/apt/sources.list with limerick-updates.*** - Note: Steps will be simpler for production.
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
### Steps for Native compilation of applications on target
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

* Steps to run the applications and Jupyter notebooks on target are provided [here](https://gitenterprise.xilinx.com/SOM/kria_app_dev_doc/blob/main/dfx/run_application_on_target.md)
