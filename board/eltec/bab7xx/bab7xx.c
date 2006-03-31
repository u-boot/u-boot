/*
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>
 * (C) Copyright 2001 ELTEC Elektronik AG
 * Frank Gottschling <fgottschling@eltec.de>
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
#include <command.h>
#include <mpc106.h>
#include <mk48t59.h>
#include <74xx_7xx.h>
#include <ns87308.h>
#include <video_fb.h>

DECLARE_GLOBAL_DATA_PTR;

/*---------------------------------------------------------------------------*/
/*
 * Get Bus clock frequency
 */
ulong bab7xx_get_bus_freq (void)
{
	/*
	 * The GPIO Port 1 on BAB7xx reflects the bus speed.
	 */
	volatile struct GPIO *gpio =
		(struct GPIO *) (CFG_ISA_IO + CFG_NS87308_GPIO_BASE);

	unsigned char data = gpio->dta1;

	if (data & 0x02)
		return 66666666;

	return 83333333;
}

/*---------------------------------------------------------------------------*/

/*
 * Measure CPU clock speed (core clock GCLK1) (Approx. GCLK frequency in Hz)
 */
ulong bab7xx_get_gclk_freq (void)
{
	static const int pllratio_to_factor[] = {
		00, 75, 70, 00, 20, 65, 100, 45, 30, 55, 40, 50, 80, 60, 35,
			00,
	};

	return pllratio_to_factor[get_hid1 () >> 28] *
		(bab7xx_get_bus_freq () / 10);
}

/*----------------------------------------------------------------------------*/

int checkcpu (void)
{
	uint pvr = get_pvr ();

	printf ("MPC7xx V%d.%d", (pvr >> 8) & 0xFF, pvr & 0xFF);
	printf (" at %ld / %ld MHz\n", bab7xx_get_gclk_freq () / 1000000,
		bab7xx_get_bus_freq () / 1000000);

	return (0);
}

/* ------------------------------------------------------------------------- */

int checkboard (void)
{
#ifdef CFG_ADDRESS_MAP_A
	puts ("Board: ELTEC BAB7xx PReP\n");
#else
	puts ("Board: ELTEC BAB7xx CHRP\n");
#endif
	return (0);
}

/* ------------------------------------------------------------------------- */

int checkflash (void)
{
	/* TODO: XXX XXX XXX */
	printf ("2 MB ## Test not implemented yet ##\n");
	return (0);
}

/* ------------------------------------------------------------------------- */


static unsigned int mpc106_read_cfg_dword (unsigned int reg)
{
	unsigned int reg_addr = MPC106_REG | (reg & 0xFFFFFFFC);

	out32r (MPC106_REG_ADDR, reg_addr);

	return (in32r (MPC106_REG_DATA | (reg & 0x3)));
}

/* ------------------------------------------------------------------------- */

long int dram_size (int board_type)
{
	/* No actual initialisation to do - done when setting up
	 * PICRs MCCRs ME/SARs etc in ram_init.S.
	 */

	register unsigned long i, msar1, mear1, memSize;

#if defined(CFG_MEMTEST)
	register unsigned long reg;

	printf ("Testing DRAM\n");

	/* write each mem addr with it's address */
	for (reg = CFG_MEMTEST_START; reg < CFG_MEMTEST_END; reg += 4)
		*reg = reg;

	for (reg = CFG_MEMTEST_START; reg < CFG_MEMTEST_END; reg += 4) {
		if (*reg != reg)
			return -1;
	}
#endif

	/*
	 * Since MPC106 memory controller chip has already been set to
	 * control all memory, just read and interpret its memory boundery register.
	 */
	memSize = 0;
	msar1 = mpc106_read_cfg_dword (MPC106_MSAR1);
	mear1 = mpc106_read_cfg_dword (MPC106_MEAR1);
	i = mpc106_read_cfg_dword (MPC106_MBER) & 0xf;

	do {
		if (i & 0x01)	/* is bank enabled ? */
			memSize += (mear1 & 0xff) - (msar1 & 0xff) + 1;
		msar1 >>= 8;
		mear1 >>= 8;
		i >>= 1;
	} while (i);

	return (memSize * 0x100000);
}

/* ------------------------------------------------------------------------- */

long int initdram (int board_type)
{
	return dram_size (board_type);
}

/* ------------------------------------------------------------------------- */

void after_reloc (ulong dest_addr)
{
	/*
	 * Jump to the main U-Boot board init code
	 */
	board_init_r ((gd_t *) gd, dest_addr);
}

/* ------------------------------------------------------------------------- */

/*
 * do_reset is done here because in this case it is board specific, since the
 * 7xx CPUs can only be reset by external HW (the RTC in this case).
 */
void do_reset (cmd_tbl_t * cmdtp, bd_t * bd, int flag, int argc, char *argv[])
{
#if defined(CONFIG_RTC_MK48T59)
	/* trigger watchdog immediately */
	rtc_set_watchdog (1, RTC_WD_RB_16TH);
#else
#error "You must define the macro CONFIG_RTC_MK48T59."
#endif
}

/* ------------------------------------------------------------------------- */

#if defined(CONFIG_WATCHDOG)
/*
 * Since the 7xx CPUs don't have an internal watchdog, this function is
 * board specific.  We use the RTC here.
 */
void watchdog_reset (void)
{
#if defined(CONFIG_RTC_MK48T59)
	/* we use a 32 sec watchdog timer */
	rtc_set_watchdog (8, RTC_WD_RB_4);
#else
#error "You must define the macro CONFIG_RTC_MK48T59."
#endif
}
#endif /* CONFIG_WATCHDOG */

/* ------------------------------------------------------------------------- */

#ifdef CONFIG_CONSOLE_EXTRA_INFO
extern GraphicDevice smi;

void video_get_info_str (int line_number, char *info)
{
	/* init video info strings for graphic console */
	switch (line_number) {
	case 1:
		sprintf (info, " MPC7xx V%d.%d at %ld / %ld MHz",
			 (get_pvr () >> 8) & 0xFF,
			 get_pvr () & 0xFF,
			 bab7xx_get_gclk_freq () / 1000000,
			 bab7xx_get_bus_freq () / 1000000);
		return;
	case 2:
		sprintf (info,
			 " ELTEC BAB7xx with %ld MB DRAM and %ld MB FLASH",
			 dram_size (0) / 0x100000, flash_init () / 0x100000);
		return;
	case 3:
		sprintf (info, " %s", smi.modeIdent);
		return;
	}

	/* no more info lines */
	*info = 0;
	return;
}
#endif

/*---------------------------------------------------------------------------*/
