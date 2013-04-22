/*
 * (C) Copyright 2012 Michal Simek <monstr@monstr.eu>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <netdev.h>
#include <zynqpl.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_FPGA
Xilinx_desc fpga;

/* It can be done differently */
Xilinx_desc fpga010 = XILINX_XC7Z010_DESC(0x10);
Xilinx_desc fpga020 = XILINX_XC7Z020_DESC(0x20);
Xilinx_desc fpga030 = XILINX_XC7Z030_DESC(0x30);
Xilinx_desc fpga045 = XILINX_XC7Z045_DESC(0x45);
#endif

int board_init(void)
{
#ifdef CONFIG_FPGA
	u32 idcode;

	idcode = zynq_slcr_get_idcode();

	switch (idcode) {
	case XILINX_ZYNQ_7010:
		fpga = fpga010;
		break;
	case XILINX_ZYNQ_7020:
		fpga = fpga020;
		break;
	case XILINX_ZYNQ_7030:
		fpga = fpga030;
		break;
	case XILINX_ZYNQ_7045:
		fpga = fpga045;
		break;
	}
#endif

	icache_enable();

#ifdef CONFIG_FPGA
	fpga_init();
	fpga_add(fpga_xilinx, &fpga);
#endif

	return 0;
}


#ifdef CONFIG_CMD_NET
int board_eth_init(bd_t *bis)
{
	u32 ret = 0;

#if defined(CONFIG_ZYNQ_GEM)
# if defined(CONFIG_ZYNQ_GEM0)
	ret |= zynq_gem_initialize(bis, ZYNQ_GEM_BASEADDR0,
						CONFIG_ZYNQ_GEM_PHY_ADDR0, 0);
# endif
# if defined(CONFIG_ZYNQ_GEM1)
	ret |= zynq_gem_initialize(bis, ZYNQ_GEM_BASEADDR1,
						CONFIG_ZYNQ_GEM_PHY_ADDR1, 0);
# endif
#endif
	return ret;
}
#endif

#ifdef CONFIG_CMD_MMC
int board_mmc_init(bd_t *bd)
{
	int ret = 0;

#if defined(CONFIG_ZYNQ_SDHCI)
# if defined(CONFIG_ZYNQ_SDHCI0)
	ret = zynq_sdhci_init(ZYNQ_SDHCI_BASEADDR0);
# endif
# if defined(CONFIG_ZYNQ_SDHCI1)
	ret |= zynq_sdhci_init(ZYNQ_SDHCI_BASEADDR1);
# endif
#endif
	return ret;
}
#endif

int dram_init(void)
{
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;

	return 0;
}
