#
# Author: Grzegorz Bernacki, Semihalf, gjb@semihalf.com
#

#
# digsyMTC board:
#
#	Valid values for CONFIG_SYS_TEXT_BASE are:
#
#	0xFFF00000   boot high (standard configuration)
#	0xFE000000   boot low
#	0x00100000   boot from RAM (for testing only)
#

sinclude $(OBJTREE)/board/$(BOARDDIR)/config.tmp

ifndef CONFIG_SYS_TEXT_BASE
## Standard: boot high
CONFIG_SYS_TEXT_BASE = 0xFFF00000
## For testing: boot from RAM
# CONFIG_SYS_TEXT_BASE = 0x00100000
endif

PLATFORM_CPPFLAGS += -DCONFIG_SYS_TEXT_BASE=$(CONFIG_SYS_TEXT_BASE) \
	-I$(TOPDIR)/board
