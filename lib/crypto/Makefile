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

#
# X.509 Certificate handling
#
obj-$(CONFIG_X509_CERTIFICATE_PARSER) += x509_key_parser.o
x509_key_parser-y := \
	x509.asn1.o \
	x509_akid.asn1.o \
	x509_cert_parser.o \
	x509_public_key.o

$(obj)/x509_cert_parser.o: \
	$(obj)/x509.asn1.h \
	$(obj)/x509_akid.asn1.h

$(obj)/x509.asn1.o: $(obj)/x509.asn1.c $(obj)/x509.asn1.h
$(obj)/x509_akid.asn1.o: $(obj)/x509_akid.asn1.c $(obj)/x509_akid.asn1.h

#
# PKCS#7 message handling
#
obj-$(CONFIG_PKCS7_MESSAGE_PARSER) += pkcs7_message.o
pkcs7_message-y := \
	pkcs7.asn1.o \
	pkcs7_parser.o

$(obj)/pkcs7_parser.o: $(obj)/pkcs7.asn1.h
$(obj)/pkcs7.asn1.o: $(obj)/pkcs7.asn1.c $(obj)/pkcs7.asn1.h
