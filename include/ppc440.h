/*----------------------------------------------------------------------------+
|
|	This source code has been made available to you by IBM on an AS-IS
|	basis.	Anyone receiving this source is licensed under IBM
|	copyrights to use it in any way he or she deems fit, including
|	copying it, modifying it, compiling it, and redistributing it either
|	with or without modifications.	No license under IBM patents or
|	patent applications is to be implied by the copyright license.
|
|	Any user of this software should understand that IBM cannot provide
|	technical support for this software and will not be responsible for
|	any consequences resulting from the use of this software.
|
|	Any person who transfers this source code or any derivative work
|	must include the IBM copyright notice, this paragraph, and the
|	preceding two paragraphs in the transferred software.
|
|	COPYRIGHT   I B M   CORPORATION 1999
|	LICENSED MATERIAL  -  PROGRAM PROPERTY OF I B M
+----------------------------------------------------------------------------*/

#ifndef __PPC440_H__
#define __PPC440_H__

/*--------------------------------------------------------------------- */
/* Special Purpose Registers						*/
/*--------------------------------------------------------------------- */
#define	 dec	0x016	/* decrementer */
#define	 srr0	0x01a	/* save/restore register 0 */
#define	 srr1	0x01b	/* save/restore register 1 */
#define	 pid	0x030	/* process id */
#define	 decar	0x036	/* decrementer auto-reload */
#define	 csrr0	0x03a	/* critical save/restore register 0 */
#define	 csrr1	0x03b	/* critical save/restore register 1 */
#define	 dear	0x03d	/* data exception address register */
#define	 esr	0x03e	/* exception syndrome register */
#define	 ivpr	0x03f	/* interrupt prefix register */
#define	 usprg0 0x100	/* user special purpose register general 0 */
#define	 usprg1 0x110	/* user special purpose register general 1 */
#define	 sprg1	0x111	/* special purpose register general 1 */
#define	 sprg2	0x112	/* special purpose register general 2 */
#define	 sprg3	0x113	/* special purpose register general 3 */
#define	 sprg4	0x114	/* special purpose register general 4 */
#define	 sprg5	0x115	/* special purpose register general 5 */
#define	 sprg6	0x116	/* special purpose register general 6 */
#define	 sprg7	0x117	/* special purpose register general 7 */
#define	 tbl	0x11c	/* time base lower (supervisor)*/
#define	 tbu	0x11d	/* time base upper (supervisor)*/
#define	 pir	0x11e	/* processor id register */
/*#define  pvr	0x11f	 processor version register */
#define	 dbsr	0x130	/* debug status register */
#define	 dbcr0	0x134	/* debug control register 0 */
#define	 dbcr1	0x135	/* debug control register 1 */
#define	 dbcr2	0x136	/* debug control register 2 */
#define	 iac1	0x138	/* instruction address compare 1 */
#define	 iac2	0x139	/* instruction address compare 2 */
#define	 iac3	0x13a	/* instruction address compare 3 */
#define	 iac4	0x13b	/* instruction address compare 4 */
#define	 dac1	0x13c	/* data address compare 1 */
#define	 dac2	0x13d	/* data address compare 2 */
#define	 dvc1	0x13e	/* data value compare 1 */
#define	 dvc2	0x13f	/* data value compare 2 */
#define	 tsr	0x150	/* timer status register */
#define	 tcr	0x154	/* timer control register */
#define	 ivor0	0x190	/* interrupt vector offset register 0 */
#define	 ivor1	0x191	/* interrupt vector offset register 1 */
#define	 ivor2	0x192	/* interrupt vector offset register 2 */
#define	 ivor3	0x193	/* interrupt vector offset register 3 */
#define	 ivor4	0x194	/* interrupt vector offset register 4 */
#define	 ivor5	0x195	/* interrupt vector offset register 5 */
#define	 ivor6	0x196	/* interrupt vector offset register 6 */
#define	 ivor7	0x197	/* interrupt vector offset register 7 */
#define	 ivor8	0x198	/* interrupt vector offset register 8 */
#define	 ivor9	0x199	/* interrupt vector offset register 9 */
#define	 ivor10 0x19a	/* interrupt vector offset register 10 */
#define	 ivor11 0x19b	/* interrupt vector offset register 11 */
#define	 ivor12 0x19c	/* interrupt vector offset register 12 */
#define	 ivor13 0x19d	/* interrupt vector offset register 13 */
#define	 ivor14 0x19e	/* interrupt vector offset register 14 */
#define	 ivor15 0x19f	/* interrupt vector offset register 15 */
#if defined(CONFIG_440GX) || defined(CONFIG_440EP) || defined(CONFIG_440GR) || defined(CONFIG_440SP)
#define	 mcsrr0 0x23a	/* machine check save/restore register 0 */
#define	 mcsrr1 0x23b	/* mahcine check save/restore register 1 */
#define	 mcsr	0x23c	/* machine check status register */
#endif
#define	 inv0	0x370	/* instruction cache normal victim 0 */
#define	 inv1	0x371	/* instruction cache normal victim 1 */
#define	 inv2	0x372	/* instruction cache normal victim 2 */
#define	 inv3	0x373	/* instruction cache normal victim 3 */
#define	 itv0	0x374	/* instruction cache transient victim 0 */
#define	 itv1	0x375	/* instruction cache transient victim 1 */
#define	 itv2	0x376	/* instruction cache transient victim 2 */
#define	 itv3	0x377	/* instruction cache transient victim 3 */
#define	 dnv0	0x390	/* data cache normal victim 0 */
#define	 dnv1	0x391	/* data cache normal victim 1 */
#define	 dnv2	0x392	/* data cache normal victim 2 */
#define	 dnv3	0x393	/* data cache normal victim 3 */
#define	 dtv0	0x394	/* data cache transient victim 0 */
#define	 dtv1	0x395	/* data cache transient victim 1 */
#define	 dtv2	0x396	/* data cache transient victim 2 */
#define	 dtv3	0x397	/* data cache transient victim 3 */
#define	 dvlim	0x398	/* data cache victim limit */
#define	 ivlim	0x399	/* instruction cache victim limit */
#define	 rstcfg 0x39b	/* reset configuration */
#define	 dcdbtrl 0x39c	/* data cache debug tag register low */
#define	 dcdbtrh 0x39d	/* data cache debug tag register high */
#define	 icdbtrl 0x39e	/* instruction cache debug tag register low */
#define	 icdbtrh 0x39f	/* instruction cache debug tag register high */
#define	 mmucr	0x3b2	/* mmu control register */
#define	 ccr0	0x3b3	/* core configuration register 0 */
#define  ccr1  	0x378	/* core configuration for 440x5 only */
#define	 icdbdr 0x3d3	/* instruction cache debug data register */
#define	 dbdr	0x3f3	/* debug data register */

/******************************************************************************
 * DCRs & Related
 ******************************************************************************/

/*-----------------------------------------------------------------------------
 | Clocking Controller
 +----------------------------------------------------------------------------*/
#define CLOCKING_DCR_BASE 0x0c
#define clkcfga	 (CLOCKING_DCR_BASE+0x0)
#define clkcfgd	 (CLOCKING_DCR_BASE+0x1)

/* values for clkcfga register - indirect addressing of these regs */
#define clk_clkukpd	0x0020
#define clk_pllc	0x0040
#define clk_plld	0x0060
#define clk_primad	0x0080
#define clk_primbd	0x00a0
#define clk_opbd	0x00c0
#define clk_perd	0x00e0
#define clk_mald	0x0100
#define clk_spcid   	0x0120
#define clk_icfg	0x0140

/* 440gx sdr register definations */
#define SDR_DCR_BASE	0x0e
#define sdrcfga		(SDR_DCR_BASE+0x0)
#define sdrcfgd		(SDR_DCR_BASE+0x1)
#define sdr_sdstp0	0x0020	    /* */
#define sdr_sdstp1	0x0021	    /* */
#define sdr_pinstp	0x0040
#define sdr_sdcs	0x0060
#define sdr_ecid0	0x0080
#define sdr_ecid1	0x0081
#define sdr_ecid2	0x0082
#define sdr_jtag	0x00c0
#define sdr_ddrdl	0x00e0
#define sdr_ebc		0x0100
#define sdr_uart0	0x0120	/* UART0 Config */
#define sdr_uart1	0x0121	/* UART1 Config */
#define sdr_uart2	0x0122	/* UART2 Config */
#define sdr_uart3	0x0123	/* UART3 Config */
#define sdr_cp440	0x0180
#define sdr_xcr		0x01c0
#define sdr_xpllc	0x01c1
#define sdr_xplld	0x01c2
#define sdr_srst	0x0200
#define sdr_slpipe	0x0220
#define sdr_amp0        0x0240  /* Override PLB4 prioritiy for up to 8 masters */
#define sdr_amp1        0x0241  /* Override PLB3 prioritiy for up to 8 masters */
#define sdr_mirq0	0x0260
#define sdr_mirq1	0x0261
#define sdr_maltbl	0x0280
#define sdr_malrbl	0x02a0
#define sdr_maltbs	0x02c0
#define sdr_malrbs	0x02e0
#define sdr_pci0        0x0300
#define sdr_usb0        0x0320
#define sdr_cust0	0x4000
#define sdr_sdstp2	0x4001
#define sdr_cust1	0x4002
#define sdr_sdstp3	0x4003
#define sdr_pfc0	0x4100	/* Pin Function 0 */
#define sdr_pfc1	0x4101	/* Pin Function 1 */
#define sdr_plbtr	0x4200
#define sdr_mfr		0x4300	/* SDR0_MFR reg */

/*-----------------------------------------------------------------------------
 | SDRAM Controller
 +----------------------------------------------------------------------------*/
#define SDRAM_DCR_BASE 0x10
#define memcfga	 (SDRAM_DCR_BASE+0x0)	/* Memory configuration address reg */
#define memcfgd	 (SDRAM_DCR_BASE+0x1)	/* Memory configuration data reg    */

/* values for memcfga register - indirect addressing of these regs	    */
#define mem_besr0_clr	0x0000	/* bus error status reg 0 (clr)		    */
#define mem_besr0_set	0x0004	/* bus error status reg 0 (set)		    */
#define mem_besr1_clr	0x0008	/* bus error status reg 1 (clr)		    */
#define mem_besr1_set	0x000c	/* bus error status reg 1 (set)		    */
#define mem_bear	0x0010	/* bus error address reg		    */
#define mem_mirq_clr	0x0011	/* bus master interrupt (clr)		    */
#define mem_mirq_set	0x0012	/* bus master interrupt (set)		    */
#define mem_slio	0x0018	/* ddr sdram slave interface options	    */
#define mem_cfg0	0x0020	/* ddr sdram options 0			    */
#define mem_cfg1	0x0021	/* ddr sdram options 1			    */
#define mem_devopt	0x0022	/* ddr sdram device options		    */
#define mem_mcsts	0x0024	/* memory controller status		    */
#define mem_rtr		0x0030	/* refresh timer register		    */
#define mem_pmit	0x0034	/* power management idle timer		    */
#define mem_uabba	0x0038	/* plb UABus base address		    */
#define mem_b0cr	0x0040	/* ddr sdram bank 0 configuration	    */
#define mem_b1cr	0x0044	/* ddr sdram bank 1 configuration	    */
#define mem_b2cr	0x0048	/* ddr sdram bank 2 configuration	    */
#define mem_b3cr	0x004c	/* ddr sdram bank 3 configuration	    */
#define mem_tr0		0x0080	/* sdram timing register 0		    */
#define mem_tr1		0x0081	/* sdram timing register 1		    */
#define mem_clktr	0x0082	/* ddr clock timing register		    */
#define mem_wddctr	0x0083	/* write data/dm/dqs clock timing reg	    */
#define mem_dlycal	0x0084	/* delay line calibration register	    */
#define mem_eccesr	0x0098	/* ECC error status			    */

/*-----------------------------------------------------------------------------
 | External Bus Controller
 +----------------------------------------------------------------------------*/
#define EBC_DCR_BASE 0x12
#define ebccfga (EBC_DCR_BASE+0x0)   /* External bus controller addr reg     */
#define ebccfgd (EBC_DCR_BASE+0x1)   /* External bus controller data reg     */
/* values for ebccfga register - indirect addressing of these regs */
#define pb0cr		0x00	/* periph bank 0 config reg		*/
#define pb1cr		0x01	/* periph bank 1 config reg		*/
#define pb2cr		0x02	/* periph bank 2 config reg		*/
#define pb3cr		0x03	/* periph bank 3 config reg		*/
#define pb4cr		0x04	/* periph bank 4 config reg		*/
#define pb5cr		0x05	/* periph bank 5 config reg		*/
#define pb6cr		0x06	/* periph bank 6 config reg		*/
#define pb7cr		0x07	/* periph bank 7 config reg		*/
#define pb0ap		0x10	/* periph bank 0 access parameters	*/
#define pb1ap		0x11	/* periph bank 1 access parameters	*/
#define pb2ap		0x12	/* periph bank 2 access parameters	*/
#define pb3ap		0x13	/* periph bank 3 access parameters	*/
#define pb4ap		0x14	/* periph bank 4 access parameters	*/
#define pb5ap		0x15	/* periph bank 5 access parameters	*/
#define pb6ap		0x16	/* periph bank 6 access parameters	*/
#define pb7ap		0x17	/* periph bank 7 access parameters	*/
#define pbear		0x20	/* periph bus error addr reg		*/
#define pbesr		0x21	/* periph bus error status reg		*/
#define xbcfg		0x23	/* external bus configuration reg	*/
#define xbcid		0x24	/* external bus core id reg		*/

#if defined(CONFIG_440EP) || defined(CONFIG_440GR)

/* PLB4 to PLB3 Bridge OUT */
#define P4P3_DCR_BASE           0x020
#define p4p3_esr0_read          (P4P3_DCR_BASE+0x0)
#define p4p3_esr0_write         (P4P3_DCR_BASE+0x1)
#define p4p3_eadr               (P4P3_DCR_BASE+0x2)
#define p4p3_euadr              (P4P3_DCR_BASE+0x3)
#define p4p3_esr1_read          (P4P3_DCR_BASE+0x4)
#define p4p3_esr1_write         (P4P3_DCR_BASE+0x5)
#define p4p3_confg              (P4P3_DCR_BASE+0x6)
#define p4p3_pic                (P4P3_DCR_BASE+0x7)
#define p4p3_peir               (P4P3_DCR_BASE+0x8)
#define p4p3_rev                (P4P3_DCR_BASE+0xA)

/* PLB3 to PLB4 Bridge IN */
#define P3P4_DCR_BASE           0x030
#define p3p4_esr0_read          (P3P4_DCR_BASE+0x0)
#define p3p4_esr0_write         (P3P4_DCR_BASE+0x1)
#define p3p4_eadr               (P3P4_DCR_BASE+0x2)
#define p3p4_euadr              (P3P4_DCR_BASE+0x3)
#define p3p4_esr1_read          (P3P4_DCR_BASE+0x4)
#define p3p4_esr1_write         (P3P4_DCR_BASE+0x5)
#define p3p4_confg              (P3P4_DCR_BASE+0x6)
#define p3p4_pic                (P3P4_DCR_BASE+0x7)
#define p3p4_peir               (P3P4_DCR_BASE+0x8)
#define p3p4_rev                (P3P4_DCR_BASE+0xA)

/* PLB3 Arbiter */
#define PLB3_DCR_BASE           0x070
#define plb3_revid              (PLB3_DCR_BASE+0x2)
#define plb3_besr               (PLB3_DCR_BASE+0x3)
#define plb3_bear               (PLB3_DCR_BASE+0x6)
#define plb3_acr                (PLB3_DCR_BASE+0x7)

/* PLB4 Arbiter - PowerPC440EP Pass1 */
#define PLB4_DCR_BASE           0x080
#define plb4_revid              (PLB4_DCR_BASE+0x2)
#define plb4_acr                (PLB4_DCR_BASE+0x3)
#define plb4_besr               (PLB4_DCR_BASE+0x4)
#define plb4_bearl              (PLB4_DCR_BASE+0x6)
#define plb4_bearh              (PLB4_DCR_BASE+0x7)

/* Nebula PLB4 Arbiter - PowerPC440EP */
#define PLB_ARBITER_BASE   0x80

#define plb0_revid                (PLB_ARBITER_BASE+ 0x00)
#define plb0_acr                  (PLB_ARBITER_BASE+ 0x01)
#define   plb0_acr_ppm_mask             0xF0000000
#define   plb0_acr_ppm_fixed            0x00000000
#define   plb0_acr_ppm_fair             0xD0000000
#define   plb0_acr_hbu_mask             0x08000000
#define   plb0_acr_hbu_disabled         0x00000000
#define   plb0_acr_hbu_enabled          0x08000000
#define   plb0_acr_rdp_mask             0x06000000
#define   plb0_acr_rdp_disabled         0x00000000
#define   plb0_acr_rdp_2deep            0x02000000
#define   plb0_acr_rdp_3deep            0x04000000
#define   plb0_acr_rdp_4deep            0x06000000
#define   plb0_acr_wrp_mask             0x01000000
#define   plb0_acr_wrp_disabled         0x00000000
#define   plb0_acr_wrp_2deep            0x01000000

#define plb0_besrl                (PLB_ARBITER_BASE+ 0x02)
#define plb0_besrh                (PLB_ARBITER_BASE+ 0x03)
#define plb0_bearl                (PLB_ARBITER_BASE+ 0x04)
#define plb0_bearh                (PLB_ARBITER_BASE+ 0x05)
#define plb0_ccr                  (PLB_ARBITER_BASE+ 0x08)

#define plb1_acr                  (PLB_ARBITER_BASE+ 0x09)
#define   plb1_acr_ppm_mask             0xF0000000
#define   plb1_acr_ppm_fixed            0x00000000
#define   plb1_acr_ppm_fair             0xD0000000
#define   plb1_acr_hbu_mask             0x08000000
#define   plb1_acr_hbu_disabled         0x00000000
#define   plb1_acr_hbu_enabled          0x08000000
#define   plb1_acr_rdp_mask             0x06000000
#define   plb1_acr_rdp_disabled         0x00000000
#define   plb1_acr_rdp_2deep            0x02000000
#define   plb1_acr_rdp_3deep            0x04000000
#define   plb1_acr_rdp_4deep            0x06000000
#define   plb1_acr_wrp_mask             0x01000000
#define   plb1_acr_wrp_disabled         0x00000000
#define   plb1_acr_wrp_2deep            0x01000000

#define plb1_besrl                (PLB_ARBITER_BASE+ 0x0A)
#define plb1_besrh                (PLB_ARBITER_BASE+ 0x0B)
#define plb1_bearl                (PLB_ARBITER_BASE+ 0x0C)
#define plb1_bearh                (PLB_ARBITER_BASE+ 0x0D)

/* Pin Function Control Register 1 */
#define SDR0_PFC1                    0x4101
#define   SDR0_PFC1_U1ME_MASK         0x02000000    /* UART1 Mode Enable */
#define   SDR0_PFC1_U1ME_DSR_DTR      0x00000000      /* UART1 in DSR/DTR Mode */
#define   SDR0_PFC1_U1ME_CTS_RTS      0x02000000      /* UART1 in CTS/RTS Mode */
#define   SDR0_PFC1_U0ME_MASK         0x00080000    /* UART0 Mode Enable */
#define   SDR0_PFC1_U0ME_DSR_DTR      0x00000000      /* UART0 in DSR/DTR Mode */
#define   SDR0_PFC1_U0ME_CTS_RTS      0x00080000      /* UART0 in CTS/RTS Mode */
#define   SDR0_PFC1_U0IM_MASK         0x00040000    /* UART0 Interface Mode */
#define   SDR0_PFC1_U0IM_8PINS        0x00000000      /* UART0 Interface Mode 8 pins */
#define   SDR0_PFC1_U0IM_4PINS        0x00040000      /* UART0 Interface Mode 4 pins */
#define   SDR0_PFC1_SIS_MASK          0x00020000    /* SCP or IIC1 Selection */
#define   SDR0_PFC1_SIS_SCP_SEL       0x00000000      /* SCP Selected */
#define   SDR0_PFC1_SIS_IIC1_SEL      0x00020000      /* IIC1 Selected */
#define   SDR0_PFC1_UES_MASK          0x00010000    /* USB2D_RX_Active / EBC_Hold Req Selection */
#define   SDR0_PFC1_UES_USB2D_SEL     0x00000000      /* USB2D_RX_Active Selected */
#define   SDR0_PFC1_UES_EBCHR_SEL     0x00010000      /* EBC_Hold Req Selected */
#define   SDR0_PFC1_DIS_MASK          0x00008000    /* DMA_Req(1) / UIC_IRQ(5) Selection */
#define   SDR0_PFC1_DIS_DMAR_SEL      0x00000000      /* DMA_Req(1) Selected */
#define   SDR0_PFC1_DIS_UICIRQ5_SEL   0x00008000      /* UIC_IRQ(5) Selected */
#define   SDR0_PFC1_ERE_MASK          0x00004000    /* EBC Mast.Ext.Req.En./GPIO0(27) Selection */
#define   SDR0_PFC1_ERE_EXTR_SEL      0x00000000      /* EBC Mast.Ext.Req.En. Selected */
#define   SDR0_PFC1_ERE_GPIO0_27_SEL  0x00004000      /* GPIO0(27) Selected */
#define   SDR0_PFC1_UPR_MASK          0x00002000    /* USB2 Device Packet Reject Selection */
#define   SDR0_PFC1_UPR_DISABLE       0x00000000      /* USB2 Device Packet Reject Disable */
#define   SDR0_PFC1_UPR_ENABLE        0x00002000      /* USB2 Device Packet Reject Enable */

#define   SDR0_PFC1_PLB_PME_MASK      0x00001000    /* PLB3/PLB4 Perf. Monitor En. Selection */
#define   SDR0_PFC1_PLB_PME_PLB3_SEL  0x00000000      /* PLB3 Performance Monitor Enable */
#define   SDR0_PFC1_PLB_PME_PLB4_SEL  0x00001000      /* PLB3 Performance Monitor Enable */
#define   SDR0_PFC1_GFGGI_MASK        0x0000000F    /* GPT Frequency Generation Gated In */

/* USB Control Register */
#define SDR0_USB0                    0x0320
#define   SDR0_USB0_USB_DEVSEL_MASK   0x00000002    /* USB Device Selection */
#define   SDR0_USB0_USB20D_DEVSEL     0x00000000      /* USB2.0 Device Selected */
#define   SDR0_USB0_USB11D_DEVSEL     0x00000002      /* USB1.1 Device Selected */
#define   SDR0_USB0_LEEN_MASK         0x00000001    /* Little Endian selection */
#define   SDR0_USB0_LEEN_DISABLE      0x00000000      /* Little Endian Disable */
#define   SDR0_USB0_LEEN_ENABLE       0x00000001      /* Little Endian Enable */

/* CUST0 Customer Configuration Register0 */
#define SDR0_CUST0                   0x4000
#define   SDR0_CUST0_MUX_E_N_G_MASK   0xC0000000     /* Mux_Emac_NDFC_GPIO */
#define   SDR0_CUST0_MUX_EMAC_SEL     0x40000000       /* Emac Selection */
#define   SDR0_CUST0_MUX_NDFC_SEL     0x80000000       /* NDFC Selection */
#define   SDR0_CUST0_MUX_GPIO_SEL     0xC0000000       /* GPIO Selection */

#define   SDR0_CUST0_NDFC_EN_MASK     0x20000000     /* NDFC Enable Mask */
#define   SDR0_CUST0_NDFC_ENABLE      0x20000000       /* NDFC Enable */
#define   SDR0_CUST0_NDFC_DISABLE     0x00000000       /* NDFC Disable */

#define   SDR0_CUST0_NDFC_BW_MASK     0x10000000     /* NDFC Boot Width */
#define   SDR0_CUST0_NDFC_BW_16_BIT   0x10000000       /* NDFC Boot Width = 16 Bit */
#define   SDR0_CUST0_NDFC_BW_8_BIT    0x00000000       /* NDFC Boot Width =  8 Bit */

#define   SDR0_CUST0_NDFC_BP_MASK     0x0F000000     /* NDFC Boot Page */
#define   SDR0_CUST0_NDFC_BP_ENCODE(n) ((((unsigned long)(n))&0xF)<<24)
#define   SDR0_CUST0_NDFC_BP_DECODE(n) ((((unsigned long)(n))>>24)&0x0F)

#define   SDR0_CUST0_NDFC_BAC_MASK    0x00C00000     /* NDFC Boot Address Cycle */
#define   SDR0_CUST0_NDFC_BAC_ENCODE(n) ((((unsigned long)(n))&0x3)<<22)
#define   SDR0_CUST0_NDFC_BAC_DECODE(n) ((((unsigned long)(n))>>22)&0x03)

#define   SDR0_CUST0_NDFC_ARE_MASK    0x00200000     /* NDFC Auto Read Enable */
#define   SDR0_CUST0_NDFC_ARE_ENABLE  0x00200000       /* NDFC Auto Read Enable */
#define   SDR0_CUST0_NDFC_ARE_DISABLE 0x00000000       /* NDFC Auto Read Disable */

#define   SDR0_CUST0_NRB_MASK         0x00100000     /* NDFC Ready / Busy */
#define   SDR0_CUST0_NRB_BUSY         0x00100000       /* Busy */
#define   SDR0_CUST0_NRB_READY        0x00000000       /* Ready */

#define   SDR0_CUST0_NDRSC_MASK       0x0000FFF0     /* NDFC Device Reset Count Mask */
#define   SDR0_CUST0_NDRSC_ENCODE(n) ((((unsigned long)(n))&0xFFF)<<4)
#define   SDR0_CUST0_NDRSC_DECODE(n) ((((unsigned long)(n))>>4)&0xFFF)

#define   SDR0_CUST0_CHIPSELGAT_MASK  0x0000000F     /* Chip Select Gating Mask */
#define   SDR0_CUST0_CHIPSELGAT_DIS   0x00000000       /* Chip Select Gating Disable */
#define   SDR0_CUST0_CHIPSELGAT_ENALL 0x0000000F       /* All Chip Select Gating Enable */
#define   SDR0_CUST0_CHIPSELGAT_EN0   0x00000008       /* Chip Select0 Gating Enable */
#define   SDR0_CUST0_CHIPSELGAT_EN1   0x00000004       /* Chip Select1 Gating Enable */
#define   SDR0_CUST0_CHIPSELGAT_EN2   0x00000002       /* Chip Select2 Gating Enable */
#define   SDR0_CUST0_CHIPSELGAT_EN3   0x00000001       /* Chip Select3 Gating Enable */

/* CUST1 Customer Configuration Register1 */
#define   SDR0_CUST1                 0x4002
#define   SDR0_CUST1_NDRSC_MASK       0xFFFF0000     /* NDRSC Device Read Count */
#define   SDR0_CUST1_NDRSC_ENCODE(n) ((((unsigned long)(n))&0xFFFF)<<16)
#define   SDR0_CUST1_NDRSC_DECODE(n) ((((unsigned long)(n))>>16)&0xFFFF)

/* Pin Function Control Register 0 */
#define SDR0_PFC0                    0x4100
#define   SDR0_PFC0_CPU_TR_EN_MASK    0x00000100    /* CPU Trace Enable Mask */
#define   SDR0_PFC0_CPU_TRACE_EN      0x00000100      /* CPU Trace Enable */
#define   SDR0_PFC0_CPU_TRACE_DIS     0x00000100      /* CPU Trace Disable */
#define   SDR0_PFC0_CTE_ENCODE(n)    ((((unsigned long)(n))&0x01)<<8)
#define   SDR0_PFC0_CTE_DECODE(n)    ((((unsigned long)(n))>>8)&0x01)

/* Pin Function Control Register 1 */
#define SDR0_PFC1                    0x4101
#define   SDR0_PFC1_U1ME_MASK         0x02000000    /* UART1 Mode Enable */
#define   SDR0_PFC1_U1ME_DSR_DTR      0x00000000      /* UART1 in DSR/DTR Mode */
#define   SDR0_PFC1_U1ME_CTS_RTS      0x02000000      /* UART1 in CTS/RTS Mode */
#define   SDR0_PFC1_U0ME_MASK         0x00080000    /* UART0 Mode Enable */
#define   SDR0_PFC1_U0ME_DSR_DTR      0x00000000      /* UART0 in DSR/DTR Mode */
#define   SDR0_PFC1_U0ME_CTS_RTS      0x00080000      /* UART0 in CTS/RTS Mode */
#define   SDR0_PFC1_U0IM_MASK         0x00040000    /* UART0 Interface Mode */
#define   SDR0_PFC1_U0IM_8PINS        0x00000000      /* UART0 Interface Mode 8 pins */
#define   SDR0_PFC1_U0IM_4PINS        0x00040000      /* UART0 Interface Mode 4 pins */
#define   SDR0_PFC1_SIS_MASK          0x00020000    /* SCP or IIC1 Selection */
#define   SDR0_PFC1_SIS_SCP_SEL       0x00000000      /* SCP Selected */
#define   SDR0_PFC1_SIS_IIC1_SEL      0x00020000      /* IIC1 Selected */
#define   SDR0_PFC1_UES_MASK          0x00010000    /* USB2D_RX_Active / EBC_Hold Req Selection */
#define   SDR0_PFC1_UES_USB2D_SEL     0x00000000      /* USB2D_RX_Active Selected */
#define   SDR0_PFC1_UES_EBCHR_SEL     0x00010000      /* EBC_Hold Req Selected */
#define   SDR0_PFC1_DIS_MASK          0x00008000    /* DMA_Req(1) / UIC_IRQ(5) Selection */
#define   SDR0_PFC1_DIS_DMAR_SEL      0x00000000      /* DMA_Req(1) Selected */
#define   SDR0_PFC1_DIS_UICIRQ5_SEL   0x00008000      /* UIC_IRQ(5) Selected */
#define   SDR0_PFC1_ERE_MASK          0x00004000    /* EBC Mast.Ext.Req.En./GPIO0(27) Selection */
#define   SDR0_PFC1_ERE_EXTR_SEL      0x00000000      /* EBC Mast.Ext.Req.En. Selected */
#define   SDR0_PFC1_ERE_GPIO0_27_SEL  0x00004000      /* GPIO0(27) Selected */
#define   SDR0_PFC1_UPR_MASK          0x00002000    /* USB2 Device Packet Reject Selection */
#define   SDR0_PFC1_UPR_DISABLE       0x00000000      /* USB2 Device Packet Reject Disable */
#define   SDR0_PFC1_UPR_ENABLE        0x00002000      /* USB2 Device Packet Reject Enable */

#define   SDR0_PFC1_PLB_PME_MASK      0x00001000    /* PLB3/PLB4 Perf. Monitor En. Selection */
#define   SDR0_PFC1_PLB_PME_PLB3_SEL  0x00000000      /* PLB3 Performance Monitor Enable */
#define   SDR0_PFC1_PLB_PME_PLB4_SEL  0x00001000      /* PLB3 Performance Monitor Enable */
#define   SDR0_PFC1_GFGGI_MASK        0x0000000F    /* GPT Frequency Generation Gated In */

/* Miscealleneaous Function Reg. */
#define SDR0_MFR                     0x4300
#define   SDR0_MFR_ETH0_CLK_SEL        0x08000000   /* Ethernet0 Clock Select */
#define   SDR0_MFR_ETH1_CLK_SEL        0x04000000   /* Ethernet1 Clock Select */
#define   SDR0_MFR_ZMII_MODE_MASK      0x03000000   /* ZMII Mode Mask */
#define   SDR0_MFR_ZMII_MODE_MII       0x00000000     /* ZMII Mode MII */
#define   SDR0_MFR_ZMII_MODE_SMII      0x01000000     /* ZMII Mode SMII */
#define   SDR0_MFR_ZMII_MODE_RMII_10M  0x02000000     /* ZMII Mode RMII - 10 Mbs */
#define   SDR0_MFR_ZMII_MODE_RMII_100M 0x03000000     /* ZMII Mode RMII - 100 Mbs */
#define   SDR0_MFR_ZMII_MODE_BIT0      0x02000000     /* ZMII Mode Bit0 */
#define   SDR0_MFR_ZMII_MODE_BIT1      0x01000000     /* ZMII Mode Bit1 */
#define   SDR0_MFR_ZM_ENCODE(n)        ((((unsigned long)(n))&0x3)<<24)
#define   SDR0_MFR_ZM_DECODE(n)        ((((unsigned long)(n))<<24)&0x3)

#define   SDR0_MFR_ERRATA3_EN0         0x00800000
#define   SDR0_MFR_ERRATA3_EN1         0x00400000
#define   SDR0_MFR_PKT_REJ_MASK        0x00300000   /* Pkt Rej. Enable Mask */
#define   SDR0_MFR_PKT_REJ_EN          0x00300000   /* Pkt Rej. Enable on both EMAC3 0-1 */
#define   SDR0_MFR_PKT_REJ_EN0         0x00200000   /* Pkt Rej. Enable on EMAC3(0) */
#define   SDR0_MFR_PKT_REJ_EN1         0x00100000   /* Pkt Rej. Enable on EMAC3(1) */
#define   SDR0_MFR_PKT_REJ_POL         0x00080000   /* Packet Reject Polarity */

#else

/*-----------------------------------------------------------------------------
 | Internal SRAM
 +----------------------------------------------------------------------------*/
#define ISRAM0_DCR_BASE 0x020
#define isram0_sb0cr	(ISRAM0_DCR_BASE+0x00)	/* SRAM bank config 0*/
#define isram0_sb1cr	(ISRAM0_DCR_BASE+0x01)	/* SRAM bank config 1*/
#define isram0_sb2cr	(ISRAM0_DCR_BASE+0x02)	/* SRAM bank config 2*/
#define isram0_sb3cr	(ISRAM0_DCR_BASE+0x03)	/* SRAM bank config 3*/
#define isram0_bear	(ISRAM0_DCR_BASE+0x04)	/* SRAM bus error addr reg */
#define isram0_besr0	(ISRAM0_DCR_BASE+0x05)	/* SRAM bus error status reg 0 */
#define isram0_besr1	(ISRAM0_DCR_BASE+0x06)	/* SRAM bus error status reg 1 */
#define isram0_pmeg	(ISRAM0_DCR_BASE+0x07)	/* SRAM power management */
#define isram0_cid	(ISRAM0_DCR_BASE+0x08)	/* SRAM bus core id reg */
#define isram0_revid	(ISRAM0_DCR_BASE+0x09)	/* SRAM bus revision id reg */
#define isram0_dpc	(ISRAM0_DCR_BASE+0x0a)	/* SRAM data parity check reg */

/*-----------------------------------------------------------------------------
 | L2 Cache
 +----------------------------------------------------------------------------*/
#if defined (CONFIG_440GX) || defined(CONFIG_440SP)
#define L2_CACHE_BASE	0x030
#define l2_cache_cfg	(L2_CACHE_BASE+0x00)	/* L2 Cache Config	*/
#define l2_cache_cmd	(L2_CACHE_BASE+0x01)	/* L2 Cache Command	*/
#define l2_cache_addr	(L2_CACHE_BASE+0x02)	/* L2 Cache Address	*/
#define l2_cache_data	(L2_CACHE_BASE+0x03)	/* L2 Cache Data	*/
#define l2_cache_stat	(L2_CACHE_BASE+0x04)	/* L2 Cache Status	*/
#define l2_cache_cver	(L2_CACHE_BASE+0x05)	/* L2 Cache Revision ID */
#define l2_cache_snp0	(L2_CACHE_BASE+0x06)	/* L2 Cache Snoop reg 0 */
#define l2_cache_snp1	(L2_CACHE_BASE+0x07)	/* L2 Cache Snoop reg 1 */

#endif /* CONFIG_440GX */
#endif /* !CONFIG_440EP !CONFIG_440GR*/

/*-----------------------------------------------------------------------------
 | On-Chip Buses
 +----------------------------------------------------------------------------*/
/* TODO: as needed */

/*-----------------------------------------------------------------------------
 | Clocking, Power Management and Chip Control
 +----------------------------------------------------------------------------*/
#define CNTRL_DCR_BASE 0x0b0
#if defined(CONFIG_440GX) || defined(CONFIG_440SP)
#define cpc0_er		(CNTRL_DCR_BASE+0x00)	/* CPM enable register		*/
#define cpc0_fr		(CNTRL_DCR_BASE+0x01)	/* CPM force register		*/
#define cpc0_sr		(CNTRL_DCR_BASE+0x02)	/* CPM status register		*/
#else
#define cpc0_sr		(CNTRL_DCR_BASE+0x00)	/* CPM status register		*/
#define cpc0_er		(CNTRL_DCR_BASE+0x01)	/* CPM enable register		*/
#define cpc0_fr		(CNTRL_DCR_BASE+0x02)	/* CPM force register		*/
#endif

#define cpc0_sys0	(CNTRL_DCR_BASE+0x30)	/* System configuration reg 0	*/
#define cpc0_sys1	(CNTRL_DCR_BASE+0x31)	/* System configuration reg 1	*/
#define cpc0_cust0	(CNTRL_DCR_BASE+0x32)	/* Customer configuration reg 0 */
#define cpc0_cust1	(CNTRL_DCR_BASE+0x33)	/* Customer configuration reg 1 */

#define cpc0_strp0	(CNTRL_DCR_BASE+0x34)	/* Power-on config reg 0 (RO)	*/
#define cpc0_strp1	(CNTRL_DCR_BASE+0x35)	/* Power-on config reg 1 (RO)	*/
#define cpc0_strp2	(CNTRL_DCR_BASE+0x36)	/* Power-on config reg 2 (RO)	*/
#define cpc0_strp3	(CNTRL_DCR_BASE+0x37)	/* Power-on config reg 3 (RO)	*/

#define cpc0_gpio	(CNTRL_DCR_BASE+0x38)	/* GPIO config reg (440GP)	*/

#define cntrl0		(CNTRL_DCR_BASE+0x3b)	/* Control 0 register		*/
#define cntrl1		(CNTRL_DCR_BASE+0x3a)	/* Control 1 register		*/

/*-----------------------------------------------------------------------------
 | Universal interrupt controller
 +----------------------------------------------------------------------------*/
#define UIC0_DCR_BASE 0xc0
#define uic0sr	(UIC0_DCR_BASE+0x0)   /* UIC0 status			   */
#define uic0er	(UIC0_DCR_BASE+0x2)   /* UIC0 enable			   */
#define uic0cr	(UIC0_DCR_BASE+0x3)   /* UIC0 critical			   */
#define uic0pr	(UIC0_DCR_BASE+0x4)   /* UIC0 polarity			   */
#define uic0tr	(UIC0_DCR_BASE+0x5)   /* UIC0 triggering		   */
#define uic0msr (UIC0_DCR_BASE+0x6)   /* UIC0 masked status		   */
#define uic0vr	(UIC0_DCR_BASE+0x7)   /* UIC0 vector			   */
#define uic0vcr (UIC0_DCR_BASE+0x8)   /* UIC0 vector configuration	   */

#define UIC1_DCR_BASE 0xd0
#define uic1sr	(UIC1_DCR_BASE+0x0)   /* UIC1 status			   */
#define uic1er	(UIC1_DCR_BASE+0x2)   /* UIC1 enable			   */
#define uic1cr	(UIC1_DCR_BASE+0x3)   /* UIC1 critical			   */
#define uic1pr	(UIC1_DCR_BASE+0x4)   /* UIC1 polarity			   */
#define uic1tr	(UIC1_DCR_BASE+0x5)   /* UIC1 triggering		   */
#define uic1msr (UIC1_DCR_BASE+0x6)   /* UIC1 masked status		   */
#define uic1vr	(UIC1_DCR_BASE+0x7)   /* UIC1 vector			   */
#define uic1vcr (UIC1_DCR_BASE+0x8)   /* UIC1 vector configuration	   */

#if defined(CONFIG_440GX)
#define UIC2_DCR_BASE 0x210
#define uic2sr	(UIC2_DCR_BASE+0x0)   /* UIC2 status			   */
#define uic2er	(UIC2_DCR_BASE+0x2)   /* UIC2 enable			   */
#define uic2cr	(UIC2_DCR_BASE+0x3)   /* UIC2 critical			   */
#define uic2pr	(UIC2_DCR_BASE+0x4)   /* UIC2 polarity			   */
#define uic2tr	(UIC2_DCR_BASE+0x5)   /* UIC2 triggering		   */
#define uic2msr (UIC2_DCR_BASE+0x6)   /* UIC2 masked status		   */
#define uic2vr	(UIC2_DCR_BASE+0x7)   /* UIC2 vector			   */
#define uic2vcr (UIC2_DCR_BASE+0x8)   /* UIC2 vector configuration	   */


#define UIC_DCR_BASE 0x200
#define uicb0sr	 (UIC_DCR_BASE+0x0)   /* UIC Base Status Register	   */
#define uicb0er	 (UIC_DCR_BASE+0x2)   /* UIC Base enable		   */
#define uicb0cr	 (UIC_DCR_BASE+0x3)   /* UIC Base critical		   */
#define uicb0pr	 (UIC_DCR_BASE+0x4)   /* UIC Base polarity		   */
#define uicb0tr	 (UIC_DCR_BASE+0x5)   /* UIC Base triggering		   */
#define uicb0msr (UIC_DCR_BASE+0x6)   /* UIC Base masked status		   */
#define uicb0vr	 (UIC_DCR_BASE+0x7)   /* UIC Base vector		   */
#define uicb0vcr (UIC_DCR_BASE+0x8)   /* UIC Base vector configuration	   */
#endif /* CONFIG_440GX */

/* The following is for compatibility with 405 code */
#define uicsr  uic0sr
#define uicer  uic0er
#define uiccr  uic0cr
#define uicpr  uic0pr
#define uictr  uic0tr
#define uicmsr uic0msr
#define uicvr  uic0vr
#define uicvcr uic0vcr

/*-----------------------------------------------------------------------------
 | DMA
 +----------------------------------------------------------------------------*/
#define DMA_DCR_BASE 0x100
#define dmacr0	(DMA_DCR_BASE+0x00)  /* DMA channel control register 0	     */
#define dmact0	(DMA_DCR_BASE+0x01)  /* DMA count register 0		     */
#define dmasah0 (DMA_DCR_BASE+0x02)  /* DMA source address high 0	     */
#define dmasal0 (DMA_DCR_BASE+0x03)  /* DMA source address low 0	     */
#define dmadah0 (DMA_DCR_BASE+0x04)  /* DMA destination address high 0	     */
#define dmadal0 (DMA_DCR_BASE+0x05)  /* DMA destination address low 0	     */
#define dmasgh0 (DMA_DCR_BASE+0x06)  /* DMA scatter/gather desc addr high 0  */
#define dmasgl0 (DMA_DCR_BASE+0x07)  /* DMA scatter/gather desc addr low 0   */
#define dmacr1	(DMA_DCR_BASE+0x08)  /* DMA channel control register 1	     */
#define dmact1	(DMA_DCR_BASE+0x09)  /* DMA count register 1		     */
#define dmasah1 (DMA_DCR_BASE+0x0a)  /* DMA source address high 1	     */
#define dmasal1 (DMA_DCR_BASE+0x0b)  /* DMA source address low 1	     */
#define dmadah1 (DMA_DCR_BASE+0x0c)  /* DMA destination address high 1	     */
#define dmadal1 (DMA_DCR_BASE+0x0d)  /* DMA destination address low 1	     */
#define dmasgh1 (DMA_DCR_BASE+0x0e)  /* DMA scatter/gather desc addr high 1  */
#define dmasgl1 (DMA_DCR_BASE+0x0f)  /* DMA scatter/gather desc addr low 1   */
#define dmacr2	(DMA_DCR_BASE+0x10)  /* DMA channel control register 2	     */
#define dmact2	(DMA_DCR_BASE+0x11)  /* DMA count register 2		     */
#define dmasah2 (DMA_DCR_BASE+0x12)  /* DMA source address high 2	     */
#define dmasal2 (DMA_DCR_BASE+0x13)  /* DMA source address low 2	     */
#define dmadah2 (DMA_DCR_BASE+0x14)  /* DMA destination address high 2	     */
#define dmadal2 (DMA_DCR_BASE+0x15)  /* DMA destination address low 2	     */
#define dmasgh2 (DMA_DCR_BASE+0x16)  /* DMA scatter/gather desc addr high 2  */
#define dmasgl2 (DMA_DCR_BASE+0x17)  /* DMA scatter/gather desc addr low 2   */
#define dmacr3	(DMA_DCR_BASE+0x18)  /* DMA channel control register 2	     */
#define dmact3	(DMA_DCR_BASE+0x19)  /* DMA count register 2		     */
#define dmasah3 (DMA_DCR_BASE+0x1a)  /* DMA source address high 2	     */
#define dmasal3 (DMA_DCR_BASE+0x1b)  /* DMA source address low 2	     */
#define dmadah3 (DMA_DCR_BASE+0x1c)  /* DMA destination address high 2	     */
#define dmadal3 (DMA_DCR_BASE+0x1d)  /* DMA destination address low 2	     */
#define dmasgh3 (DMA_DCR_BASE+0x1e)  /* DMA scatter/gather desc addr high 2  */
#define dmasgl3 (DMA_DCR_BASE+0x1f)  /* DMA scatter/gather desc addr low 2   */
#define dmasr	(DMA_DCR_BASE+0x20)  /* DMA status register		     */
#define dmasgc	(DMA_DCR_BASE+0x23)  /* DMA scatter/gather command register  */
#define dmaslp	(DMA_DCR_BASE+0x25)  /* DMA sleep mode register		     */
#define dmapol	(DMA_DCR_BASE+0x26)  /* DMA polarity configuration register  */

/*-----------------------------------------------------------------------------
 | Memory Access Layer
 +----------------------------------------------------------------------------*/
#define MAL_DCR_BASE 0x180
#define malmcr	    (MAL_DCR_BASE+0x00) /* MAL Config reg		    */
#define malesr	    (MAL_DCR_BASE+0x01) /* Error Status reg (Read/Clear)    */
#define malier	    (MAL_DCR_BASE+0x02) /* Interrupt enable reg		    */
#define maldbr	    (MAL_DCR_BASE+0x03) /* Mal Debug reg (Read only)	    */
#define maltxcasr   (MAL_DCR_BASE+0x04) /* TX Channel active reg (set)	    */
#define maltxcarr   (MAL_DCR_BASE+0x05) /* TX Channel active reg (Reset)    */
#define maltxeobisr (MAL_DCR_BASE+0x06) /* TX End of buffer int status reg  */
#define maltxdeir   (MAL_DCR_BASE+0x07) /* TX Descr. Error Int reg	    */
#define maltxtattrr (MAL_DCR_BASE+0x08) /* TX PLB attribute reg		    */
#define maltxbattr  (MAL_DCR_BASE+0x09) /* TX descriptor base addr reg	    */
#define malrxcasr   (MAL_DCR_BASE+0x10) /* RX Channel active reg (set)	    */
#define malrxcarr   (MAL_DCR_BASE+0x11) /* RX Channel active reg (Reset)    */
#define malrxeobisr (MAL_DCR_BASE+0x12) /* RX End of buffer int status reg  */
#define malrxdeir   (MAL_DCR_BASE+0x13) /* RX Descr. Error Int reg	    */
#define malrxtattrr (MAL_DCR_BASE+0x14) /* RX PLB attribute reg		    */
#define malrxbattr  (MAL_DCR_BASE+0x15) /* RX descriptor base addr reg	    */
#define maltxctp0r  (MAL_DCR_BASE+0x20) /* TX 0 Channel table pointer reg   */
#define maltxctp1r  (MAL_DCR_BASE+0x21) /* TX 1 Channel table pointer reg   */
#define maltxctp2r  (MAL_DCR_BASE+0x22) /* TX 2 Channel table pointer reg   */
#define maltxctp3r  (MAL_DCR_BASE+0x23) /* TX 3 Channel table pointer reg   */
#define malrxctp0r  (MAL_DCR_BASE+0x40) /* RX 0 Channel table pointer reg   */
#define malrxctp1r  (MAL_DCR_BASE+0x41) /* RX 1 Channel table pointer reg   */
#if defined(CONFIG_440GX)
#define malrxctp2r  (MAL_DCR_BASE+0x42) /* RX 0 Channel table pointer reg   */
#define malrxctp3r  (MAL_DCR_BASE+0x43) /* RX 1 Channel table pointer reg   */
#endif /* CONFIG_440GX */
#define malrcbs0    (MAL_DCR_BASE+0x60) /* RX 0 Channel buffer size reg	    */
#define malrcbs1    (MAL_DCR_BASE+0x61) /* RX 1 Channel buffer size reg	    */
#if defined(CONFIG_440GX)
#define malrcbs2    (MAL_DCR_BASE+0x62) /* RX 2 Channel buffer size reg	    */
#define malrcbs3    (MAL_DCR_BASE+0x63) /* RX 3 Channel buffer size reg	    */
#endif /* CONFIG_440GX */


/*---------------------------------------------------------------------------+
|  Universal interrupt controller 0 interrupts (UIC0)
+---------------------------------------------------------------------------*/
#if defined(CONFIG_440SP)
#define UIC_U0		0x80000000	/* UART 0			    */
#define UIC_U1		0x40000000	/* UART 1			    */
#define UIC_IIC0	0x20000000	/* IIC				    */
#define UIC_IIC1	0x10000000	/* IIC				    */
#define UIC_PIM		0x08000000	/* PCI0 inbound message		    */
#define UIC_PCRW	0x04000000	/* PCI0 command write register	    */
#define UIC_PPM		0x02000000	/* PCI0 power management	    */
#define UIC_PVPD	0x01000000	/* PCI0 VPD Access		    */
#define UIC_MSI0	0x00800000	/* PCI0 MSI level 0		    */
#define UIC_P1IM	0x00400000	/* PCI1 Inbound Message		    */
#define UIC_P1CRW	0x00200000	/* PCI1 command write register	    */
#define UIC_P1PM	0x00100000	/* PCI1 power management	    */
#define UIC_P1VPD	0x00080000	/* PCI1 VPD Access		    */
#define UIC_P1MSI0	0x00040000	/* PCI1 MSI level 0		    */
#define UIC_P2IM	0x00020000	/* PCI2 inbound message		    */
#define UIC_P2CRW	0x00010000	/* PCI2 command register write	    */
#define UIC_P2PM	0x00008000	/* PCI2 power management	    */
#define UIC_P2VPD	0x00004000	/* PCI2 VPD access		    */
#define UIC_P2MSI0	0x00002000	/* PCI2 MSI level 0		    */
#define UIC_D0CPF	0x00001000	/* DMA0 command pointer		    */
#define UIC_D0CSF	0x00000800	/* DMA0 command status		    */
#define UIC_D1CPF	0x00000400	/* DMA1 command pointer		    */
#define UIC_D1CSF	0x00000200	/* DMA1 command status		    */
#define UIC_I2OID	0x00000100	/* I2O inbound doorbell		    */
#define UIC_I2OPLF	0x00000080	/* I2O inbound post list	    */
#define UIC_I2O0LL	0x00000040	/* I2O0 low latency PLB write	    */
#define UIC_I2O1LL	0x00000020	/* I2O1 low latency PLB write	    */
#define UIC_I2O0HB	0x00000010	/* I2O0 high bandwidth PLB write    */
#define UIC_I2O1HB	0x00000008	/* I2O1 high bandwidth PLB write    */
#define UIC_GPTCT	0x00000004	/* GPT count timer		    */
#define UIC_UIC1NC	0x00000002	/* UIC1 non-critical interrupt	    */
#define UIC_UIC1C	0x00000001	/* UIC1 critical interrupt	    */
#else  /* CONFIG_440SP */
#define UIC_U0		0x80000000	/* UART 0			    */
#define UIC_U1		0x40000000	/* UART 1			    */
#define UIC_IIC0	0x20000000	/* IIC				    */
#define UIC_IIC1	0x10000000	/* IIC				    */
#define UIC_PIM		0x08000000	/* PCI inbound message		    */
#define UIC_PCRW	0x04000000	/* PCI command register write	    */
#define UIC_PPM		0x02000000	/* PCI power management		    */
#define UIC_MSI0	0x01000000	/* PCI MSI level 0		    */
#define UIC_MSI1	0x00800000	/* PCI MSI level 1		    */
#define UIC_MSI2	0x00400000	/* PCI MSI level 2		    */
#define UIC_MTE		0x00200000	/* MAL TXEOB			    */
#define UIC_MRE		0x00100000	/* MAL RXEOB			    */
#define UIC_D0		0x00080000	/* DMA channel 0		    */
#define UIC_D1		0x00040000	/* DMA channel 1		    */
#define UIC_D2		0x00020000	/* DMA channel 2		    */
#define UIC_D3		0x00010000	/* DMA channel 3		    */
#define UIC_RSVD0	0x00008000	/* Reserved			    */
#define UIC_RSVD1	0x00004000	/* Reserved			    */
#define UIC_CT0		0x00002000	/* GPT compare timer 0		    */
#define UIC_CT1		0x00001000	/* GPT compare timer 1		    */
#define UIC_CT2		0x00000800	/* GPT compare timer 2		    */
#define UIC_CT3		0x00000400	/* GPT compare timer 3		    */
#define UIC_CT4		0x00000200	/* GPT compare timer 4		    */
#define UIC_EIR0	0x00000100	/* External interrupt 0		    */
#define UIC_EIR1	0x00000080	/* External interrupt 1		    */
#define UIC_EIR2	0x00000040	/* External interrupt 2		    */
#define UIC_EIR3	0x00000020	/* External interrupt 3		    */
#define UIC_EIR4	0x00000010	/* External interrupt 4		    */
#define UIC_EIR5	0x00000008	/* External interrupt 5		    */
#define UIC_EIR6	0x00000004	/* External interrupt 6		    */
#define UIC_UIC1NC	0x00000002	/* UIC1 non-critical interrupt	    */
#define UIC_UIC1C	0x00000001	/* UIC1 critical interrupt	    */
#endif /* CONFIG_440SP */

/* For compatibility with 405 code */
#define UIC_MAL_TXEOB	UIC_MTE
#define UIC_MAL_RXEOB	UIC_MRE

/*---------------------------------------------------------------------------+
|  Universal interrupt controller 1 interrupts (UIC1)
+---------------------------------------------------------------------------*/
#if defined(CONFIG_440SP)
#define UIC_EIR0	0x80000000	/* External interrupt 0		    */
#define UIC_MS		0x40000000	/* MAL SERR			    */
#define UIC_MTDE	0x20000000	/* MAL TXDE			    */
#define UIC_MRDE	0x10000000	/* MAL RXDE			    */
#define UIC_DECE	0x08000000	/* DDR SDRAM correctible error	    */
#define UIC_EBCO	0x04000000	/* EBCO interrupt status	    */
#define UIC_MTE		0x02000000	/* MAL TXEOB			    */
#define UIC_MRE		0x01000000	/* MAL RXEOB			    */
#define UIC_P0MSI1	0x00800000	/* PCI0 MSI level 1		    */
#define UIC_P1MSI1	0x00400000	/* PCI1 MSI level 1		    */
#define UIC_P2MSI1	0x00200000	/* PCI2 MSI level 1		    */
#define UIC_L2C		0x00100000	/* L2 cache			    */
#define UIC_CT0		0x00080000	/* GPT compare timer 0		    */
#define UIC_CT1		0x00040000	/* GPT compare timer 1		    */
#define UIC_CT2		0x00020000	/* GPT compare timer 2		    */
#define UIC_CT3		0x00010000	/* GPT compare timer 3		    */
#define UIC_CT4		0x00008000	/* GPT compare timer 4		    */
#define UIC_EIR1	0x00004000	/* External interrupt 1		    */
#define UIC_EIR2	0x00002000	/* External interrupt 2		    */
#define UIC_EIR3	0x00001000	/* External interrupt 3		    */
#define UIC_EIR4	0x00000800	/* External interrupt 4		    */
#define UIC_EIR5	0x00000400	/* External interrupt 5		    */
#define UIC_DMAE	0x00000200	/* DMA error			    */
#define UIC_I2OE	0x00000100	/* I2O error			    */
#define UIC_SRE		0x00000080	/* Serial ROM error		    */
#define UIC_P0AE	0x00000040	/* PCI0 asynchronous error	    */
#define UIC_P1AE	0x00000020	/* PCI1 asynchronous error	    */
#define UIC_P2AE	0x00000010	/* PCI2 asynchronous error	    */
#define UIC_ETH0	0x00000008	/* Ethernet 0			    */
#define UIC_EWU0	0x00000004	/* Ethernet 0 wakeup		    */
#define UIC_ETH1	0x00000002	/* Reserved			    */
#define UIC_XOR		0x00000001	/* XOR				    */
#else /* CONFIG_440SP */
#define UIC_MS		0x80000000	/* MAL SERR			    */
#define UIC_MTDE	0x40000000	/* MAL TXDE			    */
#define UIC_MRDE	0x20000000	/* MAL RXDE			    */
#define UIC_DEUE	0x10000000	/* DDR SDRAM ECC uncorrectible error*/
#define UIC_DECE	0x08000000	/* DDR SDRAM correctible error	    */
#define UIC_EBCO	0x04000000	/* EBCO interrupt status	    */
#define UIC_EBMI	0x02000000	/* EBMI interrupt status	    */
#define UIC_OPB		0x01000000	/* OPB to PLB bridge interrupt stat */
#define UIC_MSI3	0x00800000	/* PCI MSI level 3		    */
#define UIC_MSI4	0x00400000	/* PCI MSI level 4		    */
#define UIC_MSI5	0x00200000	/* PCI MSI level 5		    */
#define UIC_MSI6	0x00100000	/* PCI MSI level 6		    */
#define UIC_MSI7	0x00080000	/* PCI MSI level 7		    */
#define UIC_MSI8	0x00040000	/* PCI MSI level 8		    */
#define UIC_MSI9	0x00020000	/* PCI MSI level 9		    */
#define UIC_MSI10	0x00010000	/* PCI MSI level 10		    */
#define UIC_MSI11	0x00008000	/* PCI MSI level 11		    */
#define UIC_PPMI	0x00004000	/* PPM interrupt status		    */
#define UIC_EIR7	0x00002000	/* External interrupt 7		    */
#define UIC_EIR8	0x00001000	/* External interrupt 8		    */
#define UIC_EIR9	0x00000800	/* External interrupt 9		    */
#define UIC_EIR10	0x00000400	/* External interrupt 10	    */
#define UIC_EIR11	0x00000200	/* External interrupt 11	    */
#define UIC_EIR12	0x00000100	/* External interrupt 12	    */
#define UIC_SRE		0x00000080	/* Serial ROM error		    */
#define UIC_RSVD2	0x00000040	/* Reserved			    */
#define UIC_RSVD3	0x00000020	/* Reserved			    */
#define UIC_PAE		0x00000010	/* PCI asynchronous error	    */
#define UIC_ETH0	0x00000008	/* Ethernet 0			    */
#define UIC_EWU0	0x00000004	/* Ethernet 0 wakeup		    */
#define UIC_ETH1	0x00000002	/* Ethernet 1			    */
#define UIC_EWU1	0x00000001	/* Ethernet 1 wakeup		    */
#endif /* CONFIG_440SP */

/* For compatibility with 405 code */
#define UIC_MAL_SERR	UIC_MS
#define UIC_MAL_TXDE	UIC_MTDE
#define UIC_MAL_RXDE	UIC_MRDE
#define UIC_ENET	UIC_ETH0

/*---------------------------------------------------------------------------+
|  Universal interrupt controller 2 interrupts (UIC2)
+---------------------------------------------------------------------------*/
#if defined(CONFIG_440GX)
#define UIC_ETH2	0x80000000	/* Ethernet 2			    */
#define UIC_EWU2	0x40000000	/* Ethernet 2 wakeup		    */
#define UIC_ETH3	0x20000000	/* Ethernet 3			    */
#define UIC_EWU3	0x10000000	/* Ethernet 3 wakeup		    */
#define UIC_TAH0	0x08000000	/* TAH 0			    */
#define UIC_TAH1	0x04000000	/* TAH 1			    */
#define UIC_IMUOBFQ	0x02000000	/* IMU outbound free queue	    */
#define UIC_IMUIBPQ	0x01000000	/* IMU inbound post queue	    */
#define UIC_IMUIRQDB	0x00800000	/* IMU irq doorbell		    */
#define UIC_IMUIBDB	0x00400000	/* IMU inbound doorbell		    */
#define UIC_IMUMSG0	0x00200000	/* IMU inbound message 0	    */
#define UIC_IMUMSG1	0x00100000	/* IMU inbound message 1	    */
#define UIC_IMUTO	0x00080000	/* IMU timeout			    */
#define UIC_MSI12	0x00040000	/* PCI MSI level 12		    */
#define UIC_MSI13	0x00020000	/* PCI MSI level 13		    */
#define UIC_MSI14	0x00010000	/* PCI MSI level 14		    */
#define UIC_MSI15	0x00008000	/* PCI MSI level 15		    */
#define UIC_EIR13	0x00004000	/* External interrupt 13	    */
#define UIC_EIR14	0x00002000	/* External interrupt 14	    */
#define UIC_EIR15	0x00001000	/* External interrupt 15	    */
#define UIC_EIR16	0x00000800	/* External interrupt 16	    */
#define UIC_EIR17	0x00000400	/* External interrupt 17	    */
#define UIC_PCIVPD	0x00000200	/* PCI VPD			    */
#define UIC_L2C		0x00000100	/* L2 Cache			    */
#define UIC_ETH2PCS	0x00000080	/* Ethernet 2 PCS		    */
#define UIC_ETH3PCS	0x00000040	/* Ethernet 3 PCS		    */
#define UIC_RSVD26	0x00000020	/* Reserved			    */
#define UIC_RSVD27	0x00000010	/* Reserved			    */
#define UIC_RSVD28	0x00000008	/* Reserved			    */
#define UIC_RSVD29	0x00000004	/* Reserved			    */
#define UIC_RSVD30	0x00000002	/* Reserved			    */
#define UIC_RSVD31	0x00000001	/* Reserved			    */
#endif	/* CONFIG_440GX */

/*---------------------------------------------------------------------------+
|  Universal interrupt controller Base 0 interrupts (UICB0)
+---------------------------------------------------------------------------*/
#if defined(CONFIG_440GX)
#define UICB0_UIC0CI	0x80000000	/* UIC0 Critical Interrupt	    */
#define UICB0_UIC0NCI	0x40000000	/* UIC0 Noncritical Interrupt	    */
#define UICB0_UIC1CI	0x20000000	/* UIC1 Critical Interrupt	    */
#define UICB0_UIC1NCI	0x10000000	/* UIC1 Noncritical Interrupt	    */
#define UICB0_UIC2CI	0x08000000	/* UIC2 Critical Interrupt	    */
#define UICB0_UIC2NCI	0x04000000	/* UIC2 Noncritical Interrupt	    */

#define UICB0_ALL		(UICB0_UIC0CI | UICB0_UIC0NCI | UICB0_UIC1CI | \
						 UICB0_UIC1NCI | UICB0_UIC2CI | UICB0_UIC2NCI)
#endif /* CONFIG_440GX */

/*-----------------------------------------------------------------------------+
|  External Bus Controller Bit Settings
+-----------------------------------------------------------------------------*/
#define EBC_CFGADDR_MASK		0x0000003F

#define EBC_BXCR_BAS_ENCODE(n)	((((unsigned long)(n))&0xFFF00000)<<0)
#define EBC_BXCR_BS_MASK		0x000E0000
#define EBC_BXCR_BS_1MB			0x00000000
#define EBC_BXCR_BS_2MB			0x00020000
#define EBC_BXCR_BS_4MB			0x00040000
#define EBC_BXCR_BS_8MB			0x00060000
#define EBC_BXCR_BS_16MB		0x00080000
#define EBC_BXCR_BS_32MB		0x000A0000
#define EBC_BXCR_BS_64MB		0x000C0000
#define EBC_BXCR_BS_128MB		0x000E0000
#define EBC_BXCR_BU_MASK		0x00018000
#define EBC_BXCR_BU_R			0x00008000
#define EBC_BXCR_BU_W			0x00010000
#define EBC_BXCR_BU_RW			0x00018000
#define EBC_BXCR_BW_MASK		0x00006000
#define EBC_BXCR_BW_8BIT		0x00000000
#define EBC_BXCR_BW_16BIT		0x00002000
#define EBC_BXCR_BW_32BIT		0x00006000
#define EBC_BXAP_BME_ENABLED		0x80000000
#define EBC_BXAP_BME_DISABLED		0x00000000
#define EBC_BXAP_TWT_ENCODE(n)		((((unsigned long)(n))&0xFF)<<23)
#define EBC_BXAP_BCE_DISABLE		0x00000000
#define EBC_BXAP_BCE_ENABLE		0x00400000
#define EBC_BXAP_BCT_MASK		0x00300000
#define EBC_BXAP_BCT_2TRANS		0x00000000
#define EBC_BXAP_BCT_4TRANS		0x00100000
#define EBC_BXAP_BCT_8TRANS		0x00200000
#define EBC_BXAP_BCT_16TRANS		0x00300000
#define EBC_BXAP_CSN_ENCODE(n)		((((unsigned long)(n))&0x3)<<18)
#define EBC_BXAP_OEN_ENCODE(n)		((((unsigned long)(n))&0x3)<<16)
#define EBC_BXAP_WBN_ENCODE(n)		((((unsigned long)(n))&0x3)<<14)
#define EBC_BXAP_WBF_ENCODE(n)		((((unsigned long)(n))&0x3)<<12)
#define EBC_BXAP_TH_ENCODE(n)		((((unsigned long)(n))&0x7)<<9)
#define EBC_BXAP_RE_ENABLED		0x00000100
#define EBC_BXAP_RE_DISABLED		0x00000000
#define EBC_BXAP_SOR_DELAYED		0x00000000
#define EBC_BXAP_SOR_NONDELAYED		0x00000080
#define EBC_BXAP_BEM_WRITEONLY		0x00000000
#define EBC_BXAP_BEM_RW			0x00000040
#define EBC_BXAP_PEN_DISABLED		0x00000000

#define EBC_CFG_LE_MASK			0x80000000
#define EBC_CFG_LE_UNLOCK		0x00000000
#define EBC_CFG_LE_LOCK			0x80000000
#define EBC_CFG_PTD_MASK		0x40000000
#define EBC_CFG_PTD_ENABLE		0x00000000
#define EBC_CFG_PTD_DISABLE		0x40000000
#define EBC_CFG_RTC_MASK		0x38000000
#define EBC_CFG_RTC_16PERCLK		0x00000000
#define EBC_CFG_RTC_32PERCLK		0x08000000
#define EBC_CFG_RTC_64PERCLK		0x10000000
#define EBC_CFG_RTC_128PERCLK		0x18000000
#define EBC_CFG_RTC_256PERCLK		0x20000000
#define EBC_CFG_RTC_512PERCLK		0x28000000
#define EBC_CFG_RTC_1024PERCLK		0x30000000
#define EBC_CFG_RTC_2048PERCLK		0x38000000
#define EBC_CFG_ATC_MASK		0x04000000
#define EBC_CFG_ATC_HI			0x00000000
#define EBC_CFG_ATC_PREVIOUS		0x04000000
#define EBC_CFG_DTC_MASK		0x02000000
#define EBC_CFG_DTC_HI			0x00000000
#define EBC_CFG_DTC_PREVIOUS		0x02000000
#define EBC_CFG_CTC_MASK		0x01000000
#define EBC_CFG_CTC_HI			0x00000000
#define EBC_CFG_CTC_PREVIOUS		0x01000000
#define EBC_CFG_OEO_MASK		0x00800000
#define EBC_CFG_OEO_HI			0x00000000
#define EBC_CFG_OEO_PREVIOUS		0x00800000
#define EBC_CFG_EMC_MASK		0x00400000
#define EBC_CFG_EMC_NONDEFAULT		0x00000000
#define EBC_CFG_EMC_DEFAULT		0x00400000
#define EBC_CFG_PME_MASK		0x00200000
#define EBC_CFG_PME_DISABLE		0x00000000
#define EBC_CFG_PME_ENABLE		0x00200000
#define EBC_CFG_PMT_MASK		0x001F0000
#define EBC_CFG_PMT_ENCODE(n)		((((unsigned long)(n))&0x1F)<<12)
#define EBC_CFG_PR_MASK			0x0000C000
#define EBC_CFG_PR_16			0x00000000
#define EBC_CFG_PR_32			0x00004000
#define EBC_CFG_PR_64			0x00008000
#define EBC_CFG_PR_128			0x0000C000

/*-----------------------------------------------------------------------------+
|  SDR0 Bit Settings
+-----------------------------------------------------------------------------*/
#define SDR0_SDCS_SDD			(0x80000000 >> 31)

#if defined(CONFIG_440GP)
#define CPC0_STRP1_PAE_MASK		(0x80000000 >> 11)
#define CPC0_STRP1_PISE_MASK		(0x80000000 >> 13)
#endif /* defined(CONFIG_440GP) */
#if defined(CONFIG_440GX) || defined(CONFIG_440SP)
#define SDR0_SDSTP1_PAE_MASK		(0x80000000 >> 13)
#define SDR0_SDSTP1_PISE_MASK		(0x80000000 >> 15)
#endif /* defined(CONFIG_440GX) || defined(CONFIG_440SP) */
#if defined(CONFIG_440EP) || defined(CONFIG_440GR)
#define SDR0_SDSTP1_PAE_MASK		(0x80000000 >> 21)
#define SDR0_SDSTP1_PAME_MASK		(0x80000000 >> 27)
#endif /* defined(CONFIG_440EP) || defined(CONFIG_440GR) */

#define SDR0_UARTX_UXICS_MASK		0xF0000000
#define SDR0_UARTX_UXICS_PLB		0x20000000
#define SDR0_UARTX_UXEC_MASK		0x00800000
#define SDR0_UARTX_UXEC_INT		0x00000000
#define SDR0_UARTX_UXEC_EXT		0x00800000
#define SDR0_UARTX_UXDTE_MASK		0x00400000
#define SDR0_UARTX_UXDTE_DISABLE	0x00000000
#define SDR0_UARTX_UXDTE_ENABLE		0x00400000
#define SDR0_UARTX_UXDRE_MASK		0x00200000
#define SDR0_UARTX_UXDRE_DISABLE	0x00000000
#define SDR0_UARTX_UXDRE_ENABLE		0x00200000
#define SDR0_UARTX_UXDC_MASK		0x00100000
#define SDR0_UARTX_UXDC_NOTCLEARED	0x00000000
#define SDR0_UARTX_UXDC_CLEARED		0x00100000
#define SDR0_UARTX_UXDIV_MASK		0x000000FF
#define SDR0_UARTX_UXDIV_ENCODE(n)	((((unsigned long)(n))&0xFF)<<0)
#define SDR0_UARTX_UXDIV_DECODE(n)	((((((unsigned long)(n))>>0)-1)&0xFF)+1)

#define SDR0_CPU440_EARV_MASK		0x30000000
#define SDR0_CPU440_EARV_EBC		0x10000000
#define SDR0_CPU440_EARV_PCI		0x20000000
#define SDR0_CPU440_EARV_ENCODE(n)	((((unsigned long)(n))&0x03)<<28)
#define SDR0_CPU440_EARV_DECODE(n)	((((unsigned long)(n))>>28)&0x03)
#define SDR0_CPU440_NTO1_MASK		0x00000002
#define SDR0_CPU440_NTO1_NTOP		0x00000000
#define SDR0_CPU440_NTO1_NTO1		0x00000002
#define SDR0_CPU440_NTO1_ENCODE(n)	((((unsigned long)(n))&0x01)<<1)
#define SDR0_CPU440_NTO1_DECODE(n)	((((unsigned long)(n))>>1)&0x01)

#define SDR0_XCR_PAE_MASK		0x80000000
#define SDR0_XCR_PAE_DISABLE		0x00000000
#define SDR0_XCR_PAE_ENABLE		0x80000000
#define SDR0_XCR_PAE_ENCODE(n)		((((unsigned long)(n))&0x01)<<31)
#define SDR0_XCR_PAE_DECODE(n)		((((unsigned long)(n))>>31)&0x01)
#define SDR0_XCR_PHCE_MASK		0x40000000
#define SDR0_XCR_PHCE_DISABLE		0x00000000
#define SDR0_XCR_PHCE_ENABLE		0x40000000
#define SDR0_XCR_PHCE_ENCODE(n)		((((unsigned long)(n))&0x01)<<30)
#define SDR0_XCR_PHCE_DECODE(n)		((((unsigned long)(n))>>30)&0x01)
#define SDR0_XCR_PISE_MASK		0x20000000
#define SDR0_XCR_PISE_DISABLE		0x00000000
#define SDR0_XCR_PISE_ENABLE		0x20000000
#define SDR0_XCR_PISE_ENCODE(n)		((((unsigned long)(n))&0x01)<<29)
#define SDR0_XCR_PISE_DECODE(n)		((((unsigned long)(n))>>29)&0x01)
#define SDR0_XCR_PCWE_MASK		0x10000000
#define SDR0_XCR_PCWE_DISABLE		0x00000000
#define SDR0_XCR_PCWE_ENABLE		0x10000000
#define SDR0_XCR_PCWE_ENCODE(n)		((((unsigned long)(n))&0x01)<<28)
#define SDR0_XCR_PCWE_DECODE(n)		((((unsigned long)(n))>>28)&0x01)
#define SDR0_XCR_PPIM_MASK		0x0F000000
#define SDR0_XCR_PPIM_ENCODE(n)		((((unsigned long)(n))&0x0F)<<24)
#define SDR0_XCR_PPIM_DECODE(n)		((((unsigned long)(n))>>24)&0x0F)
#define SDR0_XCR_PR64E_MASK		0x00800000
#define SDR0_XCR_PR64E_DISABLE		0x00000000
#define SDR0_XCR_PR64E_ENABLE		0x00800000
#define SDR0_XCR_PR64E_ENCODE(n)	((((unsigned long)(n))&0x01)<<23)
#define SDR0_XCR_PR64E_DECODE(n)	((((unsigned long)(n))>>23)&0x01)
#define SDR0_XCR_PXFS_MASK		0x00600000
#define SDR0_XCR_PXFS_HIGH		0x00000000
#define SDR0_XCR_PXFS_MED		0x00200000
#define SDR0_XCR_PXFS_LOW		0x00400000
#define SDR0_XCR_PXFS_ENCODE(n)		((((unsigned long)(n))&0x03)<<21)
#define SDR0_XCR_PXFS_DECODE(n)		((((unsigned long)(n))>>21)&0x03)
#define SDR0_XCR_PDM_MASK		0x00000040
#define SDR0_XCR_PDM_MULTIPOINT		0x00000000
#define SDR0_XCR_PDM_P2P		0x00000040
#define SDR0_XCR_PDM_ENCODE(n)		((((unsigned long)(n))&0x01)<<19)
#define SDR0_XCR_PDM_DECODE(n)		((((unsigned long)(n))>>19)&0x01)

#define SDR0_PFC0_UART1_DSR_CTS_EN_MASK 0x00030000
#define SDR0_PFC0_GEIE_MASK		0x00003E00
#define SDR0_PFC0_GEIE_TRE		0x00003E00
#define SDR0_PFC0_GEIE_NOTRE		0x00000000
#define SDR0_PFC0_TRE_MASK		0x00000100
#define SDR0_PFC0_TRE_DISABLE		0x00000000
#define SDR0_PFC0_TRE_ENABLE		0x00000100
#define SDR0_PFC0_TRE_ENCODE(n)		((((unsigned long)(n))&0x01)<<8)
#define SDR0_PFC0_TRE_DECODE(n)		((((unsigned long)(n))>>8)&0x01)

#define SDR0_PFC1_UART1_DSR_CTS_MASK	0x02000000
#define SDR0_PFC1_EPS_MASK		0x01C00000
#define SDR0_PFC1_EPS_GROUP0		0x00000000
#define SDR0_PFC1_EPS_GROUP1		0x00400000
#define SDR0_PFC1_EPS_GROUP2		0x00800000
#define SDR0_PFC1_EPS_GROUP3		0x00C00000
#define SDR0_PFC1_EPS_GROUP4		0x01000000
#define SDR0_PFC1_EPS_GROUP5		0x01400000
#define SDR0_PFC1_EPS_GROUP6		0x01800000
#define SDR0_PFC1_EPS_GROUP7		0x01C00000
#define SDR0_PFC1_EPS_ENCODE(n)		((((unsigned long)(n))&0x07)<<22)
#define SDR0_PFC1_EPS_DECODE(n)		((((unsigned long)(n))>>22)&0x07)
#define SDR0_PFC1_RMII_MASK		0x00200000
#define SDR0_PFC1_RMII_100MBIT		0x00000000
#define SDR0_PFC1_RMII_10MBIT		0x00200000
#define SDR0_PFC1_RMII_ENCODE(n)	((((unsigned long)(n))&0x01)<<21)
#define SDR0_PFC1_RMII_DECODE(n)	((((unsigned long)(n))>>21)&0x01)
#define SDR0_PFC1_CTEMS_MASK		0x00100000
#define SDR0_PFC1_CTEMS_EMS		0x00000000
#define SDR0_PFC1_CTEMS_CPUTRACE	0x00100000

#define SDR0_MFR_TAH0_MASK		0x80000000
#define SDR0_MFR_TAH0_ENABLE		0x00000000
#define SDR0_MFR_TAH0_DISABLE		0x80000000
#define SDR0_MFR_TAH1_MASK		0x40000000
#define SDR0_MFR_TAH1_ENABLE		0x00000000
#define SDR0_MFR_TAH1_DISABLE		0x40000000
#define SDR0_MFR_PCM_MASK		0x20000000
#define SDR0_MFR_PCM_PPC440GX		0x00000000
#define SDR0_MFR_PCM_PPC440GP		0x20000000
#define SDR0_MFR_ECS_MASK		0x10000000
#define SDR0_MFR_ECS_INTERNAL		0x10000000

#define SDR0_MFR_ETH0_CLK_SEL        0x08000000   /* Ethernet0 Clock Select */
#define SDR0_MFR_ETH1_CLK_SEL        0x04000000   /* Ethernet1 Clock Select */
#define SDR0_MFR_ZMII_MODE_MASK      0x03000000   /* ZMII Mode Mask   */
#define SDR0_MFR_ZMII_MODE_MII       0x00000000     /* ZMII Mode MII  */
#define SDR0_MFR_ZMII_MODE_SMII      0x01000000     /* ZMII Mode SMII */
#define SDR0_MFR_ZMII_MODE_RMII_10M  0x02000000     /* ZMII Mode RMII - 10 Mbs   */
#define SDR0_MFR_ZMII_MODE_RMII_100M 0x03000000     /* ZMII Mode RMII - 100 Mbs  */
#define SDR0_MFR_ZMII_MODE_BIT0      0x02000000     /* ZMII Mode Bit0 */
#define SDR0_MFR_ZMII_MODE_BIT1      0x01000000     /* ZMII Mode Bit1 */
#define SDR0_MFR_ERRATA3_EN0         0x00800000
#define SDR0_MFR_ERRATA3_EN1         0x00400000
#define SDR0_MFR_PKT_REJ_MASK        0x00300000   /* Pkt Rej. Enable Mask */
#define SDR0_MFR_PKT_REJ_EN          0x00300000   /* Pkt Rej. Enable on both EMAC3 0-1 */
#define SDR0_MFR_PKT_REJ_EN0         0x00200000   /* Pkt Rej. Enable on EMAC3(0) */
#define SDR0_MFR_PKT_REJ_EN1         0x00100000   /* Pkt Rej. Enable on EMAC3(1) */
#define SDR0_MFR_PKT_REJ_POL         0x00080000   /* Packet Reject Polarity      */

#define SDR0_SRST_BGO			0x80000000
#define SDR0_SRST_PLB			0x40000000
#define SDR0_SRST_EBC			0x20000000
#define SDR0_SRST_OPB			0x10000000
#define SDR0_SRST_UART0			0x08000000
#define SDR0_SRST_UART1			0x04000000
#define SDR0_SRST_IIC0			0x02000000
#define SDR0_SRST_IIC1			0x01000000
#define SDR0_SRST_GPIO			0x00800000
#define SDR0_SRST_GPT			0x00400000
#define SDR0_SRST_DMC			0x00200000
#define SDR0_SRST_PCI			0x00100000
#define SDR0_SRST_EMAC0			0x00080000
#define SDR0_SRST_EMAC1			0x00040000
#define SDR0_SRST_CPM			0x00020000
#define SDR0_SRST_IMU			0x00010000
#define SDR0_SRST_UIC01			0x00008000
#define SDR0_SRST_UICB2			0x00004000
#define SDR0_SRST_SRAM			0x00002000
#define SDR0_SRST_EBM			0x00001000
#define SDR0_SRST_BGI			0x00000800
#define SDR0_SRST_DMA			0x00000400
#define SDR0_SRST_DMAC			0x00000200
#define SDR0_SRST_MAL			0x00000100
#define SDR0_SRST_ZMII			0x00000080
#define SDR0_SRST_GPTR			0x00000040
#define SDR0_SRST_PPM			0x00000020
#define SDR0_SRST_EMAC2			0x00000010
#define SDR0_SRST_EMAC3			0x00000008
#define SDR0_SRST_RGMII			0x00000001

/*-----------------------------------------------------------------------------+
|  Clocking
+-----------------------------------------------------------------------------*/
#if !defined (CONFIG_440GX) && !defined(CONFIG_440EP) && !defined(CONFIG_440GR) && !defined(CONFIG_440SP)
#define PLLSYS0_TUNE_MASK	0xffc00000	/* PLL TUNE bits	    */
#define PLLSYS0_FB_DIV_MASK	0x003c0000	/* Feedback divisor	    */
#define PLLSYS0_FWD_DIV_A_MASK	0x00038000	/* Forward divisor A	    */
#define PLLSYS0_FWD_DIV_B_MASK	0x00007000	/* Forward divisor B	    */
#define PLLSYS0_OPB_DIV_MASK	0x00000c00	/* OPB divisor		    */
#define PLLSYS0_EPB_DIV_MASK	0x00000300	/* EPB divisor		    */
#define PLLSYS0_EXTSL_MASK	0x00000080	/* PerClk feedback path	    */
#define PLLSYS0_RW_MASK		0x00000060	/* ROM width		    */
#define PLLSYS0_RL_MASK		0x00000010	/* ROM location		    */
#define PLLSYS0_ZMII_SEL_MASK	0x0000000c	/* ZMII selection	    */
#define PLLSYS0_BYPASS_MASK	0x00000002	/* Bypass PLL		    */
#define PLLSYS0_NTO1_MASK	0x00000001	/* CPU:PLB N-to-1 ratio	    */

#define PLL_VCO_FREQ_MIN	500		/* Min VCO freq (MHz)	    */
#define PLL_VCO_FREQ_MAX	1000		/* Max VCO freq (MHz)	    */
#define PLL_CPU_FREQ_MAX	400		/* Max CPU freq (MHz)	    */
#define PLL_PLB_FREQ_MAX	133		/* Max PLB freq (MHz)	    */
#else /* !CONFIG_440GX or CONFIG_440EP or CONFIG_440GR */
#define PLLSYS0_ENG_MASK	0x80000000	/* 0 = SysClk, 1 = PLL VCO */
#define PLLSYS0_SRC_MASK	0x40000000	/* 0 = PLL A, 1 = PLL B */
#define PLLSYS0_SEL_MASK	0x38000000	/* 0 = PLL, 1 = CPU, 5 = PerClk */
#define PLLSYS0_TUNE_MASK	0x07fe0000	/* PLL Tune bits */
#define PLLSYS0_FB_DIV_MASK	0x0001f000	/* Feedback divisor */
#define PLLSYS0_FWD_DIV_A_MASK	0x00000f00	/* Fwd Div A */
#define PLLSYS0_FWD_DIV_B_MASK	0x000000e0	/* Fwd Div B */
#define PLLSYS0_PRI_DIV_B_MASK	0x0000001c	/* PLL Primary Divisor B */
#define PLLSYS0_OPB_DIV_MASK	0x00000003	/* OPB Divisor */

#define PLLC_ENG_MASK       0x20000000  /* PLL primary forward divisor source   */
#define PLLC_SRC_MASK       0x20000000  /* PLL feedback source   */
#define PLLD_FBDV_MASK      0x1f000000  /* PLL Feedback Divisor  */
#define PLLD_FWDVA_MASK     0x000f0000  /* PLL Forward Divisor A */
#define PLLD_FWDVB_MASK     0x00000700  /* PLL Forward Divisor B */
#define PLLD_LFBDV_MASK     0x0000003f  /* PLL Local Feedback Divisor */

#define OPBDDV_MASK         0x03000000  /* OPB Clock Divisor Register */
#define PERDV_MASK          0x07000000  /* Periferal Clock Divisor */
#define PRADV_MASK          0x07000000  /* Primary Divisor A */
#define PRBDV_MASK          0x07000000  /* Primary Divisor B */
#define SPCID_MASK          0x03000000  /* Sync PCI Divisor  */

#define PLL_VCO_FREQ_MIN	500		/* Min VCO freq (MHz)	    */
#define PLL_VCO_FREQ_MAX	1000		/* Max VCO freq (MHz)	    */
#define PLL_CPU_FREQ_MAX	400		/* Max CPU freq (MHz)	    */
#define PLL_PLB_FREQ_MAX	133		/* Max PLB freq (MHz)	    */

/* Strap 1 Register */
#define PLLSYS1_LF_DIV_MASK	0xfc000000	/* PLL Local Feedback Divisor */
#define PLLSYS1_PERCLK_DIV_MASK 0x03000000	/* Peripheral Clk Divisor */
#define PLLSYS1_MAL_DIV_MASK	0x00c00000	/* MAL Clk Divisor */
#define PLLSYS1_RW_MASK		0x00300000	/* ROM width */
#define PLLSYS1_EAR_MASK	0x00080000	/* ERAP Addres reset vector */
#define PLLSYS1_PAE_MASK	0x00040000	/* PCI arbitor enable */
#define PLLSYS1_PCHE_MASK	0x00020000	/* PCI host config enable */
#define PLLSYS1_PISE_MASK	0x00010000	/* PCI init seq. enable */
#define PLLSYS1_PCWE_MASK	0x00008000	/* PCI local cpu wait enable */
#define PLLSYS1_PPIM_MASK	0x00007800	/* PCI inbound map */
#define PLLSYS1_PR64E_MASK	0x00000400	/* PCI init Req64 enable */
#define PLLSYS1_PXFS_MASK	0x00000300	/* PCI-X Freq Sel */
#define PLLSYS1_RSVD_MASK	0x00000080	/* RSVD */
#define PLLSYS1_PDM_MASK	0x00000040	/* PCI-X Driver Mode */
#define PLLSYS1_EPS_MASK	0x00000038	/* Ethernet Pin Select */
#define PLLSYS1_RMII_MASK	0x00000004	/* RMII Mode */
#define PLLSYS1_TRE_MASK	0x00000002	/* GPIO Trace Enable */
#define PLLSYS1_NTO1_MASK	0x00000001	/* CPU:PLB N-to-1 ratio */
#endif /* CONFIG_440GX */

/*-----------------------------------------------------------------------------
| IIC Register Offsets
'----------------------------------------------------------------------------*/
#define IICMDBUF		0x00
#define IICSDBUF		0x02
#define IICLMADR		0x04
#define IICHMADR		0x05
#define IICCNTL			0x06
#define IICMDCNTL		0x07
#define IICSTS			0x08
#define IICEXTSTS		0x09
#define IICLSADR		0x0A
#define IICHSADR		0x0B
#define IICCLKDIV		0x0C
#define IICINTRMSK		0x0D
#define IICXFRCNT		0x0E
#define IICXTCNTLSS		0x0F
#define IICDIRECTCNTL		0x10

/*-----------------------------------------------------------------------------
| UART Register Offsets
'----------------------------------------------------------------------------*/
#define DATA_REG		0x00
#define DL_LSB			0x00
#define DL_MSB			0x01
#define INT_ENABLE		0x01
#define FIFO_CONTROL		0x02
#define LINE_CONTROL		0x03
#define MODEM_CONTROL		0x04
#define LINE_STATUS		0x05
#define MODEM_STATUS		0x06
#define SCRATCH			0x07

/*-----------------------------------------------------------------------------
| PCI Internal Registers et. al. (accessed via plb)
+----------------------------------------------------------------------------*/
#define PCIX0_CFGADR		(CFG_PCI_BASE + 0x0ec00000)
#define PCIX0_CFGDATA		(CFG_PCI_BASE + 0x0ec00004)
#define PCIX0_CFGBASE		(CFG_PCI_BASE + 0x0ec80000)
#define PCIX0_IOBASE		(CFG_PCI_BASE + 0x08000000)

#if defined(CONFIG_440EP) || defined(CONFIG_440GR)

/* PCI Local Configuration Registers
   --------------------------------- */
#define PCI_MMIO_LCR_BASE (CFG_PCI_BASE + 0x0f400000)    /* Real => 0x0EF400000 */

/* PCI Master Local Configuration Registers */
#define PCIX0_PMM0LA         (PCI_MMIO_LCR_BASE + 0x00) /* PMM0 Local Address */
#define PCIX0_PMM0MA         (PCI_MMIO_LCR_BASE + 0x04) /* PMM0 Mask/Attribute */
#define PCIX0_PMM0PCILA      (PCI_MMIO_LCR_BASE + 0x08) /* PMM0 PCI Low Address */
#define PCIX0_PMM0PCIHA      (PCI_MMIO_LCR_BASE + 0x0C) /* PMM0 PCI High Address */
#define PCIX0_PMM1LA         (PCI_MMIO_LCR_BASE + 0x10) /* PMM1 Local Address */
#define PCIX0_PMM1MA         (PCI_MMIO_LCR_BASE + 0x14) /* PMM1 Mask/Attribute */
#define PCIX0_PMM1PCILA      (PCI_MMIO_LCR_BASE + 0x18) /* PMM1 PCI Low Address */
#define PCIX0_PMM1PCIHA      (PCI_MMIO_LCR_BASE + 0x1C) /* PMM1 PCI High Address */
#define PCIX0_PMM2LA         (PCI_MMIO_LCR_BASE + 0x20) /* PMM2 Local Address */
#define PCIX0_PMM2MA         (PCI_MMIO_LCR_BASE + 0x24) /* PMM2 Mask/Attribute */
#define PCIX0_PMM2PCILA      (PCI_MMIO_LCR_BASE + 0x28) /* PMM2 PCI Low Address */
#define PCIX0_PMM2PCIHA      (PCI_MMIO_LCR_BASE + 0x2C) /* PMM2 PCI High Address */

/* PCI Target Local Configuration Registers */
#define PCIX0_PTM1MS         (PCI_MMIO_LCR_BASE + 0x30) /* PTM1 Memory Size/Attribute */
#define PCIX0_PTM1LA         (PCI_MMIO_LCR_BASE + 0x34) /* PTM1 Local Addr. Reg */
#define PCIX0_PTM2MS         (PCI_MMIO_LCR_BASE + 0x38) /* PTM2 Memory Size/Attribute */
#define PCIX0_PTM2LA         (PCI_MMIO_LCR_BASE + 0x3C) /* PTM2 Local Addr. Reg */

#else

#define PCIX0_VENDID		(PCIX0_CFGBASE + PCI_VENDOR_ID )
#define PCIX0_DEVID		(PCIX0_CFGBASE + PCI_DEVICE_ID )
#define PCIX0_CMD		(PCIX0_CFGBASE + PCI_COMMAND )
#define PCIX0_STATUS		(PCIX0_CFGBASE + PCI_STATUS )
#define PCIX0_REVID		(PCIX0_CFGBASE + PCI_REVISION_ID )
#define PCIX0_CLS		(PCIX0_CFGBASE + PCI_CLASS_CODE)
#define PCIX0_CACHELS		(PCIX0_CFGBASE + PCI_CACHE_LINE_SIZE )
#define PCIX0_LATTIM		(PCIX0_CFGBASE + PCI_LATENCY_TIMER )
#define PCIX0_HDTYPE		(PCIX0_CFGBASE + PCI_HEADER_TYPE )
#define PCIX0_BIST		(PCIX0_CFGBASE + PCI_BIST )
#define PCIX0_BAR0		(PCIX0_CFGBASE + PCI_BASE_ADDRESS_0 )
#define PCIX0_BAR1		(PCIX0_CFGBASE + PCI_BASE_ADDRESS_1 )
#define PCIX0_BAR2		(PCIX0_CFGBASE + PCI_BASE_ADDRESS_2 )
#define PCIX0_BAR3		(PCIX0_CFGBASE + PCI_BASE_ADDRESS_3 )
#define PCIX0_BAR4		(PCIX0_CFGBASE + PCI_BASE_ADDRESS_4 )
#define PCIX0_BAR5		(PCIX0_CFGBASE + PCI_BASE_ADDRESS_5 )
#define PCIX0_CISPTR		(PCIX0_CFGBASE + PCI_CARDBUS_CIS )
#define PCIX0_SBSYSVID		(PCIX0_CFGBASE + PCI_SUBSYSTEM_VENDOR_ID )
#define PCIX0_SBSYSID		(PCIX0_CFGBASE + PCI_SUBSYSTEM_ID )
#define PCIX0_EROMBA		(PCIX0_CFGBASE + PCI_ROM_ADDRESS )
#define PCIX0_CAP		(PCIX0_CFGBASE + PCI_CAPABILITY_LIST )
#define PCIX0_RES0		(PCIX0_CFGBASE + 0x0035 )
#define PCIX0_RES1		(PCIX0_CFGBASE + 0x0036 )
#define PCIX0_RES2		(PCIX0_CFGBASE + 0x0038 )
#define PCIX0_INTLN		(PCIX0_CFGBASE + PCI_INTERRUPT_LINE )
#define PCIX0_INTPN		(PCIX0_CFGBASE + PCI_INTERRUPT_PIN )
#define PCIX0_MINGNT		(PCIX0_CFGBASE + PCI_MIN_GNT )
#define PCIX0_MAXLTNCY		(PCIX0_CFGBASE + PCI_MAX_LAT )

#define PCIX0_BRDGOPT1		(PCIX0_CFGBASE + 0x0040)
#define PCIX0_BRDGOPT2		(PCIX0_CFGBASE + 0x0044)

#define PCIX0_POM0LAL		(PCIX0_CFGBASE + 0x0068)
#define PCIX0_POM0LAH		(PCIX0_CFGBASE + 0x006c)
#define PCIX0_POM0SA		(PCIX0_CFGBASE + 0x0070)
#define PCIX0_POM0PCIAL		(PCIX0_CFGBASE + 0x0074)
#define PCIX0_POM0PCIAH		(PCIX0_CFGBASE + 0x0078)
#define PCIX0_POM1LAL		(PCIX0_CFGBASE + 0x007c)
#define PCIX0_POM1LAH		(PCIX0_CFGBASE + 0x0080)
#define PCIX0_POM1SA		(PCIX0_CFGBASE + 0x0084)
#define PCIX0_POM1PCIAL		(PCIX0_CFGBASE + 0x0088)
#define PCIX0_POM1PCIAH		(PCIX0_CFGBASE + 0x008c)
#define PCIX0_POM2SA		(PCIX0_CFGBASE + 0x0090)

#define PCIX0_PIM0SA		(PCIX0_CFGBASE + 0x0098)
#define PCIX0_PIM0LAL		(PCIX0_CFGBASE + 0x009c)
#define PCIX0_PIM0LAH		(PCIX0_CFGBASE + 0x00a0)
#define PCIX0_PIM1SA		(PCIX0_CFGBASE + 0x00a4)
#define PCIX0_PIM1LAL		(PCIX0_CFGBASE + 0x00a8)
#define PCIX0_PIM1LAH		(PCIX0_CFGBASE + 0x00ac)
#define PCIX0_PIM2SA		(PCIX0_CFGBASE + 0x00b0)
#define PCIX0_PIM2LAL		(PCIX0_CFGBASE + 0x00b4)
#define PCIX0_PIM2LAH		(PCIX0_CFGBASE + 0x00b8)

#define PCIX0_STS		(PCIX0_CFGBASE + 0x00e0)

#endif /* !defined(CONFIG_440EP) !defined(CONFIG_440GR) */

/******************************************************************************
 * GPIO macro register defines
 ******************************************************************************/
#if defined(CONFIG_440GP)
#define GPIO_BASE0             (CFG_PERIPHERAL_BASE+0x00000700)

#define GPIO0_OR               (GPIO_BASE0+0x0)
#define GPIO0_TCR              (GPIO_BASE0+0x4)
#define GPIO0_ODR              (GPIO_BASE0+0x18)
#define GPIO0_IR               (GPIO_BASE0+0x1C)
#endif /* CONFIG_440GP */

#if defined(CONFIG_440EP) || defined(CONFIG_440GR)
#define GPIO_BASE0             (CFG_PERIPHERAL_BASE+0x00000B00)
#define GPIO_BASE1             (CFG_PERIPHERAL_BASE+0x00000C00)

#define GPIO0_OR               (GPIO_BASE0+0x0)
#define GPIO0_TCR              (GPIO_BASE0+0x4)
#define GPIO0_OSRL             (GPIO_BASE0+0x8)
#define GPIO0_OSRH             (GPIO_BASE0+0xC)
#define GPIO0_TSRL             (GPIO_BASE0+0x10)
#define GPIO0_TSRH             (GPIO_BASE0+0x14)
#define GPIO0_ODR              (GPIO_BASE0+0x18)
#define GPIO0_IR               (GPIO_BASE0+0x1C)
#define GPIO0_RR1              (GPIO_BASE0+0x20)
#define GPIO0_RR2              (GPIO_BASE0+0x24)
#define GPIO0_RR3	       (GPIO_BASE0+0x28)
#define GPIO0_ISR1L            (GPIO_BASE0+0x30)
#define GPIO0_ISR1H            (GPIO_BASE0+0x34)
#define GPIO0_ISR2L            (GPIO_BASE0+0x38)
#define GPIO0_ISR2H            (GPIO_BASE0+0x3C)
#define GPIO0_ISR3L            (GPIO_BASE0+0x40)
#define GPIO0_ISR3H            (GPIO_BASE0+0x44)

#define GPIO1_OR               (GPIO_BASE1+0x0)
#define GPIO1_TCR              (GPIO_BASE1+0x4)
#define GPIO1_OSRL             (GPIO_BASE1+0x8)
#define GPIO1_OSRH             (GPIO_BASE1+0xC)
#define GPIO1_TSRL             (GPIO_BASE1+0x10)
#define GPIO1_TSRH             (GPIO_BASE1+0x14)
#define GPIO1_ODR              (GPIO_BASE1+0x18)
#define GPIO1_IR               (GPIO_BASE1+0x1C)
#define GPIO1_RR1              (GPIO_BASE1+0x20)
#define GPIO1_RR2              (GPIO_BASE1+0x24)
#define GPIO1_RR3              (GPIO_BASE1+0x28)
#define GPIO1_ISR1L            (GPIO_BASE1+0x30)
#define GPIO1_ISR1H            (GPIO_BASE1+0x34)
#define GPIO1_ISR2L            (GPIO_BASE1+0x38)
#define GPIO1_ISR2H            (GPIO_BASE1+0x3C)
#define GPIO1_ISR3L            (GPIO_BASE1+0x40)
#define GPIO1_ISR3H            (GPIO_BASE1+0x44)
#endif

/*
 * Macros for accessing the indirect EBC registers
 */
#define mtebc(reg, data)	mtdcr(ebccfga,reg);mtdcr(ebccfgd,data)
#define mfebc(reg, data)	mtdcr(ebccfga,reg);data = mfdcr(ebccfgd)

/*
 * Macros for accessing the indirect SDRAM controller registers
 */
#define mtsdram(reg, data)	mtdcr(memcfga,reg);mtdcr(memcfgd,data)
#define mfsdram(reg, data)	mtdcr(memcfga,reg);data = mfdcr(memcfgd)

/*
 * Macros for accessing the indirect clocking controller registers
 */
#define mtclk(reg, data)	mtdcr(clkcfga,reg);mtdcr(clkcfgd,data)
#define mfclk(reg, data)	mtdcr(clkcfga,reg);data = mfdcr(clkcfgd)

/*
 * Macros for accessing the sdr controller registers
 */
#define mtsdr(reg, data)	mtdcr(sdrcfga,reg);mtdcr(sdrcfgd,data)
#define mfsdr(reg, data)	mtdcr(sdrcfga,reg);data = mfdcr(sdrcfgd)


#ifndef __ASSEMBLY__

typedef struct {
	unsigned long pllFwdDivA;
	unsigned long pllFwdDivB;
	unsigned long pllFbkDiv;
	unsigned long pllOpbDiv;
	unsigned long pllPciDiv;
	unsigned long pllExtBusDiv;
	unsigned long freqVCOMhz;	/* in MHz			   */
	unsigned long freqProcessor;
	unsigned long freqTmrClk;
	unsigned long freqPLB;
	unsigned long freqOPB;
	unsigned long freqEPB;
	unsigned long freqPCI;
	unsigned long pciIntArbEn;            /* Internal PCI arbiter is enabled */
	unsigned long pciClkSync;             /* PCI clock is synchronous        */
} PPC440_SYS_INFO;

#endif	/* _ASMLANGUAGE */

#define RESET_VECTOR		0xfffffffc
#define CACHELINE_MASK		(CFG_CACHELINE_SIZE - 1) /* Address mask for		*/
							 /* cache line aligned data.	*/

#endif	/* __PPC440_H__ */
