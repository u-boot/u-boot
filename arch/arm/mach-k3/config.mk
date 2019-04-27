# SPDX-License-Identifier:	GPL-2.0+
#
# Copyright (C) 2017-2018 Texas Instruments Incorporated - http://www.ti.com/
#	Lokesh Vutla <lokeshvutla@ti.com>

ifdef CONFIG_SPL_BUILD

# Openssl is required to generate x509 certificate.
# Error out if openssl is not available.
ifeq ($(shell which openssl),)
$(error "No openssl in $(PATH), consider installing openssl")
endif

SHA_VALUE=  $(shell openssl dgst -sha512 -hex $(obj)/u-boot-spl.bin | sed -e "s/^.*= //g")
IMAGE_SIZE= $(shell cat $(obj)/u-boot-spl.bin | wc -c)
LOADADDR= $(shell echo $(CONFIG_SPL_TEXT_BASE) | sed -e "s/^0x//g")
MAX_SIZE= $(shell printf "%d" $(CONFIG_SYS_K3_MAX_DOWNLODABLE_IMAGE_SIZE))

# Parameters to get populated into the x509 template
SED_OPTS=  -e s/TEST_IMAGE_LENGTH/$(IMAGE_SIZE)/
SED_OPTS+= -e s/TEST_IMAGE_SHA_VAL/$(SHA_VALUE)/
SED_OPTS+= -e s/TEST_CERT_TYPE/1/		# CERT_TYPE_PRIMARY_IMAGE_BIN
SED_OPTS+= -e s/TEST_BOOT_CORE/$(CONFIG_SYS_K3_BOOT_CORE_ID)/
SED_OPTS+= -e s/TEST_BOOT_ARCH_WIDTH/32/
SED_OPTS+= -e s/TEST_BOOT_ADDR/$(LOADADDR)/

# Command to generate ecparam key
quiet_cmd_genkey = OPENSSL $@
cmd_genkey = openssl ecparam -out $@ -name prime256v1 -genkey

# Command to generate x509 certificate
quiet_cmd_gencert = OPENSSL $@
cmd_gencert = cat $(srctree)/tools/k3_x509template.txt | sed $(SED_OPTS) > u-boot-spl-x509.txt; \
	openssl req -new -x509 -key $(KEY) -nodes -outform DER -out $@ -config u-boot-spl-x509.txt -sha512

# If external key is not provided, generate key using openssl.
ifeq ($(CONFIG_SYS_K3_KEY), "")
KEY=u-boot-spl-eckey.pem
# On HS use real key or warn if not available
ifeq ($(CONFIG_TI_SECURE_DEVICE),y)
ifneq ($(wildcard $(TI_SECURE_DEV_PKG)/keys/custMpk.pem),)
KEY=$(TI_SECURE_DEV_PKG)/keys/custMpk.pem
else
$(warning "WARNING: signing key not found. Random key will NOT work on HS hardware!")
endif
endif
else
KEY=$(patsubst "%",$(srctree)/%,$(CONFIG_SYS_K3_KEY))
endif

u-boot-spl-eckey.pem: FORCE
	$(call if_changed,genkey)

# tiboot3.bin is mandated by ROM and ROM only supports R5 boot.
# So restrict tiboot3.bin creation for CPU_V7R.
ifdef CONFIG_CPU_V7R
u-boot-spl-cert.bin: $(KEY) $(obj)/u-boot-spl.bin image_check FORCE
	$(call if_changed,gencert)

image_check: $(obj)/u-boot-spl.bin FORCE
	@if [ $(IMAGE_SIZE) -gt $(MAX_SIZE) ]; then			    \
		echo "===============================================" >&2; \
		echo "ERROR: Final Image too big. " >&2;		    \
		echo "$< size = $(IMAGE_SIZE), max size = $(MAX_SIZE)" >&2; \
		echo "===============================================" >&2; \
		exit 1;							    \
	fi

tiboot3.bin: u-boot-spl-cert.bin $(obj)/u-boot-spl.bin FORCE
	$(call if_changed,cat)

ALL-y	+= tiboot3.bin
endif

ifdef CONFIG_ARM64
ifeq ($(CONFIG_TI_SECURE_DEVICE),y)
SPL_ITS := u-boot-spl-k3_HS.its
$(SPL_ITS): FORCE
	IS_HS=1 \
	$(srctree)/tools/k3_fit_atf.sh \
	$(patsubst %,$(obj)/dts/%.dtb,$(subst ",,$(CONFIG_SPL_OF_LIST))) > $@

ALL-y	+= tispl.bin_HS
else
SPL_ITS := u-boot-spl-k3.its
$(SPL_ITS): FORCE
	$(srctree)/tools/k3_fit_atf.sh \
	$(patsubst %,$(obj)/dts/%.dtb,$(subst ",,$(CONFIG_SPL_OF_LIST))) > $@

ALL-y	+= tispl.bin
endif
endif

else

ifeq ($(CONFIG_TI_SECURE_DEVICE),y)
ALL-y	+= u-boot.img_HS
else
ALL-y	+= u-boot.img
endif
endif

include $(srctree)/arch/arm/mach-k3/config_secure.mk
