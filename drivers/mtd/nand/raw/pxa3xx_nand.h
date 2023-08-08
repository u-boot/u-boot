#ifndef __ASM_ARCH_PXA3XX_NAND_H
#define __ASM_ARCH_PXA3XX_NAND_H

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

struct pxa3xx_nand_timing {
	unsigned int	tCH;  /* Enable signal hold time */
	unsigned int	tCS;  /* Enable signal setup time */
	unsigned int	tWH;  /* ND_nWE high duration */
	unsigned int	tWP;  /* ND_nWE pulse time */
	unsigned int	tRH;  /* ND_nRE high duration */
	unsigned int	tRP;  /* ND_nRE pulse width */
	unsigned int	tR;   /* ND_nWE high to ND_nRE low for read */
	unsigned int	tWHR; /* ND_nWE high to ND_nRE low for status read */
	unsigned int	tAR;  /* ND_ALE low to ND_nRE low delay */
};

struct pxa3xx_nand_flash {
	uint32_t	chip_id;
	unsigned int	flash_width;    /* Width of Flash memory (DWIDTH_M) */
	unsigned int	dfc_width;      /* Width of flash controller(DWIDTH_C) */
	struct pxa3xx_nand_timing *timing; /* NAND Flash timing */
};

/*
 * Current pxa3xx_nand controller has two chip select which
 * both be workable.
 *
 * Notice should be taken that:
 * When you want to use this feature, you should not enable the
 * keep configuration feature, for two chip select could be
 * attached with different nand chip. The different page size
 * and timing requirement make the keep configuration impossible.
 */

/* The max num of chip select current support */
#define NUM_CHIP_SELECT		(2)
struct pxa3xx_nand_platform_data {
	/* the data flash bus is shared between the Static Memory
	 * Controller and the Data Flash Controller,  the arbiter
	 * controls the ownership of the bus
	 */
	int	enable_arbiter;

	/* allow platform code to keep OBM/bootloader defined NFC config */
	int	keep_config;

	/* indicate how many chip selects will be used */
	int	num_cs;

	/* use an flash-based bad block table */
	bool	flash_bbt;

	/* requested ECC strength and ECC step size */
	int ecc_strength, ecc_step_size;

	const struct mtd_partition		*parts[NUM_CHIP_SELECT];
	unsigned int				nr_parts[NUM_CHIP_SELECT];

	const struct pxa3xx_nand_flash		*flash;
	size_t					num_flash;
};

enum pxa3xx_nand_variant {
	PXA3XX_NAND_VARIANT_PXA,
	PXA3XX_NAND_VARIANT_ARMADA370,
};

struct pxa3xx_nand_host {
	struct nand_chip	chip;
	void			*info_data;

	/* page size of attached chip */
	int			use_ecc;
	int			cs;

	/* calculated from pxa3xx_nand_flash data */
	unsigned int		col_addr_cycles;
	unsigned int		row_addr_cycles;
};

struct pxa3xx_nand_info {
	struct nand_hw_control	controller;
	struct pxa3xx_nand_platform_data *pdata;

	struct clk		*clk;
	void __iomem		*mmio_base;
	unsigned long		mmio_phys;
	int			cmd_complete, dev_ready;

	unsigned int		buf_start;
	unsigned int		buf_count;
	unsigned int		buf_size;
	unsigned int		data_buff_pos;
	unsigned int		oob_buff_pos;

	unsigned char		*data_buff;
	unsigned char		*oob_buff;

	struct pxa3xx_nand_host *host[NUM_CHIP_SELECT];
	unsigned int		state;

	/*
	 * This driver supports NFCv1 (as found in PXA SoC)
	 * and NFCv2 (as found in Armada 370/XP SoC).
	 */
	enum pxa3xx_nand_variant variant;

	int			cs;
	int			use_ecc;	/* use HW ECC ? */
	int			force_raw;	/* prevent use_ecc to be set */
	int			ecc_bch;	/* using BCH ECC? */
	int			use_spare;	/* use spare ? */
	int			need_wait;

	/* Amount of real data per full chunk */
	unsigned int		chunk_size;

	/* Amount of spare data per full chunk */
	unsigned int		spare_size;

	/* Number of full chunks (i.e chunk_size + spare_size) */
	unsigned int            nfullchunks;

	/*
	 * Total number of chunks. If equal to nfullchunks, then there
	 * are only full chunks. Otherwise, there is one last chunk of
	 * size (last_chunk_size + last_spare_size)
	 */
	unsigned int            ntotalchunks;

	/* Amount of real data in the last chunk */
	unsigned int		last_chunk_size;

	/* Amount of spare data in the last chunk */
	unsigned int		last_spare_size;

	unsigned int		ecc_size;
	unsigned int		ecc_err_cnt;
	unsigned int		max_bitflips;
	int			retcode;

	/*
	 * Variables only valid during command
	 * execution. step_chunk_size and step_spare_size is the
	 * amount of real data and spare data in the current
	 * chunk. cur_chunk is the current chunk being
	 * read/programmed.
	 */
	unsigned int		step_chunk_size;
	unsigned int		step_spare_size;
	unsigned int            cur_chunk;

	/* cached register value */
	uint32_t		reg_ndcr;
	uint32_t		ndtr0cs0;
	uint32_t		ndtr1cs0;

	/* generated NDCBx register values */
	uint32_t		ndcb0;
	uint32_t		ndcb1;
	uint32_t		ndcb2;
	uint32_t		ndcb3;

	/* SOC related registers for nand */
	void __iomem		*nand_flash_clk_ctrl_reg;
	void __iomem		*soc_dev_multiplex_reg;
};

#endif /* __ASM_ARCH_PXA3XX_NAND_H */
