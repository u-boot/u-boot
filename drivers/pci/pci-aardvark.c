/*
 * ***************************************************************************
 * Copyright (C) 2015 Marvell International Ltd.
 * ***************************************************************************
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 2 of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * ***************************************************************************
 */
/* pcie_advk.c
 *
 * Ported from Linux driver - driver/pci/host/pci-aardvark.c
 *
 * Author: Victor Gu <xigu@marvell.com>
 *         Hezi Shahmoon <hezi.shahmoon@marvell.com>
 *
 */

#include <common.h>
#include <dm.h>
#include <pci.h>
#include <asm/io.h>
#include <asm-generic/gpio.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/ioport.h>

/* PCIe core registers */
#define PCIE_CORE_CMD_STATUS_REG				0x4
#define     PCIE_CORE_CMD_IO_ACCESS_EN				BIT(0)
#define     PCIE_CORE_CMD_MEM_ACCESS_EN				BIT(1)
#define     PCIE_CORE_CMD_MEM_IO_REQ_EN				BIT(2)
#define PCIE_CORE_DEV_REV_REG					0x8
#define PCIE_CORE_EXP_ROM_BAR_REG				0x30
#define PCIE_CORE_PCIEXP_CAP_OFF				0xc0
#define PCIE_CORE_DEV_CTRL_STATS_REG				0xc8
#define     PCIE_CORE_DEV_CTRL_STATS_RELAX_ORDER_DISABLE	(0 << 4)
#define     PCIE_CORE_DEV_CTRL_STATS_SNOOP_DISABLE		(0 << 11)
#define     PCIE_CORE_DEV_CTRL_STATS_MAX_PAYLOAD_SIZE		0x2
#define     PCIE_CORE_DEV_CTRL_STATS_MAX_PAYLOAD_SIZE_SHIFT	5
#define     PCIE_CORE_DEV_CTRL_STATS_MAX_RD_REQ_SIZE		0x2
#define     PCIE_CORE_DEV_CTRL_STATS_MAX_RD_REQ_SIZE_SHIFT	12
#define PCIE_CORE_LINK_CTRL_STAT_REG				0xd0
#define     PCIE_CORE_LINK_TRAINING				BIT(5)
#define PCIE_CORE_ERR_CAPCTL_REG				0x118
#define     PCIE_CORE_ERR_CAPCTL_ECRC_CHK_TX			BIT(5)
#define     PCIE_CORE_ERR_CAPCTL_ECRC_CHK_TX_EN			BIT(6)
#define     PCIE_CORE_ERR_CAPCTL_ECRC_CHECK			BIT(7)
#define     PCIE_CORE_ERR_CAPCTL_ECRC_CHECK_RCV			BIT(8)

/* PIO registers base address and register offsets */
#define PIO_BASE_ADDR				0x4000
#define PIO_CTRL				(PIO_BASE_ADDR + 0x0)
#define   PIO_CTRL_TYPE_MASK			GENMASK(3, 0)
#define   PIO_CTRL_ADDR_WIN_DISABLE		BIT(24)
#define PIO_STAT				(PIO_BASE_ADDR + 0x4)
#define   PIO_COMPLETION_STATUS_SHIFT		7
#define   PIO_COMPLETION_STATUS_MASK		GENMASK(9, 7)
#define   PIO_COMPLETION_STATUS_OK		0
#define   PIO_COMPLETION_STATUS_UR		1
#define   PIO_COMPLETION_STATUS_CRS		2
#define   PIO_COMPLETION_STATUS_CA		4
#define   PIO_NON_POSTED_REQ			BIT(10)
#define   PIO_ERR_STATUS			BIT(11)
#define PIO_ADDR_LS				(PIO_BASE_ADDR + 0x8)
#define PIO_ADDR_MS				(PIO_BASE_ADDR + 0xc)
#define PIO_WR_DATA				(PIO_BASE_ADDR + 0x10)
#define PIO_WR_DATA_STRB			(PIO_BASE_ADDR + 0x14)
#define PIO_RD_DATA				(PIO_BASE_ADDR + 0x18)
#define PIO_START				(PIO_BASE_ADDR + 0x1c)
#define PIO_ISR					(PIO_BASE_ADDR + 0x20)

/* Aardvark Control registers */
#define CONTROL_BASE_ADDR			0x4800
#define PCIE_CORE_CTRL0_REG			(CONTROL_BASE_ADDR + 0x0)
#define     PCIE_GEN_SEL_MSK			0x3
#define     PCIE_GEN_SEL_SHIFT			0x0
#define     SPEED_GEN_1				0
#define     SPEED_GEN_2				1
#define     SPEED_GEN_3				2
#define     IS_RC_MSK				1
#define     IS_RC_SHIFT				2
#define     LANE_CNT_MSK			0x18
#define     LANE_CNT_SHIFT			0x3
#define     LANE_COUNT_1			(0 << LANE_CNT_SHIFT)
#define     LANE_COUNT_2			(1 << LANE_CNT_SHIFT)
#define     LANE_COUNT_4			(2 << LANE_CNT_SHIFT)
#define     LANE_COUNT_8			(3 << LANE_CNT_SHIFT)
#define     LINK_TRAINING_EN			BIT(6)
#define PCIE_CORE_CTRL2_REG			(CONTROL_BASE_ADDR + 0x8)
#define     PCIE_CORE_CTRL2_RESERVED		0x7
#define     PCIE_CORE_CTRL2_TD_ENABLE		BIT(4)
#define     PCIE_CORE_CTRL2_STRICT_ORDER_ENABLE	BIT(5)
#define     PCIE_CORE_CTRL2_ADDRWIN_MAP_ENABLE	BIT(6)

/* PCIe window configuration */
#define OB_WIN_BASE_ADDR			0x4c00
#define OB_WIN_BLOCK_SIZE			0x20
#define OB_WIN_COUNT				8
#define OB_WIN_REG_ADDR(win, offset)		(OB_WIN_BASE_ADDR + \
						 OB_WIN_BLOCK_SIZE * (win) + \
						 (offset))
#define OB_WIN_MATCH_LS(win)			OB_WIN_REG_ADDR(win, 0x00)
#define     OB_WIN_ENABLE			BIT(0)
#define OB_WIN_MATCH_MS(win)			OB_WIN_REG_ADDR(win, 0x04)
#define OB_WIN_REMAP_LS(win)			OB_WIN_REG_ADDR(win, 0x08)
#define OB_WIN_REMAP_MS(win)			OB_WIN_REG_ADDR(win, 0x0c)
#define OB_WIN_MASK_LS(win)			OB_WIN_REG_ADDR(win, 0x10)
#define OB_WIN_MASK_MS(win)			OB_WIN_REG_ADDR(win, 0x14)
#define OB_WIN_ACTIONS(win)			OB_WIN_REG_ADDR(win, 0x18)
#define OB_WIN_DEFAULT_ACTIONS			(OB_WIN_ACTIONS(OB_WIN_COUNT-1) + 0x4)
#define     OB_WIN_FUNC_NUM_MASK		GENMASK(31, 24)
#define     OB_WIN_FUNC_NUM_SHIFT		24
#define     OB_WIN_FUNC_NUM_ENABLE		BIT(23)
#define     OB_WIN_BUS_NUM_BITS_MASK		GENMASK(22, 20)
#define     OB_WIN_BUS_NUM_BITS_SHIFT		20
#define     OB_WIN_MSG_CODE_ENABLE		BIT(22)
#define     OB_WIN_MSG_CODE_MASK		GENMASK(21, 14)
#define     OB_WIN_MSG_CODE_SHIFT		14
#define     OB_WIN_MSG_PAYLOAD_LEN		BIT(12)
#define     OB_WIN_ATTR_ENABLE			BIT(11)
#define     OB_WIN_ATTR_TC_MASK			GENMASK(10, 8)
#define     OB_WIN_ATTR_TC_SHIFT		8
#define     OB_WIN_ATTR_RELAXED			BIT(7)
#define     OB_WIN_ATTR_NOSNOOP			BIT(6)
#define     OB_WIN_ATTR_POISON			BIT(5)
#define     OB_WIN_ATTR_IDO			BIT(4)
#define     OB_WIN_TYPE_MASK			GENMASK(3, 0)
#define     OB_WIN_TYPE_SHIFT			0
#define     OB_WIN_TYPE_MEM			0x0
#define     OB_WIN_TYPE_IO			0x4
#define     OB_WIN_TYPE_CONFIG_TYPE0		0x8
#define     OB_WIN_TYPE_CONFIG_TYPE1		0x9
#define     OB_WIN_TYPE_MSG			0xc

/* LMI registers base address and register offsets */
#define LMI_BASE_ADDR				0x6000
#define CFG_REG					(LMI_BASE_ADDR + 0x0)
#define     LTSSM_SHIFT				24
#define     LTSSM_MASK				0x3f
#define     LTSSM_L0				0x10
#define     LTSSM_DISABLED			0x20
#define VENDOR_ID_REG				(LMI_BASE_ADDR + 0x44)

/* PCIe core controller registers */
#define CTRL_CORE_BASE_ADDR			0x18000
#define CTRL_CONFIG_REG				(CTRL_CORE_BASE_ADDR + 0x0)
#define     CTRL_MODE_SHIFT			0x0
#define     CTRL_MODE_MASK			0x1
#define     PCIE_CORE_MODE_DIRECT		0x0
#define     PCIE_CORE_MODE_COMMAND		0x1

/* Transaction types */
#define PCIE_CONFIG_RD_TYPE0			0x8
#define PCIE_CONFIG_RD_TYPE1			0x9
#define PCIE_CONFIG_WR_TYPE0			0xa
#define PCIE_CONFIG_WR_TYPE1			0xb

/* PCI_BDF shifts 8bit, so we need extra 4bit shift */
#define PCIE_BDF(b, d, f)			(PCI_BDF(b, d, f) << 4)
#define PCIE_CONF_BUS(bus)			(((bus) & 0xff) << 20)
#define PCIE_CONF_DEV(dev)			(((dev) & 0x1f) << 15)
#define PCIE_CONF_FUNC(fun)			(((fun) & 0x7)	<< 12)
#define PCIE_CONF_REG(reg)			((reg) & 0xffc)
#define PCIE_CONF_ADDR(bus, devfn, where)	\
	(PCIE_CONF_BUS(bus) | PCIE_CONF_DEV(PCI_SLOT(devfn))	| \
	 PCIE_CONF_FUNC(PCI_FUNC(devfn)) | PCIE_CONF_REG(where))

/* PCIe Retries & Timeout definitions */
#define PIO_MAX_RETRIES				1500
#define PIO_WAIT_TIMEOUT			1000
#define LINK_MAX_RETRIES			10
#define LINK_WAIT_TIMEOUT			100000

#define CFG_RD_CRS_VAL			0xFFFF0001

/**
 * struct pcie_advk - Advk PCIe controller state
 *
 * @base:        The base address of the register space.
 * @first_busno: Bus number of the PCIe root-port.
 *               This may vary depending on the PCIe setup.
 * @sec_busno:   Bus number for the device behind the PCIe root-port.
 * @dev:         The pointer to PCI uclass device.
 * @reset_gpio:  GPIO descriptor for PERST.
 * @cfgcache:    Buffer for emulation of PCIe Root Port's PCI Bridge registers
 *               that are not available on Aardvark.
 * @cfgcrssve:   For CRSSVE emulation.
 */
struct pcie_advk {
	void			*base;
	int			first_busno;
	int			sec_busno;
	struct udevice		*dev;
	struct gpio_desc	reset_gpio;
	u32			cfgcache[(0x3c - 0x10) / 4];
	bool			cfgcrssve;
};

static inline void advk_writel(struct pcie_advk *pcie, uint val, uint reg)
{
	writel(val, pcie->base + reg);
}

static inline uint advk_readl(struct pcie_advk *pcie, uint reg)
{
	return readl(pcie->base + reg);
}

/**
 * pcie_advk_addr_valid() - Check for valid bus address
 *
 * @pcie: Pointer to the PCI bus
 * @busno: Bus number of PCI device
 * @dev: Device number of PCI device
 * @func: Function number of PCI device
 * @bdf: The PCI device to access
 *
 * Return: true on valid, false on invalid
 */
static bool pcie_advk_addr_valid(struct pcie_advk *pcie,
				 int busno, u8 dev, u8 func)
{
	/* On the primary (local) bus there is only one PCI Bridge */
	if (busno == pcie->first_busno && (dev != 0 || func != 0))
		return false;

	/*
	 * In PCI-E only a single device (0) can exist on the secondary bus.
	 * Beyond the secondary bus, there might be a Switch and anything is
	 * possible.
	 */
	if (busno == pcie->sec_busno && dev != 0)
		return false;

	return true;
}

/**
 * pcie_advk_wait_pio() - Wait for PIO access to be accomplished
 *
 * @pcie: The PCI device to access
 *
 * Wait up to 1.5 seconds for PIO access to be accomplished.
 *
 * Return positive - retry count if PIO access is accomplished.
 * Return negative - error if PIO access is timed out.
 */
static int pcie_advk_wait_pio(struct pcie_advk *pcie)
{
	uint start, isr;
	uint count;

	for (count = 1; count <= PIO_MAX_RETRIES; count++) {
		start = advk_readl(pcie, PIO_START);
		isr = advk_readl(pcie, PIO_ISR);
		if (!start && isr)
			return count;
		/*
		 * Do not check the PIO state too frequently,
		 * 100us delay is appropriate.
		 */
		udelay(PIO_WAIT_TIMEOUT);
	}

	dev_err(pcie->dev, "PIO read/write transfer time out\n");
	return -ETIMEDOUT;
}

/**
 * pcie_advk_check_pio_status() - Validate PIO status and get the read result
 *
 * @pcie: Pointer to the PCI bus
 * @allow_crs: Only for read requests, if CRS response is allowed
 * @read_val: Pointer to the read result
 *
 * Return: 0 on success
 */
static int pcie_advk_check_pio_status(struct pcie_advk *pcie,
				      bool allow_crs,
				      uint *read_val)
{
	int ret;
	uint reg;
	unsigned int status;
	char *strcomp_status, *str_posted;

	reg = advk_readl(pcie, PIO_STAT);
	status = (reg & PIO_COMPLETION_STATUS_MASK) >>
		PIO_COMPLETION_STATUS_SHIFT;

	switch (status) {
	case PIO_COMPLETION_STATUS_OK:
		if (reg & PIO_ERR_STATUS) {
			strcomp_status = "COMP_ERR";
			ret = -EFAULT;
			break;
		}
		/* Get the read result */
		if (read_val)
			*read_val = advk_readl(pcie, PIO_RD_DATA);
		/* No error */
		strcomp_status = NULL;
		ret = 0;
		break;
	case PIO_COMPLETION_STATUS_UR:
		strcomp_status = "UR";
		ret = -EOPNOTSUPP;
		break;
	case PIO_COMPLETION_STATUS_CRS:
		if (allow_crs && read_val) {
			/* For reading, CRS is not an error status. */
			*read_val = CFG_RD_CRS_VAL;
			strcomp_status = NULL;
			ret = 0;
		} else {
			strcomp_status = "CRS";
			ret = -EAGAIN;
		}
		break;
	case PIO_COMPLETION_STATUS_CA:
		strcomp_status = "CA";
		ret = -ECANCELED;
		break;
	default:
		strcomp_status = "Unknown";
		ret = -EINVAL;
		break;
	}

	if (!strcomp_status)
		return ret;

	if (reg & PIO_NON_POSTED_REQ)
		str_posted = "Non-posted";
	else
		str_posted = "Posted";

	dev_dbg(pcie->dev, "%s PIO Response Status: %s, %#x @ %#x\n",
		str_posted, strcomp_status, reg,
		advk_readl(pcie, PIO_ADDR_LS));

	return ret;
}

/**
 * pcie_advk_read_config() - Read from configuration space
 *
 * @bus: Pointer to the PCI bus
 * @bdf: Identifies the PCIe device to access
 * @offset: The offset into the device's configuration space
 * @valuep: A pointer at which to store the read value
 * @size: Indicates the size of access to perform
 *
 * Read a value of size @size from offset @offset within the configuration
 * space of the device identified by the bus, device & function numbers in @bdf
 * on the PCI bus @bus.
 *
 * Return: 0 on success
 */
static int pcie_advk_read_config(const struct udevice *bus, pci_dev_t bdf,
				 uint offset, ulong *valuep,
				 enum pci_size_t size)
{
	struct pcie_advk *pcie = dev_get_priv(bus);
	int busno = PCI_BUS(bdf) - dev_seq(bus);
	int retry_count;
	bool allow_crs;
	ulong data;
	uint reg;
	int ret;

	dev_dbg(pcie->dev, "PCIE CFG read:  (b,d,f)=(%2d,%2d,%2d) ",
		PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));

	if (!pcie_advk_addr_valid(pcie, busno, PCI_DEV(bdf), PCI_FUNC(bdf))) {
		dev_dbg(pcie->dev, "- out of range\n");
		*valuep = pci_get_ff(size);
		return 0;
	}

	/*
	 * The configuration space of the PCI Bridge on primary (first) bus is
	 * not accessible via PIO transfers like all other PCIe devices. PCI
	 * Bridge config registers are available directly in Aardvark memory
	 * space starting at offset zero. The PCI Bridge config space is of
	 * Type 0, but the BAR registers (including ROM BAR) don't have the same
	 * meaning as in the PCIe specification. Therefore do not access BAR
	 * registers and non-common registers (those which have different
	 * meaning for Type 0 and Type 1 config space) of the primary PCI Bridge
	 * and instead read their content from driver virtual cfgcache[].
	 */
	if (busno == pcie->first_busno) {
		if ((offset >= 0x10 && offset < 0x34) || (offset >= 0x38 && offset < 0x3c))
			data = pcie->cfgcache[(offset - 0x10) / 4];
		else
			data = advk_readl(pcie, offset & ~3);

		if ((offset & ~3) == (PCI_HEADER_TYPE & ~3)) {
			/*
			 * Change Header Type of PCI Bridge device to Type 1
			 * (0x01, used by PCI Bridges) because hardwired value
			 * is Type 0 (0x00, used by Endpoint devices).
			 */
			data &= ~0x007f0000;
			data |= PCI_HEADER_TYPE_BRIDGE << 16;
		}

		if ((offset & ~3) == PCIE_CORE_PCIEXP_CAP_OFF + PCI_EXP_RTCTL) {
			/* CRSSVE bit is stored only in cache */
			if (pcie->cfgcrssve)
				data |= PCI_EXP_RTCTL_CRSSVE;
		}

		if ((offset & ~3) == PCIE_CORE_PCIEXP_CAP_OFF +
				     (PCI_EXP_RTCAP & ~3)) {
			/* CRS is emulated below, so set CRSVIS capability */
			data |= PCI_EXP_RTCAP_CRSVIS << 16;
		}

		*valuep = pci_conv_32_to_size(data, offset, size);

		return 0;
	}

	/*
	 * Returning fabricated CRS value (0xFFFF0001) by PCIe Root Complex to
	 * OS is allowed only for 4-byte PCI_VENDOR_ID config read request and
	 * only when CRSSVE bit in Root Port PCIe device is enabled. In all
	 * other error PCIe Root Complex must return all-ones.
	 *
	 * U-Boot currently does not support handling of CRS return value for
	 * PCI_VENDOR_ID config read request and also does not set CRSSVE bit.
	 * So it means that pcie->cfgcrssve is false. But the code is prepared
	 * for returning CRS, so that if U-Boot does support CRS in the future,
	 * it will work for Aardvark.
	 */
	allow_crs = (offset == PCI_VENDOR_ID) && (size == PCI_SIZE_32) && pcie->cfgcrssve;

	if (advk_readl(pcie, PIO_START)) {
		dev_err(pcie->dev,
			"Previous PIO read/write transfer is still running\n");
		if (allow_crs) {
			*valuep = CFG_RD_CRS_VAL;
			return 0;
		}
		*valuep = pci_get_ff(size);
		return -EAGAIN;
	}

	/* Program the control register */
	reg = advk_readl(pcie, PIO_CTRL);
	reg &= ~PIO_CTRL_TYPE_MASK;
	if (busno == pcie->sec_busno)
		reg |= PCIE_CONFIG_RD_TYPE0;
	else
		reg |= PCIE_CONFIG_RD_TYPE1;
	advk_writel(pcie, reg, PIO_CTRL);

	/* Program the address registers */
	reg = PCIE_BDF(busno, PCI_DEV(bdf), PCI_FUNC(bdf)) | PCIE_CONF_REG(offset);
	advk_writel(pcie, reg, PIO_ADDR_LS);
	advk_writel(pcie, 0, PIO_ADDR_MS);

	/* Program the data strobe */
	advk_writel(pcie, 0xf, PIO_WR_DATA_STRB);

	retry_count = 0;

retry:
	/* Start the transfer */
	advk_writel(pcie, 1, PIO_ISR);
	advk_writel(pcie, 1, PIO_START);

	ret = pcie_advk_wait_pio(pcie);
	if (ret < 0) {
		if (allow_crs) {
			*valuep = CFG_RD_CRS_VAL;
			return 0;
		}
		*valuep = pci_get_ff(size);
		return ret;
	}

	retry_count += ret;

	/* Check PIO status and get the read result */
	ret = pcie_advk_check_pio_status(pcie, allow_crs, &reg);
	if (ret == -EAGAIN && retry_count < PIO_MAX_RETRIES)
		goto retry;
	if (ret) {
		*valuep = pci_get_ff(size);
		return ret;
	}

	dev_dbg(pcie->dev, "(addr,size,val)=(0x%04x, %d, 0x%08x)\n",
		offset, size, reg);
	*valuep = pci_conv_32_to_size(reg, offset, size);

	return 0;
}

/**
 * pcie_calc_datastrobe() - Calculate data strobe
 *
 * @offset: The offset into the device's configuration space
 * @size: Indicates the size of access to perform
 *
 * Calculate data strobe according to offset and size
 *
 */
static uint pcie_calc_datastrobe(uint offset, enum pci_size_t size)
{
	uint bytes, data_strobe;

	switch (size) {
	case PCI_SIZE_8:
		bytes = 1;
		break;
	case PCI_SIZE_16:
		bytes = 2;
		break;
	default:
		bytes = 4;
	}

	data_strobe = GENMASK(bytes - 1, 0) << (offset & 0x3);

	return data_strobe;
}

/**
 * pcie_advk_write_config() - Write to configuration space
 *
 * @bus: Pointer to the PCI bus
 * @bdf: Identifies the PCIe device to access
 * @offset: The offset into the device's configuration space
 * @value: The value to write
 * @size: Indicates the size of access to perform
 *
 * Write the value @value of size @size from offset @offset within the
 * configuration space of the device identified by the bus, device & function
 * numbers in @bdf on the PCI bus @bus.
 *
 * Return: 0 on success
 */
static int pcie_advk_write_config(struct udevice *bus, pci_dev_t bdf,
				  uint offset, ulong value,
				  enum pci_size_t size)
{
	struct pcie_advk *pcie = dev_get_priv(bus);
	int busno = PCI_BUS(bdf) - dev_seq(bus);
	int retry_count;
	ulong data;
	uint reg;
	int ret;

	dev_dbg(pcie->dev, "PCIE CFG write: (b,d,f)=(%2d,%2d,%2d) ",
		PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));
	dev_dbg(pcie->dev, "(addr,size,val)=(0x%04x, %d, 0x%08lx)\n",
		offset, size, value);

	if (!pcie_advk_addr_valid(pcie, busno, PCI_DEV(bdf), PCI_FUNC(bdf))) {
		dev_dbg(pcie->dev, "- out of range\n");
		return 0;
	}

	/*
	 * As explained in pcie_advk_read_config(), PCI Bridge config registers
	 * are available directly in Aardvark memory space starting at offset
	 * zero. Type 1 specific registers are not available, so we write their
	 * content only into driver virtual cfgcache[].
	 */
	if (busno == pcie->first_busno) {
		if ((offset >= 0x10 && offset < 0x34) ||
		    (offset >= 0x38 && offset < 0x3c)) {
			data = pcie->cfgcache[(offset - 0x10) / 4];
			data = pci_conv_size_to_32(data, value, offset, size);
			/* This PCI bridge does not have configurable bars */
			if ((offset & ~3) == PCI_BASE_ADDRESS_0 ||
			    (offset & ~3) == PCI_BASE_ADDRESS_1 ||
			    (offset & ~3) == PCI_ROM_ADDRESS1)
				data = 0x0;
			pcie->cfgcache[(offset - 0x10) / 4] = data;
		} else {
			data = advk_readl(pcie, offset & ~3);
			data = pci_conv_size_to_32(data, value, offset, size);
			advk_writel(pcie, data, offset & ~3);
		}

		if (offset == PCI_PRIMARY_BUS)
			pcie->first_busno = data & 0xff;

		if (offset == PCI_SECONDARY_BUS ||
		    (offset == PCI_PRIMARY_BUS && size != PCI_SIZE_8))
			pcie->sec_busno = (data >> 8) & 0xff;

		if ((offset & ~3) == PCIE_CORE_PCIEXP_CAP_OFF + PCI_EXP_RTCTL)
			pcie->cfgcrssve = data & PCI_EXP_RTCTL_CRSSVE;

		return 0;
	}

	if (advk_readl(pcie, PIO_START)) {
		dev_err(pcie->dev,
			"Previous PIO read/write transfer is still running\n");
		return -EAGAIN;
	}

	/* Program the control register */
	reg = advk_readl(pcie, PIO_CTRL);
	reg &= ~PIO_CTRL_TYPE_MASK;
	if (busno == pcie->sec_busno)
		reg |= PCIE_CONFIG_WR_TYPE0;
	else
		reg |= PCIE_CONFIG_WR_TYPE1;
	advk_writel(pcie, reg, PIO_CTRL);

	/* Program the address registers */
	reg = PCIE_BDF(busno, PCI_DEV(bdf), PCI_FUNC(bdf)) | PCIE_CONF_REG(offset);
	advk_writel(pcie, reg, PIO_ADDR_LS);
	advk_writel(pcie, 0, PIO_ADDR_MS);
	dev_dbg(pcie->dev, "\tPIO req. - addr = 0x%08x\n", reg);

	/* Program the data register */
	reg = pci_conv_size_to_32(0, value, offset, size);
	advk_writel(pcie, reg, PIO_WR_DATA);
	dev_dbg(pcie->dev, "\tPIO req. - val  = 0x%08x\n", reg);

	/* Program the data strobe */
	reg = pcie_calc_datastrobe(offset, size);
	advk_writel(pcie, reg, PIO_WR_DATA_STRB);
	dev_dbg(pcie->dev, "\tPIO req. - strb = 0x%02x\n", reg);

	retry_count = 0;

retry:
	/* Start the transfer */
	advk_writel(pcie, 1, PIO_ISR);
	advk_writel(pcie, 1, PIO_START);

	ret = pcie_advk_wait_pio(pcie);
	if (ret < 0)
		return ret;

	retry_count += ret;

	/* Check PIO status */
	ret = pcie_advk_check_pio_status(pcie, false, NULL);
	if (ret == -EAGAIN && retry_count < PIO_MAX_RETRIES)
		goto retry;
	return ret;
}

/**
 * pcie_advk_link_up() - Check if PCIe link is up or not
 *
 * @pcie: The PCI device to access
 *
 * Return 1 (true) on link up.
 * Return 0 (false) on link down.
 */
static int pcie_advk_link_up(struct pcie_advk *pcie)
{
	u32 val, ltssm_state;

	val = advk_readl(pcie, CFG_REG);
	ltssm_state = (val >> LTSSM_SHIFT) & LTSSM_MASK;
	return ltssm_state >= LTSSM_L0 && ltssm_state < LTSSM_DISABLED;
}

/**
 * pcie_advk_wait_for_link() - Wait for link training to be accomplished
 *
 * @pcie: The PCI device to access
 *
 * Wait up to 1 second for link training to be accomplished.
 *
 * Return 1 (true) if link training ends up with link up success.
 * Return 0 (false) if link training ends up with link up failure.
 */
static int pcie_advk_wait_for_link(struct pcie_advk *pcie)
{
	int retries;

	/* check if the link is up or not */
	for (retries = 0; retries < LINK_MAX_RETRIES; retries++) {
		if (pcie_advk_link_up(pcie)) {
			printf("PCIe: Link up\n");
			return 0;
		}

		udelay(LINK_WAIT_TIMEOUT);
	}

	printf("PCIe: Link down\n");

	return -ETIMEDOUT;
}

/*
 * Set PCIe address window register which could be used for memory
 * mapping.
 */
static void pcie_advk_set_ob_win(struct pcie_advk *pcie, u8 win_num,
				 phys_addr_t match, phys_addr_t remap,
				 phys_addr_t mask, u32 actions)
{
	advk_writel(pcie, OB_WIN_ENABLE |
			  lower_32_bits(match), OB_WIN_MATCH_LS(win_num));
	advk_writel(pcie, upper_32_bits(match), OB_WIN_MATCH_MS(win_num));
	advk_writel(pcie, lower_32_bits(remap), OB_WIN_REMAP_LS(win_num));
	advk_writel(pcie, upper_32_bits(remap), OB_WIN_REMAP_MS(win_num));
	advk_writel(pcie, lower_32_bits(mask), OB_WIN_MASK_LS(win_num));
	advk_writel(pcie, upper_32_bits(mask), OB_WIN_MASK_MS(win_num));
	advk_writel(pcie, actions, OB_WIN_ACTIONS(win_num));
}

static void pcie_advk_disable_ob_win(struct pcie_advk *pcie, u8 win_num)
{
	advk_writel(pcie, 0, OB_WIN_MATCH_LS(win_num));
	advk_writel(pcie, 0, OB_WIN_MATCH_MS(win_num));
	advk_writel(pcie, 0, OB_WIN_REMAP_LS(win_num));
	advk_writel(pcie, 0, OB_WIN_REMAP_MS(win_num));
	advk_writel(pcie, 0, OB_WIN_MASK_LS(win_num));
	advk_writel(pcie, 0, OB_WIN_MASK_MS(win_num));
	advk_writel(pcie, 0, OB_WIN_ACTIONS(win_num));
}

static void pcie_advk_set_ob_region(struct pcie_advk *pcie, int *wins,
				    struct pci_region *region, u32 actions)
{
	phys_addr_t phys_start = region->phys_start;
	pci_addr_t bus_start = region->bus_start;
	pci_size_t size = region->size;
	phys_addr_t win_mask;
	u64 win_size;

	if (*wins == -1)
		return;

	/*
	 * The n-th PCIe window is configured by tuple (match, remap, mask)
	 * and an access to address A uses this window if A matches the
	 * match with given mask.
	 * So every PCIe window size must be a power of two and every start
	 * address must be aligned to window size. Minimal size is 64 KiB
	 * because lower 16 bits of mask must be zero. Remapped address
	 * may have set only bits from the mask.
	 */
	while (*wins < OB_WIN_COUNT && size > 0) {
		/* Calculate the largest aligned window size */
		win_size = (1ULL << (fls64(size) - 1)) |
			   (phys_start ? (1ULL << __ffs64(phys_start)) : 0);
		win_size = 1ULL << __ffs64(win_size);
		win_mask = ~(win_size - 1);
		if (win_size < 0x10000 || (bus_start & ~win_mask))
			break;

		dev_dbg(pcie->dev,
			"Configuring PCIe window %d: [0x%llx-0x%llx] as 0x%x\n",
			*wins, (u64)phys_start, (u64)phys_start + win_size,
			actions);
		pcie_advk_set_ob_win(pcie, *wins, phys_start, bus_start,
				     win_mask, actions);

		phys_start += win_size;
		bus_start += win_size;
		size -= win_size;
		(*wins)++;
	}

	if (size > 0) {
		*wins = -1;
		dev_err(pcie->dev,
			"Invalid PCIe region [0x%llx-0x%llx]\n",
			(u64)region->phys_start,
			(u64)region->phys_start + region->size);
	}
}

/**
 * pcie_advk_setup_hw() - PCIe initailzation
 *
 * @pcie: The PCI device to access
 *
 * Return: 0 on success
 */
static int pcie_advk_setup_hw(struct pcie_advk *pcie)
{
	struct pci_region *io, *mem, *pref;
	int i, wins;
	u32 reg;

	/* Set to Direct mode */
	reg = advk_readl(pcie, CTRL_CONFIG_REG);
	reg &= ~(CTRL_MODE_MASK << CTRL_MODE_SHIFT);
	reg |= ((PCIE_CORE_MODE_DIRECT & CTRL_MODE_MASK) << CTRL_MODE_SHIFT);
	advk_writel(pcie, reg, CTRL_CONFIG_REG);

	/* Set PCI global control register to RC mode */
	reg = advk_readl(pcie, PCIE_CORE_CTRL0_REG);
	reg |= (IS_RC_MSK << IS_RC_SHIFT);
	advk_writel(pcie, reg, PCIE_CORE_CTRL0_REG);

	/*
	 * Replace incorrect PCI vendor id value 0x1b4b by correct value 0x11ab.
	 * VENDOR_ID_REG contains vendor id in low 16 bits and subsystem vendor
	 * id in high 16 bits. Updating this register changes readback value of
	 * read-only vendor id bits in PCIE_CORE_DEV_ID_REG register. Workaround
	 * for erratum 4.1: "The value of device and vendor ID is incorrect".
	 */
	advk_writel(pcie, 0x11ab11ab, VENDOR_ID_REG);

	/*
	 * Change Class Code of PCI Bridge device to PCI Bridge (0x600400),
	 * because default value is Mass Storage Controller (0x010400), causing
	 * U-Boot to fail to recognize it as P2P Bridge.
	 *
	 * Note that this Aardvark PCI Bridge does not have a compliant Type 1
	 * Configuration Space and it even cannot be accessed via Aardvark's
	 * PCI config space access method. Aardvark PCI Bridge Config space is
	 * available in internal Aardvark registers starting at offset 0x0
	 * and has format of Type 0 config space.
	 *
	 * Moreover Type 0 BAR registers (ranges 0x10 - 0x28 and 0x30 - 0x34)
	 * have the same format in Marvell's specification as in PCIe
	 * specification, but their meaning is totally different (and not even
	 * the same meaning as explained in the corresponding comment in the
	 * pci_mvebu driver; aardvark is still different).
	 *
	 * So our driver converts Type 0 config space to Type 1 and reports
	 * Header Type as Type 1. Access to BAR registers and to non-existent
	 * Type 1 registers is redirected to the virtual cfgcache[] buffer,
	 * which avoids changing unrelated registers.
	 */
	reg = advk_readl(pcie, PCIE_CORE_DEV_REV_REG);
	reg &= ~0xffffff00;
	reg |= (PCI_CLASS_BRIDGE_PCI << 8) << 8;
	advk_writel(pcie, reg, PCIE_CORE_DEV_REV_REG);

	/* Set Advanced Error Capabilities and Control PF0 register */
	reg = PCIE_CORE_ERR_CAPCTL_ECRC_CHK_TX |
		PCIE_CORE_ERR_CAPCTL_ECRC_CHK_TX_EN |
		PCIE_CORE_ERR_CAPCTL_ECRC_CHECK |
		PCIE_CORE_ERR_CAPCTL_ECRC_CHECK_RCV;
	advk_writel(pcie, reg, PCIE_CORE_ERR_CAPCTL_REG);

	/* Set PCIe Device Control and Status 1 PF0 register */
	reg = PCIE_CORE_DEV_CTRL_STATS_RELAX_ORDER_DISABLE |
		(PCIE_CORE_DEV_CTRL_STATS_MAX_PAYLOAD_SIZE <<
		 PCIE_CORE_DEV_CTRL_STATS_MAX_PAYLOAD_SIZE_SHIFT) |
		(PCIE_CORE_DEV_CTRL_STATS_MAX_RD_REQ_SIZE <<
		 PCIE_CORE_DEV_CTRL_STATS_MAX_RD_REQ_SIZE_SHIFT) |
		PCIE_CORE_DEV_CTRL_STATS_SNOOP_DISABLE;
	advk_writel(pcie, reg, PCIE_CORE_DEV_CTRL_STATS_REG);

	/* Program PCIe Control 2 to disable strict ordering */
	reg = PCIE_CORE_CTRL2_RESERVED |
		PCIE_CORE_CTRL2_TD_ENABLE;
	advk_writel(pcie, reg, PCIE_CORE_CTRL2_REG);

	/* Set GEN2 */
	reg = advk_readl(pcie, PCIE_CORE_CTRL0_REG);
	reg &= ~PCIE_GEN_SEL_MSK;
	reg |= SPEED_GEN_2;
	advk_writel(pcie, reg, PCIE_CORE_CTRL0_REG);

	/* Set lane X1 */
	reg = advk_readl(pcie, PCIE_CORE_CTRL0_REG);
	reg &= ~LANE_CNT_MSK;
	reg |= LANE_COUNT_1;
	advk_writel(pcie, reg, PCIE_CORE_CTRL0_REG);

	/* Enable link training */
	reg = advk_readl(pcie, PCIE_CORE_CTRL0_REG);
	reg |= LINK_TRAINING_EN;
	advk_writel(pcie, reg, PCIE_CORE_CTRL0_REG);

	/*
	 * Enable AXI address window location generation:
	 * When it is enabled, the default outbound window
	 * configurations (Default User Field: 0xD0074CFC)
	 * are used to transparent address translation for
	 * the outbound transactions. Thus, PCIe address
	 * windows are not required for transparent memory
	 * access when default outbound window configuration
	 * is set for memory access.
	 */
	reg = advk_readl(pcie, PCIE_CORE_CTRL2_REG);
	reg |= PCIE_CORE_CTRL2_ADDRWIN_MAP_ENABLE;
	advk_writel(pcie, reg, PCIE_CORE_CTRL2_REG);

	/*
	 * Bypass the address window mapping for PIO:
	 * Since PIO access already contains all required
	 * info over AXI interface by PIO registers, the
	 * address window is not required.
	 */
	reg = advk_readl(pcie, PIO_CTRL);
	reg |= PIO_CTRL_ADDR_WIN_DISABLE;
	advk_writel(pcie, reg, PIO_CTRL);

	/*
	 * Set memory access in Default User Field so it
	 * is not required to configure PCIe address for
	 * transparent memory access.
	 */
	advk_writel(pcie, OB_WIN_TYPE_MEM, OB_WIN_DEFAULT_ACTIONS);

	/*
	 * Configure PCIe address windows for non-memory or
	 * non-transparent access as by default PCIe uses
	 * transparent memory access.
	 */
	wins = 0;
	pci_get_regions(pcie->dev, &io, &mem, &pref);
	if (io)
		pcie_advk_set_ob_region(pcie, &wins, io, OB_WIN_TYPE_IO);
	if (mem && mem->phys_start != mem->bus_start)
		pcie_advk_set_ob_region(pcie, &wins, mem, OB_WIN_TYPE_MEM);
	if (pref && pref->phys_start != pref->bus_start)
		pcie_advk_set_ob_region(pcie, &wins, pref, OB_WIN_TYPE_MEM);

	/* Disable remaining PCIe outbound windows */
	for (i = ((wins >= 0) ? wins : 0); i < OB_WIN_COUNT; i++)
		pcie_advk_disable_ob_win(pcie, i);

	if (wins == -1)
		return -EINVAL;

	/* Wait for PCIe link up */
	if (pcie_advk_wait_for_link(pcie))
		return -ENXIO;

	return 0;
}

/**
 * pcie_advk_probe() - Probe the PCIe bus for active link
 *
 * @dev: A pointer to the device being operated on
 *
 * Probe for an active link on the PCIe bus and configure the controller
 * to enable this port.
 *
 * Return: 0 on success, else -ENODEV
 */
static int pcie_advk_probe(struct udevice *dev)
{
	struct pcie_advk *pcie = dev_get_priv(dev);

	gpio_request_by_name(dev, "reset-gpios", 0, &pcie->reset_gpio,
			     GPIOD_IS_OUT);
	/*
	 * Issue reset to add-in card through the dedicated GPIO.
	 * Some boards are connecting the card reset pin to common system
	 * reset wire and others are using separate GPIO port.
	 * In the last case we have to release a reset of the addon card
	 * using this GPIO.
	 *
	 * FIX-ME:
	 *     The PCIe RESET signal is not supposed to be released along
	 *     with the SOC RESET signal. It should be lowered as early as
	 *     possible before PCIe PHY initialization. Moreover, the PCIe
	 *     clock should be gated as well.
	 */
	if (dm_gpio_is_valid(&pcie->reset_gpio)) {
		dev_dbg(dev, "Toggle PCIE Reset GPIO ...\n");
		dm_gpio_set_value(&pcie->reset_gpio, 1);
		mdelay(200);
		dm_gpio_set_value(&pcie->reset_gpio, 0);
	} else {
		dev_warn(dev, "PCIE Reset on GPIO support is missing\n");
	}

	pcie->dev = pci_get_controller(dev);

	/* PCI Bridge support 32-bit I/O and 64-bit prefetch mem addressing */
	pcie->cfgcache[(PCI_IO_BASE - 0x10) / 4] =
		PCI_IO_RANGE_TYPE_32 | (PCI_IO_RANGE_TYPE_32 << 8);
	pcie->cfgcache[(PCI_PREF_MEMORY_BASE - 0x10) / 4] =
		PCI_PREF_RANGE_TYPE_64 | (PCI_PREF_RANGE_TYPE_64 << 16);

	return pcie_advk_setup_hw(pcie);
}

static int pcie_advk_remove(struct udevice *dev)
{
	struct pcie_advk *pcie = dev_get_priv(dev);
	u32 reg;
	int i;

	for (i = 0; i < OB_WIN_COUNT; i++)
		pcie_advk_disable_ob_win(pcie, i);

	reg = advk_readl(pcie, PCIE_CORE_CMD_STATUS_REG);
	reg &= ~(PCIE_CORE_CMD_MEM_ACCESS_EN |
		 PCIE_CORE_CMD_IO_ACCESS_EN |
		 PCIE_CORE_CMD_MEM_IO_REQ_EN);
	advk_writel(pcie, reg, PCIE_CORE_CMD_STATUS_REG);

	reg = advk_readl(pcie, PCIE_CORE_CTRL0_REG);
	reg &= ~LINK_TRAINING_EN;
	advk_writel(pcie, reg, PCIE_CORE_CTRL0_REG);

	return 0;
}

/**
 * pcie_advk_of_to_plat() - Translate from DT to device state
 *
 * @dev: A pointer to the device being operated on
 *
 * Translate relevant data from the device tree pertaining to device @dev into
 * state that the driver will later make use of. This state is stored in the
 * device's private data structure.
 *
 * Return: 0 on success, else -EINVAL
 */
static int pcie_advk_of_to_plat(struct udevice *dev)
{
	struct pcie_advk *pcie = dev_get_priv(dev);

	/* Get the register base address */
	pcie->base = (void *)dev_read_addr_index(dev, 0);
	if ((fdt_addr_t)pcie->base == FDT_ADDR_T_NONE)
		return -EINVAL;

	return 0;
}

static const struct dm_pci_ops pcie_advk_ops = {
	.read_config	= pcie_advk_read_config,
	.write_config	= pcie_advk_write_config,
};

static const struct udevice_id pcie_advk_ids[] = {
	{ .compatible = "marvell,armada-3700-pcie" },
	{ }
};

U_BOOT_DRIVER(pcie_advk) = {
	.name			= "pcie_advk",
	.id			= UCLASS_PCI,
	.of_match		= pcie_advk_ids,
	.ops			= &pcie_advk_ops,
	.of_to_plat	= pcie_advk_of_to_plat,
	.probe			= pcie_advk_probe,
	.remove			= pcie_advk_remove,
	.flags			= DM_FLAG_OS_PREPARE,
	.priv_auto	= sizeof(struct pcie_advk),
};
