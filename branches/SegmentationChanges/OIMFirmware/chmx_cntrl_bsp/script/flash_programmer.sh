#!/bin/sh
#
# This file was automatically generated.
#
# It can be overwritten by nios2-flash-programmer-generate or nios2-flash-programmer-gui.
#

#
# Converting Binary File: C:\chamonix\software\chmx_cntrl\mem_init\tb2_qs_onchip_memory2_1.hex to: "..\flash/tb2_qs_onchip_memory2_1_onchip_flash_0_data.flash"
#
bin2flash --input="C:/chamonix/software/chmx_cntrl/mem_init/tb2_qs_onchip_memory2_1.hex" --output="../flash/tb2_qs_onchip_memory2_1_onchip_flash_0_data.flash" --location=0x0 --verbose 

#
# Programming File: "..\flash/tb2_qs_onchip_memory2_1_onchip_flash_0_data.flash" To Device: onchip_flash_0_data
#
nios2-flash-programmer "../flash/tb2_qs_onchip_memory2_1_onchip_flash_0_data.flash" --base=0x5000000 --accept-bad-sysid --device=1 --instance=0 '--cable=USB-Blaster on localhost [USB-0]' --program --verbose 

