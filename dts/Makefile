#
# Copyright (c) 2011 The Chromium OS Authors.
#
# SPDX-License-Identifier:	GPL-2.0+
#

# This Makefile builds the internal U-Boot fdt if CONFIG_OF_CONTROL is
# enabled. See doc/README.fdt-control for more details.

include $(TOPDIR)/config.mk

LIB	= $(obj)libdts.o

ifeq ($(DEVICE_TREE),)
$(if $(CONFIG_DEFAULT_DEVICE_TREE),,\
$(error Please define CONFIG_DEFAULT_DEVICE_TREE in your board header file))
DEVICE_TREE = $(subst ",,$(CONFIG_DEFAULT_DEVICE_TREE))
endif

$(if $(CONFIG_ARCH_DEVICE_TREE),,\
$(error Your architecture does not have device tree support enabled. \
Please define CONFIG_ARCH_DEVICE_TREE))

# We preprocess the device tree file provide a useful define
DTS_CPPFLAGS := -x assembler-with-cpp \
		-DARCH_CPU_DTS=\"$(SRCTREE)/arch/$(ARCH)/dts/$(CONFIG_ARCH_DEVICE_TREE).dtsi\" \
		-DBOARD_DTS=\"$(SRCTREE)/board/$(VENDOR)/$(BOARD)/dts/$(DEVICE_TREE).dts\" \
		-I$(SRCTREE)/board/$(VENDOR)/dts -I$(SRCTREE)/arch/$(ARCH)/dts

all:	$(obj).depend $(LIB)

# Use a constant name for this so we can access it from C code.
# objcopy doesn't seem to allow us to set the symbol name independently of
# the filename.
DT_BIN	:= $(obj)dt.dtb

$(DT_BIN): $(TOPDIR)/board/$(VENDOR)/dts/$(DEVICE_TREE).dts
	rc=$$( \
		cat $< | $(CPP) -P $(DTS_CPPFLAGS) - | \
		{ { $(DTC) -R 4 -p 0x1000 -O dtb -o ${DT_BIN} - 2>&1 ; \
		    echo $$? >&3 ; } | \
		  grep -v '^DTC: dts->dtb  on file' ; \
	        } 3>&1 1>&2 ) ; \
	exit $$rc

process_lds = \
	$(1) | sed -r -n 's/^OUTPUT_$(2)[ ("]*([^")]*).*/\1/p'

# Run the compiler and get the link script from the linker
GET_LDS = $(CC) $(CFLAGS) $(LDFLAGS) -Wl,--verbose 2>&1

$(obj)dt.o: $(DT_BIN)
	# We want the output format and arch.
	# We also hope to win a prize for ugliest Makefile / shell interaction
	# We look in the LDSCRIPT first.
	# Then try the linker which should give us the answer.
	# Then check it worked.
	[ -n "$(LDSCRIPT)" ] && \
		oformat=`$(call process_lds,cat $(LDSCRIPT),FORMAT)` && \
		oarch=`$(call process_lds,cat $(LDSCRIPT),ARCH)` ;\
	\
	[ -z $${oformat} ] && \
		oformat=`$(call process_lds,$(GET_LDS),FORMAT)` ;\
	[ -z $${oarch} ] && \
		oarch=`$(call process_lds,$(GET_LDS),ARCH)` ;\
	\
	[ -z $${oformat} ] && \
		echo "Cannot read OUTPUT_FORMAT from lds file $(LDSCRIPT)" && \
		exit 1 || true ;\
	[ -z $${oarch} ] && \
		echo "Cannot read OUTPUT_ARCH from lds file $(LDSCRIPT)" && \
		exit 1 || true ;\
	\
	cd $(dir ${DT_BIN}) && \
	$(OBJCOPY) -I binary -O $${oformat} -B $${oarch} \
		$(notdir ${DT_BIN}) $@
	rm $(DT_BIN)

OBJS-$(CONFIG_OF_EMBED)	:= dt.o

COBJS	:= $(OBJS-y)

OBJS	:= $(addprefix $(obj),$(COBJS))

binary:	$(DT_BIN)

$(LIB):	$(OBJS) $(DTB)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
