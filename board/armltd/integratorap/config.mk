#
# image should be loaded at 0x01000000
#

TEXT_BASE = 0x01000000

ifneq ($(OBJTREE),$(SRCTREE))
# We are building u-boot in a separate directory, use generated
# .lds script from OBJTREE directory.
LDSCRIPT := $(OBJTREE)/board/$(BOARDDIR)/u-boot.lds
endif
