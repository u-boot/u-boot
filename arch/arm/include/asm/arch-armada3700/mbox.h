/*
 * ***************************************************************************
 * Copyright (C) 2017 Marvell International Ltd.
 * ***************************************************************************
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of Marvell nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ***************************************************************************
 */

#ifndef _A3700_MBOX_H_
#define _A3700_MBOX_H_

#define MBOX_MAX_ARGS			(16)
#define MBOX_CMD_TIMEOUT		(100000)
#define A3700_MBOX_BASE			(MVEBU_REGISTER(0xb0000))

/* Mailbox command, arguments and status */
#define MBOX_SEND_ARG_OFFS(n)		(A3700_MBOX_BASE + (n) * 4)
#define MBOX_SEND_CMD_OFFS		(A3700_MBOX_BASE + 0x40)
#define MBOX_RECEIVE_STAT_OFFS		(A3700_MBOX_BASE + 0x80)
#define MBOX_RECEIVE_ARG_OFFS(n)	(A3700_MBOX_BASE + 0x84 + (n) * 4)

/* Host intterrupt reset - to complete received command/status processing */
#define MBOX_HOST_INT_RESET		(A3700_MBOX_BASE + 0xC8)
#define MBOX_SEC_CPU_CMD_COMPLETE	BIT(0)

/*
 * Host interrupt status - to poll for new command/status
 * received from secure CPU
 */
#define MBOX_SEC_CPU_INT_STAT_REG	(SOC_REGS_PHY_BASE + 0x17814)
#define MBOX_SEC_CPU_CMD_SET		BIT(31)

#define MBOX_COMMAND(sz, op)		((sz) | ((op) << 8))
#define MBOX_OP_SIZE(cmd)		((cmd) & 0xF)
#define MBOX_OPERATION(cmd)		(((cmd) >> 8) & 0xF)

enum mbox_opsize {
	MB_OPSZ_BIT	= 1,	/* single bit */
	MB_OPSZ_BYTE	= 2,	/* single byte */
	MB_OPSZ_WORD	= 3,	/* 4 bytes - half row */
	MB_OPSZ_DWORD	= 4,	/* 8 bytes - one row */
	MB_OPSZ_256B	= 5,	/* 32 bytes - 4 rows */
	MB_OPSZ_MAX
};

enum mbox_op {
	MB_OP_READ	= 1,
	MB_OP_WRITE	= 2,
	MB_OP_MAX
};

enum mbox_status {
	MB_STAT_SUCCESS			= 0,
	MB_STAT_HW_ERROR		= 1,
	MB_STAT_TIMEOUT			= 2,
	MB_STAT_BAD_ARGUMENT		= 3,

	MB_STAT_MAX
};

int mbox_send(enum mbox_opsize opsz, enum mbox_op op, uint32_t row,
	      uint32_t offs, uint32_t *args);
int mbox_receive(enum mbox_status *stat, uint32_t *args, uint32_t timeout_us);

#endif /* _A3700_MBOX_H_ */

