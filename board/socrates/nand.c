/*
 * (C) Copyright 2008
 * Sergei Poselenov, Emcraft Systems, sposelenov@emcraft.com.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.         See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>

#if defined(CONFIG_SYS_NAND_BASE)
#include <nand.h>
#include <asm/errno.h>
#include <asm/io.h>

static int state;
static void nand_write_byte(struct mtd_info *mtd, u_char byte);
static void nand_write_buf(struct mtd_info *mtd, const u_char *buf, int len);
static u_char nand_read_byte(struct mtd_info *mtd);
static u16 nand_read_word(struct mtd_info *mtd);
static void nand_read_buf(struct mtd_info *mtd, u_char *buf, int len);
static int nand_verify_buf(struct mtd_info *mtd, const u_char *buf, int len);
static int nand_device_ready(struct mtd_info *mtdinfo);

#define FPGA_NAND_CMD_MASK		(0x7 << 28)
#define FPGA_NAND_CMD_COMMAND		(0x0 << 28)
#define FPGA_NAND_CMD_ADDR		(0x1 << 28)
#define FPGA_NAND_CMD_READ		(0x2 << 28)
#define FPGA_NAND_CMD_WRITE		(0x3 << 28)
#define FPGA_NAND_BUSY			(0x1 << 15)
#define FPGA_NAND_ENABLE		(0x1 << 31)
#define FPGA_NAND_DATA_SHIFT		16

/**
 * nand_write_byte -  write one byte to the chip
 * @mtd:	MTD device structure
 * @byte:	pointer to data byte to write
 */
static void nand_write_byte(struct mtd_info *mtd, u_char byte)
{
	nand_write_buf(mtd, (const uchar *)&byte, sizeof(byte));
}

/**
 * nand_write_buf -  write buffer to chip
 * @mtd:	MTD device structure
 * @buf:	data buffer
 * @len:	number of bytes to write
 */
static void nand_write_buf(struct mtd_info *mtd, const u_char *buf, int len)
{
	int i;
	struct nand_chip *this = mtd->priv;

	for (i = 0; i < len; i++) {
		out_be32(this->IO_ADDR_W,
			 state | (buf[i] << FPGA_NAND_DATA_SHIFT));
	}
}


/**
 * nand_read_byte -  read one byte from the chip
 * @mtd:	MTD device structure
 */
static u_char nand_read_byte(struct mtd_info *mtd)
{
	u8 byte;
	nand_read_buf(mtd, (uchar *)&byte, sizeof(byte));
	return byte;
}

/**
 * nand_read_word -  read one word from the chip
 * @mtd:	MTD device structure
 */
static u16 nand_read_word(struct mtd_info *mtd)
{
	u16 word;
	nand_read_buf(mtd, (uchar *)&word, sizeof(word));
	return word;
}

/**
 * nand_read_buf -  read chip data into buffer
 * @mtd:	MTD device structure
 * @buf:	buffer to store date
 * @len:	number of bytes to read
 */
static void nand_read_buf(struct mtd_info *mtd, u_char *buf, int len)
{
	int i;
	struct nand_chip *this = mtd->priv;
	int val;

	val = (state & FPGA_NAND_ENABLE) | FPGA_NAND_CMD_READ;

	out_be32(this->IO_ADDR_W, val);
	for (i = 0; i < len; i++) {
		buf[i] = (in_be32(this->IO_ADDR_R) >> FPGA_NAND_DATA_SHIFT) & 0xff;
	}
}

/**
 * nand_verify_buf -  Verify chip data against buffer
 * @mtd:	MTD device structure
 * @buf:	buffer containing the data to compare
 * @len:	number of bytes to compare
 */
static int nand_verify_buf(struct mtd_info *mtd, const u_char *buf, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		if (buf[i] != nand_read_byte(mtd));
			return -EFAULT;
	}
	return 0;
}

/**
 * nand_device_ready - Check the NAND device is ready for next command.
 * @mtd:	MTD device structure
 */
static int nand_device_ready(struct mtd_info *mtdinfo)
{
	struct nand_chip *this = mtdinfo->priv;

	if (in_be32(this->IO_ADDR_W) & FPGA_NAND_BUSY)
		return 0; /* busy */
	return 1;
}

/**
 * nand_hwcontrol - NAND control functions wrapper.
 * @mtd:	MTD device structure
 * @cmd:	Command
 */
static void nand_hwcontrol(struct mtd_info *mtdinfo, int cmd, unsigned int ctrl)
{
	if (ctrl & NAND_CTRL_CHANGE) {
		state &= ~(FPGA_NAND_CMD_MASK | FPGA_NAND_ENABLE);

		switch (ctrl & (NAND_ALE | NAND_CLE)) {
		case 0:
			state |= FPGA_NAND_CMD_WRITE;
			break;

		case NAND_ALE:
			state |= FPGA_NAND_CMD_ADDR;
			break;

		case NAND_CLE:
			state |= FPGA_NAND_CMD_COMMAND;
			break;

		default:
			printf("%s: unknown ctrl %#x\n", __FUNCTION__, ctrl);
		}

		if (ctrl & NAND_NCE)
			state |= FPGA_NAND_ENABLE;
	}

	if (cmd != NAND_CMD_NONE)
		nand_write_byte(mtdinfo, cmd);
}

int board_nand_init(struct nand_chip *nand)
{
	nand->cmd_ctrl = nand_hwcontrol;
	nand->ecc.mode = NAND_ECC_SOFT;
	nand->dev_ready = nand_device_ready;
	nand->read_byte = nand_read_byte;
	nand->read_word = nand_read_word;
	nand->write_buf = nand_write_buf;
	nand->read_buf = nand_read_buf;
	nand->verify_buf = nand_verify_buf;

	return 0;
}

#endif
