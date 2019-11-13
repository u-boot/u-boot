# SPDX-License-Identifier: GPL-2.0+
#
# Makefile for asymmetric cryptographic keys
#

obj-$(CONFIG_ASYMMETRIC_KEY_TYPE) += asymmetric_keys.o

asymmetric_keys-y := asymmetric_type.o

obj-$(CONFIG_ASYMMETRIC_PUBLIC_KEY_SUBTYPE) += public_key.o

#
# RSA public key parser
#
obj-$(CONFIG_RSA_PUBLIC_KEY_PARSER) += rsa_public_key.o
rsa_public_key-y := \
	rsapubkey.asn1.o \
	rsa_helper.o

$(obj)/rsapubkey.asn1.o: $(obj)/rsapubkey.asn1.c $(obj)/rsapubkey.asn1.h
$(obj)/rsa_helper.o: $(obj)/rsapubkey.asn1.h
