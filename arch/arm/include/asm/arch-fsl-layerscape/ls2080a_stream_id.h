/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 */
#ifndef __FSL_STREAM_ID_H
#define __FSL_STREAM_ID_H

/* Stream IDs on ls2080a devices are not hardwired and are
 * programmed by sw.  There are a limited number of stream IDs
 * available, and the partitioning of them is scenario dependent.
 * This header defines the partitioning between legacy, PCI,
 * and DPAA2 devices.
 *
 * This partitiong can be customized in this file depending
 * on the specific hardware config-- e.g. perhaps not all
 * PEX controllers are in use.
 *
 * On LS2080 stream IDs are programmed in AMQ registers (32-bits) for
 * each of the different bus masters.  The relationship between
 * the AMQ registers and stream IDs is defined in the table below:
 *          AMQ bit    streamID bit
 *      ---------------------------
 *           PL[18]         9
 *          BMT[17]         8
 *           VA[16]         7
 *             [15]         -
 *         ICID[14:7]       -
 *         ICID[6:0]        6-0
 *     ----------------------------
 */

#define AMQ_PL_MASK			(0x1 << 18)   /* priviledge bit */
#define AMQ_BMT_MASK			(0x1 << 17)   /* bypass bit */

#define FSL_INVALID_STREAM_ID		0

#define FSL_BYPASS_AMQ			(AMQ_PL_MASK | AMQ_BMT_MASK)

/* legacy devices */
#define FSL_USB1_STREAM_ID		1
#define FSL_USB2_STREAM_ID		2
#define FSL_SDMMC_STREAM_ID		3
#define FSL_SATA1_STREAM_ID		4
#define FSL_SATA2_STREAM_ID		5
#define FSL_DMA_STREAM_ID		6

/* PCI - programmed in PEXn_LUT by OS */
/*   4 IDs per controller */
#define FSL_PEX1_STREAM_ID_START	7
#define FSL_PEX1_STREAM_ID_END		10
#define FSL_PEX2_STREAM_ID_START	11
#define FSL_PEX2_STREAM_ID_END		14
#define FSL_PEX3_STREAM_ID_START	15
#define FSL_PEX3_STREAM_ID_END		18
#define FSL_PEX4_STREAM_ID_START	19
#define FSL_PEX4_STREAM_ID_END		22

/* DPAA2 - set in MC DPC and alloced by MC */
#define FSL_DPAA2_STREAM_ID_START	23
#define FSL_DPAA2_STREAM_ID_END		63

#endif
