
#ifndef _XILINX_QSPIPSS_H_
#define _XILINX_QSPIPSS_H_

#ifndef bool
typedef unsigned int bool;
#endif

struct xqspipss {
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

        struct xqspipss_inst_format *curr_inst;
        u8 inst_response;
        bool is_inst;
};

struct spi_device {
        struct xqspipss master;
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
extern void xqspipss_init_hw(void *regs_base);
extern int  xqspipss_setup_transfer(struct spi_device   *qspi,
                                    struct spi_transfer *transfer);
extern int  xqspipss_transfer(struct spi_device   *qspi,
                              struct spi_transfer *transfer);

/**************************************************************************/

#endif
