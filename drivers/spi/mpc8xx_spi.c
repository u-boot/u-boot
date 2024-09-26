// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2001 Navin Boppuri / Prashant Patel
 *	<nboppuri@trinetcommunication.com>,
 *	<pmpatel@trinetcommunication.com>
 * Copyright (c) 2001 Gerd Mennchen <Gerd.Mennchen@icn.siemens.de>
 * Copyright (c) 2001 Wolfgang Denk, DENX Software Engineering, <wd@denx.de>.
 */

/*
 * MPC8xx CPM SPI interface.
 *
 * Parts of this code are probably not portable and/or specific to
 * the board which I used for the tests. Please send fixes/complaints
 * to wd@denx.de
 *
 */

#include <dm.h>
#include <malloc.h>
#include <mpc8xx.h>
#include <spi.h>
#include <linux/delay.h>

#include <asm/cpm_8xx.h>
#include <asm/io.h>
#include <asm/gpio.h>

#define CPM_SPI_BASE_RX	CPM_SPI_BASE
#define CPM_SPI_BASE_TX	(CPM_SPI_BASE + sizeof(cbd_t))

#define MAX_BUFFER	0x8000 /* Max possible is 0xffff. We want power of 2 */
#define MIN_HWORD_XFER	64	/* Minimum size for 16 bits transfer */

struct mpc8xx_priv {
	spi_t __iomem *spi;
	struct gpio_desc gpios[16];
	int max_cs;
};

static char dummy_buffer[MAX_BUFFER];

static int mpc8xx_spi_set_mode(struct udevice *dev, uint mod)
{
	return 0;
}

static int mpc8xx_spi_set_speed(struct udevice *dev, uint speed)
{
	immap_t __iomem *immr = (immap_t __iomem *)CONFIG_SYS_IMMR;
	cpm8xx_t __iomem *cp = &immr->im_cpm;
	u8 pm = (gd->arch.brg_clk - 1) / (speed * 16);

	if (pm > 16) {
		setbits_be16(&cp->cp_spmode, SPMODE_DIV16);
		pm /= 16;
		if (pm > 16)
			pm = 16;
	} else {
		clrbits_be16(&cp->cp_spmode, SPMODE_DIV16);
	}

	clrsetbits_be16(&cp->cp_spmode, SPMODE_PM(0xf), SPMODE_PM(pm));

	return 0;
}

static int mpc8xx_spi_probe(struct udevice *dev)
{
	immap_t __iomem *immr = (immap_t __iomem *)CONFIG_SYS_IMMR;
	cpm8xx_t __iomem *cp = &immr->im_cpm;
	spi_t __iomem *spi = (spi_t __iomem *)&cp->cp_dpmem[PROFF_SPI];
	u16 spi_rpbase;
	cbd_t __iomem *tbdf, *rbdf;

	spi_rpbase = in_be16(&spi->spi_rpbase);
	if (spi_rpbase)
		spi = (spi_t __iomem *)&cp->cp_dpmem[spi_rpbase];

/* 1 */
	/* Initialize the parameter ram.
	 * We need to make sure many things are initialized to zero
	 */
	out_be32(&spi->spi_rstate, 0);
	out_be32(&spi->spi_rdp, 0);
	out_be16(&spi->spi_rbptr, 0);
	out_be16(&spi->spi_rbc, 0);
	out_be32(&spi->spi_rxtmp, 0);
	out_be32(&spi->spi_tstate, 0);
	out_be32(&spi->spi_tdp, 0);
	out_be16(&spi->spi_tbptr, 0);
	out_be16(&spi->spi_tbc, 0);
	out_be32(&spi->spi_txtmp, 0);

/* 3 */
	/* Set up the SPI parameters in the parameter ram */
	out_be16(&spi->spi_rbase, CPM_SPI_BASE_RX);
	out_be16(&spi->spi_tbase, CPM_SPI_BASE_TX);

	/***********IMPORTANT******************/

	/*
	 * Setting transmit and receive buffer descriptor pointers
	 * initially to rbase and tbase. Only the microcode patches
	 * documentation talks about initializing this pointer. This
	 * is missing from the sample I2C driver. If you dont
	 * initialize these pointers, the kernel hangs.
	 */
	out_be16(&spi->spi_rbptr, CPM_SPI_BASE_RX);
	out_be16(&spi->spi_tbptr, CPM_SPI_BASE_TX);

/* 4 */
	/* Init SPI Tx + Rx Parameters */
	while (in_be16(&cp->cp_cpcr) & CPM_CR_FLG)
		;

	out_be16(&cp->cp_cpcr, mk_cr_cmd(CPM_CR_CH_SPI, CPM_CR_INIT_TRX) |
			       CPM_CR_FLG);
	while (in_be16(&cp->cp_cpcr) & CPM_CR_FLG)
		;

/* 6 */
	/* Set to big endian. */
	out_8(&spi->spi_tfcr, SMC_EB);
	out_8(&spi->spi_rfcr, SMC_EB);

/* 7 */
	/* Set maximum receive size. */
	out_be16(&spi->spi_mrblr, MAX_BUFFER);

/* 8 + 9 */
	/* tx and rx buffer descriptors */
	tbdf = (cbd_t __iomem *)&cp->cp_dpmem[CPM_SPI_BASE_TX];
	rbdf = (cbd_t __iomem *)&cp->cp_dpmem[CPM_SPI_BASE_RX];

	clrbits_be16(&tbdf->cbd_sc, BD_SC_READY);
	clrbits_be16(&rbdf->cbd_sc, BD_SC_EMPTY);

/* 10 + 11 */
	out_8(&cp->cp_spim, 0);			/* Mask  all SPI events */
	out_8(&cp->cp_spie, SPI_EMASK);		/* Clear all SPI events	*/

	return 0;
}

static void mpc8xx_spi_cs_activate(struct udevice *dev)
{
	struct mpc8xx_priv *priv = dev_get_priv(dev->parent);
	struct dm_spi_slave_plat *platdata = dev_get_parent_plat(dev);

	dm_gpio_set_value(&priv->gpios[platdata->cs[0]], 1);
}

static void mpc8xx_spi_cs_deactivate(struct udevice *dev)
{
	struct mpc8xx_priv *priv = dev_get_priv(dev->parent);
	struct dm_spi_slave_plat *platdata = dev_get_parent_plat(dev);

	dm_gpio_set_value(&priv->gpios[platdata->cs[0]], 0);
}

static int mpc8xx_spi_xfer_one(struct udevice *dev, size_t count,
			       const void *dout, void *din)
{
	immap_t __iomem *immr = (immap_t __iomem *)CONFIG_SYS_IMMR;
	cpm8xx_t __iomem *cp = &immr->im_cpm;
	cbd_t __iomem *tbdf, *rbdf;
	void *bufout, *bufin;
	u16 spmode_len;
	int tm;

	tbdf = (cbd_t __iomem *)&cp->cp_dpmem[CPM_SPI_BASE_TX];
	rbdf = (cbd_t __iomem *)&cp->cp_dpmem[CPM_SPI_BASE_RX];

	if (!(count & 1) && count >= MIN_HWORD_XFER) {
		spmode_len = SPMODE_LEN(16);
		if (dout) {
			int i;

			bufout = malloc(count);
			for (i = 0; i < count; i += 2)
				*(u16 *)(bufout + i) = swab16(*(u16 *)(dout + i));
		} else {
			bufout = NULL;
		}
		if (din)
			bufin = malloc(count);
		else
			bufin = NULL;
	} else {
		spmode_len = SPMODE_LEN(8);
		bufout = (void *)dout;
		bufin = din;
	}

	/* Setting tx bd status and data length */
	out_be32(&tbdf->cbd_bufaddr, bufout ? (ulong)bufout : (ulong)dummy_buffer);
	out_be16(&tbdf->cbd_sc, BD_SC_READY | BD_SC_LAST | BD_SC_WRAP);
	out_be16(&tbdf->cbd_datlen, count);

	/* Setting rx bd status and data length */
	out_be32(&rbdf->cbd_bufaddr, bufin ? (ulong)bufin : (ulong)dummy_buffer);
	out_be16(&rbdf->cbd_sc, BD_SC_EMPTY | BD_SC_WRAP);
	out_be16(&rbdf->cbd_datlen, 0);	 /* rx length has no significance */

	clrsetbits_be16(&cp->cp_spmode, ~(SPMODE_LOOP | SPMODE_PM(0xf) | SPMODE_DIV16),
			SPMODE_REV | SPMODE_MSTR | SPMODE_EN | spmode_len);
	out_8(&cp->cp_spim, 0);		/* Mask  all SPI events */
	out_8(&cp->cp_spie, SPI_EMASK);	/* Clear all SPI events	*/

	/* start spi transfer */
	setbits_8(&cp->cp_spcom, SPI_STR);		/* Start transmit */

	/* --------------------------------
	 * Wait for SPI transmit to get out
	 * or time out (1 second = 1000 ms)
	 * -------------------------------- */
	for (tm = 0; tm < 1000; ++tm) {
		if (in_8(&cp->cp_spie) & SPI_TXB)	/* Tx Buffer Empty */
			break;

		if ((in_be16(&tbdf->cbd_sc) & BD_SC_READY) == 0)
			break;
		udelay(1000);
	}

	if (tm >= 1000)
		return -ETIMEDOUT;

	if (!(count & 1) && count > MIN_HWORD_XFER) {
		if (dout)
			free(bufout);
		if (din) {
			int i;

			bufout = malloc(count);
			for (i = 0; i < count; i += 2)
				*(u16 *)(din + i) = swab16(*(u16 *)(bufin + i));
			free(bufin);
		}
	}

	return 0;
}

static int mpc8xx_spi_xfer(struct udevice *dev, unsigned int bitlen,
			   const void *dout, void *din, unsigned long flags)
{
	size_t count = (bitlen + 7) / 8;
	size_t offset = 0;
	int ret = 0;

	if (!din && !dout)
		return -EINVAL;

	/* Set CS for device */
	if (flags & SPI_XFER_BEGIN)
		mpc8xx_spi_cs_activate(dev);

	while (count > 0 && !ret) {
		size_t chunk = min(count, (size_t)MAX_BUFFER);
		const void *out = dout ? dout + offset : NULL;
		void *in = din ? din + offset : NULL;

		ret = mpc8xx_spi_xfer_one(dev, chunk, out, in);

		offset += chunk;
		count -= chunk;
	}
	/* Clear CS for device */
	if (flags & SPI_XFER_END)
		mpc8xx_spi_cs_deactivate(dev);

	if (ret)
		printf("*** spi_xfer: Time out while xferring to/from SPI!\n");

	return ret;
}

static int mpc8xx_spi_ofdata_to_platdata(struct udevice *dev)
{
	struct mpc8xx_priv *priv = dev_get_priv(dev);
	int ret;

	ret = gpio_request_list_by_name(dev, "gpios", priv->gpios,
					ARRAY_SIZE(priv->gpios), GPIOD_IS_OUT);
	if (ret < 0)
		return ret;

	priv->max_cs = ret;

	return 0;
}
static const struct dm_spi_ops mpc8xx_spi_ops = {
	.xfer		= mpc8xx_spi_xfer,
	.set_speed	= mpc8xx_spi_set_speed,
	.set_mode	= mpc8xx_spi_set_mode,
};

static const struct udevice_id mpc8xx_spi_ids[] = {
	{ .compatible = "fsl,mpc8xx-spi" },
	{ }
};

U_BOOT_DRIVER(mpc8xx_spi) = {
	.name	= "mpc8xx_spi",
	.id	= UCLASS_SPI,
	.of_match = mpc8xx_spi_ids,
	.of_to_plat = mpc8xx_spi_ofdata_to_platdata,
	.ops	= &mpc8xx_spi_ops,
	.probe	= mpc8xx_spi_probe,
	.priv_auto = sizeof(struct mpc8xx_priv),
};
