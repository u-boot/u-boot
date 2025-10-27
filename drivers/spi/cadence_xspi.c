// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2025
 * Altera Corporation <www.altera.com>
 */

#include <clk.h>
#include <dm.h>
#include <fdtdec.h>
#include <log.h>
#include <malloc.h>
#include <reset.h>
#include <spi.h>
#include <spi-mem.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/ioport.h>
#include <linux/sizes.h>
#include <linux/time.h>
#include "cadence_xspi.h"

static int cdns_xspi_wait_for_controller_idle(struct cdns_xspi_plat *cdns_xspi)
{
	u32 ctrl_stat;

	return readl_relaxed_poll_timeout(cdns_xspi->iobase +
					  CDNS_XSPI_CTRL_STATUS_REG,
					  ctrl_stat,
					  !(ctrl_stat &
					   CDNS_XSPI_CTRL_BUSY),
					   1000);
}

static int cdns_xspi_wait_for_sdma_complete(struct cdns_xspi_plat *cdns_xspi)
{
	u32 irq_status;
	int ret = 0;

	ret =  readl_relaxed_poll_timeout(cdns_xspi->iobase +
					  CDNS_XSPI_INTR_STATUS_REG,
					  irq_status,
					  (irq_status &
					   CDNS_XSPI_SDMA_TRIGGER),
					   1000);

	if (!ret) {
		/*
		 * SDMA return an interrupt,  need to clear
		 * the interrupt after read, wrtting 1 to clear the bit.
		 */
		setbits_le32(cdns_xspi->iobase + CDNS_XSPI_INTR_STATUS_REG,
			     CDNS_XSPI_SDMA_TRIGGER);
	}

	/* Check if SDMA ERROR happened */
	if (irq_status & CDNS_XSPI_SDMA_ERROR) {
		/*
		 * Need to clear the SDMA_ERROR interrupt
		 * after read, wrtting 1 to clear the bit.
		 */
		dev_err(cdns_xspi->dev,
			"Slave DMA transaction error\n");

		cdns_xspi->sdma_error = true;
		setbits_le32(cdns_xspi->iobase + CDNS_XSPI_INTR_STATUS_REG,
			     CDNS_XSPI_SDMA_ERROR);

		ret = -EIO;
	}

	return ret;
}

static int cdns_xspi_wait_for_cmd_complete(struct cdns_xspi_plat *cdns_xspi)
{
	u32 irq_status;
	int ret = 0;

	ret =  readl_relaxed_poll_timeout(cdns_xspi->iobase +
					  CDNS_XSPI_INTR_STATUS_REG,
					  irq_status,
					  (irq_status &
					   CDNS_XSPI_STIG_DONE),
					   100000);

	irq_status = readl(cdns_xspi->iobase + CDNS_XSPI_INTR_STATUS_REG);

	if (!ret) {
		/*
		 * Need to clear the interrupt after read,
		 * wrtting 1 to the clear the bit.
		 */
		writel(irq_status & CDNS_XSPI_STIG_DONE,
		       cdns_xspi->iobase + CDNS_XSPI_INTR_STATUS_REG);
	}

	return ret;
}

static void cdns_xspi_trigger_command(struct cdns_xspi_plat *cdns_xspi,
				      u32 cmd_regs[6])
{
	writel(cmd_regs[5], cdns_xspi->iobase + CDNS_XSPI_CMD_REG_5);
	writel(cmd_regs[4], cdns_xspi->iobase + CDNS_XSPI_CMD_REG_4);
	writel(cmd_regs[3], cdns_xspi->iobase + CDNS_XSPI_CMD_REG_3);
	writel(cmd_regs[2], cdns_xspi->iobase + CDNS_XSPI_CMD_REG_2);
	writel(cmd_regs[1], cdns_xspi->iobase + CDNS_XSPI_CMD_REG_1);
	writel(cmd_regs[0], cdns_xspi->iobase + CDNS_XSPI_CMD_REG_0);
}

static int cdns_xspi_check_command_status(struct cdns_xspi_plat *cdns_xspi)
{
	int ret = 0;
	u32 cmd_status = readl(cdns_xspi->iobase + CDNS_XSPI_CMD_STATUS_REG);

	/* Check if the command has completed */
	if (cmd_status & CDNS_XSPI_CMD_STATUS_COMPLETED) {
		/*Check for failure status and report each type of error */
		if ((cmd_status & CDNS_XSPI_CMD_STATUS_FAILED) != 0) {
			if (cmd_status & CDNS_XSPI_CMD_STATUS_DQS_ERROR)
				dev_err(cdns_xspi->dev,
					"Incorrect DQS pulses detected\n");

			if (cmd_status & CDNS_XSPI_CMD_STATUS_CRC_ERROR)
				dev_err(cdns_xspi->dev,
					"CRC error received\n");

			if (cmd_status & CDNS_XSPI_CMD_STATUS_BUS_ERROR)
				dev_err(cdns_xspi->dev,
					"Error resp on system DMA interface\n");

			if (cmd_status & CDNS_XSPI_CMD_STATUS_INV_SEQ_ERROR)
				dev_err(cdns_xspi->dev,
					"Invalid command sequence detected\n");

			ret = -EPROTO;
		}
	} else {
		/* Command did not complete at all -- fatal error */
		dev_err(cdns_xspi->dev, "Fatal err - command not completed\n");
		ret = -EPROTO;
	}

	return ret;
}

static void cdns_xspi_set_interrupts(struct cdns_xspi_plat *cdns_xspi,
				     bool enabled)
{
	u32 intr_enable;

	intr_enable = readl(cdns_xspi->iobase + CDNS_XSPI_INTR_ENABLE_REG);
	if (enabled)
		intr_enable |= CDNS_XSPI_INTR_MASK;
	else
		intr_enable &= ~CDNS_XSPI_INTR_MASK;
	writel(intr_enable, cdns_xspi->iobase + CDNS_XSPI_INTR_ENABLE_REG);
}

static int cdns_xspi_controller_init(struct cdns_xspi_plat *cdns_xspi)
{
	u32 ctrl_ver;
	u32 ctrl_features;
	u16 hw_magic_num;

	ctrl_ver = readl(cdns_xspi->iobase + CDNS_XSPI_CTRL_VERSION_REG);
	hw_magic_num = FIELD_GET(CDNS_XSPI_MAGIC_NUM, ctrl_ver);
	if (hw_magic_num != CDNS_XSPI_MAGIC_NUM_VALUE) {
		dev_err(cdns_xspi->dev,
			"Incorrect XSPI magic number: %x, expected: %x\n",
			hw_magic_num, CDNS_XSPI_MAGIC_NUM_VALUE);
		return -ENXIO;
	}

	ctrl_features = readl(cdns_xspi->iobase + CDNS_XSPI_CTRL_FEATURES_REG);
	cdns_xspi->hw_num_banks = FIELD_GET(CDNS_XSPI_NUM_BANKS, ctrl_features);
	cdns_xspi->set_interrupts_handler(cdns_xspi, false);

	return 0;
}

static void cdns_xspi_sdma_handle(struct cdns_xspi_plat *cdns_xspi)
{
	u32 sdma_size, sdma_trd_info;
	u8 sdma_dir;
	u8 *in_buf;
	u8 *out_buf;

	sdma_size = readl(cdns_xspi->iobase + CDNS_XSPI_SDMA_SIZE_REG);
	sdma_trd_info = readl(cdns_xspi->iobase + CDNS_XSPI_SDMA_TRD_INFO_REG);
	sdma_dir = FIELD_GET(CDNS_XSPI_SDMA_DIR, sdma_trd_info);

	in_buf = (u8 *)cdns_xspi->in_buffer;
	out_buf = (u8 *)cdns_xspi->out_buffer;

	switch (sdma_dir) {
	case CDNS_XSPI_SDMA_DIR_READ:
		if (in_buf)
			memcpy_fromio(in_buf, cdns_xspi->sdmabase, sdma_size);
	break;

	case CDNS_XSPI_SDMA_DIR_WRITE:
		if (in_buf)
			memcpy_toio(cdns_xspi->sdmabase, out_buf, sdma_size);
	break;

	default:
		/* Handle unexpected direction */
		dev_warn(cdns_xspi->dev,
			 "Unknown SDMA direction: %u\n", sdma_dir);
	break;
	}
}

static int cdns_xspi_send_stig_command(struct cdns_xspi_plat *cdns_xspi,
				       const struct spi_mem_op *op,
				       bool data_phase)
{
	u32 cmd_regs[6] = {0};
	int ret = 0;
	int dummybytes = op->dummy.nbytes;

	ret = cdns_xspi_wait_for_controller_idle(cdns_xspi);
	if (ret < 0)
		return ret;

	writel(FIELD_PREP(CDNS_XSPI_CTRL_WORK_MODE, CDNS_XSPI_WORK_MODE_STIG),
	       cdns_xspi->iobase + CDNS_XSPI_CTRL_CONFIG_REG);

	cdns_xspi->set_interrupts_handler(cdns_xspi, true);
	cdns_xspi->sdma_error = false;

	cmd_regs[1] = CDNS_XSPI_CMD_FLD_P1_INSTR_CMD_1(op, data_phase);
	cmd_regs[2] = CDNS_XSPI_CMD_FLD_P1_INSTR_CMD_2(op);
	if (dummybytes != 0) {
		cmd_regs[3] = CDNS_XSPI_CMD_FLD_P1_INSTR_CMD_3(op, 1);
		dummybytes--;
	} else {
		cmd_regs[3] = CDNS_XSPI_CMD_FLD_P1_INSTR_CMD_3(op, 0);
	}
	cmd_regs[4] = CDNS_XSPI_CMD_FLD_P1_INSTR_CMD_4(op,
						       cdns_xspi->cur_cs);

	cdns_xspi_trigger_command(cdns_xspi, cmd_regs);

	if (data_phase) {
		cmd_regs[0] = CDNS_XSPI_STIG_DONE_FLAG;
		cmd_regs[1] = CDNS_XSPI_CMD_FLD_DSEQ_CMD_1;
		cmd_regs[2] = CDNS_XSPI_CMD_FLD_DSEQ_CMD_2(op);
		cmd_regs[3] = CDNS_XSPI_CMD_FLD_DSEQ_CMD_3(op, dummybytes);
		cmd_regs[4] = CDNS_XSPI_CMD_FLD_DSEQ_CMD_4(op,
							   cdns_xspi->cur_cs);

		cdns_xspi->in_buffer = op->data.buf.in;
		cdns_xspi->out_buffer = op->data.buf.out;

		cdns_xspi_trigger_command(cdns_xspi, cmd_regs);

		cdns_xspi_wait_for_sdma_complete(cdns_xspi);

		if (cdns_xspi->sdma_error) {
			cdns_xspi->set_interrupts_handler(cdns_xspi, false);
			return -EIO;
		}
		cdns_xspi_sdma_handle(cdns_xspi);
	}

	cdns_xspi_wait_for_cmd_complete(cdns_xspi);
	ret = cdns_xspi_check_command_status(cdns_xspi);
	if (ret)
		return ret;

	return 0;
}

static int cdns_xspi_mem_op(struct udevice *bus,
			    const struct spi_mem_op *op,
			    unsigned int cs)
{
	struct cdns_xspi_plat *plat = dev_get_plat(bus);
	enum spi_mem_data_dir dir = op->data.dir;

	if (plat->cur_cs != cs)
		plat->cur_cs = cs;

	return cdns_xspi_send_stig_command(plat, op,
					   (dir != SPI_MEM_NO_DATA));
}

static int cdns_xspi_mem_op_execute(struct spi_slave *spi,
				    const struct spi_mem_op *op)
{
	struct udevice *bus = spi->dev->parent;
	unsigned int cs = 0;
	int ret = 0;

	cs = spi_chip_select(spi->dev);

	if (cs < 0) {
		/*
		 * spi_chip_select will return error number when not
		 * able to get chip select.
		 */
		pr_err("%s: Unable to get chip select, ret=%d",
		       spi->dev->name, cs);
		return cs;
	}

	ret = cdns_xspi_mem_op(bus, op, cs);

	return ret;
}

static int cdns_xspi_adjust_mem_op_size(struct spi_slave *spi,
					struct spi_mem_op *op)
{
	struct udevice *bus = spi->dev->parent;
	struct cdns_xspi_plat *plat = dev_get_plat(bus);

	op->data.nbytes = clamp_val(op->data.nbytes, 0, plat->sdmasize);

	return 0;
}

static const struct spi_controller_mem_ops cadence_xspi_mem_ops = {
	.exec_op = cdns_xspi_mem_op_execute,
	.adjust_op_size = cdns_xspi_adjust_mem_op_size,
};

static void cdns_xspi_print_phy_config(struct cdns_xspi_plat *cdns_xspi)
{
	struct device *dev = cdns_xspi->dev;

	dev_info(dev, "PHY configuration\n");
	dev_info(dev, "   * xspi_dll_phy_ctrl: %08x\n",
		 readl(cdns_xspi->iobase + CDNS_XSPI_DLL_PHY_CTRL));
	dev_info(dev, "   * phy_dq_timing: %08x\n",
		 readl(cdns_xspi->auxbase + CDNS_XSPI_CCP_PHY_DQ_TIMING));
	dev_info(dev, "   * phy_dqs_timing: %08x\n",
		 readl(cdns_xspi->auxbase + CDNS_XSPI_CCP_PHY_DQS_TIMING));
	dev_info(dev, "   * phy_gate_loopback_ctrl: %08x\n",
		 readl(cdns_xspi->auxbase + CDNS_XSPI_CCP_PHY_GATE_LPBCK_CTRL));
	dev_info(dev, "   * phy_dll_slave_ctrl: %08x\n",
		 readl(cdns_xspi->auxbase + CDNS_XSPI_CCP_PHY_DLL_SLAVE_CTRL));
}

static int cdns_xspi_probe(struct udevice *bus)
{
	struct cdns_xspi_plat *cdns_xspi = dev_get_plat(bus);
	struct resource res;
	int ret = 0;

	cdns_xspi->sdma_handler = &cdns_xspi_sdma_handle;
	cdns_xspi->set_interrupts_handler = &cdns_xspi_set_interrupts;
	cdns_xspi->cur_cs = 0;

	ret = dev_read_resource_byname(bus, "io", &res);
	if (ret)
		return ret;

	cdns_xspi->iobase = devm_ioremap(bus, res.start, resource_size(&res));

	if (IS_ERR(cdns_xspi->iobase)) {
		dev_err(bus, "Failed to remap controller base address\n");
		return PTR_ERR(cdns_xspi->iobase);
	}

	ret = dev_read_resource_byname(bus, "sdma", &res);
	if (ret)
		return ret;

	cdns_xspi->sdmabase = devm_ioremap(bus, res.start, resource_size(&res));

	if (IS_ERR(cdns_xspi->sdmabase)) {
		dev_err(bus, "Failed to remap SDMA address\n");
		return PTR_ERR(cdns_xspi->sdmabase);
	}
	cdns_xspi->sdmasize = resource_size(&res);

	ret = dev_read_resource_byname(bus, "aux", &res);
	if (ret)
		return ret;

	cdns_xspi->auxbase = devm_ioremap(bus, res.start, resource_size(&res));

	if (IS_ERR(cdns_xspi->auxbase)) {
		dev_err(bus, "Failed to remap AUX address\n");
		return PTR_ERR(cdns_xspi->auxbase);
	}

	cdns_xspi_print_phy_config(cdns_xspi);

	ret = cdns_xspi_controller_init(cdns_xspi);

	if (ret) {
		dev_err(bus, "Failed to initialize controller\n");
		return ret;
	}

	return 0;
}

static int cdns_xspi_remove(struct udevice *dev)
{
	struct cdns_xspi_plat *plat = dev_get_plat(dev);
	int ret = 0;

	if (plat->resets)
		ret = reset_release_bulk(plat->resets);

	return ret;
}

static int cadence_spi_set_speed(struct udevice *bus, uint hz)
{
	return 0;
}

static int cadence_spi_set_mode(struct udevice *bus, uint mode)
{
	return 0;
}

static const struct dm_spi_ops cdns_xspi_ops = {
	.set_speed	= cadence_spi_set_speed,
	.set_mode	= cadence_spi_set_mode,
	.mem_ops	= &cadence_xspi_mem_ops,
};

static const struct udevice_id cdns_xspi_of_match[] = {
	{
		.compatible = "cdns,xspi-nor",
	},
	{/* end of table */}
};

U_BOOT_DRIVER(cadence_xspi) = {
	.name = CDNS_XSPI_NAME,
	.id = UCLASS_SPI,
	.of_match = cdns_xspi_of_match,
	.ops = &cdns_xspi_ops,
	.probe = cdns_xspi_probe,
	.remove = cdns_xspi_remove,
	.flags = DM_FLAG_OS_PREPARE,
};
