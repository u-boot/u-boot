# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2011
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2011
# Daniel Schwierzeck, daniel.schwierzeck@googlemail.com.
#
# (C) Copyright 2011
# Texas Instruments Incorporated - https://www.ti.com/
# Aneesh V <aneesh@ti.com>
# Based on top-level Makefile.
#

src := $(obj)

# Create output directory if not already present
_dummy := $(shell [ -d $(obj) ] || mkdir -p $(obj))

include $(srctree)/scripts/Kbuild.include

-include include/config/auto.conf

# This file contains 0, or 2 lines
# It is empty for U-Boot proper (where $(obj) is empty)
# For any xPL build it contains CONFIG_XPL_BUILD=y
#    - for SPL builds it also contains CONFIG_SPL_BUILD=y
#    - for TPL builds it also contains CONFIG_TPL_BUILD=y
#    - for VPL builds it also contains CONFIG_VPL_BUILD=y
-include $(obj)/include/autoconf.mk

UBOOTINCLUDE := -I$(obj)/include $(UBOOTINCLUDE)

KBUILD_CPPFLAGS += -DCONFIG_XPL_BUILD
ifeq ($(CONFIG_SPL_BUILD),y)
KBUILD_CPPFLAGS += -DCONFIG_SPL_BUILD
endif
ifeq ($(CONFIG_TPL_BUILD),y)
KBUILD_CPPFLAGS += -DCONFIG_TPL_BUILD
else
ifeq ($(CONFIG_VPL_BUILD),y)
KBUILD_CPPFLAGS += -DCONFIG_VPL_BUILD
endif
endif

ifeq ($(CONFIG_VPL_BUILD),y)
SPL_BIN := u-boot-vpl
SPL_NAME := vpl
else
ifeq ($(CONFIG_TPL_BUILD),y)
SPL_BIN := u-boot-tpl
SPL_NAME := tpl
else
SPL_BIN := u-boot-spl
SPL_NAME := spl
endif
endif

export SPL_NAME

ifeq ($(CONFIG_SPL_BUILD),y)
PHASE_ := SPL_
else
ifeq ($(CONFIG_VPL_BUILD),y)
PHASE_ := VPL_
else
ifeq ($(CONFIG_TPL_BUILD),y)
PHASE_ := TPL_
else
PHASE_ :=
endif
endif
endif

ifeq ($(obj)$(CONFIG_SUPPORT_SPL),spl)
$(error You cannot build SPL without enabling CONFIG_SUPPORT_SPL)
endif
ifeq ($(obj)$(CONFIG_SUPPORT_TPL),tpl)
$(error You cannot build TPL without enabling CONFIG_SUPPORT_TPL)
endif
ifeq ($(obj)$(CONFIG_SUPPORT_VPL),vpl)
$(error You cannot build VPL without enabling CONFIG_SUPPORT_VPL)
endif

include $(srctree)/config.mk
include $(srctree)/arch/$(ARCH)/Makefile

include $(srctree)/scripts/Makefile.lib

# Enable garbage collection of un-used sections for SPL
KBUILD_CFLAGS += -ffunction-sections -fdata-sections
LDFLAGS_FINAL += --gc-sections

ifeq ($(CONFIG_$(PHASE_)STACKPROTECTOR),y)
KBUILD_CFLAGS += -fstack-protector-strong
else
KBUILD_CFLAGS += -fno-stack-protector
endif

# FIX ME
cpp_flags := $(KBUILD_CPPFLAGS) $(PLATFORM_CPPFLAGS) $(UBOOTINCLUDE) \
							$(NOSTDINC_FLAGS)
c_flags := $(KBUILD_CFLAGS) $(cpp_flags)

HAVE_VENDOR_COMMON_LIB = $(if $(wildcard $(srctree)/board/$(VENDOR)/common/Makefile),y,n)

libs-y += $(if $(wildcard $(srctree)/board/$(BOARDDIR)/Makefile),board/$(BOARDDIR)/)
libs-$(HAVE_VENDOR_COMMON_LIB) += board/$(VENDOR)/common/

ifeq ($(CONFIG_TPL_BUILD),y)
libs-$(CONFIG_TPL_FRAMEWORK) += common/spl/
else
libs-$(CONFIG_SPL_FRAMEWORK) += common/spl/
endif
libs-y += common/init/

# Special handling for a few options which support SPL/TPL/VPL
libs-$(CONFIG_$(PHASE_)LIBCOMMON_SUPPORT) += boot/ common/ cmd/ env/
libs-$(CONFIG_$(PHASE_)LIBGENERIC_SUPPORT) += lib/
ifdef CONFIG_SPL_FRAMEWORK
libs-$(CONFIG_PARTITIONS) += disk/
endif

libs-y += drivers/
libs-$(CONFIG_SPL_MEMORY) += drivers/memory/
libs-$(CONFIG_SPL_USB_GADGET) += drivers/usb/dwc3/
libs-$(CONFIG_SPL_USB_GADGET) += drivers/usb/cdns3/
libs-y += dts/
libs-y += fs/
libs-$(CONFIG_SPL_POST_MEM_SUPPORT) += post/drivers/
libs-$(CONFIG_SPL_NET) += net/
libs-$(CONFIG_$(PHASE_)UNIT_TEST) += test/

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
ifdef CONFIG_$(PHASE_)OF_PLATDATA
platdata-hdr := include/generated/dt-structs-gen.h include/generated/dt-decl.h
platdata-inst := $(obj)/dts/dt-uclass.o $(obj)/dts/dt-device.o
platdata-noinst := $(obj)/dts/dt-plat.o

ifdef CONFIG_$(PHASE_)OF_PLATDATA_INST
u-boot-spl-platdata := $(platdata-inst)
u-boot-spl-old-platdata := $(platdata-noinst)
else
u-boot-spl-platdata := $(platdata-noinst)
u-boot-spl-old-platdata := $(platdata-inst)
endif

# Files we need to generate
u-boot-spl-platdata_c := $(patsubst %.o,%.c,$(u-boot-spl-platdata))

# Files we won't generate and should remove
u-boot-spl-old-platdata_c := $(patsubst %.o,%.c,$(u-boot-spl-old-platdata))
endif  # OF_PLATDATA

# Linker Script
# First test whether there's a linker-script for the specific stage defined...
ifneq ($(CONFIG_$(PHASE_)LDSCRIPT),)
# need to strip off double quotes
LDSCRIPT := $(addprefix $(srctree)/,$(CONFIG_$(PHASE_)LDSCRIPT:"%"=%))
else
# ...then fall back to the generic SPL linker-script
ifneq ($(CONFIG_SPL_LDSCRIPT),)
# need to strip off double quotes
LDSCRIPT := $(addprefix $(srctree)/,$(CONFIG_SPL_LDSCRIPT:"%"=%))
endif
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

# Turn various CONFIG symbols into IMAGE symbols for easy reuse of
# the scripts between SPL, TPL and VPL.
ifneq ($(CONFIG_$(PHASE_)MAX_SIZE),0x0)
LDPPFLAGS += -DIMAGE_MAX_SIZE=$(CONFIG_$(PHASE_)MAX_SIZE)
endif
ifneq ($(CONFIG_$(PHASE_)TEXT_BASE),)
LDPPFLAGS += -DIMAGE_TEXT_BASE=$(CONFIG_$(PHASE_)TEXT_BASE)
endif

MKIMAGEOUTPUT ?= /dev/null

quiet_cmd_mkimage = MKIMAGE $@
cmd_mkimage = $(objtree)/tools/mkimage $(MKIMAGEFLAGS_$(@F)) -d $< $@ \
	>$(MKIMAGEOUTPUT) $(if $(KBUILD_VERBOSE:0=), && cat $(MKIMAGEOUTPUT))

quiet_cmd_mkfitimage = MKIMAGE $@
cmd_mkfitimage = $(objtree)/tools/mkimage $(MKIMAGEFLAGS_$(@F)) -f $(SPL_ITS) -E $@ \
	$(if $(KBUILD_VERBOSE:1=), MKIMAGEOUTPUT)

MKIMAGEFLAGS_MLO = -T omapimage -a $(CONFIG_SPL_TEXT_BASE)

MKIMAGEFLAGS_MLO.byteswap = -T omapimage -n byteswap -a $(CONFIG_SPL_TEXT_BASE)

MLO MLO.byteswap: $(obj)/u-boot-spl.bin FORCE
	$(call if_changed,mkimage)

ifeq ($(CONFIG_SYS_SOC),"at91")
MKIMAGEFLAGS_boot.bin = -T atmelimage

ifeq ($(CONFIG_SPL_GENERATE_ATMEL_PMECC_HEADER),y)
MKIMAGEFLAGS_boot.bin += -n $(shell $(obj)/../tools/atmel_pmecc_params)

$(obj)/boot.bin: $(obj)/../tools/atmel_pmecc_params
endif

$(obj)/boot.bin: $(obj)/u-boot-spl.bin FORCE
	$(call if_changed,mkimage)
else
ifdef CONFIG_ARCH_ZYNQ
MKIMAGEFLAGS_boot.bin = -T zynqimage -R $(srctree)/$(CONFIG_BOOT_INIT_FILE)
endif
ifdef CONFIG_ARCH_ZYNQMP
ifneq ($(CONFIG_PMUFW_INIT_FILE),"")
spl/boot.bin: zynqmp-check-pmufw
zynqmp-check-pmufw: FORCE
	( cd $(srctree) && test -r $(CONFIG_PMUFW_INIT_FILE) ) \
		|| ( echo "Cannot read $(CONFIG_PMUFW_INIT_FILE)" && false )
endif
MKIMAGEFLAGS_boot.bin = -T zynqmpimage -R $(srctree)/$(CONFIG_BOOT_INIT_FILE) \
	-n "$(shell cd $(srctree); readlink -f $(CONFIG_PMUFW_INIT_FILE))"
endif

$(obj)/$(SPL_BIN)-align.bin: $(obj)/$(SPL_BIN).bin
	@dd if=$< of=$@ conv=block,sync bs=4 2>/dev/null;

spl/boot.bin: $(obj)/$(SPL_BIN)-align.bin FORCE
	$(call if_changed,mkimage)
endif

INPUTS-y	+= $(obj)/$(SPL_BIN).bin $(obj)/$(SPL_BIN).sym

ifneq ($(CONFIG_ARCH_EXYNOS)$(CONFIG_ARCH_S5PC1XX),)
INPUTS-y	+= $(obj)/$(BOARD)-spl.bin
endif

ifneq ($(CONFIG_TARGET_SOCFPGA_GEN5)$(CONFIG_TARGET_SOCFPGA_ARRIA10),)
INPUTS-y	+= $(obj)/$(SPL_BIN).sfp
endif

INPUTS-$(CONFIG_TARGET_SOCFPGA_SOC64) += $(obj)/u-boot-spl-dtb.hex

ifdef CONFIG_ARCH_SUNXI
INPUTS-y	+= $(obj)/sunxi-spl.bin

ifdef CONFIG_NAND_SUNXI
INPUTS-y	+= $(obj)/sunxi-spl-with-ecc.bin
endif
endif

ifeq ($(CONFIG_SYS_SOC),"at91")
INPUTS-y	+= $(obj)/boot.bin
endif

ifndef CONFIG_VPL_BUILD
ifdef CONFIG_TPL_BUILD
INPUTS-$(CONFIG_TPL_X86_16BIT_INIT) += $(obj)/u-boot-x86-start16-tpl.bin \
	$(obj)/u-boot-x86-reset16-tpl.bin
else
INPUTS-$(CONFIG_SPL_X86_16BIT_INIT) += $(obj)/u-boot-x86-start16-spl.bin \
	$(obj)/u-boot-x86-reset16-spl.bin
endif
endif

INPUTS-$(CONFIG_ARCH_ZYNQ)		+= $(obj)/boot.bin
INPUTS-$(CONFIG_ARCH_ZYNQMP)	+= $(obj)/boot.bin

INPUTS-$(CONFIG_ARCH_MEDIATEK)	+= $(obj)/u-boot-spl-mtk.bin

all:	$(INPUTS-y)

quiet_cmd_cat = CAT     $@
cmd_cat = cat $(filter-out $(PHONY), $^) > $@

quiet_cmd_copy = COPY    $@
      cmd_copy = cp $< $@

ifneq ($(CONFIG_SPL_MULTI_DTB_FIT),y)
FINAL_DTB_CONTAINER = $(obj)/$(SPL_BIN).dtb
else ifeq ($(CONFIG_SPL_MULTI_DTB_FIT_LZO),y)
FINAL_DTB_CONTAINER = $(obj)/$(SPL_BIN).multidtb.fit.lzo
else ifeq ($(CONFIG_SPL_MULTI_DTB_FIT_GZIP),y)
FINAL_DTB_CONTAINER = $(obj)/$(SPL_BIN).multidtb.fit.gz
else
FINAL_DTB_CONTAINER = $(obj)/$(SPL_BIN).multidtb.fit
endif

# Build the .dtb file if needed
#   - OF_REAL is enabled
#   - we have either OF_SEPARATE or OF_HOSTFILE
build_dtb :=
ifneq ($(CONFIG_$(PHASE_)OF_REAL),)
ifneq ($(CONFIG_OF_SEPARATE)$(CONFIG_SANDBOX),)
build_dtb := y
endif
endif

ifneq ($(build_dtb),)
$(obj)/$(SPL_BIN)-dtb.bin: $(obj)/$(SPL_BIN)-nodtb.bin \
		$(if $(CONFIG_$(PHASE_)SEPARATE_BSS),,$(obj)/$(SPL_BIN)-pad.bin) \
		$(FINAL_DTB_CONTAINER)  FORCE
	$(call if_changed,cat)

$(obj)/$(SPL_BIN).bin: $(obj)/$(SPL_BIN)-dtb.bin FORCE
	$(call if_changed,copy)
else
$(obj)/$(SPL_BIN).bin: $(obj)/$(SPL_BIN)-nodtb.bin FORCE
	$(call if_changed,copy)
endif

# Create a file that pads from the end of u-boot-spl-nodtb.bin to bss_end
$(obj)/$(SPL_BIN)-pad.bin: $(obj)/$(SPL_BIN)
	@bss_size_str=$(shell $(NM) $< | awk 'BEGIN {size = 0} /__bss_size/ {size = $$1} END {print "ibase=16; " toupper(size)}' | bc); \
	dd if=/dev/zero of=$@ bs=1 count=$${bss_size_str} 2>/dev/null;

$(obj)/$(SPL_BIN).dtb: $(obj)/dts/dt-$(SPL_NAME).dtb FORCE
	$(call if_changed,copy)

pythonpath = PYTHONPATH=scripts/dtc/pylibfdt

DTOC_ARGS := $(pythonpath) $(srctree)/tools/dtoc/dtoc \
	-d $(obj)/$(SPL_BIN).dtb -p $(SPL_NAME)

ifneq ($(CONFIG_$(PHASE_)OF_PLATDATA_INST),)
DTOC_ARGS += -i
endif

quiet_cmd_dtoc = DTOC    $@
cmd_dtoc = $(DTOC_ARGS) -c $(obj)/dts -C include/generated all

quiet_cmd_plat = PLAT    $@
cmd_plat = $(CC) $(c_flags) -c $< -o $(filter-out $(PHONY),$@)

$(obj)/dts/dt-%.o: $(obj)/dts/dt-%.c $(platdata-hdr)
	$(call if_changed,plat)

# Don't use dts_dir here, since it forces running this expensive rule every time
$(platdata-hdr) $(u-boot-spl-platdata_c) &: $(obj)/$(SPL_BIN).dtb FORCE
	@[ -d $(obj)/dts ] || mkdir -p $(obj)/dts
	@# Remove old files since which ones we generate depends on the setting
	@# of OF_PLATDATA_INST and this might change between builds. Leaving old
	@# ones around is confusing and it is possible that switching the
	@# setting again will use the old one instead of regenerating it.
	@rm -f $(u-boot-spl-old-platdata_c) $(u-boot-spl-platdata_c) \
		$(u-boot-spl-old-platdata)
	$(call if_changed,dtoc)

ifneq ($(CONFIG_ARCH_EXYNOS)$(CONFIG_ARCH_S5PC1XX),)
ifeq ($(CONFIG_EXYNOS5420),y)
VAR_SIZE_PARAM = --vs
else
VAR_SIZE_PARAM =
endif
$(obj)/$(BOARD)-spl.bin: $(obj)/u-boot-spl.bin
	$(if $(wildcard $(objtree)/spl/board/samsung/$(BOARD)/tools/mk$(BOARD)spl),\
	$(objtree)/spl/board/samsung/$(BOARD)/tools/mk$(BOARD)spl,\
	$(objtree)/tools/mkexynosspl) $(VAR_SIZE_PARAM) $< $@
endif

$(obj)/u-boot-spl.ldr: $(obj)/u-boot-spl
	$(CREATE_LDR_ENV)
	$(LDR) -T $(CONFIG_LDR_CPU) -c $@ $< $(LDR_FLAGS)
	$(BOARD_SIZE_CHECK)

quiet_cmd_objcopy = OBJCOPY $@
cmd_objcopy = $(OBJCOPY) $(OBJCOPYFLAGS) $(OBJCOPYFLAGS_$(@F)) $< $@

OBJCOPYFLAGS_$(SPL_BIN)-nodtb.bin = $(SPL_OBJCFLAGS) -O binary \
		$(if $(CONFIG_$(PHASE_)X86_16BIT_INIT),-R .start16 -R .resetvec)

$(obj)/$(SPL_BIN)-nodtb.bin: $(obj)/$(SPL_BIN) FORCE
	$(call if_changed,objcopy)

OBJCOPYFLAGS_u-boot-x86-start16-spl.bin := -O binary -j .start16
$(obj)/u-boot-x86-start16-spl.bin: $(obj)/u-boot-spl FORCE
	$(call if_changed,objcopy)

OBJCOPYFLAGS_u-boot-x86-start16-tpl.bin := -O binary -j .start16
$(obj)/u-boot-x86-start16-tpl.bin: $(obj)/u-boot-tpl FORCE
	$(call if_changed,objcopy)

OBJCOPYFLAGS_u-boot-x86-reset16-spl.bin := -O binary -j .resetvec
$(obj)/u-boot-x86-reset16-spl.bin: $(obj)/u-boot-spl FORCE
	$(call if_changed,objcopy)

OBJCOPYFLAGS_u-boot-x86-reset16-tpl.bin := -O binary -j .resetvec
$(obj)/u-boot-x86-reset16-tpl.bin: $(obj)/u-boot-tpl FORCE
	$(call if_changed,objcopy)

LDFLAGS_$(SPL_BIN) += -T u-boot-spl.lds $(LDFLAGS_FINAL)

# Avoid 'Not enough room for program headers' error on binutils 2.28 onwards.
LDFLAGS_$(SPL_BIN) += $(call ld-option, --no-dynamic-linker)

LDFLAGS_$(SPL_BIN) += --build-id=none

# Pick the best match (e.g. SPL_TEXT_BASE for SPL, TPL_TEXT_BASE for TPL)
ifneq ($(CONFIG_$(PHASE_)TEXT_BASE),)
LDFLAGS_$(SPL_BIN) += -Ttext $(CONFIG_$(PHASE_)TEXT_BASE)
endif

ifdef CONFIG_TARGET_SOCFPGA_ARRIA10
MKIMAGEFLAGS_$(SPL_BIN).sfp = -T socfpgaimage_v1
else
MKIMAGEFLAGS_$(SPL_BIN).sfp = -T socfpgaimage
endif
$(obj)/$(SPL_BIN).sfp: $(obj)/$(SPL_BIN).bin FORCE
	$(call if_changed,mkimage)

MKIMAGEFLAGS_sunxi-spl.bin = \
	-A $(ARCH) \
	-T $(CONFIG_SPL_IMAGE_TYPE) \
	-a $(CONFIG_SPL_TEXT_BASE) \
	-n $(CONFIG_DEFAULT_DEVICE_TREE)

OBJCOPYFLAGS_u-boot-spl-dtb.hex := -I binary -O ihex --change-address=$(CONFIG_SPL_TEXT_BASE)

$(obj)/u-boot-spl-dtb.hex: $(obj)/u-boot-spl-dtb.bin FORCE
	$(call if_changed,objcopy)

$(obj)/sunxi-spl.bin: $(obj)/$(SPL_BIN).bin FORCE
	$(call if_changed,mkimage)

quiet_cmd_sunxi_spl_image_builder = SUNXI_SPL_IMAGE_BUILDER $@
cmd_sunxi_spl_image_builder = $(objtree)/tools/sunxi-spl-image-builder \
				-c $(CONFIG_NAND_SUNXI_SPL_ECC_STRENGTH)/$(CONFIG_NAND_SUNXI_SPL_ECC_SIZE) \
				-p $(CONFIG_SYS_NAND_PAGE_SIZE) \
				-o $(CONFIG_SYS_NAND_OOBSIZE) \
				-u $(CONFIG_NAND_SUNXI_SPL_USABLE_PAGE_SIZE) \
				-e $(CONFIG_SYS_NAND_BLOCK_SIZE) \
				-s -b $< $@
$(obj)/sunxi-spl-with-ecc.bin: $(obj)/sunxi-spl.bin
	$(call if_changed,sunxi_spl_image_builder)


# MediaTek's specific SPL build
MKIMAGEFLAGS_u-boot-spl-mtk.bin = -T mtk_image \
	-a $(CONFIG_SPL_TEXT_BASE) -e $(CONFIG_SPL_TEXT_BASE) \
	-n "$(patsubst "%",%,$(CONFIG_MTK_BROM_HEADER_INFO))"

$(obj)/u-boot-spl-mtk.bin: $(obj)/u-boot-spl.bin FORCE
	$(call if_changed,mkimage)

quiet_cmd_sym ?= SYM     $@
      cmd_sym ?= $(OBJDUMP) -t $< > $@
$(obj)/$(SPL_BIN).sym: $(obj)/$(SPL_BIN) FORCE
	$(call if_changed,sym)

# Generate linker list symbols references to force compiler to not optimize
# them away when compiling with LTO
ifdef CONFIG_LTO
u-boot-spl-keep-syms-lto := $(obj)/keep-syms-lto.o
u-boot-spl-keep-syms-lto_c := \
	$(patsubst $(obj)/%.o,$(obj)/%.c,$(u-boot-spl-keep-syms-lto))

quiet_cmd_keep_syms_lto = KSL     $@
      cmd_keep_syms_lto = \
	$(srctree)/scripts/gen_ll_addressable_symbols.sh $(NM) $^ > $@

quiet_cmd_keep_syms_lto_cc = KSLCC   $@
      cmd_keep_syms_lto_cc = \
	$(CC) $(filter-out $(LTO_CFLAGS),$(c_flags)) -c -o $@ $<

$(u-boot-spl-keep-syms-lto_c): $(u-boot-spl-main) $(u-boot-spl-platdata)
	$(call if_changed,keep_syms_lto)
$(u-boot-spl-keep-syms-lto): $(u-boot-spl-keep-syms-lto_c)
	$(call if_changed,keep_syms_lto_cc)
else
u-boot-spl-keep-syms-lto :=
endif

# Rule to link u-boot-spl
# May be overridden by arch/$(ARCH)/config.mk
ifeq ($(LTO_ENABLE),y)
quiet_cmd_u-boot-spl ?= LTO     $@
      cmd_u-boot-spl ?= \
	(									\
		cd $(obj) &&							\
		touch $(patsubst $(obj)/%,%,$(u-boot-spl-main)) &&		\
		$(CC) -nostdlib -nostartfiles $(LTO_FINAL_LDFLAGS) $(c_flags)	\
		$(KBUILD_LDFLAGS:%=-Wl,%) $(LDFLAGS_$(@F):%=-Wl,%)		\
		$(patsubst $(obj)/%,%,$(u-boot-spl-init))			\
		-Wl,--whole-archive						\
			$(patsubst $(obj)/%,%,$(u-boot-spl-main))		\
			$(patsubst $(obj)/%,%,$(u-boot-spl-platdata))		\
			$(patsubst $(obj)/%,%,$(u-boot-spl-keep-syms-lto))	\
			$(PLATFORM_LIBS)					\
		-Wl,--no-whole-archive						\
		-Wl,-Map,$(SPL_BIN).map -o $(SPL_BIN)				\
	)
else
quiet_cmd_u-boot-spl ?= LD      $@
      cmd_u-boot-spl ?= \
	(								\
		cd $(obj) &&						\
		touch $(patsubst $(obj)/%,%,$(u-boot-spl-main)) &&	\
		$(LD) $(KBUILD_LDFLAGS) $(LDFLAGS_$(@F))		\
		$(patsubst $(obj)/%,%,$(u-boot-spl-init))		\
		--whole-archive						\
			$(patsubst $(obj)/%,%,$(u-boot-spl-main))	\
			$(patsubst $(obj)/%,%,$(u-boot-spl-platdata))	\
		--no-whole-archive					\
		$(PLATFORM_LIBS) -Map $(SPL_BIN).map -o $(SPL_BIN)	\
	)
endif

$(obj)/$(SPL_BIN): $(u-boot-spl-platdata) $(u-boot-spl-init) \
		$(u-boot-spl-main) $(u-boot-spl-keep-syms-lto) \
		$(obj)/u-boot-spl.lds FORCE
	$(call if_changed,u-boot-spl)

$(sort $(u-boot-spl-init) $(u-boot-spl-main)): $(u-boot-spl-dirs) ;

PHONY += $(u-boot-spl-dirs)
$(u-boot-spl-dirs): $(u-boot-spl-platdata) prepare
	$(Q)$(MAKE) $(build)=$@

PHONY += prepare
prepare:
	$(Q)$(MAKE) $(build)=$(obj)/.

quiet_cmd_cpp_lds = LDS     $@
cmd_cpp_lds = $(CPP) -Wp,-MD,$(depfile) $(cpp_flags) $(LDPPFLAGS) -ansi \
		-D__ASSEMBLY__ -x assembler-with-cpp -std=c99 -P -o $@ $<

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

$(obj)/dts/dt-$(SPL_NAME).dtb: dts/dt.dtb
	$(Q)$(MAKE) $(build)=$(obj)/dts spl_dtbs

ifeq ($(CONFIG_OF_UPSTREAM),y)
ifeq ($(CONFIG_ARM64),y)
dt_dir := dts/upstream/src/arm64
else
dt_dir := dts/upstream/src/$(ARCH)
endif
else
dt_dir := arch/$(ARCH)/dts
endif

# Declare the contents of the .PHONY variable as phony.  We keep that
# information in a variable so we can use it in if_changed and friends.
.PHONY: $(PHONY)

SPL_OF_LIST_TARGETS = $(patsubst %,dts/%.dtb,$(subst ",,$(CONFIG_SPL_OF_LIST)))
SHRUNK_ARCH_DTB = $(addprefix $(obj)/,$(SPL_OF_LIST_TARGETS))
$(sort $(dir $(SHRUNK_ARCH_DTB))):
	$(shell [ -d $@ ] || mkdir -p $@)

.SECONDEXPANSION:
$(SHRUNK_ARCH_DTB): $$(patsubst $(obj)/dts/%, $(dt_dir)/%, $$@) $(dir $(SHRUNK_ARCH_DTB))
	$(call if_changed,fdtgrep)

targets += $(SPL_OF_LIST_TARGETS)

MKIMAGEFLAGS_$(SPL_BIN).multidtb.fit = -f auto -A $(ARCH) -T firmware -C none -O u-boot \
	-n "Multi DTB fit image for $(SPL_BIN)" -E \
	$(patsubst %,-b %,$(SHRUNK_ARCH_DTB))

$(obj)/$(SPL_BIN).multidtb.fit: /dev/null $(SHRUNK_ARCH_DTB) FORCE
	$(call if_changed,mkimage)
ifneq ($(SOURCE_DATE_EPOCH),)
	touch -d @$(SOURCE_DATE_EPOCH) $(obj)/$(SPL_BIN).multidtb.fit
	chmod 0600 $(obj)/$(SPL_BIN).multidtb.fit
endif

$(obj)/$(SPL_BIN).multidtb.fit.gz: $(obj)/$(SPL_BIN).multidtb.fit
	@gzip -kf9 $< > $@

$(obj)/$(SPL_BIN).multidtb.fit.lzo: $(obj)/$(SPL_BIN).multidtb.fit
	@lzop -f9 $< > $@

ifdef CONFIG_ARCH_K3
tispl.bin: $(obj)/u-boot-spl-nodtb.bin $(SHRUNK_ARCH_DTB) $(SPL_ITS) FORCE
	$(call if_changed,mkfitimage)
endif
