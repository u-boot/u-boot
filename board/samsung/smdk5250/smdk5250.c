/*
 * Copyright (C) 2012 Samsung Electronics
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <netdev.h>
#include <asm/arch/cpu.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc.h>
#include <asm/arch/sromc.h>

DECLARE_GLOBAL_DATA_PTR;
struct exynos5_gpio_part1 *gpio1;

#ifdef CONFIG_SMC911X
static void smc9115_pre_init(void)
{
	u32 smc_bw_conf, smc_bc_conf;
	int i;

	/*
	 * SROM:CS1 and EBI
	 *
	 * GPY0[0]	SROM_CSn[0]
	 * GPY0[1]	SROM_CSn[1](2)
	 * GPY0[2]	SROM_CSn[2]
	 * GPY0[3]	SROM_CSn[3]
	 * GPY0[4]	EBI_OEn(2)
	 * GPY0[5]	EBI_EEn(2)
	 *
	 * GPY1[0]	EBI_BEn[0](2)
	 * GPY1[1]	EBI_BEn[1](2)
	 * GPY1[2]	SROM_WAIT(2)
	 * GPY1[3]	EBI_DATA_RDn(2)
	 */
	s5p_gpio_cfg_pin(&gpio1->y0, CONFIG_ENV_SROM_BANK, GPIO_FUNC(2));
	s5p_gpio_cfg_pin(&gpio1->y0, 4, GPIO_FUNC(2));
	s5p_gpio_cfg_pin(&gpio1->y0, 5, GPIO_FUNC(2));

	for (i = 0; i < 4; i++)
		s5p_gpio_cfg_pin(&gpio1->y1, i, GPIO_FUNC(2));

	/*
	 * EBI: 8 Addrss Lines
	 *
	 * GPY3[0]	EBI_ADDR[0](2)
	 * GPY3[1]	EBI_ADDR[1](2)
	 * GPY3[2]	EBI_ADDR[2](2)
	 * GPY3[3]	EBI_ADDR[3](2)
	 * GPY3[4]	EBI_ADDR[4](2)
	 * GPY3[5]	EBI_ADDR[5](2)
	 * GPY3[6]	EBI_ADDR[6](2)
	 * GPY3[7]	EBI_ADDR[7](2)
	 *
	 * EBI: 16 Data Lines
	 *
	 * GPY5[0]	EBI_DATA[0](2)
	 * GPY5[1]	EBI_DATA[1](2)
	 * GPY5[2]	EBI_DATA[2](2)
	 * GPY5[3]	EBI_DATA[3](2)
	 * GPY5[4]	EBI_DATA[4](2)
	 * GPY5[5]	EBI_DATA[5](2)
	 * GPY5[6]	EBI_DATA[6](2)
	 * GPY5[7]	EBI_DATA[7](2)
	 *
	 * GPY6[0]	EBI_DATA[8](2)
	 * GPY6[1]	EBI_DATA[9](2)
	 * GPY6[2]	EBI_DATA[10](2)
	 * GPY6[3]	EBI_DATA[11](2)
	 * GPY6[4]	EBI_DATA[12](2)
	 * GPY6[5]	EBI_DATA[13](2)
	 * GPY6[6]	EBI_DATA[14](2)
	 * GPY6[7]	EBI_DATA[15](2)
	 */
	for (i = 0; i < 8; i++) {
		s5p_gpio_cfg_pin(&gpio1->y3, i, GPIO_FUNC(2));
		s5p_gpio_set_pull(&gpio1->y3, i, GPIO_PULL_UP);

		s5p_gpio_cfg_pin(&gpio1->y5, i, GPIO_FUNC(2));
		s5p_gpio_set_pull(&gpio1->y5, i, GPIO_PULL_UP);

		s5p_gpio_cfg_pin(&gpio1->y6, i, GPIO_FUNC(2));
		s5p_gpio_set_pull(&gpio1->y6, i, GPIO_PULL_UP);
	}

	/* Ethernet needs data bus width of 16 bits */
	smc_bw_conf = SROMC_DATA16_WIDTH(CONFIG_ENV_SROM_BANK)
			| SROMC_BYTE_ENABLE(CONFIG_ENV_SROM_BANK);

	smc_bc_conf = SROMC_BC_TACS(0x01) | SROMC_BC_TCOS(0x01)
			| SROMC_BC_TACC(0x06) | SROMC_BC_TCOH(0x01)
			| SROMC_BC_TAH(0x0C)  | SROMC_BC_TACP(0x09)
			| SROMC_BC_PMC(0x01);

	/* Select and configure the SROMC bank */
	s5p_config_sromc(CONFIG_ENV_SROM_BANK, smc_bw_conf, smc_bc_conf);
}
#endif

int board_init(void)
{
	gpio1 = (struct exynos5_gpio_part1 *) samsung_get_base_gpio_part1();

	gd->bd->bi_boot_params = (PHYS_SDRAM_1 + 0x100UL);
	return 0;
}

int dram_init(void)
{
	gd->ram_size	= get_ram_size((long *)PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE)
			+ get_ram_size((long *)PHYS_SDRAM_2, PHYS_SDRAM_2_SIZE)
			+ get_ram_size((long *)PHYS_SDRAM_3, PHYS_SDRAM_3_SIZE)
			+ get_ram_size((long *)PHYS_SDRAM_4, PHYS_SDRAM_4_SIZE)
			+ get_ram_size((long *)PHYS_SDRAM_5, PHYS_SDRAM_7_SIZE)
			+ get_ram_size((long *)PHYS_SDRAM_6, PHYS_SDRAM_7_SIZE)
			+ get_ram_size((long *)PHYS_SDRAM_7, PHYS_SDRAM_7_SIZE)
			+ get_ram_size((long *)PHYS_SDRAM_8, PHYS_SDRAM_8_SIZE);
	return 0;
}

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = get_ram_size((long *)PHYS_SDRAM_1,
							PHYS_SDRAM_1_SIZE);
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = get_ram_size((long *)PHYS_SDRAM_2,
							PHYS_SDRAM_2_SIZE);
	gd->bd->bi_dram[2].start = PHYS_SDRAM_3;
	gd->bd->bi_dram[2].size = get_ram_size((long *)PHYS_SDRAM_3,
							PHYS_SDRAM_3_SIZE);
	gd->bd->bi_dram[3].start = PHYS_SDRAM_4;
	gd->bd->bi_dram[3].size = get_ram_size((long *)PHYS_SDRAM_4,
							PHYS_SDRAM_4_SIZE);
	gd->bd->bi_dram[4].start = PHYS_SDRAM_5;
	gd->bd->bi_dram[4].size = get_ram_size((long *)PHYS_SDRAM_5,
							PHYS_SDRAM_5_SIZE);
	gd->bd->bi_dram[5].start = PHYS_SDRAM_6;
	gd->bd->bi_dram[5].size = get_ram_size((long *)PHYS_SDRAM_6,
							PHYS_SDRAM_6_SIZE);
	gd->bd->bi_dram[6].start = PHYS_SDRAM_7;
	gd->bd->bi_dram[6].size = get_ram_size((long *)PHYS_SDRAM_7,
							PHYS_SDRAM_7_SIZE);
	gd->bd->bi_dram[7].start = PHYS_SDRAM_8;
	gd->bd->bi_dram[7].size = get_ram_size((long *)PHYS_SDRAM_8,
							PHYS_SDRAM_8_SIZE);
}

int board_eth_init(bd_t *bis)
{
#ifdef CONFIG_SMC911X
	smc9115_pre_init();
	return smc911x_initialize(0, CONFIG_SMC911X_BASE);
#endif
	return 0;
}

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	printf("\nBoard: SMDK5250\n");

	return 0;
}
#endif

#ifdef CONFIG_GENERIC_MMC
int board_mmc_init(bd_t *bis)
{
	int i, err;

	/*
	 * MMC2 SD card GPIO:
	 *
	 * GPC2[0]	SD_2_CLK(2)
	 * GPC2[1]	SD_2_CMD(2)
	 * GPC2[2]	SD_2_CDn
	 * GPC2[3:6]	SD_2_DATA[0:3](2)
	 */
	for (i = 0; i < 7; i++) {
		/* GPC2[0:6] special function 2 */
		s5p_gpio_cfg_pin(&gpio1->c2, i, GPIO_FUNC(0x2));

		/* GPK2[0:6] drv 4x */
		s5p_gpio_set_drv(&gpio1->c2, i, GPIO_DRV_4X);

		/* GPK2[0:1] pull disable */
		if (i == 0 || i == 1) {
			s5p_gpio_set_pull(&gpio1->c2, i, GPIO_PULL_NONE);
			continue;
		}

		/* GPK2[2:6] pull up */
		s5p_gpio_set_pull(&gpio1->c2, i, GPIO_PULL_UP);
	}

	err = s5p_mmc_init(2, 4);
	return err;
}
#endif

static void board_uart_init(void)
{
	struct exynos5_gpio_part1 *gpio1 =
		(struct exynos5_gpio_part1 *) samsung_get_base_gpio_part1();
	int i;

	/*
	 * UART0 GPIOs : GPA0CON[3:0] 0x2222
	 * Must set CFG17 switches to select UART0 to use.
	 */
	for (i = 0; i <= 3; i++) {
		s5p_gpio_set_pull(&gpio1->a0, i, GPIO_PULL_NONE);
		s5p_gpio_cfg_pin(&gpio1->a0, i, GPIO_FUNC(0x2));
	}

	/*
	 * UART1 GPIOs : GPA0CON[5:4] 0x22
	 * Must set CFG17 switches to select UART1 to use.
	 *
	 * This only sets RXD/TXD, as RTS/CTS need a resistor soldered down
	 * in order to use them (so that those pins can be used for I2C).
	 */
	for (i = 4; i <= 5; i++) {
		s5p_gpio_set_pull(&gpio1->a0, i, GPIO_PULL_NONE);
		s5p_gpio_cfg_pin(&gpio1->a0, i, GPIO_FUNC(0x2));
	}

	/*
	 * UART2 GPIOs : GPA1CON[1:0] 0x22
	 * Must set CFG17 switches to select UART2 to use.
	 *
	 * This only sets RXD/TXD, as RTS/CTS need a resistor soldered down
	 * in order to use them (so that those pins can be used for I2C).
	 */
	for (i = 0; i <= 1; i++) {
		s5p_gpio_set_pull(&gpio1->a1, i, GPIO_PULL_NONE);
		s5p_gpio_cfg_pin(&gpio1->a1, i, GPIO_FUNC(0x2));
	}

	/*
	 * UART3 GPIOs : GPA1CON[5:4] 0x22
	 * Must set CFG16 switches to select UART3 to use.
	 */
	for (i = 4; i <= 5; i++) {
		s5p_gpio_set_pull(&gpio1->a1, i, GPIO_PULL_NONE);
		s5p_gpio_cfg_pin(&gpio1->a1, i, GPIO_FUNC(0x2));
	}

	/*
	 * There's no mux for UART4--it's internal only
	 */
}

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
	board_uart_init();
	return 0;
}
#endif
