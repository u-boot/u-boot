#
# LART board with SA1100 cpu
#
# see http://www.lart.tudelft.nl/ for more information on LART
#

#
# Tuxscreen has 4 banks of 4 MB DRAM each
#
# c000'0000
# c800'0000
# d000'0000
# d800'0000
#
# Linux-Kernel is expected to be at c000'8000, entry c000'8000
#
# we load ourself to d838'0000, the upper 1 MB of the last (4th) bank
#
# download areas is c800'0000
#


TEXT_BASE = 0xd8380000
