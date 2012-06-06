/*
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include <common.h>
#include <netdev.h>
#include <asm/cache.h>
#include <u-boot/md5.h>
#include <asm/arch/kirkwood.h>
#include <hush.h>

#define BUFLEN	16

void reset_cpu(unsigned long ignored)
{
	struct kwcpu_registers *cpureg =
	    (struct kwcpu_registers *)KW_CPU_REG_BASE;

	writel(readl(&cpureg->rstoutn_mask) | (1 << 2),
		&cpureg->rstoutn_mask);
	writel(readl(&cpureg->sys_soft_rst) | 1,
		&cpureg->sys_soft_rst);
	while (1) ;
}

/*
 * Generates Ramdom hex number reading some time varient system registers
 * and using md5 algorithm
 */
unsigned char get_random_hex(void)
{
	int i;
	u32 inbuf[BUFLEN];
	u8 outbuf[BUFLEN];

	/*
	 * in case of 88F6281/88F6282/88F6192 A0,
	 * Bit7 need to reset to generate random values in KW_REG_UNDOC_0x1470
	 * Soc reg offsets KW_REG_UNDOC_0x1470 and KW_REG_UNDOC_0x1478 are
	 * reserved regs and does not have names at this moment
	 * (no errata available)
	 */
	writel(readl(KW_REG_UNDOC_0x1478) & ~(1 << 7), KW_REG_UNDOC_0x1478);
	for (i = 0; i < BUFLEN; i++) {
		inbuf[i] = readl(KW_REG_UNDOC_0x1470);
	}
	md5((u8 *) inbuf, (BUFLEN * sizeof(u32)), outbuf);
	return outbuf[outbuf[7] % 0x0f];
}

/*
 * Window Size
 * Used with the Base register to set the address window size and location.
 * Must be programmed from LSB to MSB as sequence of ones followed by
 * sequence of zeros. The number of ones specifies the size of the window in
 * 64 KByte granularity (e.g., a value of 0x00FF specifies 256 = 16 MByte).
 * NOTE: A value of 0x0 specifies 64-KByte size.
 */
unsigned int kw_winctrl_calcsize(unsigned int sizeval)
{
	int i;
	unsigned int j = 0;
	u32 val = sizeval >> 1;

	for (i = 0; val >= 0x10000; i++) {
		j |= (1 << i);
		val = val >> 1;
	}
	return (0x0000ffff & j);
}

/*
 * kw_config_adr_windows - Configure address Windows
 *
 * There are 8 address windows supported by Kirkwood Soc to addess different
 * devices. Each window can be configured for size, BAR and remap addr
 * Below configuration is standard for most of the cases
 *
 * If remap function not used, remap_lo must be set as base
 *
 * Reference Documentation:
 * Mbus-L to Mbus Bridge Registers Configuration.
 * (Sec 25.1 and 25.3 of Datasheet)
 */
int kw_config_adr_windows(void)
{
	struct kwwin_registers *winregs =
		(struct kwwin_registers *)KW_CPU_WIN_BASE;

	/* Window 0: PCIE MEM address space */
	writel(KWCPU_WIN_CTRL_DATA(1024 * 1024 * 256, KWCPU_TARGET_PCIE,
		KWCPU_ATTR_PCIE_MEM, KWCPU_WIN_ENABLE), &winregs[0].ctrl);

	writel(KW_DEFADR_PCI_MEM, &winregs[0].base);
	writel(KW_DEFADR_PCI_MEM, &winregs[0].remap_lo);
	writel(0x0, &winregs[0].remap_hi);

	/* Window 1: PCIE IO address space */
	writel(KWCPU_WIN_CTRL_DATA(1024 * 64, KWCPU_TARGET_PCIE,
		KWCPU_ATTR_PCIE_IO, KWCPU_WIN_ENABLE), &winregs[1].ctrl);
	writel(KW_DEFADR_PCI_IO, &winregs[1].base);
	writel(KW_DEFADR_PCI_IO_REMAP, &winregs[1].remap_lo);
	writel(0x0, &winregs[1].remap_hi);

	/* Window 2: NAND Flash address space */
	writel(KWCPU_WIN_CTRL_DATA(1024 * 1024 * 128, KWCPU_TARGET_MEMORY,
		KWCPU_ATTR_NANDFLASH, KWCPU_WIN_ENABLE), &winregs[2].ctrl);
	writel(KW_DEFADR_NANDF, &winregs[2].base);
	writel(KW_DEFADR_NANDF, &winregs[2].remap_lo);
	writel(0x0, &winregs[2].remap_hi);

	/* Window 3: SPI Flash address space */
	writel(KWCPU_WIN_CTRL_DATA(1024 * 1024 * 128, KWCPU_TARGET_MEMORY,
		KWCPU_ATTR_SPIFLASH, KWCPU_WIN_ENABLE), &winregs[3].ctrl);
	writel(KW_DEFADR_SPIF, &winregs[3].base);
	writel(KW_DEFADR_SPIF, &winregs[3].remap_lo);
	writel(0x0, &winregs[3].remap_hi);

	/* Window 4: BOOT Memory address space */
	writel(KWCPU_WIN_CTRL_DATA(1024 * 1024 * 128, KWCPU_TARGET_MEMORY,
		KWCPU_ATTR_BOOTROM, KWCPU_WIN_ENABLE), &winregs[4].ctrl);
	writel(KW_DEFADR_BOOTROM, &winregs[4].base);

	/* Window 5: Security SRAM address space */
	writel(KWCPU_WIN_CTRL_DATA(1024 * 64, KWCPU_TARGET_SASRAM,
		KWCPU_ATTR_SASRAM, KWCPU_WIN_ENABLE), &winregs[5].ctrl);
	writel(KW_DEFADR_SASRAM, &winregs[5].base);

	/* Window 6-7: Disabled */
	writel(KWCPU_WIN_DISABLE, &winregs[6].ctrl);
	writel(KWCPU_WIN_DISABLE, &winregs[7].ctrl);

	return 0;
}

/*
 * kw_config_gpio - GPIO configuration
 */
void kw_config_gpio(u32 gpp0_oe_val, u32 gpp1_oe_val, u32 gpp0_oe, u32 gpp1_oe)
{
	struct kwgpio_registers *gpio0reg =
		(struct kwgpio_registers *)KW_GPIO0_BASE;
	struct kwgpio_registers *gpio1reg =
		(struct kwgpio_registers *)KW_GPIO1_BASE;

	/* Init GPIOS to default values as per board requirement */
	writel(gpp0_oe_val, &gpio0reg->dout);
	writel(gpp1_oe_val, &gpio1reg->dout);
	writel(gpp0_oe, &gpio0reg->oe);
	writel(gpp1_oe, &gpio1reg->oe);
}

/*
 * kw_config_mpp - Multi-Purpose Pins Functionality configuration
 *
 * Each MPP can be configured to different functionality through
 * MPP control register, ref (sec 6.1 of kirkwood h/w specification)
 *
 * There are maximum 64 Multi-Pourpose Pins on Kirkwood
 * Each MPP functionality can be configuration by a 4bit value
 * of MPP control reg, the value and associated functionality depends
 * upon used SoC varient
 */
int kw_config_mpp(u32 mpp0_7, u32 mpp8_15, u32 mpp16_23, u32 mpp24_31,
		u32 mpp32_39, u32 mpp40_47, u32 mpp48_55)
{
	u32 *mppreg = (u32 *) KW_MPP_BASE;

	/* program mpp registers */
	writel(mpp0_7, &mppreg[0]);
	writel(mpp8_15, &mppreg[1]);
	writel(mpp16_23, &mppreg[2]);
	writel(mpp24_31, &mppreg[3]);
	writel(mpp32_39, &mppreg[4]);
	writel(mpp40_47, &mppreg[5]);
	writel(mpp48_55, &mppreg[6]);
	return 0;
}

/*
 * SYSRSTn Duration Counter Support
 *
 * Kirkwood SoC implements a hardware-based SYSRSTn duration counter.
 * When SYSRSTn is asserted low, a SYSRSTn duration counter is running.
 * The SYSRSTn duration counter is useful for implementing a manufacturer
 * or factory reset. Upon a long reset assertion that is greater than a
 * pre-configured environment variable value for sysrstdelay,
 * The counter value is stored in the SYSRSTn Length Counter Register
 * The counter is based on the 25-MHz reference clock (40ns)
 * It is a 29-bit counter, yielding a maximum counting duration of
 * 2^29/25 MHz (21.4 seconds). When the counter reach its maximum value,
 * it remains at this value until counter reset is triggered by setting
 * bit 31 of KW_REG_SYSRST_CNT
 */
static void kw_sysrst_action(void)
{
	int ret;
	char *s = getenv("sysrstcmd");

	if (!s) {
		debug("Error.. %s failed, check sysrstcmd\n",
			__FUNCTION__);
		return;
	}

	debug("Starting %s process...\n", __FUNCTION__);
#if !defined(CONFIG_SYS_HUSH_PARSER)
	ret = run_command (s, 0);
#else
	ret = parse_string_outer(s, FLAG_PARSE_SEMICOLON
				  | FLAG_EXIT_FROM_LOOP);
#endif
	if (ret < 0)
		debug("Error.. %s failed\n", __FUNCTION__);
	else
		debug("%s process finished\n", __FUNCTION__);
}

static void kw_sysrst_check(void)
{
	u32 sysrst_cnt, sysrst_dly;
	char *s;

	/*
	 * no action if sysrstdelay environment variable is not defined
	 */
	s = getenv("sysrstdelay");
	if (s == NULL)
		return;

	/* read sysrstdelay value */
	sysrst_dly = (u32) simple_strtoul(s, NULL, 10);

	/* read SysRst Length counter register (bits 28:0) */
	sysrst_cnt = (0x1fffffff & readl(KW_REG_SYSRST_CNT));
	debug("H/w Rst hold time: %d.%d secs\n",
		sysrst_cnt / SYSRST_CNT_1SEC_VAL,
		sysrst_cnt % SYSRST_CNT_1SEC_VAL);

	/* clear the counter for next valid read*/
	writel(1 << 31, KW_REG_SYSRST_CNT);

	/*
	 * sysrst_action:
	 * if H/w Reset key is pressed and hold for time
	 * more than sysrst_dly in seconds
	 */
	if (sysrst_cnt >= SYSRST_CNT_1SEC_VAL * sysrst_dly)
		kw_sysrst_action();
}

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	char *rev;
	u16 devid = (readl(KW_REG_PCIE_DEVID) >> 16) & 0xffff;
	u8 revid = readl(KW_REG_PCIE_REVID) & 0xff;

	if ((readl(KW_REG_DEVICE_ID) & 0x03) > 2) {
		printf("Error.. %s:Unsupported Kirkwood SoC 88F%04x\n", __FUNCTION__, devid);
		return -1;
	}

	switch (revid) {
	case 0:
		rev = "Z0";
		break;
	case 2:
		rev = "A0";
		break;
	case 3:
		rev = "A1";
		break;
	default:
		rev = "??";
		break;
	}

	printf("SoC:   Kirkwood 88F%04x_%s\n", devid, rev);
	return 0;
}
#endif /* CONFIG_DISPLAY_CPUINFO */

#ifdef CONFIG_ARCH_CPU_INIT
int arch_cpu_init(void)
{
	u32 reg;
	struct kwcpu_registers *cpureg =
		(struct kwcpu_registers *)KW_CPU_REG_BASE;

	/* Linux expects` the internal registers to be at 0xf1000000 */
	writel(KW_REGS_PHY_BASE, KW_OFFSET_REG);

	/* Enable and invalidate L2 cache in write through mode */
	writel(readl(&cpureg->l2_cfg) | 0x18, &cpureg->l2_cfg);
	invalidate_l2_cache();

	kw_config_adr_windows();

#ifdef CONFIG_KIRKWOOD_RGMII_PAD_1V8
	/*
	 * Configures the I/O voltage of the pads connected to Egigabit
	 * Ethernet interface to 1.8V
	 * By defult it is set to 3.3V
	 */
	reg = readl(KW_REG_MPP_OUT_DRV_REG);
	reg |= (1 << 7);
	writel(reg, KW_REG_MPP_OUT_DRV_REG);
#endif
#ifdef CONFIG_KIRKWOOD_EGIGA_INIT
	/*
	 * Set egiga port0/1 in normal functional mode
	 * This is required becasue on kirkwood by default ports are in reset mode
	 * OS egiga driver may not have provision to set them in normal mode
	 * and if u-boot is build without network support, network may fail at OS level
	 */
	reg = readl(KWGBE_PORT_SERIAL_CONTROL1_REG(0));
	reg &= ~(1 << 4);	/* Clear PortReset Bit */
	writel(reg, (KWGBE_PORT_SERIAL_CONTROL1_REG(0)));
	reg = readl(KWGBE_PORT_SERIAL_CONTROL1_REG(1));
	reg &= ~(1 << 4);	/* Clear PortReset Bit */
	writel(reg, (KWGBE_PORT_SERIAL_CONTROL1_REG(1)));
#endif
#ifdef CONFIG_KIRKWOOD_PCIE_INIT
	/*
	 * Enable PCI Express Port0
	 */
	reg = readl(&cpureg->ctrl_stat);
	reg |= (1 << 0);	/* Set PEX0En Bit */
	writel(reg, &cpureg->ctrl_stat);
#endif
	return 0;
}
#endif /* CONFIG_ARCH_CPU_INIT */

/*
 * SOC specific misc init
 */
#if defined(CONFIG_ARCH_MISC_INIT)
int arch_misc_init(void)
{
	volatile u32 temp;

	/*CPU streaming & write allocate */
	temp = readfr_extra_feature_reg();
	temp &= ~(1 << 28);	/* disable wr alloc */
	writefr_extra_feature_reg(temp);

	temp = readfr_extra_feature_reg();
	temp &= ~(1 << 29);	/* streaming disabled */
	writefr_extra_feature_reg(temp);

	/* L2Cache settings */
	temp = readfr_extra_feature_reg();
	/* Disable L2C pre fetch - Set bit 24 */
	temp |= (1 << 24);
	/* enable L2C - Set bit 22 */
	temp |= (1 << 22);
	writefr_extra_feature_reg(temp);

	icache_enable();
	/* Change reset vector to address 0x0 */
	temp = get_cr();
	set_cr(temp & ~CR_V);

	/* checks and execute resset to factory event */
	kw_sysrst_check();

	return 0;
}
#endif /* CONFIG_ARCH_MISC_INIT */

#ifdef CONFIG_MVGBE
int cpu_eth_init(bd_t *bis)
{
	mvgbe_initialize(bis);
	return 0;
}
#endif
