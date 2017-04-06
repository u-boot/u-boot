/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2004
 * Mark Jonas, Freescale Semiconductor, mark.jonas@motorola.com.
 *
 * (C) Copyright 2005-2009
 * Modified for InterControl digsyMTC MPC5200 board by
 * Frank Bodammer, GCD Hard- & Software GmbH,
 *                 frank.bodammer@gcd-solutions.de
 *
 * (C) Copyright 2009
 * Grzegorz Bernacki, Semihalf, gjb@semihalf.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mpc5xxx.h>
#include <net.h>
#include <pci.h>
#include <asm/processor.h>
#include <asm/io.h>
#include "eeprom.h"
#if defined(CONFIG_DIGSY_REV5)
#include "is45s16800a2.h"
#include <mtd/cfi_flash.h>
#include <flash.h>
#else
#include "is42s16800a-7t.h"
#endif
#include <libfdt.h>
#include <fdt_support.h>
#include <i2c.h>
#include <mb862xx.h>

DECLARE_GLOBAL_DATA_PTR;

extern int usb_cpu_init(void);

#if defined(CONFIG_DIGSY_REV5)
/*
 * The M29W128GH needs a special reset command function,
 * details see the doc/README.cfi file
 */
void flash_cmd_reset(flash_info_t *info)
{
	flash_write_cmd(info, 0, 0, AMD_CMD_RESET);
}
#endif

#ifndef CONFIG_SYS_RAMBOOT
static void sdram_start(int hi_addr)
{
	long hi_addr_bit = hi_addr ? 0x01000000 : 0;
	long control = SDRAM_CONTROL | hi_addr_bit;

	/* unlock mode register */
	out_be32((void *)MPC5XXX_SDRAM_CTRL, control | 0x80000000);

	/* precharge all banks */
	out_be32((void *)MPC5XXX_SDRAM_CTRL, control | 0x80000002);

	/* auto refresh */
	out_be32((void *)MPC5XXX_SDRAM_CTRL, control | 0x80000004);

	/* set mode register */
	out_be32((void *)MPC5XXX_SDRAM_MODE, SDRAM_MODE);

	/* normal operation */
	out_be32((void *)MPC5XXX_SDRAM_CTRL, control);
}
#endif

/*
 * ATTENTION: Although partially referenced dram_init does NOT make real use
 *            use of CONFIG_SYS_SDRAM_BASE. The code does not work if
 *            CONFIG_SYS_SDRAM_BASE is something other than 0x00000000.
 */

int dram_init(void)
{
	ulong dramsize = 0;
	ulong dramsize2 = 0;
	uint svr, pvr;
#ifndef CONFIG_SYS_RAMBOOT
	ulong test1, test2;

	/* setup SDRAM chip selects */
	out_be32((void *)MPC5XXX_SDRAM_CS0CFG, 0x0000001C); /* 512MB at 0x0 */
	out_be32((void *)MPC5XXX_SDRAM_CS1CFG, 0x80000000); /* disabled */

	/* setup config registers */
	out_be32((void *)MPC5XXX_SDRAM_CONFIG1, SDRAM_CONFIG1);
	out_be32((void *)MPC5XXX_SDRAM_CONFIG2, SDRAM_CONFIG2);

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
		out_be32((void *)MPC5XXX_SDRAM_CS0CFG,
			(0x13 + __builtin_ffs(dramsize >> 20) - 1));
	} else {
		out_be32((void *)MPC5XXX_SDRAM_CS0CFG, 0); /* disabled */
	}

	/* let SDRAM CS1 start right after CS0 */
	out_be32((void *)MPC5XXX_SDRAM_CS1CFG, dramsize + 0x0000001C);

	/* find RAM size using SDRAM CS1 only */
	test1 = get_ram_size((long *)(CONFIG_SYS_SDRAM_BASE + dramsize),
			0x08000000);
		dramsize2 = test1;

	/* memory smaller than 1MB is impossible */
	if (dramsize2 < (1 << 20))
		dramsize2 = 0;

	/* set SDRAM CS1 size according to the amount of RAM found */
	if (dramsize2 > 0) {
		out_be32((void *)MPC5XXX_SDRAM_CS1CFG, (dramsize |
			(0x13 + __builtin_ffs(dramsize2 >> 20) - 1)));
	} else {
		out_be32((void *)MPC5XXX_SDRAM_CS1CFG, dramsize); /* disabled */
	}

#else /* CONFIG_SYS_RAMBOOT */

	/* retrieve size of memory connected to SDRAM CS0 */
	dramsize = in_be32((void *)MPC5XXX_SDRAM_CS0CFG) & 0xFF;
	if (dramsize >= 0x13)
		dramsize = (1 << (dramsize - 0x13)) << 20;
	else
		dramsize = 0;

	/* retrieve size of memory connected to SDRAM CS1 */
	dramsize2 = in_be32((void *)MPC5XXX_SDRAM_CS1CFG) & 0xFF;
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
		out_be32((void *)MPC5XXX_SDRAM_SDELAY, 0x04);

	gd->ram_size = dramsize + dramsize2;

	return 0;
}

int checkboard(void)
{
	char buf[64];
	int i = getenv_f("serial#", buf, sizeof(buf));

	puts ("Board: InterControl digsyMTC");
#if defined(CONFIG_DIGSY_REV5)
	puts (" rev5");
#endif
	if (i > 0) {
		puts(", ");
		puts(buf);
	}
	putc('\n');

	return 0;
}

#if defined(CONFIG_VIDEO)

#define GPIO_USB1_0		0x00010000	/* Power-On pin */
#define GPIO_USB1_9		0x08		/* PX_~EN pin */

#define GPIO_EE_DO		0x10		/* PSC6_0 (DO) pin */
#define GPIO_EE_CTS		0x20		/* PSC6_1 (CTS) pin */
#define GPIO_EE_DI		0x10000000	/* PSC6_2 (DI) pin */
#define GPIO_EE_CLK		0x20000000	/* PSC6_3 (CLK) pin */

#define GPT_GPIO_ON		0x00000034	/* GPT as simple GPIO, high */

static void exbo_hw_init(void)
{
	struct mpc5xxx_gpt *gpt = (struct mpc5xxx_gpt *)MPC5XXX_GPT;
	struct mpc5xxx_gpio *gpio = (struct mpc5xxx_gpio *)MPC5XXX_GPIO;
	struct mpc5xxx_wu_gpio *wu_gpio =
				(struct mpc5xxx_wu_gpio *)MPC5XXX_WU_GPIO;

	/* configure IrDA pins (PSC6 port) as gpios */
	gpio->port_config &= 0xFF8FFFFF;

	/* Init for USB1_0, EE_CLK and EE_DI - Low */
	setbits_be32(&gpio->simple_ddr,
			GPIO_USB1_0 | GPIO_EE_CLK | GPIO_EE_DI);
	clrbits_be32(&gpio->simple_ode,
			GPIO_USB1_0 | GPIO_EE_CLK | GPIO_EE_DI);
	clrbits_be32(&gpio->simple_dvo,
			GPIO_USB1_0 | GPIO_EE_CLK | GPIO_EE_DI);
	setbits_be32(&gpio->simple_gpioe,
			GPIO_USB1_0 | GPIO_EE_CLK | GPIO_EE_DI);

	/* Init for EE_DO, EE_CTS - Input */
	clrbits_8(&wu_gpio->ddr, GPIO_EE_DO | GPIO_EE_CTS);
	setbits_8(&wu_gpio->enable, GPIO_EE_DO | GPIO_EE_CTS);

	/* Init for PX_~EN (USB1_9) - High */
	clrbits_8(&gpio->sint_ode, GPIO_USB1_9);
	setbits_8(&gpio->sint_ddr, GPIO_USB1_9);
	clrbits_8(&gpio->sint_inten, GPIO_USB1_9);
	setbits_8(&gpio->sint_dvo, GPIO_USB1_9);
	setbits_8(&gpio->sint_gpioe, GPIO_USB1_9);

	/* Init for ~OE Switch (GPIO3) - Timer_0 GPIO High */
	out_be32(&gpt[0].emsr, GPT_GPIO_ON);
	/* Init for S Switch (GPIO4) - Timer_1 GPIO High */
	out_be32(&gpt[1].emsr, GPT_GPIO_ON);

	/* Power-On camera supply */
	setbits_be32(&gpio->simple_dvo, GPIO_USB1_0);
}
#else
static inline void exbo_hw_init(void) {}
#endif /* CONFIG_VIDEO */

int board_early_init_r(void)
{
	/*
	 * Now, when we are in RAM, enable flash write access for detection
	 * process.  Note that CS_BOOT cannot be cleared when executing in
	 * flash.
	 */
	/* disable CS_BOOT */
	clrbits_be32((void *)MPC5XXX_ADDECR, (1 << 25));
	/* enable CS1 */
	setbits_be32((void *)MPC5XXX_ADDECR, (1 << 17));
	/* enable CS0 */
	setbits_be32((void *)MPC5XXX_ADDECR, (1 << 16));

#if defined(CONFIG_USB_OHCI_NEW) && defined(CONFIG_SYS_USB_OHCI_CPU_INIT)
	/* Low level USB init, required for proper kernel operation */
	usb_cpu_init();
#endif

	return (0);
}

void board_get_enetaddr (uchar * enet)
{
	ushort read = 0;
	ushort addr_of_eth_addr = 0;
	ushort len_sys = 0;
	ushort len_sys_cfg = 0;

	/* check identification word */
	eeprom_read(EEPROM_ADDR, EEPROM_ADDR_IDENT, (uchar *)&read, 2);
	if (read != EEPROM_IDENT)
		return;

	/* calculate offset of config area */
	eeprom_read(EEPROM_ADDR, EEPROM_ADDR_LEN_SYS, (uchar *)&len_sys, 2);
	eeprom_read(EEPROM_ADDR, EEPROM_ADDR_LEN_SYSCFG,
		(uchar *)&len_sys_cfg, 2);
	addr_of_eth_addr = (len_sys + len_sys_cfg + EEPROM_ADDR_ETHADDR) << 1;
	if (addr_of_eth_addr >= EEPROM_LEN)
		return;

	eeprom_read(EEPROM_ADDR, addr_of_eth_addr, enet, 6);
}

int misc_init_r(void)
{
	pci_dev_t devbusfn;
	uchar enetaddr[6];

	/* check if graphic extension board is present */
	devbusfn = pci_find_device(PCI_VENDOR_ID_FUJITSU,
				   PCI_DEVICE_ID_CORAL_PA, 0);
	if (devbusfn != -1)
		exbo_hw_init();

	if (!eth_getenv_enetaddr("ethaddr", enetaddr)) {
		board_get_enetaddr(enetaddr);
		eth_setenv_enetaddr("ethaddr", enetaddr);
	}

	return 0;
}

#ifdef CONFIG_PCI
static struct pci_controller hose;

extern void pci_mpc5xxx_init(struct pci_controller *);

void pci_init_board(void)
{
	pci_mpc5xxx_init(&hose);
}
#endif

#ifdef CONFIG_CMD_IDE

#ifdef CONFIG_IDE_RESET

void init_ide_reset(void)
{
	debug ("init_ide_reset\n");

	/* set gpio output value to 1 */
	setbits_be32((void *)MPC5XXX_WU_GPIO_DATA_O, (1 << 25));
	/* open drain output */
	setbits_be32((void *)MPC5XXX_WU_GPIO_ODE, (1 << 25));
	/* direction output */
	setbits_be32((void *)MPC5XXX_WU_GPIO_DIR, (1 << 25));
	/* enable gpio */
	setbits_be32((void *)MPC5XXX_WU_GPIO_ENABLE, (1 << 25));

}

void ide_set_reset(int idereset)
{
	debug ("ide_reset(%d)\n", idereset);

	/* set gpio output value to 0 */
	clrbits_be32((void *)MPC5XXX_WU_GPIO_DATA_O, (1 << 25));
	/* open drain output */
	setbits_be32((void *)MPC5XXX_WU_GPIO_ODE, (1 << 25));
	/* direction output */
	setbits_be32((void *)MPC5XXX_WU_GPIO_DIR, (1 << 25));
	/* enable gpio */
	setbits_be32((void *)MPC5XXX_WU_GPIO_ENABLE, (1 << 25));

	udelay(10000);

	/* set gpio output value to 1 */
	setbits_be32((void *)MPC5XXX_WU_GPIO_DATA_O, (1 << 25));
	/* open drain output */
	setbits_be32((void *)MPC5XXX_WU_GPIO_ODE, (1 << 25));
	/* direction output */
	setbits_be32((void *)MPC5XXX_WU_GPIO_DIR, (1 << 25));
	/* enable gpio */
	setbits_be32((void *)MPC5XXX_WU_GPIO_ENABLE, (1 << 25));
}
#endif /* CONFIG_IDE_RESET */
#endif /* CONFIG_CMD_IDE */

#ifdef CONFIG_OF_BOARD_SETUP
static void ft_delete_node(void *fdt, const char *compat)
{
	int off = -1;
	int ret;

	off = fdt_node_offset_by_compatible(fdt, -1, compat);
	if (off < 0) {
		printf("Could not find %s node.\n", compat);
		return;
	}

	ret = fdt_del_node(fdt, off);
	if (ret < 0)
		printf("Could not delete %s node.\n", compat);
}
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

extern ulong flash_get_size (phys_addr_t base, int banknum);

/* Update the Flash Baseaddr settings */
int update_flash_size (int flash_size)
{
	volatile struct mpc5xxx_mmap_ctl *mm =
		(struct mpc5xxx_mmap_ctl *) CONFIG_SYS_MBAR;
	flash_info_t	*dev;
	int	i;
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

int ft_board_setup(void *blob, bd_t *bd)
{
	int phy_addr = CONFIG_PHY_ADDR;
	char eth_path[] = "/soc5200@f0000000/mdio@3000/ethernet-phy@0";

	ft_cpu_setup(blob, bd);
	/*
	 * There are 2 RTC nodes in the DTS, so remove
	 * the unneeded node here.
	 */
#if defined(CONFIG_DIGSY_REV5)
	ft_delete_node(blob, "dallas,ds1339");
#else
	ft_delete_node(blob, "mc,rv3029c2");
#endif
#if defined(CONFIG_SYS_UPDATE_FLASH_SIZE)
#ifdef CONFIG_FDT_FIXUP_NOR_FLASH_SIZE
	/* Update reg property in all nor flash nodes too */
	fdt_fixup_nor_flash_size(blob);
#endif
	ft_adapt_flash_base(blob);
#endif
	/* fix up the phy address */
	do_fixup_by_path(blob, eth_path, "reg", &phy_addr, sizeof(int), 0);

	return 0;
}
#endif /* CONFIG_OF_BOARD_SETUP */
