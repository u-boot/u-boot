#
# (C) Copyright 2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y	+= 20001122-1.o
obj-y	+= 20010114-2.o
obj-y	+= 20010226-1.o
obj-y	+= 980619-1.o
obj-y	+= acc1.o
obj-y	+= compare-fp-1.o
obj-y	+= fpu.o
obj-y	+= mul-subnormal-single-1.o
obj-y	+= darwin-ldouble.o

CFLAGS := $(shell echo $(CFLAGS) | sed s/-msoft-float//)
CFLAGS += -mhard-float -fkeep-inline-functions

$(obj)%.o:	%.c
	$(CC)  $(ALL_CFLAGS) -o $@.fp $< -c
	$(OBJCOPY) -R .gnu.attributes $@.fp $@
	rm -f $@.fp
