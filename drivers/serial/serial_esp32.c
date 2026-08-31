// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2026, Honbo He <hehongbo918@gmail.com>
 */

#define LOG_CATEGORY UCLASS_SERIAL

#include <clk.h>
#include <debug_uart.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <serial.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <linux/err.h>
#include <linux/bitops.h>

#define UART_FIFO			0x00
#define UART_INT_ENA		0x0c
#define UART_INT_CLR		0x10
#define UART_CLKDIV			0x14
#define UART_STATUS			0x1c
#define UART_CONF0			0x20

#define UART_CLKDIV_FRAG_SHIFT	20
#define UART_CLKDIV_FRAG_MASK	0xf

#define UART_TX_FLOW_EN		BIT(13)
#define UART_LOOPBACK		BIT(12)
#define UART_STOP_BIT_1		BIT(4)
#define UART_PARITY_EN		BIT(1)
#define UART_PARITY			BIT(0)

#define UART_BIT_NUM_8		(3 << 2)

#define UART_BIT_NUM_MASK	GENMASK(3, 2)
#define UART_STOP_BIT_MASK	GENMASK(5, 4)

#define UART_RXFIFO_CNT_SHIFT	0
#define UART_TXFIFO_CNT_SHIFT	16
#define UART_FIFO_LEN		128

#define UART_UPDATE_TIMEOUT 10000

struct esp32_uart_soc_data {
	u32 reg_update;		/* UART_ID_REG */
	u32 reg_update_bit; /* UART_REG_UPDATE */
	u32 clkdiv_mask;
	u32 fifo_cnt_mask;
	u32 rx_fifo_rst;	/* UART_RXFIFO_RST */
	u32 tx_fifo_rst;	/* UART_TXFIFO_RST */

	bool broken_fifo_rst; /* true for ESP32, false otherwise */
};

/* Information about a serial port */
struct esp32_uart_plat {
	void __iomem *base; /* address of registers in physical memory */
	const struct esp32_uart_soc_data *data;
	u32 clock_rate;
};

static const struct esp32_uart_soc_data esp32_uart_data = {
	.clkdiv_mask = 0xfffff,
	.fifo_cnt_mask = 0xff,
	.rx_fifo_rst = BIT(17),
	.tx_fifo_rst = BIT(18),

	.broken_fifo_rst = true,
};

static const struct esp32_uart_soc_data esp32s3_uart_data = {
	.reg_update = 0x80,
	.reg_update_bit = BIT(31),
	.clkdiv_mask = 0xfff,
	.fifo_cnt_mask = 0x3ff,
	.rx_fifo_rst = BIT(17),
	.tx_fifo_rst = BIT(18),
};

static const struct esp32_uart_soc_data esp32s31_uart_data = {
	.reg_update = 0x98,
	.reg_update_bit = BIT(0),
	.clkdiv_mask = 0xfff,
	.fifo_cnt_mask = 0xff,
	.rx_fifo_rst = BIT(22),
	.tx_fifo_rst = BIT(23),
};

static u32 esp32_uart_read(void __iomem *base, u32 offset)
{
	return readl(base + offset);
}

static void esp32_uart_write(void __iomem *base, u32 offset, u32 val)
{
	writel(val, base + offset);
}

static u32 esp32_uart_field(u32 val, u32 mask, u32 shift)
{
	return (val >> shift) & mask;
}

static u32 esp32_uart_rx_fifo_count(void __iomem *base,
				    const struct esp32_uart_soc_data *data)
{
	u32 status = esp32_uart_read(base, UART_STATUS);

	return esp32_uart_field(status, data->fifo_cnt_mask,
				UART_RXFIFO_CNT_SHIFT);
}

static u32 esp32_uart_tx_fifo_count(void __iomem *base,
				    const struct esp32_uart_soc_data *data)
{
	u32 status = esp32_uart_read(base, UART_STATUS);

	return esp32_uart_field(status, data->fifo_cnt_mask,
				UART_TXFIFO_CNT_SHIFT);
}

static int esp32_uart_update(void __iomem *base,
			     const struct esp32_uart_soc_data *data)
{
	int timeout = UART_UPDATE_TIMEOUT;

	if (!data->reg_update_bit)
		return 0;

	setbits_le32(base + data->reg_update, data->reg_update_bit);
	while ((esp32_uart_read(base, data->reg_update) & data->reg_update_bit) &&
	       timeout--)
		;

	return timeout < 0 ? -ETIMEDOUT : 0;
}

static int esp32_uart_reset_fifo(void __iomem *base,
				 const struct esp32_uart_soc_data *data)
{
	u32 mask = data->tx_fifo_rst;
	int ret;

	/*
	 * uart2 of ESP32 doesn't have any register to reset Tx_FIFO or Rx_FIFO,
	 * and the FIFO_RST of uart1 may impact the functioning of uart2
	 * so use soft reset instead
	 */
	if (data->broken_fifo_rst) {
		while (esp32_uart_rx_fifo_count(base, data))
			esp32_uart_read(base, UART_FIFO);
		return 0;
	}
	mask |= data->rx_fifo_rst;

	setbits_le32(base + UART_CONF0, mask);
	ret = esp32_uart_update(base, data);
	if (ret)
		return ret;

	clrbits_le32(base + UART_CONF0, mask);
	return esp32_uart_update(base, data);
}

/*
 * div = clkdiv_int + clkdiv_frag / 16
 * baudrate = uart_clk / div
 */
static int esp32_uart_setbrg(void __iomem *base,
			     const struct esp32_uart_soc_data *data,
			     u32 clock_rate, int baudrate)
{
	u32 div, int_div, frag;

	if (!clock_rate || baudrate <= 0)
		return -EINVAL;
	if (clock_rate > (UINT_MAX >> 4))
		return -EINVAL;

	/* shifting left by 4 bits to preserve frag field */
	div = (clock_rate << 4) / baudrate;
	int_div = div >> 4;
	frag = div & UART_CLKDIV_FRAG_MASK;

	if (!int_div || int_div > data->clkdiv_mask)
		return -EINVAL;

	esp32_uart_write(base, UART_CLKDIV,
			 int_div | (frag << UART_CLKDIV_FRAG_SHIFT));

	return esp32_uart_update(base, data);
}

static int esp32_uart_init(void __iomem *base,
			   const struct esp32_uart_soc_data *data)
{
	u32 conf0;

	esp32_uart_write(base, UART_INT_ENA, 0);
	esp32_uart_write(base, UART_INT_CLR, 0xffffffff);

	conf0 = esp32_uart_read(base, UART_CONF0);
	conf0 &= ~(UART_PARITY | UART_PARITY_EN |
		   UART_BIT_NUM_MASK | UART_STOP_BIT_MASK |
		   UART_LOOPBACK | UART_TX_FLOW_EN);
	conf0 |= UART_BIT_NUM_8 | UART_STOP_BIT_1;
	esp32_uart_write(base, UART_CONF0, conf0);

	return esp32_uart_reset_fifo(base, data);
}

static int esp32_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct esp32_uart_plat *plat = dev_get_plat(dev);

	return esp32_uart_setbrg(plat->base, plat->data, plat->clock_rate,
				 baudrate);
}

static int esp32_serial_getc(struct udevice *dev)
{
	struct esp32_uart_plat *plat = dev_get_plat(dev);

	if (!esp32_uart_rx_fifo_count(plat->base, plat->data))
		return -EAGAIN;

	return esp32_uart_read(plat->base, UART_FIFO) & 0xff;
}

static int esp32_serial_putc(struct udevice *dev, const char ch)
{
	struct esp32_uart_plat *plat = dev_get_plat(dev);

	if (esp32_uart_tx_fifo_count(plat->base, plat->data) >= UART_FIFO_LEN)
		return -EAGAIN;

	esp32_uart_write(plat->base, UART_FIFO, ch);

	return 0;
}

static int esp32_serial_pending(struct udevice *dev, bool input)
{
	struct esp32_uart_plat *plat = dev_get_plat(dev);

	if (input)
		return esp32_uart_rx_fifo_count(plat->base, plat->data);

	return esp32_uart_tx_fifo_count(plat->base, plat->data) ? 1 : 0;
}

static int esp32_serial_probe(struct udevice *dev)
{
	struct esp32_uart_plat *plat = dev_get_plat(dev);
	const struct esp32_uart_soc_data *data;

	data = (const struct esp32_uart_soc_data *)dev_get_driver_data(dev);
	if (!data)
		return -EINVAL;
	plat->data = data;

	return esp32_uart_init(plat->base, data);
}

static int esp32_serial_of_to_plat(struct udevice *dev)
{
	struct esp32_uart_plat *plat = dev_get_plat(dev);
	fdt_addr_t addr;
	struct clk clk;
	int ret;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	plat->base = (void __iomem *)(uintptr_t)addr;
	ret = clk_get_by_index(dev, 0, &clk);
	if (!ret) {
		ret = clk_get_rate(&clk);
		if (!IS_ERR_VALUE(ret))
			plat->clock_rate = ret;
	} else if (ret != -ENOENT && ret != -ENODEV && ret != -ENOSYS) {
		dev_err(dev, "failed to get clock: %d\n", ret);
		return ret;
	}

	if (!plat->clock_rate)
		plat->clock_rate = dev_read_u32_default(dev, "clock-frequency", 0);

	if (!plat->clock_rate) {
		dev_err(dev, "missing clock\n");
		return -EINVAL;
	}

	return 0;
}

static const struct dm_serial_ops esp32_serial_ops = {
	.putc = esp32_serial_putc,
	.pending = esp32_serial_pending,
	.getc = esp32_serial_getc,
	.setbrg = esp32_serial_setbrg,
};

static const struct udevice_id esp32_serial_ids[] = {
	{ .compatible = "esp,esp32-uart",
	  .data = (ulong)&esp32_uart_data },
	{ .compatible = "esp,esp32s3-uart",
	  .data = (ulong)&esp32s3_uart_data },
	{ .compatible = "esp,esp32s31-uart",
	  .data = (ulong)&esp32s31_uart_data },
	{ }
};

U_BOOT_DRIVER(serial_esp32) = {
	.name = "serial_esp32",
	.id = UCLASS_SERIAL,
	.of_match = of_match_ptr(esp32_serial_ids),
	.of_to_plat = of_match_ptr(esp32_serial_of_to_plat),
	.plat_auto = sizeof(struct esp32_uart_plat),
	.probe = esp32_serial_probe,
	.ops = &esp32_serial_ops,
#if !CONFIG_IS_ENABLED(OF_CONTROL)
	.flags = DM_FLAG_PRE_RELOC,
#endif
};

#ifdef CONFIG_DEBUG_UART_ESP32
static inline const struct esp32_uart_soc_data *
esp32_debug_uart_data(void)
{
#if defined(CONFIG_ESPRESSIF_ESP32S31)
	return &esp32s31_uart_data;
#elif defined(CONFIG_ESPRESSIF_ESP32S3)
	return &esp32s3_uart_data;
#else
	return &esp32_uart_data;
#endif
}

static inline void _debug_uart_init(void)
{
	void __iomem *base = (void __iomem *)CONFIG_VAL(DEBUG_UART_BASE);
	const struct esp32_uart_soc_data *data = esp32_debug_uart_data();

	esp32_uart_setbrg(base, data,
			CONFIG_DEBUG_UART_CLOCK,
			CONFIG_BAUDRATE);
	esp32_uart_init(base, data);
}

static inline void _debug_uart_putc(int ch)
{
	void __iomem *base = (void __iomem *)CONFIG_VAL(DEBUG_UART_BASE);
	const struct esp32_uart_soc_data *data =
			esp32_debug_uart_data();

	while (esp32_uart_tx_fifo_count(base, data)
				>= UART_FIFO_LEN)
		;
	esp32_uart_write(base, UART_FIFO, ch);
}

DEBUG_UART_FUNCS
#endif
