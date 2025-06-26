# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2018
# Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc

obj-$(CONFIG_$(PHASE_)UT_COMPRESSION) += compression.o

ifeq ($(CONFIG_XPL_BUILD),)
obj-y += abuf.o
obj-y += alist.o
obj-$(CONFIG_EFI_LOADER) += efi_device_path.o efi_memory.o
obj-$(CONFIG_EFI_SECURE_BOOT) += efi_image_region.o
obj-y += hexdump.o
obj-$(CONFIG_SANDBOX) += kconfig.o
obj-y += lmb.o
obj-$(CONFIG_HAVE_SETJMP) += longjmp.o
obj-$(CONFIG_SANDBOX) += membuf.o
obj-$(CONFIG_HAVE_INITJMP) += initjmp.o
obj-$(CONFIG_CONSOLE_RECORD) += test_print.o
obj-$(CONFIG_SSCANF) += sscanf.o
obj-$(CONFIG_$(PHASE_)STRTO) += str.o
obj-y += string.o
obj-y += strlcat.o
obj-$(CONFIG_ERRNO_STR) += test_errno_str.o
obj-$(CONFIG_UT_LIB_ASN1) += asn1.o
obj-$(CONFIG_UT_LIB_RSA) += rsa.o
obj-$(CONFIG_AES) += test_aes.o
obj-$(CONFIG_SHA256) += test_sha256_hmac.o
obj-$(CONFIG_HKDF_MBEDTLS) += test_sha256_hkdf.o
obj-$(CONFIG_GETOPT) += getopt.o
obj-$(CONFIG_CRC8) += test_crc8.o
obj-$(CONFIG_REGEX) += slre.o
obj-$(CONFIG_UT_LIB_CRYPT) += test_crypt.o
obj-$(CONFIG_UT_TIME) += time.o
obj-$(CONFIG_$(PHASE_)UT_UNICODE) += unicode.o
obj-$(CONFIG_UTHREAD) += uthread.o
obj-$(CONFIG_LIB_UUID) += uuid.o
else
obj-$(CONFIG_SANDBOX) += kconfig_spl.o
endif
