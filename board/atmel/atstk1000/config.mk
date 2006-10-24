PLATFORM_RELFLAGS	+= -ffunction-sections -fdata-sections
PLATFORM_LDFLAGS	+= --gc-sections
TEXT_BASE		= 0x00000000
LDSCRIPT		= $(obj)board/atmel/atstk1000/u-boot.lds
