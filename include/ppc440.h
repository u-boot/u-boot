/*----------------------------------------------------------------------------+
|
|       This source code has been made available to you by IBM on an AS-IS
|       basis.  Anyone receiving this source is licensed under IBM
|       copyrights to use it in any way he or she deems fit, including
|       copying it, modifying it, compiling it, and redistributing it either
|       with or without modifications.  No license under IBM patents or
|       patent applications is to be implied by the copyright license.
|
|       Any user of this software should understand that IBM cannot provide
|       technical support for this software and will not be responsible for
|       any consequences resulting from the use of this software.
|
|       Any person who transfers this source code or any derivative work
|       must include the IBM copyright notice, this paragraph, and the
|       preceding two paragraphs in the transferred software.
|
|       COPYRIGHT   I B M   CORPORATION 1999
|       LICENSED MATERIAL  -  PROGRAM PROPERTY OF I B M
+----------------------------------------------------------------------------*/

#ifndef	__PPC440_H__
#define __PPC440_H__

/*--------------------------------------------------------------------- */
/* Special Purpose Registers						*/
/*--------------------------------------------------------------------- */
	#define  dec	0x016	/* decrementer */
	#define  srr0	0x01a	/* save/restore register 0 */
	#define  srr1	0x01b	/* save/restore register 1 */
	#define  pid	0x030	/* process id */
	#define  decar	0x036	/* decrementer auto-reload */
	#define  csrr0	0x03a	/* critical save/restore register 0 */
	#define  csrr1	0x03b	/* critical save/restore register 1 */
	#define  dear	0x03d	/* data exception address register */
	#define  esr	0x03e	/* exception syndrome register */
	#define  ivpr	0x03f	/* interrupt prefix register */
	#define  usprg0	0x100	/* user special purpose register general 0 */
	#define  usprg1	0x110	/* user special purpose register general 1 */
	#define  sprg1	0x111	/* special purpose register general 1 */
	#define  sprg2	0x112	/* special purpose register general 2 */
	#define  sprg3	0x113	/* special purpose register general 3 */
	#define  sprg4	0x114	/* special purpose register general 4 */
	#define  sprg5	0x115	/* special purpose register general 5 */
	#define  sprg6	0x116	/* special purpose register general 6 */
	#define  sprg7	0x117	/* special purpose register general 7 */
	#define  tbl	0x11c	/* time base lower (supervisor)*/
	#define  tbu	0x11d	/* time base upper (supervisor)*/
	#define  pir	0x11e	/* processor id register */
	/*#define  pvr	0x11f	 processor version register */
	#define  dbsr	0x130	/* debug status register */
	#define  dbcr0	0x134	/* debug control register 0 */
	#define  dbcr1	0x135	/* debug control register 1 */
	#define  dbcr2	0x136	/* debug control register 2 */
	#define  iac1	0x138	/* instruction address compare 1 */
	#define  iac2	0x139	/* instruction address compare 2 */
	#define  iac3	0x13a	/* instruction address compare 3 */
	#define  iac4	0x13b	/* instruction address compare 4 */
	#define  dac1	0x13c	/* data address compare 1 */
	#define  dac2	0x13d	/* data address compare 2 */
	#define  dvc1	0x13e	/* data value compare 1 */
	#define  dvc2	0x13f	/* data value compare 2 */
	#define  tsr	0x150	/* timer status register */
	#define  tcr	0x154	/* timer control register */
	#define  ivor0	0x190	/* interrupt vector offset register 0 */
	#define  ivor1	0x191	/* interrupt vector offset register 1 */
	#define  ivor2	0x192	/* interrupt vector offset register 2 */
	#define  ivor3	0x193	/* interrupt vector offset register 3 */
	#define  ivor4	0x194	/* interrupt vector offset register 4 */
	#define  ivor5	0x195	/* interrupt vector offset register 5 */
	#define  ivor6	0x196	/* interrupt vector offset register 6 */
	#define  ivor7	0x197	/* interrupt vector offset register 7 */
	#define  ivor8	0x198	/* interrupt vector offset register 8 */
	#define  ivor9	0x199	/* interrupt vector offset register 9 */
	#define  ivor10	0x19a	/* interrupt vector offset register 10 */
	#define  ivor11	0x19b	/* interrupt vector offset register 11 */
	#define  ivor12	0x19c	/* interrupt vector offset register 12 */
	#define  ivor13	0x19d	/* interrupt vector offset register 13 */
	#define  ivor14	0x19e	/* interrupt vector offset register 14 */
	#define  ivor15	0x19f	/* interrupt vector offset register 15 */
	#define  inv0	0x370	/* instruction cache normal victim 0 */
	#define  inv1	0x371	/* instruction cache normal victim 1 */
	#define  inv2	0x372	/* instruction cache normal victim 2 */
	#define  inv3	0x373	/* instruction cache normal victim 3 */
	#define  itv0	0x374	/* instruction cache transient victim 0 */
	#define  itv1	0x375	/* instruction cache transient victim 1 */
	#define  itv2	0x376	/* instruction cache transient victim 2 */
	#define  itv3	0x377	/* instruction cache transient victim 3 */
	#define  dnv0	0x390	/* data cache normal victim 0 */
	#define  dnv1	0x391	/* data cache normal victim 1 */
	#define  dnv2	0x392	/* data cache normal victim 2 */
	#define  dnv3	0x393	/* data cache normal victim 3 */
	#define  dtv0	0x394	/* data cache transient victim 0 */
	#define  dtv1	0x395	/* data cache transient victim 1 */
	#define  dtv2	0x396	/* data cache transient victim 2 */
	#define  dtv3	0x397	/* data cache transient victim 3 */
	#define  dvlim	0x398	/* data cache victim limit */
	#define  ivlim	0x399	/* instruction cache victim limit */
	#define  rstcfg	0x39b	/* reset configuration */
	#define  dcdbtrl 0x39c	/* data cache debug tag register low */
	#define  dcdbtrh 0x39d	/* data cache debug tag register high */
	#define  icdbtrl 0x39e	/* instruction cache debug tag register low */
	#define  icdbtrh 0x39f	/* instruction cache debug tag register high */
	#define  mmucr	0x3b2	/* mmu control register */
	#define  ccr0	0x3b3	/* core configuration register 0 */
	#define  icdbdr 0x3d3	/* instruction cache debug data register */
	#define  dbdr	0x3f3	/* debug data register */

/******************************************************************************
 * DCRs & Related
 ******************************************************************************/

/*-----------------------------------------------------------------------------
 | SDRAM Controller
 +----------------------------------------------------------------------------*/
#define SDRAM_DCR_BASE 0x10
#define memcfga  (SDRAM_DCR_BASE+0x0)   /* Memory configuration address reg */
#define memcfgd  (SDRAM_DCR_BASE+0x1)   /* Memory configuration data reg    */

  /* values for memcfga register - indirect addressing of these regs        */
  #define mem_besr0_clr     0x0000  /* bus error status reg 0 (clr)         */
  #define mem_besr0_set     0x0004  /* bus error status reg 0 (set)         */
  #define mem_besr1_clr     0x0008  /* bus error status reg 1 (clr)         */
  #define mem_besr1_set     0x000c  /* bus error status reg 1 (set)         */
  #define mem_bear          0x0010  /* bus error address reg                */
  #define mem_mirq_clr      0x0011  /* bus master interrupt (clr)           */
  #define mem_mirq_set      0x0012  /* bus master interrupt (set)           */
  #define mem_slio          0x0018  /* ddr sdram slave interface options    */
  #define mem_cfg0          0x0020  /* ddr sdram options 0                  */
  #define mem_cfg1          0x0021  /* ddr sdram options 1                  */
  #define mem_devopt        0x0022  /* ddr sdram device options             */
  #define mem_mcsts         0x0024  /* memory controller status             */
  #define mem_rtr           0x0030  /* refresh timer register               */
  #define mem_pmit          0x0034  /* power management idle timer          */
  #define mem_uabba         0x0038  /* plb UABus base address               */
  #define mem_b0cr          0x0040  /* ddr sdram bank 0 configuration       */
  #define mem_b1cr          0x0044  /* ddr sdram bank 1 configuration       */
  #define mem_b2cr          0x0048  /* ddr sdram bank 2 configuration       */
  #define mem_b3cr          0x004c  /* ddr sdram bank 3 configuration       */
  #define mem_tr0           0x0080  /* sdram timing register 0              */
  #define mem_tr1           0x0081  /* sdram timing register 1              */
  #define mem_clktr         0x0082  /* ddr clock timing register            */
  #define mem_wddctr        0x0083  /* write data/dm/dqs clock timing reg   */
  #define mem_dlycal        0x0084  /* delay line calibration register      */
  #define mem_eccesr        0x0098  /* ECC error status                     */

/*-----------------------------------------------------------------------------
 | Extrnal Bus Controller
 +----------------------------------------------------------------------------*/
#define EBC_DCR_BASE 0x12
#define ebccfga (EBC_DCR_BASE+0x0)   /* External bus controller addr reg     */
#define ebccfgd (EBC_DCR_BASE+0x1)   /* External bus controller data reg     */
  /* values for ebccfga register - indirect addressing of these regs */
  #define pb0cr       0x00    /* periph bank 0 config reg            */
  #define pb1cr       0x01    /* periph bank 1 config reg            */
  #define pb2cr       0x02    /* periph bank 2 config reg            */
  #define pb3cr       0x03    /* periph bank 3 config reg            */
  #define pb4cr       0x04    /* periph bank 4 config reg            */
  #define pb5cr       0x05    /* periph bank 5 config reg            */
  #define pb6cr       0x06    /* periph bank 6 config reg            */
  #define pb7cr       0x07    /* periph bank 7 config reg            */
  #define pb0ap       0x10    /* periph bank 0 access parameters     */
  #define pb1ap       0x11    /* periph bank 1 access parameters     */
  #define pb2ap       0x12    /* periph bank 2 access parameters     */
  #define pb3ap       0x13    /* periph bank 3 access parameters     */
  #define pb4ap       0x14    /* periph bank 4 access parameters     */
  #define pb5ap       0x15    /* periph bank 5 access parameters     */
  #define pb6ap       0x16    /* periph bank 6 access parameters     */
  #define pb7ap       0x17    /* periph bank 7 access parameters     */
  #define pbear       0x20    /* periph bus error addr reg           */
  #define pbesr       0x21    /* periph bus error status reg         */
  #define xbcfg       0x23    /* external bus configuration reg      */
  #define xbcid       0x23    /* external bus core id reg            */

/*-----------------------------------------------------------------------------
 | Internal SRAM
 +----------------------------------------------------------------------------*/
#define ISRAM0_DCR_BASE 0x020
#define isram0_sb0cr    (ISRAM0_DCR_BASE+0x00)  /* SRAM bank config 0*/
#define isram0_sb1cr    (ISRAM0_DCR_BASE+0x01)  /* SRAM bank config 1*/
#define isram0_sb2cr    (ISRAM0_DCR_BASE+0x02)  /* SRAM bank config 2*/
#define isram0_sb3cr    (ISRAM0_DCR_BASE+0x03)  /* SRAM bank config 3*/
#define isram0_bear     (ISRAM0_DCR_BASE+0x04)  /* SRAM bus error addr reg */
#define isram0_besr0    (ISRAM0_DCR_BASE+0x05)  /* SRAM bus error status reg 0 */
#define isram0_besr1    (ISRAM0_DCR_BASE+0x06)  /* SRAM bus error status reg 1 */
#define isram0_pmeg     (ISRAM0_DCR_BASE+0x07)  /* SRAM power management */
#define isram0_cid      (ISRAM0_DCR_BASE+0x08)  /* SRAM bus core id reg */
#define isram0_revid    (ISRAM0_DCR_BASE+0x09)  /* SRAM bus revision id reg */
#define isram0_dpc      (ISRAM0_DCR_BASE+0x0a)  /* SRAM data parity check reg */

/*-----------------------------------------------------------------------------
 | On-Chip Buses
 +----------------------------------------------------------------------------*/
/* TODO: as needed */

/*-----------------------------------------------------------------------------
 | Clocking, Power Management and Chip Control
 +----------------------------------------------------------------------------*/
#define CNTRL_DCR_BASE 0x0b0

#define cpc0_sr     (CNTRL_DCR_BASE+0x00)   /* CPM status register          */
#define cpc0_er     (CNTRL_DCR_BASE+0x01)   /* CPM enable register          */
#define cpc0_fr     (CNTRL_DCR_BASE+0x02)   /* CPM force register           */

#define cpc0_sys0   (CNTRL_DCR_BASE+0x30)   /* System configuration reg 0   */
#define cpc0_sys1   (CNTRL_DCR_BASE+0x31)   /* System configuration reg 1   */
#define cpc0_cust0  (CNTRL_DCR_BASE+0x32)   /* Customer configuration reg 0 */
#define cpc0_cust1  (CNTRL_DCR_BASE+0x33)   /* Customer configuration reg 1 */

#define cpc0_strp0	(CNTRL_DCR_BASE+0x34)	/* Power-on config reg 0 (RO)	*/
#define cpc0_strp1	(CNTRL_DCR_BASE+0x35)	/* Power-on config reg 1 (RO)	*/
#define cpc0_strp2	(CNTRL_DCR_BASE+0x36)	/* Power-on config reg 2 (RO)	*/
#define cpc0_strp3	(CNTRL_DCR_BASE+0x37)	/* Power-on config reg 3 (RO)	*/

#define cntrl0      (CNTRL_DCR_BASE+0x3b)   /* Control 0 register           */
#define cntrl1      (CNTRL_DCR_BASE+0x3a)   /* Control 1 register           */

/*-----------------------------------------------------------------------------
 | Universal interrupt controller
 +----------------------------------------------------------------------------*/
#define UIC0_DCR_BASE 0xc0
#define uic0sr  (UIC0_DCR_BASE+0x0)   /* UIC0 status                       */
#define uic0er  (UIC0_DCR_BASE+0x2)   /* UIC0 enable                       */
#define uic0cr  (UIC0_DCR_BASE+0x3)   /* UIC0 critical                     */
#define uic0pr  (UIC0_DCR_BASE+0x4)   /* UIC0 polarity                     */
#define uic0tr  (UIC0_DCR_BASE+0x5)   /* UIC0 triggering                   */
#define uic0msr (UIC0_DCR_BASE+0x6)   /* UIC0 masked status                */
#define uic0vr  (UIC0_DCR_BASE+0x7)   /* UIC0 vector                       */
#define uic0vcr (UIC0_DCR_BASE+0x8)   /* UIC0 vector configuration         */

#define UIC1_DCR_BASE 0xd0
#define uic1sr  (UIC1_DCR_BASE+0x0)   /* UIC1 status                       */
#define uic1er  (UIC1_DCR_BASE+0x2)   /* UIC1 enable                       */
#define uic1cr  (UIC1_DCR_BASE+0x3)   /* UIC1 critical                     */
#define uic1pr  (UIC1_DCR_BASE+0x4)   /* UIC1 polarity                     */
#define uic1tr  (UIC1_DCR_BASE+0x5)   /* UIC1 triggering                   */
#define uic1msr (UIC1_DCR_BASE+0x6)   /* UIC1 masked status                */
#define uic1vr  (UIC1_DCR_BASE+0x7)   /* UIC1 vector                       */
#define uic1vcr (UIC1_DCR_BASE+0x8)   /* UIC1 vector configuration         */

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
#define dmacr0  (DMA_DCR_BASE+0x00)  /* DMA channel control register 0       */
#define dmact0  (DMA_DCR_BASE+0x01)  /* DMA count register 0                 */
#define dmasah0 (DMA_DCR_BASE+0x02)  /* DMA source address high 0            */
#define dmasal0 (DMA_DCR_BASE+0x03)  /* DMA source address low 0             */
#define dmadah0 (DMA_DCR_BASE+0x04)  /* DMA destination address high 0       */
#define dmadal0 (DMA_DCR_BASE+0x05)  /* DMA destination address low 0        */
#define dmasgh0 (DMA_DCR_BASE+0x06)  /* DMA scatter/gather desc addr high 0  */
#define dmasgl0 (DMA_DCR_BASE+0x07)  /* DMA scatter/gather desc addr low 0   */
#define dmacr1  (DMA_DCR_BASE+0x08)  /* DMA channel control register 1       */
#define dmact1  (DMA_DCR_BASE+0x09)  /* DMA count register 1                 */
#define dmasah1 (DMA_DCR_BASE+0x0a)  /* DMA source address high 1            */
#define dmasal1 (DMA_DCR_BASE+0x0b)  /* DMA source address low 1             */
#define dmadah1 (DMA_DCR_BASE+0x0c)  /* DMA destination address high 1       */
#define dmadal1 (DMA_DCR_BASE+0x0d)  /* DMA destination address low 1        */
#define dmasgh1 (DMA_DCR_BASE+0x0e)  /* DMA scatter/gather desc addr high 1  */
#define dmasgl1 (DMA_DCR_BASE+0x0f)  /* DMA scatter/gather desc addr low 1   */
#define dmacr2  (DMA_DCR_BASE+0x10)  /* DMA channel control register 2       */
#define dmact2  (DMA_DCR_BASE+0x11)  /* DMA count register 2                 */
#define dmasah2 (DMA_DCR_BASE+0x12)  /* DMA source address high 2            */
#define dmasal2 (DMA_DCR_BASE+0x13)  /* DMA source address low 2             */
#define dmadah2 (DMA_DCR_BASE+0x14)  /* DMA destination address high 2       */
#define dmadal2 (DMA_DCR_BASE+0x15)  /* DMA destination address low 2        */
#define dmasgh2 (DMA_DCR_BASE+0x16)  /* DMA scatter/gather desc addr high 2  */
#define dmasgl2 (DMA_DCR_BASE+0x17)  /* DMA scatter/gather desc addr low 2   */
#define dmacr3  (DMA_DCR_BASE+0x18)  /* DMA channel control register 2       */
#define dmact3  (DMA_DCR_BASE+0x19)  /* DMA count register 2                 */
#define dmasah3 (DMA_DCR_BASE+0x1a)  /* DMA source address high 2            */
#define dmasal3 (DMA_DCR_BASE+0x1b)  /* DMA source address low 2             */
#define dmadah3 (DMA_DCR_BASE+0x1c)  /* DMA destination address high 2       */
#define dmadal3 (DMA_DCR_BASE+0x1d)  /* DMA destination address low 2        */
#define dmasgh3 (DMA_DCR_BASE+0x1e)  /* DMA scatter/gather desc addr high 2  */
#define dmasgl3 (DMA_DCR_BASE+0x1f)  /* DMA scatter/gather desc addr low 2   */
#define dmasr   (DMA_DCR_BASE+0x20)  /* DMA status register                  */
#define dmasgc  (DMA_DCR_BASE+0x23)  /* DMA scatter/gather command register  */
#define dmaslp  (DMA_DCR_BASE+0x25)  /* DMA sleep mode register              */
#define dmapol  (DMA_DCR_BASE+0x26)  /* DMA polarity configuration register  */

/*-----------------------------------------------------------------------------
 | Memory Access Layer
 +----------------------------------------------------------------------------*/
#define MAL_DCR_BASE 0x180
#define malmcr      (MAL_DCR_BASE+0x00) /* MAL Config reg                   */
#define malesr      (MAL_DCR_BASE+0x01) /* Error Status reg (Read/Clear)    */
#define malier      (MAL_DCR_BASE+0x02) /* Interrupt enable reg             */
#define maldbr      (MAL_DCR_BASE+0x03) /* Mal Debug reg (Read only)        */
#define maltxcasr   (MAL_DCR_BASE+0x04) /* TX Channel active reg (set)      */
#define maltxcarr   (MAL_DCR_BASE+0x05) /* TX Channel active reg (Reset)    */
#define maltxeobisr (MAL_DCR_BASE+0x06) /* TX End of buffer int status reg  */
#define maltxdeir   (MAL_DCR_BASE+0x07) /* TX Descr. Error Int reg          */
#define maltxtattrr (MAL_DCR_BASE+0x08) /* TX PLB attribute reg             */
#define maltxbattr  (MAL_DCR_BASE+0x09) /* TX descriptor base addr reg      */
#define malrxcasr   (MAL_DCR_BASE+0x10) /* RX Channel active reg (set)      */
#define malrxcarr   (MAL_DCR_BASE+0x11) /* RX Channel active reg (Reset)    */
#define malrxeobisr (MAL_DCR_BASE+0x12) /* RX End of buffer int status reg  */
#define malrxdeir   (MAL_DCR_BASE+0x13) /* RX Descr. Error Int reg          */
#define malrxtattrr (MAL_DCR_BASE+0x14) /* RX PLB attribute reg             */
#define malrxbattr  (MAL_DCR_BASE+0x15) /* RX descriptor base addr reg      */
#define maltxctp0r  (MAL_DCR_BASE+0x20) /* TX 0 Channel table pointer reg   */
#define maltxctp1r  (MAL_DCR_BASE+0x21) /* TX 1 Channel table pointer reg   */
#define malrxctp0r  (MAL_DCR_BASE+0x40) /* RX 0 Channel table pointer reg   */
#define malrxctp1r  (MAL_DCR_BASE+0x41) /* RX 1 Channel table pointer reg   */
#define malrcbs0    (MAL_DCR_BASE+0x60) /* RX 0 Channel buffer size reg     */

/*---------------------------------------------------------------------------+
|  Universal interrupt controller 0 interrupts (UIC0)
+---------------------------------------------------------------------------*/
#define UIC_U0          0x80000000      /* UART 0                           */
#define UIC_U1          0x40000000      /* UART 1                           */
#define UIC_IIC0        0x20000000      /* IIC                              */
#define UIC_IIC1        0x10000000      /* IIC                              */
#define UIC_PIM         0x08000000      /* PCI inbound message              */
#define UIC_PCRW        0x04000000      /* PCI command register write       */
#define UIC_PPM         0x02000000      /* PCI power management             */
#define UIC_MSI0        0x01000000      /* PCI MSI level 0                  */
#define UIC_MSI1        0x00800000      /* PCI MSI level 1                  */
#define UIC_MSI2        0x00400000      /* PCI MSI level 2                  */
#define UIC_MTE         0x00200000      /* MAL TXEOB                        */
#define UIC_MRE         0x00100000      /* MAL RXEOB                        */
#define UIC_D0          0x00080000      /* DMA channel 0                    */
#define UIC_D1          0x00040000      /* DMA channel 1                    */
#define UIC_D2          0x00020000      /* DMA channel 2                    */
#define UIC_D3          0x00010000      /* DMA channel 3                    */
#define UIC_RSVD0       0x00008000      /* Reserved                         */
#define UIC_RSVD1       0x00004000      /* Reserved                         */
#define UIC_CT0         0x00002000      /* GPT compare timer 0              */
#define UIC_CT1         0x00001000      /* GPT compare timer 1              */
#define UIC_CT2         0x00000800      /* GPT compare timer 2              */
#define UIC_CT3         0x00000400      /* GPT compare timer 3              */
#define UIC_CT4         0x00000200      /* GPT compare timer 4              */
#define UIC_EIR0        0x00000100      /* External interrupt 0             */
#define UIC_EIR1        0x00000080      /* External interrupt 1             */
#define UIC_EIR2        0x00000040      /* External interrupt 2             */
#define UIC_EIR3        0x00000020      /* External interrupt 3             */
#define UIC_EIR4        0x00000010      /* External interrupt 4             */
#define UIC_EIR5        0x00000008      /* External interrupt 5             */
#define UIC_EIR6        0x00000004      /* External interrupt 6             */
#define UIC_UIC1NC      0x00000002      /* UIC1 non-critical interrupt      */
#define UIC_UIC1C       0x00000001      /* UIC1 critical interrupt          */

/* For compatibility with 405 code */
#define UIC_MAL_TXEOB   UIC_MTE
#define UIC_MAL_RXEOB   UIC_MRE

/*---------------------------------------------------------------------------+
|  Universal interrupt controller 1 interrupts (UIC1)
+---------------------------------------------------------------------------*/
#define UIC_MS          0x80000000      /* MAL SERR                         */
#define UIC_MTDE        0x40000000      /* MAL TXDE                         */
#define UIC_MRDE        0x20000000      /* MAL RXDE                         */
#define UIC_DEUE        0x10000000      /* DDR SDRAM ECC uncorrectible error*/
#define UIC_DECE        0x08000000      /* DDR SDRAM correctible error      */
#define UIC_EBCO        0x04000000      /* EBCO interrupt status            */
#define UIC_EBMI        0x02000000      /* EBMI interrupt status            */
#define UIC_OPB         0x01000000      /* OPB to PLB bridge interrupt stat */
#define UIC_MSI3        0x00800000      /* PCI MSI level 3                  */
#define UIC_MSI4        0x00400000      /* PCI MSI level 4                  */
#define UIC_MSI5        0x00200000      /* PCI MSI level 5                  */
#define UIC_MSI6        0x00100000      /* PCI MSI level 6                  */
#define UIC_MSI7        0x00080000      /* PCI MSI level 7                  */
#define UIC_MSI8        0x00040000      /* PCI MSI level 8                  */
#define UIC_MSI9        0x00020000      /* PCI MSI level 9                  */
#define UIC_MSI10       0x00010000      /* PCI MSI level 10                 */
#define UIC_MSI11       0x00008000      /* PCI MSI level 11                 */
#define UIC_PPMI        0x00004000      /* PPM interrupt status             */
#define UIC_EIR7        0x00002000      /* External interrupt 7             */
#define UIC_EIR8        0x00001000      /* External interrupt 8             */
#define UIC_EIR9        0x00000800      /* External interrupt 9             */
#define UIC_EIR10       0x00000400      /* External interrupt 10            */
#define UIC_EIR11       0x00000200      /* External interrupt 11            */
#define UIC_EIR12       0x00000100      /* External interrupt 12            */
#define UIC_SRE         0x00000080      /* Serial ROM error                 */
#define UIC_RSVD2       0x00000040      /* Reserved                         */
#define UIC_RSVD3       0x00000020      /* Reserved                         */
#define UIC_PAE         0x00000010      /* PCI asynchronous error           */
#define UIC_ETH0        0x00000008      /* Ethernet 0                       */
#define UIC_EWU0        0x00000004      /* Ethernet 0 wakeup                */
#define UIC_ETH1        0x00000002      /* Ethernet 1                       */
#define UIC_EWU1        0x00000001      /* Ethernet 1 wakeup                */

/* For compatibility with 405 code */
#define UIC_MAL_SERR    UIC_MS
#define UIC_MAL_TXDE    UIC_MTDE
#define UIC_MAL_RXDE    UIC_MRDE
#define UIC_ENET        UIC_ETH0

/*-----------------------------------------------------------------------------+
|  Clocking
+-----------------------------------------------------------------------------*/
#define PLLSYS0_TUNE_MASK       0xffc00000      /* PLL TUNE bits            */
#define PLLSYS0_FB_DIV_MASK     0x003c0000      /* Feedback divisor         */
#define PLLSYS0_FWD_DIV_A_MASK  0x00038000      /* Forward divisor A        */
#define PLLSYS0_FWD_DIV_B_MASK  0x00007000      /* Forward divisor B        */
#define PLLSYS0_OPB_DIV_MASK    0x00000c00      /* OPB divisor              */
#define PLLSYS0_EPB_DIV_MASK    0x00000300      /* EPB divisor              */
#define PLLSYS0_EXTSL_MASK      0x00000080      /* PerClk feedback path     */
#define PLLSYS0_RW_MASK         0x00000060      /* ROM width                */
#define PLLSYS0_RL_MASK         0x00000010      /* ROM location             */
#define PLLSYS0_ZMII_SEL_MASK   0x0000000c      /* ZMII selection           */
#define PLLSYS0_BYPASS_MASK     0x00000002      /* Bypass PLL               */
#define PLLSYS0_NTO1_MASK       0x00000001      /* CPU:PLB N-to-1 ratio     */

#define PLL_VCO_FREQ_MIN        500             /* Min VCO freq (MHz)       */
#define PLL_VCO_FREQ_MAX        1000            /* Max VCO freq (MHz)       */
#define PLL_CPU_FREQ_MAX        400             /* Max CPU freq (MHz)       */
#define PLL_PLB_FREQ_MAX        133             /* Max PLB freq (MHz)       */

/*-----------------------------------------------------------------------------
| IIC Register Offsets
'----------------------------------------------------------------------------*/
#define    IICMDBUF         0x00
#define    IICSDBUF         0x02
#define    IICLMADR         0x04
#define    IICHMADR         0x05
#define    IICCNTL          0x06
#define    IICMDCNTL        0x07
#define    IICSTS           0x08
#define    IICEXTSTS        0x09
#define    IICLSADR         0x0A
#define    IICHSADR         0x0B
#define    IICCLKDIV        0x0C
#define    IICINTRMSK       0x0D
#define    IICXFRCNT        0x0E
#define    IICXTCNTLSS      0x0F
#define    IICDIRECTCNTL    0x10

/*-----------------------------------------------------------------------------
| UART Register Offsets
'----------------------------------------------------------------------------*/
#define		DATA_REG	0x00
#define		DL_LSB    	0x00
#define		DL_MSB  	0x01
#define		INT_ENABLE      0x01
#define		FIFO_CONTROL    0x02
#define		LINE_CONTROL    0x03
#define		MODEM_CONTROL   0x04
#define		LINE_STATUS  	0x05
#define		MODEM_STATUS    0x06
#define		SCRATCH         0x07

/*-----------------------------------------------------------------------------
| PCI Internal Registers et. al. (accessed via plb)
+----------------------------------------------------------------------------*/
#define		PCIX0_CFGADR		(CFG_PCI_BASE + 0x0ec00000)
#define		PCIX0_CFGDATA		(CFG_PCI_BASE + 0x0ec00004)
#define		PCIX0_CFGBASE		(CFG_PCI_BASE + 0x0ec80000)
#define		PCIX0_IOBASE		(CFG_PCI_BASE + 0x08000000)

#define		PCIX0_VENDID		(PCIX0_CFGBASE + PCI_VENDOR_ID )
#define		PCIX0_DEVID			(PCIX0_CFGBASE + PCI_DEVICE_ID )
#define		PCIX0_CMD			(PCIX0_CFGBASE + PCI_COMMAND )
#define		PCIX0_STATUS		(PCIX0_CFGBASE + PCI_STATUS )
#define		PCIX0_REVID			(PCIX0_CFGBASE + PCI_REVISION_ID )
#define		PCIX0_CLS			(PCIX0_CFGBASE + PCI_CLASS_CODE)
#define		PCIX0_CACHELS		(PCIX0_CFGBASE + PCI_CACHE_LINE_SIZE )
#define		PCIX0_LATTIM		(PCIX0_CFGBASE + PCI_LATENCY_TIMER )
#define		PCIX0_HDTYPE		(PCIX0_CFGBASE + PCI_HEADER_TYPE )
#define		PCIX0_BIST			(PCIX0_CFGBASE + PCI_BIST )
#define		PCIX0_BAR0			(PCIX0_CFGBASE + PCI_BASE_ADDRESS_0 )
#define		PCIX0_BAR1			(PCIX0_CFGBASE + PCI_BASE_ADDRESS_1 )
#define		PCIX0_BAR2			(PCIX0_CFGBASE + PCI_BASE_ADDRESS_2 )
#define		PCIX0_BAR3			(PCIX0_CFGBASE + PCI_BASE_ADDRESS_3 )
#define		PCIX0_BAR4			(PCIX0_CFGBASE + PCI_BASE_ADDRESS_4 )
#define		PCIX0_BAR5			(PCIX0_CFGBASE + PCI_BASE_ADDRESS_5 )
#define		PCIX0_CISPTR		(PCIX0_CFGBASE + PCI_CARDBUS_CIS )
#define		PCIX0_SBSYSVID		(PCIX0_CFGBASE + PCI_SUBSYSTEM_VENDOR_ID )
#define		PCIX0_SBSYSID		(PCIX0_CFGBASE + PCI_SUBSYSTEM_ID )
#define		PCIX0_EROMBA		(PCIX0_CFGBASE + PCI_ROM_ADDRESS )
#define		PCIX0_CAP			(PCIX0_CFGBASE + PCI_CAPABILITY_LIST )
#define		PCIX0_RES0			(PCIX0_CFGBASE + 0x0035 )
#define		PCIX0_RES1			(PCIX0_CFGBASE + 0x0036 )
#define		PCIX0_RES2			(PCIX0_CFGBASE + 0x0038 )
#define		PCIX0_INTLN			(PCIX0_CFGBASE + PCI_INTERRUPT_LINE )
#define		PCIX0_INTPN			(PCIX0_CFGBASE + PCI_INTERRUPT_PIN )
#define		PCIX0_MINGNT		(PCIX0_CFGBASE + PCI_MIN_GNT )
#define		PCIX0_MAXLTNCY		(PCIX0_CFGBASE + PCI_MAX_LAT )

#define     PCIX0_BRDGOPT1      (PCIX0_CFGBASE + 0x0040)
#define     PCIX0_BRDGOPT2      (PCIX0_CFGBASE + 0x0044)

#define		PCIX0_POM0LAL		(PCIX0_CFGBASE + 0x0068)
#define		PCIX0_POM0LAH		(PCIX0_CFGBASE + 0x006c)
#define		PCIX0_POM0SA		(PCIX0_CFGBASE + 0x0070)
#define		PCIX0_POM0PCIAL		(PCIX0_CFGBASE + 0x0074)
#define		PCIX0_POM0PCIAH		(PCIX0_CFGBASE + 0x0078)
#define		PCIX0_POM1LAL		(PCIX0_CFGBASE + 0x007c)
#define		PCIX0_POM1LAH		(PCIX0_CFGBASE + 0x0080)
#define		PCIX0_POM1SA		(PCIX0_CFGBASE + 0x0084)
#define		PCIX0_POM1PCIAL		(PCIX0_CFGBASE + 0x0088)
#define		PCIX0_POM1PCIAH		(PCIX0_CFGBASE + 0x008c)
#define		PCIX0_POM2SA		(PCIX0_CFGBASE + 0x0090)

#define		PCIX0_PIM0SA		(PCIX0_CFGBASE + 0x0098)
#define		PCIX0_PIM0LAL		(PCIX0_CFGBASE + 0x009c)
#define		PCIX0_PIM0LAH		(PCIX0_CFGBASE + 0x00a0)
#define		PCIX0_PIM1SA		(PCIX0_CFGBASE + 0x00a4)
#define		PCIX0_PIM1LAL		(PCIX0_CFGBASE + 0x00a8)
#define		PCIX0_PIM1LAH		(PCIX0_CFGBASE + 0x00ac)
#define		PCIX0_PIM2SA		(PCIX0_CFGBASE + 0x00b0)
#define		PCIX0_PIM2LAL		(PCIX0_CFGBASE + 0x00b4)
#define		PCIX0_PIM2LAH		(PCIX0_CFGBASE + 0x00b8)

#define     PCIX0_STS           (PCIX0_CFGBASE + 0x00e0)

/*
 * Macros for accessing the indirect EBC registers
 */
#define mtebc(reg, data)  mtdcr(ebccfga,reg);mtdcr(ebccfgd,data)
#define mfebc(reg, data)  mtdcr(ebccfga,reg);data = mfdcr(ebccfgd)

/*
 * Macros for accessing the indirect SDRAM controller registers
 */
#define mtsdram(reg, data)  mtdcr(memcfga,reg);mtdcr(memcfgd,data)
#define mfsdram(reg, data)  mtdcr(memcfga,reg);data = mfdcr(memcfgd)


#ifndef __ASSEMBLY__

typedef struct
{
  unsigned long pllFwdDivA;
  unsigned long pllFwdDivB;
  unsigned long pllFbkDiv;
  unsigned long pllOpbDiv;
  unsigned long pllExtBusDiv;
  unsigned long freqVCOMhz;             /* in MHz                          */
  unsigned long freqProcessor;
  unsigned long freqPLB;
  unsigned long freqOPB;
  unsigned long freqEPB;
} PPC440_SYS_INFO;

#endif  /* _ASMLANGUAGE */

#define RESET_VECTOR	0xfffffffc
#define CACHELINE_MASK	(CFG_CACHELINE_SIZE - 1) /* Address mask for cache
						     line aligned data. */

#endif	/* __PPC440_H__ */

