/*
 * (C) Copyright 2000
 * Subodh Nijsure, SkyStream Networks, snijsure@skystream.com
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
#include <cmd_boot.h>
#if defined(CONFIG_8xx)
#include <mpc8xx.h>
#elif defined (CONFIG_405GP)
#include <asm/processor.h>
#elif defined (CONFIG_5xx)
#include <mpc5xx.h>
#endif
#if (CONFIG_COMMANDS & CFG_CMD_REGINFO)

int do_reginfo (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
#if defined(CONFIG_8xx)
	volatile immap_t     *immap  = (immap_t *)CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	volatile sysconf8xx_t *sysconf = &immap->im_siu_conf;
	volatile sit8xx_t *timers = &immap->im_sit;

	/* Hopefully more PowerPC  knowledgable people will add code to display
	 * other useful registers
	 */

	printf("\nSystem Configuration registers\n");

	printf("\tIMMR\t0x%08X\n", get_immr(0));

	printf("\tSIUMCR\t0x%08X", sysconf->sc_siumcr);
	printf("\tSYPCR\t0x%08X\n",sysconf->sc_sypcr);

	printf("\tSWT\t0x%08X",    sysconf->sc_swt);
	printf("\tSWSR\t0x%04X\n", sysconf->sc_swsr);

	printf("\tSIPEND\t0x%08X\tSIMASK\t0x%08X\n",
		sysconf->sc_sipend, sysconf->sc_simask);
	printf("\tSIEL\t0x%08X\tSIVEC\t0x%08X\n",
		sysconf->sc_siel, sysconf->sc_sivec);
	printf("\tTESR\t0x%08X\tSDCR\t0x%08X\n",
		sysconf->sc_tesr, sysconf->sc_sdcr);

	printf("Memory Controller Registers\n");

	printf("\tBR0\t0x%08X\tOR0\t0x%08X \n", memctl->memc_br0, memctl->memc_or0);
	printf("\tBR1\t0x%08X\tOR1\t0x%08X \n", memctl->memc_br1, memctl->memc_or1);
	printf("\tBR2\t0x%08X\tOR2\t0x%08X \n", memctl->memc_br2, memctl->memc_or2);
	printf("\tBR3\t0x%08X\tOR3\t0x%08X \n", memctl->memc_br3, memctl->memc_or3);
	printf("\tBR4\t0x%08X\tOR4\t0x%08X \n", memctl->memc_br4, memctl->memc_or4);
	printf("\tBR5\t0x%08X\tOR5\t0x%08X \n", memctl->memc_br5, memctl->memc_or5);
	printf("\tBR6\t0x%08X\tOR6\t0x%08X \n", memctl->memc_br6, memctl->memc_or6);
	printf("\tBR7\t0x%08X\tOR7\t0x%08X \n", memctl->memc_br7, memctl->memc_or7);
	printf("\n");

	printf("\tmamr\t0x%08X\tmbmr\t0x%08X \n",
		memctl->memc_mamr, memctl->memc_mbmr );
	printf("\tmstat\t0x%08X\tmptpr\t0x%08X \n",
		memctl->memc_mstat, memctl->memc_mptpr );
	printf("\tmdr\t0x%08X \n", memctl->memc_mdr);

	printf("\nSystem Integration Timers\n");
	printf("\tTBSCR\t0x%08X\tRTCSC\t0x%08X \n",
		timers->sit_tbscr, timers->sit_rtcsc);
	printf("\tPISCR\t0x%08X \n", timers->sit_piscr);

	/*
	 * May be some CPM info here?
	 */

/* DBU[dave@cray.com]   For the CRAY-L1, but should be generically 405gp */
#elif defined (CONFIG_405GP) || defined(CONFIG_405EP)
	printf("\n405GP registers; MSR=%x\n",mfmsr());
		printf ("\nUniversal Interrupt Controller Regs\n"
"uicsr    uicsrs   uicer    uiccr    uicpr    uictr    uicmsr   uicvr    uicvcr"
"\n"
"%08x %08x %08x %08x %08x %08x %08x %08x %08x\n",
	mfdcr(uicsr),
	mfdcr(uicsrs),
	mfdcr(uicer),
	mfdcr(uiccr),
	mfdcr(uicpr),
	mfdcr(uictr),
	mfdcr(uicmsr),
	mfdcr(uicvr),
	mfdcr(uicvcr));

	printf ("\nMemory (SDRAM) Configuration\n"
"besra    besrsa   besrb    besrsb   bear     mcopt1   rtr      pmit\n");

	mtdcr(memcfga,mem_besra); 	printf ("%08x ", mfdcr(memcfgd));
	mtdcr(memcfga,mem_besrsa);	printf ("%08x ", mfdcr(memcfgd));
	mtdcr(memcfga,mem_besrb); 	printf ("%08x ", mfdcr(memcfgd));
	mtdcr(memcfga,mem_besrsb); 	printf ("%08x ", mfdcr(memcfgd));
	mtdcr(memcfga,mem_bear); 	printf ("%08x ", mfdcr(memcfgd));
	mtdcr(memcfga,mem_mcopt1); 	printf ("%08x ", mfdcr(memcfgd));
	mtdcr(memcfga,mem_rtr); 	printf ("%08x ", mfdcr(memcfgd));
	mtdcr(memcfga,mem_pmit); 	printf ("%08x ", mfdcr(memcfgd));

	printf ("\n"
"mb0cf    mb1cf    mb2cf    mb3cf    sdtr1    ecccf    eccerr\n");
	mtdcr(memcfga,mem_mb0cf); 	printf ("%08x ", mfdcr(memcfgd));
	mtdcr(memcfga,mem_mb1cf); 	printf ("%08x ", mfdcr(memcfgd));
	mtdcr(memcfga,mem_mb2cf); 	printf ("%08x ", mfdcr(memcfgd));
	mtdcr(memcfga,mem_mb3cf); 	printf ("%08x ", mfdcr(memcfgd));
	mtdcr(memcfga,mem_sdtr1); 	printf ("%08x ", mfdcr(memcfgd));
	mtdcr(memcfga,mem_ecccf); 	printf ("%08x ", mfdcr(memcfgd));
	mtdcr(memcfga,mem_eccerr); 	printf ("%08x ", mfdcr(memcfgd));

	printf ("\n\n"
"DMA Channels\n"
"dmasr    dmasgc   dmaadr\n"						"%08x %08x %08x\n"
"dmacr_0  dmact_0  dmada_0  dmasa_0  dmasb_0\n"	"%08x %08x %08x %08x %08x\n"
"dmacr_1  dmact_1  dmada_1  dmasa_1  dmasb_1\n"	"%08x %08x %08x %08x %08x\n",
mfdcr(dmasr),  mfdcr(dmasgc),mfdcr(dmaadr),
mfdcr(dmacr0), mfdcr(dmact0),mfdcr(dmada0), mfdcr(dmasa0), mfdcr(dmasb0),
mfdcr(dmacr1), mfdcr(dmact1),mfdcr(dmada1), mfdcr(dmasa1), mfdcr(dmasb1));

	printf (
"dmacr_2  dmact_2  dmada_2  dmasa_2  dmasb_2\n"	"%08x %08x %08x %08x %08x\n"
"dmacr_3  dmact_3  dmada_3  dmasa_3  dmasb_3\n"	"%08x %08x %08x %08x %08x\n",
mfdcr(dmacr2), mfdcr(dmact2),mfdcr(dmada2), mfdcr(dmasa2), mfdcr(dmasb2),
mfdcr(dmacr3), mfdcr(dmact3),mfdcr(dmada3), mfdcr(dmasa3), mfdcr(dmasb3) );

	printf ("\n"
"External Bus\n"
"pbear    pbesr0   pbesr1   epcr\n");
	mtdcr(ebccfga,pbear); 	printf ("%08x ", mfdcr(ebccfgd));
	mtdcr(ebccfga,pbesr0); 	printf ("%08x ", mfdcr(ebccfgd));
	mtdcr(ebccfga,pbesr1); 	printf ("%08x ", mfdcr(ebccfgd));
	mtdcr(ebccfga,epcr); 	printf ("%08x ", mfdcr(ebccfgd));

	printf ("\n"
"pb0cr    pb0ap    pb1cr    bp1ap    pb2cr    pb2ap    pb3cr    pb3ap\n");
	mtdcr(ebccfga,pb0cr); 	printf ("%08x ", mfdcr(ebccfgd));
	mtdcr(ebccfga,pb0ap); 	printf ("%08x ", mfdcr(ebccfgd));
	mtdcr(ebccfga,pb1cr); 	printf ("%08x ", mfdcr(ebccfgd));
	mtdcr(ebccfga,pb1ap); 	printf ("%08x ", mfdcr(ebccfgd));
	mtdcr(ebccfga,pb2cr); 	printf ("%08x ", mfdcr(ebccfgd));
	mtdcr(ebccfga,pb2ap); 	printf ("%08x ", mfdcr(ebccfgd));
	mtdcr(ebccfga,pb3cr); 	printf ("%08x ", mfdcr(ebccfgd));
	mtdcr(ebccfga,pb3ap); 	printf ("%08x ", mfdcr(ebccfgd));

	printf ("\n"
"pb4cr    pb4ap    pb5cr    bp5ap    pb6cr    pb6ap    pb7cr    pb7ap\n");
	mtdcr(ebccfga,pb4cr); 	printf ("%08x ", mfdcr(ebccfgd));
	mtdcr(ebccfga,pb4ap); 	printf ("%08x ", mfdcr(ebccfgd));
	mtdcr(ebccfga,pb5cr); 	printf ("%08x ", mfdcr(ebccfgd));
	mtdcr(ebccfga,pb5ap); 	printf ("%08x ", mfdcr(ebccfgd));
	mtdcr(ebccfga,pb6cr); 	printf ("%08x ", mfdcr(ebccfgd));
	mtdcr(ebccfga,pb6ap); 	printf ("%08x ", mfdcr(ebccfgd));
	mtdcr(ebccfga,pb7cr); 	printf ("%08x ", mfdcr(ebccfgd));
	mtdcr(ebccfga,pb7ap); 	printf ("%08x ", mfdcr(ebccfgd));

	printf ("\n\n");
#elif defined(CONFIG_5xx)

	volatile immap_t     	*immap  = (immap_t *)CFG_IMMR;
	volatile memctl5xx_t	*memctl = &immap->im_memctl;
	volatile sysconf5xx_t	*sysconf = &immap->im_siu_conf;
	volatile sit5xx_t	*timers = &immap->im_sit;
	volatile car5xx_t	*car = &immap->im_clkrst;
	volatile uimb5xx_t	*uimb = &immap->im_uimb;

	printf("\nSystem Configuration registers\n");
	printf("\tIMMR\t0x%08X\tSIUMCR\t0x%08X \n", get_immr(0), sysconf->sc_siumcr);
	printf("\tSYPCR\t0x%08X\tSWSR\t0x%04X \n" ,sysconf->sc_sypcr, sysconf->sc_swsr);
	printf("\tSIPEND\t0x%08X\tSIMASK\t0x%08X \n", sysconf->sc_sipend, sysconf->sc_simask);
	printf("\tSIEL\t0x%08X\tSIVEC\t0x%08X \n", sysconf->sc_siel, sysconf->sc_sivec);
	printf("\tTESR\t0x%08X\n", sysconf->sc_tesr);

	printf("\nMemory Controller Registers\n");
	printf("\tBR0\t0x%08X\tOR0\t0x%08X \n", memctl->memc_br0, memctl->memc_or0);
	printf("\tBR1\t0x%08X\tOR1\t0x%08X \n", memctl->memc_br1, memctl->memc_or1);
	printf("\tBR2\t0x%08X\tOR2\t0x%08X \n", memctl->memc_br2, memctl->memc_or2);
	printf("\tBR3\t0x%08X\tOR3\t0x%08X \n", memctl->memc_br3, memctl->memc_or3);
	printf("\tDMBR\t0x%08X\tDMOR\t0x%08X \n", memctl->memc_dmbr, memctl->memc_dmor );
	printf("\tMSTAT\t0x%08X\n", memctl->memc_mstat);

	printf("\nSystem Integration Timers\n");
	printf("\tTBSCR\t0x%08X\tRTCSC\t0x%08X \n", timers->sit_tbscr, timers->sit_rtcsc);
	printf("\tPISCR\t0x%08X \n", timers->sit_piscr);

	printf("\nClocks and Reset\n");
	printf("\tSCCR\t0x%08X\tPLPRCR\t0x%08X \n", car->car_sccr, car->car_plprcr);

	printf("\nU-Bus to IMB3 Bus Interface\n");
	printf("\tUMCR\t0x%08X\tUIPEND\t0x%08X \n", uimb->uimb_umcr, uimb->uimb_uipend);
	printf ("\n\n");
#endif /* CONFIG_5xx */
	return 0;
}

#endif	/* CONFIG_COMMANDS & CFG_CMD_REGINFO */
