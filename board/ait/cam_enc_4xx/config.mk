#
# 	AIT cam_enc_4xx board
#	cam_enc_4xx board has 1 bank of 256 MB DDR RAM
#	Physical Address: 8000'0000 to 9000'0000
#
# Linux Kernel is expected to be at 8000'8000, entry 8000'8000
# (mem base + reserved)
#

#Provide at least 16MB spacing between us and the Linux Kernel image
PAD_TO	:= 12320
UBL_CONFIG = $(SRCTREE)/board/$(BOARDDIR)/ublimage.cfg
ifndef CONFIG_SPL_BUILD
ALL-y += $(obj)u-boot.ubl
endif
