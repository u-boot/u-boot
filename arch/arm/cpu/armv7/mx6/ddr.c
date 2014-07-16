/*
 * Copyright (C) 2014 Gateworks Corporation
 * Author: Tim Harvey <tharvey@gateworks.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <linux/types.h>
#include <asm/arch/mx6-ddr.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <asm/types.h>

#if defined(CONFIG_MX6QDL) || defined(CONFIG_MX6Q) || defined(CONFIG_MX6D)
/* Configure MX6DQ mmdc iomux */
void mx6dq_dram_iocfg(unsigned width,
		      const struct mx6dq_iomux_ddr_regs *ddr,
		      const struct mx6dq_iomux_grp_regs *grp)
{
	volatile struct mx6dq_iomux_ddr_regs *mx6_ddr_iomux;
	volatile struct mx6dq_iomux_grp_regs *mx6_grp_iomux;

	mx6_ddr_iomux = (struct mx6dq_iomux_ddr_regs *)MX6DQ_IOM_DDR_BASE;
	mx6_grp_iomux = (struct mx6dq_iomux_grp_regs *)MX6DQ_IOM_GRP_BASE;

	/* DDR IO Type */
	mx6_grp_iomux->grp_ddr_type = grp->grp_ddr_type;
	mx6_grp_iomux->grp_ddrpke = grp->grp_ddrpke;

	/* Clock */
	mx6_ddr_iomux->dram_sdclk_0 = ddr->dram_sdclk_0;
	mx6_ddr_iomux->dram_sdclk_1 = ddr->dram_sdclk_1;

	/* Address */
	mx6_ddr_iomux->dram_cas = ddr->dram_cas;
	mx6_ddr_iomux->dram_ras = ddr->dram_ras;
	mx6_grp_iomux->grp_addds = grp->grp_addds;

	/* Control */
	mx6_ddr_iomux->dram_reset = ddr->dram_reset;
	mx6_ddr_iomux->dram_sdcke0 = ddr->dram_sdcke0;
	mx6_ddr_iomux->dram_sdcke1 = ddr->dram_sdcke1;
	mx6_ddr_iomux->dram_sdba2 = ddr->dram_sdba2;
	mx6_ddr_iomux->dram_sdodt0 = ddr->dram_sdodt0;
	mx6_ddr_iomux->dram_sdodt1 = ddr->dram_sdodt1;
	mx6_grp_iomux->grp_ctlds = grp->grp_ctlds;

	/* Data Strobes */
	mx6_grp_iomux->grp_ddrmode_ctl = grp->grp_ddrmode_ctl;
	mx6_ddr_iomux->dram_sdqs0 = ddr->dram_sdqs0;
	mx6_ddr_iomux->dram_sdqs1 = ddr->dram_sdqs1;
	if (width >= 32) {
		mx6_ddr_iomux->dram_sdqs2 = ddr->dram_sdqs2;
		mx6_ddr_iomux->dram_sdqs3 = ddr->dram_sdqs3;
	}
	if (width >= 64) {
		mx6_ddr_iomux->dram_sdqs4 = ddr->dram_sdqs4;
		mx6_ddr_iomux->dram_sdqs5 = ddr->dram_sdqs5;
		mx6_ddr_iomux->dram_sdqs6 = ddr->dram_sdqs6;
		mx6_ddr_iomux->dram_sdqs7 = ddr->dram_sdqs7;
	}

	/* Data */
	mx6_grp_iomux->grp_ddrmode = grp->grp_ddrmode;
	mx6_grp_iomux->grp_b0ds = grp->grp_b0ds;
	mx6_grp_iomux->grp_b1ds = grp->grp_b1ds;
	if (width >= 32) {
		mx6_grp_iomux->grp_b2ds = grp->grp_b2ds;
		mx6_grp_iomux->grp_b3ds = grp->grp_b3ds;
	}
	if (width >= 64) {
		mx6_grp_iomux->grp_b4ds = grp->grp_b4ds;
		mx6_grp_iomux->grp_b5ds = grp->grp_b5ds;
		mx6_grp_iomux->grp_b6ds = grp->grp_b6ds;
		mx6_grp_iomux->grp_b7ds = grp->grp_b7ds;
	}
	mx6_ddr_iomux->dram_dqm0 = ddr->dram_dqm0;
	mx6_ddr_iomux->dram_dqm1 = ddr->dram_dqm1;
	if (width >= 32) {
		mx6_ddr_iomux->dram_dqm2 = ddr->dram_dqm2;
		mx6_ddr_iomux->dram_dqm3 = ddr->dram_dqm3;
	}
	if (width >= 64) {
		mx6_ddr_iomux->dram_dqm4 = ddr->dram_dqm4;
		mx6_ddr_iomux->dram_dqm5 = ddr->dram_dqm5;
		mx6_ddr_iomux->dram_dqm6 = ddr->dram_dqm6;
		mx6_ddr_iomux->dram_dqm7 = ddr->dram_dqm7;
	}
}
#endif

#if defined(CONFIG_MX6QDL) || defined(CONFIG_MX6DL) || defined(CONFIG_MX6S)
/* Configure MX6SDL mmdc iomux */
void mx6sdl_dram_iocfg(unsigned width,
		       const struct mx6sdl_iomux_ddr_regs *ddr,
		       const struct mx6sdl_iomux_grp_regs *grp)
{
	volatile struct mx6sdl_iomux_ddr_regs *mx6_ddr_iomux;
	volatile struct mx6sdl_iomux_grp_regs *mx6_grp_iomux;

	mx6_ddr_iomux = (struct mx6sdl_iomux_ddr_regs *)MX6SDL_IOM_DDR_BASE;
	mx6_grp_iomux = (struct mx6sdl_iomux_grp_regs *)MX6SDL_IOM_GRP_BASE;

	/* DDR IO Type */
	mx6_grp_iomux->grp_ddr_type = grp->grp_ddr_type;
	mx6_grp_iomux->grp_ddrpke = grp->grp_ddrpke;

	/* Clock */
	mx6_ddr_iomux->dram_sdclk_0 = ddr->dram_sdclk_0;
	mx6_ddr_iomux->dram_sdclk_1 = ddr->dram_sdclk_1;

	/* Address */
	mx6_ddr_iomux->dram_cas = ddr->dram_cas;
	mx6_ddr_iomux->dram_ras = ddr->dram_ras;
	mx6_grp_iomux->grp_addds = grp->grp_addds;

	/* Control */
	mx6_ddr_iomux->dram_reset = ddr->dram_reset;
	mx6_ddr_iomux->dram_sdcke0 = ddr->dram_sdcke0;
	mx6_ddr_iomux->dram_sdcke1 = ddr->dram_sdcke1;
	mx6_ddr_iomux->dram_sdba2 = ddr->dram_sdba2;
	mx6_ddr_iomux->dram_sdodt0 = ddr->dram_sdodt0;
	mx6_ddr_iomux->dram_sdodt1 = ddr->dram_sdodt1;
	mx6_grp_iomux->grp_ctlds = grp->grp_ctlds;

	/* Data Strobes */
	mx6_grp_iomux->grp_ddrmode_ctl = grp->grp_ddrmode_ctl;
	mx6_ddr_iomux->dram_sdqs0 = ddr->dram_sdqs0;
	mx6_ddr_iomux->dram_sdqs1 = ddr->dram_sdqs1;
	if (width >= 32) {
		mx6_ddr_iomux->dram_sdqs2 = ddr->dram_sdqs2;
		mx6_ddr_iomux->dram_sdqs3 = ddr->dram_sdqs3;
	}
	if (width >= 64) {
		mx6_ddr_iomux->dram_sdqs4 = ddr->dram_sdqs4;
		mx6_ddr_iomux->dram_sdqs5 = ddr->dram_sdqs5;
		mx6_ddr_iomux->dram_sdqs6 = ddr->dram_sdqs6;
		mx6_ddr_iomux->dram_sdqs7 = ddr->dram_sdqs7;
	}

	/* Data */
	mx6_grp_iomux->grp_ddrmode = grp->grp_ddrmode;
	mx6_grp_iomux->grp_b0ds = grp->grp_b0ds;
	mx6_grp_iomux->grp_b1ds = grp->grp_b1ds;
	if (width >= 32) {
		mx6_grp_iomux->grp_b2ds = grp->grp_b2ds;
		mx6_grp_iomux->grp_b3ds = grp->grp_b3ds;
	}
	if (width >= 64) {
		mx6_grp_iomux->grp_b4ds = grp->grp_b4ds;
		mx6_grp_iomux->grp_b5ds = grp->grp_b5ds;
		mx6_grp_iomux->grp_b6ds = grp->grp_b6ds;
		mx6_grp_iomux->grp_b7ds = grp->grp_b7ds;
	}
	mx6_ddr_iomux->dram_dqm0 = ddr->dram_dqm0;
	mx6_ddr_iomux->dram_dqm1 = ddr->dram_dqm1;
	if (width >= 32) {
		mx6_ddr_iomux->dram_dqm2 = ddr->dram_dqm2;
		mx6_ddr_iomux->dram_dqm3 = ddr->dram_dqm3;
	}
	if (width >= 64) {
		mx6_ddr_iomux->dram_dqm4 = ddr->dram_dqm4;
		mx6_ddr_iomux->dram_dqm5 = ddr->dram_dqm5;
		mx6_ddr_iomux->dram_dqm6 = ddr->dram_dqm6;
		mx6_ddr_iomux->dram_dqm7 = ddr->dram_dqm7;
	}
}
#endif

/*
 * Configure mx6 mmdc registers based on:
 *  - board-specific memory configuration
 *  - board-specific calibration data
 *  - ddr3 chip details
 *
 * The various calculations here are derived from the Freescale
 * i.Mx6DQSDL DDR3 Script Aid spreadsheet (DOC-94917) designed to generate MMDC
 * configuration registers based on memory system and memory chip parameters.
 *
 * The defaults here are those which were specified in the spreadsheet.
 * For details on each register, refer to the IMX6DQRM and/or IMX6SDLRM
 * section titled MMDC initialization
 */
#define MR(val, ba, cmd, cs1) \
	((val << 16) | (1 << 15) | (cmd << 4) | (cs1 << 3) | ba)
void mx6_dram_cfg(const struct mx6_ddr_sysinfo *i,
		  const struct mx6_mmdc_calibration *c,
		  const struct mx6_ddr3_cfg *m)
{
	volatile struct mmdc_p_regs *mmdc0;
	volatile struct mmdc_p_regs *mmdc1;
	u32 reg;
	u8 tcke, tcksrx, tcksre, txpdll, taofpd, taonpd, trrd;
	u8 todtlon, taxpd, tanpd, tcwl, txp, tfaw, tcl;
	u8 todt_idle_off = 0x4; /* from DDR3 Script Aid spreadsheet */
	u16 trcd, trc, tras, twr, tmrd, trtp, trp, twtr, trfc, txs, txpr;
	u16 CS0_END;
	u16 tdllk = 0x1ff; /* DLL locking time: 512 cycles (JEDEC DDR3) */
	int clkper; /* clock period in picoseconds */
	int clock; /* clock freq in mHz */
	int cs;

	mmdc0 = (struct mmdc_p_regs *)MMDC_P0_BASE_ADDR;
	mmdc1 = (struct mmdc_p_regs *)MMDC_P1_BASE_ADDR;

	/* MX6D/MX6Q: 1066 MHz memory clock, clkper = 1.894ns = 1894ps */
	if (is_cpu_type(MXC_CPU_MX6Q) || is_cpu_type(MXC_CPU_MX6D)) {
		clock = 528;
		tcwl = 4;
	}
	/* MX6S/MX6DL: 800 MHz memory clock, clkper = 2.5ns = 2500ps */
	else {
		clock = 400;
		tcwl = 3;
	}
	clkper = (1000*1000)/clock; /* ps */
	todtlon = tcwl;
	taxpd = tcwl;
	tanpd = tcwl;
	tcwl = tcwl;

	switch (m->density) {
	case 1: /* 1Gb per chip */
		trfc = DIV_ROUND_UP(110000, clkper) - 1;
		txs = DIV_ROUND_UP(120000, clkper) - 1;
		break;
	case 2: /* 2Gb per chip */
		trfc = DIV_ROUND_UP(160000, clkper) - 1;
		txs = DIV_ROUND_UP(170000, clkper) - 1;
		break;
	case 4: /* 4Gb per chip */
		trfc = DIV_ROUND_UP(260000, clkper) - 1;
		txs = DIV_ROUND_UP(270000, clkper) - 1;
		break;
	case 8: /* 8Gb per chip */
		trfc = DIV_ROUND_UP(350000, clkper) - 1;
		txs = DIV_ROUND_UP(360000, clkper) - 1;
		break;
	default:
		/* invalid density */
		printf("invalid chip density\n");
		hang();
		break;
	}
	txpr = txs;

	switch (m->mem_speed) {
	case 800:
		txp = DIV_ROUND_UP(MAX(3*clkper, 7500), clkper) - 1;
		tcke = DIV_ROUND_UP(MAX(3*clkper, 7500), clkper) - 1;
		if (m->pagesz == 1) {
			tfaw = DIV_ROUND_UP(40000, clkper) - 1;
			trrd = DIV_ROUND_UP(MAX(4*clkper, 10000), clkper) - 1;
		} else {
			tfaw = DIV_ROUND_UP(50000, clkper) - 1;
			trrd = DIV_ROUND_UP(MAX(4*clkper, 10000), clkper) - 1;
		}
		break;
	case 1066:
		txp = DIV_ROUND_UP(MAX(3*clkper, 7500), clkper) - 1;
		tcke = DIV_ROUND_UP(MAX(3*clkper, 5625), clkper) - 1;
		if (m->pagesz == 1) {
			tfaw = DIV_ROUND_UP(37500, clkper) - 1;
			trrd = DIV_ROUND_UP(MAX(4*clkper, 7500), clkper) - 1;
		} else {
			tfaw = DIV_ROUND_UP(50000, clkper) - 1;
			trrd = DIV_ROUND_UP(MAX(4*clkper, 10000), clkper) - 1;
		}
		break;
	case 1333:
		txp = DIV_ROUND_UP(MAX(3*clkper, 6000), clkper) - 1;
		tcke = DIV_ROUND_UP(MAX(3*clkper, 5625), clkper) - 1;
		if (m->pagesz == 1) {
			tfaw = DIV_ROUND_UP(30000, clkper) - 1;
			trrd = DIV_ROUND_UP(MAX(4*clkper, 6000), clkper) - 1;
		} else {
			tfaw = DIV_ROUND_UP(45000, clkper) - 1;
			trrd = DIV_ROUND_UP(MAX(4*clkper, 7500), clkper) - 1;
		}
		break;
	case 1600:
		txp = DIV_ROUND_UP(MAX(3*clkper, 6000), clkper) - 1;
		tcke = DIV_ROUND_UP(MAX(3*clkper, 5000), clkper) - 1;
		if (m->pagesz == 1) {
			tfaw = DIV_ROUND_UP(30000, clkper) - 1;
			trrd = DIV_ROUND_UP(MAX(4*clkper, 6000), clkper) - 1;
		} else {
			tfaw = DIV_ROUND_UP(40000, clkper) - 1;
			trrd = DIV_ROUND_UP(MAX(4*clkper, 7500), clkper) - 1;
		}
		break;
	default:
		printf("invalid memory speed\n");
		hang();
		break;
	}
	txpdll = DIV_ROUND_UP(MAX(10*clkper, 24000), clkper) - 1;
	tcl = DIV_ROUND_UP(m->trcd, clkper/10) - 3;
	tcksre = DIV_ROUND_UP(MAX(5*clkper, 10000), clkper);
	tcksrx = tcksre;
	taonpd = DIV_ROUND_UP(2000, clkper) - 1;
	taofpd = taonpd;
	trp = DIV_ROUND_UP(m->trcd, clkper/10) - 1;
	trcd = trp;
	trc = DIV_ROUND_UP(m->trcmin, clkper/10) - 1;
	tras = DIV_ROUND_UP(m->trasmin, clkper/10) - 1;
	twr = DIV_ROUND_UP(15000, clkper) - 1;
	tmrd = DIV_ROUND_UP(MAX(12*clkper, 15000), clkper) - 1;
	twtr = ROUND(MAX(4*clkper, 7500)/clkper, 1) - 1;
	trtp = twtr;
	CS0_END = ((4*i->cs_density) <= 120) ? (4*i->cs_density)+7 : 127;
	debug("density:%d Gb (%d Gb per chip)\n", i->cs_density, m->density);
	debug("clock: %dMHz (%d ps)\n", clock, clkper);
	debug("memspd:%d\n", m->mem_speed);
	debug("tcke=%d\n", tcke);
	debug("tcksrx=%d\n", tcksrx);
	debug("tcksre=%d\n", tcksre);
	debug("taofpd=%d\n", taofpd);
	debug("taonpd=%d\n", taonpd);
	debug("todtlon=%d\n", todtlon);
	debug("tanpd=%d\n", tanpd);
	debug("taxpd=%d\n", taxpd);
	debug("trfc=%d\n", trfc);
	debug("txs=%d\n", txs);
	debug("txp=%d\n", txp);
	debug("txpdll=%d\n", txpdll);
	debug("tfaw=%d\n", tfaw);
	debug("tcl=%d\n", tcl);
	debug("trcd=%d\n", trcd);
	debug("trp=%d\n", trp);
	debug("trc=%d\n", trc);
	debug("tras=%d\n", tras);
	debug("twr=%d\n", twr);
	debug("tmrd=%d\n", tmrd);
	debug("tcwl=%d\n", tcwl);
	debug("tdllk=%d\n", tdllk);
	debug("trtp=%d\n", trtp);
	debug("twtr=%d\n", twtr);
	debug("trrd=%d\n", trrd);
	debug("txpr=%d\n", txpr);
	debug("CS0_END=%d\n", CS0_END);
	debug("ncs=%d\n", i->ncs);
	debug("Rtt_wr=%d\n", i->rtt_wr);
	debug("Rtt_nom=%d\n", i->rtt_nom);
	debug("SRT=%d\n", m->SRT);
	debug("tcl=%d\n", tcl);
	debug("twr=%d\n", twr);

	/*
	 * board-specific configuration:
	 *  These values are determined empirically and vary per board layout
	 *  see:
	 *   appnote, ddr3 spreadsheet
	 */
	mmdc0->mpwldectrl0 = c->p0_mpwldectrl0;
	mmdc0->mpwldectrl1 = c->p0_mpwldectrl1;
	mmdc0->mpdgctrl0 = c->p0_mpdgctrl0;
	mmdc0->mpdgctrl1 = c->p0_mpdgctrl1;
	mmdc0->mprddlctl = c->p0_mprddlctl;
	mmdc0->mpwrdlctl = c->p0_mpwrdlctl;
	if (i->dsize > 1) {
		mmdc1->mpwldectrl0 = c->p1_mpwldectrl0;
		mmdc1->mpwldectrl1 = c->p1_mpwldectrl1;
		mmdc1->mpdgctrl0 = c->p1_mpdgctrl0;
		mmdc1->mpdgctrl1 = c->p1_mpdgctrl1;
		mmdc1->mprddlctl = c->p1_mprddlctl;
		mmdc1->mpwrdlctl = c->p1_mpwrdlctl;
	}

	/* Read data DQ Byte0-3 delay */
	mmdc0->mprddqby0dl = (u32)0x33333333;
	mmdc0->mprddqby1dl = (u32)0x33333333;
	if (i->dsize > 0) {
		mmdc0->mprddqby2dl = (u32)0x33333333;
		mmdc0->mprddqby3dl = (u32)0x33333333;
	}
	if (i->dsize > 1) {
		mmdc1->mprddqby0dl = (u32)0x33333333;
		mmdc1->mprddqby1dl = (u32)0x33333333;
		mmdc1->mprddqby2dl = (u32)0x33333333;
		mmdc1->mprddqby3dl = (u32)0x33333333;
	}

	/* MMDC Termination: rtt_nom:2 RZQ/2(120ohm), rtt_nom:1 RZQ/4(60ohm) */
	reg = (i->rtt_nom == 2) ? 0x00011117 : 0x00022227;
	mmdc0->mpodtctrl = reg;
	if (i->dsize > 1)
		mmdc1->mpodtctrl = reg;

	/* complete calibration */
	reg = (1 << 11); /* Force measurement on delay-lines */
	mmdc0->mpmur0 = reg;
	if (i->dsize > 1)
		mmdc1->mpmur0 = reg;

	/* Step 1: configuration request */
	mmdc0->mdscr = (u32)(1 << 15); /* config request */

	/* Step 2: Timing configuration */
	reg = (trfc << 24) | (txs << 16) | (txp << 13) | (txpdll << 9) |
	      (tfaw << 4) | tcl;
	mmdc0->mdcfg0 = reg;
	reg = (trcd << 29) | (trp << 26) | (trc << 21) | (tras << 16) |
	      (1 << 15) |		/* trpa */
	      (twr << 9) | (tmrd << 5) | tcwl;
	mmdc0->mdcfg1 = reg;
	reg = (tdllk << 16) | (trtp << 6) | (twtr << 3) | trrd;
	mmdc0->mdcfg2 = reg;
	reg = (taofpd << 27) | (taonpd << 24) | (tanpd << 20) | (taxpd << 16) |
	      (todtlon << 12) | (todt_idle_off << 4);
	mmdc0->mdotc = reg;
	mmdc0->mdasp = CS0_END; /* CS addressing */

	/* Step 3: Configure DDR type */
	reg = (i->cs1_mirror << 19) | (i->walat << 16) | (i->bi_on << 12) |
	      (i->mif3_mode << 9) | (i->ralat << 6);
	mmdc0->mdmisc = reg;

	/* Step 4: Configure delay while leaving reset */
	reg = (txpr << 16) | (i->sde_to_rst << 8) | (i->rst_to_cke << 0);
	mmdc0->mdor = reg;

	/* Step 5: Configure DDR physical parameters (density and burst len) */
	reg = (m->rowaddr - 11) << 24 |		/* ROW */
	      (m->coladdr - 9) << 20 |		/* COL */
	      (1 << 19) |			/* Burst Length = 8 for DDR3 */
	      (i->dsize << 16);			/* DDR data bus size */
	mmdc0->mdctl = reg;

	/* Step 6: Perform ZQ calibration */
	reg = (u32)0xa1390001; /* one-time HW ZQ calib */
	mmdc0->mpzqhwctrl = reg;
	if (i->dsize > 1)
		mmdc1->mpzqhwctrl = reg;

	/* Step 7: Enable MMDC with desired chip select */
	reg = mmdc0->mdctl |
	      (1 << 31) |			/* SDE_0 for CS0 */
	      ((i->ncs == 2) ? 1 : 0) << 30;	/* SDE_1 for CS1 */
	mmdc0->mdctl = reg;

	/* Step 8: Write Mode Registers to Init DDR3 devices */
	for (cs = 0; cs < i->ncs; cs++) {
		/* MR2 */
		reg = (i->rtt_wr & 3) << 9 | (m->SRT & 1) << 7 |
		      ((tcwl - 3) & 3) << 3;
		mmdc0->mdscr = (u32)MR(reg, 2, 3, cs);
		/* MR3 */
		mmdc0->mdscr = (u32)MR(0, 3, 3, cs);
		/* MR1 */
		reg = ((i->rtt_nom & 1) ? 1 : 0) << 2 |
		      ((i->rtt_nom & 2) ? 1 : 0) << 6;
		mmdc0->mdscr = (u32)MR(reg, 1, 3, cs);
		reg = ((tcl - 1) << 4) |	/* CAS */
		      (1 << 8)   |		/* DLL Reset */
		      ((twr - 3) << 9);		/* Write Recovery */
		/* MR0 */
		mmdc0->mdscr = (u32)MR(reg, 0, 3, cs);
		/* ZQ calibration */
		reg = (1 << 10);
		mmdc0->mdscr = (u32)MR(reg, 0, 4, cs);
	}

	/* Step 10: Power down control and self-refresh */
	reg = (tcke & 0x7) << 16 |
	      5            << 12 |  /* PWDT_1: 256 cycles */
	      5            <<  8 |  /* PWDT_0: 256 cycles */
	      1            <<  6 |  /* BOTH_CS_PD */
	      (tcksrx & 0x7) << 3 |
	      (tcksre & 0x7);
	mmdc0->mdpdc = reg;
	mmdc0->mapsr = (u32)0x00011006; /* ADOPT power down enabled */

	/* Step 11: Configure ZQ calibration: one-time and periodic 1ms */
	mmdc0->mpzqhwctrl = (u32)0xa1390003;
	if (i->dsize > 1)
		mmdc1->mpzqhwctrl = (u32)0xa1390003;

	/* Step 12: Configure and activate periodic refresh */
	reg = (1 << 14) |	/* REF_SEL: Periodic refresh cycles of 32kHz */
	      (7 << 11);	/* REFR: Refresh Rate - 8 refreshes */
	mmdc0->mdref = reg;

	/* Step 13: Deassert config request - init complete */
	mmdc0->mdscr = (u32)0x00000000;

	/* wait for auto-ZQ calibration to complete */
	mdelay(1);
}
