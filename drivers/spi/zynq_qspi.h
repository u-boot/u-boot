
#ifndef _XILINX_QSPIPSS_H_
#define _XILINX_QSPIPSS_H_

#ifndef bool
typedef unsigned int bool;
#endif

struct xqspips {
        u8 queue_state;
        void *regs;
        u32 input_clk_hz;
        u32 irq;
        u32 speed_hz;

        const void *txbuf;
        void *rxbuf;
        int bytes_to_transfer;
        int bytes_to_receive;
        u8 dev_busy;
        int done;

	struct xqspips_inst_format *curr_inst;
        u8 inst_response;
        bool is_inst;
};

struct spi_device {
	struct xqspips master;
        u32             max_speed_hz;
        u8              chip_select;
        u8              mode;
        u8              bits_per_word;
};

struct spi_transfer {
        const void      *tx_buf;
        void            *rx_buf;
        unsigned        len;

        unsigned        cs_change:1;
        u8              bits_per_word;
        u16             delay_usecs;
        u32             speed_hz;
};

/**************************************************************************/
extern void xqspips_init_hw(void *regs_base);
extern int  xqspips_setup_transfer(struct spi_device   *qspi,
                                    struct spi_transfer *transfer);
extern int  xqspips_transfer(struct spi_device   *qspi,
                              struct spi_transfer *transfer);

/**************************************************************************/

#endif
