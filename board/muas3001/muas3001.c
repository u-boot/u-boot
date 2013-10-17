/*
 * (C) Copyright 2008
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mpc8260.h>
#include <ioports.h>

#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)
#include <libfdt.h>
#endif

/*
 * I/O Port configuration table
 *
 * if conf is 1, then that port pin will be configured at boot time
 * according to the five values podr/pdir/ppar/psor/pdat for that entry
 */
const iop_conf_t iop_conf_tab[4][32] = {

    /* Port A */
    {	/*	      conf      ppar psor pdir podr pdat */
	/* PA31 */ { 0,          0,   0,   0,   0,   0 }, /* PA31            */
	/* PA30 */ { 0,          0,   0,   0,   0,   0 }, /* PA30            */
	/* PA29 */ { 1,          1,   1,   1,   0,   0 }, /* FCC1 TXER */
	/* PA28 */ { 1,          1,   1,   1,   0,   0 }, /* FCC1 TXEN */
	/* PA27 */ { 1,          1,   1,   0,   0,   0 }, /* FCC1 RXDV */
	/* PA26 */ { 1,          1,   1,   0,   0,   0 }, /* FCC1 RXER */
	/* PA25 */ { 1,          0,   0,   1,   0,   0 }, /* ETH_PWRDWN      */
	/* PA24 */ { 1,          0,   0,   1,   0,   1 }, /* ETH_RESET       */
	/* PA23 */ { 0,          0,   0,   0,   0,   0 }, /* PA23            */
	/* PA22 */ { 0,          0,   0,   0,   0,   0 }, /* PA22            */
	/* PA21 */ { 1,          1,   0,   1,   0,   0 }, /* FCC1 TXD3 */
	/* PA20 */ { 1,          1,   0,   1,   0,   0 }, /* FCC1 TXD2 */
	/* PA19 */ { 1,          1,   0,   1,   0,   0 }, /* FCC1 TXD1 */
	/* PA18 */ { 1,          1,   0,   1,   0,   0 }, /* FCC1 TXD0 */
	/* PA17 */ { 1,          1,   0,   0,   0,   0 }, /* FCC1 RXD0 */
	/* PA16 */ { 1,          1,   0,   0,   0,   0 }, /* FCC1 RXD1 */
	/* PA15 */ { 1,          1,   0,   0,   0,   0 }, /* FCC1 RXD2 */
	/* PA14 */ { 1,          1,   0,   0,   0,   0 }, /* FCC1 RXD3 */
	/* PA13 */ { 0,          0,   0,   0,   0,   0 }, /* PA13            */
	/* PA12 */ { 1,          0,   0,   1,   0,   0 }, /* ETH_SLEEP       */
	/* PA11 */ { 0,          0,   0,   0,   0,   0 }, /* PA11            */
	/* PA10 */ { 1,          0,   0,   1,   0,   0 }, /* MDIO            */
	/* PA9  */ { 1,          0,   0,   1,   0,   0 }, /* MDC             */
	/* PA8  */ { 1,          1,   0,   0,   0,   0 }, /* SMC2 RxD        */
	/* PA7  */ { 0,          0,   0,   0,   0,   0 }, /* PA7             */
	/* PA6  */ { 0,          0,   0,   0,   0,   0 }, /* PA6             */
	/* PA5  */ { 0,          0,   0,   0,   0,   0 }, /* PA5             */
	/* PA4  */ { 0,          0,   0,   0,   0,   0 }, /* PA4             */
	/* PA3  */ { 0,          0,   0,   0,   0,   0 }, /* PA3             */
	/* PA2  */ { 0,          0,   0,   0,   0,   0 }, /* PA2             */
	/* PA1  */ { 0,          0,   0,   0,   0,   0 }, /* PA1             */
	/* PA0  */ { 0,          0,   0,   0,   0,   0 }  /* PA0             */
    },

    /* Port B */
    {   /*	      conf      ppar psor pdir podr pdat */
	/* PB31 */ { 0,          0,   0,   0,   0,   0 }, /* PB31            */
	/* PB30 */ { 0,          0,   0,   0,   0,   0 }, /* PB30            */
	/* PB29 */ { 0,          0,   0,   0,   0,   0 }, /* PB29            */
	/* PB28 */ { 1,          1,   1,   1,   0,   0 }, /* SCC1 TxD        */
	/* PB27 */ { 0,          0,   0,   0,   0,   0 }, /* PB27            */
	/* PB26 */ { 0,          0,   0,   0,   0,   0 }, /* PB26            */
	/* PB25 */ { 0,          0,   0,   0,   0,   0 }, /* PB25            */
	/* PB24 */ { 0,          0,   0,   0,   0,   0 }, /* PB24            */
	/* PB23 */ { 0,          0,   0,   0,   0,   0 }, /* PB23            */
	/* PB22 */ { 0,          0,   0,   0,   0,   0 }, /* PB22            */
	/* PB21 */ { 0,          0,   0,   0,   0,   0 }, /* PB21            */
	/* PB20 */ { 0,          0,   0,   0,   0,   0 }, /* PB20            */
	/* PB19 */ { 0,          0,   0,   0,   0,   0 }, /* PB19            */
	/* PB18 */ { 0,          0,   0,   0,   0,   0 }, /* PB18            */
	/* PB17 */ { 0,          0,   0,   0,   0,   0 }, /* non-existent    */
	/* PB16 */ { 0,          0,   0,   0,   0,   0 }, /* non-existent    */
	/* PB15 */ { 1,          1,   0,   0,   0,   0 }, /* SCC2 RxD        */
	/* PB14 */ { 1,          1,   0,   0,   0,   0 }, /* SCC3 RxD        */
	/* PB13 */ { 0,          0,   0,   0,   0,   0 }, /* non-existent    */
	/* PB12 */ { 1,          1,   1,   1,   0,   0 }, /* SCC2 TxD        */
	/* PB11 */ { 0,          0,   0,   0,   0,   0 }, /* non-existent    */
	/* PB10 */ { 0,          0,   0,   0,   0,   0 }, /* non-existent    */
	/* PB9  */ { 0,          0,   0,   0,   0,   0 }, /* non-existent    */
	/* PB8  */ { 1,          1,   1,   1,   0,   0 }, /* SCC3 TxD        */
	/* PB7  */ { 0,          0,   0,   0,   0,   0 }, /* non-existent    */
	/* PB6  */ { 0,          0,   0,   0,   0,   0 }, /* non-existent    */
	/* PB5  */ { 0,          0,   0,   0,   0,   0 }, /* non-existent    */
	/* PB4  */ { 0,          0,   0,   0,   0,   0 }, /* non-existent    */
	/* PB3  */ { 0,          0,   0,   0,   0,   0 }, /* non-existent    */
	/* PB2  */ { 0,          0,   0,   0,   0,   0 }, /* non-existent    */
	/* PB1  */ { 0,          0,   0,   0,   0,   0 }, /* non-existent    */
	/* PB0  */ { 0,          0,   0,   0,   0,   0 }  /* non-existent    */
    },

    /* Port C */
    {   /*	      conf      ppar psor pdir podr pdat */
	/* PC31 */ { 0,          0,   0,   0,   0,   0 }, /* PC31            */
	/* PC30 */ { 1,          1,   1,   1,   0,   0 }, /* Timer1 OUT      */
	/* PC29 */ { 0,          0,   0,   0,   0,   0 }, /* PC29            */
	/* PC28 */ { 0,          0,   0,   0,   0,   0 }, /* PC28            */
	/* PC27 */ { 0,          0,   0,   0,   0,   0 }, /* PC27            */
	/* PC26 */ { 0,          0,   0,   0,   0,   0 }, /* PC26            */
	/* PC25 */ { 0,          0,   0,   0,   0,   0 }, /* PC25            */
	/* PC24 */ { 0,          0,   0,   0,   0,   0 }, /* PC24            */
	/* PC23 */ { 0,          0,   0,   0,   0,   0 }, /* PC23            */
	/* PC22 */ { 0,          0,   0,   0,   0,   0 }, /* PC22            */
	/* PC21 */ { 1,          1,   0,   0,   0,   0 }, /* FCC RxCLK 11    */
	/* PC20 */ { 1,          1,   0,   0,   0,   0 }, /* FCC TxCLK 12    */
	/* PC19 */ { 0,          0,   0,   0,   0,   0 }, /* PC19            */
	/* PC18 */ { 0,          0,   0,   0,   0,   0 }, /* PC18            */
	/* PC17 */ { 0,          0,   0,   0,   0,   0 }, /* PC17            */
	/* PC16 */ { 0,          0,   0,   0,   0,   0 }, /* PC16            */
	/* PC15 */ { 1,          1,   0,   1,   0,   0 }, /* SMC2 TxD        */
	/* PC14 */ { 0,          0,   0,   0,   0,   0 }, /* PC14            */
	/* PC13 */ { 0,          0,   0,   0,   0,   0 }, /* PC13            */
	/* PC12 */ { 1,          0,   0,   1,   0,   0 }, /* TX OUTPUT SLEW1 */
	/* PC11 */ { 1,          0,   0,   1,   0,   0 }, /* TX OUTPUT SLEW0 */
	/* PC10 */ { 0,          0,   0,   0,   0,   0 }, /* PC10            */
	/* PC9  */ { 1,          0,   0,   1,   0,   1 }, /* SPA_TX_EN       */
	/* PC8  */ { 0,          0,   0,   0,   0,   0 }, /* PC8             */
	/* PC7  */ { 0,          0,   0,   0,   0,   0 }, /* PC7             */
	/* PC6  */ { 0,          0,   0,   0,   0,   0 }, /* PC6             */
	/* PC5  */ { 0,          0,   0,   0,   0,   0 }, /* PC5             */
	/* PC4  */ { 0,          0,   0,   0,   0,   0 }, /* PC4             */
	/* PC3  */ { 0,          0,   0,   0,   0,   0 }, /* PC3             */
	/* PC2  */ { 0,          0,   0,   0,   0,   0 }, /* PC2             */
	/* PC1  */ { 0,          0,   0,   0,   0,   0 }, /* PC1             */
	/* PC0  */ { 0,          0,   0,   0,   0,   0 }, /* PC0             */
    },

    /* Port D */
    {   /*	      conf      ppar psor pdir podr pdat */
	/* PD31 */ { 1,          1,   0,   0,   0,   0 }, /* SCC1 RxD        */
	/* PD30 */ { 0,          0,   0,   0,   0,   0 }, /* PD30            */
	/* PD29 */ { 0,          0,   0,   0,   0,   0 }, /* PD29            */
	/* PD28 */ { 0,          0,   0,   0,   0,   0 }, /* PD28            */
	/* PD27 */ { 0,          0,   0,   0,   0,   0 }, /* PD27            */
	/* PD26 */ { 0,          0,   0,   0,   0,   0 }, /* PD26            */
	/* PD25 */ { 0,          0,   0,   0,   0,   0 }, /* PD25            */
	/* PD24 */ { 0,          0,   0,   0,   0,   0 }, /* PD24            */
	/* PD23 */ { 0,          0,   0,   0,   0,   0 }, /* PD23            */
	/* PD22 */ { 1,          1,   0,   0,   0,   0 }, /* SCC4: RXD       */
	/* PD21 */ { 1,          1,   0,   1,   0,   0 }, /* SCC4: TXD       */
	/* PD20 */ { 0,          0,   0,   0,   0,   0 }, /* PD18            */
	/* PD19 */ { 0,          0,   0,   0,   0,   0 }, /* PD19            */
	/* PD18 */ { 0,          0,   0,   0,   0,   0 }, /* PD18            */
	/* PD17 */ { 0,          0,   0,   0,   0,   0 }, /* PD17            */
	/* PD16 */ { 0,          0,   0,   0,   0,   0 }, /* PD16            */
#if defined(CONFIG_HARD_I2C)
	/* PD15 */ { 1,          1,   1,   0,   1,   0 }, /* I2C SDA         */
	/* PD14 */ { 1,          1,   1,   0,   1,   0 }, /* I2C SCL         */
#else
	/* PD15 */ { 1,          0,   0,   0,   1,   1 }, /* PD15            */
	/* PD14 */ { 1,          0,   0,   1,   1,   1 }, /* PD14            */
#endif
	/* PD13 */ { 0,          0,   0,   0,   0,   0 }, /* PD13            */
	/* PD12 */ { 0,          0,   0,   0,   0,   0 }, /* PD12            */
	/* PD11 */ { 0,          0,   0,   0,   0,   0 }, /* PD11            */
	/* PD10 */ { 0,          0,   0,   0,   0,   0 }, /* PD10            */
	/* PD9  */ { 1,          1,   0,   1,   0,   0 }, /* SMC1 TxD        */
	/* PD8  */ { 1,          1,   0,   0,   0,   0 }, /* SMC1 RxD        */
	/* PD7  */ { 0,          0,   0,   0,   0,   0 }, /* PD7             */
	/* PD6  */ { 0,          0,   0,   0,   0,   0 }, /* PD6             */
	/* PD5  */ { 0,          0,   0,   0,   0,   0 }, /* PD5             */
	/* PD4  */ { 0,          0,   0,   0,   0,   0 }, /* PD4             */
	/* PD3  */ { 0,          0,   0,   0,   0,   0 }, /* non-existent    */
	/* PD2  */ { 0,          0,   0,   0,   0,   0 }, /* non-existent    */
	/* PD1  */ { 0,          0,   0,   0,   0,   0 }, /* non-existent    */
	/* PD0  */ { 0,          0,   0,   0,   0,   0 }  /* non-existent    */
    }
};

/* Try SDRAM initialization with P/LSDMR=sdmr and ORx=orx
 *
 * This routine performs standard 8260 initialization sequence
 * and calculates the available memory size. It may be called
 * several times to try different SDRAM configurations on both
 * 60x and local buses.
 */
static long int try_init (volatile memctl8260_t * memctl, ulong sdmr,
						  ulong orx, volatile uchar * base)
{
	volatile uchar c = 0xff;
	volatile uint *sdmr_ptr;
	volatile uint *orx_ptr;
	ulong maxsize, size;
	int i;

	/* We must be able to test a location outsize the maximum legal size
	 * to find out THAT we are outside; but this address still has to be
	 * mapped by the controller. That means, that the initial mapping has
	 * to be (at least) twice as large as the maximum expected size.
	 */
	maxsize = (1 + (~orx | 0x7fff))/* / 2*/;

	sdmr_ptr = &memctl->memc_psdmr;
	orx_ptr = &memctl->memc_or1;

	*orx_ptr = orx;

	/*
	 * Quote from 8260 UM (10.4.2 SDRAM Power-On Initialization, 10-35):
	 *
	 * "At system reset, initialization software must set up the
	 *  programmable parameters in the memory controller banks registers
	 *  (ORx, BRx, P/LSDMR). After all memory parameters are configured,
	 *  system software should execute the following initialization sequence
	 *  for each SDRAM device.
	 *
	 *  1. Issue a PRECHARGE-ALL-BANKS command
	 *  2. Issue eight CBR REFRESH commands
	 *  3. Issue a MODE-SET command to initialize the mode register
	 *
	 *  The initial commands are executed by setting P/LSDMR[OP] and
	 *  accessing the SDRAM with a single-byte transaction."
	 *
	 * The appropriate BRx/ORx registers have already been set when we
	 * get here. The SDRAM can be accessed at the address CONFIG_SYS_SDRAM_BASE.
	 */

	*sdmr_ptr = sdmr | PSDMR_OP_PREA;
	*base = c;

	*sdmr_ptr = sdmr | PSDMR_OP_CBRR;
	for (i = 0; i < 8; i++)
		*base = c;

	*sdmr_ptr = sdmr | PSDMR_OP_MRW;
	*(base + CONFIG_SYS_MRS_OFFS) = c;	/* setting MR on address lines */

	*sdmr_ptr = sdmr | PSDMR_OP_NORM | PSDMR_RFEN;
	*base = c;

	size = get_ram_size ((long *)base, maxsize);
	*orx_ptr = orx | ~(size - 1);

	return (size);
}

phys_size_t initdram (int board_type)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8260_t *memctl = &immap->im_memctl;
	long psize;
#ifndef CONFIG_SYS_RAMBOOT
	long sizelittle, sizebig;
#endif

	memctl->memc_psrt = CONFIG_SYS_PSRT;
	memctl->memc_mptpr = CONFIG_SYS_MPTPR;

#ifndef CONFIG_SYS_RAMBOOT
	/* 60x SDRAM setup:
	 */
	sizelittle = try_init (memctl, CONFIG_SYS_PSDMR_LITTLE, CONFIG_SYS_OR1_LITTLE,
						  (uchar *) CONFIG_SYS_SDRAM_BASE);
	sizebig = try_init (memctl, CONFIG_SYS_PSDMR_BIG, CONFIG_SYS_OR1_BIG,
						  (uchar *) CONFIG_SYS_SDRAM_BASE);
	if (sizelittle < sizebig) {
		psize = sizebig;
	} else {
		psize = try_init (memctl, CONFIG_SYS_PSDMR_LITTLE, CONFIG_SYS_OR1_LITTLE,
						  (uchar *) CONFIG_SYS_SDRAM_BASE);
	}
#endif /* CONFIG_SYS_RAMBOOT */

	icache_enable ();

	return (psize);
}

int checkboard (void)
{
	puts ("Board: MUAS3001\n");

	return 0;
}

/*
 * Early board initalization.
 */
int board_early_init_r (void)
{
	return 0;
}

#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)
/*
 * update "memory" property in the blob
 */
void ft_blob_update (void *blob, bd_t *bd)
{
	int ret, nodeoffset = 0;
	ulong flash_data[4] = {0};
	ulong	speed = 0;

	/* update Flash addr, size */
	flash_data[2] = cpu_to_be32 (CONFIG_SYS_FLASH_BASE);
	flash_data[3] = cpu_to_be32 (CONFIG_SYS_FLASH_SIZE);
	nodeoffset = fdt_path_offset (blob, "/localbus");
	if (nodeoffset >= 0) {
		ret = fdt_setprop (blob, nodeoffset, "ranges", flash_data,
					sizeof (flash_data));
	if (ret < 0)
		printf ("ft_blob_update): cannot set /localbus/ranges "
			"property err:%s\n", fdt_strerror(ret));
	} else {
		/* memory node is required in dts */
		printf ("ft_blob_update(): cannot find /localbus node "
			"err:%s\n", fdt_strerror (nodeoffset));
	}

	/* baudrate */
	nodeoffset = fdt_path_offset (blob, "/soc/cpm/serial");
	if (nodeoffset >= 0) {
		speed = cpu_to_be32 (bd->bi_baudrate);
		ret = fdt_setprop (blob, nodeoffset, "current-speed", &speed,
					sizeof (unsigned long));
	if (ret < 0)
		printf ("ft_blob_update): cannot set /soc/cpm/serial/current-speed "
			"property err:%s\n", fdt_strerror (ret));
	} else {
		/* baudrate is required in dts */
		printf ("ft_blob_update(): cannot find /soc/cpm/smc2/current-speed node "
			"err:%s\n", fdt_strerror (nodeoffset));
	}
}

void ft_board_setup (void *blob, bd_t *bd)
{
	ft_cpu_setup (blob, bd);
	ft_blob_update (blob, bd);
}
#endif /* defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT) */
