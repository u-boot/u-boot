/*
 * Copyright (C) 2006 Freescale Semiconductor, Inc.
 *
 * Dave Liu <daveliu@freescale.com>
 * based on source code of Shlomi Gridish
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include "common.h"
#include "asm/errno.h"
#include "asm/io.h"
#include "asm/immap_qe.h"
#include "qe.h"

#if defined(CONFIG_QE)
qe_map_t		*qe_immr = NULL;
static qe_snum_t	snums[QE_NUM_OF_SNUM];

void qe_issue_cmd(uint cmd, uint sbc, u8 mcn, u32 cmd_data)
{
	u32           cecr;

	if (cmd == QE_RESET) {
		out_be32(&qe_immr->cp.cecr,(u32) (cmd | QE_CR_FLG));
	} else {
		out_be32(&qe_immr->cp.cecdr, cmd_data);
		out_be32(&qe_immr->cp.cecr, (sbc | QE_CR_FLG |
			 ((u32) mcn<<QE_CR_PROTOCOL_SHIFT) | cmd));
	}
	/* Wait for the QE_CR_FLG to clear */
	do {
		cecr = in_be32(&qe_immr->cp.cecr);
	} while (cecr & QE_CR_FLG);

	return;
}

uint qe_muram_alloc(uint size, uint align)
{
	DECLARE_GLOBAL_DATA_PTR;

	uint	retloc;
	uint	align_mask, off;
	uint	savebase;

	align_mask = align - 1;
	savebase = gd->mp_alloc_base;

	if ((off = (gd->mp_alloc_base & align_mask)) != 0)
		gd->mp_alloc_base += (align - off);

	if ((off = size & align_mask) != 0)
		size += (align - off);

	if ((gd->mp_alloc_base + size) >= gd->mp_alloc_top) {
		gd->mp_alloc_base = savebase;
		printf("%s: ran out of ram.\n",  __FUNCTION__);
	}

	retloc = gd->mp_alloc_base;
	gd->mp_alloc_base += size;

	memset((void *)&qe_immr->muram[retloc], 0, size);

	__asm__ __volatile__("sync");

	return retloc;
}

void *qe_muram_addr(uint offset)
{
	return (void *)&qe_immr->muram[offset];
}

static void qe_sdma_init(void)
{
	volatile sdma_t	*p;
	uint		sdma_buffer_base;

	p = (volatile sdma_t *)&qe_immr->sdma;

	/* All of DMA transaction in bus 1 */
	out_be32(&p->sdaqr, 0);
	out_be32(&p->sdaqmr, 0);

	/* Allocate 2KB temporary buffer for sdma */
	sdma_buffer_base = qe_muram_alloc(2048, 4096);
	out_be32(&p->sdwbcr, sdma_buffer_base & QE_SDEBCR_BA_MASK);

	/* Clear sdma status */
	out_be32(&p->sdsr, 0x03000000);

	/* Enable global mode on bus 1, and 2KB buffer size */
	out_be32(&p->sdmr, QE_SDMR_GLB_1_MSK | (0x3 << QE_SDMR_CEN_SHIFT));
}

static u8 thread_snum[QE_NUM_OF_SNUM] = {
	0x04, 0x05, 0x0c, 0x0d,
	0x14, 0x15, 0x1c, 0x1d,
	0x24, 0x25, 0x2c, 0x2d,
	0x34, 0x35, 0x88, 0x89,
	0x98, 0x99, 0xa8, 0xa9,
	0xb8, 0xb9, 0xc8, 0xc9,
	0xd8, 0xd9, 0xe8, 0xe9
};

static void qe_snums_init(void)
{
	int	i;

	for (i = 0; i < QE_NUM_OF_SNUM; i++) {
		snums[i].state = QE_SNUM_STATE_FREE;
		snums[i].num   = thread_snum[i];
	}
}

int qe_get_snum(void)
{
	int	snum = -EBUSY;
	int	i;

	for (i = 0; i < QE_NUM_OF_SNUM; i++) {
		if (snums[i].state == QE_SNUM_STATE_FREE) {
			snums[i].state = QE_SNUM_STATE_USED;
			snum = snums[i].num;
			break;
		}
	}

	return snum;
}

void qe_put_snum(u8 snum)
{
	int	i;

	for (i = 0; i < QE_NUM_OF_SNUM; i++) {
		if (snums[i].num == snum) {
			snums[i].state = QE_SNUM_STATE_FREE;
			break;
		}
	}
}

void qe_init(uint qe_base)
{
	DECLARE_GLOBAL_DATA_PTR;

	/* Init the QE IMMR base */
	qe_immr = (qe_map_t *)qe_base;

	gd->mp_alloc_base = QE_DATAONLY_BASE;
	gd->mp_alloc_top = gd->mp_alloc_base + QE_DATAONLY_SIZE;

	qe_sdma_init();
	qe_snums_init();
}

void qe_reset(void)
{
	qe_issue_cmd(QE_RESET, QE_CR_SUBBLOCK_INVALID,
			 (u8) QE_CR_PROTOCOL_UNSPECIFIED, 0);
}

void qe_assign_page(uint snum, uint para_ram_base)
{
	u32	cecr;

	out_be32(&qe_immr->cp.cecdr, para_ram_base);
	out_be32(&qe_immr->cp.cecr, ((u32) snum<<QE_CR_ASSIGN_PAGE_SNUM_SHIFT)
					 | QE_CR_FLG | QE_ASSIGN_PAGE);

	/* Wait for the QE_CR_FLG to clear */
	do {
		cecr = in_be32(&qe_immr->cp.cecr);
	} while (cecr & QE_CR_FLG );

	return;
}

/*
 * brg: 0~15 as BRG1~BRG16
   rate: baud rate
 * BRG input clock comes from the BRGCLK (internal clock generated from
   the QE clock, it is one-half of the QE clock), If need the clock source
   from CLKn pin, we have te change the function.
 */

#define BRG_CLK		(gd->brg_clk)

int qe_set_brg(uint brg, uint rate)
{
	DECLARE_GLOBAL_DATA_PTR;
	volatile uint	*bp;
	u32		divisor;
	int		div16 = 0;

	if (brg >= QE_NUM_OF_BRGS)
		return -EINVAL;
	bp = (uint *)&qe_immr->brg.brgc1;
	bp += brg;

	divisor = (BRG_CLK / rate);
	if (divisor > QE_BRGC_DIVISOR_MAX + 1) {
		div16 = 1;
		divisor /= 16;
	}

	*bp = ((divisor - 1) << QE_BRGC_DIVISOR_SHIFT) | QE_BRGC_ENABLE;
	__asm__ __volatile__("sync");

	if (div16) {
		*bp |= QE_BRGC_DIV16;
		__asm__ __volatile__("sync");
	}

	return 0;
}

/* Set ethernet MII clock master
*/
int qe_set_mii_clk_src(int ucc_num)
{
	u32	cmxgcr;

	/* check if the UCC number is in range. */
	if ((ucc_num > UCC_MAX_NUM - 1) || (ucc_num < 0)) {
		printf("%s: ucc num not in ranges\n", __FUNCTION__);
		return -EINVAL;
	}

	cmxgcr = in_be32(&qe_immr->qmx.cmxgcr);
	cmxgcr &= ~QE_CMXGCR_MII_ENET_MNG_MASK;
	cmxgcr |= (ucc_num <<QE_CMXGCR_MII_ENET_MNG_SHIFT);
	out_be32(&qe_immr->qmx.cmxgcr, cmxgcr);

	return 0;
}

#endif /* CONFIG_QE */
