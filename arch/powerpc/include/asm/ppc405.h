/*
 * SPDX-License-Identifier:	GPL-2.0	IBM-pibs
 */

#ifndef	__PPC405_H__
#define __PPC405_H__

/* Define bits and masks for real-mode storage attribute control registers */
#define PPC_128MB_SACR_BIT(addr)	((addr) >> 27)
#define PPC_128MB_SACR_VALUE(addr)	PPC_REG_VAL(PPC_128MB_SACR_BIT(addr),1)

#define CONFIG_SYS_DCACHE_SIZE		(16 << 10)	/* For AMCC 405 CPUs */

/* DCR registers */
#define PLB0_ACR	0x0087

/* SDR registers */
#define SDR0_PINSTP	0x0040

/* CPR registers */
#define CPR0_CLKUPD	0x0020
#define CPR0_PLLC	0x0040
#define CPR0_PLLD	0x0060
#define CPR0_CPUD	0x0080
#define CPR0_PLBD	0x00a0
#define CPR0_OPBD0	0x00c0
#define CPR0_PERD	0x00e0

/*
 * DMA
 */
#define DMA_DCR_BASE	0x0100
#define DMACR0		(DMA_DCR_BASE + 0x00)  /* DMA channel control reg 0	*/
#define DMACT0		(DMA_DCR_BASE + 0x01)  /* DMA count reg 0		*/
#define DMADA0		(DMA_DCR_BASE + 0x02)  /* DMA destination address reg 0 */
#define DMASA0		(DMA_DCR_BASE + 0x03)  /* DMA source address reg 0	*/
#define DMASB0		(DMA_DCR_BASE + 0x04)  /* DMA sg descriptor addr 0	*/
#define DMACR1		(DMA_DCR_BASE + 0x08)  /* DMA channel control reg 1	*/
#define DMACT1		(DMA_DCR_BASE + 0x09)  /* DMA count reg 1		*/
#define DMADA1		(DMA_DCR_BASE + 0x0a)  /* DMA destination address reg 1 */
#define DMASA1		(DMA_DCR_BASE + 0x0b)  /* DMA source address reg 1	*/
#define DMASB1		(DMA_DCR_BASE + 0x0c)  /* DMA sg descriptor addr 1	*/
#define DMACR2		(DMA_DCR_BASE + 0x10)  /* DMA channel control reg 2	*/
#define DMACT2		(DMA_DCR_BASE + 0x11)  /* DMA count reg 2		*/
#define DMADA2		(DMA_DCR_BASE + 0x12)  /* DMA destination address reg 2 */
#define DMASA2		(DMA_DCR_BASE + 0x13)  /* DMA source address reg 2	*/
#define DMASB2		(DMA_DCR_BASE + 0x14)  /* DMA sg descriptor addr 2	*/
#define DMACR3		(DMA_DCR_BASE + 0x18)  /* DMA channel control reg 3	*/
#define DMACT3		(DMA_DCR_BASE + 0x19)  /* DMA count reg 3		*/
#define DMADA3		(DMA_DCR_BASE + 0x1a)  /* DMA destination address reg 3 */
#define DMASA3		(DMA_DCR_BASE + 0x1b)  /* DMA source address reg 3	*/
#define DMASB3		(DMA_DCR_BASE + 0x1c)  /* DMA sg descriptor addr 3	*/
#define DMASR		(DMA_DCR_BASE + 0x20)  /* DMA status reg		*/
#define DMASGC		(DMA_DCR_BASE + 0x23)  /* DMA scatter/gather command reg*/
#define DMAADR		(DMA_DCR_BASE + 0x24)  /* DMA address decode reg	*/

#endif	/* __PPC405_H__ */
