/* Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* This is a basic implementation of the SDMA/DMA4 controller of OMAP3
 * Tested on Silicon Revision major:0x4 minor:0x0
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/omap3.h>
#include <asm/arch/dma.h>
#include <asm/io.h>
#include <asm/errno.h>

static struct dma4 *dma4_cfg = (struct dma4 *)OMAP34XX_DMA4_BASE;
uint32_t dma_active; /* if a transfer is started the respective
	bit is set for the logical channel */

/* Check if we have the given channel
 * PARAMETERS:
 * chan: Channel number
 *
 * RETURN of non-zero means error */
static inline int check_channel(uint32_t chan)
{
	if (chan < CHAN_NR_MIN || chan > CHAN_NR_MAX)
		return -EINVAL;
	return 0;
}

static inline void reset_irq(uint32_t chan)
{
	/* reset IRQ reason */
	writel(0x1DFE, &dma4_cfg->chan[chan].csr);
	/* reset IRQ */
	writel((1 << chan), &dma4_cfg->irqstatus_l[0]);
	dma_active &= ~(1 << chan);
}

/* Set Source, Destination and Size of DMA transfer for the
 * specified channel.
 * PARAMETERS:
 * chan: channel to use
 * src: source of the transfer
 * dst: destination of the transfer
 * sze: Size of the transfer
 *
 * RETURN of non-zero means error */
int omap3_dma_conf_transfer(uint32_t chan, uint32_t *src, uint32_t *dst,
		uint32_t sze)
{
	if (check_channel(chan))
		return -EINVAL;
	/* CDSA0 */
	writel((uint32_t)src, &dma4_cfg->chan[chan].cssa);
	writel((uint32_t)dst, &dma4_cfg->chan[chan].cdsa);
	writel(sze, &dma4_cfg->chan[chan].cen);
return 0;
}

/* Start the DMA transfer */
int omap3_dma_start_transfer(uint32_t chan)
{
	uint32_t val;

	if (check_channel(chan))
		return -EINVAL;

	val = readl(&dma4_cfg->chan[chan].ccr);
	/* Test for channel already in use */
	if (val & CCR_ENABLE_ENABLE)
		return -EBUSY;

	writel((val | CCR_ENABLE_ENABLE), &dma4_cfg->chan[chan].ccr);
	dma_active |= (1 << chan);
	debug("started transfer...\n");
	return 0;
}

/* Busy-waiting for a DMA transfer
 * This has to be called before another transfer is started
 * PARAMETER
 * chan: Channel to wait for
 *
 * RETURN of non-zero means error*/
int omap3_dma_wait_for_transfer(uint32_t chan)
{
	uint32_t val;

	if (!(dma_active & (1 << chan))) {
		val = readl(&dma4_cfg->irqstatus_l[0]);
		if (!(val & chan)) {
			debug("dma: The channel you are trying to wait for "
				"was never activated - ERROR\n");
			return -1; /* channel was never active */
		}
	}

	/* all irqs on line 0 */
	while (!(readl(&dma4_cfg->irqstatus_l[0]) & (1 << chan)))
		asm("nop");

	val = readl(&dma4_cfg->chan[chan].csr);
	if ((val & CSR_TRANS_ERR) | (val & CSR_SUPERVISOR_ERR) |
			(val & CSR_MISALIGNED_ADRS_ERR)) {
		debug("err code: %X\n", val);
		debug("dma: transfer error detected\n");
		reset_irq(chan);
		return -1;
	}
	reset_irq(chan);
	return 0;
}

/* Get the revision of the DMA module
 * PARAMETER
 * minor: Address of minor revision to write
 * major: Address of major revision to write
 *
 * RETURN of non-zero means error
 */
int omap3_dma_get_revision(uint32_t *minor, uint32_t *major)
{
	uint32_t val;

	/* debug information */
	val = readl(&dma4_cfg->revision);
	*major = (val & 0x000000F0) >> 4;
	*minor = (val & 0x0000000F);
	debug("DMA Silicon revision (maj/min): 0x%X/0x%X\n", *major, *minor);
	return 0;
}

/* Initial config of omap dma
 */
void omap3_dma_init(void)
{
	dma_active = 0;
	/* All interrupts on channel 0 */
	writel(0xFFFFFFFF, &dma4_cfg->irqenable_l[0]);
}

/* set channel config to config
 *
 * RETURN of non-zero means error */
int omap3_dma_conf_chan(uint32_t chan, struct dma4_chan *config)
{
	if (check_channel(chan))
		return -EINVAL;

	dma4_cfg->chan[chan] = *config;
	return 0;
}

/* get channel config to config
 *
 * RETURN of non-zero means error */
int omap3_dma_get_conf_chan(uint32_t chan, struct dma4_chan *config)
{
	if (check_channel(chan))
		return -EINVAL;
	*config = dma4_cfg->chan[chan];
	return 0;
}
