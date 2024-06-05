# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2024 Linaro Limited
# Author: Raymond Mao <raymond.mao@linaro.org>

MBEDTLS_LIB_DIR = external/mbedtls/library

# mbedtls default config file
ccflags-y += "-DMBEDTLS_CONFIG_FILE=\"mbedtls_def_config.h\""

ccflags-y += \
	-I$(src)/port \
	-I$(src)/external/mbedtls/include \
	-I$(src)/external/mbedtls/library \
	# This line is intentionally left blank

# shim layer for hash
obj-$(CONFIG_MBEDTLS_LIB_CRYPTO) += hash_mbedtls.o
hash_mbedtls-$(CONFIG_$(SPL_)MD5) += md5.o
hash_mbedtls-$(CONFIG_$(SPL_)SHA1) += sha1.o
hash_mbedtls-$(CONFIG_$(SPL_)SHA256) += sha256.o
hash_mbedtls-$(CONFIG_$(SPL_)SHA512) += sha512.o

# x509 libraries
obj-$(CONFIG_MBEDTLS_LIB_X509) += x509_mbedtls.o
x509_mbedtls-$(CONFIG_$(SPL_)ASYMMETRIC_PUBLIC_KEY_SUBTYPE) += public_key.o
x509_mbedtls-$(CONFIG_$(SPL_)X509_CERTIFICATE_PARSER) += x509_cert_parser.o
x509_mbedtls-$(CONFIG_$(SPL_)PKCS7_MESSAGE_PARSER) += pkcs7_parser.o
x509_mbedtls-$(CONFIG_$(SPL_)MSCODE_PARSER) += mscode_parser.o
x509_mbedtls-$(CONFIG_$(SPL_)RSA_PUBLIC_KEY_PARSER) += rsa_helper.o

obj-$(CONFIG_MBEDTLS_LIB_CRYPTO) += mbedtls_lib_crypto.o
mbedtls_lib_crypto-y := \
	$(MBEDTLS_LIB_DIR)/asn1parse.o \
	$(MBEDTLS_LIB_DIR)/asn1write.o \
	$(MBEDTLS_LIB_DIR)/bignum.o \
	$(MBEDTLS_LIB_DIR)/bignum_core.o \
	$(MBEDTLS_LIB_DIR)/constant_time.o \
	$(MBEDTLS_LIB_DIR)/md.o \
	$(MBEDTLS_LIB_DIR)/md5.o \
	$(MBEDTLS_LIB_DIR)/oid.o \
	$(MBEDTLS_LIB_DIR)/pk.o \
	$(MBEDTLS_LIB_DIR)/pk_wrap.o \
	$(MBEDTLS_LIB_DIR)/pkparse.o \
	$(MBEDTLS_LIB_DIR)/platform_util.o \
	$(MBEDTLS_LIB_DIR)/rsa.o \
	$(MBEDTLS_LIB_DIR)/rsa_alt_helpers.o \
	$(MBEDTLS_LIB_DIR)/sha1.o \
	$(MBEDTLS_LIB_DIR)/sha256.o \
	$(MBEDTLS_LIB_DIR)/sha512.o

obj-$(CONFIG_MBEDTLS_LIB_X509) += mbedtls_lib_x509.o
mbedtls_lib_x509-y := \
	$(MBEDTLS_LIB_DIR)/x509.o \
	$(MBEDTLS_LIB_DIR)/x509_crl.o \
	$(MBEDTLS_LIB_DIR)/x509_crt.o \
	$(MBEDTLS_LIB_DIR)/pkcs7.o
