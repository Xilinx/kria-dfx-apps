# kria-dfx-apps

## Pre-requisite to build apps

- Source vitis 2022.1 tool settings

- Clone the git repository to local sandbox.
```
git clone git@gitenterprise.xilinx.com:SOM/kria-dfx-apps.git kria_dfx_apps
```

Run "make all" after cloning the repository.
```bash
cd kria_dfx_apps
make all
```
Generated Application files after compilation are copied to kria-dfx-apps directory. These need to be transferred to the target to test post loading the acclerators to RPs on the board.
