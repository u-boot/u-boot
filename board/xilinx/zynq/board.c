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
#include <asm/io.h>
#include <asm/arch/mmc.h>
#include <asm/arch/nand.h>
#include <netdev.h>
#include <zynqpl.h>
#include <asm/arch/sys_proto.h>

#define BOOT_MODES_MASK    0x0000000F
#define QSPI_MODE         (0x00000001)            /**< QSPI */
#define NOR_FLASH_MODE    (0x00000002)            /**< NOR  */
#define NAND_FLASH_MODE   (0x00000004)            /**< NAND */
#define SD_MODE           (0x00000005)            /**< Secure Digital card */
#define JTAG_MODE	  (0x00000000)            /**< JTAG */

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_FPGA
Xilinx_desc fpga = XILINX_XC7Z020_DESC(0);
#endif

int board_init(void)
{
	/* temporary hack to clear pending irqs before Linux as it
	   will hang Linux */

	writel(0x26d, 0xe0001014);

	/* temporary hack to take USB out of reset til the is fixed
	   in Linux */

	writel(0x80, 0xe000a204);
	writel(0x80, 0xe000a208);
	writel(0x80, 0xe000a040);
	writel(0x00, 0xe000a040);
	writel(0x80, 0xe000a040);

	icache_enable();

#ifdef CONFIG_FPGA
	fpga_init();
	fpga_add(fpga_xilinx, &fpga);
#endif

	return 0;
}

int board_late_init (void)
{
	switch ((zynq_slcr_get_boot_mode()) & BOOT_MODES_MASK) {
	case QSPI_MODE:
		setenv("modeboot", "qspiboot");
		break;
	case NAND_FLASH_MODE:
		setenv("modeboot", "nandboot");
		break;
	case NOR_FLASH_MODE:
		setenv("modeboot", "norboot");
		break;
	case SD_MODE:
		setenv("modeboot", "sdboot");
		break;
	case JTAG_MODE:
		setenv("modeboot", "jtagboot");
		break;
	default:
		setenv("modeboot", "");
		break;
	}

	return 0;
}

#ifdef CONFIG_CMD_NET
int board_eth_init(bd_t *bis)
{
	u32 ret = 0;

#ifdef CONFIG_XILINX_AXIEMAC
	ret |= xilinx_axiemac_initialize(bis, XILINX_AXIEMAC_BASEADDR,
						XILINX_AXIDMA_BASEADDR);
#endif
#ifdef CONFIG_XILINX_EMACLITE
	u32 txpp = 0;
	u32 rxpp = 0;
# ifdef CONFIG_XILINX_EMACLITE_TX_PING_PONG
	txpp = 1;
# endif
# ifdef CONFIG_XILINX_EMACLITE_RX_PING_PONG
	rxpp = 1;
# endif
	ret |= xilinx_emaclite_initialize(bis, XILINX_EMACLITE_BASEADDR,
			txpp, rxpp);
#endif

#if defined(CONFIG_ZYNQ_GEM)
# if defined(CONFIG_ZYNQ_GEM_BASEADDR0)
	ret |= zynq_gem_initialize(bis, CONFIG_ZYNQ_GEM_BASEADDR0);
# endif
# if defined(CONFIG_ZYNQ_GEM_BASEADDR1)
	ret |= zynq_gem_initialize(bis, CONFIG_ZYNQ_GEM_BASEADDR1);
# endif
#endif
	return ret;
}
#endif

#ifdef CONFIG_CMD_MMC
int board_mmc_init(bd_t *bd)
{
	return zynq_mmc_init(bd);
}
#endif

#ifdef CONFIG_CMD_NAND
int board_nand_init(struct nand_chip *nand_chip)
{
	return zynq_nand_init(nand_chip);
}
#endif

int dram_init(void)
{
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;

	return 0;
}
