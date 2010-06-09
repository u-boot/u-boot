PLATFORM_RELFLAGS	+= -ffunction-sections -fdata-sections
PLATFORM_LDFLAGS	+= --gc-sections
TEXT_BASE		= 0x00000000
LDSCRIPT		= $(src)board/earthlcd/favr-32-ezkit/u-boot.lds
