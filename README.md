# DA-820C system package

## base-system
Base system for DA-820C. 
It contains the necessary tools for setting up DA-820C

## firmware
DA-820C platform firmware files. 
The Linux firmare files for use on DA-820C

### Intel "Kabylake" DMC firmware
1. copy from debian pakcage firmware-misc-nonfree (20190114-1~bpo9+2) non-free under stretch-backports
2. copy `i915/kbl_dmc_ver1_01.bin` to `/lib/firmware/i915`
