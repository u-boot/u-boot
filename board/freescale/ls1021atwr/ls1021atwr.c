/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <i2c.h>
#include <asm/io.h>
#include <asm/arch/immap_ls102xa.h>
#include <asm/arch/clock.h>
#include <asm/arch/fsl_serdes.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <fsl_ifc.h>
#include <netdev.h>
#include <fsl_mdio.h>
#include <tsec.h>

DECLARE_GLOBAL_DATA_PTR;

#define VERSION_MASK		0x00FF
#define BANK_MASK		0x0001
#define CONFIG_RESET		0x1
#define INIT_RESET		0x1

#define CPLD_SET_MUX_SERDES	0x20
#define CPLD_SET_BOOT_BANK	0x40

#define BOOT_FROM_UPPER_BANK	0x0
#define BOOT_FROM_LOWER_BANK	0x1

#define LANEB_SATA		(0x01)
#define LANEB_SGMII1		(0x02)
#define LANEC_SGMII1		(0x04)
#define LANEC_PCIEX1		(0x08)
#define LANED_PCIEX2		(0x10)
#define LANED_SGMII2		(0x20)

#define MASK_LANE_B		0x1
#define MASK_LANE_C		0x2
#define MASK_LANE_D		0x4
#define MASK_SGMII		0x8

#define KEEP_STATUS		0x0
#define NEED_RESET		0x1

struct cpld_data {
	u8 cpld_ver;		/* cpld revision */
	u8 cpld_ver_sub;	/* cpld sub revision */
	u8 pcba_ver;		/* pcb revision number */
	u8 system_rst;		/* reset system by cpld */
	u8 soft_mux_on;		/* CPLD override physical switches Enable */
	u8 cfg_rcw_src1;	/* Reset config word 1 */
	u8 cfg_rcw_src2;	/* Reset config word 2 */
	u8 vbank;		/* Flash bank selection Control */
	u8 gpio;		/* GPIO for TWR-ELEV */
	u8 i2c3_ifc_mux;
	u8 mux_spi2;
	u8 can3_usb2_mux;	/* CAN3 and USB2 Selection */
	u8 qe_lcd_mux;		/* QE and LCD Selection */
	u8 serdes_mux;		/* Multiplexed pins for SerDes Lanes */
	u8 global_rst;		/* reset with init CPLD reg to default */
	u8 rev1;		/* Reserved */
	u8 rev2;		/* Reserved */
};

static void convert_serdes_mux(int type, int need_reset);

void cpld_show(void)
{
	struct cpld_data *cpld_data = (void *)(CONFIG_SYS_CPLD_BASE);

	printf("CPLD:  V%x.%x\nPCBA:  V%x.0\nVBank: %d\n",
	       in_8(&cpld_data->cpld_ver) & VERSION_MASK,
	       in_8(&cpld_data->cpld_ver_sub) & VERSION_MASK,
	       in_8(&cpld_data->pcba_ver) & VERSION_MASK,
	       in_8(&cpld_data->vbank) & BANK_MASK);

#ifdef CONFIG_DEBUG
	printf("soft_mux_on =%x\n",
	       in_8(&cpld_data->soft_mux_on));
	printf("cfg_rcw_src1 =%x\n",
	       in_8(&cpld_data->cfg_rcw_src1));
	printf("cfg_rcw_src2 =%x\n",
	       in_8(&cpld_data->cfg_rcw_src2));
	printf("vbank =%x\n",
	       in_8(&cpld_data->vbank));
	printf("gpio =%x\n",
	       in_8(&cpld_data->gpio));
	printf("i2c3_ifc_mux =%x\n",
	       in_8(&cpld_data->i2c3_ifc_mux));
	printf("mux_spi2 =%x\n",
	       in_8(&cpld_data->mux_spi2));
	printf("can3_usb2_mux =%x\n",
	       in_8(&cpld_data->can3_usb2_mux));
	printf("qe_lcd_mux =%x\n",
	       in_8(&cpld_data->qe_lcd_mux));
	printf("serdes_mux =%x\n",
	       in_8(&cpld_data->serdes_mux));
#endif
}

int checkboard(void)
{
	puts("Board: LS1021ATWR\n");
	cpld_show();

	return 0;
}

void ddrmc_init(void)
{
	struct ccsr_ddr *ddr = (struct ccsr_ddr *)CONFIG_SYS_FSL_DDR_ADDR;

	out_be32(&ddr->sdram_cfg, DDR_SDRAM_CFG);

	out_be32(&ddr->cs0_bnds, DDR_CS0_BNDS);
	out_be32(&ddr->cs0_config, DDR_CS0_CONFIG);

	out_be32(&ddr->timing_cfg_0, DDR_TIMING_CFG_0);
	out_be32(&ddr->timing_cfg_1, DDR_TIMING_CFG_1);
	out_be32(&ddr->timing_cfg_2, DDR_TIMING_CFG_2);
	out_be32(&ddr->timing_cfg_3, DDR_TIMING_CFG_3);
	out_be32(&ddr->timing_cfg_4, DDR_TIMING_CFG_4);
	out_be32(&ddr->timing_cfg_5, DDR_TIMING_CFG_5);

	out_be32(&ddr->sdram_cfg_2,  DDR_SDRAM_CFG_2);

	out_be32(&ddr->sdram_mode, DDR_SDRAM_MODE);
	out_be32(&ddr->sdram_mode_2, DDR_SDRAM_MODE_2);

	out_be32(&ddr->sdram_interval, DDR_SDRAM_INTERVAL);

	out_be32(&ddr->ddr_wrlvl_cntl, DDR_DDR_WRLVL_CNTL);

	out_be32(&ddr->ddr_wrlvl_cntl_2, DDR_DDR_WRLVL_CNTL_2);
	out_be32(&ddr->ddr_wrlvl_cntl_3, DDR_DDR_WRLVL_CNTL_3);

	out_be32(&ddr->ddr_cdr1, DDR_DDR_CDR1);
	out_be32(&ddr->ddr_cdr2, DDR_DDR_CDR2);

	out_be32(&ddr->sdram_clk_cntl, DDR_SDRAM_CLK_CNTL);
	out_be32(&ddr->ddr_zq_cntl, DDR_DDR_ZQ_CNTL);

	out_be32(&ddr->cs0_config_2, DDR_CS0_CONFIG_2);
	udelay(1);
	out_be32(&ddr->sdram_cfg, DDR_SDRAM_CFG | DDR_SDRAM_CFG_MEM_EN);
}

int dram_init(void)
{
#if (!defined(CONFIG_SPL) || defined(CONFIG_SPL_BUILD))
	ddrmc_init();
#endif

	gd->ram_size = get_ram_size((void *)PHYS_SDRAM, PHYS_SDRAM_SIZE);
	return 0;
}

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg esdhc_cfg[1] = {
	{CONFIG_SYS_FSL_ESDHC_ADDR},
};

int board_mmc_init(bd_t *bis)
{
	esdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);

	return fsl_esdhc_initialize(bis, &esdhc_cfg[0]);
}
#endif

#ifdef CONFIG_TSEC_ENET
int board_eth_init(bd_t *bis)
{
	struct fsl_pq_mdio_info mdio_info;
	struct tsec_info_struct tsec_info[4];
	int num = 0;

#ifdef CONFIG_TSEC1
	SET_STD_TSEC_INFO(tsec_info[num], 1);
	if (is_serdes_configured(SGMII_TSEC1)) {
		puts("eTSEC1 is in sgmii mode.\n");
		tsec_info[num].flags |= TSEC_SGMII;
	}
	num++;
#endif
#ifdef CONFIG_TSEC2
	SET_STD_TSEC_INFO(tsec_info[num], 2);
	if (is_serdes_configured(SGMII_TSEC2)) {
		puts("eTSEC2 is in sgmii mode.\n");
		tsec_info[num].flags |= TSEC_SGMII;
	}
	num++;
#endif
#ifdef CONFIG_TSEC3
	SET_STD_TSEC_INFO(tsec_info[num], 3);
	num++;
#endif
	if (!num) {
		printf("No TSECs initialized\n");
		return 0;
	}

	mdio_info.regs = (struct tsec_mii_mng *)CONFIG_SYS_MDIO_BASE_ADDR;
	mdio_info.name = DEFAULT_MII_NAME;
	fsl_pq_mdio_init(bis, &mdio_info);

	tsec_eth_init(bis, tsec_info, num);

	return pci_eth_init(bis);
}
#endif

int config_serdes_mux(void)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	u32 protocol = in_be32(&gur->rcwsr[4]) & RCWSR4_SRDS1_PRTCL_MASK;

	protocol >>= RCWSR4_SRDS1_PRTCL_SHIFT;
	switch (protocol) {
	case 0x10:
		convert_serdes_mux(LANEB_SATA, KEEP_STATUS);
		convert_serdes_mux(LANED_PCIEX2 |
				LANEC_PCIEX1, KEEP_STATUS);
		break;
	case 0x20:
		convert_serdes_mux(LANEB_SGMII1, KEEP_STATUS);
		convert_serdes_mux(LANEC_PCIEX1, KEEP_STATUS);
		convert_serdes_mux(LANED_SGMII2, KEEP_STATUS);
		break;
	case 0x30:
		convert_serdes_mux(LANEB_SATA, KEEP_STATUS);
		convert_serdes_mux(LANEC_SGMII1, KEEP_STATUS);
		convert_serdes_mux(LANED_SGMII2, KEEP_STATUS);
		break;
	case 0x70:
		convert_serdes_mux(LANEB_SATA, KEEP_STATUS);
		convert_serdes_mux(LANEC_PCIEX1, KEEP_STATUS);
		convert_serdes_mux(LANED_SGMII2, KEEP_STATUS);
		break;
	}

	return 0;
}

int board_early_init_f(void)
{
	struct ccsr_scfg *scfg = (struct ccsr_scfg *)CONFIG_SYS_FSL_SCFG_ADDR;

#ifdef CONFIG_TSEC_ENET
	out_be32(&scfg->scfgrevcr, SCFG_SCFGREVCR_REV);
	out_be32(&scfg->etsecdmamcr, SCFG_ETSECDMAMCR_LE_BD_FR);
	out_be32(&scfg->etsecmcr, SCFG_ETSECCMCR_GE2_CLK125);
	udelay(10);
	out_be32(&scfg->scfgrevcr, SCFG_SCFGREVCR_NOREV);
#endif

#ifdef CONFIG_FSL_IFC
	init_early_memctl_regs();
#endif

#ifdef CONFIG_FSL_DCU_FB
	out_be32(&scfg->scfgrevcr, SCFG_SCFGREVCR_REV);
	out_be32(&scfg->pixclkcr, SCFG_PIXCLKCR_PXCKEN);
	out_be32(&scfg->scfgrevcr, SCFG_SCFGREVCR_NOREV);
#endif

	return 0;
}

int board_init(void)
{
#ifndef CONFIG_SYS_FSL_NO_SERDES
	fsl_serdes_init();
	config_serdes_mux();
#endif

	return 0;
}

void ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
}

u8 flash_read8(void *addr)
{
	return __raw_readb(addr + 1);
}

void flash_write16(u16 val, void *addr)
{
	u16 shftval = (((val >> 8) & 0xff) | ((val << 8) & 0xff00));

	__raw_writew(shftval, addr);
}

u16 flash_read16(void *addr)
{
	u16 val = __raw_readw(addr);

	return (((val) >> 8) & 0x00ff) | (((val) << 8) & 0xff00);
}

static void convert_flash_bank(char bank)
{
	struct cpld_data *cpld_data = (void *)(CONFIG_SYS_CPLD_BASE);

	printf("Now switch to boot from flash bank %d.\n", bank);
	cpld_data->soft_mux_on = CPLD_SET_BOOT_BANK;
	cpld_data->vbank = bank;

	printf("Reset board to enable configuration.\n");
	cpld_data->system_rst = CONFIG_RESET;
}

static int flash_bank_cmd(cmd_tbl_t *cmdtp, int flag, int argc,
			  char * const argv[])
{
	if (argc != 2)
		return CMD_RET_USAGE;
	if (strcmp(argv[1], "0") == 0)
		convert_flash_bank(BOOT_FROM_UPPER_BANK);
	else if (strcmp(argv[1], "1") == 0)
		convert_flash_bank(BOOT_FROM_LOWER_BANK);
	else
		return CMD_RET_USAGE;

	return 0;
}

U_BOOT_CMD(
	boot_bank, 2, 0, flash_bank_cmd,
	"Flash bank Selection Control",
	"bank[0-upper bank/1-lower bank] (e.g. boot_bank 0)"
);

static int cpld_reset_cmd(cmd_tbl_t *cmdtp, int flag, int argc,
			  char * const argv[])
{
	struct cpld_data *cpld_data = (void *)(CONFIG_SYS_CPLD_BASE);

	if (argc > 2)
		return CMD_RET_USAGE;
	if ((argc == 1) || (strcmp(argv[1], "conf") == 0))
		cpld_data->system_rst = CONFIG_RESET;
	else if (strcmp(argv[1], "init") == 0)
		cpld_data->global_rst = INIT_RESET;
	else
		return CMD_RET_USAGE;

	return 0;
}

U_BOOT_CMD(
	cpld_reset, 2, 0, cpld_reset_cmd,
	"Reset via CPLD",
	"conf\n"
	"	-reset with current CPLD configuration\n"
	"init\n"
	"	-reset and initial CPLD configuration with default value"

);

static void convert_serdes_mux(int type, int need_reset)
{
	char current_serdes;
	struct cpld_data *cpld_data = (void *)(CONFIG_SYS_CPLD_BASE);

	current_serdes = cpld_data->serdes_mux;

	switch (type) {
	case LANEB_SATA:
		current_serdes &= ~MASK_LANE_B;
		break;
	case LANEB_SGMII1:
		current_serdes |= (MASK_LANE_B | MASK_SGMII | MASK_LANE_C);
		break;
	case LANEC_SGMII1:
		current_serdes &= ~(MASK_LANE_B | MASK_SGMII | MASK_LANE_C);
		break;
	case LANED_SGMII2:
		current_serdes |= MASK_LANE_D;
		break;
	case LANEC_PCIEX1:
		current_serdes |= MASK_LANE_C;
		break;
	case (LANED_PCIEX2 | LANEC_PCIEX1):
		current_serdes |= MASK_LANE_C;
		current_serdes &= ~MASK_LANE_D;
		break;
	default:
		printf("CPLD serdes MUX: unsupported MUX type 0x%x\n", type);
		return;
	}

	cpld_data->soft_mux_on |= CPLD_SET_MUX_SERDES;
	cpld_data->serdes_mux = current_serdes;

	if (need_reset == 1) {
		printf("Reset board to enable configuration\n");
		cpld_data->system_rst = CONFIG_RESET;
	}
}

void print_serdes_mux(void)
{
	char current_serdes;
	struct cpld_data *cpld_data = (void *)(CONFIG_SYS_CPLD_BASE);

	current_serdes = cpld_data->serdes_mux;

	printf("Serdes Lane B: ");
	if ((current_serdes & MASK_LANE_B) == 0)
		printf("SATA,\n");
	else
		printf("SGMII 1,\n");

	printf("Serdes Lane C: ");
	if ((current_serdes & MASK_LANE_C) == 0)
		printf("SGMII 1,\n");
	else
		printf("PCIe,\n");

	printf("Serdes Lane D: ");
	if ((current_serdes & MASK_LANE_D) == 0)
		printf("PCIe,\n");
	else
		printf("SGMII 2,\n");

	printf("SGMII 1 is on lane ");
	if ((current_serdes & MASK_SGMII) == 0)
		printf("C.\n");
	else
		printf("B.\n");
}

static int serdes_mux_cmd(cmd_tbl_t *cmdtp, int flag, int argc,
			  char * const argv[])
{
	if (argc != 2)
		return CMD_RET_USAGE;
	if (strcmp(argv[1], "sata") == 0) {
		printf("Set serdes lane B to SATA.\n");
		convert_serdes_mux(LANEB_SATA, NEED_RESET);
	} else if (strcmp(argv[1], "sgmii1b") == 0) {
		printf("Set serdes lane B to SGMII 1.\n");
		convert_serdes_mux(LANEB_SGMII1, NEED_RESET);
	} else if (strcmp(argv[1], "sgmii1c") == 0) {
		printf("Set serdes lane C to SGMII 1.\n");
		convert_serdes_mux(LANEC_SGMII1, NEED_RESET);
	} else if (strcmp(argv[1], "sgmii2") == 0) {
		printf("Set serdes lane D to SGMII 2.\n");
		convert_serdes_mux(LANED_SGMII2, NEED_RESET);
	} else if (strcmp(argv[1], "pciex1") == 0) {
		printf("Set serdes lane C to PCIe X1.\n");
		convert_serdes_mux(LANEC_PCIEX1, NEED_RESET);
	} else if (strcmp(argv[1], "pciex2") == 0) {
		printf("Set serdes lane C & lane D to PCIe X2.\n");
		convert_serdes_mux((LANED_PCIEX2 | LANEC_PCIEX1), NEED_RESET);
	} else if (strcmp(argv[1], "show") == 0) {
		print_serdes_mux();
	} else {
		return CMD_RET_USAGE;
	}

	return 0;
}

U_BOOT_CMD(
	lane_bank, 2, 0, serdes_mux_cmd,
	"Multiplexed function setting for SerDes Lanes",
	"sata\n"
	"	-change lane B to sata\n"
	"lane_bank sgmii1b\n"
	"	-change lane B to SGMII1\n"
	"lane_bank sgmii1c\n"
	"	-change lane C to SGMII1\n"
	"lane_bank sgmii2\n"
	"	-change lane D to SGMII2\n"
	"lane_bank pciex1\n"
	"	-change lane C to PCIeX1\n"
	"lane_bank pciex2\n"
	"	-change lane C & lane D to PCIeX2\n"
	"\nWARNING: If you aren't familiar with the setting of serdes, don't try to change anything!\n"
);
