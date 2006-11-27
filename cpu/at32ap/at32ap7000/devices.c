/*
 * Copyright (C) 2006 Atmel Corporation
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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
#include <common.h>

#include <asm/arch/memory-map.h>
#include <asm/arch/platform.h>

#include "../sm.h"

#define ARRAY_SIZE(x)	(sizeof(x) / sizeof((x)[0]))

const struct clock_domain chip_clock[] = {
	[CLOCK_CPU] = {
		.reg	= SM_PM_CPU_MASK,
		.id	= CLOCK_CPU,
		.bridge	= NO_DEVICE,
	},
	[CLOCK_HSB] = {
		.reg	= SM_PM_HSB_MASK,
		.id	= CLOCK_HSB,
		.bridge	= NO_DEVICE,
	},
	[CLOCK_PBA] = {
		.reg	= SM_PM_PBA_MASK,
		.id	= CLOCK_PBA,
		.bridge	= DEVICE_PBA_BRIDGE,
	},
	[CLOCK_PBB] = {
		.reg	= SM_PM_PBB_MASK,
		.id	= CLOCK_PBB,
		.bridge	= DEVICE_PBB_BRIDGE,
	},
};

static const struct resource hebi_resource[] = {
	{
		.type	= RESOURCE_CLOCK,
		.u	= {
			.clock	= { CLOCK_HSB, 0 },
		},
	}, {
		.type	= RESOURCE_CLOCK,
		.u	= {
			.clock	= { CLOCK_PBB, 13 },
		},
	}, {
		.type	= RESOURCE_CLOCK,
		.u	= {
			.clock	= { CLOCK_PBB, 14 },
		},
	}, {
		.type	= RESOURCE_GPIO,
		.u	= {
			.gpio	= { 27, DEVICE_PIOE, GPIO_FUNC_A, 0 },
		},
	},
};
static const struct resource pba_bridge_resource[] = {
	{
		.type	= RESOURCE_CLOCK,
		.u	= {
			.clock	= { CLOCK_HSB, 1 },
		}
	}, {
		.type	= RESOURCE_CLOCK,
		.u	= {
			/* HSB-HSB Bridge */
			.clock	= { CLOCK_HSB, 4 },
		},
	},
};
static const struct resource pbb_bridge_resource[] = {
	{
		.type	= RESOURCE_CLOCK,
		.u	= {
			.clock	= { CLOCK_HSB, 2 },
		},
	},
};
static const struct resource hramc_resource[] = {
	{
		.type	= RESOURCE_CLOCK,
		.u	= {
			.clock	= { CLOCK_HSB, 3 },
		},
	},
};
static const struct resource pioa_resource[] = {
	{
		.type	= RESOURCE_CLOCK,
		.u	= {
			.clock	= { CLOCK_PBA, 10 },
		},
	},
};
static const struct resource piob_resource[] = {
	{
		.type	= RESOURCE_CLOCK,
		.u	= {
			.clock	= { CLOCK_PBA, 11 },
		},
	},
};
static const struct resource pioc_resource[] = {
	{
		.type	= RESOURCE_CLOCK,
		.u	= {
			.clock	= { CLOCK_PBA, 12 },
		},
	},
};
static const struct resource piod_resource[] = {
	{
		.type	= RESOURCE_CLOCK,
		.u	= {
			.clock	= { CLOCK_PBA, 13 },
		},
	},
};
static const struct resource pioe_resource[] = {
	{
		.type	= RESOURCE_CLOCK,
		.u	= {
			.clock	= { CLOCK_PBA, 14 },
		},
	},
};
static const struct resource sm_resource[] = {
	{
		.type	= RESOURCE_CLOCK,
		.u	= {
			.clock	= { CLOCK_PBB, 0 },
		},
	},
};
static const struct resource intc_resource[] = {
	{
		.type	= RESOURCE_CLOCK,
		.u	= {
			.clock = { CLOCK_PBB, 1 },
		},
	},
};
static const struct resource hmatrix_resource[] = {
	{
		.type	= RESOURCE_CLOCK,
		.u	= {
			.clock = { CLOCK_PBB, 2 },
		},
	},
};
#if defined(CFG_HPDC)
static const struct resource hpdc_resource[] = {
	{
		.type	= RESOURCE_CLOCK,
		.u	= {
			.clock	= { CLOCK_PBA, 16 },
		},
	},
};
#endif
#if defined(CFG_MACB0)
static const struct resource macb0_resource[] = {
	{
		.type	= RESOURCE_CLOCK,
		.u	= {
			.clock	= { CLOCK_HSB, 8 },
		},
	}, {
		.type	= RESOURCE_CLOCK,
		.u	= {
			.clock	= { CLOCK_PBB, 6 },
		},
	}, {
		.type	= RESOURCE_GPIO,
		.u	= {
			.gpio	= { 19, DEVICE_PIOC, GPIO_FUNC_A, 0 },
		},
	},
};
#endif
#if defined(CFG_MACB1)
static const struct resource macb1_resource[] = {
	{
		.type	= RESOURCE_CLOCK,
		.u	= {
			.clock	= { CLOCK_HSB, 9 },
		},
	}, {
		.type	= RESOURCE_CLOCK,
		.u	= {
			.clock	= { CLOCK_PBB, 7 },
		},
	}, {
		.type	= RESOURCE_GPIO,
		.u	= {
			.gpio	= { 12, DEVICE_PIOC, GPIO_FUNC_B, 19 },
		},
	}, {
		.type	= RESOURCE_GPIO,
		.u	= {
			.gpio	= { 14, DEVICE_PIOD, GPIO_FUNC_B, 2 },
		},
	},
};
#endif
#if defined(CFG_LCDC)
static const struct resource lcdc_resource[] = {
	{
		.type	= RESOURCE_CLOCK,
		.u	= {
			.clock	= { CLOCK_HSB, 7 },
		},
	},
};
#endif
#if defined(CFG_USART0)
static const struct resource usart0_resource[] = {
	{
		.type	= RESOURCE_CLOCK,
		.u	= {
			.clock	= { CLOCK_PBA, 3 },
		},
	}, {
		.type	= RESOURCE_GPIO,
		.u	= {
			.gpio = { 2, DEVICE_PIOA, GPIO_FUNC_B, 8 },
		},
	},
};
#endif
#if defined(CFG_USART1)
static const struct resource usart1_resource[] = {
	{
		.type	= RESOURCE_CLOCK,
		.u	= {
			.clock	= { CLOCK_PBA, 4 },
		},
	}, {
		.type	= RESOURCE_GPIO,
		.u	= {
			.gpio = { 2, DEVICE_PIOA, GPIO_FUNC_A, 17 },
		},
	},
};
#endif
#if defined(CFG_USART2)
static const struct resource usart2_resource[] = {
	{
		.type	= RESOURCE_CLOCK,
		.u	= {
			.clock	= { CLOCK_PBA, 5 },
		},
	}, {
		.type	= RESOURCE_GPIO,
		.u	= {
			.gpio = { 2, DEVICE_PIOB, GPIO_FUNC_B, 26 },
		},
	},
};
#endif
#if defined(CFG_USART3)
static const struct resource usart3_resource[] = {
	{
		.type	= RESOURCE_CLOCK,
		.u	= {
			.clock	= { CLOCK_PBA, 6 },
		},
	}, {
		.type	= RESOURCE_GPIO,
		.u	= {
			.gpio = { 2, DEVICE_PIOB, GPIO_FUNC_B, 17 },
		},
	},
};
#endif
#if defined(CFG_MMCI)
static const struct resource mmci_resource[] = {
	{
		.type	= RESOURCE_CLOCK,
		.u	= {
			.clock	= { CLOCK_PBB, 9 },
		},
	}, {
		.type	= RESOURCE_GPIO,
		.u	= {
			.gpio = { 6, DEVICE_PIOA, GPIO_FUNC_A, 10 },
		},
	},
};
#endif
#if defined(CFG_DMAC)
static const struct resource dmac_resource[] = {
	{
		.type	= RESOURCE_CLOCK,
		.u	= {
			.clock	= { CLOCK_HSB, 10 },
		},
	},
};
#endif

const struct device chip_device[] = {
	[DEVICE_HEBI] = {
		.regs		= (void *)HSMC_BASE,
		.nr_resources	= ARRAY_SIZE(hebi_resource),
		.resource	= hebi_resource,
	},
	[DEVICE_PBA_BRIDGE] = {
		.nr_resources	= ARRAY_SIZE(pba_bridge_resource),
		.resource	= pba_bridge_resource,
	},
	[DEVICE_PBB_BRIDGE] = {
		.nr_resources	= ARRAY_SIZE(pbb_bridge_resource),
		.resource	= pbb_bridge_resource,
	},
	[DEVICE_HRAMC] = {
		.nr_resources	= ARRAY_SIZE(hramc_resource),
		.resource	= hramc_resource,
	},
	[DEVICE_PIOA] = {
		.regs		= (void *)PIOA_BASE,
		.nr_resources	= ARRAY_SIZE(pioa_resource),
		.resource	= pioa_resource,
	},
	[DEVICE_PIOB] = {
		.regs		= (void *)PIOB_BASE,
		.nr_resources	= ARRAY_SIZE(piob_resource),
		.resource	= piob_resource,
	},
	[DEVICE_PIOC] = {
		.regs		= (void *)PIOC_BASE,
		.nr_resources	= ARRAY_SIZE(pioc_resource),
		.resource	= pioc_resource,
	},
	[DEVICE_PIOD] = {
		.regs		= (void *)PIOD_BASE,
		.nr_resources	= ARRAY_SIZE(piod_resource),
		.resource	= piod_resource,
	},
	[DEVICE_PIOE] = {
		.regs		= (void *)PIOE_BASE,
		.nr_resources	= ARRAY_SIZE(pioe_resource),
		.resource	= pioe_resource,
	},
	[DEVICE_SM] = {
		.regs		= (void *)SM_BASE,
		.nr_resources	= ARRAY_SIZE(sm_resource),
		.resource	= sm_resource,
	},
	[DEVICE_INTC] = {
		.regs		= (void *)INTC_BASE,
		.nr_resources	= ARRAY_SIZE(intc_resource),
		.resource	= intc_resource,
	},
	[DEVICE_HMATRIX] = {
		.regs		= (void *)HMATRIX_BASE,
		.nr_resources	= ARRAY_SIZE(hmatrix_resource),
		.resource	= hmatrix_resource,
	},
#if defined(CFG_HPDC)
	[DEVICE_HPDC] = {
		.nr_resources	= ARRAY_SIZE(hpdc_resource),
		.resource	= hpdc_resource,
	},
#endif
#if defined(CFG_MACB0)
	[DEVICE_MACB0] = {
		.regs		= (void *)MACB0_BASE,
		.nr_resources	= ARRAY_SIZE(macb0_resource),
		.resource	= macb0_resource,
	},
#endif
#if defined(CFG_MACB1)
	[DEVICE_MACB1] = {
		.regs		= (void *)MACB1_BASE,
		.nr_resources	= ARRAY_SIZE(macb1_resource),
		.resource	= macb1_resource,
	},
#endif
#if defined(CFG_LCDC)
	[DEVICE_LCDC] = {
		.nr_resources	= ARRAY_SIZE(lcdc_resource),
		.resource	= lcdc_resource,
	},
#endif
#if defined(CFG_USART0)
	[DEVICE_USART0] = {
		.regs		= (void *)USART0_BASE,
		.nr_resources	= ARRAY_SIZE(usart0_resource),
		.resource	= usart0_resource,
	},
#endif
#if defined(CFG_USART1)
	[DEVICE_USART1] = {
		.regs		= (void *)USART1_BASE,
		.nr_resources	= ARRAY_SIZE(usart1_resource),
		.resource	= usart1_resource,
	},
#endif
#if defined(CFG_USART2)
	[DEVICE_USART2] = {
		.regs		= (void *)USART2_BASE,
		.nr_resources	= ARRAY_SIZE(usart2_resource),
		.resource	= usart2_resource,
	},
#endif
#if defined(CFG_USART3)
	[DEVICE_USART3] = {
		.regs		= (void *)USART3_BASE,
		.nr_resources	= ARRAY_SIZE(usart3_resource),
		.resource	= usart3_resource,
	},
#endif
#if defined(CFG_MMCI)
	[DEVICE_MMCI] = {
		.regs		= (void *)MMCI_BASE,
		.nr_resources	= ARRAY_SIZE(mmci_resource),
		.resource	= mmci_resource,
	},
#endif
#if defined(CFG_DMAC)
	[DEVICE_DMAC] = {
		.regs		= (void *)DMAC_BASE,
		.nr_resources	= ARRAY_SIZE(dmac_resource),
		.resource	= dmac_resource,
	},
#endif
};
