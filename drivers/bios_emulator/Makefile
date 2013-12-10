X86DIR  = x86emu

obj-y = atibios.o biosemu.o besys.o bios.o \
	$(X86DIR)/decode.o \
	$(X86DIR)/ops2.o \
	$(X86DIR)/ops.o \
	$(X86DIR)/prim_ops.o \
	$(X86DIR)/sys.o \
	$(X86DIR)/debug.o

EXTRA_CFLAGS += -I. -I./include \
	-D__PPC__  -D__BIG_ENDIAN__

CFLAGS += $(EXTRA_CFLAGS)
CPPFLAGS += $(EXTRA_CFLAGS)
