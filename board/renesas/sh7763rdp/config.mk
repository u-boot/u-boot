#
# board/sh7763rdp/config.mk
#
# TEXT_BASE refers to image _after_ relocation.
#
# NOTE: Must match value used in u-boot.lds (in this directory).
#

TEXT_BASE = 0x8FFC0000

# PLATFORM_CPPFLAGS += -DCONFIG_MULTIBOOT
