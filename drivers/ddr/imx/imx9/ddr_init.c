// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 NXP
 */

#include <common.h>
#include <errno.h>
#include <log.h>
#include <asm/io.h>
#include <asm/arch/ddr.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <linux/delay.h>

void ddrphy_coldreset(void)
{
	/* dramphy_apb_n default 1 , assert -> 0, de_assert -> 1 */
	/* dramphy_reset_n default 0 , assert -> 0, de_assert -> 1 */
	/* dramphy_PwrOKIn default 0 , assert -> 1, de_assert -> 0 */

	/* src_gen_dphy_apb_sw_rst_de_assert */
	clrbits_le32(REG_SRC_DPHY_SW_CTRL, BIT(0));
	/* src_gen_dphy_sw_rst_de_assert */
	clrbits_le32(REG_SRC_DPHY_SINGLE_RESET_SW_CTRL, BIT(2));
	/* src_gen_dphy_PwrOKIn_sw_rst_de_assert() */
	setbits_le32(REG_SRC_DPHY_SINGLE_RESET_SW_CTRL, BIT(0));
	mdelay(10);

	/* src_gen_dphy_apb_sw_rst_assert */
	setbits_le32(REG_SRC_DPHY_SW_CTRL, BIT(0));
	/* src_gen_dphy_sw_rst_assert */
	setbits_le32(REG_SRC_DPHY_SINGLE_RESET_SW_CTRL, BIT(2));
	mdelay(10);
	/* src_gen_dphy_PwrOKIn_sw_rst_assert */
	clrbits_le32(REG_SRC_DPHY_SINGLE_RESET_SW_CTRL, BIT(0));
	mdelay(10);

	/* src_gen_dphy_apb_sw_rst_de_assert */
	clrbits_le32(REG_SRC_DPHY_SW_CTRL, BIT(0));
	/* src_gen_dphy_sw_rst_de_assert() */
	clrbits_le32(REG_SRC_DPHY_SINGLE_RESET_SW_CTRL, BIT(2));
}

void check_ddrc_idle(void)
{
	u32 regval;

	do {
		regval = readl(REG_DDRDSR_2);
		if (regval & BIT(31))
			break;
	} while (1);
}

void check_dfi_init_complete(void)
{
	u32 regval;

	do {
		regval = readl(REG_DDRDSR_2);
		if (regval & BIT(2))
			break;
	} while (1);
	setbits_le32(REG_DDRDSR_2, BIT(2));
}

void ddrc_config(struct dram_cfg_param *ddrc_config, int num)
{
	int i = 0;

	for (i = 0; i < num; i++) {
		writel(ddrc_config->val, (ulong)ddrc_config->reg);
		ddrc_config++;
	}
}

void get_trained_CDD(u32 fsp)
{
}

int ddr_init(struct dram_timing_info *dram_timing)
{
	unsigned int initial_drate;
	int ret;
	u32 regval;

	debug("DDRINFO: start DRAM init\n");

	/* reset ddrphy */
	ddrphy_coldreset();

	debug("DDRINFO: cfg clk\n");

	initial_drate = dram_timing->fsp_msg[0].drate;
	/* default to the frequency point 0 clock */
	ddrphy_init_set_dfi_clk(initial_drate);

	/*
	 * Start PHY initialization and training by
	 * accessing relevant PUB registers
	 */
	debug("DDRINFO:ddrphy config start\n");

	ret = ddr_cfg_phy(dram_timing);
	if (ret)
		return ret;

	debug("DDRINFO: ddrphy config done\n");

	/* rogram the ddrc registers */
	debug("DDRINFO: ddrc config start\n");
	ddrc_config(dram_timing->ddrc_cfg, dram_timing->ddrc_cfg_num);
	debug("DDRINFO: ddrc config done\n");

#ifdef CONFIG_IMX9_DRAM_PM_COUNTER
	writel(0x200000, REG_DDR_DEBUG_19);
#endif

	check_dfi_init_complete();

	regval = readl(REG_DDR_SDRAM_CFG);
	writel((regval | 0x80000000), REG_DDR_SDRAM_CFG);

	check_ddrc_idle();

	/* save the dram timing config into memory */
	dram_config_save(dram_timing, CONFIG_SAVED_DRAM_TIMING_BASE);

	return 0;
}

ulong ddrphy_addr_remap(u32 paddr_apb_from_ctlr)
{
	u32 paddr_apb_qual;
	u32 paddr_apb_unqual_dec_22_13;
	u32 paddr_apb_unqual_dec_19_13;
	u32 paddr_apb_unqual_dec_12_1;
	u32 paddr_apb_unqual;
	u32 paddr_apb_phy;

	paddr_apb_qual = (paddr_apb_from_ctlr << 1);
	paddr_apb_unqual_dec_22_13 = ((paddr_apb_qual & 0x7fe000) >> 13);
	paddr_apb_unqual_dec_12_1  = ((paddr_apb_qual & 0x1ffe) >> 1);

	switch (paddr_apb_unqual_dec_22_13) {
	case 0x000:
		paddr_apb_unqual_dec_19_13 = 0x00;
		break;
	case 0x001:
		paddr_apb_unqual_dec_19_13 = 0x01;
		break;
	case 0x002:
		paddr_apb_unqual_dec_19_13 = 0x02;
		break;
	case 0x003:
		paddr_apb_unqual_dec_19_13 = 0x03;
		break;
	case 0x004:
		paddr_apb_unqual_dec_19_13 = 0x04;
		break;
	case 0x005:
		paddr_apb_unqual_dec_19_13 = 0x05;
		break;
	case 0x006:
		paddr_apb_unqual_dec_19_13 = 0x06;
		break;
	case 0x007:
		paddr_apb_unqual_dec_19_13 = 0x07;
		break;
	case 0x008:
		paddr_apb_unqual_dec_19_13 = 0x08;
		break;
	case 0x009:
		paddr_apb_unqual_dec_19_13 = 0x09;
		break;
	case 0x00a:
		paddr_apb_unqual_dec_19_13 = 0x0a;
		break;
	case 0x00b:
		paddr_apb_unqual_dec_19_13 = 0x0b;
		break;
	case 0x100:
		paddr_apb_unqual_dec_19_13 = 0x0c;
		break;
	case 0x101:
		paddr_apb_unqual_dec_19_13 = 0x0d;
		break;
	case 0x102:
		paddr_apb_unqual_dec_19_13 = 0x0e;
		break;
	case 0x103:
		paddr_apb_unqual_dec_19_13 = 0x0f;
		break;
	case 0x104:
		paddr_apb_unqual_dec_19_13 = 0x10;
		break;
	case 0x105:
		paddr_apb_unqual_dec_19_13 = 0x11;
		break;
	case 0x106:
		paddr_apb_unqual_dec_19_13 = 0x12;
		break;
	case 0x107:
		paddr_apb_unqual_dec_19_13 = 0x13;
		break;
	case 0x108:
		paddr_apb_unqual_dec_19_13 = 0x14;
		break;
	case 0x109:
		paddr_apb_unqual_dec_19_13 = 0x15;
		break;
	case 0x10a:
		paddr_apb_unqual_dec_19_13 = 0x16;
		break;
	case 0x10b:
		paddr_apb_unqual_dec_19_13 = 0x17;
		break;
	case 0x200:
		paddr_apb_unqual_dec_19_13 = 0x18;
		break;
	case 0x201:
		paddr_apb_unqual_dec_19_13 = 0x19;
		break;
	case 0x202:
		paddr_apb_unqual_dec_19_13 = 0x1a;
		break;
	case 0x203:
		paddr_apb_unqual_dec_19_13 = 0x1b;
		break;
	case 0x204:
		paddr_apb_unqual_dec_19_13 = 0x1c;
		break;
	case 0x205:
		paddr_apb_unqual_dec_19_13 = 0x1d;
		break;
	case 0x206:
		paddr_apb_unqual_dec_19_13 = 0x1e;
		break;
	case 0x207:
		paddr_apb_unqual_dec_19_13 = 0x1f;
		break;
	case 0x208:
		paddr_apb_unqual_dec_19_13 = 0x20;
		break;
	case 0x209:
		paddr_apb_unqual_dec_19_13 = 0x21;
		break;
	case 0x20a:
		paddr_apb_unqual_dec_19_13 = 0x22;
		break;
	case 0x20b:
		paddr_apb_unqual_dec_19_13 = 0x23;
		break;
	case 0x300:
		paddr_apb_unqual_dec_19_13 = 0x24;
		break;
	case 0x301:
		paddr_apb_unqual_dec_19_13 = 0x25;
		break;
	case 0x302:
		paddr_apb_unqual_dec_19_13 = 0x26;
		break;
	case 0x303:
		paddr_apb_unqual_dec_19_13 = 0x27;
		break;
	case 0x304:
		paddr_apb_unqual_dec_19_13 = 0x28;
		break;
	case 0x305:
		paddr_apb_unqual_dec_19_13 = 0x29;
		break;
	case 0x306:
		paddr_apb_unqual_dec_19_13 = 0x2a;
		break;
	case 0x307:
		paddr_apb_unqual_dec_19_13 = 0x2b;
		break;
	case 0x308:
		paddr_apb_unqual_dec_19_13 = 0x2c;
		break;
	case 0x309:
		paddr_apb_unqual_dec_19_13 = 0x2d;
		break;
	case 0x30a:
		paddr_apb_unqual_dec_19_13 = 0x2e;
		break;
	case 0x30b:
		paddr_apb_unqual_dec_19_13 = 0x2f;
		break;
	case 0x010:
		paddr_apb_unqual_dec_19_13 = 0x30;
		break;
	case 0x011:
		paddr_apb_unqual_dec_19_13 = 0x31;
		break;
	case 0x012:
		paddr_apb_unqual_dec_19_13 = 0x32;
		break;
	case 0x013:
		paddr_apb_unqual_dec_19_13 = 0x33;
		break;
	case 0x014:
		paddr_apb_unqual_dec_19_13 = 0x34;
		break;
	case 0x015:
		paddr_apb_unqual_dec_19_13 = 0x35;
		break;
	case 0x016:
		paddr_apb_unqual_dec_19_13 = 0x36;
		break;
	case 0x017:
		paddr_apb_unqual_dec_19_13 = 0x37;
		break;
	case 0x018:
		paddr_apb_unqual_dec_19_13 = 0x38;
		break;
	case 0x019:
		paddr_apb_unqual_dec_19_13 = 0x39;
		break;
	case 0x110:
		paddr_apb_unqual_dec_19_13 = 0x3a;
		break;
	case 0x111:
		paddr_apb_unqual_dec_19_13 = 0x3b;
		break;
	case 0x112:
		paddr_apb_unqual_dec_19_13 = 0x3c;
		break;
	case 0x113:
		paddr_apb_unqual_dec_19_13 = 0x3d;
		break;
	case 0x114:
		paddr_apb_unqual_dec_19_13 = 0x3e;
		break;
	case 0x115:
		paddr_apb_unqual_dec_19_13 = 0x3f;
		break;
	case 0x116:
		paddr_apb_unqual_dec_19_13 = 0x40;
		break;
	case 0x117:
		paddr_apb_unqual_dec_19_13 = 0x41;
		break;
	case 0x118:
		paddr_apb_unqual_dec_19_13 = 0x42;
		break;
	case 0x119:
		paddr_apb_unqual_dec_19_13 = 0x43;
		break;
	case 0x210:
		paddr_apb_unqual_dec_19_13 = 0x44;
		break;
	case 0x211:
		paddr_apb_unqual_dec_19_13 = 0x45;
		break;
	case 0x212:
		paddr_apb_unqual_dec_19_13 = 0x46;
		break;
	case 0x213:
		paddr_apb_unqual_dec_19_13 = 0x47;
		break;
	case 0x214:
		paddr_apb_unqual_dec_19_13 = 0x48;
		break;
	case 0x215:
		paddr_apb_unqual_dec_19_13 = 0x49;
		break;
	case 0x216:
		paddr_apb_unqual_dec_19_13 = 0x4a;
		break;
	case 0x217:
		paddr_apb_unqual_dec_19_13 = 0x4b;
		break;
	case 0x218:
		paddr_apb_unqual_dec_19_13 = 0x4c;
		break;
	case 0x219:
		paddr_apb_unqual_dec_19_13 = 0x4d;
		break;
	case 0x310:
		paddr_apb_unqual_dec_19_13 = 0x4e;
		break;
	case 0x311:
		paddr_apb_unqual_dec_19_13 = 0x4f;
		break;
	case 0x312:
		paddr_apb_unqual_dec_19_13 = 0x50;
		break;
	case 0x313:
		paddr_apb_unqual_dec_19_13 = 0x51;
		break;
	case 0x314:
		paddr_apb_unqual_dec_19_13 = 0x52;
		break;
	case 0x315:
		paddr_apb_unqual_dec_19_13 = 0x53;
		break;
	case 0x316:
		paddr_apb_unqual_dec_19_13 = 0x54;
		break;
	case 0x317:
		paddr_apb_unqual_dec_19_13 = 0x55;
		break;
	case 0x318:
		paddr_apb_unqual_dec_19_13 = 0x56;
		break;
	case 0x319:
		paddr_apb_unqual_dec_19_13 = 0x57;
		break;
	case 0x020:
		paddr_apb_unqual_dec_19_13 = 0x58;
		break;
	case 0x120:
		paddr_apb_unqual_dec_19_13 = 0x59;
		break;
	case 0x220:
		paddr_apb_unqual_dec_19_13 = 0x5a;
		break;
	case 0x320:
		paddr_apb_unqual_dec_19_13 = 0x5b;
		break;
	case 0x040:
		paddr_apb_unqual_dec_19_13 = 0x5c;
		break;
	case 0x140:
		paddr_apb_unqual_dec_19_13 = 0x5d;
		break;
	case 0x240:
		paddr_apb_unqual_dec_19_13 = 0x5e;
		break;
	case 0x340:
		paddr_apb_unqual_dec_19_13 = 0x5f;
		break;
	case 0x050:
		paddr_apb_unqual_dec_19_13 = 0x60;
		break;
	case 0x051:
		paddr_apb_unqual_dec_19_13 = 0x61;
		break;
	case 0x052:
		paddr_apb_unqual_dec_19_13 = 0x62;
		break;
	case 0x053:
		paddr_apb_unqual_dec_19_13 = 0x63;
		break;
	case 0x054:
		paddr_apb_unqual_dec_19_13 = 0x64;
		break;
	case 0x055:
		paddr_apb_unqual_dec_19_13 = 0x65;
		break;
	case 0x056:
		paddr_apb_unqual_dec_19_13 = 0x66;
		break;
	case 0x057:
		paddr_apb_unqual_dec_19_13 = 0x67;
		break;
	case 0x070:
		paddr_apb_unqual_dec_19_13 = 0x68;
		break;
	case 0x090:
		paddr_apb_unqual_dec_19_13 = 0x69;
		break;
	case 0x190:
		paddr_apb_unqual_dec_19_13 = 0x6a;
		break;
	case 0x290:
		paddr_apb_unqual_dec_19_13 = 0x6b;
		break;
	case 0x390:
		paddr_apb_unqual_dec_19_13 = 0x6c;
		break;
	case 0x0c0:
		paddr_apb_unqual_dec_19_13 = 0x6d;
		break;
	case 0x0d0:
		paddr_apb_unqual_dec_19_13 = 0x6e;
		break;
	default:
		paddr_apb_unqual_dec_19_13 = 0x00;
		break;
	}

	paddr_apb_unqual = ((paddr_apb_unqual_dec_19_13 << 13) | (paddr_apb_unqual_dec_12_1 << 1));

	paddr_apb_phy = (paddr_apb_unqual << 1);

	return paddr_apb_phy;
}
