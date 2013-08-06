/*
 * MPC8260 Internal Memory Map
 * Copyright (c) 1999 Dan Malek (dmalek@jlc.net)
 *
 * The Internal Memory Map of the 8260.	 I don't know how generic
 * this will be, as I don't have any knowledge of the subsequent
 * parts at this time.	I copied this from the 8xx_immap.h.
 */
#ifndef __IMMAP_82XX__
#define __IMMAP_82XX__

/* System configuration registers.
*/
typedef struct sys_conf {
	uint	sc_siumcr;
	uint	sc_sypcr;
	char	res1[6];
	ushort	sc_swsr;
	char	res2[20];
	uint	sc_bcr;
	u_char	sc_ppc_acr;
	char	res3[3];
	uint	sc_ppc_alrh;
	uint	sc_ppc_alrl;
	u_char	sc_lcl_acr;
	char	res4[3];
	uint	sc_lcl_alrh;
	uint	sc_lcl_alrl;
	uint	sc_tescr1;
	uint	sc_tescr2;
	uint	sc_ltescr1;
	uint	sc_ltescr2;
	uint	sc_pdtea;
	u_char	sc_pdtem;
	char	res5[3];
	uint	sc_ldtea;
	u_char	sc_ldtem;
	char	res6[163];
} sysconf8260_t;


/* Memory controller registers.
*/
typedef struct	mem_ctlr {
	uint	memc_br0;
	uint	memc_or0;
	uint	memc_br1;
	uint	memc_or1;
	uint	memc_br2;
	uint	memc_or2;
	uint	memc_br3;
	uint	memc_or3;
	uint	memc_br4;
	uint	memc_or4;
	uint	memc_br5;
	uint	memc_or5;
	uint	memc_br6;
	uint	memc_or6;
	uint	memc_br7;
	uint	memc_or7;
	uint	memc_br8;
	uint	memc_or8;
	uint	memc_br9;
	uint	memc_or9;
	uint	memc_br10;
	uint	memc_or10;
	uint	memc_br11;
	uint	memc_or11;
	char	res1[8];
	uint	memc_mar;
	char	res2[4];
	uint	memc_mamr;
	uint	memc_mbmr;
	uint	memc_mcmr;
	char	res3[8];
	ushort	memc_mptpr;
	char	res4[2];
	uint	memc_mdr;
	char	res5[4];
	uint	memc_psdmr;
	uint	memc_lsdmr;
	u_char	memc_purt;
	char	res6[3];
	u_char	memc_psrt;
	char	res7[3];
	u_char	memc_lurt;
	char	res8[3];
	u_char	memc_lsrt;
	char	res9[3];
	uint	memc_immr;
	uint	memc_pcibr0;
	uint	memc_pcibr1;
	char	res10[16];
	uint	memc_pcimsk0;
	uint	memc_pcimsk1;
	char	res11[52];
} memctl8260_t;

/* System Integration Timers.
*/
typedef struct	sys_int_timers {
	char	res1[32];
	ushort	sit_tmcntsc;
	char	res2[2];
	uint	sit_tmcnt;
	char	res3[4];
	uint	sit_tmcntal;
	char	res4[16];
	ushort	sit_piscr;
	char	res5[2];
	uint	sit_pitc;
	uint	sit_pitr;
	char	res6[94];
	char	res7[390];
} sit8260_t;

/* PCI
 */
typedef struct pci_config {
	uint	pci_omisr;
	uint	pci_ominr;
	char	res1[8];
	uint	pci_ifqpr;
	uint	pci_ofqpr;
	char	res2[8];
	uint	pci_imr0;
	uint	pci_imr1;
	uint	pci_omr0;
	uint	pci_omr1;
	uint	pci_odr;
	char	res3[4];
	uint	pci_idr;
	char	res4[20];
	uint	pci_imisr;
	uint	pci_imimr;
	char	res5[24];
	uint	pci_ifhpr;
	char	res5_2[4];
	uint	pci_iftpr;
	char	res6[4];
	uint	pci_iphpr;
	char	res6_2[4];
	uint	pci_iptpr;
	char	res7[4];
	uint	pci_ofhpr;
	char	res7_2[4];
	uint	pci_oftpr;
	char	res8[4];
	uint	pci_ophpr;
	char	res8_2[4];
	uint	pci_optpr;
	char	res9[8];
	uint	pci_mucr;
	char	res10[8];
	uint	pci_qbar;
	char	res11[12];
	uint	pci_dmamr0;
	uint	pci_dmasr0;
	uint	pci_dmacdar0;
	char	res12[4];
	uint	pci_dmasar0;
	char	res13[4];
	uint	pci_dmadar0;
	char	res14[4];
	uint	pci_dmabcr0;
	uint	pci_dmandar0;
	char	res15[88];
	uint	pci_dmamr1;
	uint	pci_dmasr1;
	uint	pci_dmacdar1;
	char	res16[4];
	uint	pci_dmasar1;
	char	res17[4];
	uint	pci_dmadar1;
	char	res18[4];
	uint	pci_dmabcr1;
	uint	pci_dmandar1;
	char	res19[88];
	uint	pci_dmamr2;
	uint	pci_dmasr2;
	uint	pci_dmacdar2;
	char	res20[4];
	uint	pci_dmasar2;
	char	res21[4];
	uint	pci_dmadar2;
	char	res22[4];
	uint	pci_dmabcr2;
	uint	pci_dmandar2;
	char	res23[88];
	uint	pci_dmamr3;
	uint	pci_dmasr3;
	uint	pci_dmacdar3;
	char	res24[4];
	uint	pci_dmasar3;
	char	res25[4];
	uint	pci_dmadar3;
	char	res26[4];
	uint	pci_dmabcr3;
	uint	pci_dmandar3;
	char	res27[344];
	uint	pci_potar0;
	char	res28[4];
	uint	pci_pobar0;
	char	res29[4];
	uint	pci_pocmr0;
	char	res30[4];
	uint	pci_potar1;
	char	res31[4];
	uint	pci_pobar1;
	char	res32[4];
	uint	pci_pocmr1;
	char	res33[4];
	uint	pci_potar2;
	char	res34[4];
	uint	pci_pobar2;
	char	res35[4];
	uint	pci_pocmr2;
	char	res36[52];
	uint	pci_ptcr;
	uint	pci_gpcr;
	uint	pci_gcr;
	uint	pci_esr;
	uint	pci_emr;
	uint	pci_ecr;
	uint	pci_eacr;
	char	res37[4];
	uint	pci_edcr;
	char	res38[4];
	uint	pci_eccr;
	char	res39[44];
	uint	pci_pitar1;
	char	res40[4];
	uint	pci_pibar1;
	char	res41[4];
	uint	pci_picmr1;
	char	res42[4];
	uint	pci_pitar0;
	char	res43[4];
	uint	pci_pibar0;
	char	res44[4];
	uint	pci_picmr0;
	char	res45[4];
	uint	pci_cfg_addr;
	uint	pci_cfg_data;
	uint	pci_int_ack;
	char	res46[756];
}pci8260_t;
#define PISCR_PIRQ_MASK		((ushort)0xff00)
#define PISCR_PS		((ushort)0x0080)
#define PISCR_PIE		((ushort)0x0004)
#define PISCR_PTF		((ushort)0x0002)
#define PISCR_PTE		((ushort)0x0001)

/* Interrupt Controller.
*/
typedef struct interrupt_controller {
	ushort	ic_sicr;
	char	res1[2];
	uint	ic_sivec;
	uint	ic_sipnrh;
	uint	ic_sipnrl;
	uint	ic_siprr;
	uint	ic_scprrh;
	uint	ic_scprrl;
	uint	ic_simrh;
	uint	ic_simrl;
	uint	ic_siexr;
	char	res2[88];
} intctl8260_t;

/* Clocks and Reset.
*/
typedef struct clk_and_reset {
	uint	car_sccr;
	char	res1[4];
	uint	car_scmr;
	char	res2[4];
	uint	car_rsr;
	uint	car_rmr;
	char	res[104];
} car8260_t;

/* Input/Output Port control/status registers.
 * Names consistent with processor manual, although they are different
 * from the original 8xx names.......
 */
typedef struct io_port {
	uint	iop_pdira;
	uint	iop_ppara;
	uint	iop_psora;
	uint	iop_podra;
	uint	iop_pdata;
	char	res1[12];
	uint	iop_pdirb;
	uint	iop_pparb;
	uint	iop_psorb;
	uint	iop_podrb;
	uint	iop_pdatb;
	char	res2[12];
	uint	iop_pdirc;
	uint	iop_pparc;
	uint	iop_psorc;
	uint	iop_podrc;
	uint	iop_pdatc;
	char	res3[12];
	uint	iop_pdird;
	uint	iop_ppard;
	uint	iop_psord;
	uint	iop_podrd;
	uint	iop_pdatd;
	char	res4[12];
} iop8260_t;

/* Communication Processor Module Timers
*/
typedef struct cpm_timers {
	u_char	cpmt_tgcr1;
	char	res1[3];
	u_char	cpmt_tgcr2;
	char	res2[11];
	ushort	cpmt_tmr1;
	ushort	cpmt_tmr2;
	ushort	cpmt_trr1;
	ushort	cpmt_trr2;
	ushort	cpmt_tcr1;
	ushort	cpmt_tcr2;
	ushort	cpmt_tcn1;
	ushort	cpmt_tcn2;
	ushort	cpmt_tmr3;
	ushort	cpmt_tmr4;
	ushort	cpmt_trr3;
	ushort	cpmt_trr4;
	ushort	cpmt_tcr3;
	ushort	cpmt_tcr4;
	ushort	cpmt_tcn3;
	ushort	cpmt_tcn4;
	ushort	cpmt_ter1;
	ushort	cpmt_ter2;
	ushort	cpmt_ter3;
	ushort	cpmt_ter4;
	char	res3[584];
} cpmtimer8260_t;

/* DMA control/status registers.
*/
typedef struct sdma_csr {
	char	res0[24];
	u_char	sdma_sdsr;
	char	res1[3];
	u_char	sdma_sdmr;
	char	res2[3];
	u_char	sdma_idsr1;
	char	res3[3];
	u_char	sdma_idmr1;
	char	res4[3];
	u_char	sdma_idsr2;
	char	res5[3];
	u_char	sdma_idmr2;
	char	res6[3];
	u_char	sdma_idsr3;
	char	res7[3];
	u_char	sdma_idmr3;
	char	res8[3];
	u_char	sdma_idsr4;
	char	res9[3];
	u_char	sdma_idmr4;
	char	res10[707];
} sdma8260_t;

/* Fast controllers
*/
typedef struct fcc {
	uint	fcc_gfmr;
	uint	fcc_fpsmr;
	ushort	fcc_ftodr;
	char	res1[2];
	ushort	fcc_fdsr;
	char	res2[2];
	ushort	fcc_fcce;
	char	res3[2];
	ushort	fcc_fccm;
	char	res4[2];
	u_char	fcc_fccs;
	char	res5[3];
	u_char	fcc_ftirr_phy[4];
} fcc_t;

/* Fast controllers continued
 */
typedef struct fcc_c {
	uint	fcc_firper;
	uint	fcc_firer;
	uint	fcc_firsr_hi;
	uint	fcc_firsr_lo;
	u_char	fcc_gfemr;
	char	res1[15];
} fcc_c_t;

/* TC Layer
 */
typedef struct tclayer {
	ushort	tc_tcmode;
	ushort	tc_cdsmr;
	ushort	tc_tcer;
	ushort	tc_rcc;
	ushort	tc_tcmr;
	ushort	tc_fcc;
	ushort	tc_ccc;
	ushort	tc_icc;
	ushort	tc_tcc;
	ushort	tc_ecc;
	char	res1[12];
} tclayer_t;

/* I2C
*/
typedef struct i2c {
	u_char	i2c_i2mod;
	char	res1[3];
	u_char	i2c_i2add;
	char	res2[3];
	u_char	i2c_i2brg;
	char	res3[3];
	u_char	i2c_i2com;
	char	res4[3];
	u_char	i2c_i2cer;
	char	res5[3];
	u_char	i2c_i2cmr;
	char	res6[331];
} i2c8260_t;

typedef struct scc {		/* Serial communication channels */
	uint	scc_gsmrl;
	uint	scc_gsmrh;
	ushort	scc_psmr;
	char	res1[2];
	ushort	scc_todr;
	ushort	scc_dsr;
	ushort	scc_scce;
	char	res2[2];
	ushort	scc_sccm;
	char	res3;
	u_char	scc_sccs;
	char	res4[8];
} scc_t;

typedef struct smc {		/* Serial management channels */
	char	res1[2];
	ushort	smc_smcmr;
	char	res2[2];
	u_char	smc_smce;
	char	res3[3];
	u_char	smc_smcm;
	char	res4[5];
} smc_t;

/* Serial Peripheral Interface.
*/
typedef struct im_spi {
	ushort	spi_spmode;
	char	res1[4];
	u_char	spi_spie;
	char	res2[3];
	u_char	spi_spim;
	char	res3[2];
	u_char	spi_spcom;
	char	res4[82];
} im_spi_t;

/* CPM Mux.
*/
typedef struct cpmux {
	u_char	cmx_si1cr;
	char	res1;
	u_char	cmx_si2cr;
	char	res2;
	uint	cmx_fcr;
	uint	cmx_scr;
	u_char	cmx_smr;
	char	res3;
	ushort	cmx_uar;
	char	res4[16];
} cpmux_t;

/* SIRAM control
*/
typedef struct siram {
	ushort	si_amr;
	ushort	si_bmr;
	ushort	si_cmr;
	ushort	si_dmr;
	u_char	si_gmr;
	char	res1;
	u_char	si_cmdr;
	char	res2;
	u_char	si_str;
	char	res3;
	ushort	si_rsr;
} siramctl_t;

typedef struct mcc {
	ushort	mcc_mcce;
	char	res1[2];
	ushort	mcc_mccm;
	char	res2[2];
	u_char	mcc_mccf;
	char	res3[7];
} mcc_t;

typedef struct comm_proc {
	uint	cp_cpcr;
	uint	cp_rccr;
	char	res1[14];
	ushort	cp_rter;
	char	res2[2];
	ushort	cp_rtmr;
	ushort	cp_rtscr;
	char	res3[2];
	uint	cp_rtsr;
	char	res4[12];
} cpm8260_t;

/* ...and the whole thing wrapped up....
*/
typedef struct immap {
	/* Some references are into the unique and known dpram spaces,
	 * others are from the generic base.
	 */
	union {
		struct {
			u_char		im_dpram1[16 * 1024];
			char		res1[16 * 1024];
			u_char		im_dpram2[4 * 1024];
			char		res2[8 * 1024];
			u_char		im_dpram3[4 * 1024];
			char		res3[16 * 1024];
		};
		u8	im_dprambase[64 * 1024];
		u16	im_dprambase16[32 * 1024];
	};

	sysconf8260_t	im_siu_conf;	/* SIU Configuration */
	memctl8260_t	im_memctl;	/* Memory Controller */
	sit8260_t	im_sit;		/* System Integration Timers */
	pci8260_t	im_pci;		/* PCI Configuration */
	intctl8260_t	im_intctl;	/* Interrupt Controller */
	car8260_t	im_clkrst;	/* Clocks and reset */
	iop8260_t	im_ioport;	/* IO Port control/status */
	cpmtimer8260_t	im_cpmtimer;	/* CPM timers */
	sdma8260_t	im_sdma;	/* SDMA control/status */

	fcc_t		im_fcc[3];	/* Three FCCs */

	char		res4[32];
	fcc_c_t		im_fcc_c[3];	/* Continued FCCs */
	char		res4a[32];

	tclayer_t	im_tclayer[8];	/* Eight TCLayers */
	ushort		tc_tcgsr;
	ushort		tc_tcger;

	/* First set of baud rate generators.
	*/
	char		res4b[236];
	uint		im_brgc5;
	uint		im_brgc6;
	uint		im_brgc7;
	uint		im_brgc8;

	char		res5[608];

	i2c8260_t	im_i2c;		/* I2C control/status */
	cpm8260_t	im_cpm;		/* Communication processor */

	/* Second set of baud rate generators.
	*/
	uint		im_brgc1;
	uint		im_brgc2;
	uint		im_brgc3;
	uint		im_brgc4;

	scc_t		im_scc[4];	/* Four SCCs */
	smc_t		im_smc[2];	/* Couple of SMCs */
	im_spi_t	im_spi;		/* A SPI */
	cpmux_t		im_cpmux;	/* CPM clock route mux */
	siramctl_t	im_siramctl1;	/* First SI RAM Control */
	mcc_t		im_mcc1;	/* First MCC */
	siramctl_t	im_siramctl2;	/* Second SI RAM Control */
	mcc_t		im_mcc2;	/* Second MCC */

	char		res6[1184];

	ushort		im_si1txram[256];
	char		res7[512];
	ushort		im_si1rxram[256];
	char		res8[512];
	ushort		im_si2txram[256];
	char		res9[512];
	ushort		im_si2rxram[256];
	char		res10[512];
	char		res11[4096];
} immap_t;

#endif /* __IMMAP_82XX__ */
