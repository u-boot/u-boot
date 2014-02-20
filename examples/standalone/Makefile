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

LIB	= $(obj)/libstubs.o

LIBAOBJS-$(CONFIG_PPC) += ppc_longjmp.o ppc_setjmp.o
LIBAOBJS-$(CONFIG_8xx) += test_burst_lib.o
LIBAOBJS := $(LIBAOBJS-y)

LIBCOBJS = stubs.o

.SECONDARY: $(call objectify,$(COBJS))
targets += $(patsubst $(obj)/%,%,$(LIB)) $(COBJS) $(LIBAOBJS) $(LIBCOBJS)

LIBOBJS	= $(addprefix $(obj)/,$(LIBAOBJS) $(LIBCOBJS))

ELF	:= $(addprefix $(obj)/,$(ELF))

gcclibdir := $(shell dirname `$(CC) -print-libgcc-file-name`)

# For PowerPC there's no need to compile standalone applications as a
# relocatable executable.  The relocation data is not needed, and
# also causes the entry point of the standalone application to be
# inconsistent.
ifeq ($(ARCH),powerpc)
# FIX ME
CPPFLAGS := $(filter-out $(RELFLAGS), $(CPPFLAGS))
endif

# We don't want gcc reordering functions if possible.  This ensures that an
# application's entry point will be the first function in the application's
# source file.
ccflags-y += $(call cc-option,-fno-toplevel-reorder)

#########################################################################

quiet_cmd_link_lib = LD      $@
      cmd_link_lib = $(LD) $(ld_flags) -r -o $@ $(filter $(LIBOBJS), $^)

$(LIB):	$(LIBOBJS) FORCE
	$(call if_changed,link_lib)

$(ELF):
$(obj)/%:	$(obj)/%.o $(LIB)
		$(LD) $(LDFLAGS) -g -Ttext $(CONFIG_STANDALONE_LOAD_ADDR) \
			-o $@ -e $(SYM_PREFIX)$(notdir $(<:.o=)) $< $(LIB) \
			-L$(gcclibdir) -lgcc

$(obj)/%.srec:	$(obj)/%
		$(OBJCOPY) -O srec $< $@ 2>/dev/null

$(obj)/%.bin:	$(obj)/%
		$(OBJCOPY) -O binary $< $@ 2>/dev/null
