#
# DNP/1110 board with SA1100 cpu
#
# http://www.dilnetpc.com
#

#
# DILNETPC has 1 banks of 32 MB DRAM
#
# c000'0000
#
# Linux-Kernel is expected to be at c000'8000, entry c000'8000
#
# we load ourself to c1f8'0000, the upper 1 MB of the first (only) bank
#

TEXT_BASE = 0xc1f80000
