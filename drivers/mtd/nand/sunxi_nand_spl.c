/*
 * Copyright (c) 2014, Antmicro Ltd <www.antmicro.com>
 * Copyright (c) 2015, Turtle Solutions <www.turtle-solutions.eu>
 * Copyright (c) 2015, Roy Spliet <rspliet@ultimaker.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * \todo Detect chip parameters (page size, ECC mode, randomisation...)
 */

#include <common.h>
#include <config.h>
#include <asm/io.h>
#include <nand.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clock.h>
#include <asm/arch/dma.h>
#include <asm/arch/nand.h>

void
nand_init(void)
{
	struct sunxi_ccm_reg * const ccm =
			(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	struct sunxi_nand * const nand = (struct sunxi_nand *)SUNXI_NFC_BASE;
	u32 val;

	board_nand_init();

	/* "un-gate" NAND clock and clock source
	 * This assumes that the clock was already correctly configured by
	 * BootROM */
	setbits_le32(&ccm->ahb_gate0, (1 << AHB_GATE_OFFSET_NAND0));
#ifdef CONFIG_MACH_SUN9I
	setbits_le32(&ccm->ahb_gate1, (1 << AHB_GATE_OFFSET_DMA));
#else
	setbits_le32(&ccm->ahb_gate0, (1 << AHB_GATE_OFFSET_DMA));
#endif
	setbits_le32(&ccm->nand0_clk_cfg, 0x80000000);

	val = readl(&nand->ctl);
	val |= SUNXI_NAND_CTL_RST;
	writel(val, &nand->ctl);

	/* Wait until reset pin is deasserted */
	do {
		val = readl(&nand->ctl);
		if (!(val & SUNXI_NAND_CTL_RST))
			break;
	} while (1);

	/** \todo Chip select, currently kind of static */
	val = readl(&nand->ctl);
	val &= 0xf0fff0f2;
	val |= SUNXI_NAND_CTL_EN;
	val |= SUNXI_NAND_CTL_PAGE_SIZE(CONFIG_NAND_SUNXI_PAGE_SIZE);
	writel(val, &nand->ctl);

	writel(0x100, &nand->timing_ctl);
	writel(0x7ff, &nand->timing_cfg);

	/* reset CMD  */
	val = SUNXI_NAND_CMD_SEND_CMD1 | SUNXI_NAND_CMD_WAIT_FLAG |
			NAND_CMD_RESET;
	writel(val, &nand->cmd);
	do {
		val = readl(&nand->st);
		if (val & (1<<1))
			break;
		udelay(1000);
	} while (1);

	printf("Nand initialised\n");
}

int
nand_wait_timeout(u32 *reg, u32 mask, u32 val)
{
	unsigned long tmo = timer_get_us() + 1000000; /* 1s */

	while ((readl(reg) & mask) != val) {
		if (timer_get_us() > tmo)
			return -ETIMEDOUT;
	}

	return 0;
}

/* random seed */
static const uint16_t random_seed[128] = {
	0x2b75, 0x0bd0, 0x5ca3, 0x62d1, 0x1c93, 0x07e9, 0x2162, 0x3a72,
	0x0d67, 0x67f9, 0x1be7, 0x077d, 0x032f, 0x0dac, 0x2716, 0x2436,
	0x7922, 0x1510, 0x3860, 0x5287, 0x480f, 0x4252, 0x1789, 0x5a2d,
	0x2a49, 0x5e10, 0x437f, 0x4b4e, 0x2f45, 0x216e, 0x5cb7, 0x7130,
	0x2a3f, 0x60e4, 0x4dc9, 0x0ef0, 0x0f52, 0x1bb9, 0x6211, 0x7a56,
	0x226d, 0x4ea7, 0x6f36, 0x3692, 0x38bf, 0x0c62, 0x05eb, 0x4c55,
	0x60f4, 0x728c, 0x3b6f, 0x2037, 0x7f69, 0x0936, 0x651a, 0x4ceb,
	0x6218, 0x79f3, 0x383f, 0x18d9, 0x4f05, 0x5c82, 0x2912, 0x6f17,
	0x6856, 0x5938, 0x1007, 0x61ab, 0x3e7f, 0x57c2, 0x542f, 0x4f62,
	0x7454, 0x2eac, 0x7739, 0x42d4, 0x2f90, 0x435a, 0x2e52, 0x2064,
	0x637c, 0x66ad, 0x2c90, 0x0bad, 0x759c, 0x0029, 0x0986, 0x7126,
	0x1ca7, 0x1605, 0x386a, 0x27f5, 0x1380, 0x6d75, 0x24c3, 0x0f8e,
	0x2b7a, 0x1418, 0x1fd1, 0x7dc1, 0x2d8e, 0x43af, 0x2267, 0x7da3,
	0x4e3d, 0x1338, 0x50db, 0x454d, 0x764d, 0x40a3, 0x42e6, 0x262b,
	0x2d2e, 0x1aea, 0x2e17, 0x173d, 0x3a6e, 0x71bf, 0x25f9, 0x0a5d,
	0x7c57, 0x0fbe, 0x46ce, 0x4939, 0x6b17, 0x37bb, 0x3e91, 0x76db,
};

uint32_t ecc_errors = 0;

static void
nand_config_ecc(struct sunxi_nand *nand, uint32_t page, int syndrome)
{
	static u8 strength[] = {16, 24, 28, 32, 40, 48, 56, 60, 64};
	int i;
	uint32_t ecc_mode;
	u32 ecc;
	u16 seed = 0;

	for (i = 0; i < ARRAY_SIZE(strength); i++) {
		if (CONFIG_NAND_SUNXI_ECC_STRENGTH == strength[i]) {
			ecc_mode = i;
			break;
		}
	}

	if (i == ARRAY_SIZE(strength)) {
		printf("ECC strength unsupported\n");
		return;
	}

	ecc = 	SUNXI_NAND_ECC_CTL_ECC_EN |
		SUNXI_NAND_ECC_CTL_PIPELINE |
		SUNXI_NAND_ECC_CTL_RND_EN |
		SUNXI_NAND_ECC_CTL_MODE(ecc_mode);

	if (CONFIG_NAND_SUNXI_ECC_STEP == 512)
		ecc |= SUNXI_NAND_ECC_CTL_BS_512B;

	if (syndrome)
		seed = 0x4A80;
	else
		seed = random_seed[page % ARRAY_SIZE(random_seed)];

	ecc |= SUNXI_NAND_ECC_CTL_RND_SEED(seed);

	writel(ecc, &nand->ecc_ctl);
}

/* read CONFIG_NAND_SUNXI_ECC_STEP bytes from real_addr to temp_buf */
void
nand_read_block(struct sunxi_nand *nand, phys_addr_t src, dma_addr_t dst,
		int syndrome)
{
	struct sunxi_dma * const dma = (struct sunxi_dma *)SUNXI_DMA_BASE;
	struct sunxi_dma_cfg * const dma_cfg = &dma->ddma[0];

	uint32_t shift;
	uint32_t page;
	uint32_t addr;
	uint32_t oob_offset;
	uint32_t ecc_bytes;
	u32 val;
	u32 cmd;

	page = src / CONFIG_NAND_SUNXI_PAGE_SIZE;
	if (page > 0xFFFF) {
		/* TODO: currently this is not supported */
		printf("Reading from address >= %08X is not allowed.\n",
		       0xFFFF * CONFIG_NAND_SUNXI_PAGE_SIZE);
		return;
	}

	shift = src % CONFIG_NAND_SUNXI_PAGE_SIZE;
	writel(0, &nand->ecc_st);

	/* ECC_CTL, randomization */
	ecc_bytes = CONFIG_NAND_SUNXI_ECC_STRENGTH *
			fls(CONFIG_NAND_SUNXI_ECC_STEP * 8);
	ecc_bytes = DIV_ROUND_UP(ecc_bytes, 8);
	ecc_bytes += (ecc_bytes & 1); /* Align to 2-bytes */
	ecc_bytes += 4;

	nand_config_ecc(nand, page, syndrome);
	if (syndrome) {
		/* shift every 1kB in syndrome */
		shift += (shift / CONFIG_NAND_SUNXI_ECC_STEP) * ecc_bytes;
		oob_offset = CONFIG_NAND_SUNXI_ECC_STEP + shift;
	} else {
		oob_offset = CONFIG_NAND_SUNXI_PAGE_SIZE  +
			(shift / CONFIG_NAND_SUNXI_ECC_STEP) * ecc_bytes;
	}

	addr = (page << 16) | shift;

	/* DMA */
	val = readl(&nand->ctl);
	writel(val | SUNXI_NAND_CTL_RAM_METHOD_DMA, &nand->ctl);

	writel(oob_offset, &nand->spare_area);

	/* DMAC
	 * \todo Separate this into a tidy driver */
	writel(0x0, &dma->irq_en); /* clear dma interrupts */
	writel((uint32_t) &nand->io_data , &dma_cfg->src_addr);
	writel(dst            , &dma_cfg->dst_addr);
	writel(0x00007F0F     , &dma_cfg->ddma_para);
	writel(CONFIG_NAND_SUNXI_ECC_STEP, &dma_cfg->bc);

	val = 	SUNXI_DMA_CTL_SRC_DRQ(DDMA_SRC_DRQ_NAND) |
		SUNXI_DMA_CTL_MODE_IO |
		SUNXI_DMA_CTL_SRC_DATA_WIDTH_32 |
		SUNXI_DMA_CTL_DST_DRQ(DDMA_DST_DRQ_SDRAM) |
		SUNXI_DMA_CTL_DST_DATA_WIDTH_32 |
		SUNXI_DMA_CTL_TRIGGER;
	writel(val, &dma_cfg->ctl);

	writel(0x00E00530, &nand->rcmd_set);
	nand_wait_timeout(&nand->st, SUNXI_NAND_ST_FIFO_FULL, 0);

	writel(1   , &nand->block_num);
	writel(addr, &nand->addr_low);
	writel(0   , &nand->addr_high);

	/* CMD (PAGE READ) */
	cmd = 0x85E80000;
	cmd |= SUNXI_NAND_CMD_ADDR_CYCLES(CONFIG_NAND_SUNXI_ADDR_CYCLES);
	cmd |= (syndrome ? SUNXI_NAND_CMD_ORDER_SEQ :
			SUNXI_NAND_CMD_ORDER_INTERLEAVE);
	writel(cmd, &nand->cmd);

	if(nand_wait_timeout(&nand->st, SUNXI_NAND_ST_DMA_INT,
			SUNXI_NAND_ST_DMA_INT)) {
		printf("NAND timeout reading data\n");
		return;
	}

	if(nand_wait_timeout(&dma_cfg->ctl, SUNXI_DMA_CTL_TRIGGER, 0)) {
		printf("NAND timeout reading data\n");
		return;
	}

	if (readl(&nand->ecc_st))
		ecc_errors++;
}

int
nand_spl_load_image(uint32_t offs, unsigned int size, void *dest)
{
	struct sunxi_nand * const nand = (struct sunxi_nand *)SUNXI_NFC_BASE;
	dma_addr_t dst_block;
	dma_addr_t dst_end;
	phys_addr_t addr = offs;

	dst_end = ((dma_addr_t) dest) + size;

	memset((void *)dest, 0x0, size);
	ecc_errors = 0;
	for (dst_block = (dma_addr_t) dest; dst_block < dst_end;
			dst_block += CONFIG_NAND_SUNXI_ECC_STEP,
			addr += CONFIG_NAND_SUNXI_ECC_STEP) {
		/* syndrome read first 4MiB to match Allwinner BootROM */
		nand_read_block(nand, addr, dst_block, addr < 0x400000);
	}

	if (ecc_errors)
		printf("Error: %d ECC failures detected\n", ecc_errors);
	return ecc_errors == 0;
}

void
nand_deselect(void)
{}
