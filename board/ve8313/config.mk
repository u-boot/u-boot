ifndef NAND_SPL
sinclude $(OBJTREE)/board/$(BOARDDIR)/config.tmp
endif

ifndef CONFIG_SYS_TEXT_BASE
CONFIG_SYS_TEXT_BASE = 0xfe000000
endif
