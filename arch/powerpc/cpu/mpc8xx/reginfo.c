/*
 * (C) Copyright 2000
 * Subodh Nijsure, SkyStream Networks, snijsure@skystream.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mpc8xx.h>
#include <asm/io.h>
#include <asm/ppc.h>

void print_reginfo(void)
{
	immap_t __iomem     *immap  = (immap_t __iomem *)CONFIG_SYS_IMMR;
	memctl8xx_t __iomem *memctl = &immap->im_memctl;
	sysconf8xx_t __iomem *sysconf = &immap->im_siu_conf;
	sit8xx_t __iomem *timers = &immap->im_sit;

	/* Hopefully more PowerPC  knowledgable people will add code to display
	 * other useful registers
	 */

	printf("\nSystem Configuration registers\n"
		"\tIMMR\t0x%08X\n", get_immr(0));

	printf("\tSIUMCR\t0x%08X", in_be32(&sysconf->sc_siumcr));
	printf("\tSYPCR\t0x%08X\n", in_be32(&sysconf->sc_sypcr));

	printf("\tSWT\t0x%08X", in_be32(&sysconf->sc_swt));
	printf("\tSWSR\t0x%04X\n", in_be16(&sysconf->sc_swsr));

	printf("\tSIPEND\t0x%08X\tSIMASK\t0x%08X\n",
	       in_be32(&sysconf->sc_sipend), in_be32(&sysconf->sc_simask));
	printf("\tSIEL\t0x%08X\tSIVEC\t0x%08X\n",
	       in_be32(&sysconf->sc_siel), in_be32(&sysconf->sc_sivec));
	printf("\tTESR\t0x%08X\tSDCR\t0x%08X\n",
	       in_be32(&sysconf->sc_tesr), in_be32(&sysconf->sc_sdcr));

	printf("Memory Controller Registers\n");
	printf("\tBR0\t0x%08X\tOR0\t0x%08X\n", in_be32(&memctl->memc_br0),
	       in_be32(&memctl->memc_or0));
	printf("\tBR1\t0x%08X\tOR1\t0x%08X\n", in_be32(&memctl->memc_br1),
	       in_be32(&memctl->memc_or1));
	printf("\tBR2\t0x%08X\tOR2\t0x%08X\n", in_be32(&memctl->memc_br2),
	       in_be32(&memctl->memc_or2));
	printf("\tBR3\t0x%08X\tOR3\t0x%08X\n", in_be32(&memctl->memc_br3),
	       in_be32(&memctl->memc_or3));
	printf("\tBR4\t0x%08X\tOR4\t0x%08X\n", in_be32(&memctl->memc_br4),
	       in_be32(&memctl->memc_or4));
	printf("\tBR5\t0x%08X\tOR5\t0x%08X\n", in_be32(&memctl->memc_br5),
	       in_be32(&memctl->memc_or5));
	printf("\tBR6\t0x%08X\tOR6\t0x%08X\n", in_be32(&memctl->memc_br6),
	       in_be32(&memctl->memc_or6));
	printf("\tBR7\t0x%08X\tOR7\t0x%08X\n", in_be32(&memctl->memc_br7),
	       in_be32(&memctl->memc_or7));
	printf("\n\tmamr\t0x%08X\tmbmr\t0x%08X\n", in_be32(&memctl->memc_mamr),
	       in_be32(&memctl->memc_mbmr));
	printf("\tmstat\t0x%04X\tmptpr\t0x%04X\n", in_be16(&memctl->memc_mstat),
	       in_be16(&memctl->memc_mptpr));
	printf("\tmdr\t0x%08X\n", in_be32(&memctl->memc_mdr));

	printf("\nSystem Integration Timers\n");
	printf("\tTBSCR\t0x%04X\tRTCSC\t0x%04X\n",
	       in_be16(&timers->sit_tbscr), in_be16(&timers->sit_rtcsc));
	printf("\tPISCR\t0x%04X\n", in_be16(&timers->sit_piscr));

	/*
	 * May be some CPM info here?
	 */
}
