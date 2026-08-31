// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2026 Free Mobile - Vincent Jardin
 *
 * Sandbox PMBus 1.x chip stub (UCLASS_I2C_EMUL).
 *
 * Stub a DT i2c node so the PMBus framework, the generic pmbus
 * regulator and the pmbus CLI command can be tested.
 * The model is a flat per-command 16 bit
 * register file combined with some fixed identification strings.
 */

#include <dm.h>
#include <i2c.h>
#include <pmbus.h>
#include <linux/ctype.h>

#define PMBUS_EMUL_NREG		256

struct sandbox_pmbus_priv {
	u16 reg[PMBUS_EMUL_NREG];
	bool supported[PMBUS_EMUL_NREG];
};

/* Identification strings reported in the natural (forward) byte order. */
static const char *pmbus_emul_string(u8 cmd)
{
	switch (cmd) {
	case PMBUS_MFR_ID:
		return "SANDBOX";
	case PMBUS_MFR_MODEL:
		return "PMBUS-EMUL";
	case PMBUS_MFR_REVISION:
		return "1.0";
	default:
		return NULL;
	}
}

static int sandbox_pmbus_read(struct sandbox_pmbus_priv *priv, u8 cmd,
			      u8 *buf, int len)
{
	const char *str = pmbus_emul_string(cmd);
	int i;

	if (len < 1)
		return -EINVAL;

	if (str) {
		int slen = strlen(str);

		/*
		 * Block payload: [length][bytes...]. A one-byte read is
		 * the SMBus length probe and reports the true length (the
		 * master just NAKs early). Any longer read must have room
		 * for length + payload, otherwise the length byte would
		 * lie about the data actually returned.
		 */
		if (len > 1 && len < slen + 1)
			return -EREMOTEIO;
		buf[0] = (u8)slen;
		for (i = 1; i < len; i++)
			buf[i] = (i - 1 < slen) ? (u8)str[i - 1] : 0;
		return 0;
	}

	if (!priv->supported[cmd])
		return -EREMOTEIO; /* chip NAKs an unimplemented command */

	for (i = 0; i < len; i++)
		buf[i] = (u8)(priv->reg[cmd] >> (8 * i));
	return 0;
}

static int sandbox_pmbus_write(struct sandbox_pmbus_priv *priv, u8 cmd,
			       const u8 *buf, int len)
{
	if (len == 0)
		return 0; /* send-byte (eg CLEAR_FAULTS): just ACK */
	if (!priv->supported[cmd])
		return -EREMOTEIO;
	if (len == 1)
		priv->reg[cmd] = buf[0];
	else
		priv->reg[cmd] = (u16)buf[0] | ((u16)buf[1] << 8);
	return 0;
}

static int sandbox_pmbus_xfer(struct udevice *emul, struct i2c_msg *msg,
			      int nmsgs)
{
	struct sandbox_pmbus_priv *priv = dev_get_priv(emul);
	u8 cmd;

	if (nmsgs == 0)
		return 0;
	/* A PMBus transaction always opens with the command-code write. */
	if (msg[0].flags & I2C_M_RD)
		return -EIO;
	if (msg[0].len == 0)
		return 0; /* address-only probe */
	cmd = msg[0].buf[0];

	if (nmsgs >= 2 && (msg[1].flags & I2C_M_RD))
		return sandbox_pmbus_read(priv, cmd, msg[1].buf, msg[1].len);

	return sandbox_pmbus_write(priv, cmd, msg[0].buf + 1, msg[0].len - 1);
}

static void sandbox_pmbus_support(struct sandbox_pmbus_priv *priv, u8 cmd,
				  u16 val)
{
	priv->supported[cmd] = true;
	priv->reg[cmd] = val;
}

static int sandbox_pmbus_probe(struct udevice *emul)
{
	struct sandbox_pmbus_priv *priv = dev_get_priv(emul);

	/* Configuration / identification. */
	sandbox_pmbus_support(priv, PMBUS_PAGE, 0);
	sandbox_pmbus_support(priv, PMBUS_OPERATION, PB_OPERATION_ON);
	sandbox_pmbus_support(priv, PMBUS_ON_OFF_CONFIG, 0);
	sandbox_pmbus_support(priv, PMBUS_WRITE_PROTECT, 0);
	sandbox_pmbus_support(priv, PMBUS_CAPABILITY, 0x30);
	sandbox_pmbus_support(priv, PMBUS_VOUT_MODE, 0x18); /* LINEAR 2^-8 */
	sandbox_pmbus_support(priv, PMBUS_VOUT_COMMAND, 0x0200);
	sandbox_pmbus_support(priv, PMBUS_VOUT_TRIM, 0);
	sandbox_pmbus_support(priv, PMBUS_VOUT_MAX, 0x0400);
	sandbox_pmbus_support(priv, PMBUS_VOUT_SCALE_LOOP, 0);
	sandbox_pmbus_support(priv, PMBUS_REVISION, PMBUS_REV_13);

	/* Status registers, all clean. */
	sandbox_pmbus_support(priv, PMBUS_STATUS_BYTE, 0);
	sandbox_pmbus_support(priv, PMBUS_STATUS_WORD, 0);
	sandbox_pmbus_support(priv, PMBUS_STATUS_VOUT, 0);
	sandbox_pmbus_support(priv, PMBUS_STATUS_IOUT, 0);
	sandbox_pmbus_support(priv, PMBUS_STATUS_INPUT, 0);
	sandbox_pmbus_support(priv, PMBUS_STATUS_TEMPERATURE, 0);
	sandbox_pmbus_support(priv, PMBUS_STATUS_CML, 0);

	/*
	 * Telemetry the emulated chip implements. READ_IIN and READ_POUT
	 * are intentionally absent so callers see the unsupported path.
	 */
	sandbox_pmbus_support(priv, PMBUS_READ_VIN, 0x0abc);
	sandbox_pmbus_support(priv, PMBUS_READ_VOUT, 0x0200);
	sandbox_pmbus_support(priv, PMBUS_READ_IOUT, 0x0123);
	sandbox_pmbus_support(priv, PMBUS_READ_TEMPERATURE_1, 0x0019);

	return 0;
}

static struct dm_i2c_ops sandbox_pmbus_emul_ops = {
	.xfer = sandbox_pmbus_xfer,
};

static const struct udevice_id sandbox_pmbus_ids[] = {
	{ .compatible = "sandbox,i2c-pmbus" },
	{ }
};

U_BOOT_DRIVER(sandbox_pmbus_emul) = {
	.name		= "sandbox_pmbus_emul",
	.id		= UCLASS_I2C_EMUL,
	.of_match	= sandbox_pmbus_ids,
	.probe		= sandbox_pmbus_probe,
	.priv_auto	= sizeof(struct sandbox_pmbus_priv),
	.ops		= &sandbox_pmbus_emul_ops,
};
