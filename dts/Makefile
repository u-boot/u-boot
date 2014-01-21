#
# Copyright (c) 2011 The Chromium OS Authors.
#
# SPDX-License-Identifier:	GPL-2.0+
#

# This Makefile builds the internal U-Boot fdt if CONFIG_OF_CONTROL is
# enabled. See doc/README.fdt-control for more details.

ifeq ($(DEVICE_TREE),)
$(if $(CONFIG_DEFAULT_DEVICE_TREE),,\
$(error Please define CONFIG_DEFAULT_DEVICE_TREE in your board header file))
DEVICE_TREE = $(CONFIG_DEFAULT_DEVICE_TREE:"%"=%)
endif

DTS_INCDIRS =  $(SRCTREE)/board/$(VENDOR)/$(BOARD)/dts
DTS_INCDIRS += $(SRCTREE)/board/$(VENDOR)/dts
DTS_INCDIRS += $(SRCTREE)/arch/$(ARCH)/dts

DTS_CPPFLAGS := -x assembler-with-cpp -undef -D__DTS__ \
		-nostdinc $(addprefix -I,$(DTS_INCDIRS))

DTC_FLAGS := -R 4 -p 0x1000 \
	$(addprefix -i ,$(DTS_INCDIRS))

# Use a constant name for this so we can access it from C code.
# objcopy doesn't seem to allow us to set the symbol name independently of
# the filename.
DT_BIN	:= $(obj)dt.dtb

$(DT_BIN): $(TOPDIR)/board/$(VENDOR)/dts/$(DEVICE_TREE).dts
	$(CPP) $(DTS_CPPFLAGS) $< -o $(DT_BIN).dts.tmp
	$(DTC) $(DTC_FLAGS) -O dtb -o ${DT_BIN} $(DT_BIN).dts.tmp

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

obj-$(CONFIG_OF_EMBED)	:= dt.o

binary:	$(DT_BIN)
