/*
 * Copyright (c) 2011-12 The Chromium OS Authors.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * This file is derived from the flashrom project.
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <spi.h>
#include <pci.h>
#include <pci_ids.h>
#include <asm/io.h>

#include "ich.h"

#define SPI_OPCODE_WREN      0x06
#define SPI_OPCODE_FAST_READ 0x0b

struct ich_spi_platdata {
	pci_dev_t dev;		/* PCI device number */
	int ich_version;	/* Controller version, 7 or 9 */
	bool use_sbase;		/* Use SBASE instead of RCB */
};

struct ich_spi_priv {
	int ichspi_lock;
	int locked;
	int opmenu;
	int menubytes;
	void *base;		/* Base of register set */
	int preop;
	int optype;
	int addr;
	int data;
	unsigned databytes;
	int status;
	int control;
	int bbar;
	int bcr;
	uint32_t *pr;		/* only for ich9 */
	int speed;		/* pointer to speed control */
	ulong max_speed;	/* Maximum bus speed in MHz */
	ulong cur_speed;	/* Current bus speed */
	struct spi_trans trans;	/* current transaction in progress */
};

static u8 ich_readb(struct ich_spi_priv *priv, int reg)
{
	u8 value = readb(priv->base + reg);

	debug("read %2.2x from %4.4x\n", value, reg);

	return value;
}

static u16 ich_readw(struct ich_spi_priv *priv, int reg)
{
	u16 value = readw(priv->base + reg);

	debug("read %4.4x from %4.4x\n", value, reg);

	return value;
}

static u32 ich_readl(struct ich_spi_priv *priv, int reg)
{
	u32 value = readl(priv->base + reg);

	debug("read %8.8x from %4.4x\n", value, reg);

	return value;
}

static void ich_writeb(struct ich_spi_priv *priv, u8 value, int reg)
{
	writeb(value, priv->base + reg);
	debug("wrote %2.2x to %4.4x\n", value, reg);
}

static void ich_writew(struct ich_spi_priv *priv, u16 value, int reg)
{
	writew(value, priv->base + reg);
	debug("wrote %4.4x to %4.4x\n", value, reg);
}

static void ich_writel(struct ich_spi_priv *priv, u32 value, int reg)
{
	writel(value, priv->base + reg);
	debug("wrote %8.8x to %4.4x\n", value, reg);
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

/*
 * Check if this device ID matches one of supported Intel PCH devices.
 *
 * Return the ICH version if there is a match, or zero otherwise.
 */
static int get_ich_version(uint16_t device_id)
{
	if (device_id == PCI_DEVICE_ID_INTEL_TGP_LPC ||
	    device_id == PCI_DEVICE_ID_INTEL_ITC_LPC ||
	    device_id == PCI_DEVICE_ID_INTEL_QRK_ILB)
		return 7;

	if ((device_id >= PCI_DEVICE_ID_INTEL_COUGARPOINT_LPC_MIN &&
	     device_id <= PCI_DEVICE_ID_INTEL_COUGARPOINT_LPC_MAX) ||
	    (device_id >= PCI_DEVICE_ID_INTEL_PANTHERPOINT_LPC_MIN &&
	     device_id <= PCI_DEVICE_ID_INTEL_PANTHERPOINT_LPC_MAX) ||
	    device_id == PCI_DEVICE_ID_INTEL_VALLEYVIEW_LPC ||
	    device_id == PCI_DEVICE_ID_INTEL_LYNXPOINT_LPC ||
	    device_id == PCI_DEVICE_ID_INTEL_WILDCATPOINT_LPC)
		return 9;

	return 0;
}

/* @return 1 if the SPI flash supports the 33MHz speed */
static int ich9_can_do_33mhz(pci_dev_t dev)
{
	u32 fdod, speed;

	/* Observe SPI Descriptor Component Section 0 */
	pci_write_config_dword(dev, 0xb0, 0x1000);

	/* Extract the Write/Erase SPI Frequency from descriptor */
	pci_read_config_dword(dev, 0xb4, &fdod);

	/* Bits 23:21 have the fast read clock frequency, 0=20MHz, 1=33MHz */
	speed = (fdod >> 21) & 7;

	return speed == 1;
}

static int ich_find_spi_controller(struct ich_spi_platdata *ich)
{
	int last_bus = pci_last_busno();
	int bus;

	if (last_bus == -1) {
		debug("No PCI busses?\n");
		return -ENODEV;
	}

	for (bus = 0; bus <= last_bus; bus++) {
		uint16_t vendor_id, device_id;
		uint32_t ids;
		pci_dev_t dev;

		dev = PCI_BDF(bus, 31, 0);
		pci_read_config_dword(dev, 0, &ids);
		vendor_id = ids;
		device_id = ids >> 16;

		if (vendor_id == PCI_VENDOR_ID_INTEL) {
			ich->dev = dev;
			ich->ich_version = get_ich_version(device_id);
			if (device_id == PCI_DEVICE_ID_INTEL_VALLEYVIEW_LPC)
				ich->use_sbase = true;
			return ich->ich_version == 0 ? -ENODEV : 0;
		}
	}

	debug("ICH SPI: No ICH found.\n");
	return -ENODEV;
}

static int ich_init_controller(struct ich_spi_platdata *plat,
			       struct ich_spi_priv *ctlr)
{
	uint8_t *rcrb; /* Root Complex Register Block */
	uint32_t rcba; /* Root Complex Base Address */
	uint32_t sbase_addr;
	uint8_t *sbase;

	pci_read_config_dword(plat->dev, 0xf0, &rcba);
	/* Bits 31-14 are the base address, 13-1 are reserved, 0 is enable. */
	rcrb = (uint8_t *)(rcba & 0xffffc000);

	/* SBASE is similar */
	pci_read_config_dword(plat->dev, 0x54, &sbase_addr);
	sbase = (uint8_t *)(sbase_addr & 0xfffffe00);

	if (plat->ich_version == 7) {
		struct ich7_spi_regs *ich7_spi;

		ich7_spi = (struct ich7_spi_regs *)(rcrb + 0x3020);
		ctlr->ichspi_lock = readw(&ich7_spi->spis) & SPIS_LOCK;
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
	} else if (plat->ich_version == 9) {
		struct ich9_spi_regs *ich9_spi;

		if (plat->use_sbase)
			ich9_spi = (struct ich9_spi_regs *)sbase;
		else
			ich9_spi = (struct ich9_spi_regs *)(rcrb + 0x3800);
		ctlr->ichspi_lock = readw(&ich9_spi->hsfs) & HSFS_FLOCKDN;
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
	if (plat->ich_version == 9 && ich9_can_do_33mhz(plat->dev))
		ctlr->max_speed = 33000000;
	debug("ICH SPI: Version %d detected at %p, speed %ld\n",
	      plat->ich_version, ctlr->base, ctlr->max_speed);

	ich_set_bbar(ctlr, 0);

	return 0;
}

static inline void spi_use_out(struct spi_trans *trans, unsigned bytes)
{
	trans->out += bytes;
	trans->bytesout -= bytes;
}

static inline void spi_use_in(struct spi_trans *trans, unsigned bytes)
{
	trans->in += bytes;
	trans->bytesin -= bytes;
}

static void spi_setup_type(struct spi_trans *trans, int data_bytes)
{
	trans->type = 0xFF;

	/* Try to guess spi type from read/write sizes. */
	if (trans->bytesin == 0) {
		if (trans->bytesout + data_bytes > 4)
			/*
			 * If bytesin = 0 and bytesout > 4, we presume this is
			 * a write data operation, which is accompanied by an
			 * address.
			 */
			trans->type = SPI_OPCODE_TYPE_WRITE_WITH_ADDRESS;
		else
			trans->type = SPI_OPCODE_TYPE_WRITE_NO_ADDRESS;
		return;
	}

	if (trans->bytesout == 1) {	/* and bytesin is > 0 */
		trans->type = SPI_OPCODE_TYPE_READ_NO_ADDRESS;
		return;
	}

	if (trans->bytesout == 4)	/* and bytesin is > 0 */
		trans->type = SPI_OPCODE_TYPE_READ_WITH_ADDRESS;

	/* Fast read command is called with 5 bytes instead of 4 */
	if (trans->out[0] == SPI_OPCODE_FAST_READ && trans->bytesout == 5) {
		trans->type = SPI_OPCODE_TYPE_READ_WITH_ADDRESS;
		--trans->bytesout;
	}
}

static int spi_setup_opcode(struct ich_spi_priv *ctlr, struct spi_trans *trans)
{
	uint16_t optypes;
	uint8_t opmenu[ctlr->menubytes];

	trans->opcode = trans->out[0];
	spi_use_out(trans, 1);
	if (!ctlr->ichspi_lock) {
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
		if (trans->type == SPI_OPCODE_TYPE_WRITE_NO_ADDRESS &&
		    optype == SPI_OPCODE_TYPE_WRITE_WITH_ADDRESS &&
		    trans->bytesout >= 3) {
			/* We guessed wrong earlier. Fix it up. */
			trans->type = optype;
		}
		if (optype != trans->type) {
			printf("ICH SPI: Transaction doesn't fit type %d\n",
			       optype);
			return -ENOSPC;
		}
		return opcode_index;
	}
}

static int spi_setup_offset(struct spi_trans *trans)
{
	/* Separate the SPI address and data. */
	switch (trans->type) {
	case SPI_OPCODE_TYPE_READ_NO_ADDRESS:
	case SPI_OPCODE_TYPE_WRITE_NO_ADDRESS:
		return 0;
	case SPI_OPCODE_TYPE_READ_WITH_ADDRESS:
	case SPI_OPCODE_TYPE_WRITE_WITH_ADDRESS:
		trans->offset = ((uint32_t)trans->out[0] << 16) |
				((uint32_t)trans->out[1] << 8) |
				((uint32_t)trans->out[2] << 0);
		spi_use_out(trans, 3);
		return 1;
	default:
		printf("Unrecognized SPI transaction type %#x\n", trans->type);
		return -EPROTO;
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

static int ich_spi_xfer(struct udevice *dev, unsigned int bitlen,
			const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev_get_parent(dev);
	struct ich_spi_platdata *plat = dev_get_platdata(bus);
	struct ich_spi_priv *ctlr = dev_get_priv(bus);
	uint16_t control;
	int16_t opcode_index;
	int with_address;
	int status;
	int bytes = bitlen / 8;
	struct spi_trans *trans = &ctlr->trans;
	unsigned type = flags & (SPI_XFER_BEGIN | SPI_XFER_END);
	int using_cmd = 0;
	int ret;

	/* We don't support writing partial bytes */
	if (bitlen % 8) {
		debug("ICH SPI: Accessing partial bytes not supported\n");
		return -EPROTONOSUPPORT;
	}

	/* An empty end transaction can be ignored */
	if (type == SPI_XFER_END && !dout && !din)
		return 0;

	if (type & SPI_XFER_BEGIN)
		memset(trans, '\0', sizeof(*trans));

	/* Dp we need to come back later to finish it? */
	if (dout && type == SPI_XFER_BEGIN) {
		if (bytes > ICH_MAX_CMD_LEN) {
			debug("ICH SPI: Command length limit exceeded\n");
			return -ENOSPC;
		}
		memcpy(trans->cmd, dout, bytes);
		trans->cmd_len = bytes;
		debug("ICH SPI: Saved %d bytes\n", bytes);
		return 0;
	}

	/*
	 * We process a 'middle' spi_xfer() call, which has no
	 * SPI_XFER_BEGIN/END, as an independent transaction as if it had
	 * an end. We therefore repeat the command. This is because ICH
	 * seems to have no support for this, or because interest (in digging
	 * out the details and creating a special case in the code) is low.
	 */
	if (trans->cmd_len) {
		trans->out = trans->cmd;
		trans->bytesout = trans->cmd_len;
		using_cmd = 1;
		debug("ICH SPI: Using %d bytes\n", trans->cmd_len);
	} else {
		trans->out = dout;
		trans->bytesout = dout ? bytes : 0;
	}

	trans->in = din;
	trans->bytesin = din ? bytes : 0;

	/* There has to always at least be an opcode. */
	if (!trans->bytesout) {
		debug("ICH SPI: No opcode for transfer\n");
		return -EPROTO;
	}

	ret = ich_status_poll(ctlr, SPIS_SCIP, 0);
	if (ret < 0)
		return ret;

	if (plat->ich_version == 7)
		ich_writew(ctlr, SPIS_CDS | SPIS_FCERR, ctlr->status);
	else
		ich_writeb(ctlr, SPIS_CDS | SPIS_FCERR, ctlr->status);

	spi_setup_type(trans, using_cmd ? bytes : 0);
	opcode_index = spi_setup_opcode(ctlr, trans);
	if (opcode_index < 0)
		return -EINVAL;
	with_address = spi_setup_offset(trans);
	if (with_address < 0)
		return -EINVAL;

	if (trans->opcode == SPI_OPCODE_WREN) {
		/*
		 * Treat Write Enable as Atomic Pre-Op if possible
		 * in order to prevent the Management Engine from
		 * issuing a transaction between WREN and DATA.
		 */
		if (!ctlr->ichspi_lock)
			ich_writew(ctlr, trans->opcode, ctlr->preop);
		return 0;
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

	/* See if we have used up the command data */
	if (using_cmd && dout && bytes) {
		trans->out = dout;
		trans->bytesout = bytes;
		debug("ICH SPI: Moving to data, %d bytes\n", bytes);
	}

	/* Preset control fields */
	control = ich_readw(ctlr, ctlr->control);
	control &= ~SSFC_RESERVED;
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

	/*
	 * Check if this is a write command atempting to transfer more bytes
	 * than the controller can handle. Iterations for writes are not
	 * supported here because each SPI write command needs to be preceded
	 * and followed by other SPI commands, and this sequence is controlled
	 * by the SPI chip driver.
	 */
	if (trans->bytesout > ctlr->databytes) {
		debug("ICH SPI: Too much to write. This should be prevented by the driver's max_write_size?\n");
		return -EPROTO;
	}

	/*
	 * Read or write up to databytes bytes at a time until everything has
	 * been sent.
	 */
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
			spi_use_out(trans, data_length);
			if (with_address)
				trans->offset += data_length;
		}

		/* Add proper control fields' values */
		control &= ~((ctlr->databytes - 1) << 8);
		control |= SPIC_DS;
		control |= (data_length - 1) << 8;

		/* write it */
		ich_writew(ctlr, control, ctlr->control);

		/* Wait for Cycle Done Status or Flash Cycle Error. */
		status = ich_status_poll(ctlr, SPIS_CDS | SPIS_FCERR, 1);
		if (status < 0)
			return status;

		if (status & SPIS_FCERR) {
			debug("ICH SPI: Data transaction error %x\n", status);
			return -EIO;
		}

		if (trans->bytesin) {
			read_reg(ctlr, ctlr->data, trans->in, data_length);
			spi_use_in(trans, data_length);
			if (with_address)
				trans->offset += data_length;
		}
	}

	/* Clear atomic preop now that xfer is done */
	ich_writew(ctlr, 0, ctlr->preop);

	return 0;
}

/*
 * This uses the SPI controller from the Intel Cougar Point and Panther Point
 * PCH to write-protect portions of the SPI flash until reboot. The changes
 * don't actually take effect until the HSFS[FLOCKDN] bit is set, but that's
 * done elsewhere.
 */
int spi_write_protect_region(struct udevice *dev, uint32_t lower_limit,
			     uint32_t length, int hint)
{
	struct udevice *bus = dev->parent;
	struct ich_spi_priv *ctlr = dev_get_priv(bus);
	uint32_t tmplong;
	uint32_t upper_limit;

	if (!ctlr->pr) {
		printf("%s: operation not supported on this chipset\n",
		       __func__);
		return -ENOSYS;
	}

	if (length == 0 ||
	    lower_limit > (0xFFFFFFFFUL - length) + 1 ||
	    hint < 0 || hint > 4) {
		printf("%s(0x%x, 0x%x, %d): invalid args\n", __func__,
		       lower_limit, length, hint);
		return -EPERM;
	}

	upper_limit = lower_limit + length - 1;

	/*
	 * Determine bits to write, as follows:
	 *  31     Write-protection enable (includes erase operation)
	 *  30:29  reserved
	 *  28:16  Upper Limit (FLA address bits 24:12, with 11:0 == 0xfff)
	 *  15     Read-protection enable
	 *  14:13  reserved
	 *  12:0   Lower Limit (FLA address bits 24:12, with 11:0 == 0x000)
	 */
	tmplong = 0x80000000 |
		((upper_limit & 0x01fff000) << 4) |
		((lower_limit & 0x01fff000) >> 12);

	printf("%s: writing 0x%08x to %p\n", __func__, tmplong,
	       &ctlr->pr[hint]);
	ctlr->pr[hint] = tmplong;

	return 0;
}

static int ich_spi_probe(struct udevice *bus)
{
	struct ich_spi_platdata *plat = dev_get_platdata(bus);
	struct ich_spi_priv *priv = dev_get_priv(bus);
	uint8_t bios_cntl;
	int ret;

	ret = ich_init_controller(plat, priv);
	if (ret)
		return ret;
	/*
	 * Disable the BIOS write protect so write commands are allowed.  On
	 * v9, deassert SMM BIOS Write Protect Disable.
	 */
	if (plat->use_sbase) {
		bios_cntl = ich_readb(priv, priv->bcr);
		bios_cntl &= ~BIT(5);	/* clear Enable InSMM_STS (EISS) */
		bios_cntl |= 1;		/* Write Protect Disable (WPD) */
		ich_writeb(priv, bios_cntl, priv->bcr);
	} else {
		pci_read_config_byte(plat->dev, 0xdc, &bios_cntl);
		if (plat->ich_version == 9)
			bios_cntl &= ~BIT(5);
		pci_write_config_byte(plat->dev, 0xdc, bios_cntl | 0x1);
	}

	priv->cur_speed = priv->max_speed;

	return 0;
}

static int ich_spi_ofdata_to_platdata(struct udevice *bus)
{
	struct ich_spi_platdata *plat = dev_get_platdata(bus);
	int ret;

	ret = ich_find_spi_controller(plat);
	if (ret)
		return ret;

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
	if (plat->ich_version == 7) {
		slave->op_mode_rx = SPI_OPM_RX_AS;
		slave->op_mode_tx = SPI_OPM_TX_BP;
	}

	return 0;
}

static const struct dm_spi_ops ich_spi_ops = {
	.xfer		= ich_spi_xfer,
	.set_speed	= ich_spi_set_speed,
	.set_mode	= ich_spi_set_mode,
	/*
	 * cs_info is not needed, since we require all chip selects to be
	 * in the device tree explicitly
	 */
};

static const struct udevice_id ich_spi_ids[] = {
	{ .compatible = "intel,ich-spi" },
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
};
