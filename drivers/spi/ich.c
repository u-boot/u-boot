// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011-12 The Chromium OS Authors.
 *
 * This file is derived from the flashrom project.
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <pch.h>
#include <pci.h>
#include <pci_ids.h>
#include <spi.h>
#include <asm/io.h>
#include <spi-mem.h>
#include <div64.h>

#include "ich.h"

DECLARE_GLOBAL_DATA_PTR;

#ifdef DEBUG_TRACE
#define debug_trace(fmt, args...) debug(fmt, ##args)
#else
#define debug_trace(x, args...)
#endif

static u8 ich_readb(struct ich_spi_priv *priv, int reg)
{
	u8 value = readb(priv->base + reg);

	debug_trace("read %2.2x from %4.4x\n", value, reg);

	return value;
}

static u16 ich_readw(struct ich_spi_priv *priv, int reg)
{
	u16 value = readw(priv->base + reg);

	debug_trace("read %4.4x from %4.4x\n", value, reg);

	return value;
}

static u32 ich_readl(struct ich_spi_priv *priv, int reg)
{
	u32 value = readl(priv->base + reg);

	debug_trace("read %8.8x from %4.4x\n", value, reg);

	return value;
}

static void ich_writeb(struct ich_spi_priv *priv, u8 value, int reg)
{
	writeb(value, priv->base + reg);
	debug_trace("wrote %2.2x to %4.4x\n", value, reg);
}

static void ich_writew(struct ich_spi_priv *priv, u16 value, int reg)
{
	writew(value, priv->base + reg);
	debug_trace("wrote %4.4x to %4.4x\n", value, reg);
}

static void ich_writel(struct ich_spi_priv *priv, u32 value, int reg)
{
	writel(value, priv->base + reg);
	debug_trace("wrote %8.8x to %4.4x\n", value, reg);
}

static void write_reg(struct ich_spi_priv *priv, const void *value,
		      int dest_reg, uint32_t size)
{
	memcpy_toio(priv->base + dest_reg, value, size);
}

static void read_reg(struct ich_spi_priv *priv, int src_reg, void *value,
		     uint32_t size)
{
	memcpy_fromio(value, priv->base + src_reg, size);
}

static void ich_set_bbar(struct ich_spi_priv *ctlr, uint32_t minaddr)
{
	const uint32_t bbar_mask = 0x00ffff00;
	uint32_t ichspi_bbar;

	minaddr &= bbar_mask;
	ichspi_bbar = ich_readl(ctlr, ctlr->bbar) & ~bbar_mask;
	ichspi_bbar |= minaddr;
	ich_writel(ctlr, ichspi_bbar, ctlr->bbar);
}

/* @return 1 if the SPI flash supports the 33MHz speed */
static int ich9_can_do_33mhz(struct udevice *dev)
{
	u32 fdod, speed;

	/* Observe SPI Descriptor Component Section 0 */
	dm_pci_write_config32(dev->parent, 0xb0, 0x1000);

	/* Extract the Write/Erase SPI Frequency from descriptor */
	dm_pci_read_config32(dev->parent, 0xb4, &fdod);

	/* Bits 23:21 have the fast read clock frequency, 0=20MHz, 1=33MHz */
	speed = (fdod >> 21) & 7;

	return speed == 1;
}

static int ich_init_controller(struct udevice *dev,
			       struct ich_spi_platdata *plat,
			       struct ich_spi_priv *ctlr)
{
	ulong sbase_addr;
	void *sbase;

	/* SBASE is similar */
	pch_get_spi_base(dev->parent, &sbase_addr);
	sbase = (void *)sbase_addr;
	debug("%s: sbase=%p\n", __func__, sbase);

	if (plat->ich_version == ICHV_7) {
		struct ich7_spi_regs *ich7_spi = sbase;

		ctlr->opmenu = offsetof(struct ich7_spi_regs, opmenu);
		ctlr->menubytes = sizeof(ich7_spi->opmenu);
		ctlr->optype = offsetof(struct ich7_spi_regs, optype);
		ctlr->addr = offsetof(struct ich7_spi_regs, spia);
		ctlr->data = offsetof(struct ich7_spi_regs, spid);
		ctlr->databytes = sizeof(ich7_spi->spid);
		ctlr->status = offsetof(struct ich7_spi_regs, spis);
		ctlr->control = offsetof(struct ich7_spi_regs, spic);
		ctlr->bbar = offsetof(struct ich7_spi_regs, bbar);
		ctlr->preop = offsetof(struct ich7_spi_regs, preop);
		ctlr->base = ich7_spi;
	} else if (plat->ich_version == ICHV_9) {
		struct ich9_spi_regs *ich9_spi = sbase;

		ctlr->opmenu = offsetof(struct ich9_spi_regs, opmenu);
		ctlr->menubytes = sizeof(ich9_spi->opmenu);
		ctlr->optype = offsetof(struct ich9_spi_regs, optype);
		ctlr->addr = offsetof(struct ich9_spi_regs, faddr);
		ctlr->data = offsetof(struct ich9_spi_regs, fdata);
		ctlr->databytes = sizeof(ich9_spi->fdata);
		ctlr->status = offsetof(struct ich9_spi_regs, ssfs);
		ctlr->control = offsetof(struct ich9_spi_regs, ssfc);
		ctlr->speed = ctlr->control + 2;
		ctlr->bbar = offsetof(struct ich9_spi_regs, bbar);
		ctlr->preop = offsetof(struct ich9_spi_regs, preop);
		ctlr->bcr = offsetof(struct ich9_spi_regs, bcr);
		ctlr->pr = &ich9_spi->pr[0];
		ctlr->base = ich9_spi;
	} else {
		debug("ICH SPI: Unrecognised ICH version %d\n",
		      plat->ich_version);
		return -EINVAL;
	}

	/* Work out the maximum speed we can support */
	ctlr->max_speed = 20000000;
	if (plat->ich_version == ICHV_9 && ich9_can_do_33mhz(dev))
		ctlr->max_speed = 33000000;
	debug("ICH SPI: Version ID %d detected at %p, speed %ld\n",
	      plat->ich_version, ctlr->base, ctlr->max_speed);

	ich_set_bbar(ctlr, 0);

	return 0;
}

static void spi_lock_down(struct ich_spi_platdata *plat, void *sbase)
{
	if (plat->ich_version == ICHV_7) {
		struct ich7_spi_regs *ich7_spi = sbase;

		setbits_le16(&ich7_spi->spis, SPIS_LOCK);
	} else if (plat->ich_version == ICHV_9) {
		struct ich9_spi_regs *ich9_spi = sbase;

		setbits_le16(&ich9_spi->hsfs, HSFS_FLOCKDN);
	}
}

static bool spi_lock_status(struct ich_spi_platdata *plat, void *sbase)
{
	int lock = 0;

	if (plat->ich_version == ICHV_7) {
		struct ich7_spi_regs *ich7_spi = sbase;

		lock = readw(&ich7_spi->spis) & SPIS_LOCK;
	} else if (plat->ich_version == ICHV_9) {
		struct ich9_spi_regs *ich9_spi = sbase;

		lock = readw(&ich9_spi->hsfs) & HSFS_FLOCKDN;
	}

	return lock != 0;
}

static int spi_setup_opcode(struct ich_spi_priv *ctlr, struct spi_trans *trans,
			    bool lock)
{
	uint16_t optypes;
	uint8_t opmenu[ctlr->menubytes];

	if (!lock) {
		/* The lock is off, so just use index 0. */
		ich_writeb(ctlr, trans->opcode, ctlr->opmenu);
		optypes = ich_readw(ctlr, ctlr->optype);
		optypes = (optypes & 0xfffc) | (trans->type & 0x3);
		ich_writew(ctlr, optypes, ctlr->optype);
		return 0;
	} else {
		/* The lock is on. See if what we need is on the menu. */
		uint8_t optype;
		uint16_t opcode_index;

		/* Write Enable is handled as atomic prefix */
		if (trans->opcode == SPI_OPCODE_WREN)
			return 0;

		read_reg(ctlr, ctlr->opmenu, opmenu, sizeof(opmenu));
		for (opcode_index = 0; opcode_index < ctlr->menubytes;
				opcode_index++) {
			if (opmenu[opcode_index] == trans->opcode)
				break;
		}

		if (opcode_index == ctlr->menubytes) {
			printf("ICH SPI: Opcode %x not found\n",
			       trans->opcode);
			return -EINVAL;
		}

		optypes = ich_readw(ctlr, ctlr->optype);
		optype = (optypes >> (opcode_index * 2)) & 0x3;

		if (optype != trans->type) {
			printf("ICH SPI: Transaction doesn't fit type %d\n",
			       optype);
			return -ENOSPC;
		}
		return opcode_index;
	}
}

/*
 * Wait for up to 6s til status register bit(s) turn 1 (in case wait_til_set
 * below is true) or 0. In case the wait was for the bit(s) to set - write
 * those bits back, which would cause resetting them.
 *
 * Return the last read status value on success or -1 on failure.
 */
static int ich_status_poll(struct ich_spi_priv *ctlr, u16 bitmask,
			   int wait_til_set)
{
	int timeout = 600000; /* This will result in 6s */
	u16 status = 0;

	while (timeout--) {
		status = ich_readw(ctlr, ctlr->status);
		if (wait_til_set ^ ((status & bitmask) == 0)) {
			if (wait_til_set) {
				ich_writew(ctlr, status & bitmask,
					   ctlr->status);
			}
			return status;
		}
		udelay(10);
	}

	printf("ICH SPI: SCIP timeout, read %x, expected %x\n",
	       status, bitmask);
	return -ETIMEDOUT;
}

static void ich_spi_config_opcode(struct udevice *dev)
{
	struct ich_spi_priv *ctlr = dev_get_priv(dev);

	/*
	 * PREOP, OPTYPE, OPMENU1/OPMENU2 registers can be locked down
	 * to prevent accidental or intentional writes. Before they get
	 * locked down, these registers should be initialized properly.
	 */
	ich_writew(ctlr, SPI_OPPREFIX, ctlr->preop);
	ich_writew(ctlr, SPI_OPTYPE, ctlr->optype);
	ich_writel(ctlr, SPI_OPMENU_LOWER, ctlr->opmenu);
	ich_writel(ctlr, SPI_OPMENU_UPPER, ctlr->opmenu + sizeof(u32));
}

static int ich_spi_exec_op(struct spi_slave *slave, const struct spi_mem_op *op)
{
	struct udevice *bus = dev_get_parent(slave->dev);
	struct ich_spi_platdata *plat = dev_get_platdata(bus);
	struct ich_spi_priv *ctlr = dev_get_priv(bus);
	uint16_t control;
	int16_t opcode_index;
	int with_address;
	int status;
	struct spi_trans *trans = &ctlr->trans;
	bool lock = spi_lock_status(plat, ctlr->base);
	int ret = 0;

	trans->in = NULL;
	trans->out = NULL;
	trans->type = 0xFF;

	if (op->data.nbytes) {
		if (op->data.dir == SPI_MEM_DATA_IN) {
			trans->in = op->data.buf.in;
			trans->bytesin = op->data.nbytes;
		} else {
			trans->out = op->data.buf.out;
			trans->bytesout = op->data.nbytes;
		}
	}

	if (trans->opcode != op->cmd.opcode)
		trans->opcode = op->cmd.opcode;

	if (lock && trans->opcode == SPI_OPCODE_WRDIS)
		return 0;

	if (trans->opcode == SPI_OPCODE_WREN) {
		/*
		 * Treat Write Enable as Atomic Pre-Op if possible
		 * in order to prevent the Management Engine from
		 * issuing a transaction between WREN and DATA.
		 */
		if (!lock)
			ich_writew(ctlr, trans->opcode, ctlr->preop);
		return 0;
	}

	ret = ich_status_poll(ctlr, SPIS_SCIP, 0);
	if (ret < 0)
		return ret;

	if (plat->ich_version == ICHV_7)
		ich_writew(ctlr, SPIS_CDS | SPIS_FCERR, ctlr->status);
	else
		ich_writeb(ctlr, SPIS_CDS | SPIS_FCERR, ctlr->status);

	/* Try to guess spi transaction type */
	if (op->data.dir == SPI_MEM_DATA_OUT) {
		if (op->addr.nbytes)
			trans->type = SPI_OPCODE_TYPE_WRITE_WITH_ADDRESS;
		else
			trans->type = SPI_OPCODE_TYPE_WRITE_NO_ADDRESS;
	} else {
		if (op->addr.nbytes)
			trans->type = SPI_OPCODE_TYPE_READ_WITH_ADDRESS;
		else
			trans->type = SPI_OPCODE_TYPE_READ_NO_ADDRESS;
	}
	/* Special erase case handling */
	if (op->addr.nbytes && !op->data.buswidth)
		trans->type = SPI_OPCODE_TYPE_WRITE_WITH_ADDRESS;

	opcode_index = spi_setup_opcode(ctlr, trans, lock);
	if (opcode_index < 0)
		return -EINVAL;

	if (op->addr.nbytes) {
		trans->offset = op->addr.val;
		with_address = 1;
	}

	if (ctlr->speed && ctlr->max_speed >= 33000000) {
		int byte;

		byte = ich_readb(ctlr, ctlr->speed);
		if (ctlr->cur_speed >= 33000000)
			byte |= SSFC_SCF_33MHZ;
		else
			byte &= ~SSFC_SCF_33MHZ;
		ich_writeb(ctlr, byte, ctlr->speed);
	}

	/* Preset control fields */
	control = SPIC_SCGO | ((opcode_index & 0x07) << 4);

	/* Issue atomic preop cycle if needed */
	if (ich_readw(ctlr, ctlr->preop))
		control |= SPIC_ACS;

	if (!trans->bytesout && !trans->bytesin) {
		/* SPI addresses are 24 bit only */
		if (with_address) {
			ich_writel(ctlr, trans->offset & 0x00FFFFFF,
				   ctlr->addr);
		}
		/*
		 * This is a 'no data' command (like Write Enable), its
		 * bitesout size was 1, decremented to zero while executing
		 * spi_setup_opcode() above. Tell the chip to send the
		 * command.
		 */
		ich_writew(ctlr, control, ctlr->control);

		/* wait for the result */
		status = ich_status_poll(ctlr, SPIS_CDS | SPIS_FCERR, 1);
		if (status < 0)
			return status;

		if (status & SPIS_FCERR) {
			debug("ICH SPI: Command transaction error\n");
			return -EIO;
		}

		return 0;
	}

	while (trans->bytesout || trans->bytesin) {
		uint32_t data_length;

		/* SPI addresses are 24 bit only */
		ich_writel(ctlr, trans->offset & 0x00FFFFFF, ctlr->addr);

		if (trans->bytesout)
			data_length = min(trans->bytesout, ctlr->databytes);
		else
			data_length = min(trans->bytesin, ctlr->databytes);

		/* Program data into FDATA0 to N */
		if (trans->bytesout) {
			write_reg(ctlr, trans->out, ctlr->data, data_length);
			trans->bytesout -= data_length;
		}

		/* Add proper control fields' values */
		control &= ~((ctlr->databytes - 1) << 8);
		control |= SPIC_DS;
		control |= (data_length - 1) << 8;

		/* write it */
		ich_writew(ctlr, control, ctlr->control);

		/* Wait for Cycle Done Status or Flash Cycle Error */
		status = ich_status_poll(ctlr, SPIS_CDS | SPIS_FCERR, 1);
		if (status < 0)
			return status;

		if (status & SPIS_FCERR) {
			debug("ICH SPI: Data transaction error %x\n", status);
			return -EIO;
		}

		if (trans->bytesin) {
			read_reg(ctlr, ctlr->data, trans->in, data_length);
			trans->bytesin -= data_length;
		}
	}

	/* Clear atomic preop now that xfer is done */
	if (!lock)
		ich_writew(ctlr, 0, ctlr->preop);

	return 0;
}

static int ich_spi_adjust_size(struct spi_slave *slave, struct spi_mem_op *op)
{
	unsigned int page_offset;
	int addr = op->addr.val;
	unsigned int byte_count = op->data.nbytes;

	if (hweight32(ICH_BOUNDARY) == 1) {
		page_offset = addr & (ICH_BOUNDARY - 1);
	} else {
		u64 aux = addr;

		page_offset = do_div(aux, ICH_BOUNDARY);
	}

	if (op->data.dir == SPI_MEM_DATA_IN && slave->max_read_size) {
		op->data.nbytes = min(ICH_BOUNDARY - page_offset,
				      slave->max_read_size);
	} else if (slave->max_write_size) {
		op->data.nbytes = min(ICH_BOUNDARY - page_offset,
				      slave->max_write_size);
	}

	op->data.nbytes = min(op->data.nbytes, byte_count);

	return 0;
}

static int ich_spi_xfer(struct udevice *dev, unsigned int bitlen,
			const void *dout, void *din, unsigned long flags)
{
	printf("ICH SPI: Only supports memory operations\n");
	return -1;
}

static int ich_spi_probe(struct udevice *dev)
{
	struct ich_spi_platdata *plat = dev_get_platdata(dev);
	struct ich_spi_priv *priv = dev_get_priv(dev);
	uint8_t bios_cntl;
	int ret;

	ret = ich_init_controller(dev, plat, priv);
	if (ret)
		return ret;
	/* Disable the BIOS write protect so write commands are allowed */
	ret = pch_set_spi_protect(dev->parent, false);
	if (ret == -ENOSYS) {
		bios_cntl = ich_readb(priv, priv->bcr);
		bios_cntl &= ~BIT(5);	/* clear Enable InSMM_STS (EISS) */
		bios_cntl |= 1;		/* Write Protect Disable (WPD) */
		ich_writeb(priv, bios_cntl, priv->bcr);
	} else if (ret) {
		debug("%s: Failed to disable write-protect: err=%d\n",
		      __func__, ret);
		return ret;
	}

	/* Lock down SPI controller settings if required */
	if (plat->lockdown) {
		ich_spi_config_opcode(dev);
		spi_lock_down(plat, priv->base);
	}

	priv->cur_speed = priv->max_speed;

	return 0;
}

static int ich_spi_remove(struct udevice *bus)
{
	/*
	 * Configure SPI controller so that the Linux MTD driver can fully
	 * access the SPI NOR chip
	 */
	ich_spi_config_opcode(bus);

	return 0;
}

static int ich_spi_set_speed(struct udevice *bus, uint speed)
{
	struct ich_spi_priv *priv = dev_get_priv(bus);

	priv->cur_speed = speed;

	return 0;
}

static int ich_spi_set_mode(struct udevice *bus, uint mode)
{
	debug("%s: mode=%d\n", __func__, mode);

	return 0;
}

static int ich_spi_child_pre_probe(struct udevice *dev)
{
	struct udevice *bus = dev_get_parent(dev);
	struct ich_spi_platdata *plat = dev_get_platdata(bus);
	struct ich_spi_priv *priv = dev_get_priv(bus);
	struct spi_slave *slave = dev_get_parent_priv(dev);

	/*
	 * Yes this controller can only write a small number of bytes at
	 * once! The limit is typically 64 bytes.
	 */
	slave->max_write_size = priv->databytes;
	/*
	 * ICH 7 SPI controller only supports array read command
	 * and byte program command for SST flash
	 */
	if (plat->ich_version == ICHV_7)
		slave->mode = SPI_RX_SLOW | SPI_TX_BYTE;

	return 0;
}

static int ich_spi_ofdata_to_platdata(struct udevice *dev)
{
	struct ich_spi_platdata *plat = dev_get_platdata(dev);
	int node = dev_of_offset(dev);
	int ret;

	ret = fdt_node_check_compatible(gd->fdt_blob, node, "intel,ich7-spi");
	if (ret == 0) {
		plat->ich_version = ICHV_7;
	} else {
		ret = fdt_node_check_compatible(gd->fdt_blob, node,
						"intel,ich9-spi");
		if (ret == 0)
			plat->ich_version = ICHV_9;
	}

	plat->lockdown = fdtdec_get_bool(gd->fdt_blob, node,
					 "intel,spi-lock-down");

	return ret;
}

static const struct spi_controller_mem_ops ich_controller_mem_ops = {
	.adjust_op_size	= ich_spi_adjust_size,
	.supports_op	= NULL,
	.exec_op	= ich_spi_exec_op,
};

static const struct dm_spi_ops ich_spi_ops = {
	.xfer		= ich_spi_xfer,
	.set_speed	= ich_spi_set_speed,
	.set_mode	= ich_spi_set_mode,
	.mem_ops	= &ich_controller_mem_ops,
	/*
	 * cs_info is not needed, since we require all chip selects to be
	 * in the device tree explicitly
	 */
};

static const struct udevice_id ich_spi_ids[] = {
	{ .compatible = "intel,ich7-spi" },
	{ .compatible = "intel,ich9-spi" },
	{ }
};

U_BOOT_DRIVER(ich_spi) = {
	.name	= "ich_spi",
	.id	= UCLASS_SPI,
	.of_match = ich_spi_ids,
	.ops	= &ich_spi_ops,
	.ofdata_to_platdata = ich_spi_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct ich_spi_platdata),
	.priv_auto_alloc_size = sizeof(struct ich_spi_priv),
	.child_pre_probe = ich_spi_child_pre_probe,
	.probe	= ich_spi_probe,
	.remove	= ich_spi_remove,
	.flags	= DM_FLAG_OS_PREPARE,
};
