/*
 * (C) Copyright 2004
 * Stefan Roese, esd gmbh germany, stefan.roese@esd-electronics.com
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
#include <asm/processor.h>
#include <command.h>

#define MEM_MCOPT1_INIT_VAL     0x00800000
#define MEM_RTR_INIT_VAL        0x04070000
#define MEM_PMIT_INIT_VAL       0x07c00000
#define MEM_MB0CF_INIT_VAL      0x00082001
#define MEM_MB1CF_INIT_VAL      0x04082000
#define MEM_SDTR1_INIT_VAL      0x00854005
#define SDRAM0_CFG_ENABLE       0x80000000

#define CFG_SDRAM_SIZE          0x04000000      /* 64 MB */

int board_early_init_f (void)
{
#if 0 /* test-only */
	mtdcr (uicsr, 0xFFFFFFFF);      /* clear all ints */
	mtdcr (uicer, 0x00000000);      /* disable all ints */
	mtdcr (uiccr, 0x00000010);
	mtdcr (uicpr, 0xFFFF7FF0);      /* set int polarities */
	mtdcr (uictr, 0x00000010);      /* set int trigger levels */
	mtdcr (uicsr, 0xFFFFFFFF);      /* clear all ints */
#else
	mtdcr(uicsr, 0xFFFFFFFF);       /* clear all ints */
	mtdcr(uicer, 0x00000000);       /* disable all ints */
	mtdcr(uiccr, 0x00000000);       /* set all to be non-critical*/
	mtdcr(uicpr, 0xFFFFFFF0);       /* set int polarities */
	mtdcr(uictr, 0x10000000);       /* set int trigger levels */
	mtdcr(uicvcr, 0x00000001);      /* set vect base=0,INT0 highest priority*/
	mtdcr(uicsr, 0xFFFFFFFF);       /* clear all ints */
#endif

#if 1 /* test-only */
	/*
	 * EBC Configuration Register: set ready timeout to 512 ebc-clks -> ca. 15 us
	 */
	mtebc (epcr, 0xa8400000); /* ebc always driven */
#endif

	return 0;
}


int misc_init_f (void)
{
	return 0;  /* dummy implementation */
}


int misc_init_r (void)
{
#if (CONFIG_COMMANDS & CFG_CMD_NAND)
	/*
	 * Set NAND-FLASH GPIO signals to default
	 */
	out32(GPIO0_OR, in32(GPIO0_OR) & ~(CFG_NAND_CLE | CFG_NAND_ALE));
	out32(GPIO0_OR, in32(GPIO0_OR) | CFG_NAND_CE);
#endif

	return (0);
}


/*
 * Check Board Identity:
 */
int checkboard (void)
{
	char str[64];
	int i = getenv_r ("serial#", str, sizeof(str));

	puts ("Board: ");

	if (i == -1) {
		puts ("### No HW ID - assuming G2000");
	} else {
		puts(str);
	}

	putc ('\n');

	return 0;
}


/* -------------------------------------------------------------------------
  G2000 rev B is an embeded design. we don't read for spd of this version.
  Doing static SDRAM controller configuration in the following section.
   ------------------------------------------------------------------------- */

long int init_sdram_static_settings(void)
{
#define mtsdram0(reg, data)  mtdcr(memcfga,reg);mtdcr(memcfgd,data)
	/* disable memcontroller so updates work */
	mtsdram0( mem_mcopt1, MEM_MCOPT1_INIT_VAL );
	mtsdram0( mem_rtr   , MEM_RTR_INIT_VAL   );
	mtsdram0( mem_pmit  , MEM_PMIT_INIT_VAL  );
	mtsdram0( mem_mb0cf , MEM_MB0CF_INIT_VAL );
	mtsdram0( mem_mb1cf , MEM_MB1CF_INIT_VAL );
	mtsdram0( mem_sdtr1 , MEM_SDTR1_INIT_VAL );

	/* SDRAM have a power on delay,  500 micro should do */
	udelay(500);
	mtsdram0( mem_mcopt1, MEM_MCOPT1_INIT_VAL|SDRAM0_CFG_ENABLE );

	return (CFG_SDRAM_SIZE); /* CFG_SDRAM_SIZE is in G2000.h */
 }


long int initdram (int board_type)
{
	long int ret;

/* flzt, we can still turn this on in the future */
/* #ifdef CONFIG_SPD_EEPROM
	ret = spd_sdram ();
#else
	ret = init_sdram_static_settings();
#endif
*/

	ret = init_sdram_static_settings();

	return ret;
}


#if 1 /* test-only */
void sdram_init(void)
{
	init_sdram_static_settings();
}
#endif


#if 0 /* test-only */
long int initdram (int board_type)
{
	unsigned long val;

	mtdcr(memcfga, mem_mb0cf);
	val = mfdcr(memcfgd);

#if 0
	printf("\nmb0cf=%x\n", val); /* test-only */
	printf("strap=%x\n", mfdcr(strap)); /* test-only */
#endif

	return (4*1024*1024 << ((val & 0x000e0000) >> 17));
}
#endif


int testdram (void)
{
	/* TODO: XXX XXX XXX */
	printf ("test: 16 MB - ok\n");

	return (0);
}


#if (CONFIG_COMMANDS & CFG_CMD_NAND)
#include <linux/mtd/nand_legacy.h>
extern struct nand_chip nand_dev_desc[CFG_MAX_NAND_DEVICE];

void nand_init(void)
{
	nand_probe(CFG_NAND_BASE);
	if (nand_dev_desc[0].ChipID != NAND_ChipID_UNKNOWN) {
		print_size(nand_dev_desc[0].totlen, "\n");
	}
}
#endif


#if 0 /* test-only !!! */
int do_dumpebc(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ulong ap, cr;

	printf("\nEBC registers for PPC405GP:\n");
	mfebc(pb0ap, ap); mfebc(pb0cr, cr);
	printf("0: AP=%08lx CP=%08lx\n", ap, cr);
	mfebc(pb1ap, ap); mfebc(pb1cr, cr);
	printf("1: AP=%08lx CP=%08lx\n", ap, cr);
	mfebc(pb2ap, ap); mfebc(pb2cr, cr);
	printf("2: AP=%08lx CP=%08lx\n", ap, cr);
	mfebc(pb3ap, ap); mfebc(pb3cr, cr);
	printf("3: AP=%08lx CP=%08lx\n", ap, cr);
	mfebc(pb4ap, ap); mfebc(pb4cr, cr);
	printf("4: AP=%08lx CP=%08lx\n", ap, cr);
	printf("\n");

	return 0;
}
U_BOOT_CMD(
	dumpebc,	1,	1,	do_dumpebc,
	"dumpebc - Dump all EBC registers\n",
	NULL
);


int do_dumpdcr(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int i;

	printf("\nDevice Configuration Registers (DCR's) for PPC405GP:");
	for (i=0; i<=0x1e0; i++) {
		if (!(i % 0x8)) {
			printf("\n%04x ", i);
		}
		printf("%08lx ", get_dcr(i));
	}
	printf("\n");

	return 0;
}
U_BOOT_CMD(
	dumpdcr,	1,	1,	do_dumpdcr,
	"dumpdcr - Dump all DCR registers\n",
	NULL
);


int do_dumpspr(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	printf("\nSpecial Purpose Registers (SPR's) for PPC405GP:");
	printf("\n%04x %08x ", 947, mfspr(947));
	printf("\n%04x %08x ", 9, mfspr(9));
	printf("\n%04x %08x ", 1014, mfspr(1014));
	printf("\n%04x %08x ", 1015, mfspr(1015));
	printf("\n%04x %08x ", 1010, mfspr(1010));
	printf("\n%04x %08x ", 957, mfspr(957));
	printf("\n%04x %08x ", 1008, mfspr(1008));
	printf("\n%04x %08x ", 1018, mfspr(1018));
	printf("\n%04x %08x ", 954, mfspr(954));
	printf("\n%04x %08x ", 950, mfspr(950));
	printf("\n%04x %08x ", 951, mfspr(951));
	printf("\n%04x %08x ", 981, mfspr(981));
	printf("\n%04x %08x ", 980, mfspr(980));
	printf("\n%04x %08x ", 982, mfspr(982));
	printf("\n%04x %08x ", 1012, mfspr(1012));
	printf("\n%04x %08x ", 1013, mfspr(1013));
	printf("\n%04x %08x ", 948, mfspr(948));
	printf("\n%04x %08x ", 949, mfspr(949));
	printf("\n%04x %08x ", 1019, mfspr(1019));
	printf("\n%04x %08x ", 979, mfspr(979));
	printf("\n%04x %08x ", 8, mfspr(8));
	printf("\n%04x %08x ", 945, mfspr(945));
	printf("\n%04x %08x ", 987, mfspr(987));
	printf("\n%04x %08x ", 287, mfspr(287));
	printf("\n%04x %08x ", 953, mfspr(953));
	printf("\n%04x %08x ", 955, mfspr(955));
	printf("\n%04x %08x ", 272, mfspr(272));
	printf("\n%04x %08x ", 273, mfspr(273));
	printf("\n%04x %08x ", 274, mfspr(274));
	printf("\n%04x %08x ", 275, mfspr(275));
	printf("\n%04x %08x ", 260, mfspr(260));
	printf("\n%04x %08x ", 276, mfspr(276));
	printf("\n%04x %08x ", 261, mfspr(261));
	printf("\n%04x %08x ", 277, mfspr(277));
	printf("\n%04x %08x ", 262, mfspr(262));
	printf("\n%04x %08x ", 278, mfspr(278));
	printf("\n%04x %08x ", 263, mfspr(263));
	printf("\n%04x %08x ", 279, mfspr(279));
	printf("\n%04x %08x ", 26, mfspr(26));
	printf("\n%04x %08x ", 27, mfspr(27));
	printf("\n%04x %08x ", 990, mfspr(990));
	printf("\n%04x %08x ", 991, mfspr(991));
	printf("\n%04x %08x ", 956, mfspr(956));
	printf("\n%04x %08x ", 284, mfspr(284));
	printf("\n%04x %08x ", 285, mfspr(285));
	printf("\n%04x %08x ", 986, mfspr(986));
	printf("\n%04x %08x ", 984, mfspr(984));
	printf("\n%04x %08x ", 256, mfspr(256));
	printf("\n%04x %08x ", 1, mfspr(1));
	printf("\n%04x %08x ", 944, mfspr(944));
	printf("\n");

	return 0;
}
U_BOOT_CMD(
	dumpspr,	1,	1,	do_dumpspr,
	"dumpspr - Dump all SPR registers\n",
	NULL
);
#endif
