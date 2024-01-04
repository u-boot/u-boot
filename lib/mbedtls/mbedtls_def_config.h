/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * MbedTLS config file
 *
 * Derived from the MbedTLS internal config file,
 * for more information about each build option,
 * please refer to:
 * external/mbedtls/include/mbedtls/mbedtls_config.h
 *
 * Copyright (c) 2024 Linaro Limited
 * Author: Raymond Mao <raymond.mao@linaro.org>
 */

#if defined CONFIG_MBEDTLS_LIB_CRYPTO

#if CONFIG_IS_ENABLED(MD5)
#define MBEDTLS_MD_C
#define MBEDTLS_MD5_C
#endif

#if CONFIG_IS_ENABLED(SHA1)
#define MBEDTLS_MD_C
#define MBEDTLS_SHA1_C
#endif

#if CONFIG_IS_ENABLED(SHA256)
#define MBEDTLS_MD_C
#define MBEDTLS_SHA256_C
#endif

#if CONFIG_IS_ENABLED(SHA384)
#define MBEDTLS_MD_C
#define MBEDTLS_SHA384_C
#endif

#if CONFIG_IS_ENABLED(SHA512)
#define MBEDTLS_MD_C
#define MBEDTLS_SHA512_C
#endif

#endif /* #if defined CONFIG_MBEDTLS_LIB_CRYPTO */

#if defined CONFIG_MBEDTLS_LIB_X509

#if CONFIG_IS_ENABLED(X509_CERTIFICATE_PARSER)
#define MBEDTLS_X509_USE_C
#define MBEDTLS_X509_CRT_PARSE_C
#define MBEDTLS_X509_CRL_PARSE_C
#endif

#if CONFIG_IS_ENABLED(ASYMMETRIC_PUBLIC_KEY_SUBTYPE)
#define MBEDTLS_PK_C
#define MBEDTLS_PK_PARSE_C
#endif

#if CONFIG_IS_ENABLED(RSA_PUBLIC_KEY_PARSER)
#define MBEDTLS_BIGNUM_C
#define MBEDTLS_RSA_C
#define MBEDTLS_PKCS1_V15
#endif

#if CONFIG_IS_ENABLED(PKCS7_MESSAGE_PARSER)
#define MBEDTLS_PKCS7_C
#endif

#if CONFIG_IS_ENABLED(ASN1_DECODER)
#define MBEDTLS_OID_C
#define MBEDTLS_ASN1_PARSE_C
#define MBEDTLS_ASN1_WRITE_C
#endif

#endif /* #if defined CONFIG_MBEDTLS_LIB_X509 */
