
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
	bool is_dual;
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
extern void xqspips_init_hw(void *regs_base, unsigned int is_dual);
extern int  xqspips_setup_transfer(struct spi_device   *qspi,
                                    struct spi_transfer *transfer);
extern int  xqspips_transfer(struct spi_device   *qspi,
                              struct spi_transfer *transfer);
extern int xqspips_check_is_dual_flash(void *regs_base);
extern void xqspips_write_quad_bit(void *regs_base);

/* Few mtd flash functions */
extern int spi_flash_cmd(struct spi_slave *spi, u8 cmd,
		void *response, size_t len);
extern int spi_flash_cmd_read(struct spi_slave *spi, const u8 *cmd,
		size_t cmd_len, void *data, size_t data_len);
/**************************************************************************/

#endif
