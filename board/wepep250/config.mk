#
# This is config used for compilation of WEP EP250 sources
#
# You might change location of U-Boot in memory by setting right TEXT_BASE.
# This allows for example having one copy located at the end of ram and stored
# in flash device and later on while developing use other location to test
# the code in RAM device only.
#

TEXT_BASE = 0xa1fe0000
#TEXT_BASE = 0xa1001000
