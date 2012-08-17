/*
 * Faraday FTIDE020 ATA Controller (AHB)
 *
 * (C) Copyright 2011 Andes Technology
 * Greentime Hu <greentime@andestech.com>
 * Macpaul Lin <macpaul@andestech.com>
 * Kuo-Wei Chou <kwchou@andestech.com>
 *
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */
/* ftide020.c - ide support functions for the FTIDE020_S controller */

#include <config.h>
#include <common.h>
#include <ata.h>
#include <ide.h>
#include <asm/io.h>
#include <api_public.h>

#include "ftide020.h"

/* base address */
#define FTIDE_BASE	CONFIG_SYS_ATA_BASE_ADDR

/*
 * data address - The CMD and DATA use the same FIFO in FTIDE020_S
 *   FTIDE_DATA = CONFIG_SYS_ATA_BASE_ADDR + CONFIG_SYS_ATA_DATA_OFFSET
 *		= &ftide020->rw_fifo
 */
#define FTIDE_DATA	(&ftide020->rw_fifo)

/* command and data I/O macros */
/* 0x0 - DATA FIFO */
#define WRITE_DATA(x)	outl((x), &ftide020->rw_fifo)	/* 0x00 */
#define READ_DATA()	inl(&ftide020->rw_fifo)		/* 0x00 */
/* 0x04 - R: Status Reg, W: CMD_FIFO */
#define WRITE_CMD(x)	outl((x), &ftide020->cmd_fifo)	/* 0x04 */
#define READ_STATUS()	inl(&ftide020->cmd_fifo)	/* 0x04 */

void ftide_set_device(int cx8, int dev)
{
	static struct ftide020_s *ftide020 = (struct ftide020_s *) FTIDE_BASE;

	WRITE_CMD(SET_DEV_CMD | IDE_SET_CX8(cx8) | dev);
}

unsigned char ide_read_register(int dev, unsigned int port)
{
	static struct ftide020_s *ftide020 = (struct ftide020_s *) FTIDE_BASE;

	ftide_set_device(0, dev);
	WRITE_CMD(READ_REG_CMD | IDE_REG_CS_READ(CONFIG_IDE_REG_CS) |
		IDE_REG_DA_WRITE(port));

	return READ_DATA() & 0xff;
}

void ide_write_register(int dev, unsigned int port, unsigned char val)
{
	static struct ftide020_s *ftide020 = (struct ftide020_s *) FTIDE_BASE;

	ftide_set_device(0, dev);
	WRITE_CMD(WRITE_REG_CMD | IDE_REG_CS_WRITE(CONFIG_IDE_REG_CS) |
		IDE_REG_DA_WRITE(port) | val);
}

void ide_write_data(int dev, ulong *sect_buf, int words)
{
	static struct ftide020_s *ftide020 = (struct ftide020_s *) FTIDE_BASE;

	ftide_set_device(0, dev);
	WRITE_CMD(WRITE_DATA_CMD | ((words << 2) - 1));

	/* block write */
	outsl(FTIDE_DATA, sect_buf, words);
}

void ide_read_data(int dev, ulong *sect_buf, int words)
{
	static struct ftide020_s *ftide020 = (struct ftide020_s *) FTIDE_BASE;

	ftide_set_device(0, dev);
	WRITE_CMD(READ_DATA_CMD | ((words << 2) - 1));

	/* block read */
	insl(FTIDE_DATA, sect_buf, words);
}

void ftide_dfifo_ready(ulong *time)
{
	static struct ftide020_s *ftide020 = (struct ftide020_s *) FTIDE_BASE;

	while (!(READ_STATUS() & STATUS_RFE)) {
		if (*time-- == 0)
			break;

		udelay(100);
	}
}

extern ulong ide_bus_offset[CONFIG_SYS_IDE_MAXBUS];

/* Reset_IDE_controller */
static void reset_ide_controller(void)
{
	static struct ftide020_s *ftide020 = (struct ftide020_s *) FTIDE_BASE;
	unsigned int val;

	val = inl(&ftide020->cr);

	val |= CONTROL_RST;
	outl(val, &ftide020->cr);

	/* wait until reset OK, this is poor HW design */
	mdelay(50);
	val &= ~(CONTROL_RST);
	outl(val, &ftide020->cr);

	mdelay(50);
	val |= CONTROL_SRST;
	outl(val, &ftide020->cr);

	/* wait until reset OK, this is poor HW design */
	mdelay(50);
	val &= ~(CONTROL_SRST);
	outl(val, &ftide020->cr);

	/* IORDY enable for PIO, for 2 device */
	val |= (CONTROL_IRE0 | CONTROL_IRE1);
	outl(val, &ftide020->cr);
}

/* IDE clock frequence */
uint ftide_clock_freq(void)
{
	/*
	 * todo: To aquire dynamic system frequency is dependend on the power
	 * management unit which the ftide020 is connected to. In current,
	 * there are only few PMU supports in u-boot.
	 * So this function is wait for future enhancement.
	 */
	return 100;
}

/* Calculate Timing Registers */
static unsigned int timing_cal(u16 t0, u16 t1, u16 t2, u16 t4)
{
	unsigned int val, ahb_ns = 8;
	u8 TEOC, T1, T2, T4;

	T1 = (u8) (t1 / ahb_ns);
	if ((T1 * ahb_ns) == t1)
		T1--;

	T2 = (u8) (t2 / ahb_ns);
	if ((T2 * ahb_ns) == t2)
		T2--;

	T4 = (u8) (t4 / ahb_ns);
	if ((T4 * ahb_ns) == t4)
		T4--;

	TEOC = (u8) (t0 / ahb_ns);
	if ((TEOC * ahb_ns) == t0)
		TEOC--;

	TEOC = ((TEOC > (T1 + T2 + T4)) ? (TEOC - (T1 + T2 + T4)) : 0);

	/*
	 * Here the fields in data timing registers in PIO mode
	 * is accessed the same way as command timing registers.
	 */
	val =	DT_REG_PIO_T1(T1)	|
		DT_REG_PIO_T2(T2)	|
		DT_REG_PIO_T4(T4)	|
		DT_REG_PIO_TEOC(TEOC);

	return val;
}

/* Set Timing Register */
static unsigned int set_mode_timing(u8 dev, u8 id, u8 mode)
{
	static struct ftide020_s *ftide020 = (struct ftide020_s *) FTIDE_BASE;
	u16 t0, t1, t2, t4;
	u8 tcyc, tcvs, tmli, tenv, tack, trp;
	unsigned int val, sysclk = 8;

	if (id >= TATOL_TIMING)
		return 0;

	sysclk = ftide_clock_freq();
	switch (id) {
	case CMD_TIMING:
		if (mode < REG_MODE) {
			t0 = REG_ACCESS_TIMING[REG_T0][mode];
			t1 = REG_ACCESS_TIMING[REG_T1][mode];
			t2 = REG_ACCESS_TIMING[REG_T2][mode];
			t4 = REG_ACCESS_TIMING[REG_T4][mode];

			val = timing_cal(t0, t1, t2, t4);
			outl(val, (dev ? &ftide020->ctrd1 : &ftide020->ctrd0));
			return 1;
		} else
			return 0;
	case PIO_TIMING:
		if (mode < PIO_MODE) {
			t0 = PIO_ACCESS_TIMING[PIO_T0][mode];
			t1 = PIO_ACCESS_TIMING[PIO_T1][mode];
			t2 = PIO_ACCESS_TIMING[PIO_T2][mode];
			t4 = PIO_ACCESS_TIMING[PIO_T4][mode];

			val = timing_cal(t0, t1, t2, t4);

			outl(val, (dev ? &ftide020->dtrd1 : &ftide020->dtrd0));
			return 1;
		} else
			return 0;
	case DMA_TIMING:
		if (mode < UDMA_MODE) {
			/*
			 * 0.999 is ceiling
			 * for tcyc, tcvs, tmli, tenv, trp, tack
			 */
			tcyc = (u8) (((UDMA_ACCESS_TIMING[UDMA_TCYC][mode] \
						* sysclk) + 9990) / 10000);
			tcvs = (u8) (((UDMA_ACCESS_TIMING[UDMA_TCVS][mode] \
						* sysclk) + 9990) / 10000);
			tmli = (u8) (((UDMA_ACCESS_TIMING[UDMA_TMLI][mode] \
						* sysclk) + 9990) / 10000);
			tenv = (u8) (((UDMA_ACCESS_TIMING[UDMA_TENV][mode] \
						* sysclk) + 9990) / 10000);
			trp  = (u8) (((UDMA_ACCESS_TIMING[UDMA_TRP][mode] \
						* sysclk) + 9990) / 10000);
			tack = (u8) (((UDMA_ACCESS_TIMING[UDMA_TACK][mode] \
						 * sysclk) + 9990) / 10000);

			val  =	DT_REG_UDMA_TENV((tenv > 0) ? (tenv - 1) : 0) |
				DT_REG_UDMA_TMLI((tmli > 0) ? (tmli - 1) : 0) |
				DT_REG_UDMA_TCYC((tcyc > 0) ? (tcyc - 1) : 0) |
				DT_REG_UDMA_TACK((tack > 0) ? (tack - 1) : 0) |
				DT_REG_UDMA_TCVS((tcvs > 0) ? (tcvs - 1) : 0) |
				DT_REG_UDMA_TRP((trp > 0) ? (trp - 1) : 0);

			outl(val, (dev ? &ftide020->dtrd1 : &ftide020->dtrd0));
			return 1;
		} else
			return 0;
	default:
		return 0;
	}
}

static void ftide_read_hwrev(void)
{
	static struct ftide020_s *ftide020 = (struct ftide020_s *) FTIDE_BASE;
	unsigned int rev;

	rev = inl(&ftide020->revision);
}

static int ftide_controller_probe(void)
{
	static struct ftide020_s *ftide020 = (struct ftide020_s *) FTIDE_BASE;
	unsigned int bak;

	bak = inl(&ftide020->ctrd1);

	/* probing by using shorter setup time */
	outl(CONFIG_CTRD1_PROBE_T1, &ftide020->ctrd1);
	if ((inl(&ftide020->ctrd1) & 0xff) != CONFIG_CTRD1_PROBE_T1) {
		outl(bak, &ftide020->ctrd1);
		return 0;
	}

	/* probing by using longer setup time */
	outl(CONFIG_CTRD1_PROBE_T2, &ftide020->ctrd1);
	if ((inl(&ftide020->ctrd1) & 0xff) != CONFIG_CTRD1_PROBE_T2) {
		outl(bak, &ftide020->ctrd1);
		return 0;
	}

	outl(bak, &ftide020->ctrd1);

	return 1;
}

/* ide_preinit() was migrated from linux driver ide_probe_for_ftide() */
int ide_preinit(void)
{
	static struct ftide020_s *ftide020 = (struct ftide020_s *) FTIDE_BASE;
	int status;
	unsigned int val;
	int i;

	status = 1;
	for (i = 0; i < CONFIG_SYS_IDE_MAXBUS; i++)
		ide_bus_offset[i] = -ATA_STATUS;

	/* auto-detect IDE controller */
	if (ftide_controller_probe()) {
		printf("FTIDE020_S\n");
	} else {
		printf("FTIDE020_S ATA controller not found.\n");
		return API_ENODEV;
	}

	/* check HW IP revision */
	ftide_read_hwrev();

	/* set FIFO threshold */
	outl(((WRITE_FIFO - RX_THRESH) << 16) | RX_THRESH, &ftide020->dmatirr);

	/* set Device_0 PIO_4 timing */
	set_mode_timing(0, CMD_TIMING, REG_MODE4);
	set_mode_timing(0, PIO_TIMING, PIO_MODE4);

	/* set Device_1 PIO_4 timing */
	set_mode_timing(1, CMD_TIMING, REG_MODE4);
	set_mode_timing(1, PIO_TIMING, PIO_MODE4);

	/* from E-bios */
	/* little endian */
	outl(0x0, &ftide020->cr);
	mdelay(10);

	outl(0x0fff0fff, &ftide020->ahbtr);
	mdelay(10);

	/* Enable controller Interrupt */
	val = inl(&ftide020->cr);

	/* Enable: IDE IRQ, IDE Terminate ERROR IRQ, AHB Timeout error IRQ */
	val |= (CONTROL_IIE | CONTROL_TERIE | CONTROL_AERIE);
	outl(val, &ftide020->cr);

	status = 0;

	return status;
}

void ide_set_reset(int flag)
{
	debug("ide_set_reset()\n");
	reset_ide_controller();
	return;
}
