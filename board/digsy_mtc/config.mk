#
# Author: Grzegorz Bernacki, Semihalf, gjb@semihalf.com
#

#
# digsyMTC board:
#
#	Valid values for TEXT_BASE are:
#
#	0xFFF00000   boot high (standard configuration)
#	0xFE000000   boot low
#	0x00100000   boot from RAM (for testing only)
#

sinclude $(OBJTREE)/board/$(BOARDDIR)/config.tmp

ifndef TEXT_BASE
## Standard: boot high
TEXT_BASE = 0xFFF00000
## For testing: boot from RAM
# TEXT_BASE = 0x00100000
endif

PLATFORM_CPPFLAGS += -DTEXT_BASE=$(TEXT_BASE) -I$(TOPDIR)/board
