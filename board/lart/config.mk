#
# LART board with SA1100 cpu
#
# see http://www.lart.tudelft.nl/ for more information on LART
#

#
# LART has 4 banks of 8 MB DRAM
#
# c000'0000
# c100'0000
# c800'0000
# c900'0000
#
# Linux-Kernel is expected to be at c000'8000, entry c000'8000
#
# we load ourself to c178'0000, the upper 1 MB of second bank
#
# download areas is c800'0000
#


CONFIG_SYS_TEXT_BASE = 0xc1780000
