// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2018 Xilinx
 *
 * Cadence QSPI controller DMA operations
 */

#include <clk.h>
#include <common.h>
#include <cpu_func.h>
#include <memalign.h>
#include <wait_bit.h>
#include <asm/io.h>
#include "cadence_qspi.h"
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <zynqmp_firmware.h>

#define CMD_4BYTE_READ  0x13
#define CMD_4BYTE_FAST_READ  0x0C

int cadence_qspi_apb_dma_read(struct cadence_spi_platdata *plat,
			      unsigned int n_rx, u8 *rxbuf)
{
	u32 reg, ret, rx_rem, bytes_to_dma, data;
	u8 opcode, addr_bytes, dummy_cycles;

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

int cadence_qspi_apb_wait_for_dma_cmplt(struct cadence_spi_platdata *plat)
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

#if defined(CONFIG_DM_GPIO)
int cadence_spi_versal_flash_reset(struct udevice *dev)
{
	struct gpio_desc gpio;
	u32 reset_gpio;
	int ret;

	/* request gpio and set direction as output set to 1 */
	ret = gpio_request_by_name(dev, "reset-gpios", 0, &gpio,
				   GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE);
	if (ret) {
		printf("%s: unable to reset ospi flash device", __func__);
		return ret;
	}

	reset_gpio = PMIO_NODE_ID_BASE + gpio.offset;

	/* Request for pin */
	xilinx_pm_request(PM_PINCTRL_REQUEST, reset_gpio, 0, 0, 0, NULL);

	/* Enable hysteresis in cmos receiver */
	xilinx_pm_request(PM_PINCTRL_CONFIG_PARAM_SET, reset_gpio,
			  PM_PINCTRL_CONFIG_SCHMITT_CMOS,
			  PM_PINCTRL_INPUT_TYPE_SCHMITT, 0, NULL);

	/* Disable Tri-state */
	xilinx_pm_request(PM_PINCTRL_CONFIG_PARAM_SET, reset_gpio,
			  PM_PINCTRL_CONFIG_TRI_STATE,
			  PM_PINCTRL_TRI_STATE_DISABLE, 0, NULL);
	udelay(1);

	/* Set value 0 to pin */
	dm_gpio_set_value(&gpio, 0);
	udelay(1);

	/* Set value 1 to pin */
	dm_gpio_set_value(&gpio, 1);
	udelay(1);

	return 0;
}
#else
#define FLASH_RESET_GPIO	0xC
int cadence_spi_versal_flash_reset(struct udevice *dev)
{
	/* CRP WPROT */
	writel(0, 0xf126001c);
	/* GPIO Reset */
	writel(0, 0xf1260318);

	/* disable IOU write protection */
	writel(0, 0xff080728);

	/* set direction as output */
	writel((readl(0xf1020204) | BIT(FLASH_RESET_GPIO)), 0xf1020204);

	/* Data output enable */
	writel((readl(0xf1020208) | BIT(FLASH_RESET_GPIO)), 0xf1020208);

	/* IOU SLCR write enable */
	writel(0, 0xf1060828);

	/* set MIO as GPIO */
	writel(0x60, 0xf1060030);

	/* Set value 1 to pin */
	writel((readl(0xf1020040) | BIT(FLASH_RESET_GPIO)), 0xf1020040);
	udelay(10);

	/* Disable Tri-state */
	writel((readl(0xf1060200) & ~BIT(FLASH_RESET_GPIO)), 0xf1060200);
	udelay(1);

	/* Set value 0 to pin */
	writel((readl(0xf1020040) & ~BIT(FLASH_RESET_GPIO)), 0xf1020040);
	udelay(10);

	/* Set value 1 to pin */
	writel((readl(0xf1020040) | BIT(FLASH_RESET_GPIO)), 0xf1020040);
	udelay(10);

	return 0;
}
#endif
