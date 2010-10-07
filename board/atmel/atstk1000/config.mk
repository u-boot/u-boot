PLATFORM_RELFLAGS	+= -ffunction-sections -fdata-sections
PLATFORM_LDFLAGS	+= --gc-sections
CONFIG_SYS_TEXT_BASE		= 0x00000000
LDSCRIPT		= $(src)board/atmel/atstk1000/u-boot.lds
