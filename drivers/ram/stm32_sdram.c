/*
 * (C) Copyright 2017
 * Vikas Manocha, <vikas.manocha@st.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <ram.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

struct stm32_fmc_regs {
	/* 0x0 */
	u32 bcr1;	/* NOR/PSRAM Chip select control register 1 */
	u32 btr1;	/* SRAM/NOR-Flash Chip select timing register 1 */
	u32 bcr2;	/* NOR/PSRAM Chip select Control register 2 */
	u32 btr2;	/* SRAM/NOR-Flash Chip select timing register 2 */
	u32 bcr3;	/* NOR/PSRAMChip select Control register 3 */
	u32 btr3;	/* SRAM/NOR-Flash Chip select timing register 3 */
	u32 bcr4;	/* NOR/PSRAM Chip select Control register 4 */
	u32 btr4;	/* SRAM/NOR-Flash Chip select timing register 4 */
	u32 reserved1[24];

	/* 0x80 */
	u32 pcr;	/* NAND Flash control register */
	u32 sr;		/* FIFO status and interrupt register */
	u32 pmem;	/* Common memory space timing register */
	u32 patt;	/* Attribute memory space timing registers  */
	u32 reserved2[1];
	u32 eccr;	/* ECC result registers */
	u32 reserved3[27];

	/* 0x104 */
	u32 bwtr1;	/* SRAM/NOR-Flash write timing register 1 */
	u32 reserved4[1];
	u32 bwtr2;	/* SRAM/NOR-Flash write timing register 2 */
	u32 reserved5[1];
	u32 bwtr3;	/* SRAM/NOR-Flash write timing register 3 */
	u32 reserved6[1];
	u32 bwtr4;	/* SRAM/NOR-Flash write timing register 4 */
	u32 reserved7[8];

	/* 0x140 */
	u32 sdcr1;	/* SDRAM Control register 1 */
	u32 sdcr2;	/* SDRAM Control register 2 */
	u32 sdtr1;	/* SDRAM Timing register 1 */
	u32 sdtr2;	/* SDRAM Timing register 2 */
	u32 sdcmr;	/* SDRAM Mode register */
	u32 sdrtr;	/* SDRAM Refresh timing register */
	u32 sdsr;	/* SDRAM Status register */
};

/* Control register SDCR */
#define FMC_SDCR_RPIPE_SHIFT	13	/* RPIPE bit shift */
#define FMC_SDCR_RBURST_SHIFT	12	/* RBURST bit shift */
#define FMC_SDCR_SDCLK_SHIFT	10	/* SDRAM clock divisor shift */
#define FMC_SDCR_WP_SHIFT	9	/* Write protection shift */
#define FMC_SDCR_CAS_SHIFT	7	/* CAS latency shift */
#define FMC_SDCR_NB_SHIFT	6	/* Number of banks shift */
#define FMC_SDCR_MWID_SHIFT	4	/* Memory width shift */
#define FMC_SDCR_NR_SHIFT	2	/* Number of row address bits shift */
#define FMC_SDCR_NC_SHIFT	0	/* Number of col address bits shift */

/* Timings register SDTR */
#define FMC_SDTR_TMRD_SHIFT	0	/* Load mode register to active */
#define FMC_SDTR_TXSR_SHIFT	4	/* Exit self-refresh time */
#define FMC_SDTR_TRAS_SHIFT	8	/* Self-refresh time */
#define FMC_SDTR_TRC_SHIFT	12	/* Row cycle delay */
#define FMC_SDTR_TWR_SHIFT	16	/* Recovery delay */
#define FMC_SDTR_TRP_SHIFT	20	/* Row precharge delay */
#define FMC_SDTR_TRCD_SHIFT	24	/* Row-to-column delay */

#define FMC_SDCMR_NRFS_SHIFT	5

#define FMC_SDCMR_MODE_NORMAL		0
#define FMC_SDCMR_MODE_START_CLOCK	1
#define FMC_SDCMR_MODE_PRECHARGE	2
#define FMC_SDCMR_MODE_AUTOREFRESH	3
#define FMC_SDCMR_MODE_WRITE_MODE	4
#define FMC_SDCMR_MODE_SELFREFRESH	5
#define FMC_SDCMR_MODE_POWERDOWN	6

#define FMC_SDCMR_BANK_1		BIT(4)
#define FMC_SDCMR_BANK_2		BIT(3)

#define FMC_SDCMR_MODE_REGISTER_SHIFT	9

#define FMC_SDSR_BUSY			BIT(5)

#define FMC_BUSY_WAIT(regs)	do { \
		__asm__ __volatile__ ("dsb" : : : "memory"); \
		while (regs->sdsr & FMC_SDSR_BUSY) \
			; \
	} while (0)

struct stm32_sdram_control {
	u8 no_columns;
	u8 no_rows;
	u8 memory_width;
	u8 no_banks;
	u8 cas_latency;
	u8 sdclk;
	u8 rd_burst;
	u8 rd_pipe_delay;
};

struct stm32_sdram_timing {
	u8 tmrd;
	u8 txsr;
	u8 tras;
	u8 trc;
	u8 trp;
	u8 twr;
	u8 trcd;
};
struct stm32_sdram_params {
	struct stm32_fmc_regs *base;
	u8 no_sdram_banks;
	struct stm32_sdram_control sdram_control;
	struct stm32_sdram_timing sdram_timing;
	u32 sdram_ref_count;
};

#define SDRAM_MODE_BL_SHIFT	0
#define SDRAM_MODE_CAS_SHIFT	4
#define SDRAM_MODE_BL		0

int stm32_sdram_init(struct udevice *dev)
{
	struct stm32_sdram_params *params = dev_get_platdata(dev);
	struct stm32_fmc_regs *regs = params->base;

	writel(params->sdram_control.sdclk << FMC_SDCR_SDCLK_SHIFT
		| params->sdram_control.cas_latency << FMC_SDCR_CAS_SHIFT
		| params->sdram_control.no_banks << FMC_SDCR_NB_SHIFT
		| params->sdram_control.memory_width << FMC_SDCR_MWID_SHIFT
		| params->sdram_control.no_rows << FMC_SDCR_NR_SHIFT
		| params->sdram_control.no_columns << FMC_SDCR_NC_SHIFT
		| params->sdram_control.rd_pipe_delay << FMC_SDCR_RPIPE_SHIFT
		| params->sdram_control.rd_burst << FMC_SDCR_RBURST_SHIFT,
		&regs->sdcr1);

	writel(params->sdram_timing.trcd << FMC_SDTR_TRCD_SHIFT
		| params->sdram_timing.trp << FMC_SDTR_TRP_SHIFT
		| params->sdram_timing.twr << FMC_SDTR_TWR_SHIFT
		| params->sdram_timing.trc << FMC_SDTR_TRC_SHIFT
		| params->sdram_timing.tras << FMC_SDTR_TRAS_SHIFT
		| params->sdram_timing.txsr << FMC_SDTR_TXSR_SHIFT
		| params->sdram_timing.tmrd << FMC_SDTR_TMRD_SHIFT,
		&regs->sdtr1);

	writel(FMC_SDCMR_BANK_1 | FMC_SDCMR_MODE_START_CLOCK,
	       &regs->sdcmr);
	udelay(200);	/* 200 us delay, page 10, "Power-Up" */
	FMC_BUSY_WAIT(regs);

	writel(FMC_SDCMR_BANK_1 | FMC_SDCMR_MODE_PRECHARGE,
	       &regs->sdcmr);
	udelay(100);
	FMC_BUSY_WAIT(regs);

	writel((FMC_SDCMR_BANK_1 | FMC_SDCMR_MODE_AUTOREFRESH
		| 7 << FMC_SDCMR_NRFS_SHIFT), &regs->sdcmr);
	udelay(100);
	FMC_BUSY_WAIT(regs);

	writel(FMC_SDCMR_BANK_1 | (SDRAM_MODE_BL << SDRAM_MODE_BL_SHIFT
	       | params->sdram_control.cas_latency << SDRAM_MODE_CAS_SHIFT)
	       << FMC_SDCMR_MODE_REGISTER_SHIFT | FMC_SDCMR_MODE_WRITE_MODE,
	       &regs->sdcmr);
	udelay(100);
	FMC_BUSY_WAIT(regs);

	writel(FMC_SDCMR_BANK_1 | FMC_SDCMR_MODE_NORMAL,
	       &regs->sdcmr);
	FMC_BUSY_WAIT(regs);

	/* Refresh timer */
	writel((params->sdram_ref_count) << 1, &regs->sdrtr);

	return 0;
}

static int stm32_fmc_ofdata_to_platdata(struct udevice *dev)
{
	int ret;
	int node = dev_of_offset(dev);
	const void *blob = gd->fdt_blob;
	struct stm32_sdram_params *params = dev_get_platdata(dev);

	params->no_sdram_banks = fdtdec_get_uint(blob, node, "mr-nbanks", 1);
	debug("%s, no of banks = %d\n", __func__, params->no_sdram_banks);

	fdt_for_each_subnode(node, blob, node) {
		ret = fdtdec_get_byte_array(blob, node, "st,sdram-control",
					    (u8 *)&params->sdram_control,
					    sizeof(params->sdram_control));
		if (ret)
			return ret;
		ret = fdtdec_get_byte_array(blob, node, "st,sdram-timing",
					    (u8 *)&params->sdram_timing,
					    sizeof(params->sdram_timing));
		if (ret)
			return ret;

		params->sdram_ref_count = fdtdec_get_int(blob, node,
						"st,sdram-refcount", 8196);
	}

	return 0;
}

static int stm32_fmc_probe(struct udevice *dev)
{
	struct stm32_sdram_params *params = dev_get_platdata(dev);
	int ret;
	fdt_addr_t addr;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	params->base = (struct stm32_fmc_regs *)addr;

#ifdef CONFIG_CLK
	struct clk clk;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret < 0)
		return ret;

	ret = clk_enable(&clk);

	if (ret) {
		dev_err(dev, "failed to enable clock\n");
		return ret;
	}
#endif
	ret = stm32_sdram_init(dev);
	if (ret)
		return ret;

	return 0;
}

static int stm32_fmc_get_info(struct udevice *dev, struct ram_info *info)
{
	return 0;
}

static struct ram_ops stm32_fmc_ops = {
	.get_info = stm32_fmc_get_info,
};

static const struct udevice_id stm32_fmc_ids[] = {
	{ .compatible = "st,stm32-fmc" },
	{ }
};

U_BOOT_DRIVER(stm32_fmc) = {
	.name = "stm32_fmc",
	.id = UCLASS_RAM,
	.of_match = stm32_fmc_ids,
	.ops = &stm32_fmc_ops,
	.ofdata_to_platdata = stm32_fmc_ofdata_to_platdata,
	.probe = stm32_fmc_probe,
	.platdata_auto_alloc_size = sizeof(struct stm32_sdram_params),
};
