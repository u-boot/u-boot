#
# Linux-Kernel is expected to be at 1000'8000,
# entry 1000'8000 (mem base + reserved)
#

sinclude $(TOPDIR)/board/$(BOARDDIR)/config.tmp

ifeq ($(VOICEBLUE_SMALL_FLASH),y)
# We load ourself to internal SRAM at 2001'2000
# Check map file when changing TEXT_BASE.
# Everything has fit into 192kB internal SRAM!
TEXT_BASE = 0x20012000
else
# Running in SDRAM...
TEXT_BASE = 0x13000000
endif
