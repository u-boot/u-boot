ifndef NAND_SPL
sinclude $(OBJTREE)/board/$(BOARDDIR)/config.tmp
endif

ifndef CONFIG_SYS_TEXT_BASE
CONFIG_SYS_TEXT_BASE = 0x00100000
endif

ifdef CONFIG_NAND_LP
PAD_TO = 0xFFF20000
else
PAD_TO = 0xFFF04000
endif
