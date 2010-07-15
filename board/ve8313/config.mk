ifndef NAND_SPL
sinclude $(OBJTREE)/board/$(BOARDDIR)/config.tmp
endif

ifndef TEXT_BASE
TEXT_BASE = 0xfe000000
endif
