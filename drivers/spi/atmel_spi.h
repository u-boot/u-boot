/*
 * Register definitions for the Atmel AT32/AT91 SPI Controller
 */

/* Register offsets */
#define ATMEL_SPI_CR			0x0000
#define ATMEL_SPI_MR			0x0004
#define ATMEL_SPI_RDR			0x0008
#define ATMEL_SPI_TDR			0x000c
#define ATMEL_SPI_SR			0x0010
#define ATMEL_SPI_IER			0x0014
#define ATMEL_SPI_IDR			0x0018
#define ATMEL_SPI_IMR			0x001c
#define ATMEL_SPI_CSR(x)		(0x0030 + 4 * (x))
#define ATMEL_SPI_VERSION		0x00fc

/* Bits in CR */
#define ATMEL_SPI_CR_SPIEN		(1 << 0)
#define ATMEL_SPI_CR_SPIDIS		(1 << 1)
#define ATMEL_SPI_CR_SWRST		(1 << 7)
#define ATMEL_SPI_CR_LASTXFER		(1 << 24)

/* Bits in MR */
#define ATMEL_SPI_MR_MSTR		(1 << 0)
#define ATMEL_SPI_MR_PS			(1 << 1)
#define ATMEL_SPI_MR_PCSDEC		(1 << 2)
#define ATMEL_SPI_MR_FDIV		(1 << 3)
#define ATMEL_SPI_MR_MODFDIS		(1 << 4)
#define ATMEL_SPI_MR_WDRBT		(1 << 5)
#define ATMEL_SPI_MR_LLB		(1 << 7)
#define ATMEL_SPI_MR_PCS(x)		(((x) & 15) << 16)
#define ATMEL_SPI_MR_DLYBCS(x)		((x) << 24)

/* Bits in RDR */
#define ATMEL_SPI_RDR_RD(x)		(x)
#define ATMEL_SPI_RDR_PCS(x)		((x) << 16)

/* Bits in TDR */
#define ATMEL_SPI_TDR_TD(x)		(x)
#define ATMEL_SPI_TDR_PCS(x)		((x) << 16)
#define ATMEL_SPI_TDR_LASTXFER		(1 << 24)

/* Bits in SR/IER/IDR/IMR */
#define ATMEL_SPI_SR_RDRF		(1 << 0)
#define ATMEL_SPI_SR_TDRE		(1 << 1)
#define ATMEL_SPI_SR_MODF		(1 << 2)
#define ATMEL_SPI_SR_OVRES		(1 << 3)
#define ATMEL_SPI_SR_ENDRX		(1 << 4)
#define ATMEL_SPI_SR_ENDTX		(1 << 5)
#define ATMEL_SPI_SR_RXBUFF		(1 << 6)
#define ATMEL_SPI_SR_TXBUFE		(1 << 7)
#define ATMEL_SPI_SR_NSSR		(1 << 8)
#define ATMEL_SPI_SR_TXEMPTY		(1 << 9)
#define ATMEL_SPI_SR_SPIENS		(1 << 16)

/* Bits in CSRx */
#define ATMEL_SPI_CSRx_CPOL		(1 << 0)
#define ATMEL_SPI_CSRx_NCPHA		(1 << 1)
#define ATMEL_SPI_CSRx_CSAAT		(1 << 3)
#define ATMEL_SPI_CSRx_BITS(x)		((x) << 4)
#define ATMEL_SPI_CSRx_SCBR(x)		((x) << 8)
#define ATMEL_SPI_CSRx_SCBR_MAX		0xff
#define ATMEL_SPI_CSRx_DLYBS(x)		((x) << 16)
#define ATMEL_SPI_CSRx_DLYBCT(x)	((x) << 24)

/* Bits in VERSION */
#define ATMEL_SPI_VERSION_REV(x)	((x) & 0xfff)
#define ATMEL_SPI_VERSION_MFN(x)	((x) << 16)

/* Constants for CSRx:BITS */
#define ATMEL_SPI_BITS_8		0
#define ATMEL_SPI_BITS_9		1
#define ATMEL_SPI_BITS_10		2
#define ATMEL_SPI_BITS_11		3
#define ATMEL_SPI_BITS_12		4
#define ATMEL_SPI_BITS_13		5
#define ATMEL_SPI_BITS_14		6
#define ATMEL_SPI_BITS_15		7
#define ATMEL_SPI_BITS_16		8

struct atmel_spi_slave {
	struct spi_slave slave;
	void		*regs;
	u32		mr;
};

static inline struct atmel_spi_slave *to_atmel_spi(struct spi_slave *slave)
{
	return container_of(slave, struct atmel_spi_slave, slave);
}

/* Register access macros */
#define spi_readl(as, reg)					\
	readl(as->regs + ATMEL_SPI_##reg)
#define spi_writel(as, reg, value)				\
	writel(value, as->regs + ATMEL_SPI_##reg)
