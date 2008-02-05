/*
 * DMA Masks
 */

#ifndef __BFIN_PERIPHERAL_DMA__
#define __BFIN_PERIPHERAL_DMA__

/* DMAx_CONFIG, MDMA_yy_CONFIG Masks */
#define DMAEN			0x0001	/* DMA Channel Enable */
#define WNR			0x0002	/* Channel Direction (W/R*) */
#define WDSIZE_8		0x0000	/* Transfer Word Size = 8 */
#define WDSIZE_16		0x0004	/* Transfer Word Size = 16 */
#define WDSIZE_32		0x0008	/* Transfer Word Size = 32 */
#define DMA2D			0x0010	/* DMA Mode (2D/1D*) */
#define RESTART			0x0020	/* DMA Buffer Clear */
#define DI_SEL			0x0040	/* Data Interrupt Timing Select */
#define DI_EN			0x0080	/* Data Interrupt Enable */
#define NDSIZE			0x0F00	/* Next Descriptor bitmask */
#define NDSIZE_0		0x0000	/* Next Descriptor Size = 0 (Stop/Autobuffer) */
#define NDSIZE_1		0x0100	/* Next Descriptor Size = 1 */
#define NDSIZE_2		0x0200	/* Next Descriptor Size = 2 */
#define NDSIZE_3		0x0300	/* Next Descriptor Size = 3 */
#define NDSIZE_4		0x0400	/* Next Descriptor Size = 4 */
#define NDSIZE_5		0x0500	/* Next Descriptor Size = 5 */
#define NDSIZE_6		0x0600	/* Next Descriptor Size = 6 */
#define NDSIZE_7		0x0700	/* Next Descriptor Size = 7 */
#define NDSIZE_8		0x0800	/* Next Descriptor Size = 8 */
#define NDSIZE_9		0x0900	/* Next Descriptor Size = 9 */
#define FLOW_STOP		0x0000	/* Stop Mode */
#define FLOW_AUTO		0x1000	/* Autobuffer Mode */
#define FLOW_ARRAY		0x4000	/* Descriptor Array Mode */
#define FLOW_SMALL		0x6000	/* Small Model Descriptor List Mode */
#define FLOW_LARGE		0x7000	/* Large Model Descriptor List Mode */

#define DMAEN_P			0	/* Channel Enable */
#define WNR_P			1	/* Channel Direction (W/R*) */
#define DMA2D_P			4	/* 2D/1D* Mode */
#define RESTART_P		5	/* Restart */
#define DI_SEL_P		6	/* Data Interrupt Select */
#define DI_EN_P			7	/* Data Interrupt Enable */

/* DMAx_IRQ_STATUS, MDMA_yy_IRQ_STATUS Masks */
#define DMA_DONE		0x0001	/* DMA Completion Interrupt Status */
#define DMA_ERR			0x0002	/* DMA Error Interrupt Status */
#define DFETCH			0x0004	/* DMA Descriptor Fetch Indicator */
#define DMA_RUN			0x0008	/* DMA Channel Running Indicator */

#define DMA_DONE_P		0	/* DMA Done Indicator */
#define DMA_ERR_P		1	/* DMA Error Indicator */
#define DFETCH_P		2	/* Descriptor Fetch Indicator */
#define DMA_RUN_P		3	/* DMA Running Indicator */

/* DMAx_PERIPHERAL_MAP, MDMA_yy_PERIPHERAL_MAP Masks */
#define CTYPE			0x0040	/* DMA Channel Type Indicator (Memory/Peripheral*) */
#define CTYPE_P			6	/* DMA Channel Type Indicator BIT POSITION */
#define PMAP			0xF000	/* Peripheral Mapped To This Channel */

#endif
