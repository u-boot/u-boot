/*
 * (C) Copyright 2000
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * Board specific routines for the MBX
 *
 * - initialisation
 * - interface to VPD data (mac address, clock speeds)
 * - memory controller
 * - serial io initialisation
 * - ethernet io initialisation
 *
 * -----------------------------------------------------------------
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <commproc.h>
#include <mpc8xx.h>
#include <net.h>
#include "dimm.h"
#include "vpd.h"
#include "csr.h"

/* ------------------------------------------------------------------------- */

static const uint sdram_table_40[] = {
	/* DRAM - single read. (offset 0 in upm RAM)
	 */
	0xCFAFC004, 0x0FAFC404, 0x0CAF0C04, 0x30AF0C00,
	0xF1BF4805, 0xFFFFC005, 0xFFFFC005, 0xFFFFC005,

	/* DRAM - burst read. (offset 8 in upm RAM)
	 */
	0xCFAFC004, 0x0FAFC404, 0x0CAF0C04, 0x03AF0C08,
	0x0CAF0C04, 0x03AF0C08, 0x0CAF0C04, 0x03AF0C08,
	0x0CAF0C04, 0x30AF0C00, 0xF3BF4805, 0xFFFFC005,
	0xFFFFC005, 0xFFFFC005, 0xFFFFC005, 0xFFFFC005,

	/* DRAM - single write. (offset 18 in upm RAM)
	 */
	0xCFFF0004, 0x0FFF0404, 0x0CFF0C00, 0x33FF4804,
	0xFFFFC005, 0xFFFFC005, 0xFFFFC005, 0xFFFFC005,

	/* DRAM - burst write. (offset 20 in upm RAM)
	 */
	0xCFFF0004, 0x0FFF0404, 0x0CFF0C00, 0x03FF0C0C,
	0x0CFF0C00, 0x03FF0C0C, 0x0CFF0C00, 0x03FF0C0C,
	0x0CFF0C00, 0x33FF4804, 0xFFFFC005, 0xFFFFC005,
	0xFFFFC005, 0xFFFFC005, 0xFFFFC005, 0xFFFFC005,

	/* refresh  (offset 30 in upm RAM)
	 */
	0xFCFFC004, 0xC0FFC004, 0x01FFC004, 0x0FFFC004,
	0x3FFFC004, 0xFFFFC005, 0xFFFFC005, 0xFFFFC005,
	0xFFFFC005, 0xFFFFC005, 0xFFFFC005, 0xFFFFC005,

	/* exception. (offset 3c in upm RAM)
	 */
	0xFFFFC007, 0xFFFFC007, 0xFFFFC007, 0xFFFFC007,
};

static const uint sdram_table_50[] = {
	/* DRAM - single read. (offset 0 in upm RAM)
	 */
	0xCFAFC004, 0x0FAFC404, 0x0CAF8C04, 0x10AF0C04,
	0xF0AF0C00, 0xF3BF4805, 0xFFFFC005, 0xFFFFC005,

	/* DRAM - burst read. (offset 8 in upm RAM)
	 */
	0xCFAFC004, 0X0FAFC404, 0X0CAF8C04, 0X00AF0C04,
  /*	0X07AF0C08, 0X0CAF0C04, 0X01AF0C04, 0X0FAF0C04,	*/
	0X07AF0C08, 0X0CAF0C04, 0X01AF0C04, 0X0FAF0C08,
	0X0CAF0C04, 0X01AF0C04, 0X0FAF0C08, 0X0CAF0C04,
  /*	0X10AF0C04, 0XF0AFC000, 0XF3FF4805, 0XFFFFC005,	*/
	0X10AF0C04, 0XF0AFC000, 0XF3BF4805, 0XFFFFC005,

	/* DRAM - single write. (offset 18 in upm RAM)
	 */
	0xCFFF0004, 0x0FFF0404, 0x0CFF0C00, 0x13FF4804,
	0xFFFFC004, 0xFFFFC005, 0xFFFFC005, 0xFFFFC005,

	/* DRAM - burst write. (offset 20 in upm RAM)
	 */
	0xCFFF0004, 0x0FFF0404, 0x0CFF0C00, 0x03FF0C0C,
	0x0CFF0C00, 0x03FF0C0C, 0x0CFF0C00, 0x03FF0C0C,
	0x0CFF0C00, 0x13FF4804, 0xFFFFC004, 0xFFFFC005,
	0xFFFFC005, 0xFFFFC005, 0xFFFFC005, 0xFFFFC005,

	/* refresh  (offset 30 in upm RAM)
	 */
	0xFCFFC004, 0xC0FFC004, 0x01FFC004, 0x0FFFC004,
	0x1FFFC004, 0xFFFFC004, 0xFFFFC005, 0xFFFFC005,
	0xFFFFC005, 0xFFFFC005, 0xFFFFC005, 0xFFFFC005,

	/* exception. (offset 3c in upm RAM)
	 */
	0xFFFFC007, 0xFFFFC007, 0xFFFFC007, 0xFFFFC007,
};

/* ------------------------------------------------------------------------- */

static unsigned int get_reffreq(void);
static unsigned int board_get_cpufreq(void);

void mbx_init (void)
{
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immr->im_memctl;
	ulong speed, refclock, plprcr, sccr;
	ulong br0_32 = memctl->memc_br0 & 0x400;

	/* real-time clock status and control register */
	immr->im_sitk.sitk_rtcsck = KAPWR_KEY;
	immr->im_sit.sit_rtcsc = 0x00C3;

	/* SIEL and SIMASK Registers (see MBX PRG 2-3) */
	immr->im_siu_conf.sc_simask = 0x00000000;
	immr->im_siu_conf.sc_siel = 0xAAAA0000;
	immr->im_siu_conf.sc_tesr = 0xFFFFFFFF;

	/*
	 * Prepare access to i2c bus. The MBX offers 3 devices on the i2c bus:
	 * 1. Vital Product Data (contains clock speeds, MAC address etc, see vpd.h)
	 * 2. RAM  Specs (see dimm.h)
	 * 2. DIMM Specs (see dimm.h)
	 */
	vpd_init ();

	/* system clock and reset control register */
	immr->im_clkrstk.cark_sccrk = KAPWR_KEY;
	sccr = immr->im_clkrst.car_sccr;
	sccr &= SCCR_MASK;
	sccr |= CONFIG_SYS_SCCR;
	immr->im_clkrst.car_sccr = sccr;

	speed = board_get_cpufreq ();
	refclock = get_reffreq ();

#if ((CONFIG_SYS_PLPRCR & PLPRCR_MF_MSK) != 0)
	plprcr = CONFIG_SYS_PLPRCR;
#else
	plprcr = immr->im_clkrst.car_plprcr;
	plprcr &= PLPRCR_MF_MSK;	/* isolate MF field */
	plprcr |= CONFIG_SYS_PLPRCR;		/* reset control bits   */
#endif

#ifdef CONFIG_SYS_USE_OSCCLK			/* See doc/README.MBX ! */
	plprcr |= ((speed + refclock / 2) / refclock - 1) << 20;
#endif

	immr->im_clkrstk.cark_plprcrk = KAPWR_KEY;
	immr->im_clkrst.car_plprcr = plprcr;

	/*
	 * preliminary setup of memory controller:
	 * - map Flash, otherwise configuration/status
	 *    registers won't be accessible when read
	 *    by board_init_f.
	 * - map NVRAM and configuation/status registers.
	 * - map pci registers.
	 * - DON'T map ram yet, this is done in initdram().
	 */
	switch (speed / 1000000) {
	case 40:
		memctl->memc_br0 = 0xFE000000 | br0_32 | 1;
		memctl->memc_or0 = 0xFF800930;
		memctl->memc_or4 = CONFIG_SYS_NVRAM_OR | 0x920;
		memctl->memc_br4 = CONFIG_SYS_NVRAM_BASE | 0x401;
		break;
	case 50:
		memctl->memc_br0 = 0xFE000000 | br0_32 | 1;
		memctl->memc_or0 = 0xFF800940;
		memctl->memc_or4 = CONFIG_SYS_NVRAM_OR | 0x930;
		memctl->memc_br4 = CONFIG_SYS_NVRAM_BASE | 0x401;
		break;
	default:
		hang ();
		break;
	}
#ifdef CONFIG_USE_PCI
	memctl->memc_or5 = CONFIG_SYS_PCIMEM_OR;
	memctl->memc_br5 = CONFIG_SYS_PCIMEM_BASE | 0x001;
	memctl->memc_or6 = CONFIG_SYS_PCIBRIDGE_OR;
	memctl->memc_br6 = CONFIG_SYS_PCIBRIDGE_BASE | 0x001;
#endif
	/*
	 * FIXME: I do not understand why I have to call this to
	 * initialise the control register here before booting from
	 * the PCMCIA card but if I do not the Linux kernel falls
	 * over in a big heap. If you can answer this question I
	 * would like to know about it.
	 */
	board_ether_init();
}

void board_serial_init (void)
{
	MBX_CSR1 &= ~(CSR1_COM1EN | CSR1_XCVRDIS);
}

void board_ether_init (void)
{
	MBX_CSR1 &= ~(CSR1_EAEN | CSR1_ELEN);
	MBX_CSR1 |= CSR1_ETEN | CSR1_TPEN | CSR1_FDDIS;
}

static unsigned int board_get_cpufreq (void)
{
#ifndef CONFIG_8xx_GCLK_FREQ
	vpd_packet_t *packet;

	packet = vpd_find_packet (VPD_PID_ICS);
	return *((ulong *) packet->data);
#else
	return((unsigned int)CONFIG_8xx_GCLK_FREQ );
#endif /* CONFIG_8xx_GCLK_FREQ */
}

static unsigned int get_reffreq (void)
{
	vpd_packet_t *packet;

	packet = vpd_find_packet (VPD_PID_RCS);
	return *((ulong *) packet->data);
}

static void board_get_enetaddr(uchar *addr)
{
	int i;
	vpd_packet_t *packet;

	packet = vpd_find_packet (VPD_PID_EA);
	for (i = 0; i < 6; i++)
		addr[i] = packet->data[i];
}

int misc_init_r(void)
{
	uchar enetaddr[6];

	if (!eth_getenv_enetaddr("ethaddr", enetaddr)) {
		board_get_enetaddr(enetaddr);
		eth_setenv_enetaddr("ethaddr", enetaddr);
	}

	return 0;
}

/*
 * Check Board Identity:
 */

int checkboard (void)
{
	vpd_packet_t *packet;
	int i;
	const char *const fmt =
		"\n	 *** Warning: Low Battery Status - %s Battery ***";

	puts ("Board: ");

	packet = vpd_find_packet (VPD_PID_PID);
	for (i = 0; i < packet->size; i++) {
		serial_putc (packet->data[i]);
	}
	packet = vpd_find_packet (VPD_PID_MT);
	for (i = 0; i < packet->size; i++) {
		serial_putc (packet->data[i]);
	}
	serial_putc ('(');
	packet = vpd_find_packet (VPD_PID_FAN);
	for (i = 0; i < packet->size; i++) {
		serial_putc (packet->data[i]);
	}
	serial_putc (')');

	if (!(MBX_CSR2 & SR2_BATGD))
		printf (fmt, "On-Board");
	if (!(MBX_CSR2 & SR2_NVBATGD))
		printf (fmt, "NVRAM");

	serial_putc ('\n');

	return (0);
}

/* ------------------------------------------------------------------------- */

static ulong get_ramsize (dimm_t * dimm)
{
	ulong size = 0;

	if (dimm->fmt == 1 || dimm->fmt == 2 || dimm->fmt == 3
		|| dimm->fmt == 4) {
		size = (1 << (dimm->n_row + dimm->n_col)) * dimm->n_banks *
			((dimm->data_w_hi << 8 | dimm->data_w_lo) / 8);
	}

	return size;
}

phys_size_t initdram (int board_type)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	unsigned long ram_sz = 0;
	unsigned long dimm_sz = 0;
	dimm_t vpd_dimm, vpd_dram;
	unsigned int speed = board_get_cpufreq () / 1000000;

	if (vpd_read (0xa2, (uchar *) & vpd_dimm, sizeof (vpd_dimm), 0) > 0) {
		dimm_sz = get_ramsize (&vpd_dimm);
	}
	if (vpd_read (0xa6, (uchar *) & vpd_dram, sizeof (vpd_dram), 0) > 0) {
		ram_sz = get_ramsize (&vpd_dram);
	}

	/*
	 * Only initialize memory controller when running from FLASH.
	 * When running from RAM, don't touch it.
	 */
	if ((ulong) initdram & 0xff000000) {
		ulong dimm_bank;
		ulong br0_32 = memctl->memc_br0 & 0x400;

		switch (speed) {
		case 40:
			upmconfig (UPMA, (uint *) sdram_table_40,
					   sizeof (sdram_table_40) / sizeof (uint));
			memctl->memc_mptpr = 0x0200;
			memctl->memc_mamr = dimm_sz ? 0x06801000 : 0x13801000;
			memctl->memc_or7 = 0xff800930;
			memctl->memc_br7 = 0xfc000000 | (br0_32 ^ br0_32) | 1;
			break;
		case 50:
			upmconfig (UPMA, (uint *) sdram_table_50,
					   sizeof (sdram_table_50) / sizeof (uint));
			memctl->memc_mptpr = 0x0200;
			memctl->memc_mamr = dimm_sz ? 0x08801000 : 0x1880100;
			memctl->memc_or7 = 0xff800940;
			memctl->memc_br7 = 0xfc000000 | (br0_32 ^ br0_32) | 1;
			break;
		default:
			hang ();
			break;
		}

		/* now map ram and dimm, largest one first */
		dimm_bank = dimm_sz / 2;
		if (!dimm_sz) {
			memctl->memc_or1 = ~(ram_sz - 1) | 0x400;
			memctl->memc_br1 = CONFIG_SYS_SDRAM_BASE | 0x81;
			memctl->memc_br2 = 0;
			memctl->memc_br3 = 0;
		} else if (ram_sz > dimm_bank) {
			memctl->memc_or1 = ~(ram_sz - 1) | 0x400;
			memctl->memc_br1 = CONFIG_SYS_SDRAM_BASE | 0x81;
			memctl->memc_or2 = ~(dimm_bank - 1) | 0x400;
			memctl->memc_br2 = (CONFIG_SYS_SDRAM_BASE + ram_sz) | 0x81;
			memctl->memc_or3 = ~(dimm_bank - 1) | 0x400;
			memctl->memc_br3 = (CONFIG_SYS_SDRAM_BASE + ram_sz + dimm_bank) \
								     | 0x81;
		} else {
			memctl->memc_or2 = ~(dimm_bank - 1) | 0x400;
			memctl->memc_br2 = CONFIG_SYS_SDRAM_BASE | 0x81;
			memctl->memc_or3 = ~(dimm_bank - 1) | 0x400;
			memctl->memc_br3 = (CONFIG_SYS_SDRAM_BASE + dimm_bank) | 0x81;
			memctl->memc_or1 = ~(ram_sz - 1) | 0x400;
			memctl->memc_br1 = (CONFIG_SYS_SDRAM_BASE + dimm_sz) | 0x81;
		}
	}

	return ram_sz + dimm_sz;
}
