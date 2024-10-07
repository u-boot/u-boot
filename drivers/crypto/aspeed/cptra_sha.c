// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2024 ASPEED Technology Inc.
 */
#include <asm/io.h>
#include <config.h>
#include <dm.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/iopoll.h>
#include <malloc.h>
#include <u-boot/hash.h>
#include <watchdog.h>

/* SHA register offsets */
#define CPTRA_SHA_LOCK			0x00
#define CPTRA_SHA_USER			0x04
#define CPTRA_SHA_MODE			0x08
#define   CPTRA_SHA_MODE_ENDIAN		BIT(2)
#define   CPTRA_SHA_MODE_SEL		GENMASK(1, 0)
#define CPTRA_SHA_DLEN			0x10
#define CPTRA_SHA_DATAIN		0x14
#define CPTRA_SHA_EXEC			0x18
#define CPTRA_SHA_STS			0x1c
#define   CPTRA_SHA_STS_SOC_LOCK	BIT(1)
#define   CPTRA_SHA_STS_VLD		BIT(0)
#define CPTRA_SHA_DIGEST(n)		(0x20 + ((n) << 2))
#define CPTRA_SHA_CTRL			0x60
#define   CPTRA_SHA_CTRL_ZEROIZE	BIT(0)

enum cptra_sha_modes {
	CPTRA_SHA384_STREAM,
	CPTRA_SHA512_STREAM,
};

struct cptra_sha_ctx {
	enum HASH_ALGO algo;
	uint32_t dgst_len;
};

struct cptra_sha {
	void *regs;
};

static int cptra_sha_init(struct udevice *dev, enum HASH_ALGO algo, void **ctxp)
{
	struct cptra_sha_ctx *cs_ctx;
	struct cptra_sha *cs;
	uint32_t mode;
	uint32_t reg;
	int rc;

	cs_ctx = malloc(sizeof(struct cptra_sha_ctx));
	if (!cs_ctx)
		return -ENOMEM;

	memset(cs_ctx, 0, sizeof(struct cptra_sha_ctx));

	cs_ctx->algo = algo;

	switch (algo) {
	case HASH_ALGO_SHA384:
		mode = CPTRA_SHA384_STREAM;
		cs_ctx->dgst_len = 48;
		break;
	case HASH_ALGO_SHA512:
		mode = CPTRA_SHA512_STREAM;
		cs_ctx->dgst_len = 64;
		break;
	default:
		rc = -EINVAL;
		goto free_n_out;
	};

	cs = dev_get_priv(dev);

	/* get CPTRA SHA lock */
	if (readl_poll_timeout(cs->regs + CPTRA_SHA_LOCK, reg, reg == 0, 1000000))
		return -EBUSY;

	/* zero clear SHA */
	writel(CPTRA_SHA_CTRL_ZEROIZE, cs->regs + CPTRA_SHA_CTRL);

	/* zero clear length */
	writel(0x0, cs->regs + CPTRA_SHA_DLEN);

	/* set SHA mode */
	reg = readl(cs->regs + CPTRA_SHA_MODE);
	reg &= ~(CPTRA_SHA_MODE_SEL);
	reg |= FIELD_PREP(CPTRA_SHA_MODE_SEL, mode);
	writel(reg, cs->regs + CPTRA_SHA_MODE);

	*ctxp = cs_ctx;

	return 0;

free_n_out:
	free(cs_ctx);

	return rc;
}

static int cptra_sha_update(struct udevice *dev, void *ctx, const void *ibuf, uint32_t ilen)
{
	struct cptra_sha *cs;
	uint32_t din_be;
	uint32_t dlen_sum;
	uint8_t *p8;
	uint32_t i;

	cs = dev_get_priv(dev);

	/* update length */
	dlen_sum = readl(cs->regs + CPTRA_SHA_DLEN) + ilen;
	writel(dlen_sum, cs->regs + CPTRA_SHA_DLEN);

	din_be = 0;
	for (i = 0, p8 = (uint8_t *)ibuf; i < ilen; ++i) {
		if (i && (i % sizeof(din_be) == 0)) {
			writel(din_be, cs->regs + CPTRA_SHA_DATAIN);
			din_be = 0;
		}

		din_be <<= 8;
		din_be |= p8[i];
	}

	if (i % sizeof(din_be))
		din_be <<= (8 * (sizeof(din_be) - (i % sizeof(din_be))));

	writel(din_be, cs->regs + CPTRA_SHA_DATAIN);

	return 0;
}

static int cptra_sha_finish(struct udevice *dev, void *ctx, void *obuf)
{
	struct cptra_sha_ctx *cs_ctx;
	struct cptra_sha *cs;
	uint32_t i, *p32;
	uint32_t sts;

	cs = dev_get_priv(dev);
	cs_ctx = (struct cptra_sha_ctx *)ctx;

	/* trigger SHA calculation */
	writel(0x1, cs->regs + CPTRA_SHA_EXEC);

	/* wait for completion */
	while (1) {
		sts = readl(cs->regs + CPTRA_SHA_STS);
		if (sts & CPTRA_SHA_STS_VLD)
			break;
	}

	/* get the SHA digest in big-endian */
	p32 = (uint32_t *)obuf;
	for (i = 0; i < (cs_ctx->dgst_len / sizeof(*p32)); ++i, p32++)
		*p32 = be32_to_cpu(readl(cs->regs + CPTRA_SHA_DIGEST(i)));

	/* release CPTRA SHA lock */
	writel(0x1, cs->regs + CPTRA_SHA_LOCK);

	free(cs_ctx);

	return 0;
}

static int cptra_sha_digest_wd(struct udevice *dev, enum HASH_ALGO algo,
			       const void *ibuf, const uint32_t ilen,
			       void *obuf, uint32_t chunk_sz)
{
	const void *cur, *end;
	uint32_t chunk;
	void *ctx;
	int rc;

	rc = cptra_sha_init(dev, algo, &ctx);
	if (rc)
		return rc;

	if (IS_ENABLED(CONFIG_HW_WATCHDOG) || CONFIG_IS_ENABLED(WATCHDOG)) {
		cur = ibuf;
		end = ibuf + ilen;

		while (cur < end) {
			chunk = end - cur;
			if (chunk > chunk_sz)
				chunk = chunk_sz;

			rc = cptra_sha_update(dev, ctx, cur, chunk);
			if (rc)
				return rc;

			cur += chunk;
			schedule();
		}
	} else {
		rc = cptra_sha_update(dev, ctx, ibuf, ilen);
		if (rc)
			return rc;
	}

	rc = cptra_sha_finish(dev, ctx, obuf);
	if (rc)
		return rc;

	return 0;
}

static int cptra_sha_digest(struct udevice *dev, enum HASH_ALGO algo,
			    const void *ibuf, const uint32_t ilen, void *obuf)
{
	/* re-use the watchdog version with input length as the chunk_sz */
	return cptra_sha_digest_wd(dev, algo, ibuf, ilen, obuf, ilen);
}

static int cptra_sha_probe(struct udevice *dev)
{
	struct cptra_sha *cs = dev_get_priv(dev);

	cs->regs = (void *)devfdt_get_addr(dev);
	if (cs->regs == (void *)FDT_ADDR_T_NONE) {
		debug("cannot map Caliptra SHA ACC registers\n");
		return -ENODEV;
	}

	return 0;
}

static int cptra_sha_remove(struct udevice *dev)
{
	return 0;
}

static const struct hash_ops cptra_sha_ops = {
	.hash_init = cptra_sha_init,
	.hash_update = cptra_sha_update,
	.hash_finish = cptra_sha_finish,
	.hash_digest_wd = cptra_sha_digest_wd,
	.hash_digest = cptra_sha_digest,
};

static const struct udevice_id cptra_sha_ids[] = {
	{ .compatible = "aspeed,ast2700-cptra-sha" },
	{ }
};

U_BOOT_DRIVER(aspeed_cptra_sha) = {
	.name = "aspeed_cptra_sha",
	.id = UCLASS_HASH,
	.of_match = cptra_sha_ids,
	.ops = &cptra_sha_ops,
	.probe = cptra_sha_probe,
	.remove	= cptra_sha_remove,
	.priv_auto = sizeof(struct cptra_sha),
	.flags = DM_FLAG_PRE_RELOC,
};
