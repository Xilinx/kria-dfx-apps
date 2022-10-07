# kria-dfx-apps
# Prerequisite for native compilation
- Program classic-22.04-kr06 image from Ubuntu on the SD card: https://confluence.xilinx.com/display/MKTG/Ubuntu+22.04+Images+and+Collateral
- sudo snap install xlnx-config --classic --channel=2.x
- sudo xlnx-config.sysinit
- Reboot the board
- sudo apt install xrt-dkms
- Update /etc/apt/sources.list with limerick-updates link : https://confluence.xilinx.com/display/MKTG/Xilinx+Private+PPA+Access
- Add PPA Public Key. This only needs to be done once with a new SD card image: https://confluence.xilinx.com/display/MKTG/Xilinx+Private+PPA+Access
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
- On boot, k26-starter-kits accelerator is loaded on slot 0 which can be verified by running "sudo xmutil listapps"
```cpp
ubuntu@kria:~$ sudo xmutil listapps
                     Accelerator          Accel_type                            Base           Base_type      #slots(PL+AIE)         Active_slot

                          AES128         SIHA_PL_DFX                    k26_2rp_1003              PL_DFX               (2+0)                  -1
                             FIR         SIHA_PL_DFX                    k26_2rp_1003              PL_DFX               (2+0)                  -1
                          AES192         SIHA_PL_DFX                    k26_2rp_1003              PL_DFX               (2+0)                  -1
                             FFT         SIHA_PL_DFX                    k26_2rp_1003              PL_DFX               (2+0)                  -1
                k26-starter-kits            XRT_FLAT                k26-starter-kits            XRT_FLAT               (0+0)                  0,

```
- User needs to unload the default app using "sudo xmutil unloadpp" to later load the accelerator that is intended to be used.
```cpp
ubuntu@kria:~$ sudo xmutil unloadapp
remove from slot 0 returns: 0 (Ok)
ubuntu@kria:~$ sudo xmutil listapps
                     Accelerator          Accel_type                            Base           Base_type      #slots(PL+AIE)         Active_slot

                          AES128         SIHA_PL_DFX                    k26_2rp_1003              PL_DFX               (2+0)                  -1
                             FIR         SIHA_PL_DFX                    k26_2rp_1003              PL_DFX               (2+0)                  -1
                          AES192         SIHA_PL_DFX                    k26_2rp_1003              PL_DFX               (2+0)                  -1
                             FFT         SIHA_PL_DFX                    k26_2rp_1003              PL_DFX               (2+0)                  -1
                k26-starter-kits            XRT_FLAT                k26-starter-kits            XRT_FLAT               (0+0)                  -1
```
- User can now load the intended accelerator to be tested using "sudo xmutil loadapp"
```cpp
ubuntu@kria:~$ sudo xmutil loadapp AES128
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
```cpp
ubuntu@kria:~/kria-dfx-apps/bld$ sudo xmutil listapps
                     Accelerator          Accel_type                            Base           Base_type      #slots(PL+AIE)         Active_slot

                          AES128         SIHA_PL_DFX                    k26_2rp_1003              PL_DFX               (2+0)                  0,
                             FIR         SIHA_PL_DFX                    k26_2rp_1003              PL_DFX               (2+0)                  -1
                          AES192         SIHA_PL_DFX                    k26_2rp_1003              PL_DFX               (2+0)                  -1
                             FFT         SIHA_PL_DFX                    k26_2rp_1003              PL_DFX               (2+0)                  -1
                k26-starter-kits            XRT_FLAT                k26-starter-kits            XRT_FLAT               (0+0)                  -1
```
- User can now run the application and test the accelerator functionality by running the application built in "Steps for Native compilation of applications on target" section.
```cpp
ubuntu@kria:~/kria-dfx-apps/bld$ sudo ./src/AES128/aes128
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
