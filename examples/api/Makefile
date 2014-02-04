#
# (C) Copyright 2007 Semihalf
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifeq ($(ARCH),powerpc)
LOAD_ADDR = 0x40000
endif
ifeq ($(ARCH),arm)
LOAD_ADDR = 0x1000000
endif

# Resulting ELF and binary exectuables will be named demo and demo.bin
extra-y = demo

# Source files located in the examples/api directory
SOBJ_FILES-y += crt0.o
COBJ_FILES-y += demo.o
COBJ_FILES-y += glue.o
COBJ_FILES-y += libgenwrap.o

# Source files which exist outside the examples/api directory
EXT_COBJ_FILES-y += lib/crc32.o
EXT_COBJ_FILES-y += lib/ctype.o
EXT_COBJ_FILES-y += lib/div64.o
EXT_COBJ_FILES-y += lib/string.o
EXT_COBJ_FILES-y += lib/time.o
EXT_COBJ_FILES-y += lib/vsprintf.o
EXT_SOBJ_FILES-$(CONFIG_PPC) += arch/powerpc/lib/ppcstring.o

# Create a list of object files to be compiled
OBJS	+= $(addprefix $(obj)/,$(SOBJ_FILES-y))
OBJS	+= $(addprefix $(obj)/,$(COBJ_FILES-y))
OBJS	+= $(addprefix $(obj)/,$(notdir $(EXT_COBJ_FILES-y)))
OBJS	+= $(addprefix $(obj)/,$(notdir $(EXT_SOBJ_FILES-y)))

#########################################################################

$(obj)/demo:	$(OBJS)
		$(LD) --gc-sections -Ttext $(LOAD_ADDR) -o $@ $^ $(PLATFORM_LIBS)

$(obj)/demo.bin: $(obj)/demo
		$(OBJCOPY) -O binary $< $@ 2>/dev/null

# Rule to build generic library C files
$(addprefix $(obj)/,$(notdir $(EXT_COBJ_FILES-y))): $(obj)/%.o: $(SRCTREE)/lib/%.c FORCE
	$(call cmd,force_checksrc)
	$(call if_changed_rule,cc_o_c)

# Rule to build architecture-specific library assembly files
$(addprefix $(obj)/,$(notdir $(EXT_SOBJ_FILES-y))): $(obj)/%.o: $(SRCTREE)/arch/$(ARCH)/lib/%.S
	$(call if_changed_dep,as_o_S)
