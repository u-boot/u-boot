/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Yoo. Jonghoon, IPone, yooth@ipone.co.kr
 * U-Boot port on RPXlite board
 *
 * DRAM related UPMA register values are modified.
 * See RPXLite engineering note : 50MHz/60ns - UPM RAM WORDS
 */

#include <common.h>
#include "mpc8xx.h"

/* ------------------------------------------------------------------------- */

static long int dram_size (void);

/* ------------------------------------------------------------------------- */

#define MBYTE		(1024*1024)
#define DRAM_DELAY	0x00000379  /* DRAM delay count */
#define	_NOT_USED_	0xFFFFCC25

const uint sdram_table[] =
{
	/*  single read. (offset 0 in upm RAM) */
	0x1F07D004, 0xEEAEE004, 0x11ADD004, 0xEFBBA000,
	0x1FF75447, 0x1FF77C34, 0xEFEABC34, 0x1FB57C35,

	/* burst read. (Offset 8 in upm RAM)   */
	0x1F07D004, 0xEEAEE004, 0x00ADC004, 0x00AFC000,
	0x00AFC000, 0x01AFC000, 0x0FBB8000, 0x1FF75447,
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,

	/* single write. (Offset 0x18 in upm RAM) */
	0x1F27D004, 0xEEAEA000, 0x01B90004, 0x1FF75447,
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,

	/*  burst write. (Offset 0x20 in upm RAM) */
	0x1F07D004, 0xEEAEA000, 0x00AD4000, 0x00AFC000,
	0x00AFC000, 0x01BB8004, 0x1FF75447, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,

	/* Refresh cycle, offset 0x30 */
	0x1FF5DC84, 0xFFFFFC04, 0xFFFFFC04, 0xFFFFFC04,
	0xFFFFFC84, 0xFFFFFC07, 0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,

	/* Exception, 0ffset 0x3C */
	0x7FFFFC07, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
};
/* ------------------------------------------------------------------------- */


/*
 * Check Board Identity:
 *
 * Return 1 for now.
 *
 */

int checkboard (void)
{
	printf("Marel V37\n") ;
	return (0) ;
}

/* ------------------------------------------------------------------------- */

phys_size_t initdram (int board_type)
{
    volatile immap_t     *immap  = (immap_t *)CONFIG_SYS_IMMR;
    volatile memctl8xx_t *memctl = &immap->im_memctl;
    unsigned long temp;
    volatile int delay_cnt;
    long int ramsize;

    ramsize = dram_size();

	/* Refresh clock prescalar */
    memctl->memc_mptpr = 0x400 ;

    if( ramsize == 32*MBYTE )
       temp = 0xd0904110;
   else				/* 16MB */
       temp = 0xd0802110;

    memctl->memc_mbmr = temp;

    upmconfig(UPMB, (uint *)sdram_table, sizeof(sdram_table)/sizeof(uint));

	/* Map controller banks 2 to the SDRAM bank */
    memctl->memc_or2 = 0xA00 | (0 - ramsize);
    memctl->memc_br2 = 0xC1;

    memctl->memc_mbmr = temp | 0x08;
    memctl->memc_mcr  = 0x80804130;

    delay_cnt = 0;
    while( delay_cnt++ < DRAM_DELAY )
	;

    /* Run MRS command in location 5-8 of UPMB */

    memctl->memc_mbmr = temp | 0x04;
    memctl->memc_mar  = 0x88;

    memctl->memc_mcr  = 0x80804105;

    delay_cnt = 0;
    while( delay_cnt++ < DRAM_DELAY )
	;

#ifdef	CONFIG_CAN_DRIVER
    /* Initialize OR3 / BR3 */
    memctl->memc_or3 = CONFIG_SYS_OR3_CAN;
    memctl->memc_br3 = CONFIG_SYS_BR3_CAN;

    /* Initialize MBMR */
    memctl->memc_mamr = MAMR_GPL_A4DIS;	/* GPL_A4 ouput line Disable */

    /* Initialize UPMB for CAN: single read */
    memctl->memc_mdr = 0xFFFFC004;
    memctl->memc_mcr = 0x0100 | UPMA;

    memctl->memc_mdr = 0x0FFFD004;
    memctl->memc_mcr = 0x0101 | UPMA;

    memctl->memc_mdr = 0x0FFFC000;
    memctl->memc_mcr = 0x0102 | UPMA;

    memctl->memc_mdr = 0x3FFFC004;
    memctl->memc_mcr = 0x0103 | UPMA;

    memctl->memc_mdr = 0xFFFFDC05;
    memctl->memc_mcr = 0x0104 | UPMA;

    /* Initialize UPMB for CAN: single write */
    memctl->memc_mdr = 0xFFFCC004;
    memctl->memc_mcr = 0x0118 | UPMA;

    memctl->memc_mdr = 0xCFFCD004;
    memctl->memc_mcr = 0x0119 | UPMA;

    memctl->memc_mdr = 0x0FFCC000;
    memctl->memc_mcr = 0x011A | UPMA;

    memctl->memc_mdr = 0x7FFCC004;
    memctl->memc_mcr = 0x011B | UPMA;

    memctl->memc_mdr = 0xFFFDCC05;
    memctl->memc_mcr = 0x011C | UPMA;
#endif	/* CONFIG_CAN_DRIVER */

    return (dram_size());
}

/* ------------------------------------------------------------------------- */

/*
 * Find size of RAM from configuration pins.
 * The input pins that contain the memory size are also the debug port
 * pins.  Normally they are configured as debug port pins.  To be able
 * to read the memory configuration, we must deactivate the debug port
 * and enable the pcmcia input pins.  Then return the register to
 * previous state.
 */

static long int dram_size ()
{
    volatile immap_t     *immap  = (immap_t *)CONFIG_SYS_IMMR;
    volatile sysconf8xx_t *siu = &immap->im_siu_conf;
    volatile pcmconf8xx_t *pcm = &immap->im_pcmcia;
    long int		  i, memory=1;
    unsigned long siu_mcr;

    siu_mcr = siu->sc_siumcr;
    siu->sc_siumcr = siu_mcr & 0xFF9FFFFF;
    for(i=0; i<10; i++) i = i;

    memory = (pcm->pcmc_pipr>>12) & 0x3;

    siu->sc_siumcr = siu_mcr;

    switch( memory )
    {
	case 1:
	    return( 32*MBYTE );
	case 2:
	    return( 64*MBYTE );
	default:
	    break;
    }
    return( 16*MBYTE );
}
