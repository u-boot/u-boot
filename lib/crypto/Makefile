# SPDX-License-Identifier: GPL-2.0+
#
# Makefile for asymmetric cryptographic keys
#

obj-$(CONFIG_$(PHASE_)ASYMMETRIC_KEY_TYPE) += asymmetric_keys.o

asymmetric_keys-y := asymmetric_type.o

obj-$(CONFIG_$(PHASE_)ASYMMETRIC_PUBLIC_KEY_SUBTYPE) += public_key_helper.o
obj-$(CONFIG_$(PHASE_)ASYMMETRIC_PUBLIC_KEY_LEGACY) += public_key.o

#
# RSA public key parser
#
obj-$(CONFIG_$(PHASE_)RSA_PUBLIC_KEY_PARSER_LEGACY) += rsa_public_key.o
rsa_public_key-y := \
	rsapubkey.asn1.o \
	rsa_helper.o

$(obj)/rsapubkey.asn1.o: $(obj)/rsapubkey.asn1.c $(obj)/rsapubkey.asn1.h
ifdef CONFIG_XPL_BUILD
CFLAGS_rsapubkey.asn1.o += -I$(obj)
endif

$(obj)/rsa_helper.o: $(obj)/rsapubkey.asn1.h
ifdef CONFIG_XPL_BUILD
CFLAGS_rsa_helper.o += -I$(obj)
endif

#
# X.509 Certificate handling
#
obj-$(CONFIG_$(PHASE_)X509_CERTIFICATE_PARSER) += x509_key_parser.o
x509_key_parser-y := x509_helper.o
x509_key_parser-$(CONFIG_$(PHASE_)X509_CERTIFICATE_PARSER_LEGACY) += \
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
obj-$(CONFIG_$(PHASE_)PKCS7_MESSAGE_PARSER) += pkcs7_message.o
pkcs7_message-y := pkcs7_helper.o
pkcs7_message-$(CONFIG_$(PHASE_)PKCS7_MESSAGE_PARSER_LEGACY) += \
	pkcs7.asn1.o \
	pkcs7_parser.o

$(obj)/pkcs7_parser.o: $(obj)/pkcs7.asn1.h
$(obj)/pkcs7.asn1.o: $(obj)/pkcs7.asn1.c $(obj)/pkcs7.asn1.h

obj-$(CONFIG_$(PHASE_)PKCS7_VERIFY) += pkcs7_verify.o

#
# Signed PE binary-wrapped key handling
#
obj-$(CONFIG_$(PHASE_)MSCODE_PARSER_LEGACY) += mscode.o

mscode-y := \
	mscode_parser.o \
	mscode.asn1.o

$(obj)/mscode_parser.o: $(obj)/mscode.asn1.h $(obj)/mscode.asn1.h
$(obj)/mscode.asn1.o: $(obj)/mscode.asn1.c $(obj)/mscode.asn1.h
