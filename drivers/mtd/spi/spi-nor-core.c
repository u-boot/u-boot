// SPDX-License-Identifier: GPL-2.0
/*
 * Based on m25p80.c, by Mike Lavender (mike@steroidmicros.com), with
 * influence from lart.c (Abraham Van Der Merwe) and mtd_dataflash.c
 *
 * Copyright (C) 2005, Intec Automation Inc.
 * Copyright (C) 2014, Freescale Semiconductor, Inc.
 *
 * Synced from Linux v4.19
 */

#include <display_options.h>
#include <log.h>
#include <watchdog.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <linux/bitops.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/log2.h>
#include <linux/math64.h>
#include <linux/printk.h>
#include <linux/sizes.h>
#include <linux/bitfield.h>
#include <linux/delay.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/spi-nor.h>
#include <mtd/cfi_flash.h>
#include <spi-mem.h>
#include <spi.h>

#include "sf_internal.h"

/* Define max times to check status register before we give up. */

/*
 * For everything but full-chip erase; probably could be much smaller, but kept
 * around for safety for now
 */

#define HZ					CONFIG_SYS_HZ

#define DEFAULT_READY_WAIT_JIFFIES		(40UL * HZ)

/*
 * For full-chip erase, calibrated to a 2MB flash (M25P16); should be scaled up
 * for larger flash
 */
#define CHIP_ERASE_2MB_READY_WAIT_JIFFIES      (40UL * HZ)

#define ROUND_UP_TO(x, y)	(((x) + (y) - 1) / (y) * (y))

struct sfdp_parameter_header {
	u8		id_lsb;
	u8		minor;
	u8		major;
	u8		length; /* in double words */
	u8		parameter_table_pointer[3]; /* byte address */
	u8		id_msb;
};

#define SFDP_PARAM_HEADER_ID(p)	(((p)->id_msb << 8) | (p)->id_lsb)
#define SFDP_PARAM_HEADER_PTP(p) \
	(((p)->parameter_table_pointer[2] << 16) | \
	 ((p)->parameter_table_pointer[1] <<  8) | \
	 ((p)->parameter_table_pointer[0] <<  0))

#define SFDP_BFPT_ID		0xff00	/* Basic Flash Parameter Table */
#define SFDP_SECTOR_MAP_ID	0xff81	/* Sector Map Table */
#define SFDP_SST_ID		0x01bf	/* Manufacturer specific Table */
#define SFDP_PROFILE1_ID	0xff05	/* xSPI Profile 1.0 Table */
#define SFDP_SCCR_MAP_ID	0xff87	/*
					 * Status, Control and Configuration
					 * Register Map.
					 */

#define SFDP_SIGNATURE		0x50444653U
#define SFDP_JESD216_MAJOR	1
#define SFDP_JESD216_MINOR	0
#define SFDP_JESD216A_MINOR	5
#define SFDP_JESD216B_MINOR	6

struct sfdp_header {
	u32		signature; /* Ox50444653U <=> "SFDP" */
	u8		minor;
	u8		major;
	u8		nph; /* 0-base number of parameter headers */
	u8		unused;

	/* Basic Flash Parameter Table. */
	struct sfdp_parameter_header	bfpt_header;
};

/* Basic Flash Parameter Table */

/*
 * JESD216 rev D defines a Basic Flash Parameter Table of 20 DWORDs.
 * They are indexed from 1 but C arrays are indexed from 0.
 */
#define BFPT_DWORD(i)		((i) - 1)
#define BFPT_DWORD_MAX		20

/* The first version of JESB216 defined only 9 DWORDs. */
#define BFPT_DWORD_MAX_JESD216			9
#define BFPT_DWORD_MAX_JESD216B			16

/* 1st DWORD. */
#define BFPT_DWORD1_FAST_READ_1_1_2		BIT(16)
#define BFPT_DWORD1_ADDRESS_BYTES_MASK		GENMASK(18, 17)
#define BFPT_DWORD1_ADDRESS_BYTES_3_ONLY	(0x0UL << 17)
#define BFPT_DWORD1_ADDRESS_BYTES_3_OR_4	(0x1UL << 17)
#define BFPT_DWORD1_ADDRESS_BYTES_4_ONLY	(0x2UL << 17)
#define BFPT_DWORD1_DTR				BIT(19)
#define BFPT_DWORD1_FAST_READ_1_2_2		BIT(20)
#define BFPT_DWORD1_FAST_READ_1_4_4		BIT(21)
#define BFPT_DWORD1_FAST_READ_1_1_4		BIT(22)

/* 5th DWORD. */
#define BFPT_DWORD5_FAST_READ_2_2_2		BIT(0)
#define BFPT_DWORD5_FAST_READ_4_4_4		BIT(4)

/* 11th DWORD. */
#define BFPT_DWORD11_PAGE_SIZE_SHIFT		4
#define BFPT_DWORD11_PAGE_SIZE_MASK		GENMASK(7, 4)

/* 15th DWORD. */

/*
 * (from JESD216 rev B)
 * Quad Enable Requirements (QER):
 * - 000b: Device does not have a QE bit. Device detects 1-1-4 and 1-4-4
 *         reads based on instruction. DQ3/HOLD# functions are hold during
 *         instruction phase.
 * - 001b: QE is bit 1 of status register 2. It is set via Write Status with
 *         two data bytes where bit 1 of the second byte is one.
 *         [...]
 *         Writing only one byte to the status register has the side-effect of
 *         clearing status register 2, including the QE bit. The 100b code is
 *         used if writing one byte to the status register does not modify
 *         status register 2.
 * - 010b: QE is bit 6 of status register 1. It is set via Write Status with
 *         one data byte where bit 6 is one.
 *         [...]
 * - 011b: QE is bit 7 of status register 2. It is set via Write status
 *         register 2 instruction 3Eh with one data byte where bit 7 is one.
 *         [...]
 *         The status register 2 is read using instruction 3Fh.
 * - 100b: QE is bit 1 of status register 2. It is set via Write Status with
 *         two data bytes where bit 1 of the second byte is one.
 *         [...]
 *         In contrast to the 001b code, writing one byte to the status
 *         register does not modify status register 2.
 * - 101b: QE is bit 1 of status register 2. Status register 1 is read using
 *         Read Status instruction 05h. Status register2 is read using
 *         instruction 35h. QE is set via Writ Status instruction 01h with
 *         two data bytes where bit 1 of the second byte is one.
 *         [...]
 */
#define BFPT_DWORD15_QER_MASK			GENMASK(22, 20)
#define BFPT_DWORD15_QER_NONE			(0x0UL << 20) /* Micron */
#define BFPT_DWORD15_QER_SR2_BIT1_BUGGY		(0x1UL << 20)
#define BFPT_DWORD15_QER_SR1_BIT6		(0x2UL << 20) /* Macronix */
#define BFPT_DWORD15_QER_SR2_BIT7		(0x3UL << 20)
#define BFPT_DWORD15_QER_SR2_BIT1_NO_RD		(0x4UL << 20)
#define BFPT_DWORD15_QER_SR2_BIT1		(0x5UL << 20) /* Spansion */

#define BFPT_DWORD16_SOFT_RST			BIT(12)
#define BFPT_DWORD16_EX4B_PWRCYC		BIT(21)

#define BFPT_DWORD18_CMD_EXT_MASK		GENMASK(30, 29)
#define BFPT_DWORD18_CMD_EXT_REP		(0x0UL << 29) /* Repeat */
#define BFPT_DWORD18_CMD_EXT_INV		(0x1UL << 29) /* Invert */
#define BFPT_DWORD18_CMD_EXT_RES		(0x2UL << 29) /* Reserved */
#define BFPT_DWORD18_CMD_EXT_16B		(0x3UL << 29) /* 16-bit opcode */

/* xSPI Profile 1.0 table (from JESD216D.01). */
#define PROFILE1_DWORD1_RD_FAST_CMD		GENMASK(15, 8)
#define PROFILE1_DWORD1_RDSR_DUMMY		BIT(28)
#define PROFILE1_DWORD1_RDSR_ADDR_BYTES		BIT(29)
#define PROFILE1_DWORD4_DUMMY_200MHZ		GENMASK(11, 7)
#define PROFILE1_DWORD5_DUMMY_166MHZ		GENMASK(31, 27)
#define PROFILE1_DWORD5_DUMMY_133MHZ		GENMASK(21, 17)
#define PROFILE1_DWORD5_DUMMY_100MHZ		GENMASK(11, 7)
#define PROFILE1_DUMMY_DEFAULT			20

/* Status, Control and Configuration Register Map(SCCR) */
#define SCCR_DWORD22_OCTAL_DTR_EN_VOLATILE      BIT(31)

struct sfdp_bfpt {
	u32	dwords[BFPT_DWORD_MAX];
};

/**
 * struct spi_nor_fixups - SPI NOR fixup hooks
 * @post_bfpt: called after the BFPT table has been parsed
 * @post_sfdp: called after SFDP has been parsed. Typically used to tweak
 *             various parameters that could not be extracted by other means
 *             (i.e. when information provided by the SFDP tables are incomplete
 *             or wrong).
 * @late_init: used to initialize flash parameters that are not declared in the
 *             JESD216 SFDP standard, or where SFDP tables not defined at all.
 *
 * Those hooks can be used to tweak the SPI NOR configuration when the SFDP
 * table is broken or not available.
 */
struct spi_nor_fixups {
	int (*post_bfpt)(struct spi_nor *nor,
			 const struct sfdp_parameter_header *bfpt_header,
			 const struct sfdp_bfpt *bfpt,
			 struct spi_nor_flash_parameter *params);
	void (*post_sfdp)(struct spi_nor *nor,
			  struct spi_nor_flash_parameter *params);
	void (*late_init)(struct spi_nor *nor,
			  struct spi_nor_flash_parameter *params);
};

#define SPI_NOR_SRST_SLEEP_LEN			200

/**
 * spi_nor_get_cmd_ext() - Get the command opcode extension based on the
 *			   extension type.
 * @nor:		pointer to a 'struct spi_nor'
 * @op:			pointer to the 'struct spi_mem_op' whose properties
 *			need to be initialized.
 *
 * Right now, only "repeat" and "invert" are supported.
 *
 * Return: The opcode extension.
 */
static u8 spi_nor_get_cmd_ext(const struct spi_nor *nor,
			      const struct spi_mem_op *op)
{
	switch (nor->cmd_ext_type) {
	case SPI_NOR_EXT_INVERT:
		return ~op->cmd.opcode;

	case SPI_NOR_EXT_REPEAT:
		return op->cmd.opcode;

	default:
		dev_dbg(nor->dev, "Unknown command extension type\n");
		return 0;
	}
}

/**
 * spi_nor_setup_op() - Set up common properties of a spi-mem op.
 * @nor:		pointer to a 'struct spi_nor'
 * @op:			pointer to the 'struct spi_mem_op' whose properties
 *			need to be initialized.
 * @proto:		the protocol from which the properties need to be set.
 */
void spi_nor_setup_op(const struct spi_nor *nor,
		      struct spi_mem_op *op,
		      const enum spi_nor_protocol proto)
{
	u8 ext;

	op->cmd.buswidth = spi_nor_get_protocol_inst_nbits(proto);

	if (op->addr.nbytes)
		op->addr.buswidth = spi_nor_get_protocol_addr_nbits(proto);

	if (op->dummy.nbytes)
		op->dummy.buswidth = spi_nor_get_protocol_addr_nbits(proto);

	if (op->data.nbytes)
		op->data.buswidth = spi_nor_get_protocol_data_nbits(proto);

	if (spi_nor_protocol_is_dtr(proto)) {
		/*
		 * spi-mem supports mixed DTR modes, but right now we can only
		 * have all phases either DTR or STR. IOW, spi-mem can have
		 * something like 4S-4D-4D, but spi-nor can't. So, set all 4
		 * phases to either DTR or STR.
		 */
		op->cmd.dtr = op->addr.dtr = op->dummy.dtr =
			op->data.dtr = true;

		/* 2 bytes per clock cycle in DTR mode. */
		op->dummy.nbytes *= 2;

		ext = spi_nor_get_cmd_ext(nor, op);
		op->cmd.opcode = (op->cmd.opcode << 8) | ext;
		op->cmd.nbytes = 2;
	}
}

static int spi_nor_read_write_reg(struct spi_nor *nor, struct spi_mem_op
		*op, void *buf)
{
	if (op->data.dir == SPI_MEM_DATA_IN)
		op->data.buf.in = buf;
	else
		op->data.buf.out = buf;
	return spi_mem_exec_op(nor->spi, op);
}

static int spi_nor_read_reg(struct spi_nor *nor, u8 code, u8 *val, int len)
{
	struct spi_mem_op op = SPI_MEM_OP(SPI_MEM_OP_CMD(code, 0),
					  SPI_MEM_OP_NO_ADDR,
					  SPI_MEM_OP_NO_DUMMY,
					  SPI_MEM_OP_DATA_IN(len, NULL, 0));
	int ret;

	spi_nor_setup_op(nor, &op, nor->reg_proto);

	ret = spi_nor_read_write_reg(nor, &op, val);
	if (ret < 0)
		dev_dbg(nor->dev, "error %d reading %x\n", ret, code);

	return ret;
}

static int spi_nor_write_reg(struct spi_nor *nor, u8 opcode, u8 *buf, int len)
{
	struct spi_mem_op op = SPI_MEM_OP(SPI_MEM_OP_CMD(opcode, 0),
					  SPI_MEM_OP_NO_ADDR,
					  SPI_MEM_OP_NO_DUMMY,
					  SPI_MEM_OP_DATA_OUT(len, NULL, 0));

	spi_nor_setup_op(nor, &op, nor->reg_proto);

	if (len == 0)
		op.data.dir = SPI_MEM_NO_DATA;

	return spi_nor_read_write_reg(nor, &op, buf);
}

#ifdef CONFIG_SPI_FLASH_SPANSION
static int spansion_read_any_reg(struct spi_nor *nor, u32 addr, u8 dummy,
				 u8 *val)
{
	struct spi_mem_op op =
		SPI_MEM_OP(SPI_MEM_OP_CMD(SPINOR_OP_RD_ANY_REG, 0),
			   SPI_MEM_OP_ADDR(nor->addr_mode_nbytes, addr, 0),
			   SPI_MEM_OP_DUMMY(dummy, 0),
			   SPI_MEM_OP_DATA_IN(1, NULL, 0));
	u8 buf[2];
	int ret;

	spi_nor_setup_op(nor, &op, nor->reg_proto);

	/*
	 * In Octal DTR mode, the number of address bytes is always 4 regardless
	 * of addressing mode setting.
	 */
	if (nor->reg_proto == SNOR_PROTO_8_8_8_DTR)
		op.addr.nbytes = 4;

	/*
	 * We don't want to read only one byte in DTR mode. So, read 2 and then
	 * discard the second byte.
	 */
	if (spi_nor_protocol_is_dtr(nor->reg_proto))
		op.data.nbytes = 2;

	ret = spi_nor_read_write_reg(nor, &op, buf);
	if (ret)
		return ret;

	*val = buf[0];

	return 0;
}

static int spansion_write_any_reg(struct spi_nor *nor, u32 addr, u8 val)
{
	struct spi_mem_op op =
		SPI_MEM_OP(SPI_MEM_OP_CMD(SPINOR_OP_WR_ANY_REG, 1),
			   SPI_MEM_OP_ADDR(nor->addr_mode_nbytes, addr, 1),
			   SPI_MEM_OP_NO_DUMMY,
			   SPI_MEM_OP_DATA_OUT(1, NULL, 1));

	return spi_nor_read_write_reg(nor, &op, &val);
}
#endif

static ssize_t spi_nor_read_data(struct spi_nor *nor, loff_t from, size_t len,
				 u_char *buf)
{
	struct spi_mem_op op =
			SPI_MEM_OP(SPI_MEM_OP_CMD(nor->read_opcode, 0),
				   SPI_MEM_OP_ADDR(nor->addr_width, from, 0),
				   SPI_MEM_OP_DUMMY(nor->read_dummy, 0),
				   SPI_MEM_OP_DATA_IN(len, buf, 0));
	size_t remaining = len;
	int ret;

	spi_nor_setup_op(nor, &op, nor->read_proto);

	/* convert the dummy cycles to the number of bytes */
	op.dummy.nbytes = (nor->read_dummy * op.dummy.buswidth) / 8;
	if (spi_nor_protocol_is_dtr(nor->read_proto))
		op.dummy.nbytes *= 2;

	while (remaining) {
		op.data.nbytes = remaining < UINT_MAX ? remaining : UINT_MAX;

		if (CONFIG_IS_ENABLED(SPI_DIRMAP) && nor->dirmap.rdesc) {
			/*
			 * Record current operation information which may be used
			 * when the address or data length exceeds address mapping.
			 */
			memcpy(&nor->dirmap.rdesc->info.op_tmpl, &op,
			       sizeof(struct spi_mem_op));
			ret = spi_mem_dirmap_read(nor->dirmap.rdesc,
						  op.addr.val, op.data.nbytes,
						  op.data.buf.in);
			if (ret < 0)
				return ret;
			op.data.nbytes = ret;
		} else {
			ret = spi_mem_adjust_op_size(nor->spi, &op);
			if (ret)
				return ret;

			ret = spi_mem_exec_op(nor->spi, &op);
			if (ret)
				return ret;
		}

		op.addr.val += op.data.nbytes;
		remaining -= op.data.nbytes;
		op.data.buf.in += op.data.nbytes;
	}

	return len;
}

static ssize_t spi_nor_write_data(struct spi_nor *nor, loff_t to, size_t len,
				  const u_char *buf)
{
	struct spi_mem_op op =
			SPI_MEM_OP(SPI_MEM_OP_CMD(nor->program_opcode, 0),
				   SPI_MEM_OP_ADDR(nor->addr_width, to, 0),
				   SPI_MEM_OP_NO_DUMMY,
				   SPI_MEM_OP_DATA_OUT(len, buf, 0));
	int ret;

	if (nor->program_opcode == SPINOR_OP_AAI_WP && nor->sst_write_second)
		op.addr.nbytes = 0;

	spi_nor_setup_op(nor, &op, nor->write_proto);

	if (CONFIG_IS_ENABLED(SPI_DIRMAP) && nor->dirmap.wdesc) {
		memcpy(&nor->dirmap.wdesc->info.op_tmpl, &op,
		       sizeof(struct spi_mem_op));
		op.data.nbytes = spi_mem_dirmap_write(nor->dirmap.wdesc, op.addr.val,
						      op.data.nbytes, op.data.buf.out);
	} else {
		ret = spi_mem_adjust_op_size(nor->spi, &op);
		if (ret)
			return ret;
		op.data.nbytes = len < op.data.nbytes ? len : op.data.nbytes;

		ret = spi_mem_exec_op(nor->spi, &op);
		if (ret)
			return ret;
	}

	return op.data.nbytes;
}

/*
 * Return the status register value. If the chip is parallel, then the
 * read will be striped, so we should read 2 bytes to get the sr
 * register value from both of the parallel chips.
 * Returns negative if error occurred.
 */
static int read_sr(struct spi_nor *nor)
{
	struct spi_mem_op op;
	int ret;
	u8 val[2];
	u8 addr_nbytes, dummy;

	if (nor->reg_proto == SNOR_PROTO_8_8_8_DTR) {
		addr_nbytes = nor->rdsr_addr_nbytes;
		dummy = nor->rdsr_dummy;
	} else {
		addr_nbytes = 0;
		dummy = 0;
	}

	op = (struct spi_mem_op)SPI_MEM_OP(SPI_MEM_OP_CMD(SPINOR_OP_RDSR, 0),
					   SPI_MEM_OP_ADDR(addr_nbytes, 0, 0),
					   SPI_MEM_OP_DUMMY(dummy, 0),
					   SPI_MEM_OP_DATA_IN(1, NULL, 0));

	spi_nor_setup_op(nor, &op, nor->reg_proto);

	/*
	 * We don't want to read only one byte in DTR mode. So, read 2 and then
	 * discard the second byte.
	 */
	if (spi_nor_protocol_is_dtr(nor->reg_proto))
		op.data.nbytes = 2;

	if (nor->flags & SNOR_F_HAS_PARALLEL) {
		op.data.nbytes = 2;
		ret = spi_nor_read_write_reg(nor, &op, &val[0]);
		if (ret < 0) {
			pr_debug("error %d reading SR\n", (int)ret);
			return ret;
		}
		val[0] |= val[1];
	} else {
		ret = spi_nor_read_write_reg(nor, &op, &val[0]);
		if (ret < 0) {
			pr_debug("error %d reading SR\n", (int)ret);
			return ret;
		}
	}

	return val[0];
}

/*
 * Return the flag status register value. If the chip is parallel, then
 * the read will be striped, so we should read 2 bytes to get the fsr
 * register value from both of the parallel chips.
 * Returns negative if error occurred.
 */
static int read_fsr(struct spi_nor *nor)
{
	struct spi_mem_op op;
	int ret;
	u8 val[2];
	u8 addr_nbytes, dummy;

	if (nor->reg_proto == SNOR_PROTO_8_8_8_DTR) {
		addr_nbytes = nor->rdsr_addr_nbytes;
		dummy = nor->rdsr_dummy;
	} else {
		addr_nbytes = 0;
		dummy = 0;
	}

	op = (struct spi_mem_op)SPI_MEM_OP(SPI_MEM_OP_CMD(SPINOR_OP_RDFSR, 0),
					   SPI_MEM_OP_ADDR(addr_nbytes, 0, 0),
					   SPI_MEM_OP_DUMMY(dummy, 0),
					   SPI_MEM_OP_DATA_IN(1, NULL, 0));

	spi_nor_setup_op(nor, &op, nor->reg_proto);

	/*
	 * We don't want to read only one byte in DTR mode. So, read 2 and then
	 * discard the second byte.
	 */
	if (spi_nor_protocol_is_dtr(nor->reg_proto))
		op.data.nbytes = 2;

	if (nor->flags & SNOR_F_HAS_PARALLEL) {
		op.data.nbytes = 2;
		ret = spi_nor_read_write_reg(nor, &op, &val[0]);
		if (ret < 0) {
			pr_debug("error %d reading SR\n", (int)ret);
			return ret;
		}
		val[0] &= val[1];
	} else {
		ret = spi_nor_read_write_reg(nor, &op, &val[0]);
		if (ret < 0) {
			pr_debug("error %d reading FSR\n", ret);
			return ret;
		}
	}

	return val[0];
}

/*
 * Read configuration register, returning its value in the
 * location. Return the configuration register value.
 * Returns negative if error occurred.
 */
#if defined(CONFIG_SPI_FLASH_SPANSION) || defined(CONFIG_SPI_FLASH_WINBOND)
static int read_cr(struct spi_nor *nor)
{
	int ret;
	u8 val;

	ret = nor->read_reg(nor, SPINOR_OP_RDCR, &val, 1);
	if (ret < 0) {
		dev_dbg(nor->dev, "error %d reading CR\n", ret);
		return ret;
	}

	return val;
}
#endif

/**
 * read_sr3() - Read status register 3 unique to newer Winbond flashes
 * @nor:	pointer to a 'struct spi_nor'
 */
static int read_sr3(struct spi_nor *nor)
{
	int ret;
	u8 val;

	ret = nor->read_reg(nor, SPINOR_OP_RDSR3, &val, 1);
	if (ret < 0) {
		dev_dbg(nor->dev, "error %d reading SR3\n", ret);
		return ret;
	}

	return val;
}

/*
 * Write status register 1 byte
 * Returns negative if error occurred.
 */
static int write_sr(struct spi_nor *nor, u8 val)
{
	nor->cmd_buf[0] = val;
	return nor->write_reg(nor, SPINOR_OP_WRSR, nor->cmd_buf, 1);
}

/**
 * write_sr3() - Write status register 3 unique to newer Winbond flashes
 * @nor:	pointer to a 'struct spi_nor'
 * @val:	value to be written into SR3
 */
static int write_sr3(struct spi_nor *nor, u8 val)
{
	nor->cmd_buf[0] = val;
	return nor->write_reg(nor, SPINOR_OP_WRSR3, nor->cmd_buf, 1);
}

/*
 * Set write enable latch with Write Enable command.
 * Returns negative if error occurred.
 */
static int write_enable(struct spi_nor *nor)
{
	return nor->write_reg(nor, SPINOR_OP_WREN, NULL, 0);
}

/*
 * Send write disable instruction to the chip.
 */
static int write_disable(struct spi_nor *nor)
{
	return nor->write_reg(nor, SPINOR_OP_WRDI, NULL, 0);
}

static struct spi_nor *mtd_to_spi_nor(struct mtd_info *mtd)
{
	return mtd->priv;
}

#ifndef CONFIG_SPI_FLASH_BAR
static u8 spi_nor_convert_opcode(u8 opcode, const u8 table[][2], size_t size)
{
	size_t i;

	for (i = 0; i < size; i++)
		if (table[i][0] == opcode)
			return table[i][1];

	/* No conversion found, keep input op code. */
	return opcode;
}

static u8 spi_nor_convert_3to4_read(u8 opcode)
{
	static const u8 spi_nor_3to4_read[][2] = {
		{ SPINOR_OP_READ,	SPINOR_OP_READ_4B },
		{ SPINOR_OP_READ_FAST,	SPINOR_OP_READ_FAST_4B },
		{ SPINOR_OP_READ_1_1_2,	SPINOR_OP_READ_1_1_2_4B },
		{ SPINOR_OP_READ_1_2_2,	SPINOR_OP_READ_1_2_2_4B },
		{ SPINOR_OP_READ_1_1_4,	SPINOR_OP_READ_1_1_4_4B },
		{ SPINOR_OP_READ_1_4_4,	SPINOR_OP_READ_1_4_4_4B },
		{ SPINOR_OP_READ_1_1_8,	SPINOR_OP_READ_1_1_8_4B },
		{ SPINOR_OP_READ_1_8_8,	SPINOR_OP_READ_1_8_8_4B },

		{ SPINOR_OP_READ_1_1_1_DTR,	SPINOR_OP_READ_1_1_1_DTR_4B },
		{ SPINOR_OP_READ_1_2_2_DTR,	SPINOR_OP_READ_1_2_2_DTR_4B },
		{ SPINOR_OP_READ_1_4_4_DTR,	SPINOR_OP_READ_1_4_4_DTR_4B },
	};

	return spi_nor_convert_opcode(opcode, spi_nor_3to4_read,
				      ARRAY_SIZE(spi_nor_3to4_read));
}

static u8 spi_nor_convert_3to4_program(u8 opcode)
{
	static const u8 spi_nor_3to4_program[][2] = {
		{ SPINOR_OP_PP,		SPINOR_OP_PP_4B },
		{ SPINOR_OP_PP_1_1_4,	SPINOR_OP_PP_1_1_4_4B },
		{ SPINOR_OP_PP_1_4_4,	SPINOR_OP_PP_1_4_4_4B },
		{ SPINOR_OP_PP_1_1_8,	SPINOR_OP_PP_1_1_8_4B },
		{ SPINOR_OP_PP_1_8_8,	SPINOR_OP_PP_1_8_8_4B },
	};

	return spi_nor_convert_opcode(opcode, spi_nor_3to4_program,
				      ARRAY_SIZE(spi_nor_3to4_program));
}

static u8 spi_nor_convert_3to4_erase(u8 opcode)
{
	static const u8 spi_nor_3to4_erase[][2] = {
		{ SPINOR_OP_BE_4K,	SPINOR_OP_BE_4K_4B },
		{ SPINOR_OP_BE_32K,	SPINOR_OP_BE_32K_4B },
		{ SPINOR_OP_SE,		SPINOR_OP_SE_4B },
	};

	return spi_nor_convert_opcode(opcode, spi_nor_3to4_erase,
				      ARRAY_SIZE(spi_nor_3to4_erase));
}

static void spi_nor_set_4byte_opcodes(struct spi_nor *nor,
				      const struct flash_info *info)
{
	bool shift = 0;

	if (nor->flags & SNOR_F_HAS_PARALLEL)
		shift = 1;

	/* Do some manufacturer fixups first */
	switch (JEDEC_MFR(info)) {
	case SNOR_MFR_SPANSION:
		/* No small sector erase for 4-byte command set */
		nor->erase_opcode = SPINOR_OP_SE;
		nor->mtd.erasesize = info->sector_size << shift;
		break;

	default:
		break;
	}

	nor->read_opcode = spi_nor_convert_3to4_read(nor->read_opcode);
	nor->program_opcode = spi_nor_convert_3to4_program(nor->program_opcode);
	nor->erase_opcode = spi_nor_convert_3to4_erase(nor->erase_opcode);
}
#endif /* !CONFIG_SPI_FLASH_BAR */

/* Enable/disable 4-byte addressing mode. */
static int set_4byte(struct spi_nor *nor, const struct flash_info *info,
		     int enable)
{
	int status;
	bool need_wren = false;
	u8 cmd;

	switch (JEDEC_MFR(info)) {
	case SNOR_MFR_ST:
	case SNOR_MFR_MICRON:
		/* Some Micron need WREN command; all will accept it */
		need_wren = true;
		fallthrough;
	case SNOR_MFR_ISSI:
	case SNOR_MFR_MACRONIX:
	case SNOR_MFR_WINBOND:
		if (need_wren)
			write_enable(nor);

		cmd = enable ? SPINOR_OP_EN4B : SPINOR_OP_EX4B;
		status = nor->write_reg(nor, cmd, NULL, 0);
		if (need_wren)
			write_disable(nor);

		if (!status && !enable &&
		    JEDEC_MFR(info) == SNOR_MFR_WINBOND) {
			/*
			 * On Winbond W25Q256FV, leaving 4byte mode causes
			 * the Extended Address Register to be set to 1, so all
			 * 3-byte-address reads come from the second 16M.
			 * We must clear the register to enable normal behavior.
			 */
			write_enable(nor);
			nor->cmd_buf[0] = 0;
			nor->write_reg(nor, SPINOR_OP_WREAR, nor->cmd_buf, 1);
			write_disable(nor);
		}

		return status;
	case SNOR_MFR_CYPRESS:
		cmd = enable ? SPINOR_OP_EN4B : SPINOR_OP_EX4B_CYPRESS;
		return nor->write_reg(nor, cmd, NULL, 0);
	default:
		/* Spansion style */
		nor->cmd_buf[0] = enable << 7;
		return nor->write_reg(nor, SPINOR_OP_BRWR, nor->cmd_buf, 1);
	}
}

#ifdef CONFIG_SPI_FLASH_SPANSION
/*
 * Read status register 1 by using Read Any Register command to support multi
 * die package parts.
 */
static int spansion_sr_ready(struct spi_nor *nor, u32 addr_base, u8 dummy)
{
	u32 reg_addr = addr_base + SPINOR_REG_CYPRESS_STR1V;
	u8 sr;
	int ret;

	ret = spansion_read_any_reg(nor, reg_addr, dummy, &sr);
	if (ret < 0)
		return ret;

	if (sr & (SR_E_ERR | SR_P_ERR)) {
		if (sr & SR_E_ERR)
			dev_dbg(nor->dev, "Erase Error occurred\n");
		else
			dev_dbg(nor->dev, "Programming Error occurred\n");

		nor->write_reg(nor, SPINOR_OP_CYPRESS_CLPEF, NULL, 0);
		return -EIO;
	}

	return !(sr & SR_WIP);
}
#endif

static int spi_nor_sr_ready(struct spi_nor *nor)
{
	int sr = read_sr(nor);

	if (sr < 0)
		return sr;

	if (nor->flags & SNOR_F_USE_CLSR && sr & (SR_E_ERR | SR_P_ERR)) {
		if (sr & SR_E_ERR)
			dev_dbg(nor->dev, "Erase Error occurred\n");
		else
			dev_dbg(nor->dev, "Programming Error occurred\n");

		nor->write_reg(nor, SPINOR_OP_CLSR, NULL, 0);
		return -EIO;
	}

	return !(sr & SR_WIP);
}

static int spi_nor_fsr_ready(struct spi_nor *nor)
{
	int fsr = read_fsr(nor);

	if (fsr < 0)
		return fsr;

	if (fsr & (FSR_E_ERR | FSR_P_ERR)) {
		if (fsr & FSR_E_ERR)
			dev_err(nor->dev, "Erase operation failed.\n");
		else
			dev_err(nor->dev, "Program operation failed.\n");

		if (fsr & FSR_PT_ERR)
			dev_err(nor->dev,
				"Attempted to modify a protected sector.\n");

		nor->write_reg(nor, SPINOR_OP_CLFSR, NULL, 0);
		return -EIO;
	}

	return fsr & FSR_READY;
}

static int spi_nor_default_ready(struct spi_nor *nor)
{
	int sr, fsr;

	sr = spi_nor_sr_ready(nor);
	if (sr < 0)
		return sr;
	fsr = nor->flags & SNOR_F_USE_FSR ? spi_nor_fsr_ready(nor) : 1;
	if (fsr < 0)
		return fsr;
	return sr && fsr;
}

static int spi_nor_ready(struct spi_nor *nor)
{
	if (nor->ready)
		return nor->ready(nor);

	return spi_nor_default_ready(nor);
}

/*
 * Service routine to read status register until ready, or timeout occurs.
 * Returns non-zero if error.
 */
static int spi_nor_wait_till_ready_with_timeout(struct spi_nor *nor,
						unsigned long timeout)
{
	unsigned long timebase;
	int ret;

	timebase = get_timer(0);

	while (get_timer(timebase) < timeout) {
		ret = spi_nor_ready(nor);
		if (ret < 0)
			return ret;
		if (ret)
			return 0;
	}

	dev_err(nor->dev, "flash operation timed out\n");

	return -ETIMEDOUT;
}

static int spi_nor_wait_till_ready(struct spi_nor *nor)
{
	return spi_nor_wait_till_ready_with_timeout(nor,
						    DEFAULT_READY_WAIT_JIFFIES);
}

static int spi_nor_erase_chip_wait_till_ready(struct spi_nor *nor, unsigned long size)
{
	/*
	 * Scale the timeout linearly with the size of the flash, with
	 * a minimum calibrated to an old 2MB flash. We could try to
	 * pull these from CFI/SFDP, but these values should be good
	 * enough for now.
	 */
	unsigned long timeout = max(CHIP_ERASE_2MB_READY_WAIT_JIFFIES,
				    CHIP_ERASE_2MB_READY_WAIT_JIFFIES *
				    (unsigned long)(size / SZ_2M));
	return spi_nor_wait_till_ready_with_timeout(nor, timeout);
}

#ifdef CONFIG_SPI_FLASH_BAR
/*
 * This "clean_bar" is necessary in a situation when one was accessing
 * spi flash memory > 16 MiB by using Bank Address Register's BA24 bit.
 *
 * After it the BA24 bit shall be cleared to allow access to correct
 * memory region after SW reset (by calling "reset" command).
 *
 * Otherwise, the BA24 bit may be left set and then after reset, the
 * ROM would read/write/erase SPL from 16 MiB * bank_sel address.
 */
static int clean_bar(struct spi_nor *nor)
{
	u8 cmd, bank_sel = 0;

	if (nor->bank_curr == 0)
		return 0;
	cmd = nor->bank_write_cmd;
	nor->bank_curr = 0;
	write_enable(nor);

	return nor->write_reg(nor, cmd, &bank_sel, 1);
}

static int write_bar(struct spi_nor *nor, u32 offset)
{
	u8 cmd, bank_sel, upage_curr;
	int ret;
	struct mtd_info *mtd = &nor->mtd;

	/* Wait until previous write command is finished */
	if (spi_nor_wait_till_ready(nor))
		return 1;

	if (nor->flags & (SNOR_F_HAS_PARALLEL | SNOR_F_HAS_STACKED) &&
	    mtd->size <= SZ_32M)
		return 0;

	if (mtd->size <= SZ_16M)
		return 0;

	offset = offset % (u32)mtd->size;
	bank_sel = offset >> 24;

	upage_curr = nor->spi->flags & SPI_XFER_U_PAGE;

	if (!(nor->flags & SNOR_F_HAS_STACKED) && bank_sel == nor->bank_curr)
		return 0;
	else if (upage_curr == nor->upage_prev && bank_sel == nor->bank_curr)
		return 0;

	nor->upage_prev = upage_curr;

	cmd = nor->bank_write_cmd;
	write_enable(nor);
	ret = nor->write_reg(nor, cmd, &bank_sel, 1);
	if (ret < 0) {
		debug("SF: fail to write bank register\n");
		return ret;
	}

	nor->bank_curr = bank_sel;

	return write_disable(nor);
}

static int read_bar(struct spi_nor *nor, const struct flash_info *info)
{
	u8 curr_bank = 0;
	int ret;
	struct mtd_info *mtd = &nor->mtd;

	if (mtd->size <= SZ_16M)
		return 0;

	switch (JEDEC_MFR(info)) {
	case SNOR_MFR_SPANSION:
		nor->bank_read_cmd = SPINOR_OP_BRRD;
		nor->bank_write_cmd = SPINOR_OP_BRWR;
		break;
	default:
		nor->bank_read_cmd = SPINOR_OP_RDEAR;
		nor->bank_write_cmd = SPINOR_OP_WREAR;
	}

	if (nor->flags & SNOR_F_HAS_PARALLEL)
		nor->spi->flags |= SPI_XFER_LOWER;

	ret = nor->read_reg(nor, nor->bank_read_cmd,
			    &curr_bank, 1);
	if (ret) {
		debug("SF: fail to read bank addr register\n");
		return ret;
	}
	nor->bank_curr = curr_bank;

	// Make sure both chips use the same BAR
	if (nor->flags & SNOR_F_HAS_PARALLEL) {
		write_enable(nor);
		ret = nor->write_reg(nor, nor->bank_write_cmd, &curr_bank, 1);
		if (ret)
			return ret;

		ret = write_disable(nor);
		if (ret)
			return ret;
	}

	return ret;
}
#endif

/**
 * spi_nor_erase_chip() - Erase the entire flash memory.
 * @nor:	pointer to 'struct spi_nor'.
 *
 * Return: 0 on success, -errno otherwise.
 */
static int spi_nor_erase_chip(struct spi_nor *nor)
{
	struct spi_mem_op op =
		SPI_MEM_OP(SPI_MEM_OP_CMD(SPINOR_OP_CHIP_ERASE, 0),
			   SPI_MEM_OP_NO_ADDR,
			   SPI_MEM_OP_NO_DUMMY,
			   SPI_MEM_OP_NO_DATA);
	int ret;

	spi_nor_setup_op(nor, &op, nor->write_proto);

	ret = spi_mem_exec_op(nor->spi, &op);
	if (ret)
		return ret;

	return nor->mtd.size;
}

/*
 * Initiate the erasure of a single sector. Returns the number of bytes erased
 * on success, a negative error code on error.
 */
static int spi_nor_erase_sector(struct spi_nor *nor, u32 addr)
{
	struct spi_mem_op op =
		SPI_MEM_OP(SPI_MEM_OP_CMD(nor->erase_opcode, 0),
			   SPI_MEM_OP_ADDR(nor->addr_width, addr, 0),
			   SPI_MEM_OP_NO_DUMMY,
			   SPI_MEM_OP_NO_DATA);
	int ret;

	spi_nor_setup_op(nor, &op, nor->write_proto);

	if (nor->erase)
		return nor->erase(nor, addr);

	/*
	 * Default implementation, if driver doesn't have a specialized HW
	 * control
	 */
	ret = spi_mem_exec_op(nor->spi, &op);
	if (ret)
		return ret;

	return nor->mtd.erasesize;
}

/*
 * Erase an address range on the nor chip.  The address range may extend
 * one or more erase sectors.  Return an error is there is a problem erasing.
 */
static int spi_nor_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	struct spi_nor *nor = mtd_to_spi_nor(mtd);
	u32 addr, len, rem, offset, max_size;
	bool addr_known = false;
	int ret, err;

	dev_dbg(nor->dev, "at 0x%llx, len %lld\n", (long long)instr->addr,
		(long long)instr->len);

	div_u64_rem(instr->len, mtd->erasesize, &rem);
	if (rem) {
		ret = -EINVAL;
		goto err;
	}

	addr = instr->addr;
	len = instr->len;
	max_size = instr->len;

	instr->state = MTD_ERASING;
	addr_known = true;

	while (len) {
		schedule();
		if (!IS_ENABLED(CONFIG_XPL_BUILD) && ctrlc()) {
			addr_known = false;
			ret = -EINTR;
			goto erase_err;
		}
		offset = addr;
		if (CONFIG_IS_ENABLED(SPI_STACKED_PARALLEL)) {
			if (nor->flags & SNOR_F_HAS_PARALLEL)
				offset /= 2;

			if (nor->flags & SNOR_F_HAS_STACKED) {
				if (offset >= (mtd->size / 2))
					nor->spi->flags |= SPI_XFER_U_PAGE;
				else
					nor->spi->flags &= ~SPI_XFER_U_PAGE;
			}
		}
#ifdef CONFIG_SPI_FLASH_BAR
		ret = write_bar(nor, offset);
		if (ret < 0)
			goto erase_err;
#endif
		ret = write_enable(nor);
		if (ret < 0)
			goto erase_err;

		if (len == mtd->size &&
		    !(nor->flags & SNOR_F_NO_OP_CHIP_ERASE)) {
			ret = spi_nor_erase_chip(nor);
		} else {
			ret = spi_nor_erase_sector(nor, offset);
		}
		if (ret < 0)
			goto erase_err;

		addr += ret;
		len -= ret;

		if (max_size == mtd->size &&
		    !(nor->flags & SNOR_F_NO_OP_CHIP_ERASE)) {
			ret = spi_nor_erase_chip_wait_till_ready(nor, mtd->size);
		} else {
			ret = spi_nor_wait_till_ready(nor);
		}

		if (ret)
			goto erase_err;
	}

	addr_known = false;
erase_err:
#ifdef CONFIG_SPI_FLASH_BAR
	err = clean_bar(nor);
	if (!ret)
		ret = err;
#endif
	err = write_disable(nor);
	if (!ret)
		ret = err;

err:
	if (ret) {
		instr->fail_addr = addr_known ? addr : MTD_FAIL_ADDR_UNKNOWN;
		instr->state = MTD_ERASE_FAILED;
	} else {
		instr->state = MTD_ERASE_DONE;
	}

	return ret;
}

#ifdef CONFIG_SPI_FLASH_SPANSION
/**
 * spansion_erase_non_uniform() - erase non-uniform sectors for Spansion/Cypress
 *                                chips
 * @nor:	pointer to a 'struct spi_nor'
 * @addr:	address of the sector to erase
 * @opcode_4k:	opcode for 4K sector erase
 * @ovlsz_top:	size of overlaid portion at the top address
 * @ovlsz_btm:	size of overlaid portion at the bottom address
 *
 * Erase an address range on the nor chip that can contain 4KB sectors overlaid
 * on top and/or bottom. The appropriate erase opcode and size are chosen by
 * address to erase and size of overlaid portion.
 *
 * Return: number of bytes erased on success, -errno otherwise.
 */
static int spansion_erase_non_uniform(struct spi_nor *nor, u32 addr,
				      u8 opcode_4k, u32 ovlsz_top,
				      u32 ovlsz_btm)
{
	struct spi_mem_op op =
		SPI_MEM_OP(SPI_MEM_OP_CMD(nor->erase_opcode, 0),
			   SPI_MEM_OP_ADDR(nor->addr_width, addr, 0),
			   SPI_MEM_OP_NO_DUMMY,
			   SPI_MEM_OP_NO_DATA);
	struct mtd_info *mtd = &nor->mtd;
	u32 erasesize;
	int ret;

	/* 4KB sectors */
	if (op.addr.val < ovlsz_btm ||
	    op.addr.val >= mtd->size - ovlsz_top) {
		op.cmd.opcode = opcode_4k;
		erasesize = SZ_4K;

	/* Non-overlaid portion in the normal sector at the bottom */
	} else if (op.addr.val == ovlsz_btm) {
		op.cmd.opcode = nor->erase_opcode;
		erasesize = mtd->erasesize - ovlsz_btm;

	/* Non-overlaid portion in the normal sector at the top */
	} else if (op.addr.val == mtd->size - mtd->erasesize) {
		op.cmd.opcode = nor->erase_opcode;
		erasesize = mtd->erasesize - ovlsz_top;

	/* Normal sectors */
	} else {
		op.cmd.opcode = nor->erase_opcode;
		erasesize = mtd->erasesize;
	}

	spi_nor_setup_op(nor, &op, nor->write_proto);

	ret = spi_mem_exec_op(nor->spi, &op);
	if (ret)
		return ret;

	return erasesize;
}
#endif

#if defined(CONFIG_SPI_FLASH_LOCK)
#if defined(CONFIG_SPI_FLASH_STMICRO) || defined(CONFIG_SPI_FLASH_SST)
/* Write status register and ensure bits in mask match written values */
static int write_sr_and_check(struct spi_nor *nor, u8 status_new, u8 mask)
{
	int ret;

	write_enable(nor);
	ret = write_sr(nor, status_new);
	if (ret)
		return ret;

	ret = spi_nor_wait_till_ready(nor);
	if (ret)
		return ret;

	ret = read_sr(nor);
	if (ret < 0)
		return ret;

	return ((ret & mask) != (status_new & mask)) ? -EIO : 0;
}

static void stm_get_locked_range(struct spi_nor *nor, u8 sr, loff_t *ofs,
				 uint64_t *len)
{
	struct mtd_info *mtd = &nor->mtd;
	u8 mask = SR_BP2 | SR_BP1 | SR_BP0;
	int shift = ffs(mask) - 1;
	int pow;

	if (!(sr & mask)) {
		/* No protection */
		*ofs = 0;
		*len = 0;
	} else {
		pow = ((sr & mask) ^ mask) >> shift;
		*len = mtd->size >> pow;
		if (nor->flags & SNOR_F_HAS_SR_TB && sr & SR_TB)
			*ofs = 0;
		else
			*ofs = mtd->size - *len;
	}
}

/*
 * Return 1 if the entire region is locked (if @locked is true) or unlocked (if
 * @locked is false); 0 otherwise
 */
static int stm_check_lock_status_sr(struct spi_nor *nor, loff_t ofs, u64 len,
				    u8 sr, bool locked)
{
	loff_t lock_offs;
	uint64_t lock_len;

	if (!len)
		return 1;

	stm_get_locked_range(nor, sr, &lock_offs, &lock_len);

	if (locked)
		/* Requested range is a sub-range of locked range */
		return (ofs + len <= lock_offs + lock_len) && (ofs >= lock_offs);
	else
		/* Requested range does not overlap with locked range */
		return (ofs >= lock_offs + lock_len) || (ofs + len <= lock_offs);
}

static int stm_is_locked_sr(struct spi_nor *nor, loff_t ofs, uint64_t len,
			    u8 sr)
{
	return stm_check_lock_status_sr(nor, ofs, len, sr, true);
}

static int stm_is_unlocked_sr(struct spi_nor *nor, loff_t ofs, uint64_t len,
			      u8 sr)
{
	return stm_check_lock_status_sr(nor, ofs, len, sr, false);
}

/*
 * Lock a region of the flash. Compatible with ST Micro and similar flash.
 * Supports the block protection bits BP{0,1,2} in the status register
 * (SR). Does not support these features found in newer SR bitfields:
 *   - SEC: sector/block protect - only handle SEC=0 (block protect)
 *   - CMP: complement protect - only support CMP=0 (range is not complemented)
 *
 * Support for the following is provided conditionally for some flash:
 *   - TB: top/bottom protect
 *
 * Sample table portion for 8MB flash (Winbond w25q64fw):
 *
 *   SEC  |  TB   |  BP2  |  BP1  |  BP0  |  Prot Length  | Protected Portion
 *  --------------------------------------------------------------------------
 *    X   |   X   |   0   |   0   |   0   |  NONE         | NONE
 *    0   |   0   |   0   |   0   |   1   |  128 KB       | Upper 1/64
 *    0   |   0   |   0   |   1   |   0   |  256 KB       | Upper 1/32
 *    0   |   0   |   0   |   1   |   1   |  512 KB       | Upper 1/16
 *    0   |   0   |   1   |   0   |   0   |  1 MB         | Upper 1/8
 *    0   |   0   |   1   |   0   |   1   |  2 MB         | Upper 1/4
 *    0   |   0   |   1   |   1   |   0   |  4 MB         | Upper 1/2
 *    X   |   X   |   1   |   1   |   1   |  8 MB         | ALL
 *  ------|-------|-------|-------|-------|---------------|-------------------
 *    0   |   1   |   0   |   0   |   1   |  128 KB       | Lower 1/64
 *    0   |   1   |   0   |   1   |   0   |  256 KB       | Lower 1/32
 *    0   |   1   |   0   |   1   |   1   |  512 KB       | Lower 1/16
 *    0   |   1   |   1   |   0   |   0   |  1 MB         | Lower 1/8
 *    0   |   1   |   1   |   0   |   1   |  2 MB         | Lower 1/4
 *    0   |   1   |   1   |   1   |   0   |  4 MB         | Lower 1/2
 *
 * Returns negative on errors, 0 on success.
 */
static int stm_lock(struct spi_nor *nor, loff_t ofs, uint64_t len)
{
	struct mtd_info *mtd = &nor->mtd;
	int status_old, status_new;
	u8 mask = SR_BP2 | SR_BP1 | SR_BP0;
	u8 shift = ffs(mask) - 1, pow, val;
	loff_t lock_len;
	bool can_be_top = true, can_be_bottom = nor->flags & SNOR_F_HAS_SR_TB;
	bool use_top;

	status_old = read_sr(nor);
	if (status_old < 0)
		return status_old;

	/* If nothing in our range is unlocked, we don't need to do anything */
	if (stm_is_locked_sr(nor, ofs, len, status_old))
		return 0;

	/* If anything below us is unlocked, we can't use 'bottom' protection */
	if (!stm_is_locked_sr(nor, 0, ofs, status_old))
		can_be_bottom = false;

	/* If anything above us is unlocked, we can't use 'top' protection */
	if (!stm_is_locked_sr(nor, ofs + len, mtd->size - (ofs + len),
			      status_old))
		can_be_top = false;

	if (!can_be_bottom && !can_be_top)
		return -EINVAL;

	/* Prefer top, if both are valid */
	use_top = can_be_top;

	/* lock_len: length of region that should end up locked */
	if (use_top)
		lock_len = mtd->size - ofs;
	else
		lock_len = ofs + len;

	/*
	 * Need smallest pow such that:
	 *
	 *   1 / (2^pow) <= (len / size)
	 *
	 * so (assuming power-of-2 size) we do:
	 *
	 *   pow = ceil(log2(size / len)) = log2(size) - floor(log2(len))
	 */
	pow = ilog2(mtd->size) - ilog2(lock_len);
	val = mask - (pow << shift);
	if (val & ~mask)
		return -EINVAL;
	/* Don't "lock" with no region! */
	if (!(val & mask))
		return -EINVAL;

	status_new = (status_old & ~mask & ~SR_TB) | val;

	/* Disallow further writes if WP pin is asserted */
	status_new |= SR_SRWD;

	if (!use_top)
		status_new |= SR_TB;

	/* Don't bother if they're the same */
	if (status_new == status_old)
		return 0;

	/* Only modify protection if it will not unlock other areas */
	if ((status_new & mask) < (status_old & mask))
		return -EINVAL;

	return write_sr_and_check(nor, status_new, mask);
}

/*
 * Unlock a region of the flash. See stm_lock() for more info
 *
 * Returns negative on errors, 0 on success.
 */
static int stm_unlock(struct spi_nor *nor, loff_t ofs, uint64_t len)
{
	struct mtd_info *mtd = &nor->mtd;
	int status_old, status_new;
	u8 mask = SR_BP2 | SR_BP1 | SR_BP0;
	u8 shift = ffs(mask) - 1, pow, val;
	loff_t lock_len;
	bool can_be_top = true, can_be_bottom = nor->flags & SNOR_F_HAS_SR_TB;
	bool use_top;

	status_old = read_sr(nor);
	if (status_old < 0)
		return status_old;

	/* If nothing in our range is locked, we don't need to do anything */
	if (stm_is_unlocked_sr(nor, ofs, len, status_old))
		return 0;

	/* If anything below us is locked, we can't use 'top' protection */
	if (!stm_is_unlocked_sr(nor, 0, ofs, status_old))
		can_be_top = false;

	/* If anything above us is locked, we can't use 'bottom' protection */
	if (!stm_is_unlocked_sr(nor, ofs + len, mtd->size - (ofs + len),
				status_old))
		can_be_bottom = false;

	if (!can_be_bottom && !can_be_top)
		return -EINVAL;

	/* Prefer top, if both are valid */
	use_top = can_be_top;

	/* lock_len: length of region that should remain locked */
	if (use_top)
		lock_len = mtd->size - (ofs + len);
	else
		lock_len = ofs;

	/*
	 * Need largest pow such that:
	 *
	 *   1 / (2^pow) >= (len / size)
	 *
	 * so (assuming power-of-2 size) we do:
	 *
	 *   pow = floor(log2(size / len)) = log2(size) - ceil(log2(len))
	 */
	pow = ilog2(mtd->size) - order_base_2(lock_len);
	if (lock_len == 0) {
		val = 0; /* fully unlocked */
	} else {
		val = mask - (pow << shift);
		/* Some power-of-two sizes are not supported */
		if (val & ~mask)
			return -EINVAL;
	}

	status_new = (status_old & ~mask & ~SR_TB) | val;

	/* Don't protect status register if we're fully unlocked */
	if (lock_len == 0)
		status_new &= ~SR_SRWD;

	if (!use_top)
		status_new |= SR_TB;

	/* Don't bother if they're the same */
	if (status_new == status_old)
		return 0;

	/* Only modify protection if it will not lock other areas */
	if ((status_new & mask) > (status_old & mask))
		return -EINVAL;

	return write_sr_and_check(nor, status_new, mask);
}

/*
 * Check if a region of the flash is (completely) unlocked. See stm_lock() for
 * more info.
 *
 * Returns 1 if entire region is unlocked, 0 if any portion is locked, and
 * negative on errors.
 */
static int stm_is_unlocked(struct spi_nor *nor, loff_t ofs, uint64_t len)
{
	int status;

	status = read_sr(nor);
	if (status < 0)
		return status;

	return stm_is_unlocked_sr(nor, ofs, len, status);
}
#endif /* CONFIG_SPI_FLASH_STMICRO */
#endif

static const struct flash_info *spi_nor_read_id(struct spi_nor *nor)
{
	int			tmp;
	u8			id[SPI_NOR_MAX_ID_LEN];
	const struct flash_info	*info;

	if (nor->flags & SNOR_F_HAS_PARALLEL)
		nor->spi->flags |= SPI_XFER_LOWER;

	tmp = nor->read_reg(nor, SPINOR_OP_RDID, id, SPI_NOR_MAX_ID_LEN);
	if (tmp < 0) {
		dev_dbg(nor->dev, "error %d reading JEDEC ID\n", tmp);
		return ERR_PTR(tmp);
	}

	info = spi_nor_ids;
	for (; info->name; info++) {
		if (info->id_len) {
			if (!memcmp(info->id, id, info->id_len))
				return info;
		}
	}

	dev_err(nor->dev, "unrecognized JEDEC id bytes: %02x, %02x, %02x\n",
		id[0], id[1], id[2]);
	return ERR_PTR(-ENODEV);
}

static int spi_nor_read(struct mtd_info *mtd, loff_t from, size_t len,
			size_t *retlen, u_char *buf)
{
	struct spi_nor *nor = mtd_to_spi_nor(mtd);
	loff_t offset = from;
	u32 rem_bank_len = 0;
	u32 stack_shift = 0;
	size_t read_len;
	u8 bank;
	int ret;
	bool is_ofst_odd = false;

	dev_dbg(nor->dev, "from 0x%08x, len %zd\n", (u32)from, len);

	if ((nor->flags & SNOR_F_HAS_PARALLEL) && (offset & 1)) {
		/* We can hit this case when we use file system like ubifs */
		from--;
		len++;
		is_ofst_odd = true;
	}

	while (len) {
		read_len = len;
		offset = from;

		if (CONFIG_IS_ENABLED(SPI_FLASH_BAR)) {
			bank = (u32)from / SZ_16M;
			if (CONFIG_IS_ENABLED(SPI_STACKED_PARALLEL)) {
				if (nor->flags & SNOR_F_HAS_PARALLEL)
					bank /= 2;
			}
			rem_bank_len = SZ_16M * (bank + 1);
			if (CONFIG_IS_ENABLED(SPI_STACKED_PARALLEL)) {
				if (nor->flags & SNOR_F_HAS_PARALLEL)
					rem_bank_len *= 2;
			}
			rem_bank_len -= from;
		}

		if (CONFIG_IS_ENABLED(SPI_STACKED_PARALLEL)) {
			if (nor->flags & SNOR_F_HAS_STACKED) {
				stack_shift = 1;
				if (offset >= (mtd->size / 2)) {
					offset = offset - (mtd->size / 2);
					nor->spi->flags |= SPI_XFER_U_PAGE;
				} else {
					nor->spi->flags &= ~SPI_XFER_U_PAGE;
				}
			}
		}

		if (CONFIG_IS_ENABLED(SPI_STACKED_PARALLEL)) {
			if (nor->flags & SNOR_F_HAS_PARALLEL)
				offset /= 2;
		}

#ifdef CONFIG_SPI_FLASH_BAR
		ret = write_bar(nor, offset);
		if (ret < 0)
			return log_ret(ret);
		if (len < rem_bank_len)
			read_len = len;
		else
			read_len = rem_bank_len;
#endif

		if (read_len == 0)
			return -EIO;

		ret = nor->read(nor, offset, read_len, buf);
		if (ret == 0) {
			/* We shouldn't see 0-length reads */
			ret = -EIO;
			goto read_err;
		}
		if (ret < 0)
			goto read_err;

		if (is_ofst_odd == true) {
			memmove(buf, (buf + 1), (len - 1));
			*retlen += (ret - 1);
			buf += ret - 1;
			is_ofst_odd = false;
		} else {
			*retlen += ret;
			buf += ret;
		}
		from += ret;
		len -= ret;
	}
	ret = 0;

read_err:
#ifdef CONFIG_SPI_FLASH_BAR
	ret = clean_bar(nor);
#endif
	return ret;
}

#if defined(CONFIG_SPI_FLASH_LOCK)
#ifdef CONFIG_SPI_FLASH_SST
/*
 * sst26 flash series has its own block protection implementation:
 * 4x   - 8  KByte blocks - read & write protection bits - upper addresses
 * 1x   - 32 KByte blocks - write protection bits
 * rest - 64 KByte blocks - write protection bits
 * 1x   - 32 KByte blocks - write protection bits
 * 4x   - 8  KByte blocks - read & write protection bits - lower addresses
 *
 * We'll support only per 64k lock/unlock so lower and upper 64 KByte region
 * will be treated as single block.
 */
#define SST26_BPR_8K_NUM		4
#define SST26_MAX_BPR_REG_LEN		(18 + 1)
#define SST26_BOUND_REG_SIZE		((32 + SST26_BPR_8K_NUM * 8) * SZ_1K)

enum lock_ctl {
	SST26_CTL_LOCK,
	SST26_CTL_UNLOCK,
	SST26_CTL_CHECK
};

static bool sst26_process_bpr(u32 bpr_size, u8 *cmd, u32 bit, enum lock_ctl ctl)
{
	switch (ctl) {
	case SST26_CTL_LOCK:
		cmd[bpr_size - (bit / 8) - 1] |= BIT(bit % 8);
		break;
	case SST26_CTL_UNLOCK:
		cmd[bpr_size - (bit / 8) - 1] &= ~BIT(bit % 8);
		break;
	case SST26_CTL_CHECK:
		return !!(cmd[bpr_size - (bit / 8) - 1] & BIT(bit % 8));
	}

	return false;
}

/*
 * Lock, unlock or check lock status of the flash region of the flash (depending
 * on the lock_ctl value)
 */
static int sst26_lock_ctl(struct spi_nor *nor, loff_t ofs, uint64_t len, enum lock_ctl ctl)
{
	struct mtd_info *mtd = &nor->mtd;
	u32 i, bpr_ptr, rptr_64k, lptr_64k, bpr_size;
	bool lower_64k = false, upper_64k = false;
	u8 bpr_buff[SST26_MAX_BPR_REG_LEN] = {};
	int ret;

	/* Check length and offset for 64k alignment */
	if ((ofs & (SZ_64K - 1)) || (len & (SZ_64K - 1))) {
		dev_err(nor->dev, "length or offset is not 64KiB allighned\n");
		return -EINVAL;
	}

	if (ofs + len > mtd->size) {
		dev_err(nor->dev, "range is more than device size: %#llx + %#llx > %#llx\n",
			ofs, len, mtd->size);
		return -EINVAL;
	}

	/* SST26 family has only 16 Mbit, 32 Mbit and 64 Mbit IC */
	if (mtd->size != SZ_2M &&
	    mtd->size != SZ_4M &&
	    mtd->size != SZ_8M)
		return -EINVAL;

	bpr_size = 2 + (mtd->size / SZ_64K / 8);

	ret = nor->read_reg(nor, SPINOR_OP_READ_BPR, bpr_buff, bpr_size);
	if (ret < 0) {
		dev_err(nor->dev, "fail to read block-protection register\n");
		return ret;
	}

	rptr_64k = min_t(u32, ofs + len, mtd->size - SST26_BOUND_REG_SIZE);
	lptr_64k = max_t(u32, ofs, SST26_BOUND_REG_SIZE);

	upper_64k = ((ofs + len) > (mtd->size - SST26_BOUND_REG_SIZE));
	lower_64k = (ofs < SST26_BOUND_REG_SIZE);

	/* Lower bits in block-protection register are about 64k region */
	bpr_ptr = lptr_64k / SZ_64K - 1;

	/* Process 64K blocks region */
	while (lptr_64k < rptr_64k) {
		if (sst26_process_bpr(bpr_size, bpr_buff, bpr_ptr, ctl))
			return EACCES;

		bpr_ptr++;
		lptr_64k += SZ_64K;
	}

	/* 32K and 8K region bits in BPR are after 64k region bits */
	bpr_ptr = (mtd->size - 2 * SST26_BOUND_REG_SIZE) / SZ_64K;

	/* Process lower 32K block region */
	if (lower_64k)
		if (sst26_process_bpr(bpr_size, bpr_buff, bpr_ptr, ctl))
			return EACCES;

	bpr_ptr++;

	/* Process upper 32K block region */
	if (upper_64k)
		if (sst26_process_bpr(bpr_size, bpr_buff, bpr_ptr, ctl))
			return EACCES;

	bpr_ptr++;

	/* Process lower 8K block regions */
	for (i = 0; i < SST26_BPR_8K_NUM; i++) {
		if (lower_64k)
			if (sst26_process_bpr(bpr_size, bpr_buff, bpr_ptr, ctl))
				return EACCES;

		/* In 8K area BPR has both read and write protection bits */
		bpr_ptr += 2;
	}

	/* Process upper 8K block regions */
	for (i = 0; i < SST26_BPR_8K_NUM; i++) {
		if (upper_64k)
			if (sst26_process_bpr(bpr_size, bpr_buff, bpr_ptr, ctl))
				return EACCES;

		/* In 8K area BPR has both read and write protection bits */
		bpr_ptr += 2;
	}

	/* If we check region status we don't need to write BPR back */
	if (ctl == SST26_CTL_CHECK)
		return 0;

	ret = nor->write_reg(nor, SPINOR_OP_WRITE_BPR, bpr_buff, bpr_size);
	if (ret < 0) {
		dev_err(nor->dev, "fail to write block-protection register\n");
		return ret;
	}

	return 0;
}

static int sst26_unlock(struct spi_nor *nor, loff_t ofs, uint64_t len)
{
	return sst26_lock_ctl(nor, ofs, len, SST26_CTL_UNLOCK);
}

static int sst26_lock(struct spi_nor *nor, loff_t ofs, uint64_t len)
{
	return sst26_lock_ctl(nor, ofs, len, SST26_CTL_LOCK);
}

/*
 * Returns EACCES (positive value) if region is (partially) locked, 0 if region
 * is completely unlocked, and negative on errors.
 */
static int sst26_is_unlocked(struct spi_nor *nor, loff_t ofs, uint64_t len)
{
	/*
	 * is_unlocked function is used for check before reading or erasing
	 * flash region, so offset and length might be not 64k aligned, so
	 * adjust them to be 64k aligned as sst26_lock_ctl works only with 64k
	 * aligned regions.
	 */
	ofs -= ofs & (SZ_64K - 1);
	len = len & (SZ_64K - 1) ? (len & ~(SZ_64K - 1)) + SZ_64K : len;

	return !sst26_lock_ctl(nor, ofs, len, SST26_CTL_CHECK);
}

static int sst_write_byteprogram(struct spi_nor *nor, loff_t to, size_t len,
				 size_t *retlen, const u_char *buf)
{
	size_t actual;
	int ret = 0;

	for (actual = 0; actual < len; actual++) {
		nor->program_opcode = SPINOR_OP_BP;

		write_enable(nor);
		/* write one byte. */
		ret = nor->write(nor, to, 1, buf + actual);
		if (ret < 0)
			goto sst_write_err;
		ret = spi_nor_wait_till_ready(nor);
		if (ret)
			goto sst_write_err;
		to++;
	}

sst_write_err:
	write_disable(nor);
	return ret;
}

static int sst_write(struct mtd_info *mtd, loff_t to, size_t len,
		     size_t *retlen, const u_char *buf)
{
	struct spi_nor *nor = mtd_to_spi_nor(mtd);
	struct spi_slave *spi = nor->spi;
	size_t actual;
	int ret;

	dev_dbg(nor->dev, "to 0x%08x, len %zd\n", (u32)to, len);
	if (spi->mode & SPI_TX_BYTE)
		return sst_write_byteprogram(nor, to, len, retlen, buf);

	write_enable(nor);

	nor->sst_write_second = false;

	actual = to % 2;
	/* Start write from odd address. */
	if (actual) {
		nor->program_opcode = SPINOR_OP_BP;

		/* write one byte. */
		ret = nor->write(nor, to, 1, buf);
		if (ret < 0)
			goto sst_write_err;
		ret = spi_nor_wait_till_ready(nor);
		if (ret)
			goto sst_write_err;
	}
	to += actual;

	/* Write out most of the data here. */
	for (; actual < len - 1; actual += 2) {
		nor->program_opcode = SPINOR_OP_AAI_WP;

		/* write two bytes. */
		ret = nor->write(nor, to, 2, buf + actual);
		if (ret < 0)
			goto sst_write_err;
		ret = spi_nor_wait_till_ready(nor);
		if (ret)
			goto sst_write_err;
		to += 2;
		nor->sst_write_second = true;
	}
	nor->sst_write_second = false;

	write_disable(nor);
	ret = spi_nor_wait_till_ready(nor);
	if (ret)
		goto sst_write_err;

	/* Write out trailing byte if it exists. */
	if (actual != len) {
		write_enable(nor);

		nor->program_opcode = SPINOR_OP_BP;
		ret = nor->write(nor, to, 1, buf + actual);
		if (ret < 0)
			goto sst_write_err;
		ret = spi_nor_wait_till_ready(nor);
		if (ret)
			goto sst_write_err;
		write_disable(nor);
		actual += 1;
	}
sst_write_err:
	*retlen += actual;
	return ret;
}
#endif
#endif

/*
 * Write an address range to the nor chip.  Data must be written in
 * FLASH_PAGESIZE chunks.  The address range may be any size provided
 * it is within the physical boundaries.
 */
static int spi_nor_write(struct mtd_info *mtd, loff_t to, size_t len,
	size_t *retlen, const u_char *buf)
{
	struct spi_nor *nor = mtd_to_spi_nor(mtd);
	size_t page_offset, page_remain, i;
	ssize_t ret;
	u32 offset;

#ifdef CONFIG_SPI_FLASH_SST
	/* sst nor chips use AAI word program */
	if (nor->info->flags & SST_WRITE)
		return sst_write(mtd, to, len, retlen, buf);
#endif

	dev_dbg(nor->dev, "to 0x%08x, len %zd\n", (u32)to, len);

	if (!len)
		return 0;

	/*
	 * Cannot write to odd offset in parallel mode,
	 * so write 2 bytes first
	 */
	if ((nor->flags & SNOR_F_HAS_PARALLEL) && (to & 1)) {
		u8 two[2] = {0xff, buf[0]};
		size_t local_retlen;

		ret = spi_nor_write(mtd, to & ~1, 2, &local_retlen, two);
		if (ret < 0)
			return ret;

		*retlen += 1; /* We've written only one actual byte */
		buf++;
		len--;
		to++;
	}

	for (i = 0; i < len; ) {
		ssize_t written;
		loff_t addr = to + i;
		schedule();

		/*
		 * If page_size is a power of two, the offset can be quickly
		 * calculated with an AND operation. On the other cases we
		 * need to do a modulus operation (more expensive).
		 */
		if (is_power_of_2(nor->page_size)) {
			page_offset = addr & (nor->page_size - 1);
		} else {
			u64 aux = addr;

			page_offset = do_div(aux, nor->page_size);
		}
		offset = to + i;
		if (nor->flags & SNOR_F_HAS_PARALLEL)
			offset /= 2;

		if (nor->flags & SNOR_F_HAS_STACKED) {
			if (offset >= (mtd->size / 2)) {
				offset = offset - (mtd->size / 2);
				nor->spi->flags |= SPI_XFER_U_PAGE;
			} else {
				nor->spi->flags &= ~SPI_XFER_U_PAGE;
			}
		}

#ifdef CONFIG_SPI_FLASH_BAR
		ret = write_bar(nor, offset);
		if (ret < 0)
			return ret;
#endif

		/* the size of data remaining on the first page */
		page_remain = min_t(size_t,
				    nor->page_size - page_offset, len - i);

		write_enable(nor);
		/*
		 * On DTR capable flashes like Micron Xcella the writes cannot
		 * start or end at an odd address in DTR mode. So we need to
		 * append or prepend extra 0xff bytes to make sure the start
		 * address and end address are even.
		 */
		if (spi_nor_protocol_is_dtr(nor->write_proto) &&
		    ((offset | page_remain) & 1)) {
			u_char *tmp;
			size_t extra_bytes = 0;

			tmp = kmalloc(nor->page_size, 0);
			if (!tmp) {
				ret = -ENOMEM;
				goto write_err;
			}

			/* Prepend a 0xff byte if the start address is odd. */
			if (offset & 1) {
				tmp[0] = 0xff;
				memcpy(tmp + 1, buf + i, page_remain);
				offset--;
				page_remain++;
				extra_bytes++;
			} else {
				memcpy(tmp, buf + i, page_remain);
			}

			/* Append a 0xff byte if the end address is odd. */
			if ((offset + page_remain) & 1) {
				tmp[page_remain + extra_bytes] = 0xff;
				extra_bytes++;
				page_remain++;
			}

			ret = nor->write(nor, offset, page_remain, tmp);

			kfree(tmp);

			if (ret < 0)
				goto write_err;

			/*
			 * We write extra bytes but they are not part of the
			 * original write.
			 */
			written = ret - extra_bytes;
		} else {
			ret = nor->write(nor, offset, page_remain, buf + i);
			if (ret < 0)
				goto write_err;
			written = ret;
		}

		ret = spi_nor_wait_till_ready(nor);
		if (ret)
			goto write_err;

		*retlen += written;
		i += written;
	}

write_err:
#ifdef CONFIG_SPI_FLASH_BAR
	ret = clean_bar(nor);
#endif
	return ret;
}

#if defined(CONFIG_SPI_FLASH_MACRONIX) || defined(CONFIG_SPI_FLASH_ISSI)
/**
 * macronix_quad_enable() - set QE bit in Status Register.
 * @nor:	pointer to a 'struct spi_nor'
 *
 * Set the Quad Enable (QE) bit in the Status Register.
 *
 * bit 6 of the Status Register is the QE bit for Macronix like QSPI memories.
 *
 * Return: 0 on success, -errno otherwise.
 */
static int macronix_quad_enable(struct spi_nor *nor)
{
	int ret, val;

	val = read_sr(nor);
	if (val < 0)
		return val;
	if (val & SR_QUAD_EN_MX)
		return 0;

	write_enable(nor);

	write_sr(nor, val | SR_QUAD_EN_MX);

	ret = spi_nor_wait_till_ready(nor);
	if (ret)
		return ret;

	ret = read_sr(nor);
	if (!(ret > 0 && (ret & SR_QUAD_EN_MX))) {
		dev_err(nor->dev, "Macronix Quad bit not set\n");
		return -EINVAL;
	}

	return 0;
}
#endif

#ifdef CONFIG_SPI_FLASH_SPANSION
/**
 * spansion_quad_enable_volatile() - enable Quad I/O mode in volatile register.
 * @nor:	pointer to a 'struct spi_nor'
 * @addr_base:	base address of register (can be >0 in multi-die parts)
 * @dummy:	number of dummy cycles for register read
 *
 * It is recommended to update volatile registers in the field application due
 * to a risk of the non-volatile registers corruption by power interrupt. This
 * function sets Quad Enable bit in CFR1 volatile.
 *
 * Return: 0 on success, -errno otherwise.
 */
static int spansion_quad_enable_volatile(struct spi_nor *nor, u32 addr_base,
					 u8 dummy)
{
	u32 addr = addr_base + SPINOR_REG_CYPRESS_CFR1V;

	u8 cr;
	int ret;

	/* Check current Quad Enable bit value. */
	ret = spansion_read_any_reg(nor, addr, dummy, &cr);
	if (ret < 0) {
		dev_dbg(nor->dev,
			"error while reading configuration register\n");
		return -EINVAL;
	}

	if (cr & CR_QUAD_EN_SPAN)
		return 0;

	cr |= CR_QUAD_EN_SPAN;

	write_enable(nor);

	ret = spansion_write_any_reg(nor, addr, cr);

	if (ret < 0) {
		dev_dbg(nor->dev,
			"error while writing configuration register\n");
		return -EINVAL;
	}

	/* Read back and check it. */
	ret = spansion_read_any_reg(nor, addr, dummy, &cr);
	if (ret || !(cr & CR_QUAD_EN_SPAN)) {
		dev_dbg(nor->dev, "Spansion Quad bit not set\n");
		return -EINVAL;
	}

	return 0;
}
#endif

#if defined(CONFIG_SPI_FLASH_SPANSION) || defined(CONFIG_SPI_FLASH_WINBOND)
/*
 * Write status Register and configuration register with 2 bytes
 * The first byte will be written to the status register, while the
 * second byte will be written to the configuration register.
 * Return negative if error occurred.
 */
static int write_sr_cr(struct spi_nor *nor, u8 *sr_cr)
{
	int ret;

	write_enable(nor);

	ret = nor->write_reg(nor, SPINOR_OP_WRSR, sr_cr, 2);
	if (ret < 0) {
		dev_dbg(nor->dev,
			"error while writing configuration register\n");
		return -EINVAL;
	}

	ret = spi_nor_wait_till_ready(nor);
	if (ret) {
		dev_dbg(nor->dev,
			"timeout while writing configuration register\n");
		return ret;
	}

	return 0;
}

/**
 * spansion_read_cr_quad_enable() - set QE bit in Configuration Register.
 * @nor:	pointer to a 'struct spi_nor'
 *
 * Set the Quad Enable (QE) bit in the Configuration Register.
 * This function should be used with QSPI memories supporting the Read
 * Configuration Register (35h) instruction.
 *
 * bit 1 of the Configuration Register is the QE bit for Spansion like QSPI
 * memories.
 *
 * Return: 0 on success, -errno otherwise.
 */
static int spansion_read_cr_quad_enable(struct spi_nor *nor)
{
	u8 sr_cr[2];
	int ret;

	/* Check current Quad Enable bit value. */
	ret = read_cr(nor);
	if (ret < 0) {
		dev_dbg(nor->dev,
			"error while reading configuration register\n");
		return -EINVAL;
	}

	if (ret & CR_QUAD_EN_SPAN)
		return 0;

	sr_cr[1] = ret | CR_QUAD_EN_SPAN;

	/* Keep the current value of the Status Register. */
	ret = read_sr(nor);
	if (ret < 0) {
		dev_dbg(nor->dev, "error while reading status register\n");
		return -EINVAL;
	}
	sr_cr[0] = ret;

	ret = write_sr_cr(nor, sr_cr);
	if (ret)
		return ret;

	/* Read back and check it. */
	ret = read_cr(nor);
	if (!(ret > 0 && (ret & CR_QUAD_EN_SPAN))) {
		dev_dbg(nor->dev, "Spansion Quad bit not set\n");
		return -EINVAL;
	}

	return 0;
}

#if CONFIG_IS_ENABLED(SPI_FLASH_SFDP_SUPPORT)
/**
 * spansion_no_read_cr_quad_enable() - set QE bit in Configuration Register.
 * @nor:	pointer to a 'struct spi_nor'
 *
 * Set the Quad Enable (QE) bit in the Configuration Register.
 * This function should be used with QSPI memories not supporting the Read
 * Configuration Register (35h) instruction.
 *
 * bit 1 of the Configuration Register is the QE bit for Spansion like QSPI
 * memories.
 *
 * Return: 0 on success, -errno otherwise.
 */
static int spansion_no_read_cr_quad_enable(struct spi_nor *nor)
{
	u8 sr_cr[2];
	int ret;

	/* Keep the current value of the Status Register. */
	ret = read_sr(nor);
	if (ret < 0) {
		dev_dbg(nor->dev, "error while reading status register\n");
		return -EINVAL;
	}
	sr_cr[0] = ret;
	sr_cr[1] = CR_QUAD_EN_SPAN;

	return write_sr_cr(nor, sr_cr);
}

#endif /* CONFIG_SPI_FLASH_SFDP_SUPPORT */
#endif /* CONFIG_SPI_FLASH_SPANSION */

static void
spi_nor_set_read_settings(struct spi_nor_read_command *read,
			  u8 num_mode_clocks,
			  u8 num_wait_states,
			  u8 opcode,
			  enum spi_nor_protocol proto)
{
	read->num_mode_clocks = num_mode_clocks;
	read->num_wait_states = num_wait_states;
	read->opcode = opcode;
	read->proto = proto;
}

static void
spi_nor_set_pp_settings(struct spi_nor_pp_command *pp,
			u8 opcode,
			enum spi_nor_protocol proto)
{
	pp->opcode = opcode;
	pp->proto = proto;
}

#if CONFIG_IS_ENABLED(SPI_FLASH_SFDP_SUPPORT)
/*
 * Serial Flash Discoverable Parameters (SFDP) parsing.
 */

/**
 * spi_nor_read_sfdp() - read Serial Flash Discoverable Parameters.
 * @nor:	pointer to a 'struct spi_nor'
 * @addr:	offset in the SFDP area to start reading data from
 * @len:	number of bytes to read
 * @buf:	buffer where the SFDP data are copied into (dma-safe memory)
 *
 * Whatever the actual numbers of bytes for address and dummy cycles are
 * for (Fast) Read commands, the Read SFDP (5Ah) instruction is always
 * followed by a 3-byte address and 8 dummy clock cycles.
 *
 * Return: 0 on success, -errno otherwise.
 */
static int spi_nor_read_sfdp(struct spi_nor *nor, u32 addr,
			     size_t len, void *buf)
{
	u8 addr_width, read_opcode, read_dummy;
	int ret;

	read_opcode = nor->read_opcode;
	addr_width = nor->addr_width;
	read_dummy = nor->read_dummy;

	nor->read_opcode = SPINOR_OP_RDSFDP;
	nor->addr_width = 3;
	nor->read_dummy = 8;

	while (len) {
		/* Both chips are identical, so should be the SFDP data */
		if (nor->flags & SNOR_F_HAS_PARALLEL)
			nor->spi->flags |= SPI_XFER_LOWER;

		ret = nor->read(nor, addr, len, (u8 *)buf);
		if (!ret || ret > len) {
			ret = -EIO;
			goto read_err;
		}
		if (ret < 0)
			goto read_err;

		buf += ret;
		addr += ret;
		len -= ret;
	}
	ret = 0;

read_err:
	nor->read_opcode = read_opcode;
	nor->addr_width = addr_width;
	nor->read_dummy = read_dummy;

	return ret;
}

/**
 * spi_nor_read_sfdp_dma_unsafe() - read Serial Flash Discoverable Parameters.
 * @nor:	pointer to a 'struct spi_nor'
 * @addr:	offset in the SFDP area to start reading data from
 * @len:	number of bytes to read
 * @buf:	buffer where the SFDP data are copied into
 *
 * Wrap spi_nor_read_sfdp() using a kmalloc'ed bounce buffer as @buf is now not
 * guaranteed to be dma-safe.
 *
 * Return: -ENOMEM if kmalloc() fails, the return code of spi_nor_read_sfdp()
 *          otherwise.
 */
static int spi_nor_read_sfdp_dma_unsafe(struct spi_nor *nor, u32 addr,
					size_t len, void *buf)
{
	void *dma_safe_buf;
	int ret;

	dma_safe_buf = kmalloc(len, GFP_KERNEL);
	if (!dma_safe_buf)
		return -ENOMEM;

	ret = spi_nor_read_sfdp(nor, addr, len, dma_safe_buf);
	memcpy(buf, dma_safe_buf, len);
	kfree(dma_safe_buf);

	return ret;
}

/* Fast Read settings. */

static void
spi_nor_set_read_settings_from_bfpt(struct spi_nor_read_command *read,
				    u16 half,
				    enum spi_nor_protocol proto)
{
	read->num_mode_clocks = (half >> 5) & 0x07;
	read->num_wait_states = (half >> 0) & 0x1f;
	read->opcode = (half >> 8) & 0xff;
	read->proto = proto;
}

struct sfdp_bfpt_read {
	/* The Fast Read x-y-z hardware capability in params->hwcaps.mask. */
	u32			hwcaps;

	/*
	 * The <supported_bit> bit in <supported_dword> BFPT DWORD tells us
	 * whether the Fast Read x-y-z command is supported.
	 */
	u32			supported_dword;
	u32			supported_bit;

	/*
	 * The half-word at offset <setting_shift> in <setting_dword> BFPT DWORD
	 * encodes the op code, the number of mode clocks and the number of wait
	 * states to be used by Fast Read x-y-z command.
	 */
	u32			settings_dword;
	u32			settings_shift;

	/* The SPI protocol for this Fast Read x-y-z command. */
	enum spi_nor_protocol	proto;
};

static const struct sfdp_bfpt_read sfdp_bfpt_reads[] = {
	/* Fast Read 1-1-2 */
	{
		SNOR_HWCAPS_READ_1_1_2,
		BFPT_DWORD(1), BIT(16),	/* Supported bit */
		BFPT_DWORD(4), 0,	/* Settings */
		SNOR_PROTO_1_1_2,
	},

	/* Fast Read 1-2-2 */
	{
		SNOR_HWCAPS_READ_1_2_2,
		BFPT_DWORD(1), BIT(20),	/* Supported bit */
		BFPT_DWORD(4), 16,	/* Settings */
		SNOR_PROTO_1_2_2,
	},

	/* Fast Read 2-2-2 */
	{
		SNOR_HWCAPS_READ_2_2_2,
		BFPT_DWORD(5),  BIT(0),	/* Supported bit */
		BFPT_DWORD(6), 16,	/* Settings */
		SNOR_PROTO_2_2_2,
	},

	/* Fast Read 1-1-4 */
	{
		SNOR_HWCAPS_READ_1_1_4,
		BFPT_DWORD(1), BIT(22),	/* Supported bit */
		BFPT_DWORD(3), 16,	/* Settings */
		SNOR_PROTO_1_1_4,
	},

	/* Fast Read 1-4-4 */
	{
		SNOR_HWCAPS_READ_1_4_4,
		BFPT_DWORD(1), BIT(21),	/* Supported bit */
		BFPT_DWORD(3), 0,	/* Settings */
		SNOR_PROTO_1_4_4,
	},

	/* Fast Read 4-4-4 */
	{
		SNOR_HWCAPS_READ_4_4_4,
		BFPT_DWORD(5), BIT(4),	/* Supported bit */
		BFPT_DWORD(7), 16,	/* Settings */
		SNOR_PROTO_4_4_4,
	},
};

struct sfdp_bfpt_erase {
	/*
	 * The half-word at offset <shift> in DWORD <dwoard> encodes the
	 * op code and erase sector size to be used by Sector Erase commands.
	 */
	u32			dword;
	u32			shift;
};

static const struct sfdp_bfpt_erase sfdp_bfpt_erases[] = {
	/* Erase Type 1 in DWORD8 bits[15:0] */
	{BFPT_DWORD(8), 0},

	/* Erase Type 2 in DWORD8 bits[31:16] */
	{BFPT_DWORD(8), 16},

	/* Erase Type 3 in DWORD9 bits[15:0] */
	{BFPT_DWORD(9), 0},

	/* Erase Type 4 in DWORD9 bits[31:16] */
	{BFPT_DWORD(9), 16},
};

static int spi_nor_hwcaps_read2cmd(u32 hwcaps);

static int
spi_nor_post_bfpt_fixups(struct spi_nor *nor,
			 const struct sfdp_parameter_header *bfpt_header,
			 const struct sfdp_bfpt *bfpt,
			 struct spi_nor_flash_parameter *params)
{
	if (nor->fixups && nor->fixups->post_bfpt)
		return nor->fixups->post_bfpt(nor, bfpt_header, bfpt, params);

	return 0;
}

/**
 * spi_nor_parse_bfpt() - read and parse the Basic Flash Parameter Table.
 * @nor:		pointer to a 'struct spi_nor'
 * @bfpt_header:	pointer to the 'struct sfdp_parameter_header' describing
 *			the Basic Flash Parameter Table length and version
 * @params:		pointer to the 'struct spi_nor_flash_parameter' to be
 *			filled
 *
 * The Basic Flash Parameter Table is the main and only mandatory table as
 * defined by the SFDP (JESD216) specification.
 * It provides us with the total size (memory density) of the data array and
 * the number of address bytes for Fast Read, Page Program and Sector Erase
 * commands.
 * For Fast READ commands, it also gives the number of mode clock cycles and
 * wait states (regrouped in the number of dummy clock cycles) for each
 * supported instruction op code.
 * For Page Program, the page size is now available since JESD216 rev A, however
 * the supported instruction op codes are still not provided.
 * For Sector Erase commands, this table stores the supported instruction op
 * codes and the associated sector sizes.
 * Finally, the Quad Enable Requirements (QER) are also available since JESD216
 * rev A. The QER bits encode the manufacturer dependent procedure to be
 * executed to set the Quad Enable (QE) bit in some internal register of the
 * Quad SPI memory. Indeed the QE bit, when it exists, must be set before
 * sending any Quad SPI command to the memory. Actually, setting the QE bit
 * tells the memory to reassign its WP# and HOLD#/RESET# pins to functions IO2
 * and IO3 hence enabling 4 (Quad) I/O lines.
 *
 * Return: 0 on success, -errno otherwise.
 */
static int spi_nor_parse_bfpt(struct spi_nor *nor,
			      const struct sfdp_parameter_header *bfpt_header,
			      struct spi_nor_flash_parameter *params)
{
	struct mtd_info *mtd = &nor->mtd;
	struct sfdp_bfpt bfpt;
	size_t len;
	int i, cmd, err;
	u32 addr;
	u16 half;

	/* JESD216 Basic Flash Parameter Table length is at least 9 DWORDs. */
	if (bfpt_header->length < BFPT_DWORD_MAX_JESD216)
		return -EINVAL;

	/* Read the Basic Flash Parameter Table. */
	len = min_t(size_t, sizeof(bfpt),
		    bfpt_header->length * sizeof(u32));
	addr = SFDP_PARAM_HEADER_PTP(bfpt_header);
	memset(&bfpt, 0, sizeof(bfpt));
	err = spi_nor_read_sfdp_dma_unsafe(nor,  addr, len, &bfpt);
	if (err < 0)
		return err;

	/* Fix endianness of the BFPT DWORDs. */
	for (i = 0; i < BFPT_DWORD_MAX; i++)
		bfpt.dwords[i] = le32_to_cpu(bfpt.dwords[i]);

	/* Number of address bytes. */
	switch (bfpt.dwords[BFPT_DWORD(1)] & BFPT_DWORD1_ADDRESS_BYTES_MASK) {
	case BFPT_DWORD1_ADDRESS_BYTES_3_ONLY:
	case BFPT_DWORD1_ADDRESS_BYTES_3_OR_4:
		nor->addr_width = 3;
		nor->addr_mode_nbytes = 3;
		break;

	case BFPT_DWORD1_ADDRESS_BYTES_4_ONLY:
		nor->addr_width = 4;
		nor->addr_mode_nbytes = 4;
		break;

	default:
		break;
	}

	/* Flash Memory Density (in bits). */
	params->size = bfpt.dwords[BFPT_DWORD(2)];
	if (params->size & BIT(31)) {
		params->size &= ~BIT(31);

		/*
		 * Prevent overflows on params->size. Anyway, a NOR of 2^64
		 * bits is unlikely to exist so this error probably means
		 * the BFPT we are reading is corrupted/wrong.
		 */
		if (params->size > 63)
			return -EINVAL;

		params->size = 1ULL << params->size;
	} else {
		params->size++;
	}
	params->size >>= 3; /* Convert to bytes. */

	/* Fast Read settings. */
	for (i = 0; i < ARRAY_SIZE(sfdp_bfpt_reads); i++) {
		const struct sfdp_bfpt_read *rd = &sfdp_bfpt_reads[i];
		struct spi_nor_read_command *read;

		if (!(bfpt.dwords[rd->supported_dword] & rd->supported_bit)) {
			params->hwcaps.mask &= ~rd->hwcaps;
			continue;
		}

		params->hwcaps.mask |= rd->hwcaps;
		cmd = spi_nor_hwcaps_read2cmd(rd->hwcaps);
		read = &params->reads[cmd];
		half = bfpt.dwords[rd->settings_dword] >> rd->settings_shift;
		spi_nor_set_read_settings_from_bfpt(read, half, rd->proto);
	}

	/* Sector Erase settings. */
	for (i = 0; i < ARRAY_SIZE(sfdp_bfpt_erases); i++) {
		const struct sfdp_bfpt_erase *er = &sfdp_bfpt_erases[i];
		u32 erasesize;
		u8 opcode;

		half = bfpt.dwords[er->dword] >> er->shift;
		erasesize = half & 0xff;

		/* erasesize == 0 means this Erase Type is not supported. */
		if (!erasesize)
			continue;

		erasesize = 1U << erasesize;
		opcode = (half >> 8) & 0xff;
#ifdef CONFIG_SPI_FLASH_USE_4K_SECTORS
		if (erasesize == SZ_4K) {
			nor->erase_opcode = opcode;
			mtd->erasesize = erasesize;
			break;
		}
#endif
		if (!mtd->erasesize || mtd->erasesize < erasesize) {
			nor->erase_opcode = opcode;
			mtd->erasesize = erasesize;
		}
	}

	/* Stop here if not JESD216 rev A or later. */
	if (bfpt_header->length == BFPT_DWORD_MAX_JESD216)
		return spi_nor_post_bfpt_fixups(nor, bfpt_header, &bfpt,
						params);

	/* Page size: this field specifies 'N' so the page size = 2^N bytes. */
	params->page_size = bfpt.dwords[BFPT_DWORD(11)];
	params->page_size &= BFPT_DWORD11_PAGE_SIZE_MASK;
	params->page_size >>= BFPT_DWORD11_PAGE_SIZE_SHIFT;
	params->page_size = 1U << params->page_size;

	/* Quad Enable Requirements. */
	switch (bfpt.dwords[BFPT_DWORD(15)] & BFPT_DWORD15_QER_MASK) {
	case BFPT_DWORD15_QER_NONE:
		params->quad_enable = NULL;
		break;
#if defined(CONFIG_SPI_FLASH_SPANSION) || defined(CONFIG_SPI_FLASH_WINBOND)
	case BFPT_DWORD15_QER_SR2_BIT1_BUGGY:
	case BFPT_DWORD15_QER_SR2_BIT1_NO_RD:
		params->quad_enable = spansion_no_read_cr_quad_enable;
		break;
#endif
#if defined(CONFIG_SPI_FLASH_MACRONIX) || defined(CONFIG_SPI_FLASH_ISSI)
	case BFPT_DWORD15_QER_SR1_BIT6:
		params->quad_enable = macronix_quad_enable;
		break;
#endif
#if defined(CONFIG_SPI_FLASH_SPANSION) || defined(CONFIG_SPI_FLASH_WINBOND)
	case BFPT_DWORD15_QER_SR2_BIT1:
		params->quad_enable = spansion_read_cr_quad_enable;
		break;
#endif
	default:
		dev_dbg(nor->dev, "BFPT QER reserved value used\n");
		break;
	}

	/* Soft Reset support. */
	if (bfpt.dwords[BFPT_DWORD(16)] & BFPT_DWORD16_SOFT_RST)
		nor->flags |= SNOR_F_SOFT_RESET;

	/* Stop here if JESD216 rev B. */
	if (bfpt_header->length == BFPT_DWORD_MAX_JESD216B)
		return spi_nor_post_bfpt_fixups(nor, bfpt_header, &bfpt,
						params);

	/* 8D-8D-8D command extension. */
	switch (bfpt.dwords[BFPT_DWORD(18)] & BFPT_DWORD18_CMD_EXT_MASK) {
	case BFPT_DWORD18_CMD_EXT_REP:
		nor->cmd_ext_type = SPI_NOR_EXT_REPEAT;
		break;

	case BFPT_DWORD18_CMD_EXT_INV:
		nor->cmd_ext_type = SPI_NOR_EXT_INVERT;
		break;

	case BFPT_DWORD18_CMD_EXT_RES:
		return -EINVAL;

	case BFPT_DWORD18_CMD_EXT_16B:
		dev_err(nor->dev, "16-bit opcodes not supported\n");
		return -ENOTSUPP;
	}

	return spi_nor_post_bfpt_fixups(nor, bfpt_header, &bfpt, params);
}

/**
 * spi_nor_parse_microchip_sfdp() - parse the Microchip manufacturer specific
 * SFDP table.
 * @nor:		pointer to a 'struct spi_nor'.
 * @param_header:	pointer to the SFDP parameter header.
 *
 * Return: 0 on success, -errno otherwise.
 */
static int
spi_nor_parse_microchip_sfdp(struct spi_nor *nor,
			     const struct sfdp_parameter_header *param_header)
{
	size_t size;
	u32 addr;
	int ret;

	size = param_header->length * sizeof(u32);
	addr = SFDP_PARAM_HEADER_PTP(param_header);

	nor->manufacturer_sfdp = devm_kmalloc(nor->dev, size, GFP_KERNEL);
	if (!nor->manufacturer_sfdp)
		return -ENOMEM;

	ret = spi_nor_read_sfdp(nor, addr, size, nor->manufacturer_sfdp);

	return ret;
}

/**
 * spi_nor_parse_profile1() - parse the xSPI Profile 1.0 table
 * @nor:		pointer to a 'struct spi_nor'
 * @profile1_header:	pointer to the 'struct sfdp_parameter_header' describing
 *			the 4-Byte Address Instruction Table length and version.
 * @params:		pointer to the 'struct spi_nor_flash_parameter' to be.
 *
 * Return: 0 on success, -errno otherwise.
 */
static int spi_nor_parse_profile1(struct spi_nor *nor,
				  const struct sfdp_parameter_header *profile1_header,
				  struct spi_nor_flash_parameter *params)
{
	u32 *table, opcode, addr;
	size_t len;
	int ret, i;
	u8 dummy;

	len = profile1_header->length * sizeof(*table);
	table = kmalloc(len, GFP_KERNEL);
	if (!table)
		return -ENOMEM;

	addr = SFDP_PARAM_HEADER_PTP(profile1_header);
	ret = spi_nor_read_sfdp(nor, addr, len, table);
	if (ret)
		goto out;

	/* Fix endianness of the table DWORDs. */
	for (i = 0; i < profile1_header->length; i++)
		table[i] = le32_to_cpu(table[i]);

	/* Get 8D-8D-8D fast read opcode and dummy cycles. */
	opcode = FIELD_GET(PROFILE1_DWORD1_RD_FAST_CMD, table[0]);

	/*
	 * We don't know what speed the controller is running at. Find the
	 * dummy cycles for the fastest frequency the flash can run at to be
	 * sure we are never short of dummy cycles. A value of 0 means the
	 * frequency is not supported.
	 *
	 * Default to PROFILE1_DUMMY_DEFAULT if we don't find anything, and let
	 * flashes set the correct value if needed in their fixup hooks.
	 */
	dummy = FIELD_GET(PROFILE1_DWORD4_DUMMY_200MHZ, table[3]);
	if (!dummy)
		dummy = FIELD_GET(PROFILE1_DWORD5_DUMMY_166MHZ, table[4]);
	if (!dummy)
		dummy = FIELD_GET(PROFILE1_DWORD5_DUMMY_133MHZ, table[4]);
	if (!dummy)
		dummy = FIELD_GET(PROFILE1_DWORD5_DUMMY_100MHZ, table[4]);
	if (!dummy)
		dummy = PROFILE1_DUMMY_DEFAULT;

	/* Round up to an even value to avoid tripping controllers up. */
	dummy = ROUND_UP_TO(dummy, 2);

	/* Update the fast read settings. */
	spi_nor_set_read_settings(&params->reads[SNOR_CMD_READ_8_8_8_DTR],
				  0, dummy, opcode,
				  SNOR_PROTO_8_8_8_DTR);

	/*
	 * Set the Read Status Register dummy cycles and dummy address bytes.
	 */
	if (table[0] & PROFILE1_DWORD1_RDSR_DUMMY)
		params->rdsr_dummy = 8;
	else
		params->rdsr_dummy = 4;

	if (table[0] & PROFILE1_DWORD1_RDSR_ADDR_BYTES)
		params->rdsr_addr_nbytes = 4;
	else
		params->rdsr_addr_nbytes = 0;

out:
	kfree(table);
	return ret;
}

/**
 * spi_nor_parse_sccr() - Parse the Status, Control and Configuration Register
 *			  Map.
 * @nor:		  pointer to a 'struct spi_nor'
 * @sccr_header:	  pointer to the 'struct sfdp_parameter_header' describing
 * 			  the SCCR Map table length and version.
 *
 * Return: 0 on success, -errno otherwise.
 */
static int spi_nor_parse_sccr(struct spi_nor *nor,
			      const struct sfdp_parameter_header *sccr_header)
{
	u32 *table, addr;
	size_t len;
	int ret, i;

	len = sccr_header->length * sizeof(*table);
	table = kmalloc(len, GFP_KERNEL);
	if (!table)
		return -ENOMEM;

	addr = SFDP_PARAM_HEADER_PTP(sccr_header);
	ret = spi_nor_read_sfdp(nor, addr, len, table);
	if (ret)
		goto out;

	/* Fix endianness of the table DWORDs. */
	for (i = 0; i < sccr_header->length; i++)
		table[i] = le32_to_cpu(table[i]);

	if (FIELD_GET(SCCR_DWORD22_OCTAL_DTR_EN_VOLATILE, table[21]))
		nor->flags |= SNOR_F_IO_MODE_EN_VOLATILE;

out:
	kfree(table);
	return ret;
}

/**
 * spi_nor_parse_sfdp() - parse the Serial Flash Discoverable Parameters.
 * @nor:		pointer to a 'struct spi_nor'
 * @params:		pointer to the 'struct spi_nor_flash_parameter' to be
 *			filled
 *
 * The Serial Flash Discoverable Parameters are described by the JEDEC JESD216
 * specification. This is a standard which tends to supported by almost all
 * (Q)SPI memory manufacturers. Those hard-coded tables allow us to learn at
 * runtime the main parameters needed to perform basic SPI flash operations such
 * as Fast Read, Page Program or Sector Erase commands.
 *
 * Return: 0 on success, -errno otherwise.
 */
static int spi_nor_parse_sfdp(struct spi_nor *nor,
			      struct spi_nor_flash_parameter *params)
{
	const struct sfdp_parameter_header *param_header, *bfpt_header;
	struct sfdp_parameter_header *param_headers = NULL;
	struct sfdp_header header;
	size_t psize;
	int i, err;

	/* Get the SFDP header. */
	err = spi_nor_read_sfdp_dma_unsafe(nor, 0, sizeof(header), &header);
	if (err < 0)
		return err;

	/* Check the SFDP header version. */
	if (le32_to_cpu(header.signature) != SFDP_SIGNATURE ||
	    header.major != SFDP_JESD216_MAJOR)
		return -EINVAL;

	/*
	 * Verify that the first and only mandatory parameter header is a
	 * Basic Flash Parameter Table header as specified in JESD216.
	 */
	bfpt_header = &header.bfpt_header;
	if (SFDP_PARAM_HEADER_ID(bfpt_header) != SFDP_BFPT_ID ||
	    bfpt_header->major != SFDP_JESD216_MAJOR)
		return -EINVAL;

	/*
	 * Allocate memory then read all parameter headers with a single
	 * Read SFDP command. These parameter headers will actually be parsed
	 * twice: a first time to get the latest revision of the basic flash
	 * parameter table, then a second time to handle the supported optional
	 * tables.
	 * Hence we read the parameter headers once for all to reduce the
	 * processing time. Also we use kmalloc() instead of devm_kmalloc()
	 * because we don't need to keep these parameter headers: the allocated
	 * memory is always released with kfree() before exiting this function.
	 */
	if (header.nph) {
		psize = header.nph * sizeof(*param_headers);

		param_headers = kmalloc(psize, GFP_KERNEL);
		if (!param_headers)
			return -ENOMEM;

		err = spi_nor_read_sfdp(nor, sizeof(header),
					psize, param_headers);
		if (err < 0) {
			dev_err(nor->dev,
				"failed to read SFDP parameter headers\n");
			goto exit;
		}
	}

	/*
	 * Check other parameter headers to get the latest revision of
	 * the basic flash parameter table.
	 */
	for (i = 0; i < header.nph; i++) {
		param_header = &param_headers[i];

		if (SFDP_PARAM_HEADER_ID(param_header) == SFDP_BFPT_ID &&
		    param_header->major == SFDP_JESD216_MAJOR &&
		    (param_header->minor > bfpt_header->minor ||
		     (param_header->minor == bfpt_header->minor &&
		      param_header->length > bfpt_header->length)))
			bfpt_header = param_header;
	}

	err = spi_nor_parse_bfpt(nor, bfpt_header, params);
	if (err)
		goto exit;

	/* Parse other parameter headers. */
	for (i = 0; i < header.nph; i++) {
		param_header = &param_headers[i];

		switch (SFDP_PARAM_HEADER_ID(param_header)) {
		case SFDP_SECTOR_MAP_ID:
			dev_info(nor->dev,
				 "non-uniform erase sector maps are not supported yet.\n");
			break;

		case SFDP_SST_ID:
			err = spi_nor_parse_microchip_sfdp(nor, param_header);
			break;

		case SFDP_PROFILE1_ID:
			err = spi_nor_parse_profile1(nor, param_header, params);
			break;

		case SFDP_SCCR_MAP_ID:
			err = spi_nor_parse_sccr(nor, param_header);
			break;

		default:
			break;
		}

		if (err) {
			dev_warn(nor->dev,
				 "Failed to parse optional parameter table: %04x\n",
				 SFDP_PARAM_HEADER_ID(param_header));
			/*
			 * Let's not drop all information we extracted so far
			 * if optional table parsers fail. In case of failing,
			 * each optional parser is responsible to roll back to
			 * the previously known spi_nor data.
			 */
			err = 0;
		}
	}

exit:
	kfree(param_headers);
	return err;
}
#else
static int spi_nor_parse_sfdp(struct spi_nor *nor,
			      struct spi_nor_flash_parameter *params)
{
	return -EINVAL;
}
#endif /* SPI_FLASH_SFDP_SUPPORT */

/**
 * spi_nor_post_sfdp_fixups() - Updates the flash's parameters and settings
 * after SFDP has been parsed.
 * @nor:	pointer to a 'struct spi_nor'
 *
 * Typically used to tweak various parameters that could not be extracted by
 * other means (i.e. when information provided by the SFDP tables are incomplete
 * or wrong).
 */
static void spi_nor_post_sfdp_fixups(struct spi_nor *nor,
				     struct spi_nor_flash_parameter *params)
{
	if (nor->fixups && nor->fixups->post_sfdp)
		nor->fixups->post_sfdp(nor, params);
}

static void spi_nor_late_init_fixups(struct spi_nor *nor,
				     struct spi_nor_flash_parameter *params)
{
	if (nor->fixups && nor->fixups->late_init)
		nor->fixups->late_init(nor, params);
}

static int spi_nor_init_params(struct spi_nor *nor,
			       const struct flash_info *info,
			       struct spi_nor_flash_parameter *params)
{
	/* Set legacy flash parameters as default. */
	memset(params, 0, sizeof(*params));

	/* Set SPI NOR sizes. */
	params->writesize = 1;
	params->size = info->sector_size * info->n_sectors;
	params->page_size = info->page_size;

	if (!(info->flags & SPI_NOR_NO_FR)) {
		/* Default to Fast Read for DT and non-DT platform devices. */
		params->hwcaps.mask |= SNOR_HWCAPS_READ_FAST;

		/* Mask out Fast Read if not requested at DT instantiation. */
#if CONFIG_IS_ENABLED(DM_SPI)
		if (!ofnode_read_bool(dev_ofnode(nor->spi->dev),
				      "m25p,fast-read"))
			params->hwcaps.mask &= ~SNOR_HWCAPS_READ_FAST;
#endif
	}

	/* (Fast) Read settings. */
	params->hwcaps.mask |= SNOR_HWCAPS_READ;
	spi_nor_set_read_settings(&params->reads[SNOR_CMD_READ],
				  0, 0, SPINOR_OP_READ,
				  SNOR_PROTO_1_1_1);

	if (params->hwcaps.mask & SNOR_HWCAPS_READ_FAST)
		spi_nor_set_read_settings(&params->reads[SNOR_CMD_READ_FAST],
					  0, 8, SPINOR_OP_READ_FAST,
					  SNOR_PROTO_1_1_1);

	if (info->flags & SPI_NOR_DUAL_READ) {
		params->hwcaps.mask |= SNOR_HWCAPS_READ_1_1_2;
		spi_nor_set_read_settings(&params->reads[SNOR_CMD_READ_1_1_2],
					  0, 8, SPINOR_OP_READ_1_1_2,
					  SNOR_PROTO_1_1_2);
	}

	if (info->flags & SPI_NOR_QUAD_READ) {
		params->hwcaps.mask |= SNOR_HWCAPS_READ_1_1_4;
		spi_nor_set_read_settings(&params->reads[SNOR_CMD_READ_1_1_4],
					  0, 8, SPINOR_OP_READ_1_1_4,
					  SNOR_PROTO_1_1_4);
	}

	if (info->flags & SPI_NOR_OCTAL_READ) {
		params->hwcaps.mask |= SNOR_HWCAPS_READ_1_1_8;
		spi_nor_set_read_settings(&params->reads[SNOR_CMD_READ_1_1_8],
					  0, 8, SPINOR_OP_READ_1_1_8,
					  SNOR_PROTO_1_1_8);
	}

	if (info->flags & SPI_NOR_OCTAL_DTR_READ) {
		params->hwcaps.mask |= SNOR_HWCAPS_READ_8_8_8_DTR;
		spi_nor_set_read_settings(&params->reads[SNOR_CMD_READ_8_8_8_DTR],
					  0, 20, SPINOR_OP_READ_FAST,
					  SNOR_PROTO_8_8_8_DTR);
	}

	/* Page Program settings. */
	params->hwcaps.mask |= SNOR_HWCAPS_PP;
	spi_nor_set_pp_settings(&params->page_programs[SNOR_CMD_PP],
				SPINOR_OP_PP, SNOR_PROTO_1_1_1);

	/*
	 * Since xSPI Page Program opcode is backward compatible with
	 * Legacy SPI, use Legacy SPI opcode there as well.
	 */
	spi_nor_set_pp_settings(&params->page_programs[SNOR_CMD_PP_8_8_8_DTR],
				SPINOR_OP_PP, SNOR_PROTO_8_8_8_DTR);

	if (info->flags & SPI_NOR_QUAD_READ) {
		params->hwcaps.mask |= SNOR_HWCAPS_PP_1_1_4;
		spi_nor_set_pp_settings(&params->page_programs[SNOR_CMD_PP_1_1_4],
					SPINOR_OP_PP_1_1_4, SNOR_PROTO_1_1_4);
	}

	/* Select the procedure to set the Quad Enable bit. */
	if (params->hwcaps.mask & (SNOR_HWCAPS_READ_QUAD |
				   SNOR_HWCAPS_PP_QUAD)) {
		switch (JEDEC_MFR(info)) {
#if defined(CONFIG_SPI_FLASH_MACRONIX) || defined(CONFIG_SPI_FLASH_ISSI)
		case SNOR_MFR_MACRONIX:
		case SNOR_MFR_ISSI:
			params->quad_enable = macronix_quad_enable;
			break;
#endif
		case SNOR_MFR_ST:
		case SNOR_MFR_MICRON:
			break;

		default:
#if defined(CONFIG_SPI_FLASH_SPANSION) || defined(CONFIG_SPI_FLASH_WINBOND)
			/* Kept only for backward compatibility purpose. */
			params->quad_enable = spansion_read_cr_quad_enable;
#endif
			break;
		}
	}

	/* Override the parameters with data read from SFDP tables. */
	nor->addr_width = 0;
	nor->mtd.erasesize = 0;
	if ((info->flags & (SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ |
	     SPI_NOR_OCTAL_DTR_READ)) &&
	    !(info->flags & SPI_NOR_SKIP_SFDP)) {
		struct spi_nor_flash_parameter sfdp_params;

		memcpy(&sfdp_params, params, sizeof(sfdp_params));
		if (spi_nor_parse_sfdp(nor, &sfdp_params)) {
			nor->addr_width = 0;
			nor->mtd.erasesize = 0;
		} else {
			memcpy(params, &sfdp_params, sizeof(*params));
		}

		spi_nor_post_sfdp_fixups(nor, params);
	}

#if CONFIG_IS_ENABLED(DM_SPI)
	if (CONFIG_IS_ENABLED(SPI_STACKED_PARALLEL)) {
		u64 flash_size[SNOR_FLASH_CNT_MAX] = { 0 };
		struct udevice *dev = nor->spi->dev;
		u32 idx = 0, i = 0;
		int rc;

		/*
		 * The flashes that are connected in stacked mode should be of same make.
		 * Except the flash size all other properties are identical for all the
		 * flashes connected in stacked mode.
		 * The flashes that are connected in parallel mode should be identical.
		 */
		while (i < SNOR_FLASH_CNT_MAX) {
			rc = ofnode_read_u64_index(dev_ofnode(dev), "stacked-memories",
						   idx, &flash_size[i]);
			if (rc == -EINVAL) {
				break;
			} else if (rc == -EOVERFLOW) {
				idx++;
			} else {
				idx++;
				i++;
				if (!(nor->flags & SNOR_F_HAS_STACKED))
					nor->flags |= SNOR_F_HAS_STACKED;
				if (!(nor->spi->flags & SPI_XFER_STACKED))
					nor->spi->flags |= SPI_XFER_STACKED;
			}
		}

		i = 0;
		idx = 0;
		while (i < SNOR_FLASH_CNT_MAX) {
			rc = ofnode_read_u64_index(dev_ofnode(dev), "parallel-memories",
						   idx, &flash_size[i]);
			if (rc == -EINVAL) {
				break;
			} else if (rc == -EOVERFLOW) {
				idx++;
			} else {
				idx++;
				i++;
				if (!(nor->flags & SNOR_F_HAS_PARALLEL))
					nor->flags |= SNOR_F_HAS_PARALLEL;
			}
		}

		if (nor->flags & (SNOR_F_HAS_STACKED | SNOR_F_HAS_PARALLEL)) {
			params->size = 0;
			for (idx = 0; idx < SNOR_FLASH_CNT_MAX; idx++)
				params->size += flash_size[idx];
		}
		/*
		 * In parallel-memories the erase operation is
		 * performed on both the flashes simultaneously
		 * so, double the erasesize.
		 */
		if (nor->flags & SNOR_F_HAS_PARALLEL) {
			nor->mtd.erasesize <<= 1;
			params->page_size <<= 1;
		}
	}
#endif

	spi_nor_late_init_fixups(nor, params);

	return 0;
}

static int spi_nor_hwcaps2cmd(u32 hwcaps, const int table[][2], size_t size)
{
	size_t i;

	for (i = 0; i < size; i++)
		if (table[i][0] == (int)hwcaps)
			return table[i][1];

	return -EINVAL;
}

static int spi_nor_hwcaps_read2cmd(u32 hwcaps)
{
	static const int hwcaps_read2cmd[][2] = {
		{ SNOR_HWCAPS_READ,		SNOR_CMD_READ },
		{ SNOR_HWCAPS_READ_FAST,	SNOR_CMD_READ_FAST },
		{ SNOR_HWCAPS_READ_1_1_1_DTR,	SNOR_CMD_READ_1_1_1_DTR },
		{ SNOR_HWCAPS_READ_1_1_2,	SNOR_CMD_READ_1_1_2 },
		{ SNOR_HWCAPS_READ_1_2_2,	SNOR_CMD_READ_1_2_2 },
		{ SNOR_HWCAPS_READ_2_2_2,	SNOR_CMD_READ_2_2_2 },
		{ SNOR_HWCAPS_READ_1_2_2_DTR,	SNOR_CMD_READ_1_2_2_DTR },
		{ SNOR_HWCAPS_READ_1_1_4,	SNOR_CMD_READ_1_1_4 },
		{ SNOR_HWCAPS_READ_1_4_4,	SNOR_CMD_READ_1_4_4 },
		{ SNOR_HWCAPS_READ_4_4_4,	SNOR_CMD_READ_4_4_4 },
		{ SNOR_HWCAPS_READ_1_4_4_DTR,	SNOR_CMD_READ_1_4_4_DTR },
		{ SNOR_HWCAPS_READ_1_1_8,	SNOR_CMD_READ_1_1_8 },
		{ SNOR_HWCAPS_READ_1_8_8,	SNOR_CMD_READ_1_8_8 },
		{ SNOR_HWCAPS_READ_8_8_8,	SNOR_CMD_READ_8_8_8 },
		{ SNOR_HWCAPS_READ_1_8_8_DTR,	SNOR_CMD_READ_1_8_8_DTR },
		{ SNOR_HWCAPS_READ_8_8_8_DTR,	SNOR_CMD_READ_8_8_8_DTR },
	};

	return spi_nor_hwcaps2cmd(hwcaps, hwcaps_read2cmd,
				  ARRAY_SIZE(hwcaps_read2cmd));
}

static int spi_nor_hwcaps_pp2cmd(u32 hwcaps)
{
	static const int hwcaps_pp2cmd[][2] = {
		{ SNOR_HWCAPS_PP,		SNOR_CMD_PP },
		{ SNOR_HWCAPS_PP_1_1_4,		SNOR_CMD_PP_1_1_4 },
		{ SNOR_HWCAPS_PP_1_4_4,		SNOR_CMD_PP_1_4_4 },
		{ SNOR_HWCAPS_PP_4_4_4,		SNOR_CMD_PP_4_4_4 },
		{ SNOR_HWCAPS_PP_1_1_8,		SNOR_CMD_PP_1_1_8 },
		{ SNOR_HWCAPS_PP_1_8_8,		SNOR_CMD_PP_1_8_8 },
		{ SNOR_HWCAPS_PP_8_8_8,		SNOR_CMD_PP_8_8_8 },
		{ SNOR_HWCAPS_PP_8_8_8_DTR,	SNOR_CMD_PP_8_8_8_DTR },
	};

	return spi_nor_hwcaps2cmd(hwcaps, hwcaps_pp2cmd,
				  ARRAY_SIZE(hwcaps_pp2cmd));
}

#ifdef CONFIG_SPI_FLASH_SMART_HWCAPS
/**
 * spi_nor_check_op - check if the operation is supported by controller
 * @nor:        pointer to a 'struct spi_nor'
 * @op:         pointer to op template to be checked
 *
 * Returns 0 if operation is supported, -ENOTSUPP otherwise.
 */
static int spi_nor_check_op(struct spi_nor *nor,
			    struct spi_mem_op *op)
{
	/*
	 * First test with 4 address bytes. The opcode itself might be a 3B
	 * addressing opcode but we don't care, because SPI controller
	 * implementation should not check the opcode, but just the sequence.
	 */
	op->addr.nbytes = 4;
	if (!spi_mem_supports_op(nor->spi, op)) {
		if (nor->mtd.size > SZ_16M)
			return -ENOTSUPP;

		/* If flash size <= 16MB, 3 address bytes are sufficient */
		op->addr.nbytes = 3;
		if (!spi_mem_supports_op(nor->spi, op))
			return -ENOTSUPP;
	}

	return 0;
}

/**
 * spi_nor_check_readop - check if the read op is supported by controller
 * @nor:         pointer to a 'struct spi_nor'
 * @read:        pointer to op template to be checked
 *
 * Returns 0 if operation is supported, -ENOTSUPP otherwise.
 */
static int spi_nor_check_readop(struct spi_nor *nor,
				const struct spi_nor_read_command *read)
{
	struct spi_mem_op op = SPI_MEM_OP(SPI_MEM_OP_CMD(read->opcode, 0),
					  SPI_MEM_OP_ADDR(3, 0, 0),
					  SPI_MEM_OP_DUMMY(1, 0),
					  SPI_MEM_OP_DATA_IN(2, NULL, 0));

	spi_nor_setup_op(nor, &op, read->proto);

	op.dummy.nbytes = (read->num_mode_clocks + read->num_wait_states) *
			  op.dummy.buswidth / 8;
	if (spi_nor_protocol_is_dtr(nor->read_proto))
		op.dummy.nbytes *= 2;

	return spi_nor_check_op(nor, &op);
}

/**
 * spi_nor_check_pp - check if the page program op is supported by controller
 * @nor:         pointer to a 'struct spi_nor'
 * @pp:          pointer to op template to be checked
 *
 * Returns 0 if operation is supported, -ENOTSUPP otherwise.
 */
static int spi_nor_check_pp(struct spi_nor *nor,
			    const struct spi_nor_pp_command *pp)
{
	struct spi_mem_op op = SPI_MEM_OP(SPI_MEM_OP_CMD(pp->opcode, 0),
					  SPI_MEM_OP_ADDR(3, 0, 0),
					  SPI_MEM_OP_NO_DUMMY,
					  SPI_MEM_OP_DATA_OUT(2, NULL, 0));

	spi_nor_setup_op(nor, &op, pp->proto);

	return spi_nor_check_op(nor, &op);
}

/**
 * spi_nor_adjust_hwcaps - Find optimal Read/Write protocol based on SPI
 *                         controller capabilities
 * @nor:        pointer to a 'struct spi_nor'
 * @params:     pointer to the 'struct spi_nor_flash_parameter'
 *              representing SPI NOR flash capabilities
 * @hwcaps:     pointer to resulting capabilities after adjusting
 *              according to controller and flash's capability
 *
 * Discard caps based on what the SPI controller actually supports (using
 * spi_mem_supports_op()).
 */
static void
spi_nor_adjust_hwcaps(struct spi_nor *nor,
		      const struct spi_nor_flash_parameter *params,
		      u32 *hwcaps)
{
	unsigned int cap;

	/*
	 * Start by assuming the controller supports every capability.
	 * We will mask them after checking what's really supported
	 * using spi_mem_supports_op().
	 */
	*hwcaps = SNOR_HWCAPS_ALL & params->hwcaps.mask;

	/* X-X-X modes are not supported yet, mask them all. */
	*hwcaps &= ~SNOR_HWCAPS_X_X_X;

	/*
	 * If the reset line is broken, we do not want to enter a stateful
	 * mode.
	 */
	if (nor->flags & SNOR_F_BROKEN_RESET)
		*hwcaps &= ~(SNOR_HWCAPS_X_X_X | SNOR_HWCAPS_X_X_X_DTR);

	for (cap = 0; cap < sizeof(*hwcaps) * BITS_PER_BYTE; cap++) {
		int rdidx, ppidx;

		if (!(*hwcaps & BIT(cap)))
			continue;

		rdidx = spi_nor_hwcaps_read2cmd(BIT(cap));
		if (rdidx >= 0 &&
		    spi_nor_check_readop(nor, &params->reads[rdidx]))
			*hwcaps &= ~BIT(cap);

		ppidx = spi_nor_hwcaps_pp2cmd(BIT(cap));
		if (ppidx < 0)
			continue;

		if (spi_nor_check_pp(nor, &params->page_programs[ppidx]))
			*hwcaps &= ~BIT(cap);
	}
}
#else
/**
 * spi_nor_adjust_hwcaps - Find optimal Read/Write protocol based on SPI
 *                         controller capabilities
 * @nor:        pointer to a 'struct spi_nor'
 * @params:     pointer to the 'struct spi_nor_flash_parameter'
 *              representing SPI NOR flash capabilities
 * @hwcaps:     pointer to resulting capabilities after adjusting
 *              according to controller and flash's capability
 *
 * Select caps based on what the SPI controller and SPI flash both support.
 */
static void
spi_nor_adjust_hwcaps(struct spi_nor *nor,
		      const struct spi_nor_flash_parameter *params,
		      u32 *hwcaps)
{
	struct spi_slave *spi = nor->spi;
	u32 ignored_mask = (SNOR_HWCAPS_READ_2_2_2 |
			    SNOR_HWCAPS_READ_4_4_4 |
			    SNOR_HWCAPS_READ_8_8_8 |
			    SNOR_HWCAPS_PP_4_4_4   |
			    SNOR_HWCAPS_PP_8_8_8);
	u32 spi_hwcaps = (SNOR_HWCAPS_READ | SNOR_HWCAPS_READ_FAST |
			  SNOR_HWCAPS_PP);

	/* Get the hardware capabilities the SPI controller supports. */
	if (spi->mode & SPI_RX_OCTAL) {
		spi_hwcaps |= SNOR_HWCAPS_READ_1_1_8;

		if (spi->mode & SPI_TX_OCTAL)
			spi_hwcaps |= (SNOR_HWCAPS_READ_1_8_8 |
					SNOR_HWCAPS_PP_1_1_8 |
					SNOR_HWCAPS_PP_1_8_8);
	} else if (spi->mode & SPI_RX_QUAD) {
		spi_hwcaps |= SNOR_HWCAPS_READ_1_1_4;

		if (spi->mode & SPI_TX_QUAD)
			spi_hwcaps |= (SNOR_HWCAPS_READ_1_4_4 |
					SNOR_HWCAPS_PP_1_1_4 |
					SNOR_HWCAPS_PP_1_4_4);
	} else if (spi->mode & SPI_RX_DUAL) {
		spi_hwcaps |= SNOR_HWCAPS_READ_1_1_2;

		if (spi->mode & SPI_TX_DUAL)
			spi_hwcaps |= SNOR_HWCAPS_READ_1_2_2;
	}

	/*
	 * Keep only the hardware capabilities supported by both the SPI
	 * controller and the SPI flash memory.
	 */
	*hwcaps = spi_hwcaps & params->hwcaps.mask;
	if (*hwcaps & ignored_mask) {
		dev_dbg(nor->dev,
			"SPI n-n-n protocols are not supported yet.\n");
		*hwcaps &= ~ignored_mask;
	}
}
#endif /* CONFIG_SPI_FLASH_SMART_HWCAPS */

static int spi_nor_select_read(struct spi_nor *nor,
			       const struct spi_nor_flash_parameter *params,
			       u32 shared_hwcaps)
{
	int cmd, best_match = fls(shared_hwcaps & SNOR_HWCAPS_READ_MASK) - 1;
	const struct spi_nor_read_command *read;

	if (best_match < 0)
		return -EINVAL;

	cmd = spi_nor_hwcaps_read2cmd(BIT(best_match));
	if (cmd < 0)
		return -EINVAL;

	read = &params->reads[cmd];
	nor->read_opcode = read->opcode;
	nor->read_proto = read->proto;

	/*
	 * In the spi-nor framework, we don't need to make the difference
	 * between mode clock cycles and wait state clock cycles.
	 * Indeed, the value of the mode clock cycles is used by a QSPI
	 * flash memory to know whether it should enter or leave its 0-4-4
	 * (Continuous Read / XIP) mode.
	 * eXecution In Place is out of the scope of the mtd sub-system.
	 * Hence we choose to merge both mode and wait state clock cycles
	 * into the so called dummy clock cycles.
	 */
	nor->read_dummy = read->num_mode_clocks + read->num_wait_states;
	return 0;
}

static int spi_nor_select_pp(struct spi_nor *nor,
			     const struct spi_nor_flash_parameter *params,
			     u32 shared_hwcaps)
{
	int cmd, best_match = fls(shared_hwcaps & SNOR_HWCAPS_PP_MASK) - 1;
	const struct spi_nor_pp_command *pp;

	if (best_match < 0)
		return -EINVAL;

	cmd = spi_nor_hwcaps_pp2cmd(BIT(best_match));
	if (cmd < 0)
		return -EINVAL;

	pp = &params->page_programs[cmd];
	nor->program_opcode = pp->opcode;
	nor->write_proto = pp->proto;
	return 0;
}

static int spi_nor_select_erase(struct spi_nor *nor,
				const struct flash_info *info)
{
	struct mtd_info *mtd = &nor->mtd;

	/* Do nothing if already configured from SFDP. */
	if (mtd->erasesize)
		return 0;

#ifdef CONFIG_SPI_FLASH_USE_4K_SECTORS
	/* prefer "small sector" erase if possible */
	if (info->flags & SECT_4K) {
		nor->erase_opcode = SPINOR_OP_BE_4K;
		/*
		 * In parallel-memories the erase operation is
		 * performed on both the flashes simultaneously
		 * so, double the erasesize.
		 */
		if (nor->flags & SNOR_F_HAS_PARALLEL)
			mtd->erasesize = 4096 * 2;
		else
			mtd->erasesize = 4096;
	} else if (info->flags & SECT_4K_PMC) {
		nor->erase_opcode = SPINOR_OP_BE_4K_PMC;
		/*
		 * In parallel-memories the erase operation is
		 * performed on both the flashes simultaneously
		 * so, double the erasesize.
		 */
		if (nor->flags & SNOR_F_HAS_PARALLEL)
			mtd->erasesize = 4096 * 2;
		else
			mtd->erasesize = 4096;
	} else
#endif
	{
		nor->erase_opcode = SPINOR_OP_SE;
		/*
		 * In parallel-memories the erase operation is
		 * performed on both the flashes simultaneously
		 * so, double the erasesize.
		 */
		if (nor->flags & SNOR_F_HAS_PARALLEL)
			mtd->erasesize = info->sector_size * 2;
		else
			mtd->erasesize = info->sector_size;
	}

	return 0;
}

static int spi_nor_default_setup(struct spi_nor *nor,
				 const struct flash_info *info,
				 const struct spi_nor_flash_parameter *params)
{
	u32 shared_mask;
	bool enable_quad_io;
	int err;

	spi_nor_adjust_hwcaps(nor, params, &shared_mask);

	/* Select the (Fast) Read command. */
	err = spi_nor_select_read(nor, params, shared_mask);
	if (err) {
		dev_dbg(nor->dev,
			"can't select read settings supported by both the SPI controller and memory.\n");
		return err;
	}

	/* Select the Page Program command. */
	err = spi_nor_select_pp(nor, params, shared_mask);
	if (err) {
		dev_dbg(nor->dev,
			"can't select write settings supported by both the SPI controller and memory.\n");
		return err;
	}

	/* Select the Sector Erase command. */
	err = spi_nor_select_erase(nor, info);
	if (err) {
		dev_dbg(nor->dev,
			"can't select erase settings supported by both the SPI controller and memory.\n");
		return err;
	}

	/* Enable Quad I/O if needed. */
	enable_quad_io = (spi_nor_get_protocol_width(nor->read_proto) == 4 ||
			  spi_nor_get_protocol_width(nor->write_proto) == 4);
	if (enable_quad_io && params->quad_enable)
		nor->quad_enable = params->quad_enable;
	else
		nor->quad_enable = NULL;

	return 0;
}

static int spi_nor_setup(struct spi_nor *nor, const struct flash_info *info,
			 const struct spi_nor_flash_parameter *params)
{
	if (!nor->setup)
		return 0;

	return nor->setup(nor, info, params);
}

#ifdef CONFIG_SPI_FLASH_SPANSION

/* Use ID byte 4 to distinguish S25FS256T and S25Hx-T */
#define S25FS256T_ID4	(0x08)

/* Number of dummy cycle for Read Any Register (RDAR) op. */
#define S25FS_S_RDAR_DUMMY	8

static int s25fs_s_quad_enable(struct spi_nor *nor)
{
	return spansion_quad_enable_volatile(nor, 0, S25FS_S_RDAR_DUMMY);
}

static int s25fs_s_erase_non_uniform(struct spi_nor *nor, loff_t addr)
{
	u8 opcode = nor->addr_width == 4 ? SPINOR_OP_BE_4K_4B : SPINOR_OP_BE_4K;

	/* Support 8 x 4KB sectors at bottom */
	return spansion_erase_non_uniform(nor, addr, opcode, 0, SZ_32K);
}

static int s25fs_s_setup(struct spi_nor *nor, const struct flash_info *info,
			 const struct spi_nor_flash_parameter *params)
{
	int ret;
	u8 cfr3v;

	/* Bank Address Register is not supported */
	if (CONFIG_IS_ENABLED(SPI_FLASH_BAR))
		return -EOPNOTSUPP;

	/*
	 * Read CR3V to check if uniform sector is selected. If not, assign an
	 * erase hook that supports non-uniform erase.
	 */
	ret = spansion_read_any_reg(nor, SPINOR_REG_CYPRESS_CFR3V,
				    S25FS_S_RDAR_DUMMY, &cfr3v);
	if (ret)
		return ret;
	if (!(cfr3v & SPINOR_REG_CYPRESS_CFR3_UNISECT))
		nor->erase = s25fs_s_erase_non_uniform;

	return spi_nor_default_setup(nor, info, params);
}

static void s25fs_s_late_init(struct spi_nor *nor,
			      struct spi_nor_flash_parameter *params)
{
	nor->setup = s25fs_s_setup;
}

static int s25fs_s_post_bfpt_fixup(struct spi_nor *nor,
				   const struct sfdp_parameter_header *header,
				   const struct sfdp_bfpt *bfpt,
				   struct spi_nor_flash_parameter *params)
{
	/* The erase size is set to 4K from BFPT, but it's wrong. Fix it. */
	nor->erase_opcode = SPINOR_OP_SE;
	nor->mtd.erasesize = nor->info->sector_size;

	/* The S25FS-S chip family reports 512-byte pages in BFPT but
	 * in reality the write buffer still wraps at the safe default
	 * of 256 bytes.  Overwrite the page size advertised by BFPT
	 * to get the writes working.
	 */
	params->page_size = 256;

	return 0;
}

static void s25fs_s_post_sfdp_fixup(struct spi_nor *nor,
				    struct spi_nor_flash_parameter *params)
{
	/*
	 * The S25FS064S(8MB) supports 1-1-2 and 1-1-4 commands, but params for
	 * read ops in SFDP are wrong. The other density parts do not support
	 * 1-1-2 and 1-1-4 commands.
	 */
	if (params->size == SZ_8M) {
		spi_nor_set_read_settings(&params->reads[SNOR_CMD_READ_1_1_2],
					  0, 8, SPINOR_OP_READ_1_1_2,
					  SNOR_PROTO_1_1_2);
		spi_nor_set_read_settings(&params->reads[SNOR_CMD_READ_1_1_4],
					  0, 8, SPINOR_OP_READ_1_1_4,
					  SNOR_PROTO_1_1_4);
	} else {
		params->hwcaps.mask &= ~SNOR_HWCAPS_READ_1_1_2;
		params->hwcaps.mask &= ~SNOR_HWCAPS_READ_1_1_4;
		params->hwcaps.mask &= ~SNOR_HWCAPS_PP_1_1_4;
	}

	/* Use volatile register to enable quad */
	params->quad_enable = s25fs_s_quad_enable;
}

static struct spi_nor_fixups s25fs_s_fixups = {
	.post_bfpt = s25fs_s_post_bfpt_fixup,
	.post_sfdp = s25fs_s_post_sfdp_fixup,
	.late_init = s25fs_s_late_init,
};

static int s25_s28_mdp_ready(struct spi_nor *nor)
{
	u32 addr;
	int ret;

	for (addr = 0; addr < nor->mtd.size; addr += SZ_128M) {
		ret = spansion_sr_ready(nor, addr, nor->rdsr_dummy);
		if (!ret)
			return ret;
	}

	return 1;
}

static int s25_quad_enable(struct spi_nor *nor)
{
	u32 addr;
	int ret;

	for (addr = 0; addr < nor->mtd.size; addr += SZ_128M) {
		ret = spansion_quad_enable_volatile(nor, addr, 0);
		if (ret)
			return ret;
	}

	return 0;
}

static int s25_s28_erase_non_uniform(struct spi_nor *nor, loff_t addr)
{
	/* Support 32 x 4KB sectors at bottom */
	return spansion_erase_non_uniform(nor, addr, SPINOR_OP_BE_4K_4B, 0,
					  SZ_128K);
}

static int s25_s28_setup(struct spi_nor *nor, const struct flash_info *info,
			 const struct spi_nor_flash_parameter *params)
{
	int ret;
	u8 cr;

#ifdef CONFIG_SPI_FLASH_BAR
	return -ENOTSUPP; /* Bank Address Register is not supported */
#endif
	/*
	 * S25FS256T has multiple sector architecture options, with selection of
	 * count and location of 128KB and 64KB sectors. This driver supports
	 * uniform 128KB only due to complexity of non-uniform layout.
	 */
	if (nor->info->id[4] == S25FS256T_ID4) {
		ret = spansion_read_any_reg(nor, SPINOR_REG_CYPRESS_ARCFN, 8,
					    &cr);
		if (ret)
			return ret;

		if (cr) /* Option 0 (ARCFN[7:0] == 0x00) is uniform */
			return -EOPNOTSUPP;

		return spi_nor_default_setup(nor, info, params);
	}

	/*
	 * Read CFR3V to check if uniform sector is selected. If not, assign an
	 * erase hook that supports non-uniform erase.
	 */
	ret = spansion_read_any_reg(nor, SPINOR_REG_CYPRESS_CFR3V, 0, &cr);
	if (ret)
		return ret;
	if (!(cr & SPINOR_REG_CYPRESS_CFR3_UNISECT))
		nor->erase = s25_s28_erase_non_uniform;

	/*
	 * For the multi-die package parts, the ready() hook is needed to check
	 * all dies' status via read any register.
	 */
	if (nor->mtd.size > SZ_128M)
		nor->ready = s25_s28_mdp_ready;

	return spi_nor_default_setup(nor, info, params);
}

static void s25_late_init(struct spi_nor *nor,
			  struct spi_nor_flash_parameter *params)
{
	nor->setup = s25_s28_setup;

	/*
	 * Programming is supported only in 16-byte ECC data unit granularity.
	 * Byte-programming, bit-walking, or multiple program operations to the
	 * same ECC data unit without an erase are not allowed.
	 */
	params->writesize = 16;
}

static int s25_s28_post_bfpt_fixup(struct spi_nor *nor,
				   const struct sfdp_parameter_header *header,
				   const struct sfdp_bfpt *bfpt,
				   struct spi_nor_flash_parameter *params)
{
	int ret;
	u32 addr;
	u8 cfr3v;

	/* erase size in case it is set to 4K from BFPT */
	nor->erase_opcode = SPINOR_OP_SE_4B;
	nor->mtd.erasesize = nor->info->sector_size;

	/*
	 * The default address mode in multi-die package parts (>1Gb) may be
	 * 3- or 4-byte, depending on model number. BootROM code in some SoCs
	 * use 3-byte mode for backward compatibility and should switch to
	 * 4-byte mode after BootROM phase. Since registers in the 2nd die are
	 * mapped within 32-bit address space, we need to make sure the flash is
	 * in 4-byte address mode. The default address mode can be distinguished
	 * by BFPT 16th DWORD. Power cycle exits 4-byte address mode if default
	 * is 3-byte address mode.
	 */
	if (params->size > SZ_128M) {
		if (bfpt->dwords[BFPT_DWORD(16)] & BFPT_DWORD16_EX4B_PWRCYC) {
			ret = set_4byte(nor, nor->info, 1);
			if (ret)
				return ret;
		}
		nor->addr_mode_nbytes = 4;
	}

	/* The default address mode in S25FS256T is 4. */
	if (nor->info->id[4] == S25FS256T_ID4)
		nor->addr_mode_nbytes = 4;

	/*
	 * The page_size is set to 512B from BFPT, but it actually depends on
	 * the configuration register. Look up the CFR3V and determine the
	 * page_size. For multi-die package parts, use 512B only when the all
	 * dies are configured to 512B buffer.
	 */
	for (addr = 0; addr < params->size; addr += SZ_128M) {
		ret = spansion_read_any_reg(nor,
					    addr + SPINOR_REG_CYPRESS_CFR3V, 0,
					    &cfr3v);
		if (ret)
			return ret;

		if (!(cfr3v & SPINOR_REG_CYPRESS_CFR3_PGSZ)) {
			params->page_size = 256;
			return 0;
		}
	}
	params->page_size = 512;

	return 0;
}

static void s25_post_sfdp_fixup(struct spi_nor *nor,
				struct spi_nor_flash_parameter *params)
{
	if (nor->info->id[4] == S25FS256T_ID4) {
		/* PP_1_1_4 is supported */
		params->hwcaps.mask |= SNOR_HWCAPS_PP_1_1_4;
	} else {
		/* READ_FAST_4B (0Ch) requires mode cycles*/
		params->reads[SNOR_CMD_READ_FAST].num_mode_clocks = 8;
		/* PP_1_1_4 is not supported */
		params->hwcaps.mask &= ~SNOR_HWCAPS_PP_1_1_4;
		/* Use volatile register to enable quad */
		params->quad_enable = s25_quad_enable;
	}
}

static struct spi_nor_fixups s25_fixups = {
	.post_bfpt = s25_s28_post_bfpt_fixup,
	.post_sfdp = s25_post_sfdp_fixup,
	.late_init = s25_late_init,
};

static int s25fl256l_setup(struct spi_nor *nor, const struct flash_info *info,
			   const struct spi_nor_flash_parameter *params)
{
	return -ENOTSUPP; /* Bank Address Register is not supported */
}

static void s25fl256l_late_init(struct spi_nor *nor,
				struct spi_nor_flash_parameter *params)
{
	nor->setup = s25fl256l_setup;
}

static struct spi_nor_fixups s25fl256l_fixups = {
	.late_init = s25fl256l_late_init,
};
#endif

#ifdef CONFIG_SPI_FLASH_S28HX_T
/**
 * spi_nor_cypress_octal_dtr_enable() - Enable octal DTR on Cypress flashes.
 * @nor:		pointer to a 'struct spi_nor'
 *
 * This also sets the memory access latency cycles to 24 to allow the flash to
 * run at up to 200MHz.
 *
 * Return: 0 on success, -errno otherwise.
 */
static int spi_nor_cypress_octal_dtr_enable(struct spi_nor *nor)
{
	u32 addr;
	u8 buf;
	int ret;

	ret = write_enable(nor);
	if (ret)
		return ret;

	/* Use 24 dummy cycles for memory array reads. */
	for (addr = 0; addr < nor->mtd.size; addr += SZ_128M) {
		ret = spansion_read_any_reg(nor,
					    addr + SPINOR_REG_CYPRESS_CFR2V, 0,
					    &buf);
		if (ret)
			return ret;

		buf &= ~SPINOR_REG_CYPRESS_CFR2_MEMLAT_MASK;
		buf |= SPINOR_REG_CYPRESS_CFR2_MEMLAT_11_24;
		ret = spansion_write_any_reg(nor,
					     addr + SPINOR_REG_CYPRESS_CFR2V,
					     buf);
		if (ret) {
			dev_warn(nor->dev, "failed to set default memory latency value: %d\n", ret);
			return ret;
		}
	}
	nor->read_dummy = 24;

	ret = write_enable(nor);
	if (ret)
		return ret;

	/* Set the octal and DTR enable bits. */
	buf = SPINOR_REG_CYPRESS_CFR5_OCT_DTR_EN;
	for (addr = 0; addr < nor->mtd.size; addr += SZ_128M) {
		ret = spansion_write_any_reg(nor,
					     addr + SPINOR_REG_CYPRESS_CFR5V,
					     buf);
		if (ret) {
			dev_warn(nor->dev, "Failed to enable octal DTR mode\n");
			return ret;
		}
	}

	return 0;
}

static void s28hx_t_late_init(struct spi_nor *nor,
			      struct spi_nor_flash_parameter *params)
{
	nor->octal_dtr_enable = spi_nor_cypress_octal_dtr_enable;
	nor->setup = s25_s28_setup;

	/*
	 * Programming is supported only in 16-byte ECC data unit granularity.
	 * Byte-programming, bit-walking, or multiple program operations to the
	 * same ECC data unit without an erase are not allowed.
	 */
	params->writesize = 16;
}

static void s28hx_t_post_sfdp_fixup(struct spi_nor *nor,
				    struct spi_nor_flash_parameter *params)
{
	/*
	 * On older versions of the flash the xSPI Profile 1.0 table has the
	 * 8D-8D-8D Fast Read opcode as 0x00. But it actually should be 0xEE.
	 */
	if (params->reads[SNOR_CMD_READ_8_8_8_DTR].opcode == 0)
		params->reads[SNOR_CMD_READ_8_8_8_DTR].opcode =
			SPINOR_OP_CYPRESS_RD_FAST;

	params->hwcaps.mask |= SNOR_HWCAPS_PP_8_8_8_DTR;

	/* This flash is also missing the 4-byte Page Program opcode bit. */
	spi_nor_set_pp_settings(&params->page_programs[SNOR_CMD_PP],
				SPINOR_OP_PP_4B, SNOR_PROTO_1_1_1);
	/*
	 * Since xSPI Page Program opcode is backward compatible with
	 * Legacy SPI, use Legacy SPI opcode there as well.
	 */
	spi_nor_set_pp_settings(&params->page_programs[SNOR_CMD_PP_8_8_8_DTR],
				SPINOR_OP_PP_4B, SNOR_PROTO_8_8_8_DTR);

	/*
	 * The xSPI Profile 1.0 table advertises the number of additional
	 * address bytes needed for Read Status Register command as 0 but the
	 * actual value for that is 4.
	 */
	params->rdsr_addr_nbytes = 4;
}

static struct spi_nor_fixups s28hx_t_fixups = {
	.post_sfdp = s28hx_t_post_sfdp_fixup,
	.post_bfpt = s25_s28_post_bfpt_fixup,
	.late_init = s28hx_t_late_init,
};
#endif /* CONFIG_SPI_FLASH_S28HX_T */

#ifdef CONFIG_SPI_FLASH_MT35XU
static int spi_nor_micron_octal_dtr_enable(struct spi_nor *nor)
{
	struct spi_mem_op op;
	u8 buf;
	u8 addr_width = 3;
	int ret;

	/* Set dummy cycles for Fast Read to the default of 20. */
	ret = write_enable(nor);
	if (ret)
		return ret;

	buf = 20;
	op = (struct spi_mem_op)
		SPI_MEM_OP(SPI_MEM_OP_CMD(SPINOR_OP_MT_WR_ANY_REG, 1),
			   SPI_MEM_OP_ADDR(addr_width, SPINOR_REG_MT_CFR1V, 1),
			   SPI_MEM_OP_NO_DUMMY,
			   SPI_MEM_OP_DATA_OUT(1, &buf, 1));
	ret = spi_mem_exec_op(nor->spi, &op);
	if (ret)
		return ret;

	ret = spi_nor_wait_till_ready(nor);
	if (ret)
		return ret;

	nor->read_dummy = 20;

	ret = write_enable(nor);
	if (ret)
		return ret;

	buf = SPINOR_MT_OCT_DTR;
	op = (struct spi_mem_op)
		SPI_MEM_OP(SPI_MEM_OP_CMD(SPINOR_OP_MT_WR_ANY_REG, 1),
			   SPI_MEM_OP_ADDR(addr_width, SPINOR_REG_MT_CFR0V, 1),
			   SPI_MEM_OP_NO_DUMMY,
			   SPI_MEM_OP_DATA_OUT(1, &buf, 1));
	ret = spi_mem_exec_op(nor->spi, &op);
	if (ret) {
		dev_err(nor->dev, "Failed to enable octal DTR mode\n");
		return ret;
	}

	return 0;
}

static void mt35xu512aba_late_init(struct spi_nor *nor,
				   struct spi_nor_flash_parameter *params)
{
	nor->octal_dtr_enable = spi_nor_micron_octal_dtr_enable;
}

static void mt35xu512aba_post_sfdp_fixup(struct spi_nor *nor,
					 struct spi_nor_flash_parameter *params)
{
	/* Set the Fast Read settings. */
	params->hwcaps.mask |= SNOR_HWCAPS_READ_8_8_8_DTR;
	spi_nor_set_read_settings(&params->reads[SNOR_CMD_READ_8_8_8_DTR],
				  0, 20, SPINOR_OP_MT_DTR_RD,
				  SNOR_PROTO_8_8_8_DTR);

	params->hwcaps.mask |= SNOR_HWCAPS_PP_8_8_8_DTR;

	nor->cmd_ext_type = SPI_NOR_EXT_REPEAT;
	params->rdsr_dummy = 8;
	params->rdsr_addr_nbytes = 0;

	/*
	 * The BFPT quad enable field is set to a reserved value so the quad
	 * enable function is ignored by spi_nor_parse_bfpt(). Make sure we
	 * disable it.
	 */
	params->quad_enable = NULL;
}

static struct spi_nor_fixups mt35xu512aba_fixups = {
	.post_sfdp = mt35xu512aba_post_sfdp_fixup,
	.late_init = mt35xu512aba_late_init,
};
#endif /* CONFIG_SPI_FLASH_MT35XU */

#if CONFIG_IS_ENABLED(SPI_FLASH_MACRONIX)
/**
 * spi_nor_macronix_octal_dtr_enable() - Enable octal DTR on Macronix flashes.
 * @nor:	pointer to a 'struct spi_nor'
 *
 * Set Macronix max dummy cycles 20 to allow the flash to run at fastest frequency.
 *
 * Return: 0 on success, -errno otherwise.
 */
static int spi_nor_macronix_octal_dtr_enable(struct spi_nor *nor)
{
	struct spi_mem_op op;
	int ret;
	u8 buf;

	ret = write_enable(nor);
	if (ret)
		return ret;

	buf = SPINOR_REG_MXIC_DC_20;
	op = (struct spi_mem_op)
		SPI_MEM_OP(SPI_MEM_OP_CMD(SPINOR_OP_WR_CR2, 1),
			   SPI_MEM_OP_ADDR(4, SPINOR_REG_MXIC_CR2_DC, 1),
			   SPI_MEM_OP_NO_DUMMY,
			   SPI_MEM_OP_DATA_OUT(1, &buf, 1));

	ret = spi_mem_exec_op(nor->spi, &op);
	if (ret)
		return ret;

	ret = spi_nor_wait_till_ready(nor);
	if (ret)
		return ret;

	nor->read_dummy = MXIC_MAX_DC;
	ret = write_enable(nor);
	if (ret)
		return ret;

	buf = SPINOR_REG_MXIC_OPI_DTR_EN;
	op = (struct spi_mem_op)
		SPI_MEM_OP(SPI_MEM_OP_CMD(SPINOR_OP_WR_CR2, 1),
			   SPI_MEM_OP_ADDR(4, SPINOR_REG_MXIC_CR2_MODE, 1),
			   SPI_MEM_OP_NO_DUMMY,
			   SPI_MEM_OP_DATA_OUT(1, &buf, 1));

	ret = spi_mem_exec_op(nor->spi, &op);
	if (ret) {
		dev_err(nor->dev, "Failed to enable octal DTR mode\n");
		return ret;
	}
	nor->reg_proto = SNOR_PROTO_8_8_8_DTR;

	return 0;
}

static void macronix_octal_late_init(struct spi_nor *nor,
				     struct spi_nor_flash_parameter *params)
{
	nor->octal_dtr_enable = spi_nor_macronix_octal_dtr_enable;
}

static void macronix_octal_post_sfdp_fixup(struct spi_nor *nor,
					 struct spi_nor_flash_parameter *params)
{
	/*
	 * Adding SNOR_HWCAPS_PP_8_8_8_DTR in hwcaps.mask when
	 * SPI_NOR_OCTAL_DTR_READ flag exists.
	 */
	if (params->hwcaps.mask & SNOR_HWCAPS_READ_8_8_8_DTR)
		params->hwcaps.mask |= SNOR_HWCAPS_PP_8_8_8_DTR;
}

static struct spi_nor_fixups macronix_octal_fixups = {
	.post_sfdp = macronix_octal_post_sfdp_fixup,
	.late_init = macronix_octal_late_init,
};
#endif /* CONFIG_SPI_FLASH_MACRONIX */

/** spi_nor_octal_dtr_enable() - enable Octal DTR I/O if needed
 * @nor:                 pointer to a 'struct spi_nor'
 *
 * Return: 0 on success, -errno otherwise.
 */
static int spi_nor_octal_dtr_enable(struct spi_nor *nor)
{
	int ret;

	if (!nor->octal_dtr_enable)
		return 0;

	if (!(nor->read_proto == SNOR_PROTO_8_8_8_DTR &&
	      nor->write_proto == SNOR_PROTO_8_8_8_DTR))
		return 0;

	if (!(nor->flags & SNOR_F_IO_MODE_EN_VOLATILE))
		return 0;

	ret = nor->octal_dtr_enable(nor);
	if (ret)
		return ret;

	nor->reg_proto = SNOR_PROTO_8_8_8_DTR;

	return 0;
}

static int spi_nor_init(struct spi_nor *nor)
{
	int err;

	if (nor->flags & SNOR_F_HAS_PARALLEL)
		nor->spi->flags |= SPI_NOR_ENABLE_MULTI_CS;

	err = spi_nor_octal_dtr_enable(nor);
	if (err) {
		dev_dbg(nor->dev, "Octal DTR mode not supported\n");
		return err;
	}

	/*
	 * Atmel, SST, Intel/Numonyx, and others serial NOR tend to power up
	 * with the software protection bits set
	 */
	if (IS_ENABLED(CONFIG_SPI_FLASH_UNLOCK_ALL) &&
	    (JEDEC_MFR(nor->info) == SNOR_MFR_ATMEL ||
	     JEDEC_MFR(nor->info) == SNOR_MFR_INTEL ||
	     JEDEC_MFR(nor->info) == SNOR_MFR_SST ||
	     nor->info->flags & SPI_NOR_HAS_LOCK)) {
		write_enable(nor);
		write_sr(nor, 0);
		spi_nor_wait_till_ready(nor);

		/*
		 * Some Winbond SPI NORs have special SR3 register which is
		 * used among other things to control whether non-standard
		 * "Individual Block/Sector Write Protection" (WPS bit)
		 * locking scheme is activated. This non-standard locking
		 * scheme is not supported by either U-Boot or Linux SPI
		 * NOR stack so make sure it is disabled, otherwise the
		 * SPI NOR may appear locked for no obvious reason.
		 */
		if (JEDEC_MFR(nor->info) == SNOR_MFR_WINBOND) {
			err = read_sr3(nor);
			if (err > 0 && err & SR3_WPS) {
				write_enable(nor);
				write_sr3(nor, err & ~SR3_WPS);
				write_disable(nor);
			}
		}
	}

	if (nor->quad_enable) {
		err = nor->quad_enable(nor);
		if (err) {
			dev_dbg(nor->dev, "quad mode not supported\n");
			return err;
		}
	}

	if (nor->addr_width == 4 &&
	    !(nor->info->flags & SPI_NOR_OCTAL_DTR_READ) &&
	    (JEDEC_MFR(nor->info) != SNOR_MFR_SPANSION) &&
	    !(nor->info->flags & SPI_NOR_4B_OPCODES)) {
		/*
		 * If the RESET# pin isn't hooked up properly, or the system
		 * otherwise doesn't perform a reset command in the boot
		 * sequence, it's impossible to 100% protect against unexpected
		 * reboots (e.g., crashes). Warn the user (or hopefully, system
		 * designer) that this is bad.
		 */
		if (nor->flags & SNOR_F_BROKEN_RESET)
			debug("enabling reset hack; may not recover from unexpected reboots\n");
		set_4byte(nor, nor->info, 1);
	}

	return 0;
}

#ifdef CONFIG_SPI_FLASH_SOFT_RESET
/**
 * spi_nor_soft_reset() - perform the JEDEC Software Reset sequence
 * @nor:	the spi_nor structure
 *
 * This function can be used to switch from Octal DTR mode to legacy mode on a
 * flash that supports it. The soft reset is executed in Octal DTR mode.
 *
 * Return: 0 for success, -errno for failure.
 */
static int spi_nor_soft_reset(struct spi_nor *nor)
{
	struct spi_mem_op op;
	int ret;
	enum spi_nor_cmd_ext ext;

	ext = nor->cmd_ext_type;
	if (nor->cmd_ext_type == SPI_NOR_EXT_NONE) {
		nor->cmd_ext_type = SPI_NOR_EXT_REPEAT;
#if CONFIG_IS_ENABLED(SPI_NOR_BOOT_SOFT_RESET_EXT_INVERT)
		nor->cmd_ext_type = SPI_NOR_EXT_INVERT;
#endif /* SPI_NOR_BOOT_SOFT_RESET_EXT_INVERT */
	}

	op = (struct spi_mem_op)SPI_MEM_OP(SPI_MEM_OP_CMD(SPINOR_OP_SRSTEN, 0),
			SPI_MEM_OP_NO_DUMMY,
			SPI_MEM_OP_NO_ADDR,
			SPI_MEM_OP_NO_DATA);
	spi_nor_setup_op(nor, &op, SNOR_PROTO_8_8_8_DTR);
	ret = spi_mem_exec_op(nor->spi, &op);
	if (ret) {
		dev_warn(nor->dev, "Software reset enable failed: %d\n", ret);
		goto out;
	}

	op = (struct spi_mem_op)SPI_MEM_OP(SPI_MEM_OP_CMD(SPINOR_OP_SRST, 0),
			SPI_MEM_OP_NO_DUMMY,
			SPI_MEM_OP_NO_ADDR,
			SPI_MEM_OP_NO_DATA);
	spi_nor_setup_op(nor, &op, SNOR_PROTO_8_8_8_DTR);
	ret = spi_mem_exec_op(nor->spi, &op);
	if (ret) {
		dev_warn(nor->dev, "Software reset failed: %d\n", ret);
		goto out;
	}

	/*
	 * Software Reset is not instant, and the delay varies from flash to
	 * flash. Looking at a few flashes, most range somewhere below 100
	 * microseconds. So, wait for 200ms just to be sure.
	 */
	udelay(SPI_NOR_SRST_SLEEP_LEN);

out:
	nor->cmd_ext_type = ext;
	return ret;
}
#endif /* CONFIG_SPI_FLASH_SOFT_RESET */

int spi_nor_remove(struct spi_nor *nor)
{
#ifdef CONFIG_SPI_FLASH_SOFT_RESET
	if (nor->info->flags & SPI_NOR_OCTAL_DTR_READ &&
	    nor->flags & SNOR_F_SOFT_RESET)
		return spi_nor_soft_reset(nor);
#endif

	return 0;
}

void spi_nor_set_fixups(struct spi_nor *nor)
{
#ifdef CONFIG_SPI_FLASH_SPANSION
	if (JEDEC_MFR(nor->info) == SNOR_MFR_CYPRESS) {
		switch (nor->info->id[1]) {
		case 0x2a: /* S25HL (QSPI, 3.3V) */
		case 0x2b: /* S25HS (QSPI, 1.8V) */
			nor->fixups = &s25_fixups;
			break;

#ifdef CONFIG_SPI_FLASH_S28HX_T
		case 0x5a: /* S28HL (Octal, 3.3V) */
		case 0x5b: /* S28HS (Octal, 1.8V) */
			nor->fixups = &s28hx_t_fixups;
			break;
#endif

		default:
			break;
		}
	}

	if (CONFIG_IS_ENABLED(SPI_FLASH_BAR) &&
	    !strcmp(nor->info->name, "s25fl256l"))
		nor->fixups = &s25fl256l_fixups;

	/* For FS-S (family ID = 0x81)  */
	if (JEDEC_MFR(nor->info) == SNOR_MFR_SPANSION && nor->info->id[5] == 0x81)
		nor->fixups = &s25fs_s_fixups;
#endif

#ifdef CONFIG_SPI_FLASH_MT35XU
	if (!strcmp(nor->info->name, "mt35xu512aba"))
		nor->fixups = &mt35xu512aba_fixups;
#endif

#if CONFIG_IS_ENABLED(SPI_FLASH_MACRONIX)
	if (JEDEC_MFR(nor->info) == SNOR_MFR_MACRONIX &&
	    nor->info->flags & SPI_NOR_OCTAL_DTR_READ)
		nor->fixups = &macronix_octal_fixups;
#endif /* SPI_FLASH_MACRONIX */
}

int spi_nor_scan(struct spi_nor *nor)
{
	struct spi_nor_flash_parameter params;
	const struct flash_info *info = NULL;
	struct mtd_info *mtd = &nor->mtd;
	struct spi_slave *spi = nor->spi;
	int ret;
	int cfi_mtd_nb = 0;
	bool shift = 0;

#ifdef CONFIG_FLASH_CFI_MTD
	cfi_mtd_nb = CFI_FLASH_BANKS;
#endif

	/* Reset SPI protocol for all commands. */
	nor->reg_proto = SNOR_PROTO_1_1_1;
	nor->read_proto = SNOR_PROTO_1_1_1;
	nor->write_proto = SNOR_PROTO_1_1_1;
	nor->read = spi_nor_read_data;
	nor->write = spi_nor_write_data;
	nor->read_reg = spi_nor_read_reg;
	nor->write_reg = spi_nor_write_reg;

	nor->setup = spi_nor_default_setup;

#ifdef CONFIG_SPI_FLASH_SOFT_RESET_ON_BOOT
	/*
	 * When the flash is handed to us in a stateful mode like 8D-8D-8D, it
	 * is difficult to detect the mode the flash is in. One option is to
	 * read SFDP in all modes and see which one gives the correct "SFDP"
	 * signature, but not all flashes support SFDP in 8D-8D-8D mode.
	 *
	 * Further, even if you detect the mode of the flash via SFDP, you
	 * still have the problem of actually reading the ID. The Read ID
	 * command is not standardized across flash vendors. Flashes can have
	 * different dummy cycles needed for reading the ID. Some flashes even
	 * expect a 4-byte dummy address with the Read ID command. All this
	 * information cannot be obtained from the SFDP table.
	 *
	 * So, perform a Software Reset sequence before reading the ID and
	 * initializing the flash. A Soft Reset will bring back the flash in
	 * its default protocol mode assuming no non-volatile configuration was
	 * set. This will let us detect the flash even if ROM hands it to us in
	 * Octal DTR mode.
	 *
	 * To accommodate cases where there is more than one flash on a board,
	 * and only one of them needs a soft reset, failure to reset is not
	 * made fatal, and we still try to read ID if possible.
	 */
	spi_nor_soft_reset(nor);
#endif /* CONFIG_SPI_FLASH_SOFT_RESET_ON_BOOT */

	info = spi_nor_read_id(nor);
	if (IS_ERR_OR_NULL(info))
		return -ENOENT;
	nor->info = info;

	spi_nor_set_fixups(nor);

	/* Parse the Serial Flash Discoverable Parameters table. */
	ret = spi_nor_init_params(nor, info, &params);
	if (ret)
		return ret;

	if (!mtd->name) {
		sprintf(nor->mtd_name, "%s%d",
			MTD_DEV_TYPE(MTD_DEV_TYPE_NOR),
			cfi_mtd_nb + dev_seq(nor->dev));
		mtd->name = nor->mtd_name;
	}
	mtd->dev = nor->dev;
	mtd->priv = nor;
	mtd->type = MTD_NORFLASH;
	mtd->writesize = params.writesize;
	mtd->flags = MTD_CAP_NORFLASH;
	mtd->size = params.size;
	mtd->_erase = spi_nor_erase;
	mtd->_read = spi_nor_read;
	mtd->_write = spi_nor_write;

#if defined(CONFIG_SPI_FLASH_LOCK)
#if defined(CONFIG_SPI_FLASH_STMICRO) || defined(CONFIG_SPI_FLASH_SST)
	/* NOR protection support for STmicro/Micron chips and similar */
	if (JEDEC_MFR(info) == SNOR_MFR_ST ||
	    JEDEC_MFR(info) == SNOR_MFR_MICRON ||
	    JEDEC_MFR(info) == SNOR_MFR_SST ||
			info->flags & SPI_NOR_HAS_LOCK) {
		nor->flash_lock = stm_lock;
		nor->flash_unlock = stm_unlock;
		nor->flash_is_unlocked = stm_is_unlocked;
	}
#endif

#ifdef CONFIG_SPI_FLASH_SST
	/*
	 * sst26 series block protection implementation differs from other
	 * series.
	 */
	if (info->flags & SPI_NOR_HAS_SST26LOCK) {
		nor->flash_lock = sst26_lock;
		nor->flash_unlock = sst26_unlock;
		nor->flash_is_unlocked = sst26_is_unlocked;
	}
#endif
#endif
	if (info->flags & USE_FSR)
		nor->flags |= SNOR_F_USE_FSR;
	if (info->flags & SPI_NOR_HAS_TB)
		nor->flags |= SNOR_F_HAS_SR_TB;
	if (info->flags & NO_CHIP_ERASE)
		nor->flags |= SNOR_F_NO_OP_CHIP_ERASE;
	if (info->flags & USE_CLSR)
		nor->flags |= SNOR_F_USE_CLSR;

	if (info->flags & SPI_NOR_NO_ERASE)
		mtd->flags |= MTD_NO_ERASE;

	nor->page_size = params.page_size;
	mtd->writebufsize = nor->page_size;

	/* Some devices cannot do fast-read, no matter what DT tells us */
	if ((info->flags & SPI_NOR_NO_FR) || (spi->mode & SPI_RX_SLOW))
		params.hwcaps.mask &= ~SNOR_HWCAPS_READ_FAST;

	/*
	 * Configure the SPI memory:
	 * - select op codes for (Fast) Read, Page Program and Sector Erase.
	 * - set the number of dummy cycles (mode cycles + wait states).
	 * - set the SPI protocols for register and memory accesses.
	 * - set the Quad Enable bit if needed (required by SPI x-y-4 protos).
	 */
	ret = spi_nor_setup(nor, info, &params);
	if (ret)
		return ret;

	if (spi_nor_protocol_is_dtr(nor->read_proto)) {
		 /* Always use 4-byte addresses in DTR mode. */
		nor->addr_width = 4;
	} else if (nor->addr_width) {
		/* already configured from SFDP */
	} else if (info->addr_width) {
		nor->addr_width = info->addr_width;
	} else {
		nor->addr_width = 3;
	}

	if (nor->flags & (SNOR_F_HAS_PARALLEL | SNOR_F_HAS_STACKED))
		shift = 1;
	if (nor->addr_width == 3 && (mtd->size >> shift) > SZ_16M) {
#ifndef CONFIG_SPI_FLASH_BAR
		/* enable 4-byte addressing if the device exceeds 16MiB */
		nor->addr_width = 4;
		if (JEDEC_MFR(info) == SNOR_MFR_SPANSION ||
		    info->flags & SPI_NOR_4B_OPCODES)
			spi_nor_set_4byte_opcodes(nor, info);
#else
	/* Configure the BAR - discover bank cmds and read current bank */
	nor->addr_width = 3;
	ret = read_bar(nor, info);
	if (ret < 0)
		return ret;
#endif
	}

	if (nor->addr_width > SPI_NOR_MAX_ADDR_WIDTH) {
		dev_dbg(nor->dev, "address width is too large: %u\n",
			nor->addr_width);
		return -EINVAL;
	}

	/* Send all the required SPI flash commands to initialize device */
	ret = spi_nor_init(nor);
	if (ret)
		return ret;

	if (nor->flags & SNOR_F_HAS_STACKED) {
		nor->spi->flags |= SPI_XFER_U_PAGE;
		ret = spi_nor_init(nor);
		if (ret)
			return ret;
		nor->spi->flags &= ~SPI_XFER_U_PAGE;
	}

	nor->rdsr_dummy = params.rdsr_dummy;
	nor->rdsr_addr_nbytes = params.rdsr_addr_nbytes;
	nor->name = info->name;
	nor->size = mtd->size;
	nor->erase_size = mtd->erasesize;
	nor->sector_size = mtd->erasesize;

#ifndef CONFIG_XPL_BUILD
	printf("SF: Detected %s with page size ", nor->name);
	print_size(nor->page_size, ", erase size ");
	print_size(nor->erase_size, ", total ");
	print_size(nor->size, "");
	puts("\n");
#endif

	return 0;
}

/* U-Boot specific functions, need to extend MTD to support these */
int spi_flash_cmd_get_sw_write_prot(struct spi_nor *nor)
{
	int sr = read_sr(nor);

	if (sr < 0)
		return sr;

	return (sr >> 2) & 7;
}
