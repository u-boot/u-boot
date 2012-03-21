#
# Spectrum Digital DM355 EVM board
#	dm355evm board has 1 bank of 128 MB DDR RAM
#	Physical Address: 8000'0000 to 8800'0000
#
# Linux Kernel is expected to be at 8000'8000, entry 8000'8000
# (mem base + reserved)
#

#Provide at least 16MB spacing between us and the Linux Kernel image
CONFIG_SYS_TEXT_BASE = 0x81080000
