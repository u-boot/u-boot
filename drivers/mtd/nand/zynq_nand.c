/*
 * Xilinx PSS NAND Flash Controller Driver
 *
 * Copyright (C) 2009 Xilinx, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * This driver is based on plat_nand.c and mxc_nand.c drivers
 */

#ifdef LINUX_ONLY_NOT_UBOOT
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/delay.h>
#include <linux/mtd/nand_ecc.h>
#include <mach/smc.h>
#endif

#include "xbasic_types.h"
#include <common.h>
#include <malloc.h>

#include <asm/arch/nand.h>
#include <linux/mtd/compat.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/nand_ecc.h>
#include "zynq_nand.h"

/********** stubs - Make Linux code compile in this environment **************/
#define EIO              5
#define ENXIO            6
#define ENOMEM          12
#define EFAULT          14
#define ENODEV          19

#define dev_dbg(dev, format, arg...)            \
        printf(format , ## arg)

#define dev_err(dev, format, arg...)            \
        printf(format , ## arg)

#define dev_info(dev, format, arg...)            \
        printf(format , ## arg)

#define __devinitdata
#define __force

/*********************************/

#define XNANDPSS_DRIVER_NAME "Xilinx_PSS_NAND"

/*
 * The NAND flash driver defines
 */

#define XNANDPSS_CMD_PHASE	1	/* End command valid in command phase */
#define XNANDPSS_DATA_PHASE	2	/* End command valid in data phase */
#define XNANDPSS_ECC_SIZE	512	/* Size of data for ECC operation */

/*
 * Register values for using NAND interface of NAND controller
 * The SET_CYCLES_REG register value depends on the flash device. Look in to the
 * device datasheet and change its value, This value is for 2Gb Numonyx flash.
 */

/* Flash memory controller operating parameters */
#define XNANDPSS_CLR_CONFIG	((0x1 << 1)  |	/* Disable interrupt */ \
				(0x1 << 4)   |	/* Clear interrupt */ \
				(0x1 << 6))	/* Disable ECC interrupt */

/* Assuming 50MHz clock (20ns cycle time) and 3V operation */
#define XNANDPSS_SET_CYCLES	((0x2 << 20) |	/* t_rr from nand_cycles */ \
				(0x2 << 17)  |	/* t_ar from nand_cycles */ \
				(0x1 << 14)  |	/* t_clr from nand_cycles */ \
				(0x3 << 11)  |	/* t_wp from nand_cycles */ \
				(0x2 << 8)   |	/* t_rea from nand_cycles */ \
				(0x5 << 4)   |	/* t_wc from nand_cycles */ \
				(0x5 << 0))	/* t_rc from nand_cycles */

#define XNANDPSS_SET_OPMODE	0x0

#define XNANDPSS_DIRECT_CMD	((0x4 << 23) |	/* Chip 0 from interface 1 */ \
				(0x2 << 21))	/* UpdateRegs operation */

#define XNANDPSS_ECC_CONFIG	((0x1 << 2)  |	/* ECC available on APB */ \
				(0x1 << 4)   |	/* ECC read at end of page */ \
				(0x0 << 5))	/* No Jumping */

#define XNANDPSS_ECC_CMD1	((0x80)      |	/* Write command */ \
				(0x00 << 8)  |	/* Read command */ \
				(0x30 << 16) |	/* Read End command */ \
				(0x1 << 24))	/* Read End command calid */

#define XNANDPSS_ECC_CMD2	((0x85)      |	/* Write col change cmd */ \
				(0x05 << 8)  |	/* Read col change cmd */ \
				(0xE0 << 16) |	/* Read col change end cmd */ \
				(0x1 << 24))	/* Read col change
							end cmd valid */
/*
 * AXI Address definitions
 */
#define START_CMD_SHIFT		3
#define END_CMD_SHIFT		11
#define END_CMD_VALID_SHIFT	20
#define ADDR_CYCLES_SHIFT	21
#define CLEAR_CS_SHIFT		21
#define ECC_LAST_SHIFT		10
#define COMMAND_PHASE		(0 << 19)
#define DATA_PHASE		(1 << 19)

#define XNANDPSS_ECC_LAST	(1 << ECC_LAST_SHIFT)	/* Set ECC_Last */
#define XNANDPSS_CLEAR_CS	(1 << CLEAR_CS_SHIFT)	/* Clear chip select */

/*
 * ECC block registers bit position and bit mask
 */
#define XNANDPSS_ECC_BUSY	(1 << 6)	/* ECC block is busy */
#define XNANDPSS_ECC_MASK	0x00FFFFFF	/* ECC value mask */

/*
 * Macros for the NAND controller register read/write
 */
extern void XIo_Out32(u32 OutAddress, u32 Value);
extern u32  XIo_In32(u32 InAddress);

#define xnandps_read32(addr)       XIo_In32((u32)(addr))
#define xnandps_write32(addr, val)	XIo_Out32((u32)(addr), (val))
#define readl(addr)                 XIo_In32((u32)(addr))
#define writel(val, addr)           XIo_Out32((u32)(addr), (val))


/**
 * struct xnandps_command_format - Defines NAND flash command format
 * @start_cmd:		First cycle command (Start command)
 * @end_cmd:		Second cycle command (Last command)
 * @addr_cycles:	Number of address cycles required to send the address
 * @end_cmd_valid:	The second cycle command is valid for cmd or data phase
 **/
struct xnandps_command_format {
	int start_cmd;
	int end_cmd;
	u8 addr_cycles;
	u8 end_cmd_valid;
};

/**
 * struct xnandps_info - Defines the NAND flash driver instance
 * @chip:		NAND chip information structure
 * @mtd:		MTD information structure
 * @parts:		Pointer	to the mtd_partition structure
 * @nand_base:		Virtual address of the NAND flash device
 * @smc_regs:		Virtual address of the NAND controller registers
 * @end_cmd_pending:	End command is pending
 * @end_cmd:		End command
 **/
struct xnandps_info {
#ifdef LINUX_ONLY_NOT_UBOOT
	struct nand_chip	chip;
	struct mtd_info		mtd;
#endif
#ifdef CONFIG_MTD_PARTITIONS
	struct mtd_partition	*parts;
#endif
	void __iomem		*nand_base;
	void __iomem		*smc_regs;
	unsigned long		end_cmd_pending;
	unsigned long		end_cmd;
};

#define NAND_CMD_GET_FEATURES	0xEE
#define NAND_CMD_SET_FEATURES	0xEF

#define ONDIE_ECC_FEATURE_ADDR	0x90

/*
 * The NAND flash operations command format
 */
static struct xnandps_command_format xnandps_commands[] __devinitdata = {
	{NAND_CMD_READ0, NAND_CMD_READSTART, 5, XNANDPSS_CMD_PHASE},
	{NAND_CMD_RNDOUT, NAND_CMD_RNDOUTSTART, 2, XNANDPSS_CMD_PHASE},
	{NAND_CMD_READID, NAND_CMD_NONE, 1, NAND_CMD_NONE},
	{NAND_CMD_STATUS, NAND_CMD_NONE, 0, NAND_CMD_NONE},
	{NAND_CMD_SEQIN, NAND_CMD_PAGEPROG, 5, XNANDPSS_DATA_PHASE},
	{NAND_CMD_RNDIN, NAND_CMD_NONE, 2, NAND_CMD_NONE},
	{NAND_CMD_ERASE1, NAND_CMD_ERASE2, 3, XNANDPSS_CMD_PHASE},
	{NAND_CMD_RESET, NAND_CMD_NONE, 0, NAND_CMD_NONE},
	{NAND_CMD_PARAM, NAND_CMD_NONE, 1, NAND_CMD_NONE},
	{NAND_CMD_GET_FEATURES, NAND_CMD_NONE, 1, NAND_CMD_NONE},
	{NAND_CMD_SET_FEATURES, NAND_CMD_NONE, 1, NAND_CMD_NONE},
	{NAND_CMD_NONE, NAND_CMD_NONE, 0, 0},
	/* Add all the flash commands supported by the flash device and Linux */
	/* The cache program command is not supported by driver because driver
	 * cant differentiate between page program and cached page program from
	 * start command, these commands can be differentiated through end
	 * command, which doesn't fit in to the driver design. The cache program
	 * command is not supported by NAND subsystem also, look at 1612 line
	 * number (in nand_write_page function) of nand_base.c file.
	 * {NAND_CMD_SEQIN, NAND_CMD_CACHEDPROG, 5, XNANDPSS_YES}, */
};

/* Define default oob placement schemes for large and small page devices */
static struct nand_ecclayout nand_oob_16 = {
	.eccbytes = 3,
	.eccpos = {13, 14, 15},
	.oobfree = {
		{.offset = 0,
		 . length = 12} }
};

static struct nand_ecclayout nand_oob_64 = {
	.eccbytes = 12,
	.eccpos = {
		   52, 53, 54, 55, 56, 57,
		   58, 59, 60, 61, 62, 63},
	.oobfree = {
		{.offset = 2,
		 .length = 50} }
};

static struct nand_ecclayout ondie_nand_oob_64 = {
	.eccbytes = 32,

	.eccpos = {
		8, 9, 10, 11, 12, 13, 14, 15,
		24, 25, 26, 27, 28, 29, 30, 31,
		40, 41, 42, 43, 44, 45, 46, 47,
		56, 57, 58, 59, 60, 61, 62, 63
	},

	.oobfree = {
		{ .offset = 4, .length = 4 },
		{ .offset = 20, .length = 4 },
		{ .offset = 36, .length = 4 },
		{ .offset = 52, .length = 4 }
	}
};

/* Generic flash bbt decriptors
*/
static uint8_t bbt_pattern[] = {'B', 'b', 't', '0' };
static uint8_t mirror_pattern[] = {'1', 't', 'b', 'B' };

static struct nand_bbt_descr bbt_main_descr = {
        .options = NAND_BBT_LASTBLOCK | NAND_BBT_CREATE | NAND_BBT_WRITE
                | NAND_BBT_2BIT | NAND_BBT_VERSION | NAND_BBT_PERCHIP,
        .offs = 4,
        .len = 4,
        .veroffs = 20,
        .maxblocks = 4,
        .pattern = bbt_pattern
};

static struct nand_bbt_descr bbt_mirror_descr = {
        .options = NAND_BBT_LASTBLOCK | NAND_BBT_CREATE | NAND_BBT_WRITE
                | NAND_BBT_2BIT | NAND_BBT_VERSION | NAND_BBT_PERCHIP,
        .offs = 4,
        .len = 4,
        .veroffs = 20,
        .maxblocks = 4,
        .pattern = mirror_pattern
};

/**
 * xnandps_init_nand_flash - Initialize NAND controller
 * @smc_regs:	Virtual address of the NAND controller registers
 * @option:	Device property flags
 *
 * This function initializes the NAND flash interface on the NAND controller.
 **/
static void xnandps_init_nand_flash(void __iomem *smc_regs, int option)
{
	/* disable interrupts */
	xnandps_write32(smc_regs + XSMCPSS_MC_CLR_CONFIG, XNANDPSS_CLR_CONFIG);
	/* Initialize the NAND interface by setting cycles and operation mode */
	xnandps_write32(smc_regs + XSMCPSS_MC_SET_CYCLES, XNANDPSS_SET_CYCLES);
	if (option & NAND_BUSWIDTH_16)
		xnandps_write32(smc_regs + XSMCPSS_MC_SET_OPMODE,
				(XNANDPSS_SET_OPMODE | 0x1));
	else
		xnandps_write32(smc_regs + XSMCPSS_MC_SET_OPMODE,
				XNANDPSS_SET_OPMODE);
	xnandps_write32(smc_regs + XSMCPSS_MC_DIRECT_CMD, XNANDPSS_DIRECT_CMD);

	/* Wait till the ECC operation is complete */
	while ( (xnandps_read32(smc_regs + XSMCPSS_ECC_STATUS_OFFSET(
			XSMCPSS_ECC_IF1_OFFSET))) & XNANDPSS_ECC_BUSY)
			;
	/* Set the command1 and command2 register */
	xnandps_write32(smc_regs +
		(XSMCPSS_ECC_MEMCMD1_OFFSET(XSMCPSS_ECC_IF1_OFFSET)),
		XNANDPSS_ECC_CMD1);
	xnandps_write32(smc_regs +
		(XSMCPSS_ECC_MEMCMD2_OFFSET(XSMCPSS_ECC_IF1_OFFSET)),
		XNANDPSS_ECC_CMD2);
}

/**
 * xnandps_calculate_hwecc - Calculate Hardware ECC
 * @mtd:	Pointer to the mtd_info structure
 * @data:	Pointer to the page data
 * @ecc_code:	Pointer to the ECC buffer where ECC data needs to be stored
 *
 * This function retrieves the Hardware ECC data from the controller and returns
 * ECC data back to the MTD subsystem.
 *
 * returns:	0 on success or error value on failure
 **/
static int
xnandps_calculate_hwecc(struct mtd_info *mtd, const u8 *data, u8 *ecc_code)
{
	struct xnandps_info *xnand;
	struct nand_chip *chip;
	u32 ecc_value = 0;
	u8 ecc_reg, ecc_byte;
	u32 ecc_status;
	
	chip = (struct nand_chip *)mtd->priv;
	xnand = (struct xnandps_info *)chip->priv;

	/* Wait till the ECC operation is complete */
	do {
		ecc_status = xnandps_read32(xnand->smc_regs +
			XSMCPSS_ECC_STATUS_OFFSET(XSMCPSS_ECC_IF1_OFFSET));
	} while (ecc_status & XNANDPSS_ECC_BUSY);

	for (ecc_reg = 0; ecc_reg < 4; ecc_reg++) {
		/* Read ECC value for each block */
		ecc_value = (xnandps_read32(xnand->smc_regs +
			(XSMCPSS_ECC_VALUE0_OFFSET(XSMCPSS_ECC_IF1_OFFSET) +
			(ecc_reg*4))));
		ecc_status = (ecc_value >> 24) & 0xFF;
		/* ECC value valid */
		if (ecc_status & 0x40) {
			for (ecc_byte = 0; ecc_byte < 3; ecc_byte++) {
				/* Copy ECC bytes to MTD buffer */
				*ecc_code = ecc_value & 0xFF;
				ecc_value = ecc_value >> 8;
				ecc_code++;
			}
		} else {
			printk(KERN_INFO "pl350: ecc status failed\n");
		}
	}
	return 0;
}

/**
 * onehot - onehot function
 * @value:	value to check for onehot
 *
 * This function checks whether a value is onehot or not.
 * onehot is if and only if onebit is set.
 *
 **/
int onehot(unsigned short value)
{
	return ((value & (value-1)) == 0);
}

/**
 * xnandps_correct_data - ECC correction function
 * @mtd:	Pointer to the mtd_info structure
 * @buf:	Pointer to the page data
 * @read_ecc:	Pointer to the ECC value read from spare data area
 * @calc_ecc:	Pointer to the calculated ECC value
 *
 * This function corrects the ECC single bit errors & detects 2-bit errors.
 *
 * returns:	0 if no ECC errors found
 *		1 if single bit error found and corrected.
 *		-1 if multiple ECC errors found.
 **/
int xnandps_correct_data(struct mtd_info *mtd, unsigned char *buf,
			unsigned char *read_ecc, unsigned char *calc_ecc)
{
	unsigned char bit_addr;
	unsigned int byte_addr;
	unsigned short ecc_odd, ecc_even;
	unsigned short read_ecc_lower, read_ecc_upper;
	unsigned short calc_ecc_lower, calc_ecc_upper;

	read_ecc_lower = (read_ecc[0] | (read_ecc[1] << 8)) & 0xfff;
	read_ecc_upper = ((read_ecc[1] >> 4) | (read_ecc[2] << 4)) & 0xfff;

	calc_ecc_lower = (calc_ecc[0] | (calc_ecc[1] << 8)) & 0xfff;
	calc_ecc_upper = ((calc_ecc[1] >> 4) | (calc_ecc[2] << 4)) & 0xfff;

	ecc_odd = read_ecc_lower ^ calc_ecc_lower;
	ecc_even = read_ecc_upper ^ calc_ecc_upper;

	if ((ecc_odd == 0) && (ecc_even == 0))
		return 0;       /* no error */
	else if (ecc_odd == (~ecc_even & 0xfff)) {
		/* bits [11:3] of error code is byte offset */
		byte_addr = (ecc_odd >> 3) & 0x1ff;
		/* bits [2:0] of error code is bit offset */
		bit_addr = ecc_odd & 0x7;
		/* Toggling error bit */
		buf[byte_addr] ^= (1 << bit_addr);
		return 1;
	} else if (onehot(ecc_odd | ecc_even) == 1) {
		return 1; /* one error in parity */
	} else {
		return -1; /* Uncorrectable error */
	}
}

/**
 * xnandps_read_oob - [REPLACABLE] the most common OOB data read function
 * @mtd:	mtd info structure
 * @chip:	nand chip info structure
 * @page:	page number to read
 * @sndcmd:	flag whether to issue read command or not
 */
static int xnandps_read_oob(struct mtd_info *mtd, struct nand_chip *chip,
			int page, int sndcmd)
{
	unsigned long data_width = 4;
	unsigned long data_phase_addr = 0;
	uint8_t *p;

	if (sndcmd) {
		chip->cmdfunc(mtd, NAND_CMD_READOOB, 0, page);
		sndcmd = 0;
	}

	p = chip->oob_poi;
	chip->read_buf(mtd, p, (mtd->oobsize - data_width));
	p += (mtd->oobsize - data_width);

	data_phase_addr = (unsigned long __force)chip->IO_ADDR_R;
	data_phase_addr |= XNANDPSS_CLEAR_CS;
	chip->IO_ADDR_R = (void __iomem *__force)data_phase_addr;
	chip->read_buf(mtd, p, data_width);

	return sndcmd;
}

/**
 * xnandps_write_oob - [REPLACABLE] the most common OOB data write function
 * @mtd:	mtd info structure
 * @chip:	nand chip info structure
 * @page:	page number to write
 */
static int xnandps_write_oob(struct mtd_info *mtd, struct nand_chip *chip,
			int page)
{
	int status = 0;
	const uint8_t *buf = chip->oob_poi;
	unsigned long data_width = 4;
	unsigned long data_phase_addr = 0;

	chip->cmdfunc(mtd, NAND_CMD_SEQIN, mtd->writesize, page);

	chip->write_buf(mtd, buf, (mtd->oobsize - data_width));
	buf += (mtd->oobsize - data_width);

	data_phase_addr = (unsigned long __force)chip->IO_ADDR_W;
	data_phase_addr |= XNANDPSS_CLEAR_CS;
	data_phase_addr |= (1 << END_CMD_VALID_SHIFT);
	chip->IO_ADDR_W = (void __iomem *__force)data_phase_addr;
	chip->write_buf(mtd, buf, data_width);

	/* Send command to program the OOB data */
	chip->cmdfunc(mtd, NAND_CMD_PAGEPROG, -1, -1);
	status = chip->waitfunc(mtd, chip);

	return status & NAND_STATUS_FAIL ? -EIO : 0;
}

/**
 * xnandps_read_page_raw - [Intern] read raw page data without ecc
 * @mtd:        mtd info structure
 * @chip:       nand chip info structure
 * @buf:        buffer to store read data
 * @page:       page number to read
 *
 */
static int xnandps_read_page_raw(struct mtd_info *mtd, struct nand_chip *chip,
			      uint8_t *buf, int page)
{
	unsigned long data_width = 4;
	unsigned long data_phase_addr = 0;
	uint8_t *p;

	chip->read_buf(mtd, buf, mtd->writesize);

	p = chip->oob_poi;
	chip->read_buf(mtd, p, (mtd->oobsize - data_width));
	p += (mtd->oobsize - data_width);

	data_phase_addr = (unsigned long __force)chip->IO_ADDR_R;
	data_phase_addr |= XNANDPSS_CLEAR_CS;
	chip->IO_ADDR_R = (void __iomem *__force)data_phase_addr;

	chip->read_buf(mtd, p, data_width);
	return 0;
}

/**
 * xnandps_write_page_raw - [Intern] raw page write function
 * @mtd:        mtd info structure
 * @chip:       nand chip info structure
 * @buf:        data buffer
 *
 */
static void xnandps_write_page_raw(struct mtd_info *mtd,
			struct nand_chip *chip, const uint8_t *buf)
{
	unsigned long data_width = 4;
	unsigned long data_phase_addr = 0;
	uint8_t *p;

	chip->write_buf(mtd, buf, mtd->writesize);

	p = chip->oob_poi;
	chip->write_buf(mtd, p, (mtd->oobsize - data_width));
	p += (mtd->oobsize - data_width);

	data_phase_addr = (unsigned long __force)chip->IO_ADDR_W;
	data_phase_addr |= XNANDPSS_CLEAR_CS;
	data_phase_addr |= (1 << END_CMD_VALID_SHIFT);
	chip->IO_ADDR_W = (void __iomem *__force)data_phase_addr;

	chip->write_buf(mtd, p, data_width);
}

/**
 * nand_write_page_hwecc - Hardware ECC based page write function
 * @mtd:	Pointer to the mtd info structure
 * @chip:	Pointer to the NAND chip info structure
 * @buf:	Pointer to the data buffer
 *
 * This functions writes data and hardware generated ECC values in to the page.
 */
void xnandps_write_page_hwecc(struct mtd_info *mtd,
				struct nand_chip *chip, const uint8_t *buf)
{
	int i, eccsize = chip->ecc.size;
	int eccsteps = chip->ecc.steps;
	uint8_t *ecc_calc = chip->buffers->ecccalc;
	const uint8_t *p = buf;
	uint32_t *eccpos = chip->ecc.layout->eccpos;
	unsigned long data_phase_addr = 0;
	unsigned long data_width = 4;
	uint8_t *oob_ptr;

	for ( ; (eccsteps - 1); eccsteps--) {
		chip->write_buf(mtd, p, eccsize);
		p += eccsize;
	}
	chip->write_buf(mtd, p, (eccsize - data_width));
	p += (eccsize - data_width);

	/* Set ECC Last bit to 1 */
	data_phase_addr = (unsigned long __force)chip->IO_ADDR_W;
	data_phase_addr |= XNANDPSS_ECC_LAST;
	chip->IO_ADDR_W = (void __iomem *__force)data_phase_addr;
	chip->write_buf(mtd, p, data_width);

	/* Wait for ECC to be calculated and read the error values */
	p = buf;
	chip->ecc.calculate(mtd, p, &ecc_calc[0]);

	for (i = 0; i < chip->ecc.total; i++)
		chip->oob_poi[eccpos[i]] = ~(ecc_calc[i]);

	/* Clear ECC last bit */
	data_phase_addr = (unsigned long __force)chip->IO_ADDR_W;
	data_phase_addr &= ~XNANDPSS_ECC_LAST;
	chip->IO_ADDR_W = (void __iomem *__force)data_phase_addr;

	/* Write the spare area with ECC bytes */
	oob_ptr = chip->oob_poi;
	chip->write_buf(mtd, oob_ptr, (mtd->oobsize - data_width));

	data_phase_addr = (unsigned long __force)chip->IO_ADDR_W;
	data_phase_addr |= XNANDPSS_CLEAR_CS;
	data_phase_addr |= (1 << END_CMD_VALID_SHIFT);
	chip->IO_ADDR_W = (void __iomem *__force)data_phase_addr;
	oob_ptr += (mtd->oobsize - data_width);
	chip->write_buf(mtd, oob_ptr, data_width);
}

/**
 * xnandps_write_page_swecc - [REPLACABLE] software ecc based page write function
 * @mtd:	mtd info structure
 * @chip:	nand chip info structure
 * @buf:	data buffer
 */
static void xnandps_write_page_swecc(struct mtd_info *mtd,
						struct nand_chip *chip, const uint8_t *buf)
{
	int i, eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	int eccsteps = chip->ecc.steps;
	uint8_t *ecc_calc = chip->buffers->ecccalc;
	const uint8_t *p = buf;
	uint32_t *eccpos = chip->ecc.layout->eccpos;

	/* Software ecc calculation */
	for (i = 0; eccsteps; eccsteps--, i += eccbytes, p += eccsize)
		chip->ecc.calculate(mtd, p, &ecc_calc[i]);

	for (i = 0; i < chip->ecc.total; i++)
		chip->oob_poi[eccpos[i]] = ecc_calc[i];

	chip->ecc.write_page_raw(mtd, chip, buf);
}

/**
 * nand_read_page_hwecc - Hardware ECC based page read function
 * @mtd:	Pointer to the mtd info structure
 * @chip:	Pointer to the NAND chip info structure
 * @buf:	Pointer to the buffer to store read data
 * @page:	page number to read
 *
 * This functions reads data and checks the data integrity by comparing hardware
 * generated ECC values and read ECC values from spare area.
 *
 * returns:	0 always and updates ECC operation status in to MTD structure
 */
int xnandps_read_page_hwecc(struct mtd_info *mtd,
				struct nand_chip *chip, uint8_t *buf, int page)
{
	int i, stat, eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	int eccsteps = chip->ecc.steps;
	uint8_t *p = buf;
	uint8_t *ecc_calc = chip->buffers->ecccalc;
	uint8_t *ecc_code = chip->buffers->ecccode;
	uint32_t *eccpos = chip->ecc.layout->eccpos;
	unsigned long data_phase_addr = 0;
	unsigned long data_width = 4;
	uint8_t *oob_ptr;

	for ( ; (eccsteps - 1); eccsteps--) {
		chip->read_buf(mtd, p, eccsize);
		p += eccsize;
	}
	chip->read_buf(mtd, p, (eccsize - data_width));
	p += (eccsize - data_width);

	/* Set ECC Last bit to 1 */
	data_phase_addr = (unsigned long __force)chip->IO_ADDR_R;
	data_phase_addr |= XNANDPSS_ECC_LAST;
	chip->IO_ADDR_R = (void __iomem *__force)data_phase_addr;
	chip->read_buf(mtd, p, data_width);

	/* Read the calculated ECC value */
	p = buf;
	chip->ecc.calculate(mtd, p, &ecc_calc[0]);

	/* Clear ECC last bit */
	data_phase_addr = (unsigned long __force)chip->IO_ADDR_R;
	data_phase_addr &= ~XNANDPSS_ECC_LAST;
	chip->IO_ADDR_R = (void __iomem *__force)data_phase_addr;

	/* Read the stored ECC value */
	oob_ptr = chip->oob_poi;
	chip->read_buf(mtd, oob_ptr, (mtd->oobsize - data_width));

	/* de-assert chip select */
	data_phase_addr = (unsigned long __force)chip->IO_ADDR_R;
	data_phase_addr |= XNANDPSS_CLEAR_CS;
	chip->IO_ADDR_R = (void __iomem *__force)data_phase_addr;

	oob_ptr += (mtd->oobsize - data_width);
	chip->read_buf(mtd, oob_ptr, data_width);

	for (i = 0; i < chip->ecc.total; i++)
		ecc_code[i] = ~(chip->oob_poi[eccpos[i]]);

	eccsteps = chip->ecc.steps;
	p = buf;

	/* Check ECC error for all blocks and correct if it is correctable */
	for (i = 0 ; eccsteps; eccsteps--, i += eccbytes, p += eccsize) {
		stat = chip->ecc.correct(mtd, p, &ecc_code[i], &ecc_calc[i]);
		if (stat < 0)
			mtd->ecc_stats.failed++;
		else
			mtd->ecc_stats.corrected += stat;
	}
	return 0;
}

/**
 * xnandps_read_page_swecc - [REPLACABLE] software ecc based page read function
 * @mtd:	mtd info structure
 * @chip:	nand chip info structure
 * @buf:	buffer to store read data
 * @page:	page number to read
 */
static int xnandps_read_page_swecc(struct mtd_info *mtd,
				struct nand_chip *chip, uint8_t *buf, int page)
{
	int i, eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	int eccsteps = chip->ecc.steps;
	uint8_t *p = buf;
	uint8_t *ecc_calc = chip->buffers->ecccalc;
	uint8_t *ecc_code = chip->buffers->ecccode;
	uint32_t *eccpos = chip->ecc.layout->eccpos;

	chip->ecc.read_page_raw(mtd, chip, buf, page);

	for (i = 0; eccsteps; eccsteps--, i += eccbytes, p += eccsize)
		chip->ecc.calculate(mtd, p, &ecc_calc[i]);

	for (i = 0; i < chip->ecc.total; i++)
		ecc_code[i] = chip->oob_poi[eccpos[i]];

	eccsteps = chip->ecc.steps;
	p = buf;

	for (i = 0 ; eccsteps; eccsteps--, i += eccbytes, p += eccsize) {
		int stat;

		stat = chip->ecc.correct(mtd, p, &ecc_code[i], &ecc_calc[i]);
		if (stat < 0)
			mtd->ecc_stats.failed++;
		else
			mtd->ecc_stats.corrected += stat;
	}
	return 0;
}

/**
 * xnandps_select_chip - Select the flash device
 * @mtd:	Pointer to the mtd_info structure
 * @chip:	Chip number to be selected
 *
 * This function is empty as the NAND controller handles chip select line
 * internally based on the chip address passed in command and data phase.
 **/
static void xnandps_select_chip(struct mtd_info *mtd, int chip)
{
	return;
}

/**
 * xnandps_cmd_function - Send command to NAND device
 * @mtd:	Pointer to the mtd_info structure
 * @command:	The command to be sent to the flash device
 * @column:	The column address for this command, -1 if none
 * @page_addr:	The page address for this command, -1 if none
 */
static void xnandps_cmd_function(struct mtd_info *mtd, unsigned int command,
			    int column, int page_addr)
{
	struct nand_chip *chip = mtd->priv;
	struct xnandps_command_format *curr_cmd = NULL;
	struct xnandps_info *xnand;
	void *cmd_addr;
	unsigned long cmd_data = 0;
	unsigned long cmd_phase_addr = 0;
	unsigned long data_phase_addr = 0;
	unsigned long end_cmd = 0;
	unsigned long end_cmd_valid = 0;
	unsigned long i;

	xnand = (struct xnandps_info *)chip->priv;
	if (xnand->end_cmd_pending) {
		/* Check for end command if this command request is same as the
		 * pending command then return */
		if (xnand->end_cmd == command) {
			xnand->end_cmd = 0;
			xnand->end_cmd_pending = 0;
			return;
		}
	}

	/* Emulate NAND_CMD_READOOB for large page device */
	if ((mtd->writesize > XNANDPSS_ECC_SIZE) &&
		(command == NAND_CMD_READOOB)) {
		column += mtd->writesize;
		command = NAND_CMD_READ0;
	}

	/* Get the command format */
	for (i = 0; (xnandps_commands[i].start_cmd != NAND_CMD_NONE ||
		xnandps_commands[i].end_cmd != NAND_CMD_NONE); i++) {
		if (command == xnandps_commands[i].start_cmd)
			curr_cmd = &xnandps_commands[i];
	}
	if (curr_cmd == NULL)
		return;

	/* Clear interrupt */
	xnandps_write32((xnand->smc_regs + XSMCPSS_MC_CLR_CONFIG), (1 << 4));

	/* Get the command phase address */
	if (curr_cmd->end_cmd_valid == XNANDPSS_CMD_PHASE)
		end_cmd_valid = 1;

	if (curr_cmd->end_cmd == NAND_CMD_NONE)
		end_cmd = 0x0;
	else
		end_cmd = curr_cmd->end_cmd;

	cmd_phase_addr = (unsigned long __force)xnand->nand_base	|
			(curr_cmd->addr_cycles << ADDR_CYCLES_SHIFT)	|
			(end_cmd_valid << END_CMD_VALID_SHIFT)		|
			(COMMAND_PHASE)					|
			(end_cmd << END_CMD_SHIFT)			|
			(curr_cmd->start_cmd << START_CMD_SHIFT);

	cmd_addr = (void __iomem * __force)cmd_phase_addr;

	/* Get the data phase address */
	end_cmd_valid = 0;

	data_phase_addr = (unsigned long __force)xnand->nand_base	|
			(0x0 << CLEAR_CS_SHIFT)				|
			(end_cmd_valid << END_CMD_VALID_SHIFT)		|
			(DATA_PHASE)					|
			(end_cmd << END_CMD_SHIFT)			|
			(0x0 << ECC_LAST_SHIFT);

	chip->IO_ADDR_R = (void  __iomem * __force)data_phase_addr;
	chip->IO_ADDR_W = chip->IO_ADDR_R;

	/* Command phase AXI write */
	/* Read & Write */
	if (column != -1 && page_addr != -1) {
		/* Adjust columns for 16 bit bus width */
		if (chip->options & NAND_BUSWIDTH_16)
			column >>= 1;
		cmd_data = column;
		if (mtd->writesize > XNANDPSS_ECC_SIZE) {
			cmd_data |= page_addr << 16;
			/* Another address cycle for devices > 128MiB */
			if (chip->chipsize > (128 << 20)) {
				xnandps_write32(cmd_addr, cmd_data);
				cmd_data = (page_addr >> 16);
			}
		} else
			cmd_data |= page_addr << 8;
	}
	/* Erase */
	else if (page_addr != -1)
		cmd_data = page_addr;
	/* Change read/write column, read id etc */
	else if (column != -1) {
		/* Adjust columns for 16 bit bus width */
		if (chip->options & NAND_BUSWIDTH_16)
			column >>= 1;
		cmd_data = column;
	} else
		;

	xnandps_write32(cmd_addr, cmd_data);

	if (curr_cmd->end_cmd_valid) {
		xnand->end_cmd = curr_cmd->end_cmd;
		xnand->end_cmd_pending = 1;
	}

	ndelay(100);

	if ((command == NAND_CMD_READ0) || (command == NAND_CMD_RESET)) {
		while (!chip->dev_ready(mtd))
				;
		return;
	}
}

/**
 * xnandps_read_buf - read chip data into buffer
 * @mtd:        MTD device structure
 * @buf:        buffer to store date
 * @len:        number of bytes to read
 *
 */
void xnandps_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	int i;
	struct nand_chip *chip = mtd->priv;
	unsigned long *ptr = (unsigned long *)buf;

	len >>= 2;
	for (i = 0; i < len; i++)
		ptr[i] = readl(chip->IO_ADDR_R);
}

/**
 * xnandps_write_buf - write buffer to chip
 * @mtd:        MTD device structure
 * @buf:        data buffer
 * @len:        number of bytes to write
 *
 */
void xnandps_write_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	int i;
	struct nand_chip *chip = mtd->priv;
	unsigned long *ptr = (unsigned long *)buf;
	len >>= 2;

	for (i = 0; i < len; i++)
		writel(ptr[i], chip->IO_ADDR_W);
}

/**
 * xnandps_verify_buf - Verify chip data against buffer
 * @mtd:        MTD device structure
 * @buf:        buffer containing the data to compare
 * @len:        number of bytes to compare
 *
 */
static int xnandps_verify_buf(struct mtd_info *mtd, const uint8_t *buf,
				int len)
{
	int i;
	struct nand_chip *chip = mtd->priv;
	unsigned long *ptr = (unsigned long *)buf;
	unsigned long addr;

	len >>= 2;
	for (i = 0; i < (len - 1); i++) {
		if (ptr[i] != readl(chip->IO_ADDR_R))
			return -EFAULT;
	}
	addr = (unsigned long __force)chip->IO_ADDR_R;
	addr |= XNANDPSS_CLEAR_CS;
	chip->IO_ADDR_R = (void __iomem *__force)addr;
	if (ptr[i] != readl(chip->IO_ADDR_R))
		return -EFAULT;
	return 0;
}

/**
 * xnandps_device_ready - Check device ready/busy line
 * @mtd:	Pointer to the mtd_info structure
 *
 * returns:	0 on busy or 1 on ready state
 **/
static int xnandps_device_ready(struct mtd_info *mtd)
{
	struct xnandps_info *xnand;
	struct nand_chip *chip;
	unsigned long status;

	chip = (struct nand_chip *)mtd->priv;
	xnand = (struct xnandps_info *)chip->priv;

	/* Check the raw_int_status1 bit */
	status = xnandps_read32(xnand->smc_regs + XSMCPSS_MC_STATUS) & 0x40;
	/* Clear the interrupt condition */
	if (status)
		xnandps_write32((xnand->smc_regs + XSMCPSS_MC_CLR_CONFIG),
					(1<<4));
	return status ? 1 : 0;
}

/**
 * xnandps_probe - Probe method for the NAND driver
 * @pdev:	Pointer to the platform_device structure
 *
 * This function initializes the driver data structures and the hardware.
 *
 * returns:	0 on success or error value on failure
 **/
#ifdef LINUX_ONLY_NOT_UBOOT
static int __init xnandps_probe(struct platform_device *pdev)
#else
int zynq_nand_init(struct nand_chip *nand_chip)
#endif
{
	struct xnandps_info *xnand;
	struct mtd_info *mtd;
#ifdef LINUX_ONLY_NOT_UBOOT
	struct nand_chip *nand_chip;
	struct resource *nand_res, *smc_res;
#else
	extern struct mtd_info nand_info[];
#endif
	unsigned long ecc_page_size;
	int err = 0;
#ifdef CONFIG_MTD_PARTITIONS
	int  nr_parts;
	static const char *part_probe_types[] = {"cmdlinepart", NULL};
#endif
	u8 maf_id, dev_id;
	u8 get_feature[4];
	u8 set_feature[4] = {0x08, 0x00, 0x00, 0x00};
	unsigned long ecc_cfg;
	int ondie_ecc_enabled = 0;
	int ez_nand_supported = 0;

#ifdef LINUX_ONLY_NOT_UBOOT
	xnand = kzalloc(sizeof(struct xnandps_info), GFP_KERNEL);
#else
	xnand = malloc(sizeof(struct xnandps_info));
	memset(xnand, 0, sizeof(struct xnandps_info));
#endif
	if (!xnand) {
		dev_err(&pdev->dev, "failed to allocate device structure.\n");
		return -ENOMEM;
	}

#ifdef LINUX_ONLY_NOT_UBOOT
	/* Map physical address of NAND flash */
	nand_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (nand_res == NULL) {
		err = -ENODEV;
		dev_err(&pdev->dev, "platform_get_resource for NAND failed\n");
		goto out_free_data;
	}

	nand_res = request_mem_region(nand_res->start, resource_size(nand_res),
					pdev->name);
	if (nand_res == NULL) {
		err = -ENOMEM;
		dev_err(&pdev->dev, "request_mem_region for cont failed\n");
		goto out_free_data;
	}

	xnand->nand_base = ioremap(nand_res->start, resource_size(nand_res));
	if (xnand->nand_base == NULL) {
		err = -EIO;
		dev_err(&pdev->dev, "ioremap for NAND failed\n");
		goto out_release_nand_mem_region;
	}

	/* Get the NAND controller virtual address */
	smc_res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (smc_res == NULL) {
		err = -ENODEV;
		dev_err(&pdev->dev, "platform_get_resource for cont failed\n");
		goto out_nand_iounmap;
	}

	smc_res = request_mem_region(smc_res->start, resource_size(smc_res),
					pdev->name);
	if (smc_res == NULL) {
		err = -ENOMEM;
		dev_err(&pdev->dev, "request_mem_region for cont failed\n");
		goto out_nand_iounmap;
	}

	xnand->smc_regs = ioremap(smc_res->start, resource_size(smc_res));
	if (!xnand->smc_regs) {
		err = -EIO;
		dev_err(&pdev->dev, "ioremap for cont failed\n");
		goto out_release_smc_mem_region;
	}

	/* Link the private data with the MTD structure */
	mtd = &xnand->mtd;
	nand_chip = &xnand->chip;
#else
	xnand->smc_regs = (void*)XPSS_CRTL_PARPORT_BASEADDR;
	xnand->nand_base = (void*)XPSS_NAND_BASEADDR;
	mtd = &nand_info[0];
#endif

	nand_chip->priv = xnand;
	mtd->priv = nand_chip;
#ifdef LINUX_ONLY_NOT_UBOOT
	mtd->owner = THIS_MODULE;
	mtd->name = "xilinx_nand";
#endif

	/* Set address of NAND IO lines */
	nand_chip->IO_ADDR_R = xnand->nand_base;
	nand_chip->IO_ADDR_W = xnand->nand_base;

	/* Set the driver entry points for MTD */
	nand_chip->cmdfunc = xnandps_cmd_function;
	//nand_chip->dev_ready = NULL;
	nand_chip->dev_ready = xnandps_device_ready;
	nand_chip->select_chip = xnandps_select_chip;

	/* If we don't set this delay driver sets 20us by default */
	nand_chip->chip_delay = 30;

	/* Buffer read/write routines */
	nand_chip->read_buf = xnandps_read_buf;
	nand_chip->write_buf = xnandps_write_buf;
	nand_chip->verify_buf = xnandps_verify_buf;

	/* Set the device option and flash width */
#ifdef LINUX_ONLY_NOT_UBOOT
	nand_chip->options = *((u32 *)pdev->dev.platform_data);
#else
	/* arch/arm/mach-xilinx/devices.c */
	nand_chip->options = NAND_NO_AUTOINCR | NAND_USE_FLASH_BBT;
#endif

#ifdef LINUX_ONLY_NOT_UBOOT
	platform_set_drvdata(pdev, xnand);
#endif

	/* Initialize the NAND flash interface on NAND controller */
	xnandps_init_nand_flash(xnand->smc_regs, nand_chip->options);

	/* first scan to find the device and get the page size */
	if (nand_scan_ident(mtd, 1, NULL)) {
		err = -ENXIO;
		dev_err(&pdev->dev, "nand_scan_ident for NAND failed\n");
		goto out_unmap_all_mem;
	}

	/* Send the command for reading device ID */
	nand_chip->cmdfunc(mtd, NAND_CMD_READID, 0x00, -1);

	/* Read manufacturer and device IDs */
	maf_id = nand_chip->read_byte(mtd);
	dev_id = nand_chip->read_byte(mtd);

	if ((maf_id == 0x2c) && ((dev_id == 0xf1) || (dev_id == 0xa1) || (dev_id == 0xb1) ||
				(dev_id == 0xaa) || (dev_id == 0xba) ||
				(dev_id == 0xda) || (dev_id == 0xca) ||
				(dev_id == 0xac) || (dev_id == 0xbc) ||
				(dev_id == 0xdc) || (dev_id == 0xcc) ||
				(dev_id == 0xa3) || (dev_id == 0xb3) ||
				(dev_id == 0xd3) || (dev_id == 0xc3))) {
		nand_chip->cmdfunc(mtd, NAND_CMD_SET_FEATURES,
						ONDIE_ECC_FEATURE_ADDR, -1);
		nand_chip->write_buf(mtd, set_feature, 4);

		nand_chip->cmdfunc(mtd, NAND_CMD_GET_FEATURES,
						ONDIE_ECC_FEATURE_ADDR, -1);
		nand_chip->read_buf(mtd, get_feature, 4);

		if (get_feature[0] & 0x08) {
			printf("OnDie ECC flash: ");
			ondie_ecc_enabled = 1;
		}
	} else if ((nand_chip->onfi_version == 23) &&
				(nand_chip->onfi_params.features & (1 << 9))) {
		ez_nand_supported = 1;
	}

	if (ondie_ecc_enabled || ez_nand_supported) {
		/* bypass the controller ECC block */
		ecc_cfg = xnandps_read32(xnand->smc_regs +
			XSMCPSS_ECC_MEMCFG_OFFSET(XSMCPSS_ECC_IF1_OFFSET));
		ecc_cfg &= ~0xc;
		xnandps_write32(xnand->smc_regs +
			(XSMCPSS_ECC_MEMCFG_OFFSET(XSMCPSS_ECC_IF1_OFFSET)),
			ecc_cfg);

		/* The software ECC routines won't work with the
				SMC controller */
		nand_chip->ecc.mode = NAND_ECC_HW;
		nand_chip->ecc.read_page = xnandps_read_page_raw;
		nand_chip->ecc.write_page = xnandps_write_page_raw;
		nand_chip->ecc.read_page_raw = xnandps_read_page_raw;
		nand_chip->ecc.write_page_raw = xnandps_write_page_raw;
		nand_chip->ecc.read_oob = xnandps_read_oob;
		nand_chip->ecc.write_oob = xnandps_write_oob;
		nand_chip->ecc.size = mtd->writesize;
		nand_chip->ecc.bytes = 0;

		/* On-Die ECC spare bytes offset 8 is used for ECC codes */
		if (ondie_ecc_enabled) {
			nand_chip->ecc.layout = &ondie_nand_oob_64;
			/* Use the BBT pattern descriptors */
			nand_chip->bbt_td = &bbt_main_descr;
			nand_chip->bbt_md = &bbt_mirror_descr;
		}
	} else {
		/* Hardware ECC generates 3 bytes ECC code for each 512 bytes */
		nand_chip->ecc.mode = NAND_ECC_HW;
		nand_chip->ecc.size = XNANDPSS_ECC_SIZE;
		nand_chip->ecc.bytes = 3;
		nand_chip->ecc.calculate = xnandps_calculate_hwecc;
		nand_chip->ecc.correct = xnandps_correct_data;
		nand_chip->ecc.hwctl = NULL;
		nand_chip->ecc.read_page = xnandps_read_page_hwecc;
		nand_chip->ecc.write_page = xnandps_write_page_hwecc;
		nand_chip->ecc.read_page_raw = xnandps_read_page_raw;
		nand_chip->ecc.write_page_raw = xnandps_write_page_raw;
		nand_chip->ecc.read_oob = xnandps_read_oob;
		nand_chip->ecc.write_oob = xnandps_write_oob;

		switch (mtd->writesize) {
		case 512:
			ecc_page_size = 0x1;
			/* Set the ECC memory config register */
			xnandps_write32(xnand->smc_regs +
				(XSMCPSS_ECC_MEMCFG_OFFSET(XSMCPSS_ECC_IF1_OFFSET)),
				(XNANDPSS_ECC_CONFIG | ecc_page_size));
			break;
		case 1024:
			ecc_page_size = 0x2;
			/* Set the ECC memory config register */
			xnandps_write32(xnand->smc_regs +
				(XSMCPSS_ECC_MEMCFG_OFFSET(XSMCPSS_ECC_IF1_OFFSET)),
				(XNANDPSS_ECC_CONFIG | ecc_page_size));
			break;
		case 2048:
			ecc_page_size = 0x3;
			/* Set the ECC memory config register */
			xnandps_write32(xnand->smc_regs +
				(XSMCPSS_ECC_MEMCFG_OFFSET(XSMCPSS_ECC_IF1_OFFSET)),
				(XNANDPSS_ECC_CONFIG | ecc_page_size));
			break;
		default:
			/* The software ECC routines won't work with the
					SMC controller */
			nand_chip->ecc.mode = NAND_ECC_HW;
			nand_chip->ecc.calculate = nand_calculate_ecc;
			nand_chip->ecc.correct = nand_correct_data;
			nand_chip->ecc.read_page = xnandps_read_page_swecc;
			/* nand_chip->ecc.read_subpage = nand_read_subpage; */
			nand_chip->ecc.write_page = xnandps_write_page_swecc;
			nand_chip->ecc.read_page_raw = xnandps_read_page_raw;
			nand_chip->ecc.write_page_raw = xnandps_write_page_raw;
			nand_chip->ecc.read_oob = xnandps_read_oob;
			nand_chip->ecc.write_oob = xnandps_write_oob;
			nand_chip->ecc.size = 256;
			nand_chip->ecc.bytes = 3;
			break;
		}

		if (mtd->oobsize == 16)
			nand_chip->ecc.layout = &nand_oob_16;
		else if (mtd->oobsize == 64)
			nand_chip->ecc.layout = &nand_oob_64;
		else
			;
	}

	/* second phase scan */
	if (nand_scan_tail(mtd)) {
		err = -ENXIO;
		dev_err(&pdev->dev, "nand_scan_tail for NAND failed\n");
		goto out_unmap_all_mem;
	}

#ifdef CONFIG_MTD_PARTITIONS
#ifdef CONFIG_MTD_CMDLINE_PARTS
	/* Get the partition information from command line argument */
	nr_parts = parse_mtd_partitions(mtd, part_probe_types,
					&xnand->parts, 0);
	if (nr_parts > 0) {
		dev_info(&pdev->dev, "found %d partitions in command line",
				nr_parts);
		add_mtd_partitions(mtd, xnand->parts, nr_parts);
		return 0;
	} else if (pdata->parts)
		add_mtd_partitions(&xnand->mtd, pdata->parts, pdata->nr_parts);
	else {
#endif
		dev_info(&pdev->dev,
			"Command line partition table is not available\n"
			"or command line partition option is not enabled\n"
			"creating single partition on flash\n");
#endif
		err = add_mtd_device(mtd);
#ifdef CONFIG_MTD_CMDLINE_PARTS
	}
#endif
	if (!err) {
#ifdef LINUX_ONLY_NOT_UBOOT
		dev_info(&pdev->dev, "at 0x%08X mapped to 0x%08X\n",
				smc_res->start, (u32 __force) xnand->nand_base);
#endif
		return 0;
	}
printf("%s: ERROR *********** %d\n", __FUNCTION__, err);

	nand_release(mtd);
out_unmap_all_mem:
#ifdef LINUX_ONLY_NOT_UBOOT
	platform_set_drvdata(pdev, NULL);
	iounmap(xnand->smc_regs);
out_release_smc_mem_region:
	release_mem_region(smc_res->start, resource_size(smc_res));
out_nand_iounmap:
	iounmap(xnand->nand_base);
out_release_nand_mem_region:
	release_mem_region(nand_res->start, resource_size(nand_res));
out_free_data:
#endif
	kfree(xnand);
	return err;
}

/**
 * xnandps_remove - Remove method for the NAND driver
 * @pdev:	Pointer to the platform_device structure
 *
 * This function is called if the driver module is being unloaded. It frees all
 * resources allocated to the device.
 *
 * returns:	0 on success or error value on failure
 **/
#ifdef LINUX_ONLY_NOT_UBOOT
static int __devexit xnandps_remove(struct platform_device *pdev)
{
	struct xnandps_info *xnand = platform_get_drvdata(pdev);
	struct resource *nand_res, *smc_res;

	/* Release resources, unregister device */
	nand_release(&xnand->mtd);
#ifdef CONFIG_MTD_PARTITIONS
	/* kfree(NULL) is safe */
	kfree(xnand->parts);
#endif
	platform_set_drvdata(pdev, NULL);
	/* Unmap and release physical address */
	iounmap(xnand->smc_regs);
	smc_res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	release_mem_region(smc_res->start, resource_size(smc_res));

	iounmap(xnand->nand_base);
	nand_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	release_mem_region(nand_res->start, resource_size(nand_res));
	/* Free the MTD device structure */
	kfree(xnand);
	return 0;
}

/*
 * xnandps_driver - This structure defines the NAND subsystem platform driver
 */
static struct platform_driver xnandps_driver = {
	.probe		= xnandps_probe,
	.remove		= __devexit_p(xnandps_remove),
	.suspend	= NULL,
	.resume		= NULL,
	.driver		= {
		.name	= XNANDPSS_DRIVER_NAME,
		.owner	= THIS_MODULE,
	},
};

/**
 * xnandps_init - NAND driver module initialization function
 *
 * returns:	0 on success and error value on failure
 **/
static int __init xnandps_init(void)
{
	return platform_driver_register(&xnandps_driver);
}

/**
 * xnandps_exit - NAND driver module exit function
 **/
static void __exit xnandps_exit(void)
{
	platform_driver_unregister(&xnandps_driver);
}

module_init(xnandps_init);
module_exit(xnandps_exit);

MODULE_AUTHOR("Xilinx, Inc.");
MODULE_ALIAS("platform:" XNANDPSS_DRIVER_NAME);
MODULE_DESCRIPTION("Xilinx PSS NAND Flash Driver");
MODULE_LICENSE("GPL");

#endif
