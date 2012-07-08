/*
 * Copyright 2009,2012 Freescale Semiconductor, Inc.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <hwconfig.h>
#include <command.h>
#include <asm/processor.h>
#include <asm/mmu.h>
#include <asm/cache.h>
#include <asm/immap_85xx.h>
#include <asm/mpc85xx_gpio.h>
#include <asm/fsl_serdes.h>
#include <asm/io.h>
#include <miiphy.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <fsl_mdio.h>
#include <tsec.h>
#include <vsc7385.h>
#include <netdev.h>
#include <mmc.h>
#include <malloc.h>
#include <i2c.h>

#if defined(CONFIG_PCI)
#include <asm/fsl_pci.h>
#include <pci.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_PCI)
void pci_init_board(void)
{
	fsl_pcie_init_board(0);
}

void ft_pci_board_setup(void *blob)
{
	FT_FSL_PCI_SETUP;
}
#endif

#define BOARD_PERI_RST_SET	(VSC7385_RST_SET | SLIC_RST_SET | \
				 SGMII_PHY_RST_SET | PCIE_RST_SET | \
				 RGMII_PHY_RST_SET)

#define SYSCLK_MASK	0x00200000
#define BOARDREV_MASK	0x10100000
#define BOARDREV_B	0x10100000
#define BOARDREV_C	0x00100000
#define BOARDREV_D	0x00000000

#define SYSCLK_66	66666666
#define SYSCLK_50	50000000
#define SYSCLK_100	100000000

unsigned long get_board_sys_clk(ulong dummy)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 ddr_ratio = in_be32(&gur->porpllsr) & MPC85xx_PORPLLSR_DDR_RATIO;

	ddr_ratio >>= MPC85xx_PORPLLSR_DDR_RATIO_SHIFT;
	switch (ddr_ratio) {
	case 0x0C:
		return SYSCLK_66;
	case 0x0A:
	case 0x08:
		return SYSCLK_100;
	default:
		puts("ERROR: unknown DDR ratio\n");
		return SYSCLK_100;
	}
}

unsigned long get_board_ddr_clk(ulong dummy)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 ddr_ratio = in_be32(&gur->porpllsr) & MPC85xx_PORPLLSR_DDR_RATIO;

	ddr_ratio >>= MPC85xx_PORPLLSR_DDR_RATIO_SHIFT;
	switch (ddr_ratio) {
	case 0x0C:
	case 0x0A:
		return SYSCLK_66;
	case 0x08:
		return SYSCLK_100;
	default:
		puts("ERROR: unknown DDR ratio\n");
		return SYSCLK_100;
	}
}

#ifdef CONFIG_MMC
int board_early_init_f(void)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);

	setbits_be32(&gur->pmuxcr,
			(MPC85xx_PMUXCR_SDHC_CD |
			 MPC85xx_PMUXCR_SDHC_WP));

	/* All the device are enable except for SRIO12 */
	setbits_be32(&gur->devdisr, MPC85xx_DEVDISR_SRIO);
	return 0;
}
#endif

#define GPIO_DIR		0x0f3a0000
#define GPIO_ODR		0x00000000
#define GPIO_DAT		0x001a0000

int checkboard(void)
{
	ccsr_gpio_t *pgpio = (void *)(CONFIG_SYS_MPC85xx_GPIO_ADDR + 0xC00);

	/*
	 * GPIO
	 * 0 - 3: CarryBoard Input;
	 * 4 - 7: CarryBoard Output;
	 * 8 : Mux as SDHC_CD (card detection)
	 * 9 : Mux as SDHC_WP
	 * 10 : Clear Watchdog timer
	 * 11 : LED Input
	 * 12 : Output to 1
	 * 13 : Open Drain
	 * 14 : LED Output
	 * 15 : Switch Input
	 *
	 * Set GPIOs 11, 12, 14 to 1.
	 */
	out_be32(&pgpio->gpodr, GPIO_ODR);
	mpc85xx_gpio_set(0xffffffff, GPIO_DIR, GPIO_DAT);

	puts("Board: Freescale COM Express P2020\n");
	return 0;
}

#define M41ST85W_I2C_BUS	1
#define M41ST85W_I2C_ADDR	0x68
#define M41ST85W_ERROR(fmt, args...) printf("ERROR: M41ST85W: " fmt, ##args)

static void m41st85w_clear_bit(u8 reg, u8 mask, const char *name)
{
	u8 data;

	if (i2c_read(M41ST85W_I2C_ADDR, reg, 1, &data, 1)) {
		M41ST85W_ERROR("unable to read %s bit\n", name);
		return;
	}

	if (data & mask) {
		data &= ~mask;
		if (i2c_write(M41ST85W_I2C_ADDR, reg, 1, &data, 1)) {
			M41ST85W_ERROR("unable to clear %s bit\n", name);
			return;
		}
	}
}

#define M41ST85W_REG_SEC2	0x01
#define M41ST85W_REG_SEC2_ST	0x80

#define M41ST85W_REG_ALHOUR	0x0c
#define M41ST85W_REG_ALHOUR_HT	0x40

/*
 * The P2020COME board has a STMicro M41ST85W RTC/watchdog
 * at i2c bus 1 address 0x68.
 */
static void start_rtc(void)
{
	unsigned int bus = i2c_get_bus_num();

	if (i2c_set_bus_num(M41ST85W_I2C_BUS)) {
		M41ST85W_ERROR("unable to set i2c bus\n");
		goto out;
	}

	/* ensure ST (stop) and HT (halt update) bits are cleared */
	m41st85w_clear_bit(M41ST85W_REG_SEC2, M41ST85W_REG_SEC2_ST, "ST");
	m41st85w_clear_bit(M41ST85W_REG_ALHOUR, M41ST85W_REG_ALHOUR_HT, "HT");

out:
	/* reset the i2c bus */
	i2c_set_bus_num(bus);
}

int board_early_init_r(void)
{
	start_rtc();
	return 0;
}

#define M41ST85W_REG_WATCHDOG		0x09
#define M41ST85W_REG_WATCHDOG_WDS	0x80
#define M41ST85W_REG_WATCHDOG_BMB0	0x04

void board_reset(void)
{
	u8 data = M41ST85W_REG_WATCHDOG_WDS | M41ST85W_REG_WATCHDOG_BMB0;

	/* set the hardware watchdog timeout to 1/16 second, then hang */
	i2c_set_bus_num(M41ST85W_I2C_BUS);
	i2c_write(M41ST85W_I2C_ADDR, M41ST85W_REG_WATCHDOG, 1, &data, 1);

	while (1)
		/* hang */;
}

#ifdef CONFIG_TSEC_ENET
int board_eth_init(bd_t *bis)
{
	struct fsl_pq_mdio_info mdio_info;
	struct tsec_info_struct tsec_info[4];
	int num = 0;

#ifdef CONFIG_TSEC1
	SET_STD_TSEC_INFO(tsec_info[num], 1);
	num++;
#endif
#ifdef CONFIG_TSEC2
	SET_STD_TSEC_INFO(tsec_info[num], 2);
	num++;
#endif
#ifdef CONFIG_TSEC3
	SET_STD_TSEC_INFO(tsec_info[num], 3);
	if (is_serdes_configured(SGMII_TSEC3)) {
		puts("eTSEC3 is in sgmii mode.");
		tsec_info[num].flags |= TSEC_SGMII;
	}
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

#if defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t *bd)
{
	phys_addr_t base;
	phys_size_t size;

	ft_cpu_setup(blob, bd);

	base = getenv_bootm_low();
	size = getenv_bootm_size();

#if defined(CONFIG_PCI)
	ft_pci_board_setup(blob);
#endif

	fdt_fixup_memory(blob, (u64)base, (u64)size);

#ifdef CONFIG_HAS_FSL_DR_USB
	fdt_fixup_dr_usb(blob, bd);
#endif
}
#endif
