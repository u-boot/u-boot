/*
 * (C) Copyright 2015
 * Kamil Lulko, <rev13@wp.pl>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <serial.h>
#include <asm/arch/stm32.h>

/*
 * Set up the usart port
 */
#if (CONFIG_STM32_USART >= 1) && (CONFIG_STM32_USART <= 6)
#define USART_PORT	(CONFIG_STM32_USART - 1)
#else
#define USART_PORT	0
#endif
/*
 * Set up the usart base address
 *
 * --STM32_USARTD_BASE means default setting
 */
#define STM32_USART1_BASE	(STM32_APB2PERIPH_BASE + 0x1000)
#define STM32_USART2_BASE	(STM32_APB1PERIPH_BASE + 0x4400)
#define STM32_USART3_BASE	(STM32_APB1PERIPH_BASE + 0x4800)
#define STM32_USART6_BASE	(STM32_APB2PERIPH_BASE + 0x1400)
#define STM32_USARTD_BASE	STM32_USART1_BASE
/*
 * RCC USART specific definitions
 *
 * --RCC_ENR_USARTDEN means default setting
 */
#define RCC_ENR_USART1EN	(1 << 4)
#define RCC_ENR_USART2EN	(1 << 17)
#define RCC_ENR_USART3EN	(1 << 18)
#define RCC_ENR_USART6EN	(1 <<  5)
#define RCC_ENR_USARTDEN	RCC_ENR_USART1EN

struct stm32_serial {
	u32 sr;
	u32 dr;
	u32 brr;
	u32 cr1;
	u32 cr2;
	u32 cr3;
	u32 gtpr;
};

#define USART_CR1_RE		(1 << 2)
#define USART_CR1_TE		(1 << 3)
#define USART_CR1_UE		(1 << 13)

#define USART_SR_FLAG_RXNE	(1 << 5)
#define USART_SR_FLAG_TXE	(1 << 7)

#define USART_BRR_F_MASK	0xF
#define USART_BRR_M_SHIFT	4
#define USART_BRR_M_MASK	0xFFF0

DECLARE_GLOBAL_DATA_PTR;

static const unsigned long usart_base[] = {
	STM32_USART1_BASE,
	STM32_USART2_BASE,
	STM32_USART3_BASE,
	STM32_USARTD_BASE,
	STM32_USARTD_BASE,
	STM32_USART6_BASE
};

static const unsigned long rcc_enr_en[] = {
	RCC_ENR_USART1EN,
	RCC_ENR_USART2EN,
	RCC_ENR_USART3EN,
	RCC_ENR_USARTDEN,
	RCC_ENR_USARTDEN,
	RCC_ENR_USART6EN
};

static void stm32_serial_setbrg(void)
{
	serial_init();
}

static int stm32_serial_init(void)
{
	struct stm32_serial *usart =
		(struct stm32_serial *)usart_base[USART_PORT];
	u32 clock, int_div, frac_div, tmp;

	if ((usart_base[USART_PORT] & STM32_BUS_MASK) ==
			STM32_APB1PERIPH_BASE) {
		setbits_le32(&STM32_RCC->apb1enr, rcc_enr_en[USART_PORT]);
		clock = clock_get(CLOCK_APB1);
	} else if ((usart_base[USART_PORT] & STM32_BUS_MASK) ==
			STM32_APB2PERIPH_BASE) {
		setbits_le32(&STM32_RCC->apb2enr, rcc_enr_en[USART_PORT]);
		clock = clock_get(CLOCK_APB2);
	} else {
		return -1;
	}

	int_div = (25 * clock) / (4 * gd->baudrate);
	tmp = ((int_div / 100) << USART_BRR_M_SHIFT) & USART_BRR_M_MASK;
	frac_div = int_div - (100 * (tmp >> USART_BRR_M_SHIFT));
	tmp |= (((frac_div * 16) + 50) / 100) & USART_BRR_F_MASK;

	writel(tmp, &usart->brr);
	setbits_le32(&usart->cr1, USART_CR1_RE | USART_CR1_TE | USART_CR1_UE);

	return 0;
}

static int stm32_serial_getc(void)
{
	struct stm32_serial *usart =
		(struct stm32_serial *)usart_base[USART_PORT];
	while ((readl(&usart->sr) & USART_SR_FLAG_RXNE) == 0)
		;
	return readl(&usart->dr);
}

static void stm32_serial_putc(const char c)
{
	struct stm32_serial *usart =
		(struct stm32_serial *)usart_base[USART_PORT];

	if (c == '\n')
		stm32_serial_putc('\r');

	while ((readl(&usart->sr) & USART_SR_FLAG_TXE) == 0)
		;
	writel(c, &usart->dr);
}

static int stm32_serial_tstc(void)
{
	struct stm32_serial *usart =
		(struct stm32_serial *)usart_base[USART_PORT];
	u8 ret;

	ret = readl(&usart->sr) & USART_SR_FLAG_RXNE;
	return ret;
}

static struct serial_device stm32_serial_drv = {
	.name	= "stm32_serial",
	.start	= stm32_serial_init,
	.stop	= NULL,
	.setbrg	= stm32_serial_setbrg,
	.putc	= stm32_serial_putc,
	.puts	= default_serial_puts,
	.getc	= stm32_serial_getc,
	.tstc	= stm32_serial_tstc,
};

void stm32_serial_initialize(void)
{
	serial_register(&stm32_serial_drv);
}

__weak struct serial_device *default_serial_console(void)
{
	return &stm32_serial_drv;
}
