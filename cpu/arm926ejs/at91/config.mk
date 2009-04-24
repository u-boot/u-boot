PLATFORM_CPPFLAGS += $(call cc-option,-mtune=arm926ejs,)
LDSCRIPT := $(OBJTREE)/cpu/arm926ejs/at91/u-boot.lds
