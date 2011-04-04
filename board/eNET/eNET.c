/*
 * (C) Copyright 2008
 * Graeme Russ, graeme.russ@gmail.com.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/ic/sc520.h>
#include <net.h>
#include <netdev.h>

#ifdef CONFIG_HW_WATCHDOG
#include <watchdog.h>
#endif

#include "hardware.h"

DECLARE_GLOBAL_DATA_PTR;

unsigned long monitor_flash_len = CONFIG_SYS_MONITOR_LEN;

static void enet_timer_isr(void);
static void enet_toggle_run_led(void);
static void enet_setup_pars(void);

/*
 * Miscellaneous platform dependent initializations
 */
int board_early_init_f(void)
{
	u16 pio_out_cfg = 0x0000;

	/* Configure General Purpose Bus timing */
	writeb(CONFIG_SYS_SC520_GPCSRT, &sc520_mmcr->gpcsrt);
	writeb(CONFIG_SYS_SC520_GPCSPW, &sc520_mmcr->gpcspw);
	writeb(CONFIG_SYS_SC520_GPCSOFF, &sc520_mmcr->gpcsoff);
	writeb(CONFIG_SYS_SC520_GPRDW, &sc520_mmcr->gprdw);
	writeb(CONFIG_SYS_SC520_GPRDOFF, &sc520_mmcr->gprdoff);
	writeb(CONFIG_SYS_SC520_GPWRW, &sc520_mmcr->gpwrw);
	writeb(CONFIG_SYS_SC520_GPWROFF, &sc520_mmcr->gpwroff);

	/* Configure Programmable Input/Output Pins */
	writew(CONFIG_SYS_SC520_PIODIR15_0, &sc520_mmcr->piodir15_0);
	writew(CONFIG_SYS_SC520_PIODIR31_16, &sc520_mmcr->piodir31_16);
	writew(CONFIG_SYS_SC520_PIOPFS31_16, &sc520_mmcr->piopfs31_16);
	writew(CONFIG_SYS_SC520_PIOPFS15_0, &sc520_mmcr->piopfs15_0);
	writeb(CONFIG_SYS_SC520_CSPFS, &sc520_mmcr->cspfs);
	writeb(CONFIG_SYS_SC520_CLKSEL, &sc520_mmcr->clksel);

	/*
	 * Turn off top board
	 * Set StrataFlash chips to 16-bit width
	 * Set StrataFlash chips to normal (non reset/power down) mode
	 */
	pio_out_cfg |= CONFIG_SYS_ENET_TOP_BRD_PWR;
	pio_out_cfg |= CONFIG_SYS_ENET_SF_WIDTH;
	pio_out_cfg |= CONFIG_SYS_ENET_SF1_MODE;
	pio_out_cfg |= CONFIG_SYS_ENET_SF2_MODE;
	writew(pio_out_cfg, &sc520_mmcr->pioset15_0);

	/* Turn off auxiliary power output */
	writew(CONFIG_SYS_ENET_AUX_PWR, &sc520_mmcr->pioclr15_0);

	/* Clear FPGA program mode */
	writew(CONFIG_SYS_ENET_FPGA_PROG, &sc520_mmcr->pioset31_16);

	enet_setup_pars();

	/* Disable Watchdog */
	writew(0x3333, &sc520_mmcr->wdtmrctl);
	writew(0xcccc, &sc520_mmcr->wdtmrctl);
	writew(0x0000, &sc520_mmcr->wdtmrctl);

	/* Chip Select Configuration */
	writew(CONFIG_SYS_SC520_BOOTCS_CTRL, &sc520_mmcr->bootcsctl);
	writew(CONFIG_SYS_SC520_ROMCS1_CTRL, &sc520_mmcr->romcs1ctl);
	writew(CONFIG_SYS_SC520_ROMCS2_CTRL, &sc520_mmcr->romcs2ctl);

	writeb(CONFIG_SYS_SC520_ADDDECCTL, &sc520_mmcr->adddecctl);
	writeb(CONFIG_SYS_SC520_UART1CTL, &sc520_mmcr->uart1ctl);
	writeb(CONFIG_SYS_SC520_UART2CTL, &sc520_mmcr->uart2ctl);

	writeb(CONFIG_SYS_SC520_SYSARBCTL, &sc520_mmcr->sysarbctl);
	writew(CONFIG_SYS_SC520_SYSARBMENB, &sc520_mmcr->sysarbmenb);

	/* enable posted-writes */
	writeb(CONFIG_SYS_SC520_HBCTL, &sc520_mmcr->hbctl);

	return 0;
}

static void enet_setup_pars(void)
{
	/*
	 * PARs 11 and 12 are 2MB SRAM @ 0x19000000
	 *
	 * These are setup now because older version of U-Boot have them
	 * mapped to a different PAR which gets clobbered which prevents
	 * using SRAM for warm-booting a new image
	 */
	writel(CONFIG_SYS_SC520_SRAM1_PAR, &sc520_mmcr->par[11]);
	writel(CONFIG_SYS_SC520_SRAM2_PAR, &sc520_mmcr->par[12]);

	/* PARs 0 and 1 are Compact Flash slots (4kB each) */
	writel(CONFIG_SYS_SC520_CF1_PAR, &sc520_mmcr->par[0]);
	writel(CONFIG_SYS_SC520_CF2_PAR, &sc520_mmcr->par[1]);

	/* PAR 2 is used for Cache-As-RAM */

	/*
	 * PARs 5 through 8 are additional NS16550 UARTS
	 * 8 bytes each @ 0x013f8, 0x012f8, 0x011f8 and 0x010f8
	 */
	writel(CONFIG_SYS_SC520_UARTA_PAR, &sc520_mmcr->par[5]);
	writel(CONFIG_SYS_SC520_UARTB_PAR, &sc520_mmcr->par[6]);
	writel(CONFIG_SYS_SC520_UARTC_PAR, &sc520_mmcr->par[7]);
	writel(CONFIG_SYS_SC520_UARTD_PAR, &sc520_mmcr->par[8]);

	/* PARs 9 and 10 are 32MB StrataFlash @ 0x10000000 */
	writel(CONFIG_SYS_SC520_SF1_PAR, &sc520_mmcr->par[9]);
	writel(CONFIG_SYS_SC520_SF2_PAR, &sc520_mmcr->par[10]);

	/* PAR 13 is 4kB DPRAM @ 0x18100000 (implemented in FPGA) */
	writel(CONFIG_SYS_SC520_DPRAM_PAR, &sc520_mmcr->par[13]);

	/*
	 * PAR 14 is Low Level I/O (LEDs, Hex Switches etc)
	 * Already configured in board_init16 (eNET_start16.S)
	 *
	 * PAR 15 is Boot ROM
	 * Already configured in board_init16 (eNET_start16.S)
	 */
}


int board_early_init_r(void)
{
	/* CPU Speed to 100MHz */
	gd->cpu_clk = 100000000;

	/* Crystal is 33.000MHz */
	gd->bus_clk = 33000000;

	return 0;
}

void show_boot_progress(int val)
{
	uchar led_mask;

	led_mask = 0x00;

	if (val < 0)
		led_mask |= LED_ERR_BITMASK;

	led_mask |= (uchar)(val & 0x001f);
	outb(led_mask, LED_LATCH_ADDRESS);
}


int last_stage_init(void)
{
	int minor;
	int major;

	major = minor = 0;

	outb(0x00, LED_LATCH_ADDRESS);

	register_timer_isr(enet_timer_isr);

	printf("Serck Controls eNET\n");

	return 0;
}

ulong board_flash_get_legacy(ulong base, int banknum, flash_info_t *info)
{
	if (banknum == 0) {	/* non-CFI boot flash */
		info->portwidth = FLASH_CFI_8BIT;
		info->chipwidth = FLASH_CFI_BY8;
		info->interface = FLASH_CFI_X8;
		return 1;
	} else {
		return 0;
	}
}

int board_eth_init(bd_t *bis)
{
	return pci_eth_init(bis);
}

void setup_pcat_compatibility()
{
	/* disable global interrupt mode */
	writeb(0x40, &sc520_mmcr->picicr);

	/* set all irqs to edge */
	writeb(0x00, &sc520_mmcr->pic_mode[0]);
	writeb(0x00, &sc520_mmcr->pic_mode[1]);
	writeb(0x00, &sc520_mmcr->pic_mode[2]);

	/*
	 *  active low polarity on PIC interrupt pins,
	 *  active high polarity on all other irq pins
	 */
	writew(0x0000,&sc520_mmcr->intpinpol);

	/*
	 * PIT 0 -> IRQ0
	 * RTC -> IRQ8
	 * FP error -> IRQ13
	 * UART1 -> IRQ4
	 * UART2 -> IRQ3
	 */
	writeb(SC520_IRQ0, &sc520_mmcr->pit_int_map[0]);
	writeb(SC520_IRQ8, &sc520_mmcr->rtcmap);
	writeb(SC520_IRQ13, &sc520_mmcr->ferrmap);
	writeb(SC520_IRQ4, &sc520_mmcr->uart_int_map[0]);
	writeb(SC520_IRQ3, &sc520_mmcr->uart_int_map[1]);

	/* Disable all other interrupt sources */
	writeb(SC520_IRQ_DISABLED, &sc520_mmcr->gp_tmr_int_map[0]);
	writeb(SC520_IRQ_DISABLED, &sc520_mmcr->gp_tmr_int_map[1]);
	writeb(SC520_IRQ_DISABLED, &sc520_mmcr->gp_tmr_int_map[2]);
	writeb(SC520_IRQ_DISABLED, &sc520_mmcr->pit_int_map[1]);
	writeb(SC520_IRQ_DISABLED, &sc520_mmcr->pit_int_map[2]);
	writeb(SC520_IRQ_DISABLED, &sc520_mmcr->ssimap);
	writeb(SC520_IRQ_DISABLED, &sc520_mmcr->wdtmap);
	writeb(SC520_IRQ_DISABLED, &sc520_mmcr->wpvmap);
	writeb(SC520_IRQ_DISABLED, &sc520_mmcr->icemap);
}

void enet_timer_isr(void)
{
	static long enet_ticks = 0;

	enet_ticks++;

	/* Toggle Watchdog every 100ms */
	if ((enet_ticks % 100) == 0)
		hw_watchdog_reset();

	/* Toggle Run LED every 500ms */
	if ((enet_ticks % 500) == 0)
		enet_toggle_run_led();
}

void hw_watchdog_reset(void)
{
	/* Watchdog Reset must be atomic */
	long flag = disable_interrupts();

	if (sc520_mmcr->piodata15_0 & WATCHDOG_PIO_BIT)
		sc520_mmcr->pioclr15_0 = WATCHDOG_PIO_BIT;
	else
		sc520_mmcr->pioset15_0 = WATCHDOG_PIO_BIT;

	if (flag)
		enable_interrupts();
}

void enet_toggle_run_led(void)
{
	unsigned char leds_state= inb(LED_LATCH_ADDRESS);
	if (leds_state & LED_RUN_BITMASK)
		outb(leds_state &~ LED_RUN_BITMASK, LED_LATCH_ADDRESS);
	else
		outb(leds_state | LED_RUN_BITMASK, LED_LATCH_ADDRESS);
}
