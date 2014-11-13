/*
 * Copyright (C) 2014       Panasonic Corporation
 * Copyright (C) 2013-2014, Altera Corporation <www.altera.com>
 * Copyright (C) 2009-2010, Intel Corporation and its suppliers.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <nand.h>
#include <asm/errno.h>
#include <asm/io.h>

#include "denali.h"

#define NAND_DEFAULT_TIMINGS	-1

static int onfi_timing_mode = NAND_DEFAULT_TIMINGS;

/* We define a macro here that combines all interrupts this driver uses into
 * a single constant value, for convenience. */
#define DENALI_IRQ_ALL	(INTR_STATUS__DMA_CMD_COMP | \
			INTR_STATUS__ECC_TRANSACTION_DONE | \
			INTR_STATUS__ECC_ERR | \
			INTR_STATUS__PROGRAM_FAIL | \
			INTR_STATUS__LOAD_COMP | \
			INTR_STATUS__PROGRAM_COMP | \
			INTR_STATUS__TIME_OUT | \
			INTR_STATUS__ERASE_FAIL | \
			INTR_STATUS__RST_COMP | \
			INTR_STATUS__ERASE_COMP | \
			INTR_STATUS__ECC_UNCOR_ERR | \
			INTR_STATUS__INT_ACT | \
			INTR_STATUS__LOCKED_BLK)

/* indicates whether or not the internal value for the flash bank is
 * valid or not */
#define CHIP_SELECT_INVALID	-1

#define SUPPORT_8BITECC		1

/*
 * this macro allows us to convert from an MTD structure to our own
 * device context (denali) structure.
 */
#define mtd_to_denali(m) container_of(m->priv, struct denali_nand_info, nand)

/* These constants are defined by the driver to enable common driver
 * configuration options. */
#define SPARE_ACCESS		0x41
#define MAIN_ACCESS		0x42
#define MAIN_SPARE_ACCESS	0x43

#define DENALI_UNLOCK_START	0x10
#define DENALI_UNLOCK_END	0x11
#define DENALI_LOCK		0x21
#define DENALI_LOCK_TIGHT	0x31
#define DENALI_BUFFER_LOAD	0x60
#define DENALI_BUFFER_WRITE	0x62

#define DENALI_READ	0
#define DENALI_WRITE	0x100

/* types of device accesses. We can issue commands and get status */
#define COMMAND_CYCLE	0
#define ADDR_CYCLE	1
#define STATUS_CYCLE	2

/* this is a helper macro that allows us to
 * format the bank into the proper bits for the controller */
#define BANK(x) ((x) << 24)

/* Interrupts are cleared by writing a 1 to the appropriate status bit */
static inline void clear_interrupt(struct denali_nand_info *denali,
							uint32_t irq_mask)
{
	uint32_t intr_status_reg;

	intr_status_reg = INTR_STATUS(denali->flash_bank);

	writel(irq_mask, denali->flash_reg + intr_status_reg);
}

static uint32_t read_interrupt_status(struct denali_nand_info *denali)
{
	uint32_t intr_status_reg;

	intr_status_reg = INTR_STATUS(denali->flash_bank);

	return readl(denali->flash_reg + intr_status_reg);
}

static void clear_interrupts(struct denali_nand_info *denali)
{
	uint32_t status;

	status = read_interrupt_status(denali);
	clear_interrupt(denali, status);

	denali->irq_status = 0;
}

static void denali_irq_enable(struct denali_nand_info *denali,
							uint32_t int_mask)
{
	int i;

	for (i = 0; i < denali->max_banks; ++i)
		writel(int_mask, denali->flash_reg + INTR_EN(i));
}

static uint32_t wait_for_irq(struct denali_nand_info *denali, uint32_t irq_mask)
{
	unsigned long timeout = 1000000;
	uint32_t intr_status;

	do {
		intr_status = read_interrupt_status(denali) & DENALI_IRQ_ALL;
		if (intr_status & irq_mask) {
			denali->irq_status &= ~irq_mask;
			/* our interrupt was detected */
			break;
		}
		udelay(1);
		timeout--;
	} while (timeout != 0);

	if (timeout == 0) {
		/* timeout */
		printf("Denali timeout with interrupt status %08x\n",
		       read_interrupt_status(denali));
		intr_status = 0;
	}
	return intr_status;
}

/*
 * Certain operations for the denali NAND controller use an indexed mode to
 * read/write data. The operation is performed by writing the address value
 * of the command to the device memory followed by the data. This function
 * abstracts this common operation.
*/
static void index_addr(struct denali_nand_info *denali,
				uint32_t address, uint32_t data)
{
	writel(address, denali->flash_mem + INDEX_CTRL_REG);
	writel(data, denali->flash_mem + INDEX_DATA_REG);
}

/* Perform an indexed read of the device */
static void index_addr_read_data(struct denali_nand_info *denali,
				 uint32_t address, uint32_t *pdata)
{
	writel(address, denali->flash_mem + INDEX_CTRL_REG);
	*pdata = readl(denali->flash_mem + INDEX_DATA_REG);
}

/* We need to buffer some data for some of the NAND core routines.
 * The operations manage buffering that data. */
static void reset_buf(struct denali_nand_info *denali)
{
	denali->buf.head = 0;
	denali->buf.tail = 0;
}

static void write_byte_to_buf(struct denali_nand_info *denali, uint8_t byte)
{
	denali->buf.buf[denali->buf.tail++] = byte;
}

/* resets a specific device connected to the core */
static void reset_bank(struct denali_nand_info *denali)
{
	uint32_t irq_status;
	uint32_t irq_mask = INTR_STATUS__RST_COMP |
			    INTR_STATUS__TIME_OUT;

	clear_interrupts(denali);

	writel(1 << denali->flash_bank, denali->flash_reg + DEVICE_RESET);

	irq_status = wait_for_irq(denali, irq_mask);
	if (irq_status & INTR_STATUS__TIME_OUT)
		debug("reset bank failed.\n");
}

/* Reset the flash controller */
static uint32_t denali_nand_reset(struct denali_nand_info *denali)
{
	uint32_t i;

	for (i = 0; i < denali->max_banks; i++)
		writel(INTR_STATUS__RST_COMP | INTR_STATUS__TIME_OUT,
		       denali->flash_reg + INTR_STATUS(i));

	for (i = 0; i < denali->max_banks; i++) {
		writel(1 << i, denali->flash_reg + DEVICE_RESET);
		while (!(readl(denali->flash_reg + INTR_STATUS(i)) &
			(INTR_STATUS__RST_COMP | INTR_STATUS__TIME_OUT)))
			if (readl(denali->flash_reg + INTR_STATUS(i)) &
				INTR_STATUS__TIME_OUT)
				debug("NAND Reset operation timed out on bank"
				      " %d\n", i);
	}

	for (i = 0; i < denali->max_banks; i++)
		writel(INTR_STATUS__RST_COMP | INTR_STATUS__TIME_OUT,
		       denali->flash_reg + INTR_STATUS(i));

	return 0;
}

/*
 * this routine calculates the ONFI timing values for a given mode and
 * programs the clocking register accordingly. The mode is determined by
 * the get_onfi_nand_para routine.
 */
static void nand_onfi_timing_set(struct denali_nand_info *denali,
								uint16_t mode)
{
	uint32_t trea[6] = {40, 30, 25, 20, 20, 16};
	uint32_t trp[6] = {50, 25, 17, 15, 12, 10};
	uint32_t treh[6] = {30, 15, 15, 10, 10, 7};
	uint32_t trc[6] = {100, 50, 35, 30, 25, 20};
	uint32_t trhoh[6] = {0, 15, 15, 15, 15, 15};
	uint32_t trloh[6] = {0, 0, 0, 0, 5, 5};
	uint32_t tcea[6] = {100, 45, 30, 25, 25, 25};
	uint32_t tadl[6] = {200, 100, 100, 100, 70, 70};
	uint32_t trhw[6] = {200, 100, 100, 100, 100, 100};
	uint32_t trhz[6] = {200, 100, 100, 100, 100, 100};
	uint32_t twhr[6] = {120, 80, 80, 60, 60, 60};
	uint32_t tcs[6] = {70, 35, 25, 25, 20, 15};

	uint32_t tclsrising = 1;
	uint32_t data_invalid_rhoh, data_invalid_rloh, data_invalid;
	uint32_t dv_window = 0;
	uint32_t en_lo, en_hi;
	uint32_t acc_clks;
	uint32_t addr_2_data, re_2_we, re_2_re, we_2_re, cs_cnt;

	en_lo = DIV_ROUND_UP(trp[mode], CLK_X);
	en_hi = DIV_ROUND_UP(treh[mode], CLK_X);
	if ((en_hi * CLK_X) < (treh[mode] + 2))
		en_hi++;

	if ((en_lo + en_hi) * CLK_X < trc[mode])
		en_lo += DIV_ROUND_UP((trc[mode] - (en_lo + en_hi) * CLK_X),
				      CLK_X);

	if ((en_lo + en_hi) < CLK_MULTI)
		en_lo += CLK_MULTI - en_lo - en_hi;

	while (dv_window < 8) {
		data_invalid_rhoh = en_lo * CLK_X + trhoh[mode];

		data_invalid_rloh = (en_lo + en_hi) * CLK_X + trloh[mode];

		data_invalid =
		    data_invalid_rhoh <
		    data_invalid_rloh ? data_invalid_rhoh : data_invalid_rloh;

		dv_window = data_invalid - trea[mode];

		if (dv_window < 8)
			en_lo++;
	}

	acc_clks = DIV_ROUND_UP(trea[mode], CLK_X);

	while (((acc_clks * CLK_X) - trea[mode]) < 3)
		acc_clks++;

	if ((data_invalid - acc_clks * CLK_X) < 2)
		debug("%s, Line %d: Warning!\n", __FILE__, __LINE__);

	addr_2_data = DIV_ROUND_UP(tadl[mode], CLK_X);
	re_2_we = DIV_ROUND_UP(trhw[mode], CLK_X);
	re_2_re = DIV_ROUND_UP(trhz[mode], CLK_X);
	we_2_re = DIV_ROUND_UP(twhr[mode], CLK_X);
	cs_cnt = DIV_ROUND_UP((tcs[mode] - trp[mode]), CLK_X);
	if (!tclsrising)
		cs_cnt = DIV_ROUND_UP(tcs[mode], CLK_X);
	if (cs_cnt == 0)
		cs_cnt = 1;

	if (tcea[mode]) {
		while (((cs_cnt * CLK_X) + trea[mode]) < tcea[mode])
			cs_cnt++;
	}

	/* Sighting 3462430: Temporary hack for MT29F128G08CJABAWP:B */
	if ((readl(denali->flash_reg + MANUFACTURER_ID) == 0) &&
	    (readl(denali->flash_reg + DEVICE_ID) == 0x88))
		acc_clks = 6;

	writel(acc_clks, denali->flash_reg + ACC_CLKS);
	writel(re_2_we, denali->flash_reg + RE_2_WE);
	writel(re_2_re, denali->flash_reg + RE_2_RE);
	writel(we_2_re, denali->flash_reg + WE_2_RE);
	writel(addr_2_data, denali->flash_reg + ADDR_2_DATA);
	writel(en_lo, denali->flash_reg + RDWR_EN_LO_CNT);
	writel(en_hi, denali->flash_reg + RDWR_EN_HI_CNT);
	writel(cs_cnt, denali->flash_reg + CS_SETUP_CNT);
}

/* queries the NAND device to see what ONFI modes it supports. */
static uint32_t get_onfi_nand_para(struct denali_nand_info *denali)
{
	int i;
	/*
	 * we needn't to do a reset here because driver has already
	 * reset all the banks before
	 */
	if (!(readl(denali->flash_reg + ONFI_TIMING_MODE) &
	    ONFI_TIMING_MODE__VALUE))
		return -EIO;

	for (i = 5; i > 0; i--) {
		if (readl(denali->flash_reg + ONFI_TIMING_MODE) &
			(0x01 << i))
			break;
	}

	nand_onfi_timing_set(denali, i);

	/* By now, all the ONFI devices we know support the page cache */
	/* rw feature. So here we enable the pipeline_rw_ahead feature */
	return 0;
}

static void get_samsung_nand_para(struct denali_nand_info *denali,
							uint8_t device_id)
{
	if (device_id == 0xd3) { /* Samsung K9WAG08U1A */
		/* Set timing register values according to datasheet */
		writel(5, denali->flash_reg + ACC_CLKS);
		writel(20, denali->flash_reg + RE_2_WE);
		writel(12, denali->flash_reg + WE_2_RE);
		writel(14, denali->flash_reg + ADDR_2_DATA);
		writel(3, denali->flash_reg + RDWR_EN_LO_CNT);
		writel(2, denali->flash_reg + RDWR_EN_HI_CNT);
		writel(2, denali->flash_reg + CS_SETUP_CNT);
	}
}

static void get_toshiba_nand_para(struct denali_nand_info *denali)
{
	uint32_t tmp;

	/* Workaround to fix a controller bug which reports a wrong */
	/* spare area size for some kind of Toshiba NAND device */
	if ((readl(denali->flash_reg + DEVICE_MAIN_AREA_SIZE) == 4096) &&
	    (readl(denali->flash_reg + DEVICE_SPARE_AREA_SIZE) == 64)) {
		writel(216, denali->flash_reg + DEVICE_SPARE_AREA_SIZE);
		tmp = readl(denali->flash_reg + DEVICES_CONNECTED) *
			readl(denali->flash_reg + DEVICE_SPARE_AREA_SIZE);
		writel(tmp, denali->flash_reg + LOGICAL_PAGE_SPARE_SIZE);
	}
}

static void get_hynix_nand_para(struct denali_nand_info *denali,
							uint8_t device_id)
{
	uint32_t main_size, spare_size;

	switch (device_id) {
	case 0xD5: /* Hynix H27UAG8T2A, H27UBG8U5A or H27UCG8VFA */
	case 0xD7: /* Hynix H27UDG8VEM, H27UCG8UDM or H27UCG8V5A */
		writel(128, denali->flash_reg + PAGES_PER_BLOCK);
		writel(4096, denali->flash_reg + DEVICE_MAIN_AREA_SIZE);
		writel(224, denali->flash_reg + DEVICE_SPARE_AREA_SIZE);
		main_size = 4096 *
			readl(denali->flash_reg + DEVICES_CONNECTED);
		spare_size = 224 *
			readl(denali->flash_reg + DEVICES_CONNECTED);
		writel(main_size, denali->flash_reg + LOGICAL_PAGE_DATA_SIZE);
		writel(spare_size, denali->flash_reg + LOGICAL_PAGE_SPARE_SIZE);
		writel(0, denali->flash_reg + DEVICE_WIDTH);
		break;
	default:
		debug("Spectra: Unknown Hynix NAND (Device ID: 0x%x)."
		      "Will use default parameter values instead.\n",
		      device_id);
	}
}

/*
 * determines how many NAND chips are connected to the controller. Note for
 * Intel CE4100 devices we don't support more than one device.
 */
static void find_valid_banks(struct denali_nand_info *denali)
{
	uint32_t id[denali->max_banks];
	int i;

	denali->total_used_banks = 1;
	for (i = 0; i < denali->max_banks; i++) {
		index_addr(denali, (uint32_t)(MODE_11 | (i << 24) | 0), 0x90);
		index_addr(denali, (uint32_t)(MODE_11 | (i << 24) | 1), 0);
		index_addr_read_data(denali,
				     (uint32_t)(MODE_11 | (i << 24) | 2),
				     &id[i]);

		if (i == 0) {
			if (!(id[i] & 0x0ff))
				break;
		} else {
			if ((id[i] & 0x0ff) == (id[0] & 0x0ff))
				denali->total_used_banks++;
			else
				break;
		}
	}
}

/*
 * Use the configuration feature register to determine the maximum number of
 * banks that the hardware supports.
 */
static void detect_max_banks(struct denali_nand_info *denali)
{
	uint32_t features = readl(denali->flash_reg + FEATURES);
	denali->max_banks = 2 << (features & FEATURES__N_BANKS);
}

static void detect_partition_feature(struct denali_nand_info *denali)
{
	/*
	 * For MRST platform, denali->fwblks represent the
	 * number of blocks firmware is taken,
	 * FW is in protect partition and MTD driver has no
	 * permission to access it. So let driver know how many
	 * blocks it can't touch.
	 */
	if (readl(denali->flash_reg + FEATURES) & FEATURES__PARTITION) {
		if ((readl(denali->flash_reg + PERM_SRC_ID(1)) &
			PERM_SRC_ID__SRCID) == SPECTRA_PARTITION_ID) {
			denali->fwblks =
			    ((readl(denali->flash_reg + MIN_MAX_BANK(1)) &
			      MIN_MAX_BANK__MIN_VALUE) *
			     denali->blksperchip)
			    +
			    (readl(denali->flash_reg + MIN_BLK_ADDR(1)) &
			    MIN_BLK_ADDR__VALUE);
		} else {
			denali->fwblks = SPECTRA_START_BLOCK;
		}
	} else {
		denali->fwblks = SPECTRA_START_BLOCK;
	}
}

static uint32_t denali_nand_timing_set(struct denali_nand_info *denali)
{
	uint32_t id_bytes[5], addr;
	uint8_t i, maf_id, device_id;

	/* Use read id method to get device ID and other
	 * params. For some NAND chips, controller can't
	 * report the correct device ID by reading from
	 * DEVICE_ID register
	 * */
	addr = (uint32_t)MODE_11 | BANK(denali->flash_bank);
	index_addr(denali, (uint32_t)addr | 0, 0x90);
	index_addr(denali, (uint32_t)addr | 1, 0);
	for (i = 0; i < 5; i++)
		index_addr_read_data(denali, addr | 2, &id_bytes[i]);
	maf_id = id_bytes[0];
	device_id = id_bytes[1];

	if (readl(denali->flash_reg + ONFI_DEVICE_NO_OF_LUNS) &
		ONFI_DEVICE_NO_OF_LUNS__ONFI_DEVICE) { /* ONFI 1.0 NAND */
		if (get_onfi_nand_para(denali))
			return -EIO;
	} else if (maf_id == 0xEC) { /* Samsung NAND */
		get_samsung_nand_para(denali, device_id);
	} else if (maf_id == 0x98) { /* Toshiba NAND */
		get_toshiba_nand_para(denali);
	} else if (maf_id == 0xAD) { /* Hynix NAND */
		get_hynix_nand_para(denali, device_id);
	}

	find_valid_banks(denali);

	detect_partition_feature(denali);

	/* If the user specified to override the default timings
	 * with a specific ONFI mode, we apply those changes here.
	 */
	if (onfi_timing_mode != NAND_DEFAULT_TIMINGS)
		nand_onfi_timing_set(denali, onfi_timing_mode);

	return 0;
}

/* validation function to verify that the controlling software is making
 * a valid request
 */
static inline bool is_flash_bank_valid(int flash_bank)
{
	return flash_bank >= 0 && flash_bank < 4;
}

static void denali_irq_init(struct denali_nand_info *denali)
{
	uint32_t int_mask = 0;
	int i;

	/* Disable global interrupts */
	writel(0, denali->flash_reg + GLOBAL_INT_ENABLE);

	int_mask = DENALI_IRQ_ALL;

	/* Clear all status bits */
	for (i = 0; i < denali->max_banks; ++i)
		writel(0xFFFF, denali->flash_reg + INTR_STATUS(i));

	denali_irq_enable(denali, int_mask);
}

/* This helper function setups the registers for ECC and whether or not
 * the spare area will be transferred. */
static void setup_ecc_for_xfer(struct denali_nand_info *denali, bool ecc_en,
				bool transfer_spare)
{
	int ecc_en_flag = 0, transfer_spare_flag = 0;

	/* set ECC, transfer spare bits if needed */
	ecc_en_flag = ecc_en ? ECC_ENABLE__FLAG : 0;
	transfer_spare_flag = transfer_spare ? TRANSFER_SPARE_REG__FLAG : 0;

	/* Enable spare area/ECC per user's request. */
	writel(ecc_en_flag, denali->flash_reg + ECC_ENABLE);
	/* applicable for MAP01 only */
	writel(transfer_spare_flag, denali->flash_reg + TRANSFER_SPARE_REG);
}

/* sends a pipeline command operation to the controller. See the Denali NAND
 * controller's user guide for more information (section 4.2.3.6).
 */
static int denali_send_pipeline_cmd(struct denali_nand_info *denali,
					bool ecc_en, bool transfer_spare,
					int access_type, int op)
{
	uint32_t addr, cmd, irq_status;
	static uint32_t page_count = 1;

	setup_ecc_for_xfer(denali, ecc_en, transfer_spare);

	/* clear interrupts */
	clear_interrupts(denali);

	addr = BANK(denali->flash_bank) | denali->page;

	/* setup the acccess type */
	cmd = MODE_10 | addr;
	index_addr(denali, cmd, access_type);

	/* setup the pipeline command */
	index_addr(denali, cmd, 0x2000 | op | page_count);

	cmd = MODE_01 | addr;
	writel(cmd, denali->flash_mem + INDEX_CTRL_REG);

	if (op == DENALI_READ) {
		/* wait for command to be accepted */
		irq_status = wait_for_irq(denali, INTR_STATUS__LOAD_COMP);

		if (irq_status == 0)
			return -EIO;
	}

	return 0;
}

/* helper function that simply writes a buffer to the flash */
static int write_data_to_flash_mem(struct denali_nand_info *denali,
						const uint8_t *buf, int len)
{
	uint32_t i = 0, *buf32;

	/* verify that the len is a multiple of 4. see comment in
	 * read_data_from_flash_mem() */
	BUG_ON((len % 4) != 0);

	/* write the data to the flash memory */
	buf32 = (uint32_t *)buf;
	for (i = 0; i < len / 4; i++)
		writel(*buf32++, denali->flash_mem + INDEX_DATA_REG);
	return i * 4; /* intent is to return the number of bytes read */
}

/* helper function that simply reads a buffer from the flash */
static int read_data_from_flash_mem(struct denali_nand_info *denali,
						uint8_t *buf, int len)
{
	uint32_t i, *buf32;

	/*
	 * we assume that len will be a multiple of 4, if not
	 * it would be nice to know about it ASAP rather than
	 * have random failures...
	 * This assumption is based on the fact that this
	 * function is designed to be used to read flash pages,
	 * which are typically multiples of 4...
	 */

	BUG_ON((len % 4) != 0);

	/* transfer the data from the flash */
	buf32 = (uint32_t *)buf;
	for (i = 0; i < len / 4; i++)
		*buf32++ = readl(denali->flash_mem + INDEX_DATA_REG);

	return i * 4; /* intent is to return the number of bytes read */
}

static void denali_mode_main_access(struct denali_nand_info *denali)
{
	uint32_t addr, cmd;

	addr = BANK(denali->flash_bank) | denali->page;
	cmd = MODE_10 | addr;
	index_addr(denali, cmd, MAIN_ACCESS);
}

static void denali_mode_main_spare_access(struct denali_nand_info *denali)
{
	uint32_t addr, cmd;

	addr = BANK(denali->flash_bank) | denali->page;
	cmd = MODE_10 | addr;
	index_addr(denali, cmd, MAIN_SPARE_ACCESS);
}

/* writes OOB data to the device */
static int write_oob_data(struct mtd_info *mtd, uint8_t *buf, int page)
{
	struct denali_nand_info *denali = mtd_to_denali(mtd);
	uint32_t irq_status;
	uint32_t irq_mask = INTR_STATUS__PROGRAM_COMP |
						INTR_STATUS__PROGRAM_FAIL;
	int status = 0;

	denali->page = page;

	if (denali_send_pipeline_cmd(denali, false, true, SPARE_ACCESS,
				     DENALI_WRITE) == 0) {
		write_data_to_flash_mem(denali, buf, mtd->oobsize);

		/* wait for operation to complete */
		irq_status = wait_for_irq(denali, irq_mask);

		if (irq_status == 0) {
			dev_err(denali->dev, "OOB write failed\n");
			status = -EIO;
		}
	} else {
		printf("unable to send pipeline command\n");
		status = -EIO;
	}
	return status;
}

/* reads OOB data from the device */
static void read_oob_data(struct mtd_info *mtd, uint8_t *buf, int page)
{
	struct denali_nand_info *denali = mtd_to_denali(mtd);
	uint32_t irq_mask = INTR_STATUS__LOAD_COMP,
			 irq_status = 0, addr = 0x0, cmd = 0x0;

	denali->page = page;

	if (denali_send_pipeline_cmd(denali, false, true, SPARE_ACCESS,
				     DENALI_READ) == 0) {
		read_data_from_flash_mem(denali, buf, mtd->oobsize);

		/* wait for command to be accepted
		 * can always use status0 bit as the mask is identical for each
		 * bank. */
		irq_status = wait_for_irq(denali, irq_mask);

		if (irq_status == 0)
			printf("page on OOB timeout %d\n", denali->page);

		/* We set the device back to MAIN_ACCESS here as I observed
		 * instability with the controller if you do a block erase
		 * and the last transaction was a SPARE_ACCESS. Block erase
		 * is reliable (according to the MTD test infrastructure)
		 * if you are in MAIN_ACCESS.
		 */
		addr = BANK(denali->flash_bank) | denali->page;
		cmd = MODE_10 | addr;
		index_addr(denali, cmd, MAIN_ACCESS);
	}
}

/* this function examines buffers to see if they contain data that
 * indicate that the buffer is part of an erased region of flash.
 */
static bool is_erased(uint8_t *buf, int len)
{
	int i = 0;
	for (i = 0; i < len; i++)
		if (buf[i] != 0xFF)
			return false;
	return true;
}

/* programs the controller to either enable/disable DMA transfers */
static void denali_enable_dma(struct denali_nand_info *denali, bool en)
{
	uint32_t reg_val = 0x0;

	if (en)
		reg_val = DMA_ENABLE__FLAG;

	writel(reg_val, denali->flash_reg + DMA_ENABLE);
	readl(denali->flash_reg + DMA_ENABLE);
}

/* setups the HW to perform the data DMA */
static void denali_setup_dma(struct denali_nand_info *denali, int op)
{
	uint32_t mode;
	const int page_count = 1;
	uint32_t addr = (uint32_t)denali->buf.dma_buf;

	flush_dcache_range(addr, addr + sizeof(denali->buf.dma_buf));

/* For Denali controller that is 64 bit bus IP core */
#ifdef CONFIG_SYS_NAND_DENALI_64BIT
	mode = MODE_10 | BANK(denali->flash_bank) | denali->page;

	/* DMA is a three step process */

	/* 1. setup transfer type, interrupt when complete,
	      burst len = 64 bytes, the number of pages */
	index_addr(denali, mode, 0x01002000 | (64 << 16) | op | page_count);

	/* 2. set memory low address bits 31:0 */
	index_addr(denali, mode, addr);

	/* 3. set memory high address bits 64:32 */
	index_addr(denali, mode, 0);
#else
	mode = MODE_10 | BANK(denali->flash_bank);

	/* DMA is a four step process */

	/* 1. setup transfer type and # of pages */
	index_addr(denali, mode | denali->page, 0x2000 | op | page_count);

	/* 2. set memory high address bits 23:8 */
	index_addr(denali, mode | ((uint32_t)(addr >> 16) << 8), 0x2200);

	/* 3. set memory low address bits 23:8 */
	index_addr(denali, mode | ((uint32_t)addr << 8), 0x2300);

	/* 4.  interrupt when complete, burst len = 64 bytes*/
	index_addr(denali, mode | 0x14000, 0x2400);
#endif
}

/* Common DMA function */
static uint32_t denali_dma_configuration(struct denali_nand_info *denali,
					 uint32_t ops, bool raw_xfer,
					 uint32_t irq_mask, int oob_required)
{
	uint32_t irq_status = 0;
	/* setup_ecc_for_xfer(bool ecc_en, bool transfer_spare) */
	setup_ecc_for_xfer(denali, !raw_xfer, oob_required);

	/* clear any previous interrupt flags */
	clear_interrupts(denali);

	/* enable the DMA */
	denali_enable_dma(denali, true);

	/* setup the DMA */
	denali_setup_dma(denali, ops);

	/* wait for operation to complete */
	irq_status = wait_for_irq(denali, irq_mask);

	/* if ECC fault happen, seems we need delay before turning off DMA.
	 * If not, the controller will go into non responsive condition */
	if (irq_status & INTR_STATUS__ECC_UNCOR_ERR)
		udelay(100);

	/* disable the DMA */
	denali_enable_dma(denali, false);

	return irq_status;
}

static int write_page(struct mtd_info *mtd, struct nand_chip *chip,
			const uint8_t *buf, bool raw_xfer, int oob_required)
{
	struct denali_nand_info *denali = mtd_to_denali(mtd);

	uint32_t irq_status = 0;
	uint32_t irq_mask = INTR_STATUS__DMA_CMD_COMP;

	denali->status = 0;

	/* copy buffer into DMA buffer */
	memcpy(denali->buf.dma_buf, buf, mtd->writesize);

	/* need extra memcpy for raw transfer */
	if (raw_xfer)
		memcpy(denali->buf.dma_buf + mtd->writesize,
		       chip->oob_poi, mtd->oobsize);

	/* setting up DMA */
	irq_status = denali_dma_configuration(denali, DENALI_WRITE, raw_xfer,
					      irq_mask, oob_required);

	/* if timeout happen, error out */
	if (!(irq_status & INTR_STATUS__DMA_CMD_COMP)) {
		debug("DMA timeout for denali write_page\n");
		denali->status = NAND_STATUS_FAIL;
		return -EIO;
	}

	if (irq_status & INTR_STATUS__LOCKED_BLK) {
		debug("Failed as write to locked block\n");
		denali->status = NAND_STATUS_FAIL;
		return -EIO;
	}
	return 0;
}

/* NAND core entry points */

/*
 * this is the callback that the NAND core calls to write a page. Since
 * writing a page with ECC or without is similar, all the work is done
 * by write_page above.
 */
static int denali_write_page(struct mtd_info *mtd, struct nand_chip *chip,
				const uint8_t *buf, int oob_required)
{
	struct denali_nand_info *denali = mtd_to_denali(mtd);

	/*
	 * for regular page writes, we let HW handle all the ECC
	 * data written to the device.
	 */
	if (oob_required)
		/* switch to main + spare access */
		denali_mode_main_spare_access(denali);
	else
		/* switch to main access only */
		denali_mode_main_access(denali);

	return write_page(mtd, chip, buf, false, oob_required);
}

/*
 * This is the callback that the NAND core calls to write a page without ECC.
 * raw access is similar to ECC page writes, so all the work is done in the
 * write_page() function above.
 */
static int denali_write_page_raw(struct mtd_info *mtd, struct nand_chip *chip,
					const uint8_t *buf, int oob_required)
{
	struct denali_nand_info *denali = mtd_to_denali(mtd);

	/*
	 * for raw page writes, we want to disable ECC and simply write
	 * whatever data is in the buffer.
	 */

	if (oob_required)
		/* switch to main + spare access */
		denali_mode_main_spare_access(denali);
	else
		/* switch to main access only */
		denali_mode_main_access(denali);

	return write_page(mtd, chip, buf, true, oob_required);
}

static int denali_write_oob(struct mtd_info *mtd, struct nand_chip *chip,
				int page)
{
	return write_oob_data(mtd, chip->oob_poi, page);
}

/* raw include ECC value and all the spare area */
static int denali_read_page_raw(struct mtd_info *mtd, struct nand_chip *chip,
				uint8_t *buf, int oob_required, int page)
{
	struct denali_nand_info *denali = mtd_to_denali(mtd);

	uint32_t irq_status, irq_mask = INTR_STATUS__DMA_CMD_COMP;

	if (denali->page != page) {
		debug("Missing NAND_CMD_READ0 command\n");
		return -EIO;
	}

	if (oob_required)
		/* switch to main + spare access */
		denali_mode_main_spare_access(denali);
	else
		/* switch to main access only */
		denali_mode_main_access(denali);

	/* setting up the DMA where ecc_enable is false */
	irq_status = denali_dma_configuration(denali, DENALI_READ, true,
					      irq_mask, oob_required);

	/* if timeout happen, error out */
	if (!(irq_status & INTR_STATUS__DMA_CMD_COMP)) {
		debug("DMA timeout for denali_read_page_raw\n");
		return -EIO;
	}

	/* splitting the content to destination buffer holder */
	memcpy(chip->oob_poi, (denali->buf.dma_buf + mtd->writesize),
	       mtd->oobsize);
	memcpy(buf, denali->buf.dma_buf, mtd->writesize);

	return 0;
}

static int denali_read_page(struct mtd_info *mtd, struct nand_chip *chip,
				uint8_t *buf, int oob_required, int page)
{
	struct denali_nand_info *denali = mtd_to_denali(mtd);
	uint32_t irq_status, irq_mask =	INTR_STATUS__DMA_CMD_COMP;

	if (denali->page != page) {
		debug("Missing NAND_CMD_READ0 command\n");
		return -EIO;
	}

	if (oob_required)
		/* switch to main + spare access */
		denali_mode_main_spare_access(denali);
	else
		/* switch to main access only */
		denali_mode_main_access(denali);

	/* setting up the DMA where ecc_enable is true */
	irq_status = denali_dma_configuration(denali, DENALI_READ, false,
					      irq_mask, oob_required);

	memcpy(buf, denali->buf.dma_buf, mtd->writesize);

	/* check whether any ECC error */
	if (irq_status & INTR_STATUS__ECC_UNCOR_ERR) {
		/* is the ECC cause by erase page, check using read_page_raw */
		debug("  Uncorrected ECC detected\n");
		denali_read_page_raw(mtd, chip, buf, oob_required,
				     denali->page);

		if (is_erased(buf, mtd->writesize) == true &&
		    is_erased(chip->oob_poi, mtd->oobsize) == true) {
			debug("  ECC error cause by erased block\n");
			/* false alarm, return the 0xFF */
		} else {
			return -EIO;
		}
	}
	memcpy(buf, denali->buf.dma_buf, mtd->writesize);
	return 0;
}

static int denali_read_oob(struct mtd_info *mtd, struct nand_chip *chip,
				int page)
{
	read_oob_data(mtd, chip->oob_poi, page);

	return 0;
}

static uint8_t denali_read_byte(struct mtd_info *mtd)
{
	struct denali_nand_info *denali = mtd_to_denali(mtd);
	uint32_t addr, result;

	addr = (uint32_t)MODE_11 | BANK(denali->flash_bank);
	index_addr_read_data(denali, addr | 2, &result);
	return (uint8_t)result & 0xFF;
}

static void denali_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	struct denali_nand_info *denali = mtd_to_denali(mtd);
	uint32_t i, addr, result;

	/* delay for tR (data transfer from Flash array to data register) */
	udelay(25);

	/* ensure device completed else additional delay and polling */
	wait_for_irq(denali, INTR_STATUS__INT_ACT);

	addr = (uint32_t)MODE_11 | BANK(denali->flash_bank);
	for (i = 0; i < len; i++) {
		index_addr_read_data(denali, (uint32_t)addr | 2, &result);
		write_byte_to_buf(denali, result);
	}
	memcpy(buf, denali->buf.buf, len);
}

static void denali_select_chip(struct mtd_info *mtd, int chip)
{
	struct denali_nand_info *denali = mtd_to_denali(mtd);

	denali->flash_bank = chip;
}

static int denali_waitfunc(struct mtd_info *mtd, struct nand_chip *chip)
{
	struct denali_nand_info *denali = mtd_to_denali(mtd);
	int status = denali->status;
	denali->status = 0;

	return status;
}

static void denali_erase(struct mtd_info *mtd, int page)
{
	struct denali_nand_info *denali = mtd_to_denali(mtd);
	uint32_t cmd, irq_status;

	/* clear interrupts */
	clear_interrupts(denali);

	/* setup page read request for access type */
	cmd = MODE_10 | BANK(denali->flash_bank) | page;
	index_addr(denali, cmd, 0x1);

	/* wait for erase to complete or failure to occur */
	irq_status = wait_for_irq(denali, INTR_STATUS__ERASE_COMP |
					INTR_STATUS__ERASE_FAIL);

	if (irq_status & INTR_STATUS__ERASE_FAIL ||
	    irq_status & INTR_STATUS__LOCKED_BLK)
		denali->status = NAND_STATUS_FAIL;
	else
		denali->status = 0;
}

static void denali_cmdfunc(struct mtd_info *mtd, unsigned int cmd, int col,
			   int page)
{
	struct denali_nand_info *denali = mtd_to_denali(mtd);
	uint32_t addr;

	switch (cmd) {
	case NAND_CMD_PAGEPROG:
		break;
	case NAND_CMD_STATUS:
		addr = MODE_11 | BANK(denali->flash_bank);
		index_addr(denali, addr | 0, cmd);
		break;
	case NAND_CMD_READID:
	case NAND_CMD_PARAM:
		reset_buf(denali);
		/* sometimes ManufactureId read from register is not right
		 * e.g. some of Micron MT29F32G08QAA MLC NAND chips
		 * So here we send READID cmd to NAND insteand
		 * */
		addr = MODE_11 | BANK(denali->flash_bank);
		index_addr(denali, addr | 0, cmd);
		index_addr(denali, addr | 1, col & 0xFF);
		if (cmd == NAND_CMD_PARAM)
			udelay(50);
		break;
	case NAND_CMD_RNDOUT:
		addr = MODE_11 | BANK(denali->flash_bank);
		index_addr(denali, addr | 0, cmd);
		index_addr(denali, addr | 1, col & 0xFF);
		index_addr(denali, addr | 1, col >> 8);
		index_addr(denali, addr | 0, NAND_CMD_RNDOUTSTART);
		break;
	case NAND_CMD_READ0:
	case NAND_CMD_SEQIN:
		denali->page = page;
		break;
	case NAND_CMD_RESET:
		reset_bank(denali);
		break;
	case NAND_CMD_READOOB:
		/* TODO: Read OOB data */
		break;
	case NAND_CMD_ERASE1:
		/*
		 * supporting block erase only, not multiblock erase as
		 * it will cross plane and software need complex calculation
		 * to identify the block count for the cross plane
		 */
		denali_erase(mtd, page);
		break;
	case NAND_CMD_ERASE2:
		/* nothing to do here as it was done during NAND_CMD_ERASE1 */
		break;
	case NAND_CMD_UNLOCK1:
		addr = MODE_10 | BANK(denali->flash_bank) | page;
		index_addr(denali, addr | 0, DENALI_UNLOCK_START);
		break;
	case NAND_CMD_UNLOCK2:
		addr = MODE_10 | BANK(denali->flash_bank) | page;
		index_addr(denali, addr | 0, DENALI_UNLOCK_END);
		break;
	case NAND_CMD_LOCK:
		addr = MODE_10 | BANK(denali->flash_bank);
		index_addr(denali, addr | 0, DENALI_LOCK);
		break;
	default:
		printf(": unsupported command received 0x%x\n", cmd);
		break;
	}
}
/* end NAND core entry points */

/* Initialization code to bring the device up to a known good state */
static void denali_hw_init(struct denali_nand_info *denali)
{
	/*
	 * tell driver how many bit controller will skip before writing
	 * ECC code in OOB. This is normally used for bad block marker
	 */
	writel(CONFIG_NAND_DENALI_SPARE_AREA_SKIP_BYTES,
	       denali->flash_reg + SPARE_AREA_SKIP_BYTES);
	detect_max_banks(denali);
	denali_nand_reset(denali);
	writel(0x0F, denali->flash_reg + RB_PIN_ENABLED);
	writel(CHIP_EN_DONT_CARE__FLAG,
	       denali->flash_reg + CHIP_ENABLE_DONT_CARE);
	writel(0xffff, denali->flash_reg + SPARE_AREA_MARKER);

	/* Should set value for these registers when init */
	writel(0, denali->flash_reg + TWO_ROW_ADDR_CYCLES);
	writel(1, denali->flash_reg + ECC_ENABLE);
	denali_nand_timing_set(denali);
	denali_irq_init(denali);
}

static struct nand_ecclayout nand_oob;

static int denali_init(struct denali_nand_info *denali)
{
	int ret;

	denali_hw_init(denali);

	denali->mtd->name = "denali-nand";
	denali->mtd->owner = THIS_MODULE;
	denali->mtd->priv = &denali->nand;

	/* register the driver with the NAND core subsystem */
	denali->nand.select_chip = denali_select_chip;
	denali->nand.cmdfunc = denali_cmdfunc;
	denali->nand.read_byte = denali_read_byte;
	denali->nand.read_buf = denali_read_buf;
	denali->nand.waitfunc = denali_waitfunc;

	/*
	 * scan for NAND devices attached to the controller
	 * this is the first stage in a two step process to register
	 * with the nand subsystem
	 */
	if (nand_scan_ident(denali->mtd, denali->max_banks, NULL)) {
		ret = -ENXIO;
		goto fail;
	}

#ifdef CONFIG_SYS_NAND_USE_FLASH_BBT
	/* check whether flash got BBT table (located at end of flash). As we
	 * use NAND_BBT_NO_OOB, the BBT page will start with
	 * bbt_pattern. We will have mirror pattern too */
	denali->nand.bbt_options |= NAND_BBT_USE_FLASH;
	/*
	 * We are using main + spare with ECC support. As BBT need ECC support,
	 * we need to ensure BBT code don't write to OOB for the BBT pattern.
	 * All BBT info will be stored into data area with ECC support.
	 */
	denali->nand.bbt_options |= NAND_BBT_NO_OOB;
#endif

	denali->nand.ecc.mode = NAND_ECC_HW;
	denali->nand.ecc.size = CONFIG_NAND_DENALI_ECC_SIZE;

	/*
	 * Tell driver the ecc strength. This register may be already set
	 * correctly. So we read this value out.
	 */
	denali->nand.ecc.strength = readl(denali->flash_reg + ECC_CORRECTION);
	switch (denali->nand.ecc.size) {
	case 512:
		denali->nand.ecc.bytes =
			(denali->nand.ecc.strength * 13 + 15) / 16 * 2;
		break;
	case 1024:
		denali->nand.ecc.bytes =
			(denali->nand.ecc.strength * 14 + 15) / 16 * 2;
		break;
	default:
		pr_err("Unsupported ECC size\n");
		ret = -EINVAL;
		goto fail;
	}
	nand_oob.eccbytes = denali->nand.ecc.bytes;
	denali->nand.ecc.layout = &nand_oob;

	writel(denali->mtd->erasesize / denali->mtd->writesize,
	       denali->flash_reg + PAGES_PER_BLOCK);
	writel(denali->nand.options & NAND_BUSWIDTH_16 ? 1 : 0,
	       denali->flash_reg + DEVICE_WIDTH);
	writel(denali->mtd->writesize,
	       denali->flash_reg + DEVICE_MAIN_AREA_SIZE);
	writel(denali->mtd->oobsize,
	       denali->flash_reg + DEVICE_SPARE_AREA_SIZE);
	if (readl(denali->flash_reg + DEVICES_CONNECTED) == 0)
		writel(1, denali->flash_reg + DEVICES_CONNECTED);

	/* override the default operations */
	denali->nand.ecc.read_page = denali_read_page;
	denali->nand.ecc.read_page_raw = denali_read_page_raw;
	denali->nand.ecc.write_page = denali_write_page;
	denali->nand.ecc.write_page_raw = denali_write_page_raw;
	denali->nand.ecc.read_oob = denali_read_oob;
	denali->nand.ecc.write_oob = denali_write_oob;

	if (nand_scan_tail(denali->mtd)) {
		ret = -ENXIO;
		goto fail;
	}

	ret = nand_register(0);

fail:
	return ret;
}

static int __board_nand_init(void)
{
	struct denali_nand_info *denali;

	denali = kzalloc(sizeof(*denali), GFP_KERNEL);
	if (!denali)
		return -ENOMEM;

	/*
	 * If CONFIG_SYS_NAND_SELF_INIT is defined, each driver is responsible
	 * for instantiating struct nand_chip, while drivers/mtd/nand/nand.c
	 * still provides a "struct mtd_info nand_info" instance.
	 */
	denali->mtd = &nand_info[0];

	/*
	 * In the future, these base addresses should be taken from
	 * Device Tree or platform data.
	 */
	denali->flash_reg = (void  __iomem *)CONFIG_SYS_NAND_REGS_BASE;
	denali->flash_mem = (void  __iomem *)CONFIG_SYS_NAND_DATA_BASE;

	return denali_init(denali);
}

void board_nand_init(void)
{
	if (__board_nand_init() < 0)
		pr_warn("Failed to initialize Denali NAND controller.\n");
}
