/*
 * Copyright (C) 2002 ETC s.r.o.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the ETC s.r.o. nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Written by Marcel Telka <marcel@telka.sk>, 2002.
 *
 * Documentation:
 * [1] Intel Corporation, "3 Volt Intel Strata Flash Memory 28F128J3A, 28F640J3A,
 *     28F320J3A (x8/x16)", April 2002, Order Number: 290667-011
 * [2] Intel Corporation, "3 Volt Synchronous Intel Strata Flash Memory 28F640K3, 28F640K18,
 *     28F128K3, 28F128K18, 28F256K3, 28F256K18 (x16)", June 2002, Order Number: 290737-005
 *
 * This file is taken from OpenWinCE project hosted by SourceForge.net
 *
 */

#ifndef	FLASH_INTEL_H
#define	FLASH_INTEL_H

#include <common.h>

/* Intel CFI commands - see Table 4. in [1] and Table 3. in [2] */

#define	CFI_INTEL_CMD_READ_ARRAY		0xFF	/* 28FxxxJ3A, 28FxxxK3, 28FxxxK18 */
#define	CFI_INTEL_CMD_READ_IDENTIFIER		0x90	/* 28FxxxJ3A, 28FxxxK3, 28FxxxK18 */
#define	CFI_INTEL_CMD_READ_QUERY		0x98	/* 28FxxxJ3A, 28FxxxK3, 28FxxxK18 */
#define	CFI_INTEL_CMD_READ_STATUS_REGISTER	0x70	/* 28FxxxJ3A, 28FxxxK3, 28FxxxK18 */
#define	CFI_INTEL_CMD_CLEAR_STATUS_REGISTER	0x50	/* 28FxxxJ3A, 28FxxxK3, 28FxxxK18 */
#define	CFI_INTEL_CMD_PROGRAM1			0x40	/* 28FxxxJ3A, 28FxxxK3, 28FxxxK18 */
#define	CFI_INTEL_CMD_PROGRAM2			0x10	/* 28FxxxJ3A, 28FxxxK3, 28FxxxK18 */
#define	CFI_INTEL_CMD_WRITE_TO_BUFFER		0xE8	/* 28FxxxJ3A, 28FxxxK3, 28FxxxK18 */
#define	CFI_INTEL_CMD_CONFIRM			0xD0	/* 28FxxxJ3A, 28FxxxK3, 28FxxxK18 */
#define	CFI_INTEL_CMD_BLOCK_ERASE		0x20	/* 28FxxxJ3A, 28FxxxK3, 28FxxxK18 */
#define	CFI_INTEL_CMD_SUSPEND			0xB0	/* 28FxxxJ3A, 28FxxxK3, 28FxxxK18 */
#define	CFI_INTEL_CMD_RESUME			0xD0	/* 28FxxxJ3A, 28FxxxK3, 28FxxxK18 */
#define	CFI_INTEL_CMD_LOCK_SETUP		0x60	/* 28FxxxJ3A, 28FxxxK3, 28FxxxK18 */
#define	CFI_INTEL_CMD_LOCK_BLOCK		0x01	/* 28FxxxJ3A, 28FxxxK3, 28FxxxK18 */
#define	CFI_INTEL_CMD_UNLOCK_BLOCK		0xD0	/* 28FxxxJ3A - unlocks all blocks, 28FFxxxK3, 28FxxxK18 */
#define	CFI_INTEL_CMD_LOCK_DOWN_BLOCK		0x2F	/* 28FxxxK3, 28FxxxK18 */

/* Intel CFI Status Register bits - see Table 6. in [1] and Table 7. in [2] */

#define	CFI_INTEL_SR_READY			1 << 7	/* 28FxxxJ3A, 28FxxxK3, 28FxxxK18 */
#define	CFI_INTEL_SR_ERASE_SUSPEND		1 << 6	/* 28FxxxJ3A, 28FxxxK3, 28FxxxK18 */
#define	CFI_INTEL_SR_ERASE_ERROR		1 << 5	/* 28FxxxJ3A, 28FxxxK3, 28FxxxK18 */
#define	CFI_INTEL_SR_PROGRAM_ERROR		1 << 4	/* 28FxxxJ3A, 28FxxxK3, 28FxxxK18 */
#define	CFI_INTEL_SR_VPEN_ERROR			1 << 3	/* 28FxxxJ3A, 28FxxxK3, 28FxxxK18 */
#define	CFI_INTEL_SR_PROGRAM_SUSPEND		1 << 2	/* 28FxxxJ3A, 28FxxxK3, 28FxxxK18 */
#define	CFI_INTEL_SR_BLOCK_LOCKED		1 << 1	/* 28FxxxJ3A, 28FxxxK3, 28FxxxK18 */
#define	CFI_INTEL_SR_BEFP			1 << 0	/* 28FxxxK3, 28FxxxK18 */

/* Intel flash device ID codes for 28FxxxJ3A - see Table 5. in [1] */

#define	CFI_CHIP_INTEL_28F320J3A		0x0016
#define	CFI_CHIPN_INTEL_28F320J3A		"28F320J3A"
#define	CFI_CHIP_INTEL_28F640J3A		0x0017
#define	CFI_CHIPN_INTEL_28F640J3A		"28F640J3A"
#define	CFI_CHIP_INTEL_28F128J3A		0x0018
#define	CFI_CHIPN_INTEL_28F128J3A		"28F128J3A"

/* Intel flash device ID codes for 28FxxxK3 and 28FxxxK18 - see Table 8. in [2] */

#define	CFI_CHIP_INTEL_28F640K3			0x8801
#define	CFI_CHIPN_INTEL_28F640K3		"28F640K3"
#define	CFI_CHIP_INTEL_28F128K3			0x8802
#define	CFI_CHIPN_INTEL_28F128K3		"28F128K3"
#define	CFI_CHIP_INTEL_28F256K3			0x8803
#define	CFI_CHIPN_INTEL_28F256K3		"28F256K3"
#define	CFI_CHIP_INTEL_28F640K18		0x8805
#define	CFI_CHIPN_INTEL_28F640K18		"28F640K18"
#define	CFI_CHIP_INTEL_28F128K18		0x8806
#define	CFI_CHIPN_INTEL_28F128K18		"28F128K18"
#define	CFI_CHIP_INTEL_28F256K18		0x8807
#define	CFI_CHIPN_INTEL_28F256K18		"28F256K18"

#endif /* FLASH_INTEL_H */
