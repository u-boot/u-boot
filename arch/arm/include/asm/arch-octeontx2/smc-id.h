/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#ifndef __SMC_ID_H__
#define __SMC_ID_H__

/* SMC function IDs for general purpose queries */

#define OCTEONTX2_SVC_CALL_COUNT	0xc200ff00
#define OCTEONTX2_SVC_UID		0xc200ff01

#define OCTEONTX2_SVC_VERSION		0xc200ff03

/* OcteonTX Service Calls version numbers */
#define OCTEONTX2_VERSION_MAJOR	0x1
#define OCTEONTX2_VERSION_MINOR	0x0

/* x1 - node number */
#define OCTEONTX2_DRAM_SIZE		0xc2000301
#define OCTEONTX2_NODE_COUNT		0xc2000601
#define OCTEONTX2_DISABLE_RVU_LFS	0xc2000b01

#define OCTEONTX2_CONFIG_OOO		0xc2000b04

/* fail safe */
#define OCTEONTX2_FSAFE_PR_BOOT_SUCCESS	0xc2000b02
#define OCTEONTX2_FSAFE_CLR_FORCE_SEC	0xc2000b03

#define OCTEONTX2_MDIO_DBG_READ		0xc2000d01
#define OCTEONTX2_MDIO_DBG_WRITE	0xc2000d02

#define OCTEONTX_ATTESTATION_QUERY	0xc2000d03
#define OCTEONTX_ATTESTATION_QUERY_SUBCMD_LEGACY 0
#define OCTEONTX_ATTESTATION_QUERY_SUBCMD_BUFFER 1
#define OCTEONTX_ATTESTATION_QUERY_SUBCMD_INFO   2

#endif /* __SMC_ID_H__ */
