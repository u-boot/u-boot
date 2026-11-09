// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2005, Intec Automation Inc.
 * Copyright (C) 2014, Freescale Semiconductor, Inc.
 */

#include <stddef.h>
#include <string.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <linux/errno.h>
#include <linux/sizes.h>
#include <linux/bitfield.h>
#include <linux/mtd/spi-nor.h>
#include <spi.h>

#include "sf_internal.h"

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
int spi_nor_parse_sfdp(struct spi_nor *nor,
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
