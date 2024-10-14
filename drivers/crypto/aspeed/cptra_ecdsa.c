// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2024 ASPEED Technology Inc.
 */
#include <asm/io.h>
#include <config.h>
#include <crypto/ecdsa-uclass.h>
#include <dm.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/iopoll.h>
#include <malloc.h>
#include <u-boot/ecdsa.h>

/* SCU register offsets */
#define SCU1_CPTRA				0x130
#define   SCU1_CPTRA_RDY_FOR_RT	BIT(18)

/* CPTRA MBOX register offsets */
#define CPTRA_MBOX_LOCK			0x00
#define CPTRA_MBOX_USER			0x04
#define CPTRA_MBOX_CMD			0x08
#define CPTRA_MBOX_DLEN			0x0c
#define CPTRA_MBOX_DATAIN		0x10
#define CPTRA_MBOX_DATAOUT		0x14
#define CPTRA_MBOX_EXEC			0x18
#define CPTRA_MBOX_STS			0x1c
#define   CPTRA_MBOX_STS_SOC_LOCK	BIT(9)
#define   CPTRA_MBOX_STS_FSM_PS		GENMASK(8, 6)
#define   CPTRA_MBOX_STS_PS		GENMASK(3, 0)
#define CPTRA_MBOX_UNLOCK		0x20

#define CPTRA_ECDSA_SIG_LEN	96	/* ECDSA384 */
#define CPTRA_ECDSA_SHA_LEN	48	/* SHA384 */

#define CPTRA_MBCMD_ECDSA384_SIGNATURE_VERIFY	0x53494756

enum cptra_mbox_sts {
	CPTRA_MBSTS_CMD_BUSY,
	CPTRA_MBSTS_DATA_READY,
	CPTRA_MBSTS_CMD_COMPLETE,
	CPTRA_MBSTS_CMD_FAILURE,
};

enum cptra_mbox_fsm {
	CPTRA_MBFSM_IDLE,
	CPTRA_MBFSM_RDY_FOR_CMD,
	CPTRA_MBFSM_RDY_FOR_DLEN,
	CPTRA_MBFSM_RDY_FOR_DATA,
	CPTRA_MBFSM_EXEC_UC,
	CPTRA_MBFSM_EXEC_SOC,
	CPTRA_MBFSM_ERROR,
};

struct cptra_ecdsa {
	void *regs;
};

static uint32_t mbox_csum(uint32_t csum, uint8_t *data, uint32_t dlen)
{
	uint32_t i;

	for (i = 0; i < dlen; ++i)
		csum -= data[i];

	return csum;
}

static int cptra_ecdsa_verify(struct udevice *dev, const struct ecdsa_public_key *pubkey,
			      const void *hash, size_t hash_len,
			      const void *signature, size_t sig_len)
{
	struct cptra_ecdsa *ce;
	uint8_t *x, *y, *r, *s;
	uint32_t cmd, csum;
	uint32_t reg, sts;
	uint32_t *p32;
	int i;

	if (hash_len != CPTRA_ECDSA_SHA_LEN || sig_len != CPTRA_ECDSA_SIG_LEN)
		return -EINVAL;

	if ((strcmp(pubkey->curve_name, "secp384r1") && strcmp(pubkey->curve_name, "prime384v1")) ||
	    pubkey->size_bits != ((CPTRA_ECDSA_SIG_LEN / 2) << 3))
		return -EINVAL;

	ce = dev_get_priv(dev);

	/* get CPTRA MBOX lock */
	if (readl_poll_timeout(ce->regs + CPTRA_MBOX_LOCK, reg, reg == 0, 1000000))
		return -EBUSY;

	/* check MBOX is ready for command */
	sts = readl(ce->regs + CPTRA_MBOX_STS);
	if (FIELD_GET(CPTRA_MBOX_STS_FSM_PS, sts) != CPTRA_MBFSM_RDY_FOR_CMD)
		return -EACCES;

	/* init mbox parameters */
	cmd = CPTRA_MBCMD_ECDSA384_SIGNATURE_VERIFY;
	csum = 0;
	x = (uint8_t *)pubkey->x;
	y = (uint8_t *)pubkey->y;
	r = (uint8_t *)signature;
	s = (uint8_t *)signature + (CPTRA_ECDSA_SIG_LEN / 2);

	/* calculate checksum */
	csum = mbox_csum(csum, (uint8_t *)&cmd, sizeof(cmd));
	csum = mbox_csum(csum, x, CPTRA_ECDSA_SIG_LEN / 2);
	csum = mbox_csum(csum, y, CPTRA_ECDSA_SIG_LEN / 2);
	csum = mbox_csum(csum, r, CPTRA_ECDSA_SIG_LEN / 2);
	csum = mbox_csum(csum, s, CPTRA_ECDSA_SIG_LEN / 2);

	/* write command, data length */
	writel(cmd, ce->regs + CPTRA_MBOX_CMD);
	writel(sizeof(csum) + (CPTRA_ECDSA_SIG_LEN << 1), ce->regs + CPTRA_MBOX_DLEN);

	/* write ECDSA384_SIGNATURE_VERIFY command parameters */
	writel(csum, ce->regs + CPTRA_MBOX_DATAIN);

	for (i = 0, p32 = (uint32_t *)x; i < ((CPTRA_ECDSA_SIG_LEN / 2) / sizeof(*p32)); ++i)
		writel(p32[i], ce->regs + CPTRA_MBOX_DATAIN);

	for (i = 0, p32 = (uint32_t *)y; i < ((CPTRA_ECDSA_SIG_LEN / 2) / sizeof(*p32)); ++i)
		writel(p32[i], ce->regs + CPTRA_MBOX_DATAIN);

	for (i = 0, p32 = (uint32_t *)r; i < ((CPTRA_ECDSA_SIG_LEN / 2) / sizeof(*p32)); ++i)
		writel(p32[i], ce->regs + CPTRA_MBOX_DATAIN);

	for (i = 0, p32 = (uint32_t *)s; i < ((CPTRA_ECDSA_SIG_LEN / 2) / sizeof(*p32)); ++i)
		writel(p32[i], ce->regs + CPTRA_MBOX_DATAIN);

	/* trigger mbox command */
	writel(0x1, ce->regs + CPTRA_MBOX_EXEC);

	/* poll for result */
	while (1) {
		sts = FIELD_GET(CPTRA_MBOX_STS_PS, readl(ce->regs + CPTRA_MBOX_STS));
		if (sts != CPTRA_MBSTS_CMD_BUSY)
			break;
	}

	/* unlock mbox */
	writel(0x0, ce->regs + CPTRA_MBOX_EXEC);

	return (sts == CPTRA_MBSTS_CMD_FAILURE) ? sts : 0;
}

static int cptra_ecdsa_probe(struct udevice *dev)
{
	struct cptra_ecdsa *ce = dev_get_priv(dev);

	ce->regs = (void *)devfdt_get_addr(dev);
	if (ce->regs == (void *)FDT_ADDR_T_NONE) {
		debug("cannot map Caliptra mailbox registers\n");
		return -EINVAL;
	}

	return 0;
}

static int cptra_ecdsa_remove(struct udevice *dev)
{
	return 0;
}

static const struct ecdsa_ops cptra_ecdsa_ops = {
	.verify = cptra_ecdsa_verify,
};

static const struct udevice_id cptra_ecdsa_ids[] = {
	{ .compatible = "aspeed,ast2700-cptra-ecdsa" },
	{ }
};

U_BOOT_DRIVER(aspeed_cptra_ecdsa) = {
	.name = "aspeed_cptra_ecdsa",
	.id = UCLASS_ECDSA,
	.of_match = cptra_ecdsa_ids,
	.ops = &cptra_ecdsa_ops,
	.probe = cptra_ecdsa_probe,
	.remove = cptra_ecdsa_remove,
	.priv_auto = sizeof(struct cptra_ecdsa),
	.flags = DM_FLAG_PRE_RELOC,
};
