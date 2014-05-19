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

src := $(obj)

# Create output directory if not already present
_dummy := $(shell [ -d $(obj) ] || mkdir -p $(obj))

include $(srctree)/scripts/Kbuild.include

CONFIG_SPL_BUILD := y
export CONFIG_SPL_BUILD

KBUILD_CPPFLAGS += -DCONFIG_SPL_BUILD
ifeq ($(CONFIG_TPL_BUILD),y)
KBUILD_CPPFLAGS += -DCONFIG_TPL_BUILD
endif

ifeq ($(CONFIG_TPL_BUILD),y)
export CONFIG_TPL_BUILD
SPL_BIN := u-boot-tpl
else
SPL_BIN := u-boot-spl
endif

include include/config.mk

ifeq ($(CONFIG_TPL_BUILD),y)
  -include include/tpl-autoconf.mk
else
  -include include/spl-autoconf.mk
endif

include $(srctree)/config.mk

# Enable garbage collection of un-used sections for SPL
KBUILD_CFLAGS += -ffunction-sections -fdata-sections
LDFLAGS_FINAL += --gc-sections

# FIX ME
cpp_flags := $(KBUILD_CPPFLAGS) $(PLATFORM_CPPFLAGS) $(UBOOTINCLUDE) \
							$(NOSTDINC_FLAGS)
c_flags := $(KBUILD_CFLAGS) $(cpp_flags)

# Auto-generate the spl-autoconf.mk file (which is included by all makefiles for SPL)
quiet_cmd_autoconf = GEN     $@
      cmd_autoconf = \
	$(CPP) $(c_flags) -DDO_DEPS_ONLY -dM $(srctree)/include/common.h > $@.tmp && \
		sed -n -f $(srctree)/tools/scripts/define2mk.sed $@.tmp > $@; \
	rm $@.tmp

include/tpl-autoconf.mk: include/config.h
	$(call cmd,autoconf)

include/spl-autoconf.mk: include/config.h
	$(call cmd,autoconf)

HAVE_VENDOR_COMMON_LIB = $(if $(wildcard $(srctree)/board/$(VENDOR)/common/Makefile),y,n)

ifdef	CONFIG_SPL_START_S_PATH
START_PATH := $(CONFIG_SPL_START_S_PATH:"%"=%)
else
START_PATH := $(CPUDIR)
endif

head-y := $(START_PATH)/start.o
head-$(CONFIG_X86) += $(START_PATH)/start16.o $(START_PATH)/resetvec.o
head-$(CONFIG_4xx) += $(START_PATH)/resetvec.o
head-$(CONFIG_MPC85xx) += $(START_PATH)/resetvec.o

libs-y += arch/$(ARCH)/lib/

libs-y += $(CPUDIR)/

ifdef SOC
libs-y += $(CPUDIR)/$(SOC)/
endif
libs-y += $(if $(BOARDDIR),board/$(BOARDDIR)/)
libs-$(HAVE_VENDOR_COMMON_LIB) += board/$(VENDOR)/common/

libs-$(CONFIG_SPL_FRAMEWORK) += common/spl/
libs-$(CONFIG_SPL_LIBCOMMON_SUPPORT) += common/
libs-$(CONFIG_SPL_LIBDISK_SUPPORT) += disk/
libs-$(CONFIG_SPL_I2C_SUPPORT) += drivers/i2c/
libs-$(CONFIG_SPL_GPIO_SUPPORT) += drivers/gpio/
libs-$(CONFIG_SPL_MMC_SUPPORT) += drivers/mmc/
libs-$(CONFIG_SPL_MPC8XXX_INIT_DDR_SUPPORT) += drivers/ddr/fsl/
libs-$(CONFIG_SPL_SERIAL_SUPPORT) += drivers/serial/
libs-$(CONFIG_SPL_SPI_FLASH_SUPPORT) += drivers/mtd/spi/
libs-$(CONFIG_SPL_SPI_SUPPORT) += drivers/spi/
libs-y += fs/
libs-$(CONFIG_SPL_LIBGENERIC_SUPPORT) += lib/
libs-$(CONFIG_SPL_POWER_SUPPORT) += drivers/power/ drivers/power/pmic/
libs-$(CONFIG_SPL_MTD_SUPPORT) += drivers/mtd/
libs-$(if $(CONFIG_CMD_NAND),$(CONFIG_SPL_NAND_SUPPORT)) += drivers/mtd/nand/
libs-$(CONFIG_SPL_DRIVERS_MISC_SUPPORT) += drivers/misc/
libs-$(CONFIG_SPL_ONENAND_SUPPORT) += drivers/mtd/onenand/
libs-$(CONFIG_SPL_DMA_SUPPORT) += drivers/dma/
libs-$(CONFIG_SPL_POST_MEM_SUPPORT) += post/drivers/
libs-$(CONFIG_SPL_NET_SUPPORT) += net/
libs-$(CONFIG_SPL_ETH_SUPPORT) += drivers/net/
libs-$(CONFIG_SPL_ETH_SUPPORT) += drivers/net/phy/
libs-$(CONFIG_SPL_USBETH_SUPPORT) += drivers/net/phy/
libs-$(CONFIG_SPL_MUSB_NEW_SUPPORT) += drivers/usb/musb-new/
libs-$(CONFIG_SPL_USBETH_SUPPORT) += drivers/usb/gadget/
libs-$(CONFIG_SPL_WATCHDOG_SUPPORT) += drivers/watchdog/
libs-$(CONFIG_SPL_USB_HOST_SUPPORT) += drivers/usb/host/
libs-$(CONFIG_OMAP_USB_PHY) += drivers/usb/phy/
libs-$(CONFIG_SPL_SATA_SUPPORT) += drivers/block/

ifneq (,$(CONFIG_MX23)$(CONFIG_MX35)$(filter $(SOC), mx25 mx27 mx5 mx6 mx31 mx35))
libs-y += arch/$(ARCH)/imx-common/
endif

libs-$(CONFIG_ARM) += arch/arm/cpu/
libs-$(CONFIG_PPC) += arch/powerpc/cpu/

head-y		:= $(addprefix $(obj)/,$(head-y))
libs-y		:= $(addprefix $(obj)/,$(libs-y))
u-boot-spl-dirs	:= $(patsubst %/,%,$(filter %/, $(libs-y)))

libs-y := $(patsubst %/, %/built-in.o, $(libs-y))

# Add GCC lib
ifeq ($(CONFIG_USE_PRIVATE_LIBGCC),y)
PLATFORM_LIBGCC = arch/$(ARCH)/lib/lib.a
PLATFORM_LIBS := $(filter-out %/lib.a, $(filter-out -lgcc, $(PLATFORM_LIBS))) $(PLATFORM_LIBGCC)
endif

u-boot-spl-init := $(head-y)
u-boot-spl-main := $(libs-y)

# Linker Script
ifdef CONFIG_SPL_LDSCRIPT
# need to strip off double quotes
LDSCRIPT := $(addprefix $(srctree)/,$(CONFIG_SPL_LDSCRIPT:"%"=%))
endif

ifeq ($(wildcard $(LDSCRIPT)),)
	LDSCRIPT := $(srctree)/board/$(BOARDDIR)/u-boot-spl.lds
endif
ifeq ($(wildcard $(LDSCRIPT)),)
	LDSCRIPT := $(srctree)/$(CPUDIR)/u-boot-spl.lds
endif
ifeq ($(wildcard $(LDSCRIPT)),)
	LDSCRIPT := $(srctree)/arch/$(ARCH)/cpu/u-boot-spl.lds
endif
ifeq ($(wildcard $(LDSCRIPT)),)
$(error could not find linker script)
endif

# Special flags for CPP when processing the linker script.
# Pass the version down so we can handle backwards compatibility
# on the fly.
LDPPFLAGS += \
	-include $(srctree)/include/u-boot/u-boot.lds.h \
	-include $(objtree)/include/config.h \
	-DCPUDIR=$(CPUDIR) \
	$(shell $(LD) --version | \
	  sed -ne 's/GNU ld version \([0-9][0-9]*\)\.\([0-9][0-9]*\).*/-DLD_MAJOR=\1 -DLD_MINOR=\2/p')

quiet_cmd_mkimage = MKIMAGE $@
cmd_mkimage = $(objtree)/tools/mkimage $(MKIMAGEFLAGS_$(@F)) -d $< $@ \
	$(if $(KBUILD_VERBOSE:1=), >/dev/null)

MKIMAGEFLAGS_MLO = -T omapimage -a $(CONFIG_SPL_TEXT_BASE)

MKIMAGEFLAGS_MLO.byteswap = -T omapimage -n byteswap -a $(CONFIG_SPL_TEXT_BASE)

MLO MLO.byteswap: $(obj)/u-boot-spl.bin
	$(call if_changed,mkimage)

MKIMAGEFLAGS_boot.bin = -T atmelimage

ifeq ($(CONFIG_SPL_GENERATE_ATMEL_PMECC_HEADER),y)
MKIMAGEFLAGS_boot.bin += -n $(shell $(obj)/../tools/atmel_pmecc_params)

boot.bin: $(obj)/../tools/atmel_pmecc_params
endif

boot.bin: $(obj)/u-boot-spl.bin
	$(call if_changed,mkimage)

ALL-y	+= $(obj)/$(SPL_BIN).bin

ifdef CONFIG_SAMSUNG
ALL-y	+= $(obj)/$(BOARD)-spl.bin
endif

ifdef CONFIG_SUNXI
ifndef CONFIG_SPL_FEL
ALL-y	+= $(obj)/sunxi-spl.bin
endif
endif

all:	$(ALL-y)

ifdef CONFIG_SAMSUNG
ifdef CONFIG_VAR_SIZE_SPL
VAR_SIZE_PARAM = --vs
else
VAR_SIZE_PARAM =
endif
$(obj)/$(BOARD)-spl.bin: $(obj)/u-boot-spl.bin
	$(if $(wildcard $(objtree)/spl/board/samsung/$(BOARD)/tools/mk$(BOARD)spl),\
	$(objtree)/spl/board/samsung/$(BOARD)/tools/mk$(BOARD)spl,\
	$(objtree)/tools/mkexynosspl) $(VAR_SIZE_PARAM) $< $@
endif

quiet_cmd_objcopy = OBJCOPY $@
cmd_objcopy = $(OBJCOPY) $(OBJCOPYFLAGS) $(OBJCOPYFLAGS_$(@F)) $< $@

OBJCOPYFLAGS_$(SPL_BIN).bin = $(SPL_OBJCFLAGS) -O binary

$(obj)/$(SPL_BIN).bin: $(obj)/$(SPL_BIN) FORCE
	$(call if_changed,objcopy)

LDFLAGS_$(SPL_BIN) += -T u-boot-spl.lds $(LDFLAGS_FINAL)
ifneq ($(CONFIG_SPL_TEXT_BASE),)
LDFLAGS_$(SPL_BIN) += -Ttext $(CONFIG_SPL_TEXT_BASE)
endif

ifdef CONFIG_SUNXI
quiet_cmd_mksunxiboot = MKSUNXI $@
cmd_mksunxiboot = $(objtree)/tools/mksunxiboot $< $@
$(obj)/sunxi-spl.bin: $(obj)/$(SPL_BIN).bin
	$(call if_changed,mksunxiboot)
endif

quiet_cmd_u-boot-spl = LD      $@
      cmd_u-boot-spl = cd $(obj) && $(LD) $(LDFLAGS) $(LDFLAGS_$(@F)) \
		       $(patsubst $(obj)/%,%,$(u-boot-spl-init)) --start-group \
		       $(patsubst $(obj)/%,%,$(u-boot-spl-main)) --end-group \
		       $(PLATFORM_LIBS) -Map $(SPL_BIN).map -o $(SPL_BIN)

$(obj)/$(SPL_BIN): $(u-boot-spl-init) $(u-boot-spl-main) $(obj)/u-boot-spl.lds
	$(call cmd,u-boot-spl)

$(sort $(u-boot-spl-init) $(u-boot-spl-main)): $(u-boot-spl-dirs) ;

PHONY += $(u-boot-spl-dirs)
$(u-boot-spl-dirs):
	$(Q)$(MAKE) $(build)=$@

quiet_cmd_cpp_lds = LDS     $@
cmd_cpp_lds = $(CPP) -Wp,-MD,$(depfile) $(cpp_flags) $(LDPPFLAGS) -ansi \
		-D__ASSEMBLY__ -x assembler-with-cpp -P -o $@ $<

$(obj)/u-boot-spl.lds: $(LDSCRIPT) FORCE
	$(call if_changed_dep,cpp_lds)

# read all saved command lines

targets := $(wildcard $(sort $(targets)))
cmd_files := $(wildcard $(obj)/.*.cmd $(foreach f,$(targets),$(dir $(f)).$(notdir $(f)).cmd))

ifneq ($(cmd_files),)
  $(cmd_files): ;	# Do not try to update included dependency files
  include $(cmd_files)
endif

PHONY += FORCE
FORCE:

# Declare the contents of the .PHONY variable as phony.  We keep that
# information in a variable so we can use it in if_changed and friends.
.PHONY: $(PHONY)
