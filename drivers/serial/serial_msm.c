// SPDX-License-Identifier: GPL-2.0+
/*
 * Qualcomm UART driver
 *
 * (C) Copyright 2015 Mateusz Kulikowski <mateusz.kulikowski@gmail.com>
 *
 * UART will work in Data Mover mode.
 * Based on Linux driver.
 */

#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <serial.h>
#include <watchdog.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/compiler.h>
#include <dm/pinctrl.h>

/* Serial registers - this driver works in uartdm mode*/

#define UARTDM_DMEN			0x3C /* DMA/data-packing mode */
#define UARTDM_DMEN_TXRX_SC_ENABLE	(BIT(4) | BIT(5))

#define UARTDM_MR1				 0x00
#define UARTDM_MR1_RX_RDY_CTL			 BIT(7)
#define UARTDM_MR2				 0x04
#define UARTDM_MR2_8_N_1_MODE			 0x34
/*
 * This is documented on page 1817 of the apq8016e technical reference manual.
 * section 6.2.5.3.26
 *
 * The upper nybble contains the bit clock divider for the RX pin, the lower
 * nybble defines the TX pin. In almost all cases these should be the same value.
 *
 * The baud rate is the core clock frequency divided by the fixed divider value
 * programmed into this register (defined in calc_csr_bitrate()).
 */
#define UARTDM_CSR				 0xA0

#define UARTDM_SR                0xA4 /* Status register */
#define UARTDM_SR_RX_READY       (1 << 0) /* Receiver FIFO has data */
#define UARTDM_SR_TX_READY       (1 << 2) /* Transmitter FIFO has space */
#define UARTDM_SR_TX_EMPTY       (1 << 3) /* Transmitter underrun */

#define UARTDM_CR                         0xA8 /* Command register */
#define UARTDM_CR_RX_ENABLE               (1 << 0) /* Enable receiver */
#define UARTDM_CR_TX_ENABLE               (1 << 2) /* Enable transmitter */
#define UARTDM_CR_CMD_RESET_RX            (1 << 4) /* Reset receiver */
#define UARTDM_CR_CMD_RESET_TX            (2 << 4) /* Reset transmitter */

#define UARTDM_TF               0x100 /* UART Transmit FIFO register */
#define UARTDM_RF               0x140 /* UART Receive FIFO register */
#define UARTDM_RF_CHAR          0xff /* higher bits contain error information */

DECLARE_GLOBAL_DATA_PTR;

struct msm_serial_data {
	phys_addr_t base;
	uint32_t clk_rate; /* core clock rate */
};

static int msm_serial_getc(struct udevice *dev)
{
	struct msm_serial_data *priv = dev_get_priv(dev);

	if (!(readl(priv->base + UARTDM_SR) & UARTDM_SR_RX_READY))
		return -EAGAIN;

	return readl(priv->base + UARTDM_RF) & UARTDM_RF_CHAR;
}

static int msm_serial_putc(struct udevice *dev, const char ch)
{
	struct msm_serial_data *priv = dev_get_priv(dev);

	if (!(readl(priv->base + UARTDM_SR) & UARTDM_SR_TX_READY))
		return -EAGAIN;

	writel(ch, priv->base + UARTDM_TF);
	return 0;
}

static int msm_serial_pending(struct udevice *dev, bool input)
{
	struct msm_serial_data *priv = dev_get_priv(dev);

	if (input)
		return !!(readl(priv->base + UARTDM_SR) & UARTDM_SR_RX_READY);
	else
		return !(readl(priv->base + UARTDM_SR) & UARTDM_SR_TX_EMPTY);
}

static const struct dm_serial_ops msm_serial_ops = {
	.putc = msm_serial_putc,
	.pending = msm_serial_pending,
	.getc = msm_serial_getc,
};

static long msm_uart_clk_init(struct udevice *dev)
{
	struct msm_serial_data *priv = dev_get_priv(dev);
	struct clk clk;
	int ret;
	long rate;

	ret = clk_get_by_name(dev, "core", &clk);
	if (ret < 0) {
		pr_warn("%s: Failed to get clock: %d\n", __func__, ret);
		return 0;
	}

	rate = clk_set_rate(&clk, priv->clk_rate);

	return rate;
}

static int calc_csr_bitrate(struct msm_serial_data *priv)
{
	/* This table is from the TRE. See the definition of UARTDM_CSR */
	unsigned int csr_div_table[] = {24576, 12288, 6144, 3072, 1536, 768, 512, 384,
					256,   192,   128,  96,   64,   48,  32,  16};
	int i = ARRAY_SIZE(csr_div_table) - 1;
	/* Currently we only support one baudrate */
	int baud = 115200;

	for (; i >= 0; i--) {
		int x = priv->clk_rate / csr_div_table[i];

		if (x == baud)
			/* Duplicate the configuration for RX
			 * as the lower nybble only configures TX
			 */
			return i + (i << 4);
	}

	return -EINVAL;
}

static void uart_dm_init(struct msm_serial_data *priv)
{
	int bitrate = calc_csr_bitrate(priv);
	if (bitrate < 0) {
		log_warning("Couldn't calculate bit clock divider! Using default\n");
		/* This happens to be the value used on MSM8916 for the hardcoded clockrate
		 * in clock-apq8016. It's at least a better guess than a value we *know*
		 * is wrong...
		 */
		bitrate = 0xCC;
	}

	writel(bitrate, priv->base + UARTDM_CSR);
	/* Enable RS232 flow control to support RS232 db9 connector */
	writel(UARTDM_MR1_RX_RDY_CTL, priv->base + UARTDM_MR1);
	writel(UARTDM_MR2_8_N_1_MODE, priv->base + UARTDM_MR2);

	/* Enable single character mode */
	writel(UARTDM_DMEN_TXRX_SC_ENABLE, priv->base + UARTDM_DMEN);

	writel(UARTDM_CR_CMD_RESET_RX, priv->base + UARTDM_CR);
	writel(UARTDM_CR_CMD_RESET_TX, priv->base + UARTDM_CR);
	writel(UARTDM_CR_RX_ENABLE, priv->base + UARTDM_CR);
	writel(UARTDM_CR_TX_ENABLE, priv->base + UARTDM_CR);
}
static int msm_serial_probe(struct udevice *dev)
{
	struct msm_serial_data *priv = dev_get_priv(dev);
	long rate;

	/* No need to reinitialize the UART after relocation */
	if (gd->flags & GD_FLG_RELOC)
		return 0;

	rate = msm_uart_clk_init(dev);
	if (rate < 0)
		return rate;
	if (!rate) {
		log_err("Got core clock rate of 0... Please fix your clock driver\n");
		return -EINVAL;
	}

	/* Update the clock rate to the actual programmed rate returned by the
	 * clock driver
	 */
	priv->clk_rate = rate;

	uart_dm_init(priv);

	return 0;
}

static int msm_serial_of_to_plat(struct udevice *dev)
{
	struct msm_serial_data *priv = dev_get_priv(dev);
	int ret;

	priv->base = dev_read_addr(dev);
	if (priv->base == FDT_ADDR_T_NONE)
		return -EINVAL;

	ret = dev_read_u32(dev, "clock-frequency", &priv->clk_rate);
	if (ret < 0) {
		log_debug("No clock frequency specified, using default rate\n");
		/* Default for APQ8016 */
		priv->clk_rate = 7372800;
	}

	return 0;
}

static const struct udevice_id msm_serial_ids[] = {
	{ .compatible = "qcom,msm-uartdm-v1.4" },
	{ }
};

U_BOOT_DRIVER(serial_msm) = {
	.name	= "serial_msm",
	.id	= UCLASS_SERIAL,
	.of_match = msm_serial_ids,
	.of_to_plat = msm_serial_of_to_plat,
	.priv_auto	= sizeof(struct msm_serial_data),
	.probe = msm_serial_probe,
	.ops	= &msm_serial_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

#ifdef CONFIG_DEBUG_UART_MSM

static struct msm_serial_data init_serial_data = {
	.base = CONFIG_VAL(DEBUG_UART_BASE),
	.clk_rate = CONFIG_VAL(DEBUG_UART_CLOCK),
};

#include <debug_uart.h>

/* Uncomment to turn on UART clocks when debugging U-Boot as aboot on MSM8916 */
//int apq8016_clk_init_uart(phys_addr_t gcc_base, unsigned long id);

static inline void _debug_uart_init(void)
{
	/*
	 * Uncomment to turn on UART clocks when debugging U-Boot as aboot
	 * on MSM8916. Supported debug UART clock IDs:
	 *   - db410c: GCC_BLSP1_UART2_APPS_CLK
	 *   - HMIBSC: GCC_BLSP1_UART1_APPS_CLK
	 */
	//apq8016_clk_init_uart(0x1800000, <uart_clk_id>);
	uart_dm_init(&init_serial_data);
}

static inline void _debug_uart_putc(int ch)
{
	struct msm_serial_data *priv = &init_serial_data;

	while (!(readl(priv->base + UARTDM_SR) & UARTDM_SR_TX_READY))
		;

	writel(ch, priv->base + UARTDM_TF);
}

DEBUG_UART_FUNCS

#endif
