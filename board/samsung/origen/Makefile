#
# Copyright (C) 2011 Samsung Electronics
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifndef CONFIG_SPL_BUILD
obj-y	+= origen.o
endif

ifdef CONFIG_SPL_BUILD
all: $(OBJTREE)/tools/mk$(BOARD)spl
endif

# Fix ME after we implement hostprogs-y.
ifdef CONFIG_SPL_BUILD
$(OBJTREE)/tools/mk$(BOARD)spl:	tools/mkv310_image.c
	$(HOSTCC) tools/mkv310_image.c -o $(OBJTREE)/tools/mk$(BOARD)spl
endif
