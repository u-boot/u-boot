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
# SPDX-License-Identifier:	GPL-2.0+
#
# Based on top-level Makefile.
#

CONFIG_SPL_BUILD := y
export CONFIG_SPL_BUILD

ifeq ($(CONFIG_TPL_BUILD),y)
export CONFIG_TPL_BUILD
SPL_BIN := u-boot-tpl
else
SPL_BIN := u-boot-spl
endif

include $(TOPDIR)/config.mk

# We want the final binaries in this directory
ifeq ($(CONFIG_TPL_BUILD),y)
obj := $(OBJTREE)/tpl/
SPLTREE := $(TPLTREE)
else
obj := $(OBJTREE)/spl/
endif

HAVE_VENDOR_COMMON_LIB = $(if $(wildcard $(SRCTREE)/board/$(VENDOR)/common/Makefile),y,n)

ifdef	CONFIG_SPL_START_S_PATH
START_PATH := $(subst ",,$(CONFIG_SPL_START_S_PATH))
else
START_PATH := $(CPUDIR)
endif

START := $(START_PATH)/start.o
ifeq ($(CPU),x86)
START += $(START_PATH)/start16.o
START += $(START_PATH)/resetvec.o
endif
ifeq ($(CPU),ppc4xx)
START += $(START_PATH)/resetvec.o
endif
ifeq ($(CPU),mpc85xx)
START += $(START_PATH)/resetvec.o
endif

LIBS-y += arch/$(ARCH)/lib/

LIBS-y += $(CPUDIR)/

ifdef SOC
LIBS-y += $(CPUDIR)/$(SOC)/
endif
LIBS-y += board/$(BOARDDIR)/
LIBS-$(HAVE_VENDOR_COMMON_LIB) += board/$(VENDOR)/common/

LIBS-$(CONFIG_SPL_FRAMEWORK) += common/spl/
LIBS-$(CONFIG_SPL_LIBCOMMON_SUPPORT) += common/
LIBS-$(CONFIG_SPL_LIBDISK_SUPPORT) += disk/
LIBS-$(CONFIG_SPL_I2C_SUPPORT) += drivers/i2c/
LIBS-$(CONFIG_SPL_GPIO_SUPPORT) += drivers/gpio/
LIBS-$(CONFIG_SPL_MMC_SUPPORT) += drivers/mmc/
LIBS-$(CONFIG_SPL_MPC8XXX_INIT_DDR_SUPPORT) += drivers/ddr/fsl/
LIBS-$(CONFIG_SPL_SERIAL_SUPPORT) += drivers/serial/
LIBS-$(CONFIG_SPL_SPI_FLASH_SUPPORT) += drivers/mtd/spi/
LIBS-$(CONFIG_SPL_SPI_SUPPORT) += drivers/spi/
LIBS-y += fs/
LIBS-$(CONFIG_SPL_LIBGENERIC_SUPPORT) += lib/
LIBS-$(CONFIG_SPL_POWER_SUPPORT) += drivers/power/ \
	drivers/power/pmic/
LIBS-$(CONFIG_SPL_NAND_SUPPORT) += drivers/mtd/nand/
LIBS-$(CONFIG_SPL_ONENAND_SUPPORT) += drivers/mtd/onenand/
LIBS-$(CONFIG_SPL_DMA_SUPPORT) += drivers/dma/
LIBS-$(CONFIG_SPL_POST_MEM_SUPPORT) += post/drivers/
LIBS-$(CONFIG_SPL_NET_SUPPORT) += net/
LIBS-$(CONFIG_SPL_ETH_SUPPORT) += drivers/net/
LIBS-$(CONFIG_SPL_ETH_SUPPORT) += drivers/net/phy/
LIBS-$(CONFIG_SPL_USBETH_SUPPORT) += drivers/net/phy/
LIBS-$(CONFIG_SPL_MUSB_NEW_SUPPORT) += drivers/usb/musb-new/
LIBS-$(CONFIG_SPL_USBETH_SUPPORT) += drivers/usb/gadget/
LIBS-$(CONFIG_SPL_WATCHDOG_SUPPORT) += drivers/watchdog/

ifneq (,$(CONFIG_MX23)$(filter $(SOC), mx25 mx27 mx5 mx6 mx31 mx35))
LIBS-y += arch/$(ARCH)/imx-common/
endif

LIBS-$(CONFIG_ARM) += arch/arm/cpu/
LIBS-$(CONFIG_PPC) += arch/powerpc/cpu/

ifneq ($(CONFIG_MX23)$(CONFIG_MX35),)
LIBS-y += arch/$(ARCH)/imx-common/
endif

LIBS-y := $(patsubst %/, %/built-in.o, $(LIBS-y))

# Add GCC lib
ifeq ("$(USE_PRIVATE_LIBGCC)", "yes")
PLATFORM_LIBGCC = $(SPLTREE)/arch/$(ARCH)/lib/libgcc.o
PLATFORM_LIBS := $(filter-out %/libgcc.o, $(filter-out -lgcc, $(PLATFORM_LIBS))) $(PLATFORM_LIBGCC)
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
	LDSCRIPT := $(TOPDIR)/arch/$(ARCH)/cpu/u-boot-spl.lds
endif
ifeq ($(wildcard $(LDSCRIPT)),)
$(error could not find linker script)
endif

build := -f $(TOPDIR)/scripts/Makefile.build -C

# Special flags for CPP when processing the linker script.
# Pass the version down so we can handle backwards compatibility
# on the fly.
LDPPFLAGS += \
	-include $(TOPDIR)/include/u-boot/u-boot.lds.h \
	-include $(OBJTREE)/include/config.h \
	-DCPUDIR=$(CPUDIR) \
	$(shell $(LD) --version | \
	  sed -ne 's/GNU ld version \([0-9][0-9]*\)\.\([0-9][0-9]*\).*/-DLD_MAJOR=\1 -DLD_MINOR=\2/p')

$(OBJTREE)/MLO:	$(obj)u-boot-spl.bin
	$(OBJTREE)/tools/mkimage -T omapimage \
		-a $(CONFIG_SPL_TEXT_BASE) -d $< $@

$(OBJTREE)/MLO.byteswap: $(obj)u-boot-spl.bin
	$(OBJTREE)/tools/mkimage -T omapimage -n byteswap \
		-a $(CONFIG_SPL_TEXT_BASE) -d $< $@

$(OBJTREE)/SPL : $(obj)u-boot-spl.bin depend
		$(MAKE) $(build) $(SRCTREE)/arch/arm/imx-common $@

ALL-y	+= $(obj)$(SPL_BIN).bin

ifdef CONFIG_SAMSUNG
ALL-y	+= $(obj)$(BOARD)-spl.bin
endif

all:	$(ALL-y)

ifdef CONFIG_SAMSUNG
ifdef CONFIG_VAR_SIZE_SPL
VAR_SIZE_PARAM = --vs
else
VAR_SIZE_PARAM =
endif
$(obj)$(BOARD)-spl.bin: $(obj)u-boot-spl.bin
	$(OBJTREE)/tools/mk$(BOARD)spl $(VAR_SIZE_PARAM) $< $@
endif

$(obj)$(SPL_BIN).bin:	$(obj)$(SPL_BIN)
	$(OBJCOPY) $(OBJCFLAGS) -O binary $< $@

GEN_UBOOT = \
	cd $(obj) && $(LD) $(LDFLAGS) $(LDFLAGS_$(@F)) $(__START) \
		--start-group $(__LIBS) --end-group $(PLATFORM_LIBS) \
		-Map $(SPL_BIN).map -o $(SPL_BIN)

$(obj)$(SPL_BIN):	depend $(START) $(LIBS) $(obj)u-boot-spl.lds
	$(GEN_UBOOT)

$(START):
	@:

$(LIBS):	depend
	$(MAKE) $(build) $(SRCTREE)$(dir $(subst $(SPLTREE),,$@))

$(obj)u-boot-spl.lds: $(LDSCRIPT) depend
	$(CPP) $(CPPFLAGS) $(LDPPFLAGS) -I$(obj). -ansi -D__ASSEMBLY__ -P - < $< > $@

depend:	$(obj).depend
.PHONY: depend

# defines $(obj).depend target
include $(SRCTREE)/rules.mk
