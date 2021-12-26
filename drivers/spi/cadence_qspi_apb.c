/*
 * Copyright (C) 2012 Altera Corporation <www.altera.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  - Neither the name of the Altera Corporation nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL ALTERA CORPORATION BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <common.h>
#include <log.h>
#include <asm/io.h>
#include <dma.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <wait_bit.h>
#include <spi.h>
#include <spi-mem.h>
#include <malloc.h>
#include "cadence_qspi.h"

#define CQSPI_REG_POLL_US			1 /* 1us */
#define CQSPI_REG_RETRY				10000
#define CQSPI_POLL_IDLE_RETRY			3

/* Transfer mode */
#define CQSPI_INST_TYPE_SINGLE			0
#define CQSPI_INST_TYPE_DUAL			1
#define CQSPI_INST_TYPE_QUAD			2
#define CQSPI_INST_TYPE_OCTAL			3

#define CQSPI_STIG_DATA_LEN_MAX			8

#define CQSPI_DUMMY_CLKS_PER_BYTE		8
#define CQSPI_DUMMY_CLKS_MAX			31

/****************************************************************************
 * Controller's configuration and status register (offset from QSPI_BASE)
 ****************************************************************************/
#define	CQSPI_REG_CONFIG			0x00
#define	CQSPI_REG_CONFIG_ENABLE			BIT(0)
#define	CQSPI_REG_CONFIG_CLK_POL		BIT(1)
#define	CQSPI_REG_CONFIG_CLK_PHA		BIT(2)
#define	CQSPI_REG_CONFIG_DIRECT			BIT(7)
#define	CQSPI_REG_CONFIG_DECODE			BIT(9)
#define	CQSPI_REG_CONFIG_XIP_IMM		BIT(18)
#define	CQSPI_REG_CONFIG_CHIPSELECT_LSB		10
#define	CQSPI_REG_CONFIG_BAUD_LSB		19
#define CQSPI_REG_CONFIG_DTR_PROTO		BIT(24)
#define CQSPI_REG_CONFIG_DUAL_OPCODE		BIT(30)
#define	CQSPI_REG_CONFIG_IDLE_LSB		31
#define	CQSPI_REG_CONFIG_CHIPSELECT_MASK	0xF
#define	CQSPI_REG_CONFIG_BAUD_MASK		0xF

#define	CQSPI_REG_RD_INSTR			0x04
#define	CQSPI_REG_RD_INSTR_OPCODE_LSB		0
#define	CQSPI_REG_RD_INSTR_TYPE_INSTR_LSB	8
#define	CQSPI_REG_RD_INSTR_TYPE_ADDR_LSB	12
#define	CQSPI_REG_RD_INSTR_TYPE_DATA_LSB	16
#define	CQSPI_REG_RD_INSTR_MODE_EN_LSB		20
#define	CQSPI_REG_RD_INSTR_DUMMY_LSB		24
#define	CQSPI_REG_RD_INSTR_TYPE_INSTR_MASK	0x3
#define	CQSPI_REG_RD_INSTR_TYPE_ADDR_MASK	0x3
#define	CQSPI_REG_RD_INSTR_TYPE_DATA_MASK	0x3
#define	CQSPI_REG_RD_INSTR_DUMMY_MASK		0x1F

#define	CQSPI_REG_WR_INSTR			0x08
#define	CQSPI_REG_WR_INSTR_OPCODE_LSB		0
#define CQSPI_REG_WR_INSTR_TYPE_ADDR_LSB	12
#define	CQSPI_REG_WR_INSTR_TYPE_DATA_LSB	16

#define	CQSPI_REG_DELAY				0x0C
#define	CQSPI_REG_DELAY_TSLCH_LSB		0
#define	CQSPI_REG_DELAY_TCHSH_LSB		8
#define	CQSPI_REG_DELAY_TSD2D_LSB		16
#define	CQSPI_REG_DELAY_TSHSL_LSB		24
#define	CQSPI_REG_DELAY_TSLCH_MASK		0xFF
#define	CQSPI_REG_DELAY_TCHSH_MASK		0xFF
#define	CQSPI_REG_DELAY_TSD2D_MASK		0xFF
#define	CQSPI_REG_DELAY_TSHSL_MASK		0xFF

#define	CQSPI_REG_RD_DATA_CAPTURE		0x10
#define	CQSPI_REG_RD_DATA_CAPTURE_BYPASS	BIT(0)
#define	CQSPI_REG_RD_DATA_CAPTURE_DELAY_LSB	1
#define	CQSPI_REG_RD_DATA_CAPTURE_DELAY_MASK	0xF

#define	CQSPI_REG_SIZE				0x14
#define	CQSPI_REG_SIZE_ADDRESS_LSB		0
#define	CQSPI_REG_SIZE_PAGE_LSB			4
#define	CQSPI_REG_SIZE_BLOCK_LSB		16
#define	CQSPI_REG_SIZE_ADDRESS_MASK		0xF
#define	CQSPI_REG_SIZE_PAGE_MASK		0xFFF
#define	CQSPI_REG_SIZE_BLOCK_MASK		0x3F

#define	CQSPI_REG_SRAMPARTITION			0x18
#define	CQSPI_REG_INDIRECTTRIGGER		0x1C

#define	CQSPI_REG_REMAP				0x24
#define	CQSPI_REG_MODE_BIT			0x28

#define	CQSPI_REG_SDRAMLEVEL			0x2C
#define	CQSPI_REG_SDRAMLEVEL_RD_LSB		0
#define	CQSPI_REG_SDRAMLEVEL_WR_LSB		16
#define	CQSPI_REG_SDRAMLEVEL_RD_MASK		0xFFFF
#define	CQSPI_REG_SDRAMLEVEL_WR_MASK		0xFFFF

#define CQSPI_REG_WR_COMPLETION_CTRL		0x38
#define CQSPI_REG_WR_DISABLE_AUTO_POLL		BIT(14)

#define	CQSPI_REG_IRQSTATUS			0x40
#define	CQSPI_REG_IRQMASK			0x44

#define	CQSPI_REG_INDIRECTRD			0x60
#define	CQSPI_REG_INDIRECTRD_START		BIT(0)
#define	CQSPI_REG_INDIRECTRD_CANCEL		BIT(1)
#define	CQSPI_REG_INDIRECTRD_INPROGRESS		BIT(2)
#define	CQSPI_REG_INDIRECTRD_DONE		BIT(5)

#define	CQSPI_REG_INDIRECTRDWATERMARK		0x64
#define	CQSPI_REG_INDIRECTRDSTARTADDR		0x68
#define	CQSPI_REG_INDIRECTRDBYTES		0x6C

#define	CQSPI_REG_CMDCTRL			0x90
#define	CQSPI_REG_CMDCTRL_EXECUTE		BIT(0)
#define	CQSPI_REG_CMDCTRL_INPROGRESS		BIT(1)
#define	CQSPI_REG_CMDCTRL_DUMMY_LSB		7
#define	CQSPI_REG_CMDCTRL_WR_BYTES_LSB		12
#define	CQSPI_REG_CMDCTRL_WR_EN_LSB		15
#define	CQSPI_REG_CMDCTRL_ADD_BYTES_LSB		16
#define	CQSPI_REG_CMDCTRL_ADDR_EN_LSB		19
#define	CQSPI_REG_CMDCTRL_RD_BYTES_LSB		20
#define	CQSPI_REG_CMDCTRL_RD_EN_LSB		23
#define	CQSPI_REG_CMDCTRL_OPCODE_LSB		24
#define	CQSPI_REG_CMDCTRL_DUMMY_MASK		0x1F
#define	CQSPI_REG_CMDCTRL_WR_BYTES_MASK		0x7
#define	CQSPI_REG_CMDCTRL_ADD_BYTES_MASK	0x3
#define	CQSPI_REG_CMDCTRL_RD_BYTES_MASK		0x7
#define	CQSPI_REG_CMDCTRL_OPCODE_MASK		0xFF

#define	CQSPI_REG_INDIRECTWR			0x70
#define	CQSPI_REG_INDIRECTWR_START		BIT(0)
#define	CQSPI_REG_INDIRECTWR_CANCEL		BIT(1)
#define	CQSPI_REG_INDIRECTWR_INPROGRESS		BIT(2)
#define	CQSPI_REG_INDIRECTWR_DONE		BIT(5)

#define	CQSPI_REG_INDIRECTWRWATERMARK		0x74
#define	CQSPI_REG_INDIRECTWRSTARTADDR		0x78
#define	CQSPI_REG_INDIRECTWRBYTES		0x7C

#define	CQSPI_REG_CMDADDRESS			0x94
#define	CQSPI_REG_CMDREADDATALOWER		0xA0
#define	CQSPI_REG_CMDREADDATAUPPER		0xA4
#define	CQSPI_REG_CMDWRITEDATALOWER		0xA8
#define	CQSPI_REG_CMDWRITEDATAUPPER		0xAC

#define CQSPI_REG_OP_EXT_LOWER			0xE0
#define CQSPI_REG_OP_EXT_READ_LSB		24
#define CQSPI_REG_OP_EXT_WRITE_LSB		16
#define CQSPI_REG_OP_EXT_STIG_LSB		0

#define CQSPI_REG_IS_IDLE(base)					\
	((readl(base + CQSPI_REG_CONFIG) >>		\
		CQSPI_REG_CONFIG_IDLE_LSB) & 0x1)

#define CQSPI_GET_RD_SRAM_LEVEL(reg_base)			\
	(((readl(reg_base + CQSPI_REG_SDRAMLEVEL)) >>	\
	CQSPI_REG_SDRAMLEVEL_RD_LSB) & CQSPI_REG_SDRAMLEVEL_RD_MASK)

#define CQSPI_GET_WR_SRAM_LEVEL(reg_base)			\
	(((readl(reg_base + CQSPI_REG_SDRAMLEVEL)) >>	\
	CQSPI_REG_SDRAMLEVEL_WR_LSB) & CQSPI_REG_SDRAMLEVEL_WR_MASK)

void cadence_qspi_apb_controller_enable(void *reg_base)
{
	unsigned int reg;
	reg = readl(reg_base + CQSPI_REG_CONFIG);
	reg |= CQSPI_REG_CONFIG_ENABLE;
	writel(reg, reg_base + CQSPI_REG_CONFIG);
}

void cadence_qspi_apb_controller_disable(void *reg_base)
{
	unsigned int reg;
	reg = readl(reg_base + CQSPI_REG_CONFIG);
	reg &= ~CQSPI_REG_CONFIG_ENABLE;
	writel(reg, reg_base + CQSPI_REG_CONFIG);
}

void cadence_qspi_apb_dac_mode_enable(void *reg_base)
{
	unsigned int reg;

	reg = readl(reg_base + CQSPI_REG_CONFIG);
	reg |= CQSPI_REG_CONFIG_DIRECT;
	writel(reg, reg_base + CQSPI_REG_CONFIG);
}

static unsigned int cadence_qspi_calc_dummy(const struct spi_mem_op *op,
					    bool dtr)
{
	unsigned int dummy_clk;

	if (!op->dummy.nbytes || !op->dummy.buswidth)
		return 0;

	dummy_clk = op->dummy.nbytes * (8 / op->dummy.buswidth);
	if (dtr)
		dummy_clk /= 2;

	return dummy_clk;
}

static u32 cadence_qspi_calc_rdreg(struct cadence_spi_plat *plat)
{
	u32 rdreg = 0;

	rdreg |= plat->inst_width << CQSPI_REG_RD_INSTR_TYPE_INSTR_LSB;
	rdreg |= plat->addr_width << CQSPI_REG_RD_INSTR_TYPE_ADDR_LSB;
	rdreg |= plat->data_width << CQSPI_REG_RD_INSTR_TYPE_DATA_LSB;

	return rdreg;
}

static int cadence_qspi_buswidth_to_inst_type(u8 buswidth)
{
	switch (buswidth) {
	case 0:
	case 1:
		return CQSPI_INST_TYPE_SINGLE;

	case 2:
		return CQSPI_INST_TYPE_DUAL;

	case 4:
		return CQSPI_INST_TYPE_QUAD;

	case 8:
		return CQSPI_INST_TYPE_OCTAL;

	default:
		return -ENOTSUPP;
	}
}

static int cadence_qspi_set_protocol(struct cadence_spi_plat *plat,
				     const struct spi_mem_op *op)
{
	int ret;

	plat->dtr = op->data.dtr && op->cmd.dtr && op->addr.dtr;

	ret = cadence_qspi_buswidth_to_inst_type(op->cmd.buswidth);
	if (ret < 0)
		return ret;
	plat->inst_width = ret;

	ret = cadence_qspi_buswidth_to_inst_type(op->addr.buswidth);
	if (ret < 0)
		return ret;
	plat->addr_width = ret;

	ret = cadence_qspi_buswidth_to_inst_type(op->data.buswidth);
	if (ret < 0)
		return ret;
	plat->data_width = ret;

	return 0;
}

/* Return 1 if idle, otherwise return 0 (busy). */
static unsigned int cadence_qspi_wait_idle(void *reg_base)
{
	unsigned int start, count = 0;
	/* timeout in unit of ms */
	unsigned int timeout = 5000;

	start = get_timer(0);
	for ( ; get_timer(start) < timeout ; ) {
		if (CQSPI_REG_IS_IDLE(reg_base))
			count++;
		else
			count = 0;
		/*
		 * Ensure the QSPI controller is in true idle state after
		 * reading back the same idle status consecutively
		 */
		if (count >= CQSPI_POLL_IDLE_RETRY)
			return 1;
	}

	/* Timeout, still in busy mode. */
	printf("QSPI: QSPI is still busy after poll for %d times.\n",
	       CQSPI_REG_RETRY);
	return 0;
}

void cadence_qspi_apb_readdata_capture(void *reg_base,
				unsigned int bypass, unsigned int delay)
{
	unsigned int reg;
	cadence_qspi_apb_controller_disable(reg_base);

	reg = readl(reg_base + CQSPI_REG_RD_DATA_CAPTURE);

	if (bypass)
		reg |= CQSPI_REG_RD_DATA_CAPTURE_BYPASS;
	else
		reg &= ~CQSPI_REG_RD_DATA_CAPTURE_BYPASS;

	reg &= ~(CQSPI_REG_RD_DATA_CAPTURE_DELAY_MASK
		<< CQSPI_REG_RD_DATA_CAPTURE_DELAY_LSB);

	reg |= (delay & CQSPI_REG_RD_DATA_CAPTURE_DELAY_MASK)
		<< CQSPI_REG_RD_DATA_CAPTURE_DELAY_LSB;

	writel(reg, reg_base + CQSPI_REG_RD_DATA_CAPTURE);

	cadence_qspi_apb_controller_enable(reg_base);
}

void cadence_qspi_apb_config_baudrate_div(void *reg_base,
	unsigned int ref_clk_hz, unsigned int sclk_hz)
{
	unsigned int reg;
	unsigned int div;

	cadence_qspi_apb_controller_disable(reg_base);
	reg = readl(reg_base + CQSPI_REG_CONFIG);
	reg &= ~(CQSPI_REG_CONFIG_BAUD_MASK << CQSPI_REG_CONFIG_BAUD_LSB);

	/*
	 * The baud_div field in the config reg is 4 bits, and the ref clock is
	 * divided by 2 * (baud_div + 1). Round up the divider to ensure the
	 * SPI clock rate is less than or equal to the requested clock rate.
	 */
	div = DIV_ROUND_UP(ref_clk_hz, sclk_hz * 2) - 1;

	/* ensure the baud rate doesn't exceed the max value */
	if (div > CQSPI_REG_CONFIG_BAUD_MASK)
		div = CQSPI_REG_CONFIG_BAUD_MASK;

	debug("%s: ref_clk %dHz sclk %dHz Div 0x%x, actual %dHz\n", __func__,
	      ref_clk_hz, sclk_hz, div, ref_clk_hz / (2 * (div + 1)));

	reg |= (div << CQSPI_REG_CONFIG_BAUD_LSB);
	writel(reg, reg_base + CQSPI_REG_CONFIG);

	cadence_qspi_apb_controller_enable(reg_base);
}

void cadence_qspi_apb_set_clk_mode(void *reg_base, uint mode)
{
	unsigned int reg;

	cadence_qspi_apb_controller_disable(reg_base);
	reg = readl(reg_base + CQSPI_REG_CONFIG);
	reg &= ~(CQSPI_REG_CONFIG_CLK_POL | CQSPI_REG_CONFIG_CLK_PHA);

	if (mode & SPI_CPOL)
		reg |= CQSPI_REG_CONFIG_CLK_POL;
	if (mode & SPI_CPHA)
		reg |= CQSPI_REG_CONFIG_CLK_PHA;

	writel(reg, reg_base + CQSPI_REG_CONFIG);

	cadence_qspi_apb_controller_enable(reg_base);
}

void cadence_qspi_apb_chipselect(void *reg_base,
	unsigned int chip_select, unsigned int decoder_enable)
{
	unsigned int reg;

	cadence_qspi_apb_controller_disable(reg_base);

	debug("%s : chipselect %d decode %d\n", __func__, chip_select,
	      decoder_enable);

	reg = readl(reg_base + CQSPI_REG_CONFIG);
	/* docoder */
	if (decoder_enable) {
		reg |= CQSPI_REG_CONFIG_DECODE;
	} else {
		reg &= ~CQSPI_REG_CONFIG_DECODE;
		/* Convert CS if without decoder.
		 * CS0 to 4b'1110
		 * CS1 to 4b'1101
		 * CS2 to 4b'1011
		 * CS3 to 4b'0111
		 */
		chip_select = 0xF & ~(1 << chip_select);
	}

	reg &= ~(CQSPI_REG_CONFIG_CHIPSELECT_MASK
			<< CQSPI_REG_CONFIG_CHIPSELECT_LSB);
	reg |= (chip_select & CQSPI_REG_CONFIG_CHIPSELECT_MASK)
			<< CQSPI_REG_CONFIG_CHIPSELECT_LSB;
	writel(reg, reg_base + CQSPI_REG_CONFIG);

	cadence_qspi_apb_controller_enable(reg_base);
}

void cadence_qspi_apb_delay(void *reg_base,
	unsigned int ref_clk, unsigned int sclk_hz,
	unsigned int tshsl_ns, unsigned int tsd2d_ns,
	unsigned int tchsh_ns, unsigned int tslch_ns)
{
	unsigned int ref_clk_ns;
	unsigned int sclk_ns;
	unsigned int tshsl, tchsh, tslch, tsd2d;
	unsigned int reg;

	cadence_qspi_apb_controller_disable(reg_base);

	/* Convert to ns. */
	ref_clk_ns = DIV_ROUND_UP(1000000000, ref_clk);

	/* Convert to ns. */
	sclk_ns = DIV_ROUND_UP(1000000000, sclk_hz);

	/* The controller adds additional delay to that programmed in the reg */
	if (tshsl_ns >= sclk_ns + ref_clk_ns)
		tshsl_ns -= sclk_ns + ref_clk_ns;
	if (tchsh_ns >= sclk_ns + 3 * ref_clk_ns)
		tchsh_ns -= sclk_ns + 3 * ref_clk_ns;
	tshsl = DIV_ROUND_UP(tshsl_ns, ref_clk_ns);
	tchsh = DIV_ROUND_UP(tchsh_ns, ref_clk_ns);
	tslch = DIV_ROUND_UP(tslch_ns, ref_clk_ns);
	tsd2d = DIV_ROUND_UP(tsd2d_ns, ref_clk_ns);

	reg = ((tshsl & CQSPI_REG_DELAY_TSHSL_MASK)
			<< CQSPI_REG_DELAY_TSHSL_LSB);
	reg |= ((tchsh & CQSPI_REG_DELAY_TCHSH_MASK)
			<< CQSPI_REG_DELAY_TCHSH_LSB);
	reg |= ((tslch & CQSPI_REG_DELAY_TSLCH_MASK)
			<< CQSPI_REG_DELAY_TSLCH_LSB);
	reg |= ((tsd2d & CQSPI_REG_DELAY_TSD2D_MASK)
			<< CQSPI_REG_DELAY_TSD2D_LSB);
	writel(reg, reg_base + CQSPI_REG_DELAY);

	cadence_qspi_apb_controller_enable(reg_base);
}

void cadence_qspi_apb_controller_init(struct cadence_spi_plat *plat)
{
	unsigned reg;

	cadence_qspi_apb_controller_disable(plat->regbase);

	/* Configure the device size and address bytes */
	reg = readl(plat->regbase + CQSPI_REG_SIZE);
	/* Clear the previous value */
	reg &= ~(CQSPI_REG_SIZE_PAGE_MASK << CQSPI_REG_SIZE_PAGE_LSB);
	reg &= ~(CQSPI_REG_SIZE_BLOCK_MASK << CQSPI_REG_SIZE_BLOCK_LSB);
	reg |= (plat->page_size << CQSPI_REG_SIZE_PAGE_LSB);
	reg |= (plat->block_size << CQSPI_REG_SIZE_BLOCK_LSB);
	writel(reg, plat->regbase + CQSPI_REG_SIZE);

	/* Configure the remap address register, no remap */
	writel(0, plat->regbase + CQSPI_REG_REMAP);

	/* Indirect mode configurations */
	writel(plat->fifo_depth / 2, plat->regbase + CQSPI_REG_SRAMPARTITION);

	/* Disable all interrupts */
	writel(0, plat->regbase + CQSPI_REG_IRQMASK);

	cadence_qspi_apb_controller_enable(plat->regbase);
}

static int cadence_qspi_apb_exec_flash_cmd(void *reg_base,
	unsigned int reg)
{
	unsigned int retry = CQSPI_REG_RETRY;

	/* Write the CMDCTRL without start execution. */
	writel(reg, reg_base + CQSPI_REG_CMDCTRL);
	/* Start execute */
	reg |= CQSPI_REG_CMDCTRL_EXECUTE;
	writel(reg, reg_base + CQSPI_REG_CMDCTRL);

	while (retry--) {
		reg = readl(reg_base + CQSPI_REG_CMDCTRL);
		if ((reg & CQSPI_REG_CMDCTRL_INPROGRESS) == 0)
			break;
		udelay(1);
	}

	if (!retry) {
		printf("QSPI: flash command execution timeout\n");
		return -EIO;
	}

	/* Polling QSPI idle status. */
	if (!cadence_qspi_wait_idle(reg_base))
		return -EIO;

	return 0;
}

static int cadence_qspi_setup_opcode_ext(struct cadence_spi_plat *plat,
					 const struct spi_mem_op *op,
					 unsigned int shift)
{
	unsigned int reg;
	u8 ext;

	if (op->cmd.nbytes != 2)
		return -EINVAL;

	/* Opcode extension is the LSB. */
	ext = op->cmd.opcode & 0xff;

	reg = readl(plat->regbase + CQSPI_REG_OP_EXT_LOWER);
	reg &= ~(0xff << shift);
	reg |= ext << shift;
	writel(reg, plat->regbase + CQSPI_REG_OP_EXT_LOWER);

	return 0;
}

static int cadence_qspi_enable_dtr(struct cadence_spi_plat *plat,
				   const struct spi_mem_op *op,
				   unsigned int shift,
				   bool enable)
{
	unsigned int reg;
	int ret;

	reg = readl(plat->regbase + CQSPI_REG_CONFIG);

	if (enable) {
		reg |= CQSPI_REG_CONFIG_DTR_PROTO;
		reg |= CQSPI_REG_CONFIG_DUAL_OPCODE;

		/* Set up command opcode extension. */
		ret = cadence_qspi_setup_opcode_ext(plat, op, shift);
		if (ret)
			return ret;
	} else {
		reg &= ~CQSPI_REG_CONFIG_DTR_PROTO;
		reg &= ~CQSPI_REG_CONFIG_DUAL_OPCODE;
	}

	writel(reg, plat->regbase + CQSPI_REG_CONFIG);

	return 0;
}

int cadence_qspi_apb_command_read_setup(struct cadence_spi_plat *plat,
					const struct spi_mem_op *op)
{
	int ret;
	unsigned int reg;

	ret = cadence_qspi_set_protocol(plat, op);
	if (ret)
		return ret;

	ret = cadence_qspi_enable_dtr(plat, op, CQSPI_REG_OP_EXT_STIG_LSB,
				      plat->dtr);
	if (ret)
		return ret;

	reg = cadence_qspi_calc_rdreg(plat);
	writel(reg, plat->regbase + CQSPI_REG_RD_INSTR);

	return 0;
}

/* For command RDID, RDSR. */
int cadence_qspi_apb_command_read(struct cadence_spi_plat *plat,
				  const struct spi_mem_op *op)
{
	void *reg_base = plat->regbase;
	unsigned int reg;
	unsigned int read_len;
	int status;
	unsigned int rxlen = op->data.nbytes;
	void *rxbuf = op->data.buf.in;
	unsigned int dummy_clk;
	u8 opcode;

	if (rxlen > CQSPI_STIG_DATA_LEN_MAX || !rxbuf) {
		printf("QSPI: Invalid input arguments rxlen %u\n", rxlen);
		return -EINVAL;
	}

	if (plat->dtr)
		opcode = op->cmd.opcode >> 8;
	else
		opcode = op->cmd.opcode;

	reg = opcode << CQSPI_REG_CMDCTRL_OPCODE_LSB;

	/* Set up dummy cycles. */
	dummy_clk = cadence_qspi_calc_dummy(op, plat->dtr);
	if (dummy_clk > CQSPI_DUMMY_CLKS_MAX)
		return -ENOTSUPP;

	if (dummy_clk)
		reg |= (dummy_clk & CQSPI_REG_CMDCTRL_DUMMY_MASK)
		     << CQSPI_REG_CMDCTRL_DUMMY_LSB;

	reg |= (0x1 << CQSPI_REG_CMDCTRL_RD_EN_LSB);

	/* 0 means 1 byte. */
	reg |= (((rxlen - 1) & CQSPI_REG_CMDCTRL_RD_BYTES_MASK)
		<< CQSPI_REG_CMDCTRL_RD_BYTES_LSB);
	status = cadence_qspi_apb_exec_flash_cmd(reg_base, reg);
	if (status != 0)
		return status;

	reg = readl(reg_base + CQSPI_REG_CMDREADDATALOWER);

	/* Put the read value into rx_buf */
	read_len = (rxlen > 4) ? 4 : rxlen;
	memcpy(rxbuf, &reg, read_len);
	rxbuf += read_len;

	if (rxlen > 4) {
		reg = readl(reg_base + CQSPI_REG_CMDREADDATAUPPER);

		read_len = rxlen - read_len;
		memcpy(rxbuf, &reg, read_len);
	}
	return 0;
}

int cadence_qspi_apb_command_write_setup(struct cadence_spi_plat *plat,
					 const struct spi_mem_op *op)
{
	int ret;
	unsigned int reg;

	ret = cadence_qspi_set_protocol(plat, op);
	if (ret)
		return ret;

	ret = cadence_qspi_enable_dtr(plat, op, CQSPI_REG_OP_EXT_STIG_LSB,
				      plat->dtr);
	if (ret)
		return ret;

	reg = cadence_qspi_calc_rdreg(plat);
	writel(reg, plat->regbase + CQSPI_REG_RD_INSTR);

	return 0;
}

/* For commands: WRSR, WREN, WRDI, CHIP_ERASE, BE, etc. */
int cadence_qspi_apb_command_write(struct cadence_spi_plat *plat,
				   const struct spi_mem_op *op)
{
	unsigned int reg = 0;
	unsigned int wr_data;
	unsigned int wr_len;
	unsigned int txlen = op->data.nbytes;
	const void *txbuf = op->data.buf.out;
	void *reg_base = plat->regbase;
	u32 addr;
	u8 opcode;

	/* Reorder address to SPI bus order if only transferring address */
	if (!txlen) {
		addr = cpu_to_be32(op->addr.val);
		if (op->addr.nbytes == 3)
			addr >>= 8;
		txbuf = &addr;
		txlen = op->addr.nbytes;
	}

	if (txlen > CQSPI_STIG_DATA_LEN_MAX) {
		printf("QSPI: Invalid input arguments txlen %u\n", txlen);
		return -EINVAL;
	}

	if (plat->dtr)
		opcode = op->cmd.opcode >> 8;
	else
		opcode = op->cmd.opcode;

	reg |= opcode << CQSPI_REG_CMDCTRL_OPCODE_LSB;

	if (txlen) {
		/* writing data = yes */
		reg |= (0x1 << CQSPI_REG_CMDCTRL_WR_EN_LSB);
		reg |= ((txlen - 1) & CQSPI_REG_CMDCTRL_WR_BYTES_MASK)
			<< CQSPI_REG_CMDCTRL_WR_BYTES_LSB;

		wr_len = txlen > 4 ? 4 : txlen;
		memcpy(&wr_data, txbuf, wr_len);
		writel(wr_data, reg_base +
			CQSPI_REG_CMDWRITEDATALOWER);

		if (txlen > 4) {
			txbuf += wr_len;
			wr_len = txlen - wr_len;
			memcpy(&wr_data, txbuf, wr_len);
			writel(wr_data, reg_base +
				CQSPI_REG_CMDWRITEDATAUPPER);
		}
	}

	/* Execute the command */
	return cadence_qspi_apb_exec_flash_cmd(reg_base, reg);
}

/* Opcode + Address (3/4 bytes) + dummy bytes (0-4 bytes) */
int cadence_qspi_apb_read_setup(struct cadence_spi_plat *plat,
				const struct spi_mem_op *op)
{
	unsigned int reg;
	unsigned int rd_reg;
	unsigned int dummy_clk;
	unsigned int dummy_bytes = op->dummy.nbytes;
	int ret;
	u8 opcode;

	ret = cadence_qspi_set_protocol(plat, op);
	if (ret)
		return ret;

	ret = cadence_qspi_enable_dtr(plat, op, CQSPI_REG_OP_EXT_READ_LSB,
				      plat->dtr);
	if (ret)
		return ret;

	/* Setup the indirect trigger address */
	writel(plat->trigger_address,
	       plat->regbase + CQSPI_REG_INDIRECTTRIGGER);

	/* Configure the opcode */
	if (plat->dtr)
		opcode = op->cmd.opcode >> 8;
	else
		opcode = op->cmd.opcode;

	rd_reg = opcode << CQSPI_REG_RD_INSTR_OPCODE_LSB;
	rd_reg |= cadence_qspi_calc_rdreg(plat);

	writel(op->addr.val, plat->regbase + CQSPI_REG_INDIRECTRDSTARTADDR);

	if (dummy_bytes) {
		/* Convert to clock cycles. */
		dummy_clk = cadence_qspi_calc_dummy(op, plat->dtr);

		if (dummy_clk > CQSPI_DUMMY_CLKS_MAX)
			return -ENOTSUPP;

		if (dummy_clk)
			rd_reg |= (dummy_clk & CQSPI_REG_RD_INSTR_DUMMY_MASK)
				<< CQSPI_REG_RD_INSTR_DUMMY_LSB;
	}

	writel(rd_reg, plat->regbase + CQSPI_REG_RD_INSTR);

	/* set device size */
	reg = readl(plat->regbase + CQSPI_REG_SIZE);
	reg &= ~CQSPI_REG_SIZE_ADDRESS_MASK;
	reg |= (op->addr.nbytes - 1);
	writel(reg, plat->regbase + CQSPI_REG_SIZE);
	return 0;
}

static u32 cadence_qspi_get_rd_sram_level(struct cadence_spi_plat *plat)
{
	u32 reg = readl(plat->regbase + CQSPI_REG_SDRAMLEVEL);
	reg >>= CQSPI_REG_SDRAMLEVEL_RD_LSB;
	return reg & CQSPI_REG_SDRAMLEVEL_RD_MASK;
}

static int cadence_qspi_wait_for_data(struct cadence_spi_plat *plat)
{
	unsigned int timeout = 10000;
	u32 reg;

	while (timeout--) {
		reg = cadence_qspi_get_rd_sram_level(plat);
		if (reg)
			return reg;
		udelay(1);
	}

	return -ETIMEDOUT;
}

static int
cadence_qspi_apb_indirect_read_execute(struct cadence_spi_plat *plat,
				       unsigned int n_rx, u8 *rxbuf)
{
	unsigned int remaining = n_rx;
	unsigned int bytes_to_read = 0;
	int ret;

	writel(n_rx, plat->regbase + CQSPI_REG_INDIRECTRDBYTES);

	/* Start the indirect read transfer */
	writel(CQSPI_REG_INDIRECTRD_START,
	       plat->regbase + CQSPI_REG_INDIRECTRD);

	while (remaining > 0) {
		ret = cadence_qspi_wait_for_data(plat);
		if (ret < 0) {
			printf("Indirect write timed out (%i)\n", ret);
			goto failrd;
		}

		bytes_to_read = ret;

		while (bytes_to_read != 0) {
			bytes_to_read *= plat->fifo_width;
			bytes_to_read = bytes_to_read > remaining ?
					remaining : bytes_to_read;
			/*
			 * Handle non-4-byte aligned access to avoid
			 * data abort.
			 */
			if (((uintptr_t)rxbuf % 4) || (bytes_to_read % 4))
				readsb(plat->ahbbase, rxbuf, bytes_to_read);
			else
				readsl(plat->ahbbase, rxbuf,
				       bytes_to_read >> 2);
			rxbuf += bytes_to_read;
			remaining -= bytes_to_read;
			bytes_to_read = cadence_qspi_get_rd_sram_level(plat);
		}
	}

	/* Check indirect done status */
	ret = wait_for_bit_le32(plat->regbase + CQSPI_REG_INDIRECTRD,
				CQSPI_REG_INDIRECTRD_DONE, 1, 10, 0);
	if (ret) {
		printf("Indirect read completion error (%i)\n", ret);
		goto failrd;
	}

	/* Clear indirect completion status */
	writel(CQSPI_REG_INDIRECTRD_DONE,
	       plat->regbase + CQSPI_REG_INDIRECTRD);

	/* Check indirect done status */
	ret = wait_for_bit_le32(plat->regbase + CQSPI_REG_INDIRECTRD,
				CQSPI_REG_INDIRECTRD_DONE, 0, 10, 0);
	if (ret) {
		printf("Indirect read clear completion error (%i)\n", ret);
		goto failrd;
	}

	return 0;

failrd:
	/* Cancel the indirect read */
	writel(CQSPI_REG_INDIRECTRD_CANCEL,
	       plat->regbase + CQSPI_REG_INDIRECTRD);
	return ret;
}

int cadence_qspi_apb_read_execute(struct cadence_spi_plat *plat,
				  const struct spi_mem_op *op)
{
	u64 from = op->addr.val;
	void *buf = op->data.buf.in;
	size_t len = op->data.nbytes;

	if (plat->use_dac_mode && (from + len < plat->ahbsize)) {
		if (len < 256 ||
		    dma_memcpy(buf, plat->ahbbase + from, len) < 0) {
			memcpy_fromio(buf, plat->ahbbase + from, len);
		}
		if (!cadence_qspi_wait_idle(plat->regbase))
			return -EIO;
		return 0;
	}

	return cadence_qspi_apb_indirect_read_execute(plat, len, buf);
}

/* Opcode + Address (3/4 bytes) */
int cadence_qspi_apb_write_setup(struct cadence_spi_plat *plat,
				 const struct spi_mem_op *op)
{
	unsigned int reg;
	int ret;
	u8 opcode;

	ret = cadence_qspi_set_protocol(plat, op);
	if (ret)
		return ret;

	ret = cadence_qspi_enable_dtr(plat, op, CQSPI_REG_OP_EXT_WRITE_LSB,
				      plat->dtr);
	if (ret)
		return ret;

	/* Setup the indirect trigger address */
	writel(plat->trigger_address,
	       plat->regbase + CQSPI_REG_INDIRECTTRIGGER);

	/* Configure the opcode */
	if (plat->dtr)
		opcode = op->cmd.opcode >> 8;
	else
		opcode = op->cmd.opcode;

	reg = opcode << CQSPI_REG_WR_INSTR_OPCODE_LSB;
	reg |= plat->data_width << CQSPI_REG_WR_INSTR_TYPE_DATA_LSB;
	reg |= plat->addr_width << CQSPI_REG_WR_INSTR_TYPE_ADDR_LSB;
	writel(reg, plat->regbase + CQSPI_REG_WR_INSTR);

	reg = cadence_qspi_calc_rdreg(plat);
	writel(reg, plat->regbase + CQSPI_REG_RD_INSTR);

	writel(op->addr.val, plat->regbase + CQSPI_REG_INDIRECTWRSTARTADDR);

	if (plat->dtr) {
		/*
		 * Some flashes like the cypress Semper flash expect a 4-byte
		 * dummy address with the Read SR command in DTR mode, but this
		 * controller does not support sending address with the Read SR
		 * command. So, disable write completion polling on the
		 * controller's side. spi-nor will take care of polling the
		 * status register.
		 */
		reg = readl(plat->regbase + CQSPI_REG_WR_COMPLETION_CTRL);
		reg |= CQSPI_REG_WR_DISABLE_AUTO_POLL;
		writel(reg, plat->regbase + CQSPI_REG_WR_COMPLETION_CTRL);
	}

	reg = readl(plat->regbase + CQSPI_REG_SIZE);
	reg &= ~CQSPI_REG_SIZE_ADDRESS_MASK;
	reg |= (op->addr.nbytes - 1);
	writel(reg, plat->regbase + CQSPI_REG_SIZE);
	return 0;
}

static int
cadence_qspi_apb_indirect_write_execute(struct cadence_spi_plat *plat,
					unsigned int n_tx, const u8 *txbuf)
{
	unsigned int page_size = plat->page_size;
	unsigned int remaining = n_tx;
	const u8 *bb_txbuf = txbuf;
	void *bounce_buf = NULL;
	unsigned int write_bytes;
	int ret;

	/*
	 * Use bounce buffer for non 32 bit aligned txbuf to avoid data
	 * aborts
	 */
	if ((uintptr_t)txbuf % 4) {
		bounce_buf = malloc(n_tx);
		if (!bounce_buf)
			return -ENOMEM;
		memcpy(bounce_buf, txbuf, n_tx);
		bb_txbuf = bounce_buf;
	}

	/* Configure the indirect read transfer bytes */
	writel(n_tx, plat->regbase + CQSPI_REG_INDIRECTWRBYTES);

	/* Start the indirect write transfer */
	writel(CQSPI_REG_INDIRECTWR_START,
	       plat->regbase + CQSPI_REG_INDIRECTWR);

	/*
	 * Some delay is required for the above bit to be internally
	 * synchronized by the QSPI module.
	 */
	ndelay(plat->wr_delay);

	while (remaining > 0) {
		write_bytes = remaining > page_size ? page_size : remaining;
		writesl(plat->ahbbase, bb_txbuf, write_bytes >> 2);
		if (write_bytes % 4)
			writesb(plat->ahbbase,
				bb_txbuf + rounddown(write_bytes, 4),
				write_bytes % 4);

		ret = wait_for_bit_le32(plat->regbase + CQSPI_REG_SDRAMLEVEL,
					CQSPI_REG_SDRAMLEVEL_WR_MASK <<
					CQSPI_REG_SDRAMLEVEL_WR_LSB, 0, 10, 0);
		if (ret) {
			printf("Indirect write timed out (%i)\n", ret);
			goto failwr;
		}

		bb_txbuf += write_bytes;
		remaining -= write_bytes;
	}

	/* Check indirect done status */
	ret = wait_for_bit_le32(plat->regbase + CQSPI_REG_INDIRECTWR,
				CQSPI_REG_INDIRECTWR_DONE, 1, 10, 0);
	if (ret) {
		printf("Indirect write completion error (%i)\n", ret);
		goto failwr;
	}

	/* Clear indirect completion status */
	writel(CQSPI_REG_INDIRECTWR_DONE,
	       plat->regbase + CQSPI_REG_INDIRECTWR);

	/* Check indirect done status */
	ret = wait_for_bit_le32(plat->regbase + CQSPI_REG_INDIRECTWR,
				CQSPI_REG_INDIRECTWR_DONE, 0, 10, 0);
	if (ret) {
		printf("Indirect write clear completion error (%i)\n", ret);
		goto failwr;
	}

	if (bounce_buf)
		free(bounce_buf);
	return 0;

failwr:
	/* Cancel the indirect write */
	writel(CQSPI_REG_INDIRECTWR_CANCEL,
	       plat->regbase + CQSPI_REG_INDIRECTWR);
	if (bounce_buf)
		free(bounce_buf);
	return ret;
}

int cadence_qspi_apb_write_execute(struct cadence_spi_plat *plat,
				   const struct spi_mem_op *op)
{
	u32 to = op->addr.val;
	const void *buf = op->data.buf.out;
	size_t len = op->data.nbytes;

	/*
	 * Some flashes like the Cypress Semper flash expect a dummy 4-byte
	 * address (all 0s) with the read status register command in DTR mode.
	 * But this controller does not support sending dummy address bytes to
	 * the flash when it is polling the write completion register in DTR
	 * mode. So, we can not use direct mode when in DTR mode for writing
	 * data.
	 */
	if (!plat->dtr && plat->use_dac_mode && (to + len < plat->ahbsize)) {
		memcpy_toio(plat->ahbbase + to, buf, len);
		if (!cadence_qspi_wait_idle(plat->regbase))
			return -EIO;
		return 0;
	}

	return cadence_qspi_apb_indirect_write_execute(plat, len, buf);
}

void cadence_qspi_apb_enter_xip(void *reg_base, char xip_dummy)
{
	unsigned int reg;

	/* enter XiP mode immediately and enable direct mode */
	reg = readl(reg_base + CQSPI_REG_CONFIG);
	reg |= CQSPI_REG_CONFIG_ENABLE;
	reg |= CQSPI_REG_CONFIG_DIRECT;
	reg |= CQSPI_REG_CONFIG_XIP_IMM;
	writel(reg, reg_base + CQSPI_REG_CONFIG);

	/* keep the XiP mode */
	writel(xip_dummy, reg_base + CQSPI_REG_MODE_BIT);

	/* Enable mode bit at devrd */
	reg = readl(reg_base + CQSPI_REG_RD_INSTR);
	reg |= (1 << CQSPI_REG_RD_INSTR_MODE_EN_LSB);
	writel(reg, reg_base + CQSPI_REG_RD_INSTR);
}
