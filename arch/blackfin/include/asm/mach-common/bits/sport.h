/*
 * SPORT Masks
 */

#ifndef __BFIN_PERIPHERAL_SPORT__
#define __BFIN_PERIPHERAL_SPORT__

/* SPORTx_TCR1 Masks */
#define TSPEN			0x0001	/* TX enable */
#define ITCLK			0x0002	/* Internal TX Clock Select */
#define TDTYPE			0x000C	/* TX Data Formatting Select */
#define DTYPE_NORM		0x0004	/* Data Format Normal */
#define DTYPE_ULAW		0x0008	/* Compand Using u-Law */
#define DTYPE_ALAW		0x000C	/* Compand Using A-Law */
#define TLSBIT			0x0010	/* TX Bit Order */
#define ITFS			0x0200	/* Internal TX Frame Sync Select */
#define TFSR			0x0400	/* TX Frame Sync Required Select */
#define DITFS			0x0800	/* Data Independent TX Frame Sync Select */
#define LTFS			0x1000	/* Low TX Frame Sync Select */
#define LATFS			0x2000	/* Late TX Frame Sync Select */
#define TCKFE			0x4000	/* TX Clock Falling Edge Select */

/* SPORTx_TCR2 Masks */
#define SLEN			0x001F	/* TX Word Length */
#define TXSE			0x0100	/* TX Secondary Enable */
#define TSFSE			0x0200	/* TX Stereo Frame Sync Enable */
#define TRFST			0x0400	/* TX Right-First Data Order */

/* SPORTx_RCR1 Masks */
#define RSPEN			0x0001	/* RX enable */
#define IRCLK			0x0002	/* Internal RX Clock Select */
#define RDTYPE			0x000C	/* RX Data Formatting Select */
#define DTYPE_NORM		0x0004	/* Data Format Normal */
#define DTYPE_ULAW		0x0008	/* Compand Using u-Law */
#define DTYPE_ALAW		0x000C	/* Compand Using A-Law */
#define RLSBIT			0x0010	/* RX Bit Order */
#define IRFS			0x0200	/* Internal RX Frame Sync Select */
#define RFSR			0x0400	/* RX Frame Sync Required Select */
#define LRFS			0x1000	/* Low RX Frame Sync Select */
#define LARFS			0x2000	/* Late RX Frame Sync Select */
#define RCKFE			0x4000	/* RX Clock Falling Edge Select */

/* SPORTx_RCR2 Masks */
#define SLEN			0x001F	/* RX Word Length */
#define RXSE			0x0100	/* RX Secondary Enable */
#define RSFSE			0x0200	/* RX Stereo Frame Sync Enable */
#define RRFST			0x0400	/* Right-First Data Order */

/* SPORTx_STAT Masks */
#define RXNE			0x0001	/* RX FIFO Not Empty Status */
#define RUVF			0x0002	/* RX Underflow Status */
#define ROVF			0x0004	/* RX Overflow Status */
#define TXF			0x0008	/* TX FIFO Full Status */
#define TUVF			0x0010	/* TX Underflow Status */
#define TOVF			0x0020	/* TX Overflow Status */
#define TXHRE			0x0040	/* TX Hold Register Empty */

/* SPORTx_MCMC1 Masks */
#define WSIZE			0xF000	/* Multichannel Window Size Field */
#define WOFF			0x03FF	/* Multichannel Window Offset Field */

/* SPORTx_MCMC2 Masks */
#define MCCRM			0x0003	/* Multichannel Clock Recovery Mode */
#define REC_BYPASS		0x0000	/* Bypass Mode (No Clock Recovery) */
#define REC_2FROM4		0x0002	/* Recover 2 MHz Clock from 4 MHz Clock */
#define REC_8FROM16		0x0003	/* Recover 8 MHz Clock from 16 MHz Clock */
#define MCDTXPE			0x0004	/* Multichannel DMA Transmit Packing */
#define MCDRXPE			0x0008	/* Multichannel DMA Receive Packing */
#define MCMEN			0x0010	/* Multichannel Frame Mode Enable */
#define FSDR			0x0080	/* Multichannel Frame Sync to Data Relationship */
#define MFD			0xF000	/* Multichannel Frame Delay */
#define MFD_0			0x0000	/* Multichannel Frame Delay = 0 */
#define MFD_1			0x1000	/* Multichannel Frame Delay = 1 */
#define MFD_2			0x2000	/* Multichannel Frame Delay = 2 */
#define MFD_3			0x3000	/* Multichannel Frame Delay = 3 */
#define MFD_4			0x4000	/* Multichannel Frame Delay = 4 */
#define MFD_5			0x5000	/* Multichannel Frame Delay = 5 */
#define MFD_6			0x6000	/* Multichannel Frame Delay = 6 */
#define MFD_7			0x7000	/* Multichannel Frame Delay = 7 */
#define MFD_8			0x8000	/* Multichannel Frame Delay = 8 */
#define MFD_9			0x9000	/* Multichannel Frame Delay = 9 */
#define MFD_10			0xA000	/* Multichannel Frame Delay = 10 */
#define MFD_11			0xB000	/* Multichannel Frame Delay = 11 */
#define MFD_12			0xC000	/* Multichannel Frame Delay = 12 */
#define MFD_13			0xD000	/* Multichannel Frame Delay = 13 */
#define MFD_14			0xE000	/* Multichannel Frame Delay = 14 */
#define MFD_15			0xF000	/* Multichannel Frame Delay = 15 */

#endif
