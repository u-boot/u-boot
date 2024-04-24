# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2023 Linaro Limited
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
	$(MBEDTLS_LIB_DIR)/aes.o \
	$(MBEDTLS_LIB_DIR)/aesni.o \
	$(MBEDTLS_LIB_DIR)/aesce.o \
	$(MBEDTLS_LIB_DIR)/aria.o \
	$(MBEDTLS_LIB_DIR)/asn1parse.o \
	$(MBEDTLS_LIB_DIR)/asn1write.o \
	$(MBEDTLS_LIB_DIR)/base64.o \
	$(MBEDTLS_LIB_DIR)/bignum.o \
	$(MBEDTLS_LIB_DIR)/bignum_core.o \
	$(MBEDTLS_LIB_DIR)/bignum_mod.o \
	$(MBEDTLS_LIB_DIR)/bignum_mod_raw.o \
	$(MBEDTLS_LIB_DIR)/camellia.o \
	$(MBEDTLS_LIB_DIR)/ccm.o \
	$(MBEDTLS_LIB_DIR)/chacha20.o \
	$(MBEDTLS_LIB_DIR)/chachapoly.o \
	$(MBEDTLS_LIB_DIR)/cipher.o \
	$(MBEDTLS_LIB_DIR)/cipher_wrap.o \
	$(MBEDTLS_LIB_DIR)/cmac.o \
	$(MBEDTLS_LIB_DIR)/constant_time.o \
	$(MBEDTLS_LIB_DIR)/ctr_drbg.o \
	$(MBEDTLS_LIB_DIR)/des.o \
	$(MBEDTLS_LIB_DIR)/dhm.o \
	$(MBEDTLS_LIB_DIR)/ecdh.o \
	$(MBEDTLS_LIB_DIR)/ecdsa.o \
	$(MBEDTLS_LIB_DIR)/ecjpake.o \
	$(MBEDTLS_LIB_DIR)/ecp.o \
	$(MBEDTLS_LIB_DIR)/ecp_curves.o \
	$(MBEDTLS_LIB_DIR)/ecp_curves_new.o \
	$(MBEDTLS_LIB_DIR)/entropy.o \
	$(MBEDTLS_LIB_DIR)/entropy_poll.o \
	$(MBEDTLS_LIB_DIR)/error.o \
	$(MBEDTLS_LIB_DIR)/gcm.o \
	$(MBEDTLS_LIB_DIR)/hkdf.o \
	$(MBEDTLS_LIB_DIR)/hmac_drbg.o \
	$(MBEDTLS_LIB_DIR)/lmots.o \
	$(MBEDTLS_LIB_DIR)/lms.o \
	$(MBEDTLS_LIB_DIR)/md.o \
	$(MBEDTLS_LIB_DIR)/md5.o \
	$(MBEDTLS_LIB_DIR)/memory_buffer_alloc.o \
	$(MBEDTLS_LIB_DIR)/nist_kw.o \
	$(MBEDTLS_LIB_DIR)/oid.o \
	$(MBEDTLS_LIB_DIR)/padlock.o \
	$(MBEDTLS_LIB_DIR)/pem.o \
	$(MBEDTLS_LIB_DIR)/pk.o \
	$(MBEDTLS_LIB_DIR)/pk_wrap.o \
	$(MBEDTLS_LIB_DIR)/pkcs12.o \
	$(MBEDTLS_LIB_DIR)/pkcs5.o \
	$(MBEDTLS_LIB_DIR)/pkparse.o \
	$(MBEDTLS_LIB_DIR)/pkwrite.o \
	$(MBEDTLS_LIB_DIR)/platform.o \
	$(MBEDTLS_LIB_DIR)/platform_util.o \
	$(MBEDTLS_LIB_DIR)/poly1305.o \
	$(MBEDTLS_LIB_DIR)/ripemd160.o \
	$(MBEDTLS_LIB_DIR)/rsa.o \
	$(MBEDTLS_LIB_DIR)/rsa_alt_helpers.o \
	$(MBEDTLS_LIB_DIR)/sha1.o \
	$(MBEDTLS_LIB_DIR)/sha256.o \
	$(MBEDTLS_LIB_DIR)/sha512.o \
	$(MBEDTLS_LIB_DIR)/sha3.o \
	$(MBEDTLS_LIB_DIR)/threading.o \
	$(MBEDTLS_LIB_DIR)/timing.o \
	$(MBEDTLS_LIB_DIR)/version.o \
	$(MBEDTLS_LIB_DIR)/version_features.o \
	# This line is intentionally left blank

obj-$(CONFIG_MBEDTLS_LIB_X509) += mbedtls_lib_x509.o
mbedtls_lib_x509-y := \
	$(MBEDTLS_LIB_DIR)/x509.o \
	$(MBEDTLS_LIB_DIR)/x509_create.o \
	$(MBEDTLS_LIB_DIR)/x509_crl.o \
	$(MBEDTLS_LIB_DIR)/x509_crt.o \
	$(MBEDTLS_LIB_DIR)/x509_csr.o \
	$(MBEDTLS_LIB_DIR)/x509write.o \
	$(MBEDTLS_LIB_DIR)/x509write_crt.o \
	$(MBEDTLS_LIB_DIR)/x509write_csr.o \
	$(MBEDTLS_LIB_DIR)/pkcs7.o \
	# This line is intentionally left blank

obj-$(CONFIG_MBEDTLS_LIB_TLS) += mbedtls_lib_tls.o
mbedtls_lib_tls-y := \
	$(MBEDTLS_LIB_DIR)/debug.o \
	$(MBEDTLS_LIB_DIR)/mps_reader.o \
	$(MBEDTLS_LIB_DIR)/mps_trace.o \
	$(MBEDTLS_LIB_DIR)/net_sockets.o \
	$(MBEDTLS_LIB_DIR)/ssl_cache.o \
	$(MBEDTLS_LIB_DIR)/ssl_ciphersuites.o \
	$(MBEDTLS_LIB_DIR)/ssl_client.o \
	$(MBEDTLS_LIB_DIR)/ssl_cookie.o \
	$(MBEDTLS_LIB_DIR)/ssl_debug_helpers_generated.o \
	$(MBEDTLS_LIB_DIR)/ssl_msg.o \
	$(MBEDTLS_LIB_DIR)/ssl_ticket.o \
	$(MBEDTLS_LIB_DIR)/ssl_tls.o \
	$(MBEDTLS_LIB_DIR)/ssl_tls12_client.o \
	$(MBEDTLS_LIB_DIR)/ssl_tls12_server.o \
	$(MBEDTLS_LIB_DIR)/ssl_tls13_keys.o \
	$(MBEDTLS_LIB_DIR)/ssl_tls13_client.o \
	$(MBEDTLS_LIB_DIR)/ssl_tls13_server.o \
	$(MBEDTLS_LIB_DIR)/ssl_tls13_generic.o \
	# This line is intentionally left blank
