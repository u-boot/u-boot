/*
 * (C) Copyright 2000-2007
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * CPU specific code
 *
 * written or collected and sometimes rewritten by
 * Magnus Damm <damm@bitsmart.com>
 *
 * minor modifications by
 * Wolfgang Denk <wd@denx.de>
 */

#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <asm/cache.h>
#include <asm/ppc4xx.h>
#include <netdev.h>

DECLARE_GLOBAL_DATA_PTR;

void board_reset(void);

/*
 * To provide an interface to detect CPU number for boards that support
 * more then one CPU, we implement the "weak" default functions here.
 *
 * Returns CPU number
 */
int __get_cpu_num(void)
{
	return NA_OR_UNKNOWN_CPU;
}
int get_cpu_num(void) __attribute__((weak, alias("__get_cpu_num")));

#if defined(CONFIG_PCI)
#if defined(CONFIG_405GP) || \
    defined(CONFIG_440EP) || defined(CONFIG_440GR) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX)

#define PCI_ASYNC

static int pci_async_enabled(void)
{
#if defined(CONFIG_405GP)
	return (mfdcr(CPC0_PSR) & PSR_PCI_ASYNC_EN);
#endif

#if defined(CONFIG_440EP) || defined(CONFIG_440GR) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX) || \
    defined(CONFIG_460EX) || defined(CONFIG_460GT)
	unsigned long val;

	mfsdr(SDR0_SDSTP1, val);
	return (val & SDR0_SDSTP1_PAME_MASK);
#endif
}
#endif
#endif /* CONFIG_PCI */

#if defined(CONFIG_PCI) && \
    !defined(CONFIG_405) && !defined(CONFIG_405EX)
int pci_arbiter_enabled(void)
{
#if defined(CONFIG_405GP)
	return (mfdcr(CPC0_PSR) & PSR_PCI_ARBIT_EN);
#endif

#if defined(CONFIG_405EP)
	return (mfdcr(CPC0_PCI) & CPC0_PCI_ARBIT_EN);
#endif

#if defined(CONFIG_440GP)
	return (mfdcr(CPC0_STRP1) & CPC0_STRP1_PAE_MASK);
#endif

#if defined(CONFIG_440GX) || defined(CONFIG_440SP) || defined(CONFIG_440SPE)
	unsigned long val;

	mfsdr(SDR0_XCR0, val);
	return (val & SDR0_XCR0_PAE_MASK);
#endif
#if defined(CONFIG_440EP) || defined(CONFIG_440GR) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX) || \
    defined(CONFIG_460EX) || defined(CONFIG_460GT)
	unsigned long val;

	mfsdr(SDR0_PCI0, val);
	return (val & SDR0_PCI0_PAE_MASK);
#endif
}
#endif

#if defined(CONFIG_405EP)
#define I2C_BOOTROM

static int i2c_bootrom_enabled(void)
{
#if defined(CONFIG_405EP)
	return (mfdcr(CPC0_BOOT) & CPC0_BOOT_SEP);
#else
	unsigned long val;

	mfsdr(SDR0_SDCS0, val);
	return (val & SDR0_SDCS_SDD);
#endif
}
#endif

#if defined(CONFIG_440GX)
#define SDR0_PINSTP_SHIFT	29
static char *bootstrap_str[] = {
	"EBC (16 bits)",
	"EBC (8 bits)",
	"EBC (32 bits)",
	"EBC (8 bits)",
	"PCI",
	"I2C (Addr 0x54)",
	"Reserved",
	"I2C (Addr 0x50)",
};
static char bootstrap_char[] = { 'A', 'B', 'C', 'B', 'D', 'E', 'x', 'F' };
#endif

#if defined(CONFIG_440SP) || defined(CONFIG_440SPE)
#define SDR0_PINSTP_SHIFT	30
static char *bootstrap_str[] = {
	"EBC (8 bits)",
	"PCI",
	"I2C (Addr 0x54)",
	"I2C (Addr 0x50)",
};
static char bootstrap_char[] = { 'A', 'B', 'C', 'D'};
#endif

#if defined(CONFIG_440EP) || defined(CONFIG_440GR)
#define SDR0_PINSTP_SHIFT	29
static char *bootstrap_str[] = {
	"EBC (8 bits)",
	"PCI",
	"NAND (8 bits)",
	"EBC (16 bits)",
	"EBC (16 bits)",
	"I2C (Addr 0x54)",
	"PCI",
	"I2C (Addr 0x52)",
};
static char bootstrap_char[] = { 'A', 'B', 'C', 'D', 'E', 'G', 'F', 'H' };
#endif

#if defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
#define SDR0_PINSTP_SHIFT	29
static char *bootstrap_str[] = {
	"EBC (8 bits)",
	"EBC (16 bits)",
	"EBC (16 bits)",
	"NAND (8 bits)",
	"PCI",
	"I2C (Addr 0x54)",
	"PCI",
	"I2C (Addr 0x52)",
};
static char bootstrap_char[] = { 'A', 'B', 'C', 'D', 'E', 'G', 'F', 'H' };
#endif

#if defined(CONFIG_460EX) || defined(CONFIG_460GT)
#define SDR0_PINSTP_SHIFT	29
static char *bootstrap_str[] = {
	"EBC (8 bits)",
	"EBC (16 bits)",
	"PCI",
	"PCI",
	"EBC (16 bits)",
	"NAND (8 bits)",
	"I2C (Addr 0x54)",	/* A8 */
	"I2C (Addr 0x52)",	/* A4 */
};
static char bootstrap_char[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H' };
#endif

#if defined(CONFIG_460SX)
#define SDR0_PINSTP_SHIFT	29
static char *bootstrap_str[] = {
	"EBC (8 bits)",
	"EBC (16 bits)",
	"EBC (32 bits)",
	"NAND (8 bits)",
	"I2C (Addr 0x54)",      /* A8 */
	"I2C (Addr 0x52)",      /* A4 */
};
static char bootstrap_char[] = { 'A', 'B', 'C', 'D', 'E', 'G' };
#endif

#if defined(CONFIG_405EZ)
#define SDR0_PINSTP_SHIFT	28
static char *bootstrap_str[] = {
	"EBC (8 bits)",
	"SPI (fast)",
	"NAND (512 page, 4 addr cycle)",
	"I2C (Addr 0x50)",
	"EBC (32 bits)",
	"I2C (Addr 0x50)",
	"NAND (2K page, 5 addr cycle)",
	"I2C (Addr 0x50)",
	"EBC (16 bits)",
	"Reserved",
	"NAND (2K page, 4 addr cycle)",
	"I2C (Addr 0x50)",
	"NAND (512 page, 3 addr cycle)",
	"I2C (Addr 0x50)",
	"SPI (slow)",
	"I2C (Addr 0x50)",
};
static char bootstrap_char[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', \
				 'I', 'x', 'K', 'L', 'M', 'N', 'O', 'P' };
#endif

#if defined(CONFIG_405EX)
#define SDR0_PINSTP_SHIFT	29
static char *bootstrap_str[] = {
	"EBC (8 bits)",
	"EBC (16 bits)",
	"EBC (16 bits)",
	"NAND (8 bits)",
	"NAND (8 bits)",
	"I2C (Addr 0x54)",
	"EBC (8 bits)",
	"I2C (Addr 0x52)",
};
static char bootstrap_char[] = { 'A', 'B', 'C', 'D', 'E', 'G', 'F', 'H' };
#endif

#if defined(SDR0_PINSTP_SHIFT)
static int bootstrap_option(void)
{
	unsigned long val;

	mfsdr(SDR0_PINSTP, val);
	return ((val & 0xf0000000) >> SDR0_PINSTP_SHIFT);
}
#endif /* SDR0_PINSTP_SHIFT */


#if defined(CONFIG_440GP)
static int do_chip_reset (unsigned long sys0, unsigned long sys1)
{
	/* Changes to CPC0_SYS0 and CPC0_SYS1 require chip
	 * reset.
	 */
	mtdcr (CPC0_CR0, mfdcr (CPC0_CR0) | 0x80000000);	/* Set SWE */
	mtdcr (CPC0_SYS0, sys0);
	mtdcr (CPC0_SYS1, sys1);
	mtdcr (CPC0_CR0, mfdcr (CPC0_CR0) & ~0x80000000);	/* Clr SWE */
	mtspr (SPRN_DBCR0, 0x20000000);	/* Reset the chip */

	return 1;
}
#endif /* CONFIG_440GP */


int checkcpu (void)
{
#if !defined(CONFIG_405)	/* not used on Xilinx 405 FPGA implementations */
	uint pvr = get_pvr();
	ulong clock = gd->cpu_clk;
	char buf[32];
#if defined(CONFIG_460EX) || defined(CONFIG_460GT)
	u32 reg;
#endif

	char addstr[64] = "";
	sys_info_t sys_info;
	int cpu_num;

	cpu_num = get_cpu_num();
	if (cpu_num >= 0)
		printf("CPU%d:  ", cpu_num);
	else
		puts("CPU:   ");

	get_sys_info(&sys_info);

#if defined(CONFIG_XILINX_440)
	puts("IBM PowerPC ");
#else
	puts("AMCC PowerPC ");
#endif

	switch (pvr) {

#if !defined(CONFIG_440)
	case PVR_405GP_RB:
		puts("405GP Rev. B");
		break;

	case PVR_405GP_RC:
		puts("405GP Rev. C");
		break;

	case PVR_405GP_RD:
		puts("405GP Rev. D");
		break;

	case PVR_405GP_RE:
		puts("405GP Rev. E");
		break;

	case PVR_405GPR_RB:
		puts("405GPr Rev. B");
		break;

	case PVR_405EP_RB:
		puts("405EP Rev. B");
		break;

	case PVR_405EZ_RA:
		puts("405EZ Rev. A");
		break;

	case PVR_405EX1_RA:
		puts("405EX Rev. A");
		strcpy(addstr, "Security support");
		break;

	case PVR_405EXR2_RA:
		puts("405EXr Rev. A");
		strcpy(addstr, "No Security support");
		break;

	case PVR_405EX1_RC:
		puts("405EX Rev. C");
		strcpy(addstr, "Security support");
		break;

	case PVR_405EX2_RC:
		puts("405EX Rev. C");
		strcpy(addstr, "No Security support");
		break;

	case PVR_405EXR1_RC:
		puts("405EXr Rev. C");
		strcpy(addstr, "Security support");
		break;

	case PVR_405EXR2_RC:
		puts("405EXr Rev. C");
		strcpy(addstr, "No Security support");
		break;

	case PVR_405EX1_RD:
		puts("405EX Rev. D");
		strcpy(addstr, "Security support");
		break;

	case PVR_405EX2_RD:
		puts("405EX Rev. D");
		strcpy(addstr, "No Security support");
		break;

	case PVR_405EXR1_RD:
		puts("405EXr Rev. D");
		strcpy(addstr, "Security support");
		break;

	case PVR_405EXR2_RD:
		puts("405EXr Rev. D");
		strcpy(addstr, "No Security support");
		break;

#else /* CONFIG_440 */

#if defined(CONFIG_440GP)
	case PVR_440GP_RB:
		puts("440GP Rev. B");
		/* See errata 1.12: CHIP_4 */
		if ((mfdcr(CPC0_SYS0) != mfdcr(CPC0_STRP0)) ||
		    (mfdcr(CPC0_SYS1) != mfdcr(CPC0_STRP1)) ){
			puts (  "\n\t CPC0_SYSx DCRs corrupted. "
				"Resetting chip ...\n");
			udelay( 1000 * 1000 ); /* Give time for serial buf to clear */
			do_chip_reset ( mfdcr(CPC0_STRP0),
					mfdcr(CPC0_STRP1) );
		}
		break;

	case PVR_440GP_RC:
		puts("440GP Rev. C");
		break;
#endif /* CONFIG_440GP */

	case PVR_440GX_RA:
		puts("440GX Rev. A");
		break;

	case PVR_440GX_RB:
		puts("440GX Rev. B");
		break;

	case PVR_440GX_RC:
		puts("440GX Rev. C");
		break;

	case PVR_440GX_RF:
		puts("440GX Rev. F");
		break;

	case PVR_440EP_RA:
		puts("440EP Rev. A");
		break;

#ifdef CONFIG_440EP
	case PVR_440EP_RB: /* 440EP rev B and 440GR rev A have same PVR */
		puts("440EP Rev. B");
		break;

	case PVR_440EP_RC: /* 440EP rev C and 440GR rev B have same PVR */
		puts("440EP Rev. C");
		break;
#endif /*  CONFIG_440EP */

#ifdef CONFIG_440GR
	case PVR_440GR_RA: /* 440EP rev B and 440GR rev A have same PVR */
		puts("440GR Rev. A");
		break;

	case PVR_440GR_RB: /* 440EP rev C and 440GR rev B have same PVR */
		puts("440GR Rev. B");
		break;
#endif /* CONFIG_440GR */

#ifdef CONFIG_440EPX
	case PVR_440EPX1_RA: /* 440EPx rev A and 440GRx rev A have same PVR */
		puts("440EPx Rev. A");
		strcpy(addstr, "Security/Kasumi support");
		break;

	case PVR_440EPX2_RA: /* 440EPx rev A and 440GRx rev A have same PVR */
		puts("440EPx Rev. A");
		strcpy(addstr, "No Security/Kasumi support");
		break;
#endif /* CONFIG_440EPX */

#ifdef CONFIG_440GRX
	case PVR_440GRX1_RA: /* 440EPx rev A and 440GRx rev A have same PVR */
		puts("440GRx Rev. A");
		strcpy(addstr, "Security/Kasumi support");
		break;

	case PVR_440GRX2_RA: /* 440EPx rev A and 440GRx rev A have same PVR */
		puts("440GRx Rev. A");
		strcpy(addstr, "No Security/Kasumi support");
		break;
#endif /* CONFIG_440GRX */

	case PVR_440SP_6_RAB:
		puts("440SP Rev. A/B");
		strcpy(addstr, "RAID 6 support");
		break;

	case PVR_440SP_RAB:
		puts("440SP Rev. A/B");
		strcpy(addstr, "No RAID 6 support");
		break;

	case PVR_440SP_6_RC:
		puts("440SP Rev. C");
		strcpy(addstr, "RAID 6 support");
		break;

	case PVR_440SP_RC:
		puts("440SP Rev. C");
		strcpy(addstr, "No RAID 6 support");
		break;

	case PVR_440SPe_6_RA:
		puts("440SPe Rev. A");
		strcpy(addstr, "RAID 6 support");
		break;

	case PVR_440SPe_RA:
		puts("440SPe Rev. A");
		strcpy(addstr, "No RAID 6 support");
		break;

	case PVR_440SPe_6_RB:
		puts("440SPe Rev. B");
		strcpy(addstr, "RAID 6 support");
		break;

	case PVR_440SPe_RB:
		puts("440SPe Rev. B");
		strcpy(addstr, "No RAID 6 support");
		break;

#if defined(CONFIG_460EX) || defined(CONFIG_460GT)
	case PVR_460EX_RA:
		puts("460EX Rev. A");
		strcpy(addstr, "No Security/Kasumi support");
		break;

	case PVR_460EX_SE_RA:
		puts("460EX Rev. A");
		strcpy(addstr, "Security/Kasumi support");
		break;

	case PVR_460EX_RB:
		puts("460EX Rev. B");
		mfsdr(SDR0_ECID3, reg);
		if (reg & 0x00100000)
			strcpy(addstr, "No Security/Kasumi support");
		else
			strcpy(addstr, "Security/Kasumi support");
		break;

	case PVR_460GT_RA:
		puts("460GT Rev. A");
		strcpy(addstr, "No Security/Kasumi support");
		break;

	case PVR_460GT_SE_RA:
		puts("460GT Rev. A");
		strcpy(addstr, "Security/Kasumi support");
		break;

	case PVR_460GT_RB:
		puts("460GT Rev. B");
		mfsdr(SDR0_ECID3, reg);
		if (reg & 0x00100000)
			strcpy(addstr, "No Security/Kasumi support");
		else
			strcpy(addstr, "Security/Kasumi support");
		break;
#endif

	case PVR_460SX_RA:
		puts("460SX Rev. A");
		strcpy(addstr, "Security support");
		break;

	case PVR_460SX_RA_V1:
		puts("460SX Rev. A");
		strcpy(addstr, "No Security support");
		break;

	case PVR_460GX_RA:
		puts("460GX Rev. A");
		strcpy(addstr, "Security support");
		break;

	case PVR_460GX_RA_V1:
		puts("460GX Rev. A");
		strcpy(addstr, "No Security support");
		break;

	case PVR_APM821XX_RA:
		puts("APM821XX Rev. A");
		strcpy(addstr, "Security support");
		break;

	case PVR_VIRTEX5:
		puts("440x5 VIRTEX5");
		break;
#endif /* CONFIG_440 */

	default:
		printf (" UNKNOWN (PVR=%08x)", pvr);
		break;
	}

	printf (" at %s MHz (PLB=%lu OPB=%lu EBC=%lu",
		strmhz(buf, clock),
		sys_info.freqPLB / 1000000,
		get_OPB_freq() / 1000000,
		sys_info.freqEBC / 1000000);
#if defined(CONFIG_PCI) && \
	(defined(CONFIG_440EP) || defined(CONFIG_440EPX) || \
	 defined(CONFIG_440GR) || defined(CONFIG_440GRX))
	printf(" PCI=%lu MHz", sys_info.freqPCI / 1000000);
#endif
	printf(")\n");

	if (addstr[0] != 0)
		printf("       %s\n", addstr);

#if defined(I2C_BOOTROM)
	printf ("       I2C boot EEPROM %sabled\n", i2c_bootrom_enabled() ? "en" : "dis");
#endif	/* I2C_BOOTROM */
#if defined(SDR0_PINSTP_SHIFT)
	printf ("       Bootstrap Option %c - ", bootstrap_char[bootstrap_option()]);
	printf ("Boot ROM Location %s", bootstrap_str[bootstrap_option()]);
	putc('\n');
#endif	/* SDR0_PINSTP_SHIFT */

#if defined(CONFIG_PCI) && !defined(CONFIG_405EX)
	printf ("       Internal PCI arbiter %sabled", pci_arbiter_enabled() ? "en" : "dis");
#endif

#if defined(CONFIG_PCI) && defined(PCI_ASYNC)
	if (pci_async_enabled()) {
		printf (", PCI async ext clock used");
	} else {
		printf (", PCI sync clock at %lu MHz",
		       sys_info.freqPLB / sys_info.pllPciDiv / 1000000);
	}
#endif

#if defined(CONFIG_PCI) && !defined(CONFIG_405EX)
	putc('\n');
#endif

#if defined(CONFIG_405EP) || defined(CONFIG_405EZ) || defined(CONFIG_405EX)
	printf("       16 KiB I-Cache 16 KiB D-Cache");
#elif defined(CONFIG_440)
	printf("       32 KiB I-Cache 32 KiB D-Cache");
#else
	printf("       16 KiB I-Cache %d KiB D-Cache",
	       ((pvr | 0x00000001) == PVR_405GPR_RB) ? 16 : 8);
#endif

#endif /* !defined(CONFIG_405) */

	putc ('\n');

	return 0;
}

int ppc440spe_revB() {
	unsigned int pvr;

	pvr = get_pvr();
	if ((pvr == PVR_440SPe_6_RB) || (pvr == PVR_440SPe_RB))
		return 1;
	else
		return 0;
}

/* ------------------------------------------------------------------------- */

int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
#if defined(CONFIG_BOARD_RESET)
	board_reset();
#else
#if defined(CONFIG_SYS_4xx_RESET_TYPE)
	mtspr(SPRN_DBCR0, CONFIG_SYS_4xx_RESET_TYPE << 28);
#else
	/*
	 * Initiate system reset in debug control register DBCR
	 */
	mtspr(SPRN_DBCR0, 0x30000000);
#endif /* defined(CONFIG_SYS_4xx_RESET_TYPE) */
#endif /* defined(CONFIG_BOARD_RESET) */

	return 1;
}


/*
 * Get timebase clock frequency
 */
unsigned long get_tbclk (void)
{
	sys_info_t  sys_info;

	get_sys_info(&sys_info);
	return (sys_info.freqProcessor);
}


#if defined(CONFIG_WATCHDOG)
void watchdog_reset(void)
{
	int re_enable = disable_interrupts();
	reset_4xx_watchdog();
	if (re_enable) enable_interrupts();
}

void reset_4xx_watchdog(void)
{
	/*
	 * Clear TSR(WIS) bit
	 */
	mtspr(SPRN_TSR, 0x40000000);
}
#endif	/* CONFIG_WATCHDOG */

/*
 * Initializes on-chip ethernet controllers.
 * to override, implement board_eth_init()
 */
int cpu_eth_init(bd_t *bis)
{
#if defined(CONFIG_PPC4xx_EMAC)
	ppc_4xx_eth_initialize(bis);
#endif
	return 0;
}
