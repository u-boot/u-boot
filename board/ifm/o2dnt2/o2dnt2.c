/*
 * Partially derived from board code for digsyMTC,
 * (C) Copyright 2009
 * Grzegorz Bernacki, Semihalf, gjb@semihalf.com
 *
 * (C) Copyright 2012
 * DENX Software Engineering, Anatolij Gustschin <agust@denx.de>
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
#include <mpc5xxx.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <i2c.h>
#include <miiphy.h>
#include <net.h>
#include <pci.h>

DECLARE_GLOBAL_DATA_PTR;

#define SDRAM_MODE	0x00CD0000
#define SDRAM_CONTROL	0x504F0000
#define SDRAM_CONFIG1	0xD2322800
#define SDRAM_CONFIG2	0x8AD70000

enum ifm_sensor_type {
	O2DNT		= 0x00,	/* !< O2DNT 32MB */
	O2DNT2		= 0x01,	/* !< O2DNT2 64MB */
	O3DNT		= 0x02,	/* !< O3DNT 32MB */
	O3DNT_MIN	= 0x40,	/* !< O3DNT Minerva 32MB */
	UNKNOWN		= 0xff,	/* !< Unknow sensor */
};

static enum ifm_sensor_type gt_ifm_sensor_type;

#ifndef CONFIG_SYS_RAMBOOT
static void sdram_start(int hi_addr)
{
	struct mpc5xxx_sdram *sdram = (struct mpc5xxx_sdram *)MPC5XXX_SDRAM;
	long hi_addr_bit = hi_addr ? 0x01000000 : 0;
	long control = SDRAM_CONTROL | hi_addr_bit;

	/* unlock mode register */
	out_be32(&sdram->ctrl, control | 0x80000000);

	/* precharge all banks */
	out_be32(&sdram->ctrl, control | 0x80000002);

	/* auto refresh */
	out_be32(&sdram->ctrl, control | 0x80000004);

	/* set mode register */
	out_be32(&sdram->mode, SDRAM_MODE);

	/* normal operation */
	out_be32(&sdram->ctrl, control);
}
#endif

/*
 * ATTENTION: Although partially referenced initdram does NOT make real use
 *            use of CONFIG_SYS_SDRAM_BASE. The code does not work if
 *            CONFIG_SYS_SDRAM_BASE is something else than 0x00000000.
 */
phys_size_t initdram(int board_type)
{
	struct mpc5xxx_mmap_ctl *mmap_ctl =
		(struct mpc5xxx_mmap_ctl *)CONFIG_SYS_MBAR;
	struct mpc5xxx_sdram *sdram = (struct mpc5xxx_sdram *)MPC5XXX_SDRAM;
	ulong dramsize = 0;
	ulong dramsize2 = 0;
	uint svr, pvr;

	if (gt_ifm_sensor_type == O2DNT2) {
		/* activate SDRAM CS1 */
		setbits_be32((void *)MPC5XXX_GPS_PORT_CONFIG, 0x80000000);
	}

#ifndef CONFIG_SYS_RAMBOOT
	ulong test1, test2;

	/* setup SDRAM chip selects */
	out_be32(&mmap_ctl->sdram0, 0x0000001E); /* 2 GB at 0x0 */
	out_be32(&mmap_ctl->sdram1, 0x00000000); /* disabled */

	/* setup config registers */
	out_be32(&sdram->config1, SDRAM_CONFIG1);
	out_be32(&sdram->config2, SDRAM_CONFIG2);

	/* find RAM size using SDRAM CS0 only */
	sdram_start(0);
	test1 = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE, 0x08000000);
	sdram_start(1);
	test2 = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE, 0x08000000);
	if (test1 > test2) {
		sdram_start(0);
		dramsize = test1;
	} else {
		dramsize = test2;
	}

	/* memory smaller than 1MB is impossible */
	if (dramsize < (1 << 20))
		dramsize = 0;

	/* set SDRAM CS0 size according to the amount of RAM found */
	if (dramsize > 0) {
		out_be32(&mmap_ctl->sdram0,
			 (0x13 + __builtin_ffs(dramsize >> 20) - 1));
	} else {
		out_be32(&mmap_ctl->sdram0, 0); /* disabled */
	}

	/* let SDRAM CS1 start right after CS0 */
	out_be32(&mmap_ctl->sdram1, dramsize + 0x0000001E); /* 2G */

	/* find RAM size using SDRAM CS1 only */
	if (!dramsize)
		sdram_start(0);

	test2 = test1 = get_ram_size((long *)(CONFIG_SYS_SDRAM_BASE + dramsize),
					0x80000000);
	if (!dramsize) {
		sdram_start(1);
		test2 = get_ram_size((long *)(CONFIG_SYS_SDRAM_BASE + dramsize),
					0x80000000);
	}

	if (test1 > test2) {
		sdram_start(0);
		dramsize2 = test1;
	} else {
		dramsize2 = test2;
	}

	/* memory smaller than 1MB is impossible */
	if (dramsize2 < (1 << 20))
		dramsize2 = 0;

	/* set SDRAM CS1 size according to the amount of RAM found */
	if (dramsize2 > 0) {
		out_be32(&mmap_ctl->sdram1, (dramsize |
			 (0x13 + __builtin_ffs(dramsize2 >> 20) - 1)));
	} else {
		out_be32(&mmap_ctl->sdram1, dramsize); /* disabled */
	}

#else /* CONFIG_SYS_RAMBOOT */
	/* retrieve size of memory connected to SDRAM CS0 */
	dramsize = in_be32(&mmap_ctl->sdram0) & 0xFF;
	if (dramsize >= 0x13)
		dramsize = (1 << (dramsize - 0x13)) << 20;
	else
		dramsize = 0;

	/* retrieve size of memory connected to SDRAM CS1 */
	dramsize2 = in_be32(&mmap_ctl->sdram1) & 0xFF;
	if (dramsize2 >= 0x13)
		dramsize2 = (1 << (dramsize2 - 0x13)) << 20;
	else
		dramsize2 = 0;

#endif /* CONFIG_SYS_RAMBOOT */

	/*
	 * On MPC5200B we need to set the special configuration delay in the
	 * DDR controller. Please refer to Freescale's AN3221 "MPC5200B SDRAM
	 * Initialization and Configuration", 3.3.1 SDelay--MBAR + 0x0190:
	 *
	 * "The SDelay should be written to a value of 0x00000004. It is
	 * required to account for changes caused by normal wafer processing
	 * parameters."
	 */
	svr = get_svr();
	pvr = get_pvr();
	if ((SVR_MJREV(svr) >= 2) &&
	    (PVR_MAJ(pvr) == 1) && (PVR_MIN(pvr) == 4))
		out_be32(&sdram->sdelay, 0x04);

	return dramsize + dramsize2;
}


#define GPT_GPIO_IN	0x4

int checkboard(void)
{
	struct mpc5xxx_gpt *gpt = (struct mpc5xxx_gpt *)MPC5XXX_GPT;
	unsigned char board_config = 0;
	int i;

	/* switch gpt0 - gpt7 to input */
	for (i = 0; i < 7; i++)
		out_be32(&gpt[i].emsr, GPT_GPIO_IN);

	/* get configuration byte on timer-port */
	for (i = 0; i < 7; i++)
		board_config |= (in_be32(&gpt[i].sr) & 0x100) >> (8 - i);

	puts("Board: ");

	switch (board_config) {
	case 0:
		puts("O2DNT\n");
		gt_ifm_sensor_type = O2DNT;
		break;
	case 1:
		puts("O3DNT\n");
		gt_ifm_sensor_type = O3DNT;
		break;
	case 2:
		puts("O2DNT2\n");
		gt_ifm_sensor_type = O2DNT2;
		break;
	case 64:
		puts("O3DNT Minerva\n");
		gt_ifm_sensor_type = O3DNT_MIN;
		break;
	default:
		puts("Unknown\n");
		gt_ifm_sensor_type = UNKNOWN;
		break;
	}

	return 0;
}

int board_early_init_r(void)
{
	struct mpc5xxx_lpb *lpb_regs = (struct mpc5xxx_lpb *)MPC5XXX_LPB;

	/*
	 * Now, when we are in RAM, enable flash write access for detection
	 * process. Note that CS_BOOT cannot be cleared when executing in flash.
	 */
	clrbits_be32(&lpb_regs->cs0_cfg, 1); /* clear RO */
	/* disable CS_BOOT */
	clrbits_be32((void *)MPC5XXX_ADDECR, (1 << 25));
	/* enable CS0 */
	setbits_be32((void *)MPC5XXX_ADDECR, (1 << 16));

	return 0;
}

#define MIIM_LXT971_LED_CFG_REG		0x14
#define LXT971_LED_CFG_LINK_STATUS	0x4000
#define LXT971_LED_CFG_RX_TX_ACTIVITY	0x0700
#define LXT971_LED_CFG_LINK_ACTIVITY	0x00D0
#define LXT971_LED_CFG_PULSE_STRETCH	0x0002
/*
 * Additional PHY intialization after reset in mpc5xxx_fec_init_phy()
 */
void reset_phy(void)
{
	/*
	 * Set LED configuration bits.
	 * It can't be done in misc_init_r() since FEC is not
	 * initialized at this time. Therefore we do it here.
	 */
	miiphy_write("FEC", CONFIG_PHY_ADDR, MIIM_LXT971_LED_CFG_REG,
			LXT971_LED_CFG_LINK_STATUS |
			LXT971_LED_CFG_RX_TX_ACTIVITY |
			LXT971_LED_CFG_LINK_ACTIVITY |
			LXT971_LED_CFG_PULSE_STRETCH);
}

#if defined(CONFIG_POST)
/*
 * Reads GPIO pin PSC6_3. A keypress is reported, if PSC6_3 is low. If PSC6_3
 * is left open, no keypress is detected.
 */
int post_hotkeys_pressed(void)
{
	struct mpc5xxx_gpio *gpio = (struct mpc5xxx_gpio *) MPC5XXX_GPIO;

	/*
	 * Configure PSC6_1 and PSC6_3 as GPIO. PSC6 then couldn't be used in
	 * CODEC or UART mode. Consumer IrDA should still be possible.
	 */
	clrbits_be32(&gpio->port_config, 0x07000000);
	setbits_be32(&gpio->port_config, 0x03000000);

	/* Enable GPIO for GPIO_IRDA_1 (IR_USB_CLK pin) = PSC6_3 */
	setbits_be32(&gpio->simple_gpioe, 0x20000000);

	/* Configure GPIO_IRDA_1 as input */
	clrbits_be32(&gpio->simple_ddr, 0x20000000);

	return (in_be32(&gpio->simple_ival) & 0x20000000) ? 0 : 1;
}
#endif

#ifdef CONFIG_PCI
static struct pci_controller hose;

void pci_init_board(void)
{
	pci_mpc5xxx_init(&hose);
}
#endif

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
#if defined(CONFIG_SYS_UPDATE_FLASH_SIZE)
static void ft_adapt_flash_base(void *blob)
{
	flash_info_t	*dev = &flash_info[0];
	int off;
	struct fdt_property *prop;
	int len;
	u32 *reg, *reg2;

	off = fdt_node_offset_by_compatible(blob, -1, "fsl,mpc5200b-lpb");
	if (off < 0) {
		printf("Could not find fsl,mpc5200b-lpb node.\n");
		return;
	}

	/* found compatible property */
	prop = fdt_get_property_w(blob, off, "ranges", &len);
	if (prop) {
		reg = reg2 = (u32 *)&prop->data[0];

		reg[2] = dev->start[0];
		reg[3] = dev->size;
		fdt_setprop(blob, off, "ranges", reg2, len);
	} else
		printf("Could not find ranges\n");
}

extern ulong flash_get_size(phys_addr_t base, int banknum);

/* Update the flash baseaddr settings */
int update_flash_size(int flash_size)
{
	struct mpc5xxx_mmap_ctl *mm =
		(struct mpc5xxx_mmap_ctl *) CONFIG_SYS_MBAR;
	flash_info_t *dev;
	int i;
	int size = 0;
	unsigned long base = 0x0;
	u32 *cs_reg = (u32 *)&mm->cs0_start;

	for (i = 0; i < 2; i++) {
		dev = &flash_info[i];

		if (dev->size) {
			/* calculate new base addr for this chipselect */
			base -= dev->size;
			out_be32(cs_reg, START_REG(base));
			cs_reg++;
			out_be32(cs_reg, STOP_REG(base, dev->size));
			cs_reg++;
			/* recalculate the sectoraddr in the cfi driver */
			size += flash_get_size(base, i);
		}
	}
	flash_protect_default();
	gd->bd->bi_flashstart = base;
	return 0;
}
#endif /* defined(CONFIG_SYS_UPDATE_FLASH_SIZE) */

void ft_board_setup(void *blob, bd_t *bd)
{
	int phy_addr = CONFIG_PHY_ADDR;
	char eth_path[] = "/soc5200@f0000000/mdio@3000/ethernet-phy@0";

	ft_cpu_setup(blob, bd);

#if defined(CONFIG_SYS_UPDATE_FLASH_SIZE)
#ifdef CONFIG_FDT_FIXUP_NOR_FLASH_SIZE
	/* Update reg property in all nor flash nodes too */
	fdt_fixup_nor_flash_size(blob);
#endif
	ft_adapt_flash_base(blob);
#endif
	/* fix up the phy address */
	do_fixup_by_path(blob, eth_path, "reg", &phy_addr, sizeof(int), 0);
}
#endif /* defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP) */
