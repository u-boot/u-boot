/*
 * (C) Copyright 2003
 * Martin Winistoerfer, martinwinistoerfer@gmx.ch.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * File:		5xx_immap.h
 *
 * Discription:		MPC555 Internal Memory Map
 *
 */

#ifndef __IMMAP_5XX__
#define __IMMAP_5XX__

/* System Configuration Registers.
*/
typedef	struct sys_conf {
	uint sc_siumcr;
	uint sc_sypcr;
	char res1[6];
	ushort sc_swsr;
	uint sc_sipend;
	uint sc_simask;
	uint sc_siel;
	uint sc_sivec;
	uint sc_tesr;
	uint sc_sgpiodt1;
	uint sc_sgpiodt2;
	uint sc_sgpiocr;
	uint sc_emcr;
	uint sc_res1aa;
	uint sc_res1ab;
	uint sc_pdmcr;
	char res3[192];
} sysconf5xx_t;


/* Memory Controller Registers.
*/
typedef struct	mem_ctlr {
	uint memc_br0;
	uint memc_or0;
	uint memc_br1;
	uint memc_or1;
	uint memc_br2;
	uint memc_or2;
	uint memc_br3;
	uint memc_or3;
	char res1[32];
	uint memc_dmbr;
	uint memc_dmor;
	char res2[48];
	ushort memc_mstat;
	ushort memc_res4a;
	char res3[132];
} memctl5xx_t;

/* System Integration Timers.
*/
typedef struct	sys_int_timers {
	ushort sit_tbscr;
	char res1[2];
	uint sit_tbref0;
	uint sit_tbref1;
	char res2[20];
	ushort sit_rtcsc;
	char res3[2];
	uint sit_rtc;
	uint sit_rtsec;
	uint sit_rtcal;
	char res4[16];
	ushort sit_piscr;
	char res5[2];
	uint sit_pitc;
	uint sit_pitr;
	char res6[52];
} sit5xx_t;

/* Clocks and Reset
*/
typedef struct clk_and_reset {
	uint car_sccr;
	uint car_plprcr;
	ushort car_rsr;
	ushort car_res7a;
	ushort car_colir;
	ushort car_res7b;
	ushort car_vsrmcr;
	ushort car_res7c;
	char res1[108];

} car5xx_t;

#define TBSCR_TBE		((ushort)0x0001)

/* System Integration Timer Keys
*/
typedef struct sitk {
	uint sitk_tbscrk;
	uint sitk_tbref0k;
	uint sitk_tbref1k;
	uint sitk_tbk;
	char res1[16];
	uint sitk_rtcsck;
	uint sitk_rtck;
	uint sitk_rtseck;
	uint sitk_rtcalk;
	char res2[16];
	uint sitk_piscrk;
	uint sitk_pitck;
	char res3[56];
} sitk5xx_t;

/* Clocks and Reset Keys.
*/
typedef struct cark {
	uint	cark_sccrk;
	uint	cark_plprcrk;
	uint	cark_rsrk;
	char	res1[1140];
} cark8xx_t;

/* The key to unlock registers maintained by keep-alive power.
*/
#define KAPWR_KEY	((unsigned int)0x55ccaa33)

/* Flash Configuration
*/
typedef struct fl {
	uint fl_cmfmcr;
	uint fl_cmftst;
	uint fl_cmfctl;
	char res1[52];
} fl5xx_t;

/* Dpram Control
*/
typedef struct dprc {
	ushort dprc_dptmcr;
	ushort dprc_ramtst;
	ushort dprc_rambar;
	ushort dprc_misrh;
	ushort dprc_misrl;
	ushort dprc_miscnt;
} dprc5xx_t;

/* Time Processor Unit
*/
typedef struct tpu {
	ushort tpu_tpumcr;
	ushort tpu_tcr;
	ushort tpu_dscr;
	ushort tpu_dssr;
	ushort tpu_ticr;
	ushort tpu_cier;
	ushort tpu_cfsr0;
	ushort tpu_cfsr1;
	ushort tpu_cfsr2;
	ushort tpu_cfsr3;
	ushort tpu_hsqr0;
	ushort tpu_hsqr1;
	ushort tpu_hsrr0;
	ushort tpu_hsrr1;
	ushort tpu_cpr0;
	ushort tpu_cpr1;
	ushort tpu_cisr;
	ushort tpu_lr;
	ushort tpu_sglr;
	ushort tpu_dcnr;
	ushort tpu_tpumcr2;
	ushort tpu_tpumcr3;
	ushort tpu_isdr;
	ushort tpu_iscr;
	char   res1[208];
	char   tpu[16][16];
	char   res2[512];
} tpu5xx_t;

/* QADC
*/
typedef struct qadc {
	ushort qadc_64mcr;
	ushort qadc_64test;
	ushort qadc_64int;
	u_char  qadc_portqa;
	u_char  qadc_portqb;
	ushort qadc_ddrqa;
	ushort qadc_qacr0;
	ushort qadc_qacr1;
	ushort qadc_qacr2;
	ushort qadc_qasr0;
	ushort qadc_qasr1;
	char   res1[492];
       /* command convertion word table */
	ushort qadc_ccw[64];
       /* result word table, unsigned right justified */
	ushort qadc_rjurr[64];
       /* result word table, signed left justified */
	ushort qadc_ljsrr[64];
       /* result word table, unsigned left justified */
	ushort qadc_ljurr[64];
} qadc5xx_t;

/* QSMCM
*/
typedef struct qsmcm {
	ushort qsmcm_qsmcr;
	ushort qsmcm_qtest;
	ushort qsmcm_qdsci_il;
	ushort qsmcm_qspi_il;
	ushort qsmcm_scc1r0;
	ushort qsmcm_scc1r1;
	ushort qsmcm_sc1sr;
	ushort qsmcm_sc1dr;
	char   res1[2];
	char   res2[2];
	ushort qsmcm_portqs;
	u_char qsmcm_pqspar;
	u_char qsmcm_ddrqs;
	ushort qsmcm_spcr0;
	ushort qsmcm_spcr1;
	ushort qsmcm_spcr2;
	u_char qsmcm_spcr3;
	u_char qsmcm_spsr;
	ushort qsmcm_scc2r0;
	ushort qsmcm_scc2r1;
	ushort qsmcm_sc2sr;
	ushort qsmcm_sc2dr;
	ushort qsmcm_qsci1cr;
	ushort qsmcm_qsci1sr;
	ushort qsmcm_sctq[16];
	ushort qsmcm_scrq[16];
	char   res3[212];
	ushort qsmcm_recram[32];
	ushort qsmcm_tranram[32];
	u_char qsmcm_comdram[32];
	char   res[3616];
} qsmcm5xx_t;


/* MIOS
*/

typedef struct mios {
	ushort mios_mpwmsm0perr;                 /* mpwmsm0 */
	ushort mios_mpwmsm0pulr;
	ushort mios_mpwmsm0cntr;
	ushort mios_mpwmsm0scr;
	ushort mios_mpwmsm1perr;                 /* mpwmsm1 */
	ushort mios_mpwmsm1pulr;
	ushort mios_mpwmsm1cntr;
	ushort mios_mpwmsm1scr;
	ushort mios_mpwmsm2perr;                 /* mpwmsm2 */
	ushort mios_mpwmsm2pulr;
	ushort mios_mpwmsm2cntr;
	ushort mios_mpwmsm2scr;
	ushort mios_mpwmsm3perr;                 /* mpwmsm3 */
	ushort mios_mpwmsm3pulr;
	ushort mios_mpwmsm3cntr;
	ushort mios_mpwmsm3scr;
	char res1[16];
	ushort mios_mmcsm6cnt;                   /* mmcsm6 */
	ushort mios_mmcsm6mlr;
	ushort mios_mmcsm6scrd, mmcsm6scr;
	char res2[32];
	ushort mios_mdasm11ar;                   /* mdasm11 */
	ushort mios_mdasm11br;
	ushort mios_mdasm11scrd, mdasm11scr;
	ushort mios_mdasm12ar;                   /* mdasm12 */
	ushort mios_mdasm12br;
	ushort mios_mdasm12scrd, mdasm12scr;
	ushort mios_mdasm13ar;                   /* mdasm13 */
	ushort mios_mdasm13br;
	ushort mios_mdasm13scrd, mdasm13scr;
	ushort mios_mdasm14ar;                   /* mdasm14 */
	ushort mios_mdasm14br;
	ushort mios_mdasm14scrd, mdasm14scr;
	ushort mios_mdasm15ar;                   /* mdasm15 */
	ushort mios_mdasm15br;
	ushort mios_mdasm15scrd, mdasm15scr;
	ushort mios_mpwmsm16perr;                /* mpwmsm16 */
	ushort mios_mpwmsm16pulr;
	ushort mios_mpwmsm16cntr;
	ushort mios_mpwmsm16scr;
	ushort mios_mpwmsm17perr;                /* mpwmsm17 */
	ushort mios_mpwmsm17pulr;
	ushort mios_mpwmsm17cntr;
	ushort mios_mpwmsm17scr;
	ushort mios_mpwmsm18perr;                /* mpwmsm18 */
	ushort mios_mpwmsm18pulr;
	ushort mios_mpwmsm18cntr;
	ushort mios_mpwmsm18scr;
	ushort mios_mpwmsm19perr;                /* mpwmsm19 */
	ushort mios_mpwmsm19pulr;
	ushort mios_mpwmsm19cntr;
	ushort mios_mpwmsm19scr;
	char res3[16];
	ushort mios_mmcsm22cnt;                  /* mmcsm22 */
	ushort mios_mmcsm22mlr;
	ushort mios_mmcsm22scrd, mmcsm22scr;
	char res4[32];
	ushort mios_mdasm27ar;                   /* mdasm27 */
	ushort mios_mdasm27br;
	ushort mios_mdasm27scrd, mdasm27scr;
	ushort mios_mdasm28ar;                   /*mdasm28 */
	ushort mios_mdasm28br;
	ushort mios_mdasm28scrd, mdasm28scr;
	ushort mios_mdasm29ar;                   /* mdasm29 */
	ushort mios_mdasm29br;
	ushort mios_mdasm29scrd, mdasm29scr;
	ushort mios_mdasm30ar;                   /* mdasm30 */
	ushort mios_mdasm30br;
	ushort mios_mdasm30scrd, mdasm30scr;
	ushort mios_mdasm31ar;                   /* mdasm31 */
	ushort mios_mdasm31br;
	ushort mios_mdasm31scrd, mdasm31scr;
	ushort mios_mpiosm32dr;
	ushort mios_mpiosm32ddr;
	char res5[1788];
	ushort mios_mios1tpcr;
	char mios_res13[2];
	ushort mios_mios1vnr;
	ushort mios_mios1mcr;
	char res6[12];
	ushort mios_res42z;
	ushort mios_mcpsmscr;
	char res7[1000];
	ushort mios_mios1sr0;
	char res12[2];
	ushort mios_mios1er0;
	ushort mios_mios1rpr0;
	char res8[40];
	ushort mios_mios1lvl0;
	char res9[14];
	ushort mios_mios1sr1;
	char res10[2];
	ushort mios_mios1er1;
	ushort mios_mios1rpr1;
	char res11[40];
	ushort mios_mios1lvl1;
	char res13[1038];
} mios5xx_t;

/* Toucan Module
*/
typedef struct tcan {
	ushort tcan_tcnmcr;
	ushort tcan_cantcr;
	ushort tcan_canicr;
	u_char tcan_canctrl0;
	u_char tcan_canctrl1;
	u_char tcan_presdiv;
	u_char tcan_canctrl2;
	ushort tcan_timer;
	char res1[4];
	ushort tcan_rxgmskhi;
	ushort tcan_rxgmsklo;
	ushort tcan_rx14mskhi;
	ushort tcan_rx14msklo;
	ushort tcan_rx15mskhi;
	ushort tcan_rx15msklo;
	char res2[4];
	ushort tcan_estat;
	ushort tcan_imask;
	ushort tcan_iflag;
	u_char tcan_rxectr;
	u_char tcan_txectr;
	char res3[88];
	struct {
	       ushort scr;
	       ushort id_high;
	       ushort id_low;
	       u_char data[8];
		   char res4[2];
	    } tcan_mbuff[16];
	    char res5[640];
} tcan5xx_t;

/* UIMB
*/
typedef struct uimb {
	uint uimb_umcr;
	char res1[12];
	uint uimb_utstcreg;
	char res2[12];
	uint uimb_uipend;
} uimb5xx_t;


/* Internal Memory Map MPC555
*/
typedef struct immap {
	char               res1[262144];	/* CMF Flash A 256 Kbytes */
	char               res2[196608];	/* CMF Flash B 192 Kbytes */
	char               res3[2670592];	/* Reserved for Flash */
	sysconf5xx_t       im_siu_conf;		/* SIU Configuration */
	memctl5xx_t	   im_memctl;		/* Memory Controller */
	sit5xx_t           im_sit;		/* System Integration Timers */
	car5xx_t	   im_clkrst;		/* Clocks and Reset */
	sitk5xx_t          im_sitk;		/* System Integration Timer Keys*/
	cark8xx_t          im_clkrstk;		/* Clocks and Resert Keys */
	fl5xx_t	           im_fla;	        /* Flash Module A */
	fl5xx_t	           im_flb;	        /* Flash Module B */
	char               res4[14208];		/* Reserved for SIU */
	dprc5xx_t	   im_dprc;		/* Dpram Control Register */
	char               res5[8180];		/* Reserved */
	char               dptram[6144];	/* Dptram */
	char               res6[2048];		/* Reserved */
	tpu5xx_t	   im_tpua;		/* Time Proessing Unit A */
	tpu5xx_t	   im_tpub;		/* Time Processing Unit B */
	qadc5xx_t	   im_qadca;		/* QADC A */
	qadc5xx_t	   im_qadcb;		/* QADC B */
	qsmcm5xx_t	   im_qsmcm;		/* SCI and SPI */
	mios5xx_t	   im_mios;		/* MIOS */
	tcan5xx_t          im_tcana;		/* Toucan A */
	tcan5xx_t          im_tcanb;		/* Toucan B */
	char               res7[1792];		/* Reserved */
	uimb5xx_t          im_uimb;	        /* UIMB */
} immap_t;

#endif /* __IMMAP_5XX__ */
