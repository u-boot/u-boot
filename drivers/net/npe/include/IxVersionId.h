/**
 * @file IxVersionId.h
 *
 * @date 22-Aug-2002
 *
 * @brief This file contains the IXP400 Software version identifier
 *
 * 
 * @par
 * IXP400 SW Release version 2.0
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright 2001-2005, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * @par
 * -- End of Copyright Notice --
 */

/**
 * @defgroup IxVersionId IXP400 Version ID (IxVersionId)
 *
 * @brief Version Identifiers
 *
 * @{
 */

#ifndef IXVERSIONID_H
#define IXVERSIONID_H

/**
 * @brief Version Identifier String
 *
 * This string will be updated with each customer release of the IXP400
 * Software.
 */
#define IX_VERSION_ID "2_0"

/**
 * This string will be updated with each customer release of the IXP400
 * ADSL driver package.
 */
#define IX_VERSION_ADSL_ID "1_12"


/**
 * This string will be updated with each customer release of the IXP400
 * USB Client driver package.
 */
#define IX_VERSION_USBRNDIS_ID "1_9"

/**
 * This string will be updated with each customer release of the IXP400
 * I2C Linux driver package.
 */
#define IX_VERSION_I2C_LINUX_ID "1_0"

/**
 * @brief Linux Ethernet Driver Patch Version Identifier String
 *
 * This string will be updated with each release of Linux Ethernet Patch
 */
#define LINUX_ETHERNET_DRIVER_PATCH_ID "1_4"

/**
 * @brief Linux Integration Patch Version Identifier String
 *
 * This String will be updated with each release of Linux Integration Patch
 */
#define LINUX_INTEGRATION_PATCH_ID "1_3"

/**
 * @brief Linux Ethernet Readme version Identifier String
 *
 * This string will be updated with each release of Linux Ethernet Readme
 */
#define LINUX_ETHERNET_README_ID "1_3"

/**
 * @brief Linux Integration Readme version Identifier String
 *
 * This string will be updated with each release of Linux Integration Readme
 */

#define LINUX_INTEGRATION_README_ID "1_3"

/**
 * @brief Linux I2C driver Readme version Identifier String
 *
 * This string will be updated with each release of Linux I2C Driver Readme
 */
#define LINUX_I2C_DRIVER_README_ID "1_0"

/**
 * @brief ixp425_eth_update_nf_bridge.patch version Identifier String
 *
 * This string will be updated with each release of ixp425_eth_update_nf_bridge.
patch
 *
 */

#define IXP425_ETH_UPDATE_NF_BRIDGE_ID "1_3"

/**
 * @brief Internal Release Identifier String
 *
 * This string will be updated with each internal release (SQA drop)
 * of the IXP400 Software.
 */
#define IX_VERSION_INTERNAL_ID "SQA3_5"

/**
 * @brief Compatible Tornado Version Identifier
 */
#define IX_VERSION_COMPATIBLE_TORNADO "Tornado2_2_1-PNE2_0"

/**
 * @brief Compatible Linux Version Identifier
 */
#define IX_VERSION_COMPATIBLE_LINUX "MVL3_1"


#endif /* IXVERSIONID_H */

/**
 * @} addtogroup IxVersionId
 */
