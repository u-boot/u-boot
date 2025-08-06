// SPDX-License-Identifier: GPL-2.0-only
/*
 * PCIe controller driver for Renesas R-Car Gen4 Series SoCs
 * Copyright (C) 2025 Marek Vasut <marek.vasut+renesas@mailbox.org>
 * Based on Linux kernel driver
 * Copyright (C) 2022-2023 Renesas Electronics Corporation
 *
 * The r8a779g0 (R-Car V4H) controller requires a specific firmware to be
 * provided, to initialize the PHY. Otherwise, the PCIe controller will not
 * work.
 */

#include <asm-generic/gpio.h>
#include <asm/arch/gpio.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <clk.h>
#include <command.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <env.h>
#include <log.h>
#include <reset.h>

#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/iopoll.h>

#include "pcie_dw_common.h"

/* Renesas-specific */
/* PCIe Mode Setting Register 0 */
#define PCIEMSR0				0x0000
#define APP_SRIS_MODE				BIT(6)
#define DEVICE_TYPE_EP				0
#define DEVICE_TYPE_RC				BIT(4)
#define BIFUR_MOD_SET_ON			BIT(0)

/* PCIe Interrupt Status 0 */
#define PCIEINTSTS0				0x0084

/* PCIe Interrupt Status 0 Enable */
#define PCIEINTSTS0EN				0x0310
#define MSI_CTRL_INT				BIT(26)
#define SMLH_LINK_UP				BIT(7)
#define RDLH_LINK_UP				BIT(6)

/* PCIe DMA Interrupt Status Enable */
#define PCIEDMAINTSTSEN				0x0314
#define PCIEDMAINTSTSEN_INIT			GENMASK(15, 0)

/* Port Logic Registers 89 */
#define PRTLGC89				0x0b70

/* Port Logic Registers 90 */
#define PRTLGC90				0x0b74

/* PCIe Reset Control Register 1 */
#define PCIERSTCTRL1				0x0014
#define APP_HOLD_PHY_RST			BIT(16)
#define APP_LTSSM_ENABLE			BIT(0)

/* PCIe Power Management Control */
#define PCIEPWRMNGCTRL				0x0070
#define APP_CLK_REQ_N				BIT(11)
#define APP_CLK_PM_EN				BIT(10)

#define RCAR_NUM_SPEED_CHANGE_RETRIES		10
#define RCAR_MAX_LINK_SPEED			4

#define RCAR_GEN4_PCIE_EP_FUNC_DBI_OFFSET	0x1000
#define RCAR_GEN4_PCIE_EP_FUNC_DBI2_OFFSET	0x800

#define RCAR_GEN4_PCIE_FIRMWARE_NAME		"rcar_gen4_pcie.bin"
#define RCAR_GEN4_PCIE_FIRMWARE_BASE_ADDR	0xc000

#define PCIE_T_PVPERL_MS			100

/**
 * struct rcar_gen4_pcie - Renesas R-Car Gen4 DW PCIe controller state
 *
 * @rcar:		The common PCIe DW structure
 * @pwr_rst:		The PWR reset of the PCIe core
 * @core_clk:		The core clock of the PCIe core
 * @ref_clk:		The reference clock of the PCIe core and possibly bus
 * @pe_rst:		PERST GPIO
 * @app_base:		The base address of application register space
 * @dbi2_base:		The base address of DBI2 register space
 * @phy_base:		The base address of PHY register space
 * @max_link_speed:	Maximum PCIe link speed supported by the setup
 * @num_lanes:		Number of PCIe lanes used by the setup
 * @firmware:		PHY firmware
 * @firmware_size:	PHY firmware size in Bytes
 */
struct rcar_gen4_pcie {
	/* Must be first member of the struct */
	struct			pcie_dw dw;
	struct reset_ctl	pwr_rst;
	struct clk		*core_clk;
	struct clk		*ref_clk;
	struct gpio_desc	pe_rst;
	void			*app_base;
	void			*dbi2_base;
	void			*phy_base;
	u32			max_link_speed;
	u32			num_lanes;
	u16			*firmware;
	u32			firmware_size;
};

/* Common */
static bool rcar_gen4_pcie_link_up(struct rcar_gen4_pcie *rcar)
{
	u32 val, mask;

	val = readl(rcar->app_base + PCIEINTSTS0);
	mask = RDLH_LINK_UP | SMLH_LINK_UP;

	return (val & mask) == mask;
}

/*
 * Manually initiate the speed change. Return 0 if change succeeded; otherwise
 * -ETIMEDOUT.
 */
static int rcar_gen4_pcie_speed_change(struct rcar_gen4_pcie *rcar)
{
	u32 val;
	int i;

	clrbits_le32(rcar->dw.dbi_base + PCIE_LINK_WIDTH_SPEED_CONTROL,
		     PORT_LOGIC_SPEED_CHANGE);

	setbits_le32(rcar->dw.dbi_base + PCIE_LINK_WIDTH_SPEED_CONTROL,
		     PORT_LOGIC_SPEED_CHANGE);

	for (i = 0; i < RCAR_NUM_SPEED_CHANGE_RETRIES; i++) {
		val = readl(rcar->dw.dbi_base + PCIE_LINK_WIDTH_SPEED_CONTROL);
		if (!(val & PORT_LOGIC_SPEED_CHANGE))
			return 0;
		mdelay(10);
	}

	return -ETIMEDOUT;
}

/*
 * SoC datasheet suggests checking port logic register bits during firmware
 * write. If read returns non-zero value, then this function returns -EAGAIN
 * indicating that the write needs to be done again. If read returns zero,
 * then return 0 to indicate success.
 */
static int rcar_gen4_pcie_reg_test_bit(struct rcar_gen4_pcie *rcar,
				       u32 offset, u32 mask)
{
	if (readl(rcar->dw.dbi_base + offset) & mask)
		return -EAGAIN;

	return 0;
}

static int rcar_gen4_pcie_download_phy_firmware(struct rcar_gen4_pcie *rcar)
{
	/* The check_addr values are magical numbers in the datasheet */
	static const u32 check_addr[] = {
		0x00101018,
		0x00101118,
		0x00101021,
		0x00101121,
	};
	unsigned int i, timeout;
	u32 data;
	int ret;

	for (i = 0; i < rcar->firmware_size / 2; i++) {
		data = rcar->firmware[i];
		timeout = 100;
		do {
			writel(RCAR_GEN4_PCIE_FIRMWARE_BASE_ADDR + i, rcar->dw.dbi_base + PRTLGC89);
			writel(data, rcar->dw.dbi_base + PRTLGC90);
			if (!rcar_gen4_pcie_reg_test_bit(rcar, PRTLGC89, BIT(30)))
				break;
			if (!(--timeout))
				return -ETIMEDOUT;
			udelay(100);
		} while (1);
	}

	setbits_le32(rcar->phy_base + 0x0f8, BIT(17));

	for (i = 0; i < ARRAY_SIZE(check_addr); i++) {
		timeout = 100;
		do {
			writel(check_addr[i], rcar->dw.dbi_base + PRTLGC89);
			ret = rcar_gen4_pcie_reg_test_bit(rcar, PRTLGC89, BIT(30));
			ret |= rcar_gen4_pcie_reg_test_bit(rcar, PRTLGC90, BIT(0));
			if (!ret)
				break;
			if (!(--timeout))
				return -ETIMEDOUT;
			udelay(100);
		} while (1);
	}

	return ret;
}

static int rcar_gen4_pcie_ltssm_control(struct rcar_gen4_pcie *rcar, bool enable)
{
	u32 val;
	int ret;

	if (!enable) {
		clrbits_le32(rcar->app_base + PCIERSTCTRL1, APP_LTSSM_ENABLE);
		return 0;
	}

	setbits_le32(rcar->dw.dbi_base + PCIE_PORT_FORCE,
		     PORT_FORCE_DO_DESKEW_FOR_SRIS);

	setbits_le32(rcar->app_base + PCIEMSR0, APP_SRIS_MODE);

	/*
	 * The R-Car Gen4 datasheet doesn't describe the PHY registers' name.
	 * But, the initialization procedure describes these offsets. So,
	 * this driver has magical offset numbers.
	 */
	clrsetbits_le32(rcar->phy_base + 0x700, BIT(28), 0);
	clrsetbits_le32(rcar->phy_base + 0x700, BIT(20), 0);
	clrsetbits_le32(rcar->phy_base + 0x700, BIT(12), 0);
	clrsetbits_le32(rcar->phy_base + 0x700, BIT(4), 0);

	clrsetbits_le32(rcar->phy_base + 0x148, GENMASK(23, 22), BIT(22));
	clrsetbits_le32(rcar->phy_base + 0x148, GENMASK(18, 16), GENMASK(17, 16));
	clrsetbits_le32(rcar->phy_base + 0x148, GENMASK(7, 6), BIT(6));
	clrsetbits_le32(rcar->phy_base + 0x148, GENMASK(2, 0), GENMASK(1, 0));
	clrsetbits_le32(rcar->phy_base + 0x1d4, GENMASK(16, 15), GENMASK(16, 15));
	clrsetbits_le32(rcar->phy_base + 0x514, BIT(26), BIT(26));
	clrsetbits_le32(rcar->phy_base + 0x0f8, BIT(16), 0);
	clrsetbits_le32(rcar->phy_base + 0x0f8, BIT(19), BIT(19));

	clrbits_le32(rcar->app_base + PCIERSTCTRL1, APP_HOLD_PHY_RST);

	ret = readl_poll_timeout(rcar->phy_base + 0x0f8, val, !(val & BIT(18)), 10000);
	if (ret < 0)
		return ret;

	ret = rcar_gen4_pcie_download_phy_firmware(rcar);
	if (ret)
		return ret;

	setbits_le32(rcar->app_base + PCIERSTCTRL1, APP_LTSSM_ENABLE);

	return 0;
}

/*
 * Enable LTSSM of this controller and manually initiate the speed change.
 * Always return 0.
 */
static int rcar_gen4_pcie_start_link(struct rcar_gen4_pcie *rcar)
{
	int i, ret;

	ret = rcar_gen4_pcie_ltssm_control(rcar, true);
	if (ret)
		return ret;

	/*
	 * Require direct speed change with retrying here if the max_link_speed
	 * is PCIe Gen2 or higher.
	 */
	if (rcar->max_link_speed == LINK_SPEED_GEN_1)
		return 0;

	for (i = 0; i < RCAR_MAX_LINK_SPEED; i++) {
		/* It may not be connected in EP mode yet. So, break the loop */
		if (rcar_gen4_pcie_speed_change(rcar))
			break;
	}

	return 0;
}

static void rcar_gen4_pcie_additional_common_init(struct rcar_gen4_pcie *rcar)
{
	clrsetbits_le32(rcar->dw.dbi_base + PCIE_PORT_LANE_SKEW,
			PORT_LANE_SKEW_INSERT_MASK,
			(rcar->num_lanes < 4) ? BIT(6) : 0);

	setbits_le32(rcar->app_base + PCIEPWRMNGCTRL,
		     APP_CLK_REQ_N | APP_CLK_PM_EN);
}

static int rcar_gen4_pcie_common_init(struct rcar_gen4_pcie *rcar)
{
	int ret;

	ret = clk_prepare_enable(rcar->core_clk);
	if (ret)
		return ret;

	ret = reset_assert(&rcar->pwr_rst);
	if (ret)
		goto err_unprepare;

	setbits_le32(rcar->app_base + PCIEMSR0,
		     DEVICE_TYPE_RC |
		     ((rcar->num_lanes < 4) ? BIFUR_MOD_SET_ON : 0));

	ret = reset_deassert(&rcar->pwr_rst);
	if (ret)
		goto err_unprepare;

	rcar_gen4_pcie_additional_common_init(rcar);

	return 0;

err_unprepare:
	clk_disable_unprepare(rcar->core_clk);

	return ret;
}

/* Host mode */
static int rcar_gen4_pcie_host_init(struct udevice *dev)
{
	struct rcar_gen4_pcie *rcar = dev_get_priv(dev);
	int ret;

	dm_gpio_set_value(&rcar->pe_rst, 1);

	ret = rcar_gen4_pcie_common_init(rcar);
	if (ret)
		return ret;

	/*
	 * According to the section 3.5.7.2 "RC Mode" in DWC PCIe Dual Mode
	 * Rev.5.20a and 3.5.6.1 "RC mode" in DWC PCIe RC databook v5.20a, we
	 * should disable two BARs to avoid unnecessary memory assignment
	 * during device enumeration.
	 */
	writel(0x0, rcar->dbi2_base + PCI_BASE_ADDRESS_0);
	writel(0x0, rcar->dbi2_base + PCI_BASE_ADDRESS_1);

	/* Disable MSI interrupt signal */
	clrbits_le32(rcar->app_base + PCIEINTSTS0EN, MSI_CTRL_INT);

	mdelay(PCIE_T_PVPERL_MS);	/* pe_rst requires 100msec delay */

	dm_gpio_set_value(&rcar->pe_rst, 0);

	return 0;
}

static int rcar_gen4_pcie_load_firmware(struct rcar_gen4_pcie *rcar)
{
	ulong addr, size;
	int ret;

	/*
	 * Run user specified firmware loading script, which loads the
	 * firmware from whichever location the user decides it should
	 * load the firmware from, by whatever means the user decides.
	 */
	ret = run_command_list("run renesas_rcar_gen4_load_firmware", -1, 0);
	if (ret) {
		printf("Firmware loading script 'renesas_rcar_gen4_load_firmware' not defined or failed.\n");
		goto fail;
	}

	/* Find out where the firmware got loaded and how long it is. */
	addr = env_get_hex("renesas_rcar_gen4_load_firmware_addr", 0);
	size = env_get_hex("renesas_rcar_gen4_load_firmware_size", 0);

	/*
	 * Clear the variables set by the firmware loading script, as
	 * their content would become stale once this function exits.
	 */
	env_set("renesas_rcar_gen4_load_firmware_addr", NULL);
	env_set("renesas_rcar_gen4_load_firmware_size", NULL);

	if (!addr || !size) {
		printf("Firmware address (%lx) or size (%lx) are invalid.\n", addr, size);
		goto fail;
	}

	/* Create local copy of the loaded firmware. */
	rcar->firmware = (u16 *)memdup((void *)addr, size);
	if (!rcar->firmware)
		return -ENOMEM;

	rcar->firmware_size = size;

	return 0;

fail:
	printf("Define 'renesas_rcar_gen4_load_firmware' script which loads the R-Car\n"
	       "Gen4 PCIe controller firmware from storage into memory and sets these\n"
	       "two environment variables:\n"
	       "  renesas_rcar_gen4_load_firmware_addr ... address of firmware in memory\n"
	       "  renesas_rcar_gen4_load_firmware_size ... length of firmware in bytes\n"
	       "\n"
	       "Example:\n"
	       "  => env set renesas_rcar_gen4_load_firmware 'env set renesas_rcar_gen4_load_firmware_addr 0x54000000 && load mmc 0:1 ${renesas_rcar_gen4_load_firmware_addr} lib/firmware/rcar_gen4_pcie.bin && env set renesas_rcar_gen4_load_firmware_size ${filesize}'\n"
	       );
	return -EINVAL;
}

/**
 * rcar_gen4_pcie_probe() - Probe the PCIe bus for active link
 *
 * @dev: A pointer to the device being operated on
 *
 * Probe for an active link on the PCIe bus and configure the controller
 * to enable this port.
 *
 * Return: 0 on success, else -ENODEV
 */
static int rcar_gen4_pcie_probe(struct udevice *dev)
{
	struct rcar_gen4_pcie *rcar = dev_get_priv(dev);
	struct udevice *ctlr = pci_get_controller(dev);
	struct pci_controller *hose = dev_get_uclass_priv(ctlr);
	int ret;

	ret = rcar_gen4_pcie_load_firmware(rcar);
	if (ret)
		return ret;

	rcar->dw.first_busno = dev_seq(dev);
	rcar->dw.dev = dev;

	ret = reset_get_by_name(dev, "pwr", &rcar->pwr_rst);
	if (ret)
		return ret;

	rcar->core_clk = devm_clk_get(dev, "core");
	if (IS_ERR(rcar->core_clk))
		return PTR_ERR(rcar->core_clk);

	rcar->ref_clk = devm_clk_get(dev, "ref");
	if (IS_ERR(rcar->ref_clk))
		return PTR_ERR(rcar->ref_clk);

	ret = clk_prepare_enable(rcar->ref_clk);
	if (ret)
		return ret;

	ret = gpio_request_by_name(dev, "reset-gpios", 0, &rcar->pe_rst,
				   GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE);
	if (ret)
		return ret;

	ret = rcar_gen4_pcie_host_init(dev);
	if (ret)
		return ret;

	pcie_dw_setup_host(&rcar->dw);

	dw_pcie_dbi_write_enable(&rcar->dw, true);

	dw_pcie_link_set_max_link_width(&rcar->dw, rcar->num_lanes);

	ret = rcar_gen4_pcie_start_link(rcar);
	if (ret)
		return ret;

	dw_pcie_dbi_write_enable(&rcar->dw, false);

	if (!rcar_gen4_pcie_link_up(rcar)) {
		printf("PCIE-%d: Link down\n", dev_seq(dev));
		return -ENODEV;
	}

	printf("PCIE-%d: Link up (Gen%d-x%d, Bus%d)\n", dev_seq(dev),
	       pcie_dw_get_link_speed(&rcar->dw),
	       pcie_dw_get_link_width(&rcar->dw),
	       hose->first_busno);

	pcie_dw_prog_outbound_atu_unroll(&rcar->dw, PCIE_ATU_REGION_INDEX0,
					 PCIE_ATU_TYPE_MEM,
					 rcar->dw.mem.phys_start,
					 rcar->dw.mem.bus_start, rcar->dw.mem.size);

	return 0;
}

/**
 * rcar_gen4_pcie_of_to_plat() - Translate from DT to device state
 *
 * @dev: A pointer to the device being operated on
 *
 * Translate relevant data from the device tree pertaining to device @dev into
 * state that the driver will later make use of. This state is stored in the
 * device's private data structure.
 *
 * Return: 0 on success, else -EINVAL
 */
static int rcar_gen4_pcie_of_to_plat(struct udevice *dev)
{
	struct rcar_gen4_pcie *rcar = dev_get_priv(dev);

	/* Get the controller base address */
	rcar->dw.dbi_base = (void *)dev_read_addr_name(dev, "dbi");
	if ((fdt_addr_t)rcar->dw.dbi_base == FDT_ADDR_T_NONE)
		return -EINVAL;

	/* Get the config space base address and size */
	rcar->dw.cfg_base = (void *)dev_read_addr_size_name(dev, "config",
							    &rcar->dw.cfg_size);
	if ((fdt_addr_t)rcar->dw.cfg_base == FDT_ADDR_T_NONE)
		return -EINVAL;

	/* Get the iATU base address and size */
	rcar->dw.atu_base = (void *)dev_read_addr_name(dev, "atu");
	if ((fdt_addr_t)rcar->dw.atu_base == FDT_ADDR_T_NONE)
		return -EINVAL;

	/* Get the PHY base address and size */
	rcar->phy_base = (void *)dev_read_addr_name(dev, "phy");
	if ((fdt_addr_t)rcar->phy_base == FDT_ADDR_T_NONE)
		return -EINVAL;

	/* Get the app base address and size */
	rcar->app_base = (void *)dev_read_addr_name(dev, "app");
	if ((fdt_addr_t)rcar->app_base == FDT_ADDR_T_NONE)
		return -EINVAL;

	/* Get the dbi2 base address and size */
	rcar->dbi2_base = (void *)dev_read_addr_name(dev, "dbi2");
	if ((fdt_addr_t)rcar->dbi2_base == FDT_ADDR_T_NONE)
		return -EINVAL;

	rcar->max_link_speed =
		clamp(dev_read_u32_default(dev, "max-link-speed",
					   LINK_SPEED_GEN_4),
		      LINK_SPEED_GEN_1, RCAR_MAX_LINK_SPEED);

	rcar->num_lanes = dev_read_u32_default(dev, "num-lanes", 4);

	return 0;
}

static const struct dm_pci_ops rcar_gen4_pcie_ops = {
	.read_config	= pcie_dw_read_config,
	.write_config	= pcie_dw_write_config,
};

static const struct udevice_id rcar_gen4_pcie_ids[] = {
	{ .compatible = "renesas,rcar-gen4-pcie" },
	{ }
};

U_BOOT_DRIVER(rcar_gen4_pcie) = {
	.name		= "rcar_gen4_pcie",
	.id		= UCLASS_PCI,
	.of_match	= rcar_gen4_pcie_ids,
	.ops		= &rcar_gen4_pcie_ops,
	.of_to_plat	= rcar_gen4_pcie_of_to_plat,
	.probe		= rcar_gen4_pcie_probe,
	.priv_auto	= sizeof(struct rcar_gen4_pcie),
};
