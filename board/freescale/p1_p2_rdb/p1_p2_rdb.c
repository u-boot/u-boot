/*
 * Copyright 2009-2011 Freescale Semiconductor, Inc.
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
#include <command.h>
#include <asm/processor.h>
#include <asm/mmu.h>
#include <asm/cache.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_serdes.h>
#include <asm/io.h>
#include <miiphy.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <fsl_mdio.h>
#include <tsec.h>
#include <vsc7385.h>
#include <netdev.h>
#include <rtc.h>
#include <i2c.h>
#include <hwconfig.h>

DECLARE_GLOBAL_DATA_PTR;

#define VSC7385_RST_SET		0x00080000
#define SLIC_RST_SET		0x00040000
#define SGMII_PHY_RST_SET	0x00020000
#define PCIE_RST_SET		0x00010000
#define RGMII_PHY_RST_SET	0x02000000

#define USB_RST_CLR		0x04000000
#define USB2_PORT_OUT_EN        0x01000000

#define GPIO_DIR		0x060f0000

#define BOARD_PERI_RST_SET	VSC7385_RST_SET | SLIC_RST_SET | \
				SGMII_PHY_RST_SET | PCIE_RST_SET | \
				RGMII_PHY_RST_SET

#define SYSCLK_MASK	0x00200000
#define BOARDREV_MASK	0x10100000
#define BOARDREV_C	0x00100000
#define BOARDREV_D	0x00000000

#define SYSCLK_66	66666666
#define SYSCLK_100	100000000

unsigned long get_board_sys_clk(ulong dummy)
{
	volatile ccsr_gpio_t *pgpio = (void *)(CONFIG_SYS_MPC85xx_GPIO_ADDR);
	u32 val_gpdat, sysclk_gpio;

	val_gpdat = in_be32(&pgpio->gpdat);
	sysclk_gpio = val_gpdat & SYSCLK_MASK;

	if(sysclk_gpio == 0)
		return SYSCLK_66;
	else
		return SYSCLK_100;

	return 0;
}

#ifdef CONFIG_MMC
int board_early_init_f (void)
{
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);

	setbits_be32(&gur->pmuxcr,
			(MPC85xx_PMUXCR_SDHC_CD |
			 MPC85xx_PMUXCR_SDHC_WP));
	return 0;
}
#endif

int checkboard (void)
{
	u32 val_gpdat, board_rev_gpio;
	volatile ccsr_gpio_t *pgpio = (void *)(CONFIG_SYS_MPC85xx_GPIO_ADDR);
	char board_rev = 0;
	struct cpu_type *cpu;

	val_gpdat = in_be32(&pgpio->gpdat);
	board_rev_gpio = val_gpdat & BOARDREV_MASK;
	if (board_rev_gpio == BOARDREV_C)
		board_rev = 'C';
	else if (board_rev_gpio == BOARDREV_D)
		board_rev = 'D';
	else
		panic ("Unexpected Board REV %x detected!!\n", board_rev_gpio);

	cpu = gd->cpu;
	printf ("Board: %sRDB Rev%c\n", cpu->name, board_rev);

	setbits_be32(&pgpio->gpdir, GPIO_DIR);

/*
 * Bringing the following peripherals out of reset via GPIOs
 * 0 = reset and 1 = out of reset
 * GPIO12 - Reset to Ethernet Switch
 * GPIO13 - Reset to SLIC/SLAC devices
 * GPIO14 - Reset to SGMII_PHY_N
 * GPIO15 - Reset to PCIe slots
 * GPIO6  - Reset to RGMII PHY
 * GPIO5  - Reset to USB3300 devices 1 = reset and 0 = out of reset
 */
	clrsetbits_be32(&pgpio->gpdat, USB_RST_CLR, BOARD_PERI_RST_SET);

	return 0;
}

int misc_init_r(void)
{
#if defined(CONFIG_SDCARD) || defined(CONFIG_SPIFLASH)
	ccsr_gur_t *gur = (void *)CONFIG_SYS_MPC85xx_GUTS_ADDR;
	ccsr_gpio_t *gpio = (void *)CONFIG_SYS_MPC85xx_GPIO_ADDR;

	setbits_be32(&gpio->gpdir, USB2_PORT_OUT_EN);
	setbits_be32(&gpio->gpdat, USB2_PORT_OUT_EN);
	setbits_be32(&gur->pmuxcr, MPC85xx_PMUXCR_ELBC_OFF_USB2_ON);
#endif
	return 0;
}

int board_early_init_r(void)
{
	const unsigned int flashbase = CONFIG_SYS_FLASH_BASE;
	const u8 flash_esel = find_tlb_idx((void *)flashbase, 1);
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	unsigned int orig_bus = i2c_get_bus_num();
	u8 i2c_data;

	i2c_set_bus_num(1);
	if (i2c_read(CONFIG_SYS_I2C_PCA9557_ADDR, 0,
		1, &i2c_data, sizeof(i2c_data)) == 0) {
		if (i2c_data & 0x2)
			puts("NOR Flash Bank : Secondary\n");
		else
			puts("NOR Flash Bank : Primary\n");

		if (i2c_data & 0x1) {
			setbits_be32(&gur->pmuxcr, MPC85xx_PMUXCR_SD_DATA);
			puts("SD/MMC : 8-bit Mode\n");
			puts("eSPI : Disabled\n");
		} else {
			puts("SD/MMC : 4-bit Mode\n");
			puts("eSPI : Enabled\n");
		}
	} else {
		puts("Failed reading I2C Chip 0x18 on bus 1\n");
	}
	i2c_set_bus_num(orig_bus);

	/*
	 * Remap Boot flash region to caching-inhibited
	 * so that flash can be erased properly.
	 */

	/* Flush d-cache and invalidate i-cache of any FLASH data */
	flush_dcache();
	invalidate_icache();

	/* invalidate existing TLB entry for flash */
	disable_tlb(flash_esel);

	set_tlb(1, flashbase, CONFIG_SYS_FLASH_BASE_PHYS,
			MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
			0, flash_esel, BOOKE_PAGESZ_16M, 1);
	rtc_reset();
	return 0;
}


#ifdef CONFIG_TSEC_ENET
int board_eth_init(bd_t *bis)
{
	struct fsl_pq_mdio_info mdio_info;
	struct tsec_info_struct tsec_info[4];
	int num = 0;
	char *tmp;
	unsigned int vscfw_addr;

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
		puts("eTSEC3 is in sgmii mode.\n");
		tsec_info[num].flags |= TSEC_SGMII;
	}
	num++;
#endif
	if (!num) {
		printf("No TSECs initialized\n");
		return 0;
	}
#ifdef CONFIG_VSC7385_ENET
/* If a VSC7385 microcode image is present, then upload it. */
	if ((tmp = getenv ("vscfw_addr")) != NULL) {
		vscfw_addr = simple_strtoul (tmp, NULL, 16);
		printf("uploading VSC7385 microcode from %x\n", vscfw_addr);
		if (vsc7385_upload_firmware((void *) vscfw_addr,
					CONFIG_VSC7385_IMAGE_SIZE))
			puts("Failure uploading VSC7385 microcode.\n");
	} else
		puts("No address specified for VSC7385 microcode.\n");
#endif

	mdio_info.regs = (struct tsec_mii_mng *)CONFIG_SYS_MDIO_BASE_ADDR;
	mdio_info.name = DEFAULT_MII_NAME;
	fsl_pq_mdio_init(bis, &mdio_info);

	tsec_eth_init(bis, tsec_info, num);

	return pci_eth_init(bis);
}
#endif

#if defined(CONFIG_OF_BOARD_SETUP)
extern void ft_pci_board_setup(void *blob);

void ft_board_setup(void *blob, bd_t *bd)
{
	const char *soc_usb_compat = "fsl-usb2-dr";
	int err, usb1_off, usb2_off;
	phys_addr_t base;
	phys_size_t size;

	ft_cpu_setup(blob, bd);

	base = getenv_bootm_low();
	size = getenv_bootm_size();

#if defined(CONFIG_PCI)
	ft_pci_board_setup(blob);
#endif /* #if defined(CONFIG_PCI) */

	fdt_fixup_memory(blob, (u64)base, (u64)size);

#if defined(CONFIG_HAS_FSL_DR_USB)
	fdt_fixup_dr_usb(blob, bd);
#endif

#if defined(CONFIG_SDCARD) || defined(CONFIG_SPIFLASH)
	/* Delete eLBC node as it is muxed with USB2 controller */
	if (hwconfig("usb2")) {
		const char *soc_elbc_compat = "fsl,p1020-elbc";
		int off = fdt_node_offset_by_compatible(blob, -1,
			soc_elbc_compat);
		if (off < 0) {
			printf("WARNING: could not find compatible node"
				" %s: %s.\n", soc_elbc_compat,
				fdt_strerror(off));
				return;
		}
		err = fdt_del_node(blob, off);
		if (err < 0) {
			printf("WARNING: could not remove %s: %s.\n",
				soc_elbc_compat, fdt_strerror(err));
		}
		return;
	}
#endif
	/* Delete USB2 node as it is muxed with eLBC */
	usb1_off = fdt_node_offset_by_compatible(blob, -1,
		soc_usb_compat);
	if (usb1_off < 0) {
		printf("WARNING: could not find compatible node"
			" %s: %s.\n", soc_usb_compat,
			fdt_strerror(usb1_off));
		return;
	}
	usb2_off = fdt_node_offset_by_compatible(blob, usb1_off,
			soc_usb_compat);
	if (usb2_off < 0) {
		printf("WARNING: could not find compatible node"
			" %s: %s.\n", soc_usb_compat,
			fdt_strerror(usb2_off));
		return;
	}
	err = fdt_del_node(blob, usb2_off);
	if (err < 0)
		printf("WARNING: could not remove %s: %s.\n",
			soc_usb_compat, fdt_strerror(err));
}
#endif
