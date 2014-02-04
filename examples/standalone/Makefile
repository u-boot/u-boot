#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

extra-y        := hello_world
extra-$(CONFIG_SMC91111)           += smc91111_eeprom
extra-$(CONFIG_SMC911X)            += smc911x_eeprom
extra-$(CONFIG_SPI_FLASH_ATMEL)    += atmel_df_pow2
extra-$(CONFIG_MPC5xxx)            += interrupt
extra-$(CONFIG_8xx)                += test_burst timer
extra-$(CONFIG_8260)               += mem_to_mem_idma2intr
extra-$(CONFIG_PPC)                += sched

#
# Some versions of make do not handle trailing white spaces properly;
# leading to build failures. The problem was found with GNU Make 3.80.
# Using 'strip' as a workaround for the problem.
#
ELF := $(strip $(extra-y))

extra-y += $(addsuffix .srec,$(extra-y)) $(addsuffix .bin,$(extra-y))
clean-files  := $(extra-) $(addsuffix .srec,$(extra-)) $(addsuffix .bin,$(extra-))


COBJS	:= $(ELF:=.o)

LIB	= $(obj)libstubs.o

LIBAOBJS-$(CONFIG_PPC) += ppc_longjmp.o ppc_setjmp.o
LIBAOBJS-$(CONFIG_8xx) += test_burst_lib.o
LIBAOBJS := $(LIBAOBJS-y)

LIBCOBJS = stubs.o

LIBOBJS	= $(addprefix $(obj),$(LIBAOBJS) $(LIBCOBJS))

SRCS	:= $(COBJS:.o=.c) $(LIBCOBJS:.o=.c) $(LIBAOBJS:.o=.S)
OBJS	:= $(addprefix $(obj),$(COBJS))
ELF	:= $(addprefix $(obj),$(ELF))

gcclibdir := $(shell dirname `$(CC) -print-libgcc-file-name`)

# For PowerPC there's no need to compile standalone applications as a
# relocatable executable.  The relocation data is not needed, and
# also causes the entry point of the standalone application to be
# inconsistent.
ifeq ($(ARCH),powerpc)
AFLAGS := $(filter-out $(RELFLAGS),$(AFLAGS))
CFLAGS := $(filter-out $(RELFLAGS),$(CFLAGS))
CPPFLAGS := $(filter-out $(RELFLAGS),$(CPPFLAGS))
endif

# We don't want gcc reordering functions if possible.  This ensures that an
# application's entry point will be the first function in the application's
# source file.
CFLAGS += $(call cc-option,-fno-toplevel-reorder)

#########################################################################
$(LIB):	$(LIBOBJS)
	$(call cmd_link_o_target, $(LIBOBJS))

$(ELF):
$(obj)%:	$(obj)%.o $(LIB)
		$(LD) $(LDFLAGS) -g -Ttext $(CONFIG_STANDALONE_LOAD_ADDR) \
			-o $@ -e $(SYM_PREFIX)$(notdir $(<:.o=)) $< $(LIB) \
			-L$(gcclibdir) -lgcc

$(obj)%.srec:	$(obj)%
		$(OBJCOPY) -O srec $< $@ 2>/dev/null

$(obj)%.bin:	$(obj)%
		$(OBJCOPY) -O binary $< $@ 2>/dev/null
