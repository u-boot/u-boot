// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2018 Xilinx
 *
 * Cadence QSPI controller DMA operations
 */

#include <clk.h>
#include <common.h>
#include <memalign.h>
#include <wait_bit.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/cache.h>
#include <cpu_func.h>
#include <zynqmp_firmware.h>
#include <asm/arch/hardware.h>
#include "cadence_qspi.h"
#include <dt-bindings/power/xlnx-versal-power.h>

#define CMD_4BYTE_READ  0x13
#define CMD_4BYTE_FAST_READ  0x0C

int cadence_qspi_apb_dma_read(struct cadence_spi_plat *plat,
			      const struct spi_mem_op *op)
{
	u32 reg, ret, rx_rem, n_rx, bytes_to_dma, data;
	u8 opcode, addr_bytes, *rxbuf, dummy_cycles;

	n_rx = op->data.nbytes;
	rxbuf = op->data.buf.in;
	rx_rem = n_rx % 4;
	bytes_to_dma = n_rx - rx_rem;

	if (bytes_to_dma) {
		reg = readl(plat->regbase + CQSPI_REG_CONFIG);
		reg |= CQSPI_REG_CONFIG_ENBL_DMA;
		writel(reg, plat->regbase + CQSPI_REG_CONFIG);

		writel(bytes_to_dma, plat->regbase + CQSPI_REG_INDIRECTRDBYTES);

		writel(CQSPI_DFLT_INDIR_TRIG_ADDR_RANGE,
		       plat->regbase + CQSPI_REG_INDIR_TRIG_ADDR_RANGE);
		writel(CQSPI_DFLT_DMA_PERIPH_CFG,
		       plat->regbase + CQSPI_REG_DMA_PERIPH_CFG);
		writel((unsigned long)rxbuf, plat->regbase +
		       CQSPI_DMA_DST_ADDR_REG);
		writel(plat->trigger_address, plat->regbase +
		       CQSPI_DMA_SRC_RD_ADDR_REG);
		writel(bytes_to_dma, plat->regbase +
		       CQSPI_DMA_DST_SIZE_REG);
		flush_dcache_range((unsigned long)rxbuf,
				   (unsigned long)rxbuf + bytes_to_dma);
		writel(CQSPI_DFLT_DST_CTRL_REG_VAL,
		       plat->regbase + CQSPI_DMA_DST_CTRL_REG);

		/* Start the indirect read transfer */
		writel(CQSPI_REG_INDIRECTRD_START, plat->regbase +
		       CQSPI_REG_INDIRECTRD);
		/* Wait for dma to complete transfer */
		ret = cadence_qspi_apb_wait_for_dma_cmplt(plat);
		if (ret)
			return ret;

		/* Clear indirect completion status */
		writel(CQSPI_REG_INDIRECTRD_DONE, plat->regbase +
		       CQSPI_REG_INDIRECTRD);
		rxbuf += bytes_to_dma;
	}

	if (rx_rem) {
		reg = readl(plat->regbase + CQSPI_REG_CONFIG);
		reg &= ~CQSPI_REG_CONFIG_ENBL_DMA;
		writel(reg, plat->regbase + CQSPI_REG_CONFIG);

		reg = readl(plat->regbase + CQSPI_REG_INDIRECTRDSTARTADDR);
		reg += bytes_to_dma;
		writel(reg, plat->regbase + CQSPI_REG_CMDADDRESS);

		addr_bytes = readl(plat->regbase + CQSPI_REG_SIZE) &
				   CQSPI_REG_SIZE_ADDRESS_MASK;

		opcode = CMD_4BYTE_FAST_READ;
		dummy_cycles = 8;
		writel((dummy_cycles << CQSPI_REG_RD_INSTR_DUMMY_LSB) | opcode,
		       plat->regbase + CQSPI_REG_RD_INSTR);

		reg = opcode << CQSPI_REG_CMDCTRL_OPCODE_LSB;
		reg |= (0x1 << CQSPI_REG_CMDCTRL_RD_EN_LSB);
		reg |= (addr_bytes & CQSPI_REG_CMDCTRL_ADD_BYTES_MASK) <<
			CQSPI_REG_CMDCTRL_ADD_BYTES_LSB;
		reg |= (0x1 << CQSPI_REG_CMDCTRL_ADDR_EN_LSB);
		dummy_cycles = (readl(plat->regbase + CQSPI_REG_RD_INSTR) >>
				CQSPI_REG_RD_INSTR_DUMMY_LSB) &
				CQSPI_REG_RD_INSTR_DUMMY_MASK;
		reg |= (dummy_cycles & CQSPI_REG_CMDCTRL_DUMMY_MASK) <<
			CQSPI_REG_CMDCTRL_DUMMY_LSB;
		reg |= (((rx_rem - 1) & CQSPI_REG_CMDCTRL_RD_BYTES_MASK) <<
			CQSPI_REG_CMDCTRL_RD_BYTES_LSB);
		ret = cadence_qspi_apb_exec_flash_cmd(plat->regbase, reg);
		if (ret)
			return ret;

		data = readl(plat->regbase + CQSPI_REG_CMDREADDATALOWER);
		memcpy(rxbuf, &data, rx_rem);
	}

	return 0;
}

int cadence_qspi_apb_wait_for_dma_cmplt(struct cadence_spi_plat *plat)
{
	u32 timeout = CQSPI_DMA_TIMEOUT;

	while (!(readl(plat->regbase + CQSPI_DMA_DST_I_STS_REG) &
		 CQSPI_DMA_DST_I_STS_DONE) && timeout--)
		udelay(1);

	if (!timeout) {
		printf("DMA timeout\n");
		return -ETIMEDOUT;
	}

	writel(readl(plat->regbase + CQSPI_DMA_DST_I_STS_REG),
	       plat->regbase + CQSPI_DMA_DST_I_STS_REG);
	return 0;
}
