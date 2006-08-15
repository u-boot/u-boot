/*
 * (C) Copyright 2006
 * Heiko Schocher, DENX Software Engineering, hs@denx.de
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>

#if (CONFIG_COMMANDS & CFG_CMD_NAND)

#include <nand.h>

#if 0
#define HS_printf(fmt,arg...) \
        printf("HS %s %s: " fmt,__FILE__, __FUNCTION__, ##arg)
#else
#define HS_printf(fmt,arg...) \
        do { } while (0)
#endif

#if 0
#define	CPLD_REG	uchar
#else
#define	CPLD_REG	u16
#endif

struct alpr_ndfc_regs {
	CPLD_REG cmd[4];
	CPLD_REG addr_wait;
	CPLD_REG term;
	CPLD_REG dummy;
	uchar    dum2[2];
	CPLD_REG data;
};

static u8 hwctl;
static struct alpr_ndfc_regs *alpr_ndfc;
static int	alpr_chip = 0;

#if 1
static int pdnb3_nand_dev_ready(struct mtd_info *mtd);

#if 1
static u_char alpr_read (void *padr) {
	return (u_char )*((u16 *)(padr));
}
#else
static u_char alpr_read (void *padr) {
	u16	hilf;
	u_char ret = 0;
	hilf = *((u16 *)(padr));
	ret = hilf;
printf("%p hilf: %x ret: %x\n", padr, hilf, ret);
	return ret;
}
#endif

static void alpr_write (u_char byte, void *padr) {
HS_printf("%p  Byte: %x\n", padr, byte);
	*(volatile u16 *)padr = (u16)(byte);
}

#elif 0
#define alpr_read(a) (*(volatile u16 *) (a))
#define alpr_write(a, b) ((*(volatile u16 *) (a)) = (b))
#else
#define alpr_read(a) readw(a)
#define alpr_write(a, b) writew(a, b)
#endif
/*
 * The ALPR has a NAND Flash Controller (NDFC) that handles all accesses to
 * the NAND devices.  The NDFC has command, address and data registers that
 * when accessed will set up the NAND flash pins appropriately.  We'll use the
 * hwcontrol function to save the configuration in a global variable.
 * We can then use this information in the read and write functions to
 * determine which NDFC register to access.
 *
 * There are 2 NAND devices on the board, a Hynix HY27US08561A (32 MByte).
 */
static void pdnb3_nand_hwcontrol(struct mtd_info *mtd, int cmd)
{
HS_printf("cmd: %x\n", cmd);
	switch (cmd) {
	case NAND_CTL_SETCLE:
		hwctl |= 0x1;
		break;
	case NAND_CTL_CLRCLE:
		hwctl &= ~0x1;
		break;
	case NAND_CTL_SETALE:
		hwctl |= 0x2;
		break;
	case NAND_CTL_CLRALE:
		hwctl &= ~0x2;
		break;
	case NAND_CTL_SETNCE:
		break;
	case NAND_CTL_CLRNCE:
		alpr_write(0x00, &(alpr_ndfc->term));
		break;
	}
}

static void pdnb3_nand_write_byte(struct mtd_info *mtd, u_char byte)
{
HS_printf("hwctl: %x %x %x %x\n", hwctl, byte, &(alpr_ndfc->cmd[alpr_chip]), &(alpr_ndfc->addr_wait));
	if (hwctl & 0x1)
		alpr_write(byte, &(alpr_ndfc->cmd[alpr_chip]));
	else if (hwctl & 0x2) {
		alpr_write(byte, &(alpr_ndfc->addr_wait));
	} else
		alpr_write(byte, &(alpr_ndfc->data));
}

static u_char pdnb3_nand_read_byte(struct mtd_info *mtd)
{
	return alpr_read(&(alpr_ndfc->data));
}

static void pdnb3_nand_write_buf(struct mtd_info *mtd, const u_char *buf, int len)
{
	int i;

/*printf("%s chip:%d hwctl:%x size:%d\n", __FUNCTION__, alpr_chip, hwctl, len);*/
	for (i = 0; i < len; i++) {
		if (hwctl & 0x1)
			alpr_write(buf[i], &(alpr_ndfc->cmd[alpr_chip]));
		else if (hwctl & 0x2) {
			alpr_write(buf[i], &(alpr_ndfc->addr_wait));
		} else {
			alpr_write(buf[i], &(alpr_ndfc->data));
			/*printf("i: %d\n", i);*/
		}	
	}
}

static void pdnb3_nand_read_buf(struct mtd_info *mtd, u_char *buf, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		buf[i] = alpr_read(&(alpr_ndfc->data));
	}
}

static int pdnb3_nand_verify_buf(struct mtd_info *mtd, const u_char *buf, int len)
{
	int i;

	for (i = 0; i < len; i++)
		if (buf[i] != alpr_read(&(alpr_ndfc->data)))
			return i;

	return 0;
}

static int pdnb3_nand_dev_ready(struct mtd_info *mtd)
{
#if 1
	volatile u_char val;

/*printf("%s aufruf\n", __FUNCTION__);*/
	/*
	 * Blocking read to wait for NAND to be ready
	 */
	val = alpr_read(&(alpr_ndfc->addr_wait));

	/*
	 * Return always true
	 */
	return 1;
#else
	u8 hwctl_org = hwctl;
	unsigned long	timeo;
	u8	val;

	hwctl = 0x01;
	pdnb3_nand_write_byte (mtd, NAND_CMD_STATUS);
	hwctl = hwctl_org;

	reset_timer();
	while (1) {
		if (get_timer(0) > timeo) {
			printf("Timeout!");
			return 0;
			}

val = pdnb3_nand_read_byte(mtd);
/*printf("%s val: %x\n", __FUNCTION__, val);*/
			if (val & NAND_STATUS_READY)
				break;
	}
	return 1;
#endif

}

static void alpr_select_chip(struct mtd_info *mtd, int chip)
{
	alpr_chip = chip;
}

static int alpr_nand_wait(struct mtd_info *mtd, struct nand_chip *this, int state)
{
	unsigned long	timeo;

	if (state == FL_ERASING)
		timeo = CFG_HZ * 400;
	else
		timeo = CFG_HZ * 20;

	if ((state == FL_ERASING) && (this->options & NAND_IS_AND))
		this->cmdfunc(mtd, NAND_CMD_STATUS_MULTI, -1, -1);
	else
		this->cmdfunc(mtd, NAND_CMD_STATUS, -1, -1);

	reset_timer();

	while (1) {
		if (get_timer(0) > timeo) {
			printf("Timeout!");
			return 0;
			}

			if (this->read_byte(mtd) & NAND_STATUS_READY)
				break;
	}
	return this->read_byte(mtd);
}

void board_nand_init(struct nand_chip *nand)
{
	alpr_ndfc = (struct alpr_ndfc_regs *)CFG_NAND_BASE;

	nand->eccmode = NAND_ECC_SOFT;

	/* Set address of NAND IO lines (Using Linear Data Access Region) */
	nand->IO_ADDR_R = (void __iomem *) ((ulong) alpr_ndfc + 0x10);
	nand->IO_ADDR_W = (void __iomem *) ((ulong) alpr_ndfc + 0x10);
	/* Reference hardware control function */
	nand->hwcontrol  = pdnb3_nand_hwcontrol;
	/* Set command delay time */
	nand->hwcontrol  = pdnb3_nand_hwcontrol;
	nand->write_byte = pdnb3_nand_write_byte;
	nand->read_byte  = pdnb3_nand_read_byte;
	nand->write_buf  = pdnb3_nand_write_buf;
	nand->read_buf   = pdnb3_nand_read_buf;
	nand->verify_buf = pdnb3_nand_verify_buf;
	nand->dev_ready  = pdnb3_nand_dev_ready;
	nand->select_chip = alpr_select_chip;
	nand->waitfunc 	 = alpr_nand_wait;
}
#endif
