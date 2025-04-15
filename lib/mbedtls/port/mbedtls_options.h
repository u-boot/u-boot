/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Internal build options for MbedTLS
 *
 * Copyright (c) 2025 Linaro Limited
 * Author: Raymond Mao <raymond.mao@linaro.org>
 */

#ifndef _MBEDTLS_OPT_H
#define _MBEDTLS_OPT_H

/*
 * FIXME:
 * U-Boot/MbedTLS port requires to access a few of members which are defined
 * as private in MbedTLS context.
 * E.g: x509_internal.h, mbedtls_sha256_context and mbedtls_sha1_context.
 * MBEDTLS_ALLOW_PRIVATE_ACCESS needs to be enabled to allow the external
 * access, but directly including <external/mbedtls/library/common.h> is not
 * allowed, since this will include <malloc.h> and break the sandbox test.
 */
#define MBEDTLS_ALLOW_PRIVATE_ACCESS

#endif /* _MBEDTLS_OPT_H */
