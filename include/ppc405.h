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

#ifndef	__PPC405_H__
#define __PPC405_H__

/*--------------------------------------------------------------------- */
/* Special Purpose Registers						*/
/*--------------------------------------------------------------------- */
	#define  srr2  0x3de      /* save/restore register 2 */
	#define  srr3  0x3df      /* save/restore register 3 */
	#define  dbsr  0x3f0      /* debug status register */
	#define  dbcr0 0x3f2      /* debug control register 0 */
	#define  dbcr1 0x3bd      /* debug control register 1 */
	#define  iac1  0x3f4      /* instruction address comparator 1 */
	#define  iac2  0x3f5      /* instruction address comparator 2 */
	#define  iac3  0x3b4      /* instruction address comparator 3 */
	#define  iac4  0x3b5      /* instruction address comparator 4 */
	#define  dac1  0x3f6      /* data address comparator 1 */
	#define  dac2  0x3f7      /* data address comparator 2 */
	#define  dccr  0x3fa      /* data cache control register */
	#define  iccr  0x3fb      /* instruction cache control register */
	#define  esr   0x3d4      /* execption syndrome register */
	#define  dear  0x3d5      /* data exeption address register */
	#define  evpr  0x3d6      /* exeption vector prefix register */
	#define  tsr   0x3d8      /* timer status register */
	#define  tcr   0x3da      /* timer control register */
	#define  pit   0x3db      /* programmable interval timer */
	#define  sgr   0x3b9      /* storage guarded reg      */
	#define  dcwr  0x3ba      /* data cache write-thru reg*/
	#define  sler  0x3bb      /* storage little-endian reg */
	#define  cdbcr 0x3d7      /* cache debug cntrl reg    */
	#define  icdbdr 0x3d3     /* instr cache dbug data reg*/
	#define  ccr0  0x3b3      /* core configuration register */
	#define  dvc1  0x3b6      /* data value compare register 1 */
	#define  dvc2  0x3b7      /* data value compare register 2 */
	#define  pid   0x3b1      /* process ID */
	#define  su0r  0x3bc      /* storage user-defined register 0 */
	#define  zpr   0x3b0      /* zone protection regsiter */

	#define  tbl   0x11c      /* time base lower - privileged write */
	#define  tbu   0x11d      /* time base upper - privileged write */

	#define  sprg4r 0x104     /* Special purpose general 4 - read only */
	#define  sprg5r 0x105     /* Special purpose general 5 - read only */
	#define  sprg6r 0x106     /* Special purpose general 6 - read only */
	#define  sprg7r 0x107     /* Special purpose general 7 - read only */
	#define  sprg4w 0x114     /* Special purpose general 4 - write only */
	#define  sprg5w 0x115     /* Special purpose general 5 - write only */
	#define  sprg6w 0x116     /* Special purpose general 6 - write only */
	#define  sprg7w 0x117     /* Special purpose general 7 - write only */

/******************************************************************************
 * Special for PPC405GP
 ******************************************************************************/

/******************************************************************************
 * DMA
 ******************************************************************************/
#define DMA_DCR_BASE 0x100
#define dmacr0  (DMA_DCR_BASE+0x00)  /* DMA channel control register 0       */
#define dmact0  (DMA_DCR_BASE+0x01)  /* DMA count register 0                 */
#define dmada0  (DMA_DCR_BASE+0x02)  /* DMA destination address register 0   */
#define dmasa0  (DMA_DCR_BASE+0x03)  /* DMA source address register 0        */
#define dmasb0  (DMA_DCR_BASE+0x04)  /* DMA scatter/gather descriptor addr 0 */
#define dmacr1  (DMA_DCR_BASE+0x08)  /* DMA channel control register 1       */
#define dmact1  (DMA_DCR_BASE+0x09)  /* DMA count register 1                 */
#define dmada1  (DMA_DCR_BASE+0x0a)  /* DMA destination address register 1   */
#define dmasa1  (DMA_DCR_BASE+0x0b)  /* DMA source address register 1        */
#define dmasb1  (DMA_DCR_BASE+0x0c)  /* DMA scatter/gather descriptor addr 1 */
#define dmacr2  (DMA_DCR_BASE+0x10)  /* DMA channel control register 2       */
#define dmact2  (DMA_DCR_BASE+0x11)  /* DMA count register 2                 */
#define dmada2  (DMA_DCR_BASE+0x12)  /* DMA destination address register 2   */
#define dmasa2  (DMA_DCR_BASE+0x13)  /* DMA source address register 2        */
#define dmasb2  (DMA_DCR_BASE+0x14)  /* DMA scatter/gather descriptor addr 2 */
#define dmacr3  (DMA_DCR_BASE+0x18)  /* DMA channel control register 3       */
#define dmact3  (DMA_DCR_BASE+0x19)  /* DMA count register 3                 */
#define dmada3  (DMA_DCR_BASE+0x1a)  /* DMA destination address register 3   */
#define dmasa3  (DMA_DCR_BASE+0x1b)  /* DMA source address register 3        */
#define dmasb3  (DMA_DCR_BASE+0x1c)  /* DMA scatter/gather descriptor addr 3 */
#define dmasr   (DMA_DCR_BASE+0x20)  /* DMA status register                  */
#define dmasgc  (DMA_DCR_BASE+0x23)  /* DMA scatter/gather command register  */
#define dmaadr  (DMA_DCR_BASE+0x24)  /* DMA address decode register          */

/******************************************************************************
 * Universal interrupt controller
 ******************************************************************************/
#define UIC_DCR_BASE 0xc0
#define uicsr        (UIC_DCR_BASE+0x0)  /* UIC status                       */
#define uicsrs       (UIC_DCR_BASE+0x1)  /* UIC status set                   */
#define uicer        (UIC_DCR_BASE+0x2)  /* UIC enable                       */
#define uiccr        (UIC_DCR_BASE+0x3)  /* UIC critical                     */
#define uicpr        (UIC_DCR_BASE+0x4)  /* UIC polarity                     */
#define uictr        (UIC_DCR_BASE+0x5)  /* UIC triggering                   */
#define uicmsr       (UIC_DCR_BASE+0x6)  /* UIC masked status                */
#define uicvr        (UIC_DCR_BASE+0x7)  /* UIC vector                       */
#define uicvcr       (UIC_DCR_BASE+0x8)  /* UIC vector configuration         */

/*-----------------------------------------------------------------------------+
|  Universal interrupt controller interrupts
+-----------------------------------------------------------------------------*/
#define UIC_UART0     0x80000000      /* UART 0                             */
#define UIC_UART1     0x40000000      /* UART 1                             */
#define UIC_IIC       0x20000000      /* IIC                                */
#define UIC_EXT_MAST  0x10000000      /* External Master                    */
#define UIC_PCI       0x08000000      /* PCI write to command reg           */
#define UIC_DMA0      0x04000000      /* DMA chan. 0                        */
#define UIC_DMA1      0x02000000      /* DMA chan. 1                        */
#define UIC_DMA2      0x01000000      /* DMA chan. 2                        */
#define UIC_DMA3      0x00800000      /* DMA chan. 3                        */
#define UIC_EMAC_WAKE 0x00400000      /* EMAC wake up                       */
#define UIC_MAL_SERR  0x00200000      /* MAL SERR                           */
#define UIC_MAL_TXEOB 0x00100000      /* MAL TXEOB                          */
#define UIC_MAL_RXEOB 0x00080000      /* MAL RXEOB                          */
#define UIC_MAL_TXDE  0x00040000      /* MAL TXDE                           */
#define UIC_MAL_RXDE  0x00020000      /* MAL RXDE                           */
#define UIC_ENET      0x00010000      /* Ethernet0                          */
#define UIC_ENET1     0x00004000      /* Ethernet1 on 405EP                 */
#define UIC_ECC_CE    0x00004000      /* ECC Correctable Error on 405GP     */
#define UIC_EXT_PCI_SERR 0x00008000   /* External PCI SERR#                 */
#define UIC_PCI_PM    0x00002000      /* PCI Power Management               */
#define UIC_EXT0      0x00000040      /* External  interrupt 0              */
#define UIC_EXT1      0x00000020      /* External  interrupt 1              */
#define UIC_EXT2      0x00000010      /* External  interrupt 2              */
#define UIC_EXT3      0x00000008      /* External  interrupt 3              */
#define UIC_EXT4      0x00000004      /* External  interrupt 4              */
#define UIC_EXT5      0x00000002      /* External  interrupt 5              */
#define UIC_EXT6      0x00000001      /* External  interrupt 6              */

/******************************************************************************
 * SDRAM Controller
 ******************************************************************************/
#define SDRAM_DCR_BASE 0x10
#define memcfga  (SDRAM_DCR_BASE+0x0)   /* Memory configuration address reg  */
#define memcfgd  (SDRAM_DCR_BASE+0x1)   /* Memory configuration data    reg  */
  /* values for memcfga register - indirect addressing of these regs */
#ifndef CONFIG_405EP
  #define mem_besra   0x00    /* bus error syndrome reg a	     */
  #define mem_besrsa  0x04    /* bus error syndrome reg set a	     */
  #define mem_besrb   0x08    /* bus error syndrome reg b	     */
  #define mem_besrsb  0x0c    /* bus error syndrome reg set b	     */
  #define mem_bear    0x10    /* bus error address reg		     */
#endif
  #define mem_mcopt1  0x20    /* memory controller options 1	     */
  #define mem_rtr     0x30    /* refresh timer reg		     */
  #define mem_pmit    0x34    /* power management idle timer	     */
  #define mem_mb0cf   0x40    /* memory bank 0 configuration	     */
  #define mem_mb1cf   0x44    /* memory bank 1 configuration	     */
#ifndef CONFIG_405EP
  #define mem_mb2cf   0x48    /* memory bank 2 configuration	     */
  #define mem_mb3cf   0x4c    /* memory bank 3 configuration	     */
#endif
  #define mem_sdtr1   0x80    /* timing reg 1			     */
#ifndef CONFIG_405EP
  #define mem_ecccf   0x94    /* ECC configuration		     */
  #define mem_eccerr  0x98    /* ECC error status		     */
#endif

#ifndef CONFIG_405EP
/******************************************************************************
 * Decompression Controller
 ******************************************************************************/
#define DECOMP_DCR_BASE 0x14
#define kiar  (DECOMP_DCR_BASE+0x0)  /* Decompression controller addr reg    */
#define kidr  (DECOMP_DCR_BASE+0x1)  /* Decompression controller data reg    */
  /* values for kiar register - indirect addressing of these regs */
  #define kitor0      0x00    /* index table origin register 0	      */
  #define kitor1      0x01    /* index table origin register 1	      */
  #define kitor2      0x02    /* index table origin register 2	      */
  #define kitor3      0x03    /* index table origin register 3	      */
  #define kaddr0      0x04    /* address decode definition regsiter 0 */
  #define kaddr1      0x05    /* address decode definition regsiter 1 */
  #define kconf       0x40    /* decompression core config register   */
  #define kid         0x41    /* decompression core ID     register   */
  #define kver        0x42    /* decompression core version # reg     */
  #define kpear       0x50    /* bus error addr reg (PLB addr)        */
  #define kbear       0x51    /* bus error addr reg (DCP to EBIU addr)*/
  #define kesr0       0x52    /* bus error status reg 0  (R/clear)    */
  #define kesr0s      0x53    /* bus error status reg 0  (set)        */
  /* There are 0x400 of the following registers, from krom0 to krom3ff*/
  /* Only the first one is given here.                                */
  #define krom0      0x400    /* SRAM/ROM read/write                  */
#endif

/******************************************************************************
 * Power Management
 ******************************************************************************/
#define POWERMAN_DCR_BASE 0xb8
#define cpmsr (POWERMAN_DCR_BASE+0x0) /* Power management status             */
#define cpmer (POWERMAN_DCR_BASE+0x1) /* Power management enable             */
#define cpmfr (POWERMAN_DCR_BASE+0x2) /* Power management force              */

/******************************************************************************
 * Extrnal Bus Controller
 ******************************************************************************/
#define EBC_DCR_BASE 0x12
#define ebccfga (EBC_DCR_BASE+0x0)   /* External bus controller addr reg     */
#define ebccfgd (EBC_DCR_BASE+0x1)   /* External bus controller data reg     */
  /* values for ebccfga register - indirect addressing of these regs */
  #define pb0cr       0x00    /* periph bank 0 config reg            */
  #define pb1cr       0x01    /* periph bank 1 config reg            */
  #define pb2cr       0x02    /* periph bank 2 config reg            */
  #define pb3cr       0x03    /* periph bank 3 config reg            */
  #define pb4cr       0x04    /* periph bank 4 config reg            */
#ifndef CONFIG_405EP
  #define pb5cr       0x05    /* periph bank 5 config reg            */
  #define pb6cr       0x06    /* periph bank 6 config reg            */
  #define pb7cr       0x07    /* periph bank 7 config reg            */
#endif
  #define pb0ap       0x10    /* periph bank 0 access parameters     */
  #define pb1ap       0x11    /* periph bank 1 access parameters     */
  #define pb2ap       0x12    /* periph bank 2 access parameters     */
  #define pb3ap       0x13    /* periph bank 3 access parameters     */
  #define pb4ap       0x14    /* periph bank 4 access parameters     */
#ifndef CONFIG_405EP
  #define pb5ap       0x15    /* periph bank 5 access parameters     */
  #define pb6ap       0x16    /* periph bank 6 access parameters     */
  #define pb7ap       0x17    /* periph bank 7 access parameters     */
#endif
  #define pbear       0x20    /* periph bus error addr reg           */
  #define pbesr0      0x21    /* periph bus error status reg 0       */
  #define pbesr1      0x22    /* periph bus error status reg 1       */
  #define epcr        0x23    /* external periph control reg         */

#ifdef CONFIG_405EP
/******************************************************************************
 * Control
 ******************************************************************************/
#define CNTRL_DCR_BASE 0x0f0
#define cpc0_pllmr0   (CNTRL_DCR_BASE+0x0)  /* PLL mode  register 0                */
#define cpc0_boot     (CNTRL_DCR_BASE+0x1)  /* Clock status register               */
#define cpc0_epctl    (CNTRL_DCR_BASE+0x3)  /* EMAC to PHY control register        */
#define cpc0_pllmr1   (CNTRL_DCR_BASE+0x4)  /* PLL mode  register 1                */
#define cpc0_ucr      (CNTRL_DCR_BASE+0x5)  /* UART control register               */
#define cpc0_pci      (CNTRL_DCR_BASE+0x9)  /* PCI control register                */

#define CPC0_PLLMR0  (CNTRL_DCR_BASE+0x0)  /* PLL mode 0 register          */
#define CPC0_BOOT    (CNTRL_DCR_BASE+0x1)  /* Chip Clock Status register   */
#define CPC0_CR1     (CNTRL_DCR_BASE+0x2)  /* Chip Control 1 register      */
#define CPC0_EPRCSR  (CNTRL_DCR_BASE+0x3)  /* EMAC PHY Rcv Clk Src register*/
#define CPC0_PLLMR1  (CNTRL_DCR_BASE+0x4)  /* PLL mode 1 register          */
#define CPC0_UCR     (CNTRL_DCR_BASE+0x5)  /* UART Control register        */
#define CPC0_SRR     (CNTRL_DCR_BASE+0x6)  /* Soft Reset register          */
#define CPC0_JTAGID  (CNTRL_DCR_BASE+0x7)  /* JTAG ID register             */
#define CPC0_SPARE   (CNTRL_DCR_BASE+0x8)  /* Spare DCR                    */
#define CPC0_PCI     (CNTRL_DCR_BASE+0x9)  /* PCI Control register         */

/* Bit definitions */
#define PLLMR0_CPU_DIV_MASK      0x00300000     /* CPU clock divider */
#define PLLMR0_CPU_DIV_BYPASS    0x00000000
#define PLLMR0_CPU_DIV_2         0x00100000
#define PLLMR0_CPU_DIV_3         0x00200000
#define PLLMR0_CPU_DIV_4         0x00300000

#define PLLMR0_CPU_TO_PLB_MASK   0x00030000     /* CPU:PLB Frequency Divisor */
#define PLLMR0_CPU_PLB_DIV_1     0x00000000
#define PLLMR0_CPU_PLB_DIV_2     0x00010000
#define PLLMR0_CPU_PLB_DIV_3     0x00020000
#define PLLMR0_CPU_PLB_DIV_4     0x00030000

#define PLLMR0_OPB_TO_PLB_MASK   0x00003000     /* OPB:PLB Frequency Divisor */
#define PLLMR0_OPB_PLB_DIV_1     0x00000000
#define PLLMR0_OPB_PLB_DIV_2     0x00001000
#define PLLMR0_OPB_PLB_DIV_3     0x00002000
#define PLLMR0_OPB_PLB_DIV_4     0x00003000

#define PLLMR0_EXB_TO_PLB_MASK   0x00000300     /* External Bus:PLB Divisor  */
#define PLLMR0_EXB_PLB_DIV_2     0x00000000
#define PLLMR0_EXB_PLB_DIV_3     0x00000100
#define PLLMR0_EXB_PLB_DIV_4     0x00000200
#define PLLMR0_EXB_PLB_DIV_5     0x00000300

#define PLLMR0_MAL_TO_PLB_MASK   0x00000030     /* MAL:PLB Divisor  */
#define PLLMR0_MAL_PLB_DIV_1     0x00000000
#define PLLMR0_MAL_PLB_DIV_2     0x00000010
#define PLLMR0_MAL_PLB_DIV_3     0x00000020
#define PLLMR0_MAL_PLB_DIV_4     0x00000030

#define PLLMR0_PCI_TO_PLB_MASK   0x00000003     /* PCI:PLB Frequency Divisor */
#define PLLMR0_PCI_PLB_DIV_1     0x00000000
#define PLLMR0_PCI_PLB_DIV_2     0x00000001
#define PLLMR0_PCI_PLB_DIV_3     0x00000002
#define PLLMR0_PCI_PLB_DIV_4     0x00000003

#define PLLMR1_SSCS_MASK         0x80000000     /* Select system clock source */
#define PLLMR1_PLLR_MASK         0x40000000     /* PLL reset */
#define PLLMR1_FBMUL_MASK        0x00F00000     /* PLL feedback multiplier value */
#define PLLMR1_FBMUL_DIV_16      0x00000000
#define PLLMR1_FBMUL_DIV_1       0x00100000
#define PLLMR1_FBMUL_DIV_2       0x00200000
#define PLLMR1_FBMUL_DIV_3       0x00300000
#define PLLMR1_FBMUL_DIV_4       0x00400000
#define PLLMR1_FBMUL_DIV_5       0x00500000
#define PLLMR1_FBMUL_DIV_6       0x00600000
#define PLLMR1_FBMUL_DIV_7       0x00700000
#define PLLMR1_FBMUL_DIV_8       0x00800000
#define PLLMR1_FBMUL_DIV_9       0x00900000
#define PLLMR1_FBMUL_DIV_10      0x00A00000
#define PLLMR1_FBMUL_DIV_11      0x00B00000
#define PLLMR1_FBMUL_DIV_12      0x00C00000
#define PLLMR1_FBMUL_DIV_13      0x00D00000
#define PLLMR1_FBMUL_DIV_14      0x00E00000
#define PLLMR1_FBMUL_DIV_15      0x00F00000

#define PLLMR1_FWDVA_MASK        0x00070000     /* PLL forward divider A value */
#define PLLMR1_FWDVA_DIV_8       0x00000000
#define PLLMR1_FWDVA_DIV_7       0x00010000
#define PLLMR1_FWDVA_DIV_6       0x00020000
#define PLLMR1_FWDVA_DIV_5       0x00030000
#define PLLMR1_FWDVA_DIV_4       0x00040000
#define PLLMR1_FWDVA_DIV_3       0x00050000
#define PLLMR1_FWDVA_DIV_2       0x00060000
#define PLLMR1_FWDVA_DIV_1       0x00070000
#define PLLMR1_FWDVB_MASK        0x00007000     /* PLL forward divider B value */
#define PLLMR1_TUNING_MASK       0x000003FF     /* PLL tune bits */

/* Defines for CPC0_EPRCSR register */
#define CPC0_EPRCSR_E0NFE          0x80000000
#define CPC0_EPRCSR_E1NFE          0x40000000
#define CPC0_EPRCSR_E1RPP          0x00000080
#define CPC0_EPRCSR_E0RPP          0x00000040
#define CPC0_EPRCSR_E1ERP          0x00000020
#define CPC0_EPRCSR_E0ERP          0x00000010
#define CPC0_EPRCSR_E1PCI          0x00000002
#define CPC0_EPRCSR_E0PCI          0x00000001

/* Defines for CPC0_PCI Register */
#define CPC0_PCI_SPE                       0x00000010 /* PCIINT/WE select       */
#define CPC0_PCI_HOST_CFG_EN               0x00000008 /* PCI host config Enable */
#define CPC0_PCI_ARBIT_EN                  0x00000001 /* PCI Internal Arb Enabled*/

/* Defines for CPC0_BOOR Register */
#define CPC0_BOOT_SEP                      0x00000002 /* serial EEPROM present  */

/* Defines for CPC0_PLLMR1 Register fields */
#define PLL_ACTIVE                 0x80000000
#define CPC0_PLLMR1_SSCS           0x80000000
#define PLL_RESET                  0x40000000
#define CPC0_PLLMR1_PLLR           0x40000000
    /* Feedback multiplier */
#define PLL_FBKDIV                 0x00F00000
#define CPC0_PLLMR1_FBDV           0x00F00000
#define PLL_FBKDIV_16              0x00000000
#define PLL_FBKDIV_1               0x00100000
#define PLL_FBKDIV_2               0x00200000
#define PLL_FBKDIV_3               0x00300000
#define PLL_FBKDIV_4               0x00400000
#define PLL_FBKDIV_5               0x00500000
#define PLL_FBKDIV_6               0x00600000
#define PLL_FBKDIV_7               0x00700000
#define PLL_FBKDIV_8               0x00800000
#define PLL_FBKDIV_9               0x00900000
#define PLL_FBKDIV_10              0x00A00000
#define PLL_FBKDIV_11              0x00B00000
#define PLL_FBKDIV_12              0x00C00000
#define PLL_FBKDIV_13              0x00D00000
#define PLL_FBKDIV_14              0x00E00000
#define PLL_FBKDIV_15              0x00F00000
    /* Forward A divisor */
#define PLL_FWDDIVA                0x00070000
#define CPC0_PLLMR1_FWDVA          0x00070000
#define PLL_FWDDIVA_8              0x00000000
#define PLL_FWDDIVA_7              0x00010000
#define PLL_FWDDIVA_6              0x00020000
#define PLL_FWDDIVA_5              0x00030000
#define PLL_FWDDIVA_4              0x00040000
#define PLL_FWDDIVA_3              0x00050000
#define PLL_FWDDIVA_2              0x00060000
#define PLL_FWDDIVA_1              0x00070000
    /* Forward B divisor */
#define PLL_FWDDIVB                0x00007000
#define CPC0_PLLMR1_FWDVB          0x00007000
#define PLL_FWDDIVB_8              0x00000000
#define PLL_FWDDIVB_7              0x00001000
#define PLL_FWDDIVB_6              0x00002000
#define PLL_FWDDIVB_5              0x00003000
#define PLL_FWDDIVB_4              0x00004000
#define PLL_FWDDIVB_3              0x00005000
#define PLL_FWDDIVB_2              0x00006000
#define PLL_FWDDIVB_1              0x00007000
    /* PLL tune bits */
#define PLL_TUNE_MASK            0x000003FF
#define PLL_TUNE_2_M_3           0x00000133     /*  2 <= M <= 3               */
#define PLL_TUNE_4_M_6           0x00000134     /*  3 <  M <= 6               */
#define PLL_TUNE_7_M_10          0x00000138     /*  6 <  M <= 10              */
#define PLL_TUNE_11_M_14         0x0000013C     /* 10 <  M <= 14              */
#define PLL_TUNE_15_M_40         0x0000023E     /* 14 <  M <= 40              */
#define PLL_TUNE_VCO_LOW         0x00000000     /* 500MHz <= VCO <=  800MHz   */
#define PLL_TUNE_VCO_HI          0x00000080     /* 800MHz <  VCO <= 1000MHz   */

/* Defines for CPC0_PLLMR0 Register fields */
    /* CPU divisor */
#define PLL_CPUDIV                 0x00300000
#define CPC0_PLLMR0_CCDV           0x00300000
#define PLL_CPUDIV_1               0x00000000
#define PLL_CPUDIV_2               0x00100000
#define PLL_CPUDIV_3               0x00200000
#define PLL_CPUDIV_4               0x00300000
    /* PLB divisor */
#define PLL_PLBDIV                 0x00030000
#define CPC0_PLLMR0_CBDV           0x00030000
#define PLL_PLBDIV_1               0x00000000
#define PLL_PLBDIV_2               0x00010000
#define PLL_PLBDIV_3               0x00020000
#define PLL_PLBDIV_4               0x00030000
    /* OPB divisor */
#define PLL_OPBDIV                 0x00003000
#define CPC0_PLLMR0_OPDV           0x00003000
#define PLL_OPBDIV_1               0x00000000
#define PLL_OPBDIV_2               0x00001000
#define PLL_OPBDIV_3               0x00002000
#define PLL_OPBDIV_4               0x00003000
    /* EBC divisor */
#define PLL_EXTBUSDIV              0x00000300
#define CPC0_PLLMR0_EPDV           0x00000300
#define PLL_EXTBUSDIV_2            0x00000000
#define PLL_EXTBUSDIV_3            0x00000100
#define PLL_EXTBUSDIV_4            0x00000200
#define PLL_EXTBUSDIV_5            0x00000300
    /* MAL divisor */
#define PLL_MALDIV                 0x00000030
#define CPC0_PLLMR0_MPDV           0x00000030
#define PLL_MALDIV_1               0x00000000
#define PLL_MALDIV_2               0x00000010
#define PLL_MALDIV_3               0x00000020
#define PLL_MALDIV_4               0x00000030
    /* PCI divisor */
#define PLL_PCIDIV                 0x00000003
#define CPC0_PLLMR0_PPFD           0x00000003
#define PLL_PCIDIV_1               0x00000000
#define PLL_PCIDIV_2               0x00000001
#define PLL_PCIDIV_3               0x00000002
#define PLL_PCIDIV_4               0x00000003

/*
 *-------------------------------------------------------------------------------
 * PLL settings for 266MHz CPU, 133MHz PLB/SDRAM, 66MHz EBC, 33MHz PCI,
 * assuming a 33.3MHz input clock to the 405EP.
 *-------------------------------------------------------------------------------
 */
#define PLLMR0_266_133_66  (PLL_CPUDIV_1 | PLL_PLBDIV_2 |  \
			    PLL_OPBDIV_2 | PLL_EXTBUSDIV_2 |  \
			    PLL_MALDIV_1 | PLL_PCIDIV_4)
#define PLLMR1_266_133_66  (PLL_FBKDIV_8  |  \
			    PLL_FWDDIVA_3 | PLL_FWDDIVB_3 |  \
			    PLL_TUNE_15_M_40 | PLL_TUNE_VCO_LOW)

#define PLLMR0_133_66_66_33  (PLL_CPUDIV_1 | PLL_PLBDIV_1 |  \
			      PLL_OPBDIV_2 | PLL_EXTBUSDIV_4 |  \
			      PLL_MALDIV_1 | PLL_PCIDIV_4)
#define PLLMR1_133_66_66_33  (PLL_FBKDIV_4  |  \
			      PLL_FWDDIVA_6 | PLL_FWDDIVB_6 |  \
			      PLL_TUNE_15_M_40 | PLL_TUNE_VCO_LOW)
#define PLLMR0_200_100_50_33 (PLL_CPUDIV_1 | PLL_PLBDIV_2 |  \
			      PLL_OPBDIV_2 | PLL_EXTBUSDIV_3 |  \
			      PLL_MALDIV_1 | PLL_PCIDIV_4)
#define PLLMR1_200_100_50_33 (PLL_FBKDIV_6  |  \
			      PLL_FWDDIVA_4 | PLL_FWDDIVB_4 |  \
			      PLL_TUNE_15_M_40 | PLL_TUNE_VCO_LOW)
#define PLLMR0_266_133_66_33 (PLL_CPUDIV_1 | PLL_PLBDIV_2 |  \
			      PLL_OPBDIV_2 | PLL_EXTBUSDIV_4 |  \
			      PLL_MALDIV_1 | PLL_PCIDIV_4)
#define PLLMR1_266_133_66_33 (PLL_FBKDIV_8  |  \
			      PLL_FWDDIVA_3 | PLL_FWDDIVB_3 |  \
			      PLL_TUNE_15_M_40 | PLL_TUNE_VCO_LOW)
#define PLLMR0_266_66_33_33 (PLL_CPUDIV_1 | PLL_PLBDIV_4 |  \
			      PLL_OPBDIV_2 | PLL_EXTBUSDIV_2 |  \
			      PLL_MALDIV_1 | PLL_PCIDIV_2)
#define PLLMR1_266_66_33_33 (PLL_FBKDIV_8  |  \
			      PLL_FWDDIVA_3 | PLL_FWDDIVB_3 |  \
			      PLL_TUNE_15_M_40 | PLL_TUNE_VCO_LOW)

/*
 * PLL Voltage Controlled Oscillator (VCO) definitions
 * Maximum and minimum values (in MHz) for correct PLL operation.
 */
#define VCO_MIN     500
#define VCO_MAX     1000
#else /* #ifdef CONFIG_405EP */
/******************************************************************************
 * Control
 ******************************************************************************/
#define CNTRL_DCR_BASE 0x0b0
#define pllmd   (CNTRL_DCR_BASE+0x0)  /* PLL mode  register                  */
#define cntrl0  (CNTRL_DCR_BASE+0x1)  /* Control 0 register                  */
#define cntrl1  (CNTRL_DCR_BASE+0x2)  /* Control 1 register		     */
#define reset   (CNTRL_DCR_BASE+0x3)  /* reset register			     */
#define strap   (CNTRL_DCR_BASE+0x4)  /* strap register		   	     */

#define ecr     (0xaa)                /* edge conditioner register (405gpr)  */

/* Bit definitions */
#define PLLMR_FWD_DIV_MASK      0xE0000000     /* Forward Divisor */
#define PLLMR_FWD_DIV_BYPASS    0xE0000000
#define PLLMR_FWD_DIV_3         0xA0000000
#define PLLMR_FWD_DIV_4         0x80000000
#define PLLMR_FWD_DIV_6         0x40000000

#define PLLMR_FB_DIV_MASK       0x1E000000     /* Feedback Divisor */
#define PLLMR_FB_DIV_1          0x02000000
#define PLLMR_FB_DIV_2          0x04000000
#define PLLMR_FB_DIV_3          0x06000000
#define PLLMR_FB_DIV_4          0x08000000

#define PLLMR_TUNING_MASK       0x01F80000

#define PLLMR_CPU_TO_PLB_MASK   0x00060000     /* CPU:PLB Frequency Divisor */
#define PLLMR_CPU_PLB_DIV_1     0x00000000
#define PLLMR_CPU_PLB_DIV_2     0x00020000
#define PLLMR_CPU_PLB_DIV_3     0x00040000
#define PLLMR_CPU_PLB_DIV_4     0x00060000

#define PLLMR_OPB_TO_PLB_MASK   0x00018000     /* OPB:PLB Frequency Divisor */
#define PLLMR_OPB_PLB_DIV_1     0x00000000
#define PLLMR_OPB_PLB_DIV_2     0x00008000
#define PLLMR_OPB_PLB_DIV_3     0x00010000
#define PLLMR_OPB_PLB_DIV_4     0x00018000

#define PLLMR_PCI_TO_PLB_MASK   0x00006000     /* PCI:PLB Frequency Divisor */
#define PLLMR_PCI_PLB_DIV_1     0x00000000
#define PLLMR_PCI_PLB_DIV_2     0x00002000
#define PLLMR_PCI_PLB_DIV_3     0x00004000
#define PLLMR_PCI_PLB_DIV_4     0x00006000

#define PLLMR_EXB_TO_PLB_MASK   0x00001800     /* External Bus:PLB Divisor  */
#define PLLMR_EXB_PLB_DIV_2     0x00000000
#define PLLMR_EXB_PLB_DIV_3     0x00000800
#define PLLMR_EXB_PLB_DIV_4     0x00001000
#define PLLMR_EXB_PLB_DIV_5     0x00001800

/* definitions for PPC405GPr (new mode strapping) */
#define PLLMR_FWDB_DIV_MASK     0x00000007     /* Forward Divisor B */

#define PSR_PLL_FWD_MASK        0xC0000000
#define PSR_PLL_FDBACK_MASK     0x30000000
#define PSR_PLL_TUNING_MASK     0x0E000000
#define PSR_PLB_CPU_MASK        0x01800000
#define PSR_OPB_PLB_MASK        0x00600000
#define PSR_PCI_PLB_MASK        0x00180000
#define PSR_EB_PLB_MASK         0x00060000
#define PSR_ROM_WIDTH_MASK      0x00018000
#define PSR_ROM_LOC             0x00004000
#define PSR_PCI_ASYNC_EN        0x00001000
#define PSR_PERCLK_SYNC_MODE_EN 0x00000800     /* PPC405GPr only */
#define PSR_PCI_ARBIT_EN        0x00000400
#define PSR_NEW_MODE_EN         0x00000020     /* PPC405GPr only */

#ifndef CONFIG_IOP480
/*
 * PLL Voltage Controlled Oscillator (VCO) definitions
 * Maximum and minimum values (in MHz) for correct PLL operation.
 */
#define VCO_MIN     400
#define VCO_MAX     800
#endif /* #ifndef CONFIG_IOP480 */
#endif /* #ifdef CONFIG_405EP */

/******************************************************************************
 * Memory Access Layer
 ******************************************************************************/
#define MAL_DCR_BASE 0x180
#define malmcr  (MAL_DCR_BASE+0x00)  /* MAL Config reg                       */
#define malesr  (MAL_DCR_BASE+0x01)  /* Error Status reg (Read/Clear)        */
#define malier  (MAL_DCR_BASE+0x02)  /* Interrupt enable reg                 */
#define maldbr  (MAL_DCR_BASE+0x03)  /* Mal Debug reg (Read only)            */
#define maltxcasr  (MAL_DCR_BASE+0x04)  /* TX Channel active reg (set)       */
#define maltxcarr  (MAL_DCR_BASE+0x05)  /* TX Channel active reg (Reset)     */
#define maltxeobisr (MAL_DCR_BASE+0x06) /* TX End of buffer int status reg   */
#define maltxdeir  (MAL_DCR_BASE+0x07)  /* TX Descr. Error Int reg           */
#define malrxcasr  (MAL_DCR_BASE+0x10)  /* RX Channel active reg (set)       */
#define malrxcarr  (MAL_DCR_BASE+0x11)  /* RX Channel active reg (Reset)     */
#define malrxeobisr (MAL_DCR_BASE+0x12) /* RX End of buffer int status reg   */
#define malrxdeir  (MAL_DCR_BASE+0x13)  /* RX Descr. Error Int reg           */
#define maltxctp0r (MAL_DCR_BASE+0x20)  /* TX 0 Channel table pointer reg    */
#define maltxctp1r (MAL_DCR_BASE+0x21)  /* TX 1 Channel table pointer reg    */
#define maltxctp2r (MAL_DCR_BASE+0x22)  /* TX 2 Channel table pointer reg    */
#define malrxctp0r (MAL_DCR_BASE+0x40)  /* RX 0 Channel table pointer reg    */
#define malrxctp1r (MAL_DCR_BASE+0x41)  /* RX 1 Channel table pointer reg    */
#define malrcbs0   (MAL_DCR_BASE+0x60)  /* RX 0 Channel buffer size reg      */
#define malrcbs1   (MAL_DCR_BASE+0x61)  /* RX 1 Channel buffer size reg      */

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

/******************************************************************************
 * On Chip Memory
 ******************************************************************************/
#define OCM_DCR_BASE 0x018
#define ocmisarc   (OCM_DCR_BASE+0x00)  /* OCM I-side address compare reg    */
#define ocmiscntl  (OCM_DCR_BASE+0x01)  /* OCM I-side control reg            */
#define ocmdsarc   (OCM_DCR_BASE+0x02)  /* OCM D-side address compare reg    */
#define ocmdscntl  (OCM_DCR_BASE+0x03)  /* OCM D-side control reg            */

/******************************************************************************
 * GPIO macro register defines
 ******************************************************************************/
#define GPIO_BASE  0xEF600700
#define GPIO0_OR               (GPIO_BASE+0x0)
#define GPIO0_TCR              (GPIO_BASE+0x4)
#define GPIO0_OSRH             (GPIO_BASE+0x8)
#define GPIO0_OSRL             (GPIO_BASE+0xC)
#define GPIO0_TSRH             (GPIO_BASE+0x10)
#define GPIO0_TSRL             (GPIO_BASE+0x14)
#define GPIO0_ODR              (GPIO_BASE+0x18)
#define GPIO0_IR               (GPIO_BASE+0x1C)
#define GPIO0_RR1              (GPIO_BASE+0x20)
#define GPIO0_RR2              (GPIO_BASE+0x24)
#define GPIO0_ISR1H            (GPIO_BASE+0x30)
#define GPIO0_ISR1L            (GPIO_BASE+0x34)
#define GPIO0_ISR2H            (GPIO_BASE+0x38)
#define GPIO0_ISR2L            (GPIO_BASE+0x3C)


/*
 * Macro for accessing the indirect EBC register
 */
#define mtebc(reg, data)  mtdcr(ebccfga,reg);mtdcr(ebccfgd,data)
#define mfebc(reg, data)  mtdcr(ebccfga,reg);data = mfdcr(ebccfgd)


#ifndef __ASSEMBLY__

typedef struct
{
  unsigned long pllFwdDiv;
  unsigned long pllFwdDivB;
  unsigned long pllFbkDiv;
  unsigned long pllPlbDiv;
  unsigned long pllPciDiv;
  unsigned long pllExtBusDiv;
  unsigned long pllOpbDiv;
  unsigned long freqVCOMhz;             /* in MHz                          */
  unsigned long freqProcessor;
  unsigned long freqPLB;
  unsigned long freqPCI;
  unsigned long pciIntArbEn;            /* Internal PCI arbiter is enabled */
  unsigned long pciClkSync;             /* PCI clock is synchronous        */
  unsigned long freqVCOHz;
} PPC405_SYS_INFO;

#endif  /* _ASMLANGUAGE */

#define RESET_VECTOR	0xfffffffc
#define CACHELINE_MASK	(CFG_CACHELINE_SIZE - 1) /* Address mask for cache
						     line aligned data. */

#endif	/* __PPC405_H__ */
