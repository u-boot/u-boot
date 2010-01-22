/* CPU specific code for the LEON3 CPU
 *
 * (C) Copyright 2007, 2015
 * Daniel Hellstrom, Cobham Gaisler, daniel@gaisler.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <netdev.h>

#include <asm/io.h>
#include <asm/processor.h>
#include <ambapp.h>

DECLARE_GLOBAL_DATA_PTR;

extern void _reset_reloc(void);

int leon_cpu_cnt = 1;
int leon_ver = 3;
unsigned int leon_cpu_freq = CONFIG_SYS_CLK_FREQ;

int cpu_freq(void)
{
	ambapp_ahbdev dev;

	if (leon_ver == 3) {
		ambapp_ahbmst_find(&ambapp_plb, VENDOR_GAISLER,
			GAISLER_LEON3, 0, &dev);
	} else {
		ambapp_ahbmst_find(&ambapp_plb, VENDOR_GAISLER,
			GAISLER_LEON4, 0, &dev);
	}

	leon_cpu_freq = ambapp_bus_freq(&ambapp_plb, dev.ahb_bus_index);

	return 0;
}

int checkcpu(void)
{
	int cnt;
	char str[4];

	/* check LEON version here */
	cnt = ambapp_ahbmst_count(&ambapp_plb, VENDOR_GAISLER, GAISLER_LEON3);
	if (cnt <= 0) {
		cnt = ambapp_ahbmst_count(&ambapp_plb, VENDOR_GAISLER,
			GAISLER_LEON4);
		if (cnt > 0)
			leon_ver = 4;
	}

	cpu_freq();

	str[0] = '\0';
	if (cnt > 1) {
		leon_cpu_cnt = cnt;
		str[0] = '0' + cnt;
		str[1] = 'x';
		str[2] = '\0';
	}
	printf("CPU: %sLEON%d @ %dMHz\n", str, leon_ver,
		leon_cpu_freq / 1000000);

	return 0;
}

#ifdef CONFIG_DISPLAY_CPUINFO

int print_cpuinfo(void)
{
	printf("CPU:   LEON3\n");
	return 0;
}

#endif

/* ------------------------------------------------------------------------- */

void cpu_reset(void)
{
	/* Interrupts off */
	disable_interrupts();

	/* jump to restart in flash */
	_reset_reloc();
}

int do_reset(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	cpu_reset();

	return 1;

}

u64 flash_read64(void *addr)
{
	return __raw_readq(addr);
}

/* ------------------------------------------------------------------------- */

#ifdef CONFIG_GRETH
int cpu_eth_init(bd_t *bis)
{
	return greth_initialize(bis);
}
#endif
