#
# ADS GCPlus board with SA1110 cpu
#
# The ADS GCPlus has 2 banks of 16 MiB SDRAM
#
# We use the ADS GCPlus Linux boot ROM to load U-Boot into SDRAM
# at c020'0000 and then move ourself to c8f0'0000. Basically, just
# install the U-Boot binary as you would the Linux zImage and then
# reap the benfits of more convenient Linux development cycles, i.e.
# bootp;tftp;bootm, repeat, etc.,.
#

CONFIG_SYS_TEXT_BASE = 0xc8f00000
