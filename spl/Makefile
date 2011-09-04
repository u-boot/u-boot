#
# (C) Copyright 2000-2011
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2011
# Daniel Schwierzeck, daniel.schwierzeck@googlemail.com.
#
# (C) Copyright 2011
# Texas Instruments Incorporated - http://www.ti.com/
# Aneesh V <aneesh@ti.com>
#
# This file is released under the terms of GPL v2 and any later version.
# See the file COPYING in the root directory of the source tree for details.
#
# Based on top-level Makefile.
#

CONFIG_SPL_BUILD := y
export CONFIG_SPL_BUILD

include $(TOPDIR)/config.mk

# We want the final binaries in this directory
obj := $(OBJTREE)/spl/

HAVE_VENDOR_COMMON_LIB := $(shell [ -f $(SRCTREE)/board/$(VENDOR)/common/Makefile ] \
			&& echo y || echo n)

START := $(CPUDIR)/start.o

LIBS-y += arch/$(ARCH)/lib/lib$(ARCH).o
LIBS-y += $(CPUDIR)/lib$(CPU).o
ifdef SOC
LIBS-y += $(CPUDIR)/$(SOC)/lib$(SOC).o
endif
LIBS-y += board/$(BOARDDIR)/lib$(BOARD).o
LIBS-$(HAVE_VENDOR_COMMON_LIB) += board/$(VENDOR)/common/lib$(VENDOR).o

LIBS-$(CONFIG_SPL_LIBCOMMON_SUPPORT) += common/libcommon.o
LIBS-$(CONFIG_SPL_LIBDISK_SUPPORT) += disk/libdisk.o
LIBS-$(CONFIG_SPL_I2C_SUPPORT) += drivers/i2c/libi2c.o
LIBS-$(CONFIG_SPL_GPIO_SUPPORT) += drivers/gpio/libgpio.o
LIBS-$(CONFIG_SPL_MMC_SUPPORT) += drivers/mmc/libmmc.o
LIBS-$(CONFIG_SPL_SERIAL_SUPPORT) += drivers/serial/libserial.o
LIBS-$(CONFIG_SPL_SPI_FLASH_SUPPORT) += drivers/mtd/spi/libspi_flash.o
LIBS-$(CONFIG_SPL_SPI_SUPPORT) += drivers/spi/libspi.o
LIBS-$(CONFIG_SPL_FAT_SUPPORT) += fs/fat/libfat.o
LIBS-$(CONFIG_SPL_LIBGENERIC_SUPPORT) += lib/libgeneric.o

ifeq ($(SOC),omap3)
LIBS-y += $(CPUDIR)/omap-common/libomap-common.o
endif
ifeq ($(SOC),omap4)
LIBS-y += $(CPUDIR)/omap-common/libomap-common.o
endif

START := $(addprefix $(SPLTREE)/,$(START))
LIBS := $(addprefix $(SPLTREE)/,$(sort $(LIBS-y)))

__START := $(subst $(obj),,$(START))
__LIBS := $(subst $(obj),,$(LIBS))

# Linker Script
ifdef CONFIG_SPL_LDSCRIPT
# need to strip off double quotes
LDSCRIPT := $(addprefix $(SRCTREE)/,$(subst ",,$(CONFIG_SPL_LDSCRIPT)))
endif

ifeq ($(wildcard $(LDSCRIPT)),)
	LDSCRIPT := $(TOPDIR)/board/$(BOARDDIR)/u-boot-spl.lds
endif
ifeq ($(wildcard $(LDSCRIPT)),)
	LDSCRIPT := $(TOPDIR)/$(CPUDIR)/u-boot-spl.lds
endif
ifeq ($(wildcard $(LDSCRIPT)),)
$(error could not find linker script)
endif

# Special flags for CPP when processing the linker script.
# Pass the version down so we can handle backwards compatibility
# on the fly.
LDPPFLAGS += \
	-include $(TOPDIR)/include/u-boot/u-boot.lds.h \
	-include $(OBJTREE)/include/config.h \
	$(shell $(LD) --version | \
	  sed -ne 's/GNU ld version \([0-9][0-9]*\)\.\([0-9][0-9]*\).*/-DLD_MAJOR=\1 -DLD_MINOR=\2/p')

ifdef CONFIG_OMAP
$(OBJTREE)/MLO:	$(obj)u-boot-spl.bin
	$(OBJTREE)/tools/mkimage -T omapimage \
		-a $(CONFIG_SPL_TEXT_BASE) -d $< $@
endif

ALL-y	+= $(obj)u-boot-spl.bin

ifdef CONFIG_SAMSUNG
ALL-y	+= $(obj)$(BOARD)-spl.bin
endif

all:	$(ALL-y)

ifdef CONFIG_SAMSUNG
$(obj)$(BOARD)-spl.bin: $(obj)u-boot-spl.bin
	$(TOPDIR)/board/$(BOARDDIR)/tools/mk$(BOARD)spl.exe \
		$(obj)u-boot-spl.bin $(obj)$(BOARD)-spl.bin
endif

$(obj)u-boot-spl.bin:	$(obj)u-boot-spl
	$(OBJCOPY) $(OBJCFLAGS) -O binary $< $@

GEN_UBOOT = \
	UNDEF_SYM=`$(OBJDUMP) -x $(LIBS) | \
	sed  -n -e 's/.*\($(SYM_PREFIX)__u_boot_cmd_.*\)/-u\1/p'|sort|uniq`;\
	cd $(obj) && $(LD) $(LDFLAGS) $(LDFLAGS_$(@F)) $$UNDEF_SYM $(__START) \
		--start-group $(__LIBS) --end-group $(PLATFORM_LIBS) \
		-Map u-boot-spl.map -o u-boot-spl

$(obj)u-boot-spl:	depend $(START) $(LIBS) $(obj)u-boot-spl.lds
	$(GEN_UBOOT)

$(START):	depend
	$(MAKE) -C $(SRCTREE)/$(CPUDIR) $@

$(LIBS):	depend
	$(MAKE) -C $(SRCTREE)$(dir $(subst $(SPLTREE),,$@))

$(obj)u-boot-spl.lds: $(LDSCRIPT) depend
	$(CPP) $(CPPFLAGS) $(LDPPFLAGS) -ansi -D__ASSEMBLY__ -P - < $< > $@

depend:	$(obj).depend
.PHONY: depend

# defines $(obj).depend target
include $(SRCTREE)/rules.mk
