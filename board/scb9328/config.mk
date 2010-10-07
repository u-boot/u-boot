#
# This config file is used for compilation of scb93328 sources
#
# You might change location of U-Boot in memory by setting right CONFIG_SYS_TEXT_BASE.
# This allows for example having one copy located at the end of ram and stored
# in flash device and later on while developing use other location to test
# the code in RAM device only.
#

CONFIG_SYS_TEXT_BASE = 0x08f00000
