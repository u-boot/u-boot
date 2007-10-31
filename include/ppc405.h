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

#ifndef CONFIG_IOP480
#define CFG_DCACHE_SIZE		(16 << 10)	/* For AMCC 405 CPUs	*/
#else
#define CFG_DCACHE_SIZE		(2 << 10)	/* For PLX IOP480 (403)	*/
#endif

/*--------------------------------------------------------------------- */
/* Special Purpose Registers						*/
/*--------------------------------------------------------------------- */
	#define  srr2  0x3de      /* save/restore register 2 */
	#define  srr3  0x3df      /* save/restore register 3 */

	/*
	 * 405 does not really have CSRR0/1 but SRR2/3 are used during critical
	 * exception for the exact same purposes - let's alias them and have a
	 * common handling in crit_return() and CRIT_EXCEPTION
	 */
	#define  csrr0 srr2
	#define  csrr1 srr3

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

#if defined(CONFIG_405EX)
#define uic0sr        uicsr		/* UIC status            */
#define uic0srs       uicsrs		/* UIC status set        */
#define uic0er        uicer		/* UIC enable            */
#define uic0cr        uiccr		/* UIC critical          */
#define uic0pr        uicpr		/* UIC polarity          */
#define uic0tr        uictr		/* UIC triggering        */
#define uic0msr       uicmsr		/* UIC masked status     */
#define uic0vr        uicvr		/* UIC vector            */
#define uic0vcr       uicvcr		/* UIC vector configuration*/

#define UIC_DCR_BASE1 0xd0
#define uic1sr        (UIC_DCR_BASE1+0x0)  /* UIC status            */
#define uic1srs       (UIC_DCR_BASE1+0x1)  /* UIC status set        */
#define uic1er        (UIC_DCR_BASE1+0x2)  /* UIC enable            */
#define uic1cr        (UIC_DCR_BASE1+0x3)  /* UIC critical          */
#define uic1pr        (UIC_DCR_BASE1+0x4)  /* UIC polarity          */
#define uic1tr        (UIC_DCR_BASE1+0x5)  /* UIC triggering        */
#define uic1msr       (UIC_DCR_BASE1+0x6)  /* UIC masked status     */
#define uic1vr        (UIC_DCR_BASE1+0x7)  /* UIC vector            */
#define uic1vcr       (UIC_DCR_BASE1+0x8)  /* UIC vector configuration*/

#define UIC_DCR_BASE2 0xe0
#define uic2sr        (UIC_DCR_BASE2+0x0)  /* UIC status            */
#define uic2srs       (UIC_DCR_BASE2+0x1)  /* UIC status set        */
#define uic2er        (UIC_DCR_BASE2+0x2)  /* UIC enable            */
#define uic2cr        (UIC_DCR_BASE2+0x3)  /* UIC critical          */
#define uic2pr        (UIC_DCR_BASE2+0x4)  /* UIC polarity          */
#define uic2tr        (UIC_DCR_BASE2+0x5)  /* UIC triggering        */
#define uic2msr       (UIC_DCR_BASE2+0x6)  /* UIC masked status     */
#define uic2vr        (UIC_DCR_BASE2+0x7)  /* UIC vector            */
#define uic2vcr       (UIC_DCR_BASE2+0x8)  /* UIC vector configuration*/
#endif

/*-----------------------------------------------------------------------------+
|  Universal interrupt controller interrupts
+-----------------------------------------------------------------------------*/
#if defined(CONFIG_405EZ)
#define UIC_DMA0	0x80000000	/* DMA chan. 0			*/
#define UIC_DMA1	0x40000000	/* DMA chan. 1			*/
#define UIC_DMA2	0x20000000	/* DMA chan. 2			*/
#define UIC_DMA3	0x10000000	/* DMA chan. 3			*/
#define UIC_1588	0x08000000	/* IEEE 1588 network synchronization */
#define UIC_UART0	0x04000000	/* UART 0			*/
#define UIC_UART1	0x02000000	/* UART 1			*/
#define UIC_CAN0	0x01000000	/* CAN 0			*/
#define UIC_CAN1	0x00800000	/* CAN 1			*/
#define UIC_SPI		0x00400000	/* SPI				*/
#define UIC_IIC		0x00200000	/* IIC				*/
#define UIC_CHT0	0x00100000	/* Chameleon timer high pri interrupt */
#define UIC_CHT1	0x00080000	/* Chameleon timer high pri interrupt */
#define UIC_USBH1	0x00040000	/* USB Host 1			*/
#define UIC_USBH2	0x00020000	/* USB Host 2			*/
#define UIC_USBDEV	0x00010000	/* USB Device			*/
#define UIC_ENET	0x00008000	/* Ethernet interrupt status	*/
#define UIC_ENET1	0x00008000	/* dummy define			*/
#define UIC_EMAC_WAKE	0x00004000	/* EMAC wake up			*/

#define UIC_MADMAL	0x00002000	/* Logical OR of following MadMAL int */
#define UIC_MAL_SERR	0x00002000	/*   MAL SERR			*/
#define UIC_MAL_TXDE	0x00002000	/*   MAL TXDE			*/
#define UIC_MAL_RXDE	0x00002000	/*   MAL RXDE			*/

#define UIC_MAL_TXEOB	0x00001000	/* MAL TXEOB			*/
#define UIC_MAL_TXEOB1	0x00000800	/* MAL TXEOB1			*/
#define UIC_MAL_RXEOB	0x00000400	/* MAL RXEOB			*/
#define UIC_NAND	0x00000200	/* NAND Flash controller	*/
#define UIC_ADC		0x00000100	/* ADC				*/
#define UIC_DAC		0x00000080	/* DAC				*/
#define UIC_OPB2PLB	0x00000040	/* OPB to PLB bridge interrupt	*/
#define UIC_RESERVED0	0x00000020	/* Reserved			*/
#define UIC_EXT0	0x00000010	/* External  interrupt 0	*/
#define UIC_EXT1	0x00000008	/* External  interrupt 1	*/
#define UIC_EXT2	0x00000004	/* External  interrupt 2	*/
#define UIC_EXT3	0x00000002	/* External  interrupt 3	*/
#define UIC_EXT4	0x00000001	/* External  interrupt 4	*/

#elif defined(CONFIG_405EX)

/* UIC 0 */
#define UIC_U0			0x80000000      /* */
#define UIC_U1			0x40000000      /* */
#define UIC_IIC0		0x20000000      /* */
#define UIC_PKA			0x10000000      /* */
#define UIC_TRNG		0x08000000      /* */
#define UIC_EBM			0x04000000      /* */
#define UIC_BGI			0x02000000      /* */
#define UIC_IIC1		0x01000000      /* */
#define UIC_SPI			0x00800000      /* */
#define UIC_EIRQ0		0x00400000      /**/
#define UIC_MTE			0x00200000      /*MAL Tx EOB */
#define UIC_MRE			0x00100000      /*MAL Rx EOB */
#define UIC_DMA0		0x00080000      /* */
#define UIC_DMA1		0x00040000      /* */
#define UIC_DMA2		0x00020000      /* */
#define UIC_DMA3		0x00010000      /* */
#define UIC_PCIE0AL		0x00008000      /* */
#define UIC_PCIE0VPD		0x00004000      /* */
#define UIC_RPCIE0HRST		0x00002000      /* */
#define UIC_FPCIE0HRST		0x00001000      /* */
#define UIC_PCIE0TCR		0x00000800      /* */
#define UIC_PCIEMSI0		0x00000400      /* */
#define UIC_PCIEMSI1		0x00000200      /* */
#define UIC_SECURITY		0x00000100      /* */
#define UIC_ENET		0x00000080      /* */
#define UIC_ENET1		0x00000040      /* */
#define UIC_PCIEMSI2		0x00000020      /* */
#define UIC_EIRQ4		0x00000010      /**/
#define UIC_UIC2NC		0x00000008      /* */
#define UIC_UIC2C		0x00000004      /* */
#define UIC_UIC1NC		0x00000002      /* */
#define UIC_UIC1C		0x00000001      /* */

#define UIC_MAL_TXEOB 		UIC_MTE/* MAL TXEOB                          */
#define UIC_MAL_RXEOB 		UIC_MRE/* MAL RXEOB                          */
/* UIC 1 */
#define UIC_MS			0x80000000      /* MAL SERR */
#define UIC_MTDE		0x40000000      /* MAL TXDE */
#define UIC_MRDE		0x20000000      /* MAL RXDE */
#define UIC_PCIE0BMVC0		0x10000000      /* */
#define UIC_PCIE0DCRERR		0x08000000      /* */
#define UIC_EBC			0x04000000      /* */
#define UIC_NDFC		0x02000000      /* */
#define UIC_PCEI1DCRERR		0x01000000      /* */
#define UIC_GPTCMPT8		0x00800000      /* */
#define UIC_GPTCMPT9		0x00400000      /* */
#define UIC_PCIE1AL		0x00200000      /* */
#define UIC_PCIE1VPD		0x00100000      /* */
#define UIC_RPCE1HRST		0x00080000      /* */
#define UIC_FPCE1HRST		0x00040000      /* */
#define UIC_PCIE1TCR		0x00020000      /* */
#define UIC_PCIE1VC0		0x00010000      /* */
#define UIC_GPTCMPT3		0x00008000      /* */
#define UIC_GPTCMPT4		0x00004000      /* */
#define UIC_EIRQ7		0x00002000      /* */
#define UIC_EIRQ8		0x00001000      /* */
#define UIC_EIRQ9		0x00000800      /* */
#define UIC_GPTCMP5		0x00000400      /* */
#define UIC_GPTCMP6		0x00000200      /* */
#define UIC_GPTCMP7		0x00000100      /* */
#define UIC_SROM		0x00000080      /* SERIAL ROM*/
#define UIC_GPTDECPULS		0x00000040      /* GPT Decrement pulse*/
#define UIC_EIRQ2		0x00000020      /* */
#define UIC_EIRQ5		0x00000010      /* */
#define UIC_EIRQ6		0x00000008      /* */
#define UIC_EMAC0WAKE		0x00000004      /* */
#define UIC_EIRQ1		0x00000002      /* */
#define UIC_EMAC1WAKE		0x00000001      /* */
#define UIC_MAL_SERR		UIC_MS 		/* MAL SERR     */
#define UIC_MAL_TXDE		UIC_MTDE		/* MAL TXDE     */
#define UIC_MAL_RXDE		UIC_MRDE 		/* MAL RXDE     */
/* UIC 2 */
#define UIC_PCIE0INTA		0x80000000      /* PCIE0 INTA*/
#define UIC_PCIE0INTB		0x40000000      /* PCIE0 INTB*/
#define UIC_PCIE0INTC		0x20000000      /* PCIE0 INTC*/
#define UIC_PCIE0INTD		0x10000000      /* PCIE0 INTD*/
#define UIC_EIRQ3		0x08000000      /* External IRQ 3*/
#define UIC_DDRMCUE		0x04000000      /* */
#define UIC_DDRMCCE		0x02000000      /* */
#define UIC_MALINTCOATX0	0x01000000      /* Interrupt coalecence TX0*/
#define UIC_MALINTCOATX1	0x00800000      /* Interrupt coalecence TX1*/
#define UIC_MALINTCOARX0	0x00400000      /* Interrupt coalecence RX0*/
#define UIC_MALINTCOARX1	0x00200000      /* Interrupt coalecence RX1*/
#define UIC_PCIE1INTA		0x00100000      /* PCIE0 INTA*/
#define UIC_PCIE1INTB		0x00080000      /* PCIE0 INTB*/
#define UIC_PCIE1INTC		0x00040000      /* PCIE0 INTC*/
#define UIC_PCIE1INTD		0x00020000      /* PCIE0 INTD*/
#define UIC_RPCIEMSI2		0x00010000      /* MSI level 2 Note this looks same as uic0-26*/
#define UIC_PCIEMSI3		0x00008000      /* MSI level 2*/
#define UIC_PCIEMSI4		0x00004000      /* MSI level 2*/
#define UIC_PCIEMSI5		0x00002000      /* MSI level 2*/
#define UIC_PCIEMSI6		0x00001000      /* MSI level 2*/
#define UIC_PCIEMSI7		0x00000800      /* MSI level 2*/
#define UIC_PCIEMSI8		0x00000400      /* MSI level 2*/
#define UIC_PCIEMSI9		0x00000200      /* MSI level 2*/
#define UIC_PCIEMSI10		0x00000100      /* MSI level 2*/
#define UIC_PCIEMSI11		0x00000080      /* MSI level 2*/
#define UIC_PCIEMSI12		0x00000040      /* MSI level 2*/
#define UIC_PCIEMSI13		0x00000020      /* MSI level 2*/
#define UIC_PCIEMSI14		0x00000010      /* MSI level 2*/
#define UIC_PCIEMSI15		0x00000008      /* MSI level 2*/
#define UIC_PLB4XAHB		0x00000004      /* PLBxAHB bridge*/
#define UIC_USBWAKE		0x00000002      /* USB wakup*/
#define UIC_USBOTG		0x00000001      /*  USB OTG*/
#define UIC_ETH0	UIC_ENET
#define UIC_ETH1	UIC_ENET1

#else	/* !defined(CONFIG_405EZ) */

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
#endif	/* defined(CONFIG_405EZ) */

/******************************************************************************
 * SDRAM Controller
 ******************************************************************************/
  /* values for memcfga register - indirect addressing of these regs */
#ifndef CONFIG_405EP
  #define mem_besra   0x00    /* bus error syndrome reg a	     */
  #define mem_besrsa  0x04    /* bus error syndrome reg set a	     */
  #define mem_besrb   0x08    /* bus error syndrome reg b	     */
  #define mem_besrsb  0x0c    /* bus error syndrome reg set b	     */
  #define mem_bear    0x10    /* bus error address reg		     */
#endif
  #define mem_mcopt1  0x20    /* memory controller options 1	     */
  #define mem_status  0x24    /* memory status			     */
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
#ifdef CONFIG_405EX
#define POWERMAN_DCR_BASE 0xb0
#else
#define POWERMAN_DCR_BASE 0xb8
#endif
#define cpmsr (POWERMAN_DCR_BASE+0x0) /* Power management status             */
#define cpmer (POWERMAN_DCR_BASE+0x1) /* Power management enable             */
#define cpmfr (POWERMAN_DCR_BASE+0x2) /* Power management force              */

/******************************************************************************
 * Extrnal Bus Controller
 ******************************************************************************/
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
#define EBC0_CFG	0x23	/* external bus configuration reg	*/

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
#define PLLMR0_333_111_55_37 (PLL_CPUDIV_1 | PLL_PLBDIV_3 |  \
			      PLL_OPBDIV_2 | PLL_EXTBUSDIV_2 |  \
			      PLL_MALDIV_1 | PLL_PCIDIV_3)
#define PLLMR1_333_111_55_37 (PLL_FBKDIV_10  |  \
			      PLL_FWDDIVA_3 | PLL_FWDDIVB_3 |  \
			      PLL_TUNE_15_M_40 | PLL_TUNE_VCO_HI)
#define PLLMR0_333_111_55_111 (PLL_CPUDIV_1 | PLL_PLBDIV_3 |  \
			      PLL_OPBDIV_2 | PLL_EXTBUSDIV_2 |  \
			      PLL_MALDIV_1 | PLL_PCIDIV_1)
#define PLLMR1_333_111_55_111 (PLL_FBKDIV_10  |  \
			      PLL_FWDDIVA_3 | PLL_FWDDIVB_3 |  \
			      PLL_TUNE_15_M_40 | PLL_TUNE_VCO_HI)

/*
 * PLL Voltage Controlled Oscillator (VCO) definitions
 * Maximum and minimum values (in MHz) for correct PLL operation.
 */
#define VCO_MIN     500
#define VCO_MAX     1000
#elif defined(CONFIG_405EZ)
#define sdrnand0	0x4000
#define sdrultra0	0x4040
#define sdrultra1	0x4050
#define sdricintstat	0x4510

#define SDR_NAND0_NDEN		0x80000000
#define SDR_NAND0_NDBTEN	0x40000000
#define SDR_NAND0_NDBADR_MASK	0x30000000
#define SDR_NAND0_NDBPG_MASK	0x0f000000
#define SDR_NAND0_NDAREN	0x00800000
#define SDR_NAND0_NDRBEN	0x00400000

#define SDR_ULTRA0_NDGPIOBP	0x80000000
#define SDR_ULTRA0_CSN_MASK	0x78000000
#define SDR_ULTRA0_CSNSEL0	0x40000000
#define SDR_ULTRA0_CSNSEL1	0x20000000
#define SDR_ULTRA0_CSNSEL2	0x10000000
#define SDR_ULTRA0_CSNSEL3	0x08000000
#define SDR_ULTRA0_EBCRDYEN	0x04000000
#define SDR_ULTRA0_SPISSINEN	0x02000000
#define SDR_ULTRA0_NFSRSTEN	0x01000000

#define SDR_ULTRA1_LEDNENABLE	0x40000000

#define SDR_ICRX_STAT	0x80000000
#define SDR_ICTX0_STAT	0x40000000
#define SDR_ICTX1_STAT	0x20000000

#define SDR_PINSTP	0x40

/******************************************************************************
 * Control
 ******************************************************************************/
/* CPR Registers */
#define cprclkupd       0x020		/* CPR_CLKUPD */
#define cprpllc         0x040		/* CPR_PLLC */
#define cprplld         0x060		/* CPR_PLLD */
#define cprprimad       0x080		/* CPR_PRIMAD */
#define cprperd0        0x0e0		/* CPR_PERD0 */
#define cprperd1        0x0e1		/* CPR_PERD1 */
#define cprperc0        0x180		/* CPR_PERC0 */
#define cprmisc0        0x181		/* CPR_MISC0 */
#define cprmisc1        0x182		/* CPR_MISC1 */

#define CPR_CLKUPD_ENPLLCH_EN  0x40000000     /* Enable CPR PLL Changes */
#define CPR_CLKUPD_ENDVCH_EN   0x20000000     /* Enable CPR Sys. Div. Changes */
#define CPR_PERD0_SPIDV_MASK   0x000F0000     /* SPI Clock Divider */

#define PLLC_SRC_MASK          0x20000000     /* PLL feedback source */

#define PLLD_FBDV_MASK         0x1F000000     /* PLL feedback divider value */
#define PLLD_FWDVA_MASK        0x000F0000     /* PLL forward divider A value */
#define PLLD_FWDVB_MASK        0x00000700     /* PLL forward divider B value */

#define PRIMAD_CPUDV_MASK      0x0F000000     /* CPU Clock Divisor Mask */
#define PRIMAD_PLBDV_MASK      0x000F0000     /* PLB Clock Divisor Mask */
#define PRIMAD_OPBDV_MASK      0x00000F00     /* OPB Clock Divisor Mask */
#define PRIMAD_EBCDV_MASK      0x0000000F     /* EBC Clock Divisor Mask */

#define PERD0_PWMDV_MASK       0xFF000000     /* PWM Divider Mask */
#define PERD0_SPIDV_MASK       0x000F0000     /* SPI Divider Mask */
#define PERD0_U0DV_MASK        0x0000FF00     /* UART 0 Divider Mask */
#define PERD0_U1DV_MASK        0x000000FF     /* UART 1 Divider Mask */

#else /* #ifdef CONFIG_405EP */
/******************************************************************************
 * Control
 ******************************************************************************/
#define CNTRL_DCR_BASE 0x0b0
#define pllmd   (CNTRL_DCR_BASE+0x0)  /* PLL mode  register                  */
#define cntrl0  (CNTRL_DCR_BASE+0x1)  /* Control 0 register                  */
#define cntrl1  (CNTRL_DCR_BASE+0x2)  /* Control 1 register		     */
#define reset   (CNTRL_DCR_BASE+0x3)  /* reset register			     */
#define strap   (CNTRL_DCR_BASE+0x4)  /* strap register			     */

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
#if defined(CONFIG_405EZ)
#define	MAL_DCR_BASE	0x380
#define	malmcr		(MAL_DCR_BASE+0x00)	/* MAL Config reg	      */
#define	malesr		(MAL_DCR_BASE+0x01)	/* Err Status reg (Read/Clear)*/
#define	malier		(MAL_DCR_BASE+0x02)	/* Interrupt enable reg	      */
#define	maldbr		(MAL_DCR_BASE+0x03)	/* Mal Debug reg (Read only)  */
#define	maltxcasr	(MAL_DCR_BASE+0x04)	/* TX Channel active reg (set)*/
#define	maltxcarr	(MAL_DCR_BASE+0x05)	/* TX Channel active reg (Reset)     */
#define	maltxeobisr	(MAL_DCR_BASE+0x06)	/* TX End of buffer int status reg   */
#define	maltxdeir	(MAL_DCR_BASE+0x07)	/* TX Descr. Error Int reg    */
/*				      0x08-0x0F	   Reserved		      */
#define	malrxcasr	(MAL_DCR_BASE+0x10)	/* RX Channel active reg (set)*/
#define	malrxcarr	(MAL_DCR_BASE+0x11)	/* RX Channel active reg (Reset)     */
#define	malrxeobisr	(MAL_DCR_BASE+0x12)	/* RX End of buffer int status reg   */
#define	malrxdeir	(MAL_DCR_BASE+0x13)	/* RX Descr. Error Int reg  */
/*				      0x14-0x1F	   Reserved		    */
#define	maltxctp0r	(MAL_DCR_BASE+0x20)  /* TX 0 Channel table ptr reg  */
#define	maltxctp1r	(MAL_DCR_BASE+0x21)  /* TX 1 Channel table ptr reg  */
#define	maltxctp2r	(MAL_DCR_BASE+0x22)  /* TX 2 Channel table ptr reg  */
#define	maltxctp3r	(MAL_DCR_BASE+0x23)  /* TX 3 Channel table ptr reg  */
#define	maltxctp4r	(MAL_DCR_BASE+0x24)  /* TX 4 Channel table ptr reg  */
#define	maltxctp5r	(MAL_DCR_BASE+0x25)  /* TX 5 Channel table ptr reg  */
#define	maltxctp6r	(MAL_DCR_BASE+0x26)  /* TX 6 Channel table ptr reg  */
#define	maltxctp7r	(MAL_DCR_BASE+0x27)  /* TX 7 Channel table ptr reg  */
#define	maltxctp8r	(MAL_DCR_BASE+0x28)  /* TX 8 Channel table ptr reg  */
#define	maltxctp9r	(MAL_DCR_BASE+0x29)  /* TX 9 Channel table ptr reg  */
#define	maltxctp10r	(MAL_DCR_BASE+0x2A)  /* TX 10 Channel table ptr reg */
#define	maltxctp11r	(MAL_DCR_BASE+0x2B)  /* TX 11 Channel table ptr reg */
#define	maltxctp12r	(MAL_DCR_BASE+0x2C)  /* TX 12 Channel table ptr reg */
#define	maltxctp13r	(MAL_DCR_BASE+0x2D)  /* TX 13 Channel table ptr reg */
#define	maltxctp14r	(MAL_DCR_BASE+0x2E)  /* TX 14 Channel table ptr reg */
#define	maltxctp15r	(MAL_DCR_BASE+0x2F)  /* TX 15 Channel table ptr reg */
#define	maltxctp16r	(MAL_DCR_BASE+0x30)  /* TX 16 Channel table ptr reg */
#define	maltxctp17r	(MAL_DCR_BASE+0x31)  /* TX 17 Channel table ptr reg */
#define	maltxctp18r	(MAL_DCR_BASE+0x32)  /* TX 18 Channel table ptr reg */
#define	maltxctp19r	(MAL_DCR_BASE+0x33)  /* TX 19 Channel table ptr reg */
#define	maltxctp20r	(MAL_DCR_BASE+0x34)  /* TX 20 Channel table ptr reg */
#define	maltxctp21r	(MAL_DCR_BASE+0x35)  /* TX 21 Channel table ptr reg */
#define	maltxctp22r	(MAL_DCR_BASE+0x36)  /* TX 22 Channel table ptr reg */
#define	maltxctp23r	(MAL_DCR_BASE+0x37)  /* TX 23 Channel table ptr reg */
#define	maltxctp24r	(MAL_DCR_BASE+0x38)  /* TX 24 Channel table ptr reg */
#define	maltxctp25r	(MAL_DCR_BASE+0x39)  /* TX 25 Channel table ptr reg */
#define	maltxctp26r	(MAL_DCR_BASE+0x3A)  /* TX 26 Channel table ptr reg */
#define	maltxctp27r	(MAL_DCR_BASE+0x3B)  /* TX 27 Channel table ptr reg */
#define	maltxctp28r	(MAL_DCR_BASE+0x3C)  /* TX 28 Channel table ptr reg */
#define	maltxctp29r	(MAL_DCR_BASE+0x3D)  /* TX 29 Channel table ptr reg */
#define	maltxctp30r	(MAL_DCR_BASE+0x3E)  /* TX 30 Channel table ptr reg */
#define	maltxctp31r	(MAL_DCR_BASE+0x3F)  /* TX 31 Channel table ptr reg */
#define	malrxctp0r	(MAL_DCR_BASE+0x40)  /* RX 0 Channel table ptr reg  */
#define	malrxctp1r	(MAL_DCR_BASE+0x41)  /* RX 1 Channel table ptr reg  */
#define	malrxctp2r	(MAL_DCR_BASE+0x42)  /* RX 2 Channel table ptr reg  */
#define	malrxctp3r	(MAL_DCR_BASE+0x43)  /* RX 3 Channel table ptr reg  */
#define	malrxctp4r	(MAL_DCR_BASE+0x44)  /* RX 4 Channel table ptr reg  */
#define	malrxctp5r	(MAL_DCR_BASE+0x45)  /* RX 5 Channel table ptr reg  */
#define	malrxctp6r	(MAL_DCR_BASE+0x46)  /* RX 6 Channel table ptr reg  */
#define	malrxctp7r	(MAL_DCR_BASE+0x47)  /* RX 7 Channel table ptr reg  */
#define	malrxctp8r	(MAL_DCR_BASE+0x48)  /* RX 8 Channel table ptr reg  */
#define	malrxctp9r	(MAL_DCR_BASE+0x49)  /* RX 9 Channel table ptr reg  */
#define	malrxctp10r	(MAL_DCR_BASE+0x4A)  /* RX 10 Channel table ptr reg */
#define	malrxctp11r	(MAL_DCR_BASE+0x4B)  /* RX 11 Channel table ptr reg */
#define	malrxctp12r	(MAL_DCR_BASE+0x4C)  /* RX 12 Channel table ptr reg */
#define	malrxctp13r	(MAL_DCR_BASE+0x4D)  /* RX 13 Channel table ptr reg */
#define	malrxctp14r	(MAL_DCR_BASE+0x4E)  /* RX 14 Channel table ptr reg */
#define	malrxctp15r	(MAL_DCR_BASE+0x4F)  /* RX 15 Channel table ptr reg */
#define	malrxctp16r	(MAL_DCR_BASE+0x50)  /* RX 16 Channel table ptr reg */
#define	malrxctp17r	(MAL_DCR_BASE+0x51)  /* RX 17 Channel table ptr reg */
#define	malrxctp18r	(MAL_DCR_BASE+0x52)  /* RX 18 Channel table ptr reg */
#define	malrxctp19r	(MAL_DCR_BASE+0x53)  /* RX 19 Channel table ptr reg */
#define	malrxctp20r	(MAL_DCR_BASE+0x54)  /* RX 20 Channel table ptr reg */
#define	malrxctp21r	(MAL_DCR_BASE+0x55)  /* RX 21 Channel table ptr reg */
#define	malrxctp22r	(MAL_DCR_BASE+0x56)  /* RX 22 Channel table ptr reg */
#define	malrxctp23r	(MAL_DCR_BASE+0x57)  /* RX 23 Channel table ptr reg */
#define	malrxctp24r	(MAL_DCR_BASE+0x58)  /* RX 24 Channel table ptr reg */
#define	malrxctp25r	(MAL_DCR_BASE+0x59)  /* RX 25 Channel table ptr reg */
#define	malrxctp26r	(MAL_DCR_BASE+0x5A)  /* RX 26 Channel table ptr reg */
#define	malrxctp27r	(MAL_DCR_BASE+0x5B)  /* RX 27 Channel table ptr reg */
#define	malrxctp28r	(MAL_DCR_BASE+0x5C)  /* RX 28 Channel table ptr reg */
#define	malrxctp29r	(MAL_DCR_BASE+0x5D)  /* RX 29 Channel table ptr reg */
#define	malrxctp30r	(MAL_DCR_BASE+0x5E)  /* RX 30 Channel table ptr reg */
#define	malrxctp31r	(MAL_DCR_BASE+0x5F)  /* RX 31 Channel table ptr reg */
#define	malrcbs0	(MAL_DCR_BASE+0x60)  /* RX 0 Channel buffer size reg */
#define	malrcbs1	(MAL_DCR_BASE+0x61)  /* RX 1 Channel buffer size reg */
#define	malrcbs2	(MAL_DCR_BASE+0x62)  /* RX 2 Channel buffer size reg */
#define	malrcbs3	(MAL_DCR_BASE+0x63)  /* RX 3 Channel buffer size reg */
#define	malrcbs4	(MAL_DCR_BASE+0x64)  /* RX 4 Channel buffer size reg */
#define	malrcbs5	(MAL_DCR_BASE+0x65)  /* RX 5 Channel buffer size reg */
#define	malrcbs6	(MAL_DCR_BASE+0x66)  /* RX 6 Channel buffer size reg */
#define	malrcbs7	(MAL_DCR_BASE+0x67)  /* RX 7 Channel buffer size reg */
#define	malrcbs8	(MAL_DCR_BASE+0x68)  /* RX 8 Channel buffer size reg */
#define	malrcbs9	(MAL_DCR_BASE+0x69)  /* RX 9 Channel buffer size reg */
#define	malrcbs10	(MAL_DCR_BASE+0x6A)  /* RX 10 Channel buffer size reg */
#define	malrcbs11	(MAL_DCR_BASE+0x6B)  /* RX 11 Channel buffer size reg */
#define	malrcbs12	(MAL_DCR_BASE+0x6C)  /* RX 12 Channel buffer size reg */
#define	malrcbs13	(MAL_DCR_BASE+0x6D)  /* RX 13 Channel buffer size reg */
#define	malrcbs14	(MAL_DCR_BASE+0x6E)  /* RX 14 Channel buffer size reg */
#define	malrcbs15	(MAL_DCR_BASE+0x6F)  /* RX 15 Channel buffer size reg */
#define	malrcbs16	(MAL_DCR_BASE+0x70)  /* RX 16 Channel buffer size reg */
#define	malrcbs17	(MAL_DCR_BASE+0x71)  /* RX 17 Channel buffer size reg */
#define	malrcbs18	(MAL_DCR_BASE+0x72)  /* RX 18 Channel buffer size reg */
#define	malrcbs19	(MAL_DCR_BASE+0x73)  /* RX 19 Channel buffer size reg */
#define	malrcbs20	(MAL_DCR_BASE+0x74)  /* RX 20 Channel buffer size reg */
#define	malrcbs21	(MAL_DCR_BASE+0x75)  /* RX 21 Channel buffer size reg */
#define	malrcbs22	(MAL_DCR_BASE+0x76)  /* RX 22 Channel buffer size reg */
#define	malrcbs23	(MAL_DCR_BASE+0x77)  /* RX 23 Channel buffer size reg */
#define	malrcbs24	(MAL_DCR_BASE+0x78)  /* RX 24 Channel buffer size reg */
#define	malrcbs25	(MAL_DCR_BASE+0x79)  /* RX 25 Channel buffer size reg */
#define	malrcbs26	(MAL_DCR_BASE+0x7A)  /* RX 26 Channel buffer size reg */
#define	malrcbs27	(MAL_DCR_BASE+0x7B)  /* RX 27 Channel buffer size reg */
#define	malrcbs28	(MAL_DCR_BASE+0x7C)  /* RX 28 Channel buffer size reg */
#define	malrcbs29	(MAL_DCR_BASE+0x7D)  /* RX 29 Channel buffer size reg */
#define	malrcbs30	(MAL_DCR_BASE+0x7E)  /* RX 30 Channel buffer size reg */
#define	malrcbs31	(MAL_DCR_BASE+0x7F)  /* RX 31 Channel buffer size reg */

#else /* !defined(CONFIG_405EZ) */

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
#endif /* defined(CONFIG_405EZ) */

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
#define		DL_LSB		0x00
#define		DL_MSB		0x01
#define		INT_ENABLE      0x01
#define		FIFO_CONTROL    0x02
#define		LINE_CONTROL    0x03
#define		MODEM_CONTROL   0x04
#define		LINE_STATUS	0x05
#define		MODEM_STATUS    0x06
#define		SCRATCH         0x07

/******************************************************************************
 * On Chip Memory
 ******************************************************************************/
#if defined(CONFIG_405EZ)
#define OCM_DCR_BASE 0x020
#define ocmplb3cr1      (OCM_DCR_BASE+0x00)  /* OCM PLB3 Bank 1 Config Reg    */
#define ocmplb3cr2      (OCM_DCR_BASE+0x01)  /* OCM PLB3 Bank 2 Config Reg    */
#define ocmplb3bear     (OCM_DCR_BASE+0x02)  /* OCM PLB3 Bus Error Add Reg    */
#define ocmplb3besr0    (OCM_DCR_BASE+0x03)  /* OCM PLB3 Bus Error Stat Reg 0 */
#define ocmplb3besr1    (OCM_DCR_BASE+0x04)  /* OCM PLB3 Bus Error Stat Reg 1 */
#define ocmcid          (OCM_DCR_BASE+0x05)  /* OCM Core ID                   */
#define ocmrevid        (OCM_DCR_BASE+0x06)  /* OCM Revision ID               */
#define ocmplb3dpc      (OCM_DCR_BASE+0x07)  /* OCM PLB3 Data Parity Check    */
#define ocmdscr1        (OCM_DCR_BASE+0x08)  /* OCM D-side Bank 1 Config Reg  */
#define ocmdscr2        (OCM_DCR_BASE+0x09)  /* OCM D-side Bank 2 Config Reg  */
#define ocmiscr1        (OCM_DCR_BASE+0x0A)  /* OCM I-side Bank 1 Config Reg  */
#define ocmiscr2        (OCM_DCR_BASE+0x0B)  /* OCM I-side Bank 2 Config Reg  */
#define ocmdsisdpc      (OCM_DCR_BASE+0x0C)  /* OCM D-side/I-side Data Par Chk*/
#define ocmdsisbear     (OCM_DCR_BASE+0x0D)  /* OCM D-side/I-side Bus Err Addr*/
#define ocmdsisbesr     (OCM_DCR_BASE+0x0E)  /* OCM D-side/I-side Bus Err Stat*/
#else
#define OCM_DCR_BASE 0x018
#define ocmisarc   (OCM_DCR_BASE+0x00)  /* OCM I-side address compare reg    */
#define ocmiscntl  (OCM_DCR_BASE+0x01)  /* OCM I-side control reg            */
#define ocmdsarc   (OCM_DCR_BASE+0x02)  /* OCM D-side address compare reg    */
#define ocmdscntl  (OCM_DCR_BASE+0x03)  /* OCM D-side control reg            */
#endif /* CONFIG_405EZ */

/******************************************************************************
 * GPIO macro register defines
 ******************************************************************************/
#if defined(CONFIG_405EZ)
/* Only the 405EZ has 2 GPIOs */
#define GPIO_BASE  0xEF600700
#define GPIO0_OR		(GPIO_BASE+0x0)
#define GPIO0_TCR		(GPIO_BASE+0x4)
#define GPIO0_OSRL		(GPIO_BASE+0x8)
#define GPIO0_OSRH		(GPIO_BASE+0xC)
#define GPIO0_TSRL		(GPIO_BASE+0x10)
#define GPIO0_TSRH		(GPIO_BASE+0x14)
#define GPIO0_ODR		(GPIO_BASE+0x18)
#define GPIO0_IR		(GPIO_BASE+0x1C)
#define GPIO0_RR1		(GPIO_BASE+0x20)
#define GPIO0_RR2		(GPIO_BASE+0x24)
#define GPIO0_RR3		(GPIO_BASE+0x28)
#define GPIO0_ISR1L		(GPIO_BASE+0x30)
#define GPIO0_ISR1H		(GPIO_BASE+0x34)
#define GPIO0_ISR2L		(GPIO_BASE+0x38)
#define GPIO0_ISR2H		(GPIO_BASE+0x3C)
#define GPIO0_ISR3L		(GPIO_BASE+0x40)
#define GPIO0_ISR3H		(GPIO_BASE+0x44)

#define GPIO1_BASE  0xEF600800
#define GPIO1_OR		(GPIO1_BASE+0x0)
#define GPIO1_TCR		(GPIO1_BASE+0x4)
#define GPIO1_OSRL		(GPIO1_BASE+0x8)
#define GPIO1_OSRH		(GPIO1_BASE+0xC)
#define GPIO1_TSRL		(GPIO1_BASE+0x10)
#define GPIO1_TSRH		(GPIO1_BASE+0x14)
#define GPIO1_ODR		(GPIO1_BASE+0x18)
#define GPIO1_IR		(GPIO1_BASE+0x1C)
#define GPIO1_RR1		(GPIO1_BASE+0x20)
#define GPIO1_RR2		(GPIO1_BASE+0x24)
#define GPIO1_RR3		(GPIO1_BASE+0x28)
#define GPIO1_ISR1L		(GPIO1_BASE+0x30)
#define GPIO1_ISR1H		(GPIO1_BASE+0x34)
#define GPIO1_ISR2L		(GPIO1_BASE+0x38)
#define GPIO1_ISR2H		(GPIO1_BASE+0x3C)
#define GPIO1_ISR3L		(GPIO1_BASE+0x40)
#define GPIO1_ISR3H		(GPIO1_BASE+0x44)

#elif defined(CONFIG_405EX)
#define GPIO_BASE  0xEF600800
#define GPIO0_OR               (GPIO_BASE+0x0)
#define GPIO0_TCR              (GPIO_BASE+0x4)
#define GPIO0_OSRL             (GPIO_BASE+0x8)
#define GPIO0_OSRH             (GPIO_BASE+0xC)
#define GPIO0_TSRL             (GPIO_BASE+0x10)
#define GPIO0_TSRH             (GPIO_BASE+0x14)
#define GPIO0_ODR              (GPIO_BASE+0x18)
#define GPIO0_IR               (GPIO_BASE+0x1C)
#define GPIO0_RR1              (GPIO_BASE+0x20)
#define GPIO0_RR2              (GPIO_BASE+0x24)
#define GPIO0_ISR1L            (GPIO_BASE+0x30)
#define GPIO0_ISR1H            (GPIO_BASE+0x34)
#define GPIO0_ISR2L            (GPIO_BASE+0x38)
#define GPIO0_ISR2H            (GPIO_BASE+0x3C)
#define GPIO0_ISR3L            (GPIO_BASE+0x40)
#define GPIO0_ISR3H            (GPIO_BASE+0x44)

#else	/* !405EZ */

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

#endif /* CONFIG_405EZ */

#if defined(CONFIG_405EX)
#define SDR0_SRST		0x0200

#define SDRAM_BESR0	0x00
#define SDRAM_BEARL	0x02
#define SDRAM_BEARU	0x03
#define SDRAM_WMIRQ	0x06	/**/
#define SDRAM_PLBOPT	0x08	/**/
#define SDRAM_PUABA	0x09	/**/
#define SDRAM_MCSTAT    0x1F    /* memory controller status           */
#define SDRAM_MCOPT1    0x20    /* memory controller options 1        */
#define SDRAM_MCOPT2    0x21    /* memory controller options 2        */
#define SDRAM_MODT0     0x22    /* on die termination for bank 0      */
#define SDRAM_MODT1     0x23    /* on die termination for bank 1      */
#define SDRAM_MODT2     0x24    /* on die termination for bank 2      */
#define SDRAM_MODT3     0x25    /* on die termination for bank 3      */
#define SDRAM_CODT      0x26    /* on die termination for controller  */
#define SDRAM_VVPR      0x27    /* variable VRef programmming         */
#define SDRAM_OPARS     0x28    /* on chip driver control setup       */
#define SDRAM_OPART     0x29    /* on chip driver control trigger     */
#define SDRAM_RTR       0x30    /* refresh timer                      */
#define SDRAM_PMIT      0x34    /* power management idle timer        */
#define SDRAM_MB0CF     0x40    /* memory bank 0 configuration        */
#define SDRAM_MB1CF     0x44    /* memory bank 1 configuration        */
#define SDRAM_MB2CF     0x48    /* memory bank 2 configuration        */
#define SDRAM_MB3CF     0x4C    /* memory bank 3 configuration        */
#define SDRAM_INITPLR0  0x50    /* manual initialization control      */
#define SDRAM_INITPLR1  0x51    /* manual initialization control      */
#define SDRAM_INITPLR2  0x52    /* manual initialization control      */
#define SDRAM_INITPLR3  0x53    /* manual initialization control      */
#define SDRAM_INITPLR4  0x54    /* manual initialization control      */
#define SDRAM_INITPLR5  0x55    /* manual initialization control      */
#define SDRAM_INITPLR6  0x56    /* manual initialization control      */
#define SDRAM_INITPLR7  0x57    /* manual initialization control      */
#define SDRAM_INITPLR8  0x58    /* manual initialization control      */
#define SDRAM_INITPLR9  0x59    /* manual initialization control      */
#define SDRAM_INITPLR10 0x5a    /* manual initialization control      */
#define SDRAM_INITPLR11 0x5b    /* manual initialization control      */
#define SDRAM_INITPLR12 0x5c    /* manual initialization control      */
#define SDRAM_INITPLR13 0x5d    /* manual initialization control      */
#define SDRAM_INITPLR14 0x5e    /* manual initialization control      */
#define SDRAM_INITPLR15 0x5f    /* manual initialization control      */
#define SDRAM_RQDC      0x70    /* read DQS delay control             */
#define SDRAM_RFDC      0x74    /* read feedback delay control        */
#define SDRAM_RDCC      0x78    /* read data capture control          */
#define SDRAM_DLCR      0x7A    /* delay line calibration             */
#define SDRAM_CLKTR     0x80    /* DDR clock timing                   */
#define SDRAM_WRDTR     0x81    /* write data, DQS, DM clock, timing  */
#define SDRAM_SDTR1     0x85    /* DDR SDRAM timing 1                 */
#define SDRAM_SDTR2     0x86    /* DDR SDRAM timing 2                 */
#define SDRAM_SDTR3     0x87    /* DDR SDRAM timing 3                 */
#define SDRAM_MMODE     0x88    /* memory mode                        */
#define SDRAM_MEMODE    0x89    /* memory extended mode               */
#define SDRAM_ECCCR     0x98    /* ECC error status                   */
#define SDRAM_RID       0xF8    /* revision ID                        */

/*-----------------------------------------------------------------------------+
|  Memory Bank 0-7 configuration
+-----------------------------------------------------------------------------*/
#define SDRAM_RXBAS_SDSZ_4         0x00000000      /*   4M                    */
#define SDRAM_RXBAS_SDSZ_8         0x00001000      /*   8M                    */
#define SDRAM_RXBAS_SDSZ_16        0x00002000      /*  16M                    */
#define SDRAM_RXBAS_SDSZ_32        0x00003000      /*  32M                    */
#define SDRAM_RXBAS_SDSZ_64        0x00004000      /*  64M                    */
#define SDRAM_RXBAS_SDSZ_128       0x00005000      /* 128M                    */
#define SDRAM_RXBAS_SDSZ_256       0x00006000      /* 256M                    */
#define SDRAM_RXBAS_SDSZ_512       0x00007000      /* 512M                    */
#define SDRAM_RXBAS_SDSZ_1024      0x00008000      /* 1024M                   */
#define SDRAM_RXBAS_SDSZ_2048      0x00009000      /* 2048M                   */
#define SDRAM_RXBAS_SDSZ_4096      0x0000a000      /* 4096M                   */
#define SDRAM_RXBAS_SDSZ_8192      0x0000b000      /* 8192M                   */

/*-----------------------------------------------------------------------------+
|  Memory Controller Status
+-----------------------------------------------------------------------------*/
#define SDRAM_MCSTAT_MIC_MASK       0x80000000  /* Memory init status mask    */
#define   SDRAM_MCSTAT_MIC_NOTCOMP  0x00000000  /* Mem init not complete      */
#define   SDRAM_MCSTAT_MIC_COMP     0x80000000  /* Mem init complete          */
#define SDRAM_MCSTAT_SRMS_MASK      0x80000000  /* Mem self refresh stat mask */
#define   SDRAM_MCSTAT_SRMS_NOT_SF  0x00000000  /* Mem not in self refresh    */
#define   SDRAM_MCSTAT_SRMS_SF      0x80000000  /* Mem in self refresh        */

/*-----------------------------------------------------------------------------+
|  Memory Controller Options 1
+-----------------------------------------------------------------------------*/
#define SDRAM_MCOPT1_MCHK_MASK       0x30000000 /* Memory data err check mask */
#define   SDRAM_MCOPT1_MCHK_NON      0x00000000 /* No ECC generation          */
#define   SDRAM_MCOPT1_MCHK_GEN      0x20000000 /* ECC generation             */
#define   SDRAM_MCOPT1_MCHK_CHK      0x10000000 /* ECC generation and check   */
#define   SDRAM_MCOPT1_MCHK_CHK_REP  0x30000000 /* ECC generation, chk, report*/
#define   SDRAM_MCOPT1_MCHK_CHK_DECODE(n)  ((((unsigned long)(n))>>28)&0x3)
#define SDRAM_MCOPT1_RDEN_MASK       0x08000000 /* Registered DIMM mask       */
#define   SDRAM_MCOPT1_RDEN          0x08000000 /* Registered DIMM enable     */
#define SDRAM_MCOPT1_PMU_MASK        0x06000000 /* Page management unit mask  */
#define   SDRAM_MCOPT1_PMU_CLOSE     0x00000000 /* PMU Close                  */
#define   SDRAM_MCOPT1_PMU_OPEN      0x04000000 /* PMU Open                   */
#define   SDRAM_MCOPT1_PMU_AUTOCLOSE 0x02000000 /* PMU AutoClose              */
#define SDRAM_MCOPT1_DMWD_MASK       0x01000000 /* DRAM width mask            */
#define   SDRAM_MCOPT1_DMWD_32       0x00000000 /* 32 bits                    */
#define   SDRAM_MCOPT1_DMWD_64       0x01000000 /* 64 bits                    */
#define SDRAM_MCOPT1_UIOS_MASK       0x00C00000 /* Unused IO State            */
#define SDRAM_MCOPT1_BCNT_MASK       0x00200000 /* Bank count                 */
#define   SDRAM_MCOPT1_4_BANKS       0x00000000 /* 4 Banks                    */
#define   SDRAM_MCOPT1_8_BANKS       0x00200000 /* 8 Banks                    */
#define SDRAM_MCOPT1_DDR_TYPE_MASK   0x00100000 /* DDR Memory Type mask       */
#define   SDRAM_MCOPT1_DDR1_TYPE     0x00000000 /* DDR1 Memory Type           */
#define   SDRAM_MCOPT1_DDR2_TYPE     0x00100000 /* DDR2 Memory Type           */
#define   SDRAM_MCOPT1_QDEP          0x00020000 /* 4 commands deep            */
#define SDRAM_MCOPT1_RWOO_MASK       0x00008000 /* Out of Order Read mask     */
#define   SDRAM_MCOPT1_RWOO_DISABLED 0x00000000 /* disabled                   */
#define   SDRAM_MCOPT1_RWOO_ENABLED  0x00008000 /* enabled                    */
#define SDRAM_MCOPT1_WOOO_MASK       0x00004000 /* Out of Order Write mask    */
#define   SDRAM_MCOPT1_WOOO_DISABLED 0x00000000 /* disabled                   */
#define   SDRAM_MCOPT1_WOOO_ENABLED  0x00004000 /* enabled                    */
#define SDRAM_MCOPT1_DCOO_MASK       0x00002000 /* All Out of Order mask      */
#define   SDRAM_MCOPT1_DCOO_DISABLED 0x00002000 /* disabled                   */
#define   SDRAM_MCOPT1_DCOO_ENABLED  0x00000000 /* enabled                    */
#define SDRAM_MCOPT1_DREF_MASK       0x00001000 /* Deferred refresh mask      */
#define   SDRAM_MCOPT1_DREF_NORMAL   0x00000000 /* normal refresh             */
#define   SDRAM_MCOPT1_DREF_DEFER_4  0x00001000 /* defer up to 4 refresh cmd  */

/*-----------------------------------------------------------------------------+
|  Memory Controller Options 2
+-----------------------------------------------------------------------------*/
#define SDRAM_MCOPT2_SREN_MASK        0x80000000 /* Self Test mask            */
#define   SDRAM_MCOPT2_SREN_EXIT      0x00000000 /* Self Test exit            */
#define   SDRAM_MCOPT2_SREN_ENTER     0x80000000 /* Self Test enter           */
#define SDRAM_MCOPT2_PMEN_MASK        0x40000000 /* Power Management mask     */
#define   SDRAM_MCOPT2_PMEN_DISABLE   0x00000000 /* disable                   */
#define   SDRAM_MCOPT2_PMEN_ENABLE    0x40000000 /* enable                    */
#define SDRAM_MCOPT2_IPTR_MASK        0x20000000 /* Init Trigger Reg mask     */
#define   SDRAM_MCOPT2_IPTR_IDLE      0x00000000 /* idle                      */
#define   SDRAM_MCOPT2_IPTR_EXECUTE   0x20000000 /* execute preloaded init    */
#define SDRAM_MCOPT2_XSRP_MASK        0x10000000 /* Exit Self Refresh Prevent */
#define   SDRAM_MCOPT2_XSRP_ALLOW     0x00000000 /* allow self refresh exit   */
#define   SDRAM_MCOPT2_XSRP_PREVENT   0x10000000 /* prevent self refresh exit */
#define SDRAM_MCOPT2_DCEN_MASK        0x08000000 /* SDRAM Controller Enable   */
#define   SDRAM_MCOPT2_DCEN_DISABLE   0x00000000 /* SDRAM Controller Enable   */
#define   SDRAM_MCOPT2_DCEN_ENABLE    0x08000000 /* SDRAM Controller Enable   */
#define SDRAM_MCOPT2_ISIE_MASK        0x04000000 /* Init Seq Interruptable mas*/
#define   SDRAM_MCOPT2_ISIE_DISABLE   0x00000000 /* disable                   */
#define   SDRAM_MCOPT2_ISIE_ENABLE    0x04000000 /* enable                    */

/*-----------------------------------------------------------------------------+
|  SDRAM Refresh Timer Register
+-----------------------------------------------------------------------------*/
#define SDRAM_RTR_RINT_MASK       0xFFF80000
#define   SDRAM_RTR_RINT_ENCODE(n)  ((((unsigned long)(n))&0xFFF8)<<16)
#define   SDRAM_RTR_RINT_DECODE(n)  ((((unsigned long)(n))>>16)&0xFFF8)

/*-----------------------------------------------------------------------------+
|  SDRAM Read DQS Delay Control Register
+-----------------------------------------------------------------------------*/
#define SDRAM_RQDC_RQDE_MASK        0x80000000
#define   SDRAM_RQDC_RQDE_DISABLE   0x00000000
#define   SDRAM_RQDC_RQDE_ENABLE    0x80000000
#define SDRAM_RQDC_RQFD_MASK        0x000001FF
#define   SDRAM_RQDC_RQFD_ENCODE(n) ((((unsigned long)(n))&0x1FF)<<0)

#define SDRAM_RQDC_RQFD_MAX         0xFF

/*-----------------------------------------------------------------------------+
|  SDRAM Read Data Capture Control Register
+-----------------------------------------------------------------------------*/
#define SDRAM_RDCC_RDSS_MASK        0xC0000000
#define   SDRAM_RDCC_RDSS_T1        0x00000000
#define   SDRAM_RDCC_RDSS_T2        0x40000000
#define   SDRAM_RDCC_RDSS_T3        0x80000000
#define   SDRAM_RDCC_RDSS_T4        0xC0000000
#define SDRAM_RDCC_RSAE_MASK        0x00000001
#define   SDRAM_RDCC_RSAE_DISABLE   0x00000001
#define   SDRAM_RDCC_RSAE_ENABLE    0x00000000

/*-----------------------------------------------------------------------------+
|  SDRAM Read Feedback Delay Control Register
+-----------------------------------------------------------------------------*/
#define SDRAM_RFDC_ARSE_MASK        0x80000000
#define   SDRAM_RFDC_ARSE_DISABLE   0x80000000
#define   SDRAM_RFDC_ARSE_ENABLE    0x00000000
#define SDRAM_RFDC_RFOS_MASK        0x007F0000
#define   SDRAM_RFDC_RFOS_ENCODE(n) ((((unsigned long)(n))&0x7F)<<16)
#define SDRAM_RFDC_RFFD_MASK        0x000003FF
#define   SDRAM_RFDC_RFFD_ENCODE(n) ((((unsigned long)(n))&0x3FF)<<0)

#define SDRAM_RFDC_RFFD_MAX         0x4FF

/*-----------------------------------------------------------------------------+
|  SDRAM Delay Line Calibration Register
+-----------------------------------------------------------------------------*/
#define SDRAM_DLCR_DCLM_MASK          0x80000000
#define   SDRAM_DLCR_DCLM_MANUEL      0x80000000
#define   SDRAM_DLCR_DCLM_AUTO        0x00000000
#define SDRAM_DLCR_DLCR_MASK          0x08000000
#define   SDRAM_DLCR_DLCR_CALIBRATE   0x08000000
#define   SDRAM_DLCR_DLCR_IDLE        0x00000000
#define SDRAM_DLCR_DLCS_MASK          0x07000000
#define   SDRAM_DLCR_DLCS_NOT_RUN     0x00000000
#define   SDRAM_DLCR_DLCS_IN_PROGRESS 0x01000000
#define   SDRAM_DLCR_DLCS_COMPLETE    0x02000000
#define   SDRAM_DLCR_DLCS_CONT_DONE   0x03000000
#define   SDRAM_DLCR_DLCS_ERROR       0x04000000
#define SDRAM_DLCR_DLCV_MASK          0x000001FF
#define   SDRAM_DLCR_DLCV_ENCODE(n) ((((unsigned long)(n))&0x1FF)<<0)
#define   SDRAM_DLCR_DLCV_DECODE(n) ((((unsigned long)(n))>>0)&0x1FF)

/*-----------------------------------------------------------------------------+
|  SDRAM Controller On Die Termination Register
+-----------------------------------------------------------------------------*/
#define   SDRAM_CODT_ODT_ON                   0x80000000
#define   SDRAM_CODT_ODT_OFF                  0x00000000
#define SDRAM_CODT_DQS_VOLTAGE_DDR_MASK       0x00000020
#define   SDRAM_CODT_DQS_2_5_V_DDR1           0x00000000
#define   SDRAM_CODT_DQS_1_8_V_DDR2           0x00000020
#define SDRAM_CODT_DQS_MASK                   0x00000010
#define   SDRAM_CODT_DQS_DIFFERENTIAL         0x00000000
#define   SDRAM_CODT_DQS_SINGLE_END           0x00000010
#define   SDRAM_CODT_CKSE_DIFFERENTIAL         	0x00000000
#define   SDRAM_CODT_CKSE_SINGLE_END           	0x00000008
#define   SDRAM_CODT_FEEBBACK_RCV_SINGLE_END  0x00000004
#define   SDRAM_CODT_FEEBBACK_DRV_SINGLE_END  0x00000002
#define   SDRAM_CODT_IO_HIZ  					0x00000000
#define   SDRAM_CODT_IO_NMODE  					0x00000001

/*-----------------------------------------------------------------------------+
|  SDRAM Mode Register
+-----------------------------------------------------------------------------*/
#define SDRAM_MMODE_WR_MASK              0x00000E00
#define   SDRAM_MMODE_WR_DDR1            0x00000000
#define   SDRAM_MMODE_WR_DDR2_3_CYC      0x00000400
#define   SDRAM_MMODE_WR_DDR2_4_CYC      0x00000600
#define   SDRAM_MMODE_WR_DDR2_5_CYC      0x00000800
#define   SDRAM_MMODE_WR_DDR2_6_CYC      0x00000A00
#define SDRAM_MMODE_DCL_MASK             0x00000070
#define   SDRAM_MMODE_DCL_DDR1_2_0_CLK   0x00000020
#define   SDRAM_MMODE_DCL_DDR1_2_5_CLK   0x00000060
#define   SDRAM_MMODE_DCL_DDR1_3_0_CLK   0x00000030
#define   SDRAM_MMODE_DCL_DDR2_2_0_CLK   0x00000020
#define   SDRAM_MMODE_DCL_DDR2_3_0_CLK   0x00000030
#define   SDRAM_MMODE_DCL_DDR2_4_0_CLK   0x00000040
#define   SDRAM_MMODE_DCL_DDR2_5_0_CLK   0x00000050
#define   SDRAM_MMODE_DCL_DDR2_6_0_CLK   0x00000060
#define   SDRAM_MMODE_DCL_DDR2_7_0_CLK   0x00000070

/*-----------------------------------------------------------------------------+
|  SDRAM Extended Mode Register
+-----------------------------------------------------------------------------*/
#define SDRAM_MEMODE_DIC_MASK            0x00000002
#define   SDRAM_MEMODE_DIC_NORMAL        0x00000000
#define   SDRAM_MEMODE_DIC_WEAK          0x00000002
#define SDRAM_MEMODE_DLL_MASK            0x00000001
#define   SDRAM_MEMODE_DLL_DISABLE       0x00000001
#define   SDRAM_MEMODE_DLL_ENABLE        0x00000000
#define SDRAM_MEMODE_RTT_MASK 		     0x00000044
#define   SDRAM_MEMODE_RTT_DISABLED      0x00000000
#define   SDRAM_MEMODE_RTT_75OHM         0x00000004
#define   SDRAM_MEMODE_RTT_150OHM        0x00000040
#define SDRAM_MEMODE_DQS_MASK            0x00000400
#define   SDRAM_MEMODE_DQS_DISABLE       0x00000400
#define   SDRAM_MEMODE_DQS_ENABLE        0x00000000

/*-----------------------------------------------------------------------------+
|  SDRAM Clock Timing Register
+-----------------------------------------------------------------------------*/
#define SDRAM_CLKTR_CLKP_MASK            0xC0000000
#define   SDRAM_CLKTR_CLKP_0_DEG         0x00000000
#define   SDRAM_CLKTR_CLKP_180_DEG_ADV   0x80000000

/*-----------------------------------------------------------------------------+
|  SDRAM Write Timing Register
+-----------------------------------------------------------------------------*/
#define SDRAM_WRDTR_WDTP_1_CYC		 0x80000000
#define SDRAM_WRDTR_LLWP_MASK            0x10000000
#define   SDRAM_WRDTR_LLWP_DIS           0x10000000
#define   SDRAM_WRDTR_LLWP_1_CYC         0x00000000
#define SDRAM_WRDTR_WTR_MASK             0x0E000000
#define   SDRAM_WRDTR_WTR_0_DEG          0x06000000
#define   SDRAM_WRDTR_WTR_180_DEG_ADV    0x02000000
#define   SDRAM_WRDTR_WTR_270_DEG_ADV    0x00000000

/*-----------------------------------------------------------------------------+
|  SDRAM SDTR1 Options
+-----------------------------------------------------------------------------*/
#define SDRAM_SDTR1_LDOF_MASK        0x80000000
#define   SDRAM_SDTR1_LDOF_1_CLK     0x00000000
#define   SDRAM_SDTR1_LDOF_2_CLK     0x80000000
#define SDRAM_SDTR1_RTW_MASK         0x00F00000
#define   SDRAM_SDTR1_RTW_2_CLK      0x00200000
#define   SDRAM_SDTR1_RTW_3_CLK      0x00300000
#define SDRAM_SDTR1_WTWO_MASK        0x000F0000
#define   SDRAM_SDTR1_WTWO_0_CLK     0x00000000
#define   SDRAM_SDTR1_WTWO_1_CLK     0x00010000
#define SDRAM_SDTR1_RTRO_MASK        0x0000F000
#define   SDRAM_SDTR1_RTRO_1_CLK     0x00000000
#define   SDRAM_SDTR1_RTRO_2_CLK     0x00002000

/*-----------------------------------------------------------------------------+
|  SDRAM SDTR2 Options
+-----------------------------------------------------------------------------*/
#define SDRAM_SDTR2_RCD_MASK         0xF0000000
#define   SDRAM_SDTR2_RCD_1_CLK      0x10000000
#define   SDRAM_SDTR2_RCD_2_CLK      0x20000000
#define   SDRAM_SDTR2_RCD_3_CLK      0x30000000
#define   SDRAM_SDTR2_RCD_4_CLK      0x40000000
#define   SDRAM_SDTR2_RCD_5_CLK      0x50000000
#define SDRAM_SDTR2_WTR_MASK         0x0F000000
#define   SDRAM_SDTR2_WTR_1_CLK      0x01000000
#define   SDRAM_SDTR2_WTR_2_CLK      0x02000000
#define   SDRAM_SDTR2_WTR_3_CLK      0x03000000
#define   SDRAM_SDTR2_WTR_4_CLK      0x04000000
#define   SDRAM_SDTR3_WTR_ENCODE(n)  ((((unsigned long)(n))&0xF)<<24)
#define SDRAM_SDTR2_XSNR_MASK        0x00FF0000
#define   SDRAM_SDTR2_XSNR_8_CLK     0x00080000
#define   SDRAM_SDTR2_XSNR_16_CLK    0x00100000
#define   SDRAM_SDTR2_XSNR_32_CLK    0x00200000
#define   SDRAM_SDTR2_XSNR_64_CLK    0x00400000
#define SDRAM_SDTR2_WPC_MASK         0x0000F000
#define   SDRAM_SDTR2_WPC_2_CLK      0x00002000
#define   SDRAM_SDTR2_WPC_3_CLK      0x00003000
#define   SDRAM_SDTR2_WPC_4_CLK      0x00004000
#define   SDRAM_SDTR2_WPC_5_CLK      0x00005000
#define   SDRAM_SDTR2_WPC_6_CLK      0x00006000
#define   SDRAM_SDTR3_WPC_ENCODE(n)  ((((unsigned long)(n))&0xF)<<12)
#define SDRAM_SDTR2_RPC_MASK         0x00000F00
#define   SDRAM_SDTR2_RPC_2_CLK      0x00000200
#define   SDRAM_SDTR2_RPC_3_CLK      0x00000300
#define   SDRAM_SDTR2_RPC_4_CLK      0x00000400
#define SDRAM_SDTR2_RP_MASK          0x000000F0
#define   SDRAM_SDTR2_RP_3_CLK       0x00000030
#define   SDRAM_SDTR2_RP_4_CLK       0x00000040
#define   SDRAM_SDTR2_RP_5_CLK       0x00000050
#define   SDRAM_SDTR2_RP_6_CLK       0x00000060
#define   SDRAM_SDTR2_RP_7_CLK       0x00000070
#define SDRAM_SDTR2_RRD_MASK         0x0000000F
#define   SDRAM_SDTR2_RRD_2_CLK      0x00000002
#define   SDRAM_SDTR2_RRD_3_CLK      0x00000003

/*-----------------------------------------------------------------------------+
|  SDRAM SDTR3 Options
+-----------------------------------------------------------------------------*/
#define SDRAM_SDTR3_RAS_MASK         0x1F000000
#define   SDRAM_SDTR3_RAS_ENCODE(n)  ((((unsigned long)(n))&0x1F)<<24)
#define SDRAM_SDTR3_RC_MASK          0x001F0000
#define   SDRAM_SDTR3_RC_ENCODE(n)   ((((unsigned long)(n))&0x1F)<<16)
#define SDRAM_SDTR3_XCS_MASK         0x00001F00
#define SDRAM_SDTR3_XCS              0x00000D00
#define SDRAM_SDTR3_RFC_MASK         0x0000003F
#define   SDRAM_SDTR3_RFC_ENCODE(n)  ((((unsigned long)(n))&0x3F)<<0)

/*-----------------------------------------------------------------------------+
|  Memory Bank 0-1 configuration
+-----------------------------------------------------------------------------*/
#define SDRAM_BXCF_M_AM_MASK      0x00000F00      /* Addressing mode          */
#define   SDRAM_BXCF_M_AM_0       0x00000000      /*   Mode 0                 */
#define   SDRAM_BXCF_M_AM_1       0x00000100      /*   Mode 1                 */
#define   SDRAM_BXCF_M_AM_2       0x00000200      /*   Mode 2                 */
#define   SDRAM_BXCF_M_AM_3       0x00000300      /*   Mode 3                 */
#define   SDRAM_BXCF_M_AM_4       0x00000400      /*   Mode 4                 */
#define   SDRAM_BXCF_M_AM_5       0x00000500      /*   Mode 5                 */
#define   SDRAM_BXCF_M_AM_6       0x00000600      /*   Mode 6                 */
#define   SDRAM_BXCF_M_AM_7       0x00000700      /*   Mode 7                 */
#define   SDRAM_BXCF_M_AM_8       0x00000800      /*   Mode 8                 */
#define   SDRAM_BXCF_M_AM_9       0x00000900      /*   Mode 9                 */
#define SDRAM_BXCF_M_BE_MASK      0x00000001      /* Memory Bank Enable       */
#define   SDRAM_BXCF_M_BE_DISABLE 0x00000000      /* Memory Bank Enable       */
#define   SDRAM_BXCF_M_BE_ENABLE  0x00000001      /* Memory Bank Enable       */

#define sdr_uart0	0x0120	/* UART0 Config */
#define sdr_uart1	0x0121	/* UART1 Config */
#define sdr_mfr		0x4300	/* SDR0_MFR reg */

/* Defines for CPC0_EPRCSR register */
#define CPC0_EPRCSR_E0NFE          0x80000000
#define CPC0_EPRCSR_E1NFE          0x40000000
#define CPC0_EPRCSR_E1RPP          0x00000080
#define CPC0_EPRCSR_E0RPP          0x00000040
#define CPC0_EPRCSR_E1ERP          0x00000020
#define CPC0_EPRCSR_E0ERP          0x00000010
#define CPC0_EPRCSR_E1PCI          0x00000002
#define CPC0_EPRCSR_E0PCI          0x00000001

#define cpr0_clkupd	0x020
#define cpr0_pllc	0x040
#define cpr0_plld	0x060
#define cpr0_cpud	0x080
#define cpr0_plbd	0x0a0
#define cpr0_opbd	0x0c0
#define cpr0_perd	0x0e0
#define cpr0_ahbd	0x100
#define cpr0_icfg	0x140

#define SDR_PINSTP	0x0040
#define sdr_sdcs	0x0060

#define SDR0_SDCS_SDD			(0x80000000 >> 31)

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
#endif

#endif	/* __PPC405_H__ */
