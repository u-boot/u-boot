/*
 * (C) Copyright 2010-2011
 * NVIDIA Corporation <www.nvidia.com>
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
#include <asm/types.h>

/* Stabilization delays, in usec */
#define PLL_STABILIZATION_DELAY (300)
#define IO_STABILIZATION_DELAY	(1000)

#if defined(CONFIG_TEGRA20)
#define NVBL_PLLP_KHZ	(216000)
#elif defined(CONFIG_TEGRA30) || defined(CONFIG_TEGRA114)
#define NVBL_PLLP_KHZ	(408000)
#else
#error "Unknown Tegra chip!"
#endif

#define PLLX_ENABLED		(1 << 30)
#define CCLK_BURST_POLICY	0x20008888
#define SUPER_CCLK_DIVIDER	0x80000000

/* Calculate clock fractional divider value from ref and target frequencies */
#define CLK_DIVIDER(REF, FREQ)  ((((REF) * 2) / FREQ) - 2)

/* Calculate clock frequency value from reference and clock divider value */
#define CLK_FREQUENCY(REF, REG)  (((REF) * 2) / (REG + 2))

/* AVP/CPU ID */
#define PG_UP_TAG_0_PID_CPU	0x55555555	/* CPU aka "a9" aka "mpcore" */
#define PG_UP_TAG_0             0x0

#define CORESIGHT_UNLOCK	0xC5ACCE55;

#define EXCEP_VECTOR_CPU_RESET_VECTOR	(NV_PA_EVP_BASE + 0x100)
#define CSITE_CPU_DBG0_LAR		(NV_PA_CSITE_BASE + 0x10FB0)
#define CSITE_CPU_DBG1_LAR		(NV_PA_CSITE_BASE + 0x12FB0)
#define CSITE_CPU_DBG2_LAR		(NV_PA_CSITE_BASE + 0x14FB0)
#define CSITE_CPU_DBG3_LAR		(NV_PA_CSITE_BASE + 0x16FB0)

#define FLOW_CTLR_HALT_COP_EVENTS	(NV_PA_FLOW_BASE + 4)
#define FLOW_MODE_STOP			2
#define HALT_COP_EVENT_JTAG		(1 << 28)
#define HALT_COP_EVENT_IRQ_1		(1 << 11)
#define HALT_COP_EVENT_FIQ_1		(1 << 9)

#define FLOW_MODE_NONE		0

#define SIMPLE_PLLX     (CLOCK_ID_XCPU - CLOCK_ID_FIRST_SIMPLE)

struct clk_pll_table {
	u16	n;
	u16	m;
	u8	p;
	u8	cpcon;
};

void clock_enable_coresight(int enable);
void enable_cpu_clock(int enable);
void halt_avp(void)  __attribute__ ((noreturn));
void init_pllx(void);
void powerup_cpu(void);
void reset_A9_cpu(int reset);
void start_cpu(u32 reset_vector);
int tegra_get_chip_type(void);
void adjust_pllp_out_freqs(void);
