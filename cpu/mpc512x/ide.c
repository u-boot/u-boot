/*
 * (C) Copyright 2007-2009 DENX Software Engineering
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
 *
 */

#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <asm/processor.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_IDE_RESET)

void ide_set_reset (int idereset)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	debug ("ide_set_reset(%d)\n", idereset);

	if (idereset) {
		out_be32(&im->pata.pata_ata_control, 0);
	} else {
		out_be32(&im->pata.pata_ata_control, FSL_ATA_CTRL_ATA_RST_B);
	}
	udelay(100);
}

void init_ide_reset (void)
{
	debug ("init_ide_reset\n");

	/*
	 * Clear the reset bit to reset the interface
	 * cf. RefMan MPC5121EE: 28.4.1 Resetting the ATA Bus
	 */
	ide_set_reset(1);

	/* Assert the reset bit to enable the interface */
	ide_set_reset(0);

}

#define CALC_TIMING(t) (t + period - 1) / period

int ide_preinit (void)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	long t;
	const struct {
		short t0;
		short t1;
		short t2_8;
		short t2_16;
		short t2i;
		short t4;
		short t9;
		short tA;
	} pio_specs = {
		.t0    = 600,
		.t1    =  70,
		.t2_8  = 290,
		.t2_16 = 165,
		.t2i   =   0,
		.t4    =  30,
		.t9    =  20,
		.tA    =  50,
	};
	union {
		u32 config;
		struct {
			u8 field1;
			u8 field2;
			u8 field3;
			u8 field4;
		}bytes;
	} cfg;

	debug ("IDE preinit using PATA peripheral at IMMR-ADDR %08x\n",
		(u32)&im->pata);

	/* Set the reset bit to 1 to enable the interface */
	ide_set_reset(0);

	/* Init timings : we use PIO mode 0 timings */
	t = 1000000000 / gd->ips_clk;	/* period in ns */
	cfg.bytes.field1 = 3;
	cfg.bytes.field2 = 3;
	cfg.bytes.field3 = (pio_specs.t1 + t) / t;
	cfg.bytes.field4 = (pio_specs.t2_8 + t) / t;

	out_be32(&im->pata.pata_time1, cfg.config);

	cfg.bytes.field1 = (pio_specs.t2_8 + t) / t;
	cfg.bytes.field2 = (pio_specs.tA + t) / t + 2;
	cfg.bytes.field3 = 1;
	cfg.bytes.field4 = (pio_specs.t4 + t) / t;

	out_be32(&im->pata.pata_time2, cfg.config);

	cfg.config = in_be32(&im->pata.pata_time3);
	cfg.bytes.field1 = (pio_specs.t9 + t) / t;

	out_be32(&im->pata.pata_time3, cfg.config);

	debug ("PATA preinit complete.\n");

	return 0;
}

#endif /* defined(CONFIG_IDE_RESET) */
