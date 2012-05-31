# Linux Kernel is expected to be at 8000'8000, entry 8000'8000
# (mem base + reserved)
#

#Provide at least 16MB spacing between us and the Linux Kernel image
CONFIG_SYS_TEXT_BASE = 0x81080000
