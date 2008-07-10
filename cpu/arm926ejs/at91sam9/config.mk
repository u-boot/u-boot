PLATFORM_CPPFLAGS += -march=armv5te
PLATFORM_CPPFLAGS += $(call cc-option,-mtune=arm926ejs,)
LDSCRIPT := $(SRCTREE)/cpu/arm926ejs/at91sam9/u-boot.lds
