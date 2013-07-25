/*
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mpc8xx.h>

#define	_NOT_USED_	0xFFFFFFFF

/*Orginal table, GPL4 disabled*/
const uint sdram_table[] =
{
	/* single read   (offset 0x00 in upm ram) */
	0x1f07cc04, 0xeeaeec04, 0x11adcc04, 0xefbbac00,
	0x1ff74c47,
	/* Precharge */
	0x1FF74C05,
	_NOT_USED_,
	_NOT_USED_,
	/* burst read    (offset 0x08 in upm ram) */
	0x1f07cc04, 0xeeaeec04, 0x00adcc04, 0x00afcc00,
	0x00afcc00, 0x01afcc00, 0x0fbb8c00, 0x1ff74c47,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/* single write  (offset 0x18 in upm ram) */
	0x1f27cc04, 0xeeaeac00, 0x01b90c04, 0x1ff74c47,
	/* Load moderegister */
	0x1FF74C34, /*Precharge*/
	0xEFEA8C34, /*NOP*/
	0x1FB54C35, /*Load moderegister*/
	_NOT_USED_,

	/* burst write   (offset 0x20 in upm ram) */
	0x1f07cc04, 0xeeaeac00, 0x00ad4c00, 0x00afcc00,
	0x00afcc00, 0x01bb8c04, 0x1ff74c47, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/* refresh       (offset 0x30 in upm ram) */
	0x1ff5cc84, 0xffffec04, 0xffffec04, 0xffffec04,
	0xffffec84, 0xffffec07, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/* exception     (offset 0x3C in upm ram) */
	0x7fffec07, _NOT_USED_, _NOT_USED_, _NOT_USED_,
};

/* GPL5 driven every cycle */
/* the display and the DSP */
const uint dsp_disp_table[] =
{
	/* single read   (offset 0x00 in upm ram) */
	0xffffc80c, 0xffffc004, 0x0fffc004, 0x0fffd004,
	0x0fffc000, 0x0fffc004, 0x3fffc004, 0xffffcc05,
	/* burst read    (offset 0x08 in upm ram) */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/* single write  (offset 0x18 in upm ram) */
	0xffffcc0c, 0xffffc004, 0x0fffc004, 0x0fffd004,
	0x0fffc000, 0x0fffc004, 0x7fffc004, 0xfffffc05,
	/* burst write   (offset 0x20 in upm ram) */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/* refresh       (offset 0x30 in upm ram) */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/* exception     (offset 0x3C in upm ram) */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
};

int checkboard (void)
{
	puts ("Board: FlagaDM V3.0\n");
	return 0;
}

phys_size_t initdram (int board_type)
{
	volatile immap_t     *immap  = (immap_t *)CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	long int size_b0;

	memctl->memc_or2 = CONFIG_SYS_OR2;
	memctl->memc_br2 = CONFIG_SYS_BR2;

	udelay(100);
	upmconfig(UPMA, (uint *)sdram_table, sizeof(sdram_table)/sizeof(uint));

	memctl->memc_mptpr = MPTPR_PTP_DIV16;
	memctl->memc_mamr = CONFIG_SYS_MAMR_48_SDR | MAMR_TLFA_1X;

	/*Do the initialization of the SDRAM*/
	/*Start with the precharge cycle*/
	memctl->memc_mcr = (MCR_OP_RUN | MCR_UPM_A | MCR_MB_CS2 | \
				MCR_MLCF(1) | MCR_MAD(0x5));

	/*Then we need two refresh cycles*/
	memctl->memc_mamr = CONFIG_SYS_MAMR_48_SDR | MAMR_TLFA_2X;
	memctl->memc_mcr = (MCR_OP_RUN | MCR_UPM_A | MCR_MB_CS2 | \
				MCR_MLCF(2) | MCR_MAD(0x30));

	/*Mode register programming*/
	memctl->memc_mar = 0x00000088; /*CAS Latency = 2 and burst length = 4*/
	memctl->memc_mcr = (MCR_OP_RUN | MCR_UPM_A | MCR_MB_CS2 | \
				MCR_MLCF(1) | MCR_MAD(0x1C));

	/* That should do it, just enable the periodic refresh in burst of 4*/
	memctl->memc_mamr = CONFIG_SYS_MAMR_48_SDR | MAMR_TLFA_4X;
	memctl->memc_mamr |= (MAMR_PTAE | MAMR_GPL_A4DIS);

	size_b0 = 16*1024*1024;

	/*
	 * No bank 1 or 3
	 * invalidate bank
	 */
	memctl->memc_br1 = 0;
	memctl->memc_br3 = 0;

	upmconfig(UPMB, (uint *)dsp_disp_table, sizeof(dsp_disp_table)/sizeof(uint));

	memctl->memc_mbmr = MBMR_GPL_B4DIS;

	memctl->memc_or4 = CONFIG_SYS_OR4;
	memctl->memc_br4 = CONFIG_SYS_BR4;

	return (size_b0);
}
