#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

extra-$(CONFIG_SPL_BUILD) := start.o

obj-y	= clock.o mxs.o iomux.o timer.o

ifdef	CONFIG_SPL_BUILD
obj-y	+= spl_boot.o spl_lradc_init.o spl_mem_init.o spl_power_init.o
endif

# Specify the target for use in elftosb call
MKIMAGE_TARGET-$(CONFIG_MX23) = mx23
MKIMAGE_TARGET-$(CONFIG_MX28) = mx28

# Convert hexadecimal value to bytes
define hex2bin
$(shell echo -n "$1" | sed 's/0x//;s/\(..\)\(..\)\(..\)\(..\)/\4\3\2\1/;s/../\\\\x&/g')
endef

# Compute the post-IVT size field value for the U-Boot binary.
# The value is the result of adding the following:
#  -> The size of U-Boot binary aligned to 64B (u-boot.bin)
#  -> The size of IVT block aligned to 64B (u-boot.ivt)
#  -> The size of U-Boot signature (u-boot.sig), 3904 B
#  -> The 64B hole in front of U-Boot binary for 'struct mxs_spl_data' passing
define uboot_ivt_size
$(shell expr `stat -c "%s" $1` + 64 + 3904 + 128 | xargs printf 0x%08x)
endef

$(OBJTREE)/mxsimage.cfg: $(SRCTREE)/$(CPUDIR)/$(SOC)/mxsimage.$(MKIMAGE_TARGET-y).cfg
	sed "s@OBJTREE@$(OBJTREE)@g" $^ > $@

# HAB signature is i.MX28 only
$(OBJTREE)/mxsimage-signed.cfg: $(SRCTREE)/$(CPUDIR)/$(SOC)/mxsimage-signed.cfg
	sed "s@OBJTREE@$(OBJTREE)@g" $^ > $@

$(OBJTREE)/spl/u-boot-spl.ivt: $(OBJTREE)/spl/u-boot-spl.bin
	# Align U-Boot SPL binary to 64B
	dd if=$^ of=$@ ibs=64 conv=sync 2>/dev/null
	mv $@ $^
	# Assemble IVT, append size field and align it to 64B.
	(echo -ne "$(call hex2bin,0x402000d1)" ;		\
	 echo -ne "$(call hex2bin,$(CONFIG_SPL_TEXT_BASE))" ;	\
	 echo -ne "$(call hex2bin,0x00000000)" ;		\
	 echo -ne "$(call hex2bin,0x00000000)" ;		\
	 echo -ne "$(call hex2bin,0x00000000)" ;		\
	 echo -ne "$(call hex2bin,0x00008000)" ; 		\
	 echo -ne "$(call hex2bin,0x00008040)" ;		\
	 echo -ne "$(call hex2bin,0x00000000)" ;		\
	 echo -ne "$(call hex2bin,$(call uboot_ivt_size,$^))"	\
	) | dd of=$@ ibs=64 count=1 conv=sync 2>/dev/null

$(OBJTREE)/u-boot.ivt: $(OBJTREE)/u-boot.bin
	# Align U-Boot binary to 64B
	dd if=$^ of=$@ ibs=64 conv=sync 2>/dev/null
	mv $@ $^
	# Assemble IVT, append size field and align it to 64B.
	(echo -ne "$(call hex2bin,0x402000d1)" ;		\
	 echo -ne "$(call hex2bin,$(CONFIG_SYS_TEXT_BASE))" ;	\
	 echo -ne "$(call hex2bin,0x00000000)" ;		\
	 echo -ne "$(call hex2bin,0x00000000)" ;		\
	 echo -ne "$(call hex2bin,0x00000000)" ;		\
	 echo -ne "$(call hex2bin,0x40001000)" ; 		\
	 echo -ne "$(call hex2bin,0x40001040)" ;		\
	 echo -ne "$(call hex2bin,0x00000000)" ;		\
	 echo -ne "$(call hex2bin,$(call uboot_ivt_size,$^))"	\
	) | dd of=$@ ibs=64 count=1 conv=sync 2>/dev/null

$(OBJTREE)/spl/u-boot-spl.csf: $(OBJTREE)/spl/u-boot-spl.ivt $(OBJTREE)/spl/u-boot-spl.bin $(TOPDIR)/board/$(VENDOR)/$(BOARD)/sign/u-boot-spl.csf
	# Assemble the CSF file
	sed "s@TOPDIR@$(TOPDIR)@g;s@VENDOR@$(VENDOR)@g;s@BOARD@$(BOARD)@g" \
		$(word 3,$^) > $@
	sed -i "/^##Blocks.*/ d" $@
	echo "  Blocks = $(CONFIG_SPL_TEXT_BASE) 0x0 "			\
		"`stat -c '%s' $(word 2,$^)` \"$(word 2,$^)\" , \\"	\
		>> $@
	echo "           0x8000 0x0 0x40 \"$(word 1,$^)\"" >> $@

$(OBJTREE)/u-boot.csf: $(OBJTREE)/u-boot.ivt $(OBJTREE)/u-boot.bin $(TOPDIR)/board/$(VENDOR)/$(BOARD)/sign/u-boot.csf
	# Assemble the CSF file
	sed "s@TOPDIR@$(TOPDIR)@g;s@VENDOR@$(VENDOR)@g;s@BOARD@$(BOARD)@g" \
		$(word 3,$^) > $@
	sed -i "/^##Blocks.*/ d" $@
	echo "  Blocks = $(CONFIG_SYS_TEXT_BASE) 0x0 "			\
		"`stat -c '%s' $(word 2,$^)` \"$(word 2,$^)\" , \\"	\
		>> $@
	echo "           0x40001000 0x0 0x40 \"$(word 1,$^)\"" >> $@

%.sig: %.csf
	cst -o $@ < $^

$(OBJTREE)/u-boot-signed.sb: $(OBJTREE)/u-boot.ivt $(OBJTREE)/u-boot.sig $(OBJTREE)/spl/u-boot-spl.ivt $(OBJTREE)/spl/u-boot-spl.sig $(OBJTREE)/mxsimage-signed.cfg
	$(OBJTREE)/tools/mkimage -n $(OBJTREE)/mxsimage-signed.cfg -T mxsimage $@

$(OBJTREE)/u-boot.sb: $(OBJTREE)/u-boot.bin $(OBJTREE)/spl/u-boot-spl.bin $(OBJTREE)/mxsimage.cfg
	$(OBJTREE)/tools/mkimage -n $(OBJTREE)/mxsimage.cfg -T mxsimage $@
