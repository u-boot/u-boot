/*
 * Copyright (C) 2016-2017 Intel Corporation
 *
 * SPDX-License-Identifier:    GPL-2.0
 */

#include <altera.h>
#include <common.h>
#include <errno.h>
#include <fdtdec.h>
#include <miiphy.h>
#include <netdev.h>
#include <ns16550.h>
#include <watchdog.h>
#include <asm/arch/misc.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/reset_manager.h>
#include <asm/arch/sdram_arria10.h>
#include <asm/arch/system_manager.h>
#include <asm/arch/nic301.h>
#include <asm/io.h>
#include <asm/pl310.h>

#define PINMUX_UART0_TX_SHARED_IO_OFFSET_Q1_3	0x08
#define PINMUX_UART0_TX_SHARED_IO_OFFSET_Q2_11	0x58
#define PINMUX_UART0_TX_SHARED_IO_OFFSET_Q3_3	0x68
#define PINMUX_UART1_TX_SHARED_IO_OFFSET_Q1_7	0x18
#define PINMUX_UART1_TX_SHARED_IO_OFFSET_Q3_7	0x78
#define PINMUX_UART1_TX_SHARED_IO_OFFSET_Q4_3	0x98

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_SPL_BUILD)
static struct pl310_regs *const pl310 =
	(struct pl310_regs *)CONFIG_SYS_PL310_BASE;
static const struct socfpga_noc_fw_ocram *noc_fw_ocram_base =
	(void *)SOCFPGA_SDR_FIREWALL_OCRAM_ADDRESS;
#endif

static struct socfpga_system_manager *sysmgr_regs =
	(struct socfpga_system_manager *)SOCFPGA_SYSMGR_ADDRESS;

/*
 * DesignWare Ethernet initialization
 */
#ifdef CONFIG_ETH_DESIGNWARE
void dwmac_deassert_reset(const unsigned int of_reset_id,
				 const u32 phymode)
{
	u32 reset;

	if (of_reset_id == EMAC0_RESET) {
		reset = SOCFPGA_RESET(EMAC0);
	} else if (of_reset_id == EMAC1_RESET) {
		reset = SOCFPGA_RESET(EMAC1);
	} else if (of_reset_id == EMAC2_RESET) {
		reset = SOCFPGA_RESET(EMAC2);
	} else {
		printf("GMAC: Invalid reset ID (%i)!\n", of_reset_id);
		return;
	}

	clrsetbits_le32(&sysmgr_regs->emac[of_reset_id - EMAC0_RESET],
			SYSMGR_EMACGRP_CTRL_PHYSEL_MASK,
			phymode);

	/* Release the EMAC controller from reset */
	socfpga_per_reset(reset, 0);
}
#endif

#if defined(CONFIG_SPL_BUILD)
/*
+ * This function initializes security policies to be consistent across
+ * all logic units in the Arria 10.
+ *
+ * The idea is to set all security policies to be normal, nonsecure
+ * for all units.
+ */
static void initialize_security_policies(void)
{
	/* Put OCRAM in non-secure */
	writel(0x003f0000, &noc_fw_ocram_base->region0);
	writel(0x1, &noc_fw_ocram_base->enable);
}

int arch_early_init_r(void)
{
	initialize_security_policies();

	/* Configure the L2 controller to make SDRAM start at 0 */
	writel(0x1, &pl310->pl310_addr_filter_start);

	/* assert reset to all except L4WD0 and L4TIMER0 */
	socfpga_per_reset_all();

	/* configuring the clock based on handoff */
	/* TODO: Add call to cm_basic_init() */

	/* Add device descriptor to FPGA device table */
	socfpga_fpga_add();
	return 0;
}
#else
int arch_early_init_r(void)
{
	return 0;
}
#endif

/*
 * This function looking the 1st encounter UART peripheral,
 * and then return its offset of the dedicated/shared IO pin
 * mux. offset value (zero and above).
 */
static int find_peripheral_uart(const void *blob,
	int child, const char *node_name)
{
	int len;
	fdt_addr_t base_addr = 0;
	fdt_size_t size;
	const u32 *cell;
	u32 value, offset = 0;

	base_addr = fdtdec_get_addr_size(blob, child, "reg", &size);
	if (base_addr != FDT_ADDR_T_NONE) {
		cell = fdt_getprop(blob, child, "pinctrl-single,pins",
			&len);
		if (cell != NULL) {
			for (; len > 0; len -= (2 * sizeof(u32))) {
				offset = fdt32_to_cpu(*cell++);
				value = fdt32_to_cpu(*cell++);
				/* Found UART peripheral. */
				if (value == PINMUX_UART)
					return offset;
			}
		}
	}
	return -EINVAL;
}

/*
 * This function looks up the 1st encounter UART peripheral,
 * and then return its offset of the dedicated/shared IO pin
 * mux. UART peripheral is found if the offset is not in negative
 * value.
 */
static int is_peripheral_uart_true(const void *blob,
	int node, const char *child_name)
{
	int child, len;
	const char *node_name;

	child = fdt_first_subnode(blob, node);

	if (child < 0)
		return -EINVAL;

	node_name = fdt_get_name(blob, child, &len);

	while (node_name) {
		if (!strcmp(child_name, node_name))
			return find_peripheral_uart(blob, child, node_name);

		child = fdt_next_subnode(blob, child);
		if (child < 0)
			break;

		node_name = fdt_get_name(blob, child, &len);
	}

	return -1;
}

/*
 * This function looking the 1st encounter UART dedicated IO peripheral,
 * and then return based address of the 1st encounter UART dedicated
 * IO peripheral.
 */
unsigned int dedicated_uart_com_port(const void *blob)
{
	int node;

	node = fdtdec_next_compatible(blob, 0,
		 COMPAT_ALTERA_SOCFPGA_PINCTRL_SINGLE);
	if (node < 0)
		return 0;

	if (is_peripheral_uart_true(blob, node, "dedicated") >= 0)
		return SOCFPGA_UART1_ADDRESS;

	return 0;
}

/*
 * This function looking the 1st encounter UART shared IO peripheral, and then
 * return based address of the 1st encounter UART shared IO peripheral.
 */
unsigned int shared_uart_com_port(const void *blob)
{
	int node, ret;

	node = fdtdec_next_compatible(blob, 0,
		 COMPAT_ALTERA_SOCFPGA_PINCTRL_SINGLE);
	if (node < 0)
		return 0;

	ret = is_peripheral_uart_true(blob, node, "shared");

	if (ret == PINMUX_UART0_TX_SHARED_IO_OFFSET_Q1_3 ||
	    ret == PINMUX_UART0_TX_SHARED_IO_OFFSET_Q2_11 ||
	    ret == PINMUX_UART0_TX_SHARED_IO_OFFSET_Q3_3)
		return SOCFPGA_UART0_ADDRESS;
	else if (ret == PINMUX_UART1_TX_SHARED_IO_OFFSET_Q1_7 ||
		ret == PINMUX_UART1_TX_SHARED_IO_OFFSET_Q3_7 ||
		ret == PINMUX_UART1_TX_SHARED_IO_OFFSET_Q4_3)
		return SOCFPGA_UART1_ADDRESS;

	return 0;
}

/*
 * This function looking the 1st encounter UART peripheral, and then return
 * base address of the 1st encounter UART peripheral.
 */
unsigned int uart_com_port(const void *blob)
{
	unsigned int ret;

	ret = dedicated_uart_com_port(blob);

	if (ret)
		return ret;

	return shared_uart_com_port(blob);
}

/*
 * Print CPU information
 */
#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	const u32 bsel =
		SYSMGR_GET_BOOTINFO_BSEL(readl(&sysmgr_regs->bootinfo));

	puts("CPU:   Altera SoCFPGA Arria 10\n");

	printf("BOOT:  %s\n", bsel_str[bsel].name);
	return 0;
}
#endif

#ifdef CONFIG_ARCH_MISC_INIT
int arch_misc_init(void)
{
	return 0;
}
#endif
