#
# 	AIT cam_enc_4xx board
#	cam_enc_4xx board has 1 bank of 256 MB DDR RAM
#	Physical Address: 8000'0000 to 9000'0000
#
# Linux Kernel is expected to be at 8000'8000, entry 8000'8000
# (mem base + reserved)
#

UBL_CONFIG = $(srctree)/board/$(BOARDDIR)/ublimage.cfg
ifndef CONFIG_SPL_BUILD
ALL-y += u-boot.ubl
else
# as SPL_TEXT_BASE is not page-aligned, we need for some
# linkers the -n flag (Do not page align data), to prevent
# the following error message:
# arm-linux-ld: u-boot-spl: Not enough room for program headers, try linking
# with -N
LDFLAGS_u-boot-spl += -n
endif
