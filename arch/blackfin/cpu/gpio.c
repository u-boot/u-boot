/*
 * ADI GPIO1 Abstraction Layer
 * Support BF50x, BF51x, BF52x, BF53x and BF561 only.
 *
 * Copyright 2006-2010 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later
 */

#include <common.h>
#include <asm/errno.h>
#include <asm/gpio.h>
#include <asm/portmux.h>

#ifndef CONFIG_ADI_GPIO2
#if ANOMALY_05000311 || ANOMALY_05000323
enum {
	AWA_data = SYSCR,
	AWA_data_clear = SYSCR,
	AWA_data_set = SYSCR,
	AWA_toggle = SYSCR,
	AWA_maska = UART_SCR,
	AWA_maska_clear = UART_SCR,
	AWA_maska_set = UART_SCR,
	AWA_maska_toggle = UART_SCR,
	AWA_maskb = UART_GCTL,
	AWA_maskb_clear = UART_GCTL,
	AWA_maskb_set = UART_GCTL,
	AWA_maskb_toggle = UART_GCTL,
	AWA_dir = SPORT1_STAT,
	AWA_polar = SPORT1_STAT,
	AWA_edge = SPORT1_STAT,
	AWA_both = SPORT1_STAT,
#if ANOMALY_05000311
	AWA_inen = TIMER_ENABLE,
#elif ANOMALY_05000323
	AWA_inen = DMA1_1_CONFIG,
#endif
};
	/* Anomaly Workaround */
#define AWA_DUMMY_READ(name) bfin_read16(AWA_ ## name)
#else
#define AWA_DUMMY_READ(...)  do { } while (0)
#endif

static struct gpio_port_t * const gpio_array[] = {
#if defined(BF533_FAMILY)
	(struct gpio_port_t *) FIO_FLAG_D,
#elif defined(CONFIG_BF52x) || defined(BF537_FAMILY) || defined(CONFIG_BF51x) \
	|| defined(BF538_FAMILY) || defined(CONFIG_BF50x)
	(struct gpio_port_t *) PORTFIO,
# if !defined(BF538_FAMILY)
	(struct gpio_port_t *) PORTGIO,
	(struct gpio_port_t *) PORTHIO,
# endif
#elif defined(BF561_FAMILY)
	(struct gpio_port_t *) FIO0_FLAG_D,
	(struct gpio_port_t *) FIO1_FLAG_D,
	(struct gpio_port_t *) FIO2_FLAG_D,
#else
# error no gpio arrays defined
#endif
};

#if defined(CONFIG_BF52x) || defined(BF537_FAMILY) || defined(CONFIG_BF51x) || \
    defined(CONFIG_BF50x)
static unsigned short * const port_fer[] = {
	(unsigned short *) PORTF_FER,
	(unsigned short *) PORTG_FER,
	(unsigned short *) PORTH_FER,
};

# if !defined(BF537_FAMILY)
static unsigned short * const port_mux[] = {
	(unsigned short *) PORTF_MUX,
	(unsigned short *) PORTG_MUX,
	(unsigned short *) PORTH_MUX,
};

static const
u8 pmux_offset[][16] = {
#  if defined(CONFIG_BF52x)
	{ 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 4, 6, 8, 8, 10, 10 }, /* PORTF */
	{ 0, 0, 0, 0, 0, 2, 2, 4, 4, 6, 8, 10, 10, 10, 12, 12 }, /* PORTG */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 2, 4, 4, 4, 4, 4, 4, 4 }, /* PORTH */
#  elif defined(CONFIG_BF51x)
	{ 0, 2, 2, 2, 2, 2, 2, 4, 6, 6, 6, 8, 8, 8, 8, 10 }, /* PORTF */
	{ 0, 0, 0, 2, 4, 6, 6, 6, 8, 10, 10, 12, 14, 14, 14, 14 }, /* PORTG */
	{ 0, 0, 0, 0, 2, 2, 4, 6, 10, 10, 10, 10, 10, 10, 10, 10 }, /* PORTH */
#  endif
};
# endif

#elif defined(BF538_FAMILY)
static unsigned short * const port_fer[] = {
	(unsigned short *) PORTCIO_FER,
	(unsigned short *) PORTDIO_FER,
	(unsigned short *) PORTEIO_FER,
};
#endif

#ifdef CONFIG_BFIN_GPIO_TRACK
#define RESOURCE_LABEL_SIZE	16

static struct str_ident {
	char name[RESOURCE_LABEL_SIZE];
} str_ident[MAX_RESOURCES];

static void gpio_error(unsigned gpio)
{
	printf("bfin-gpio: GPIO %d wasn't requested!\n", gpio);
}

static void set_label(unsigned short ident, const char *label)
{
	if (label) {
		strncpy(str_ident[ident].name, label,
			 RESOURCE_LABEL_SIZE);
		str_ident[ident].name[RESOURCE_LABEL_SIZE - 1] = 0;
	}
}

static char *get_label(unsigned short ident)
{
	return (*str_ident[ident].name ? str_ident[ident].name : "UNKNOWN");
}

static int cmp_label(unsigned short ident, const char *label)
{
	if (label == NULL)
		printf("bfin-gpio: please provide none-null label\n");

	if (label)
		return strcmp(str_ident[ident].name, label);
	else
		return -EINVAL;
}

#define map_entry(m, i)      reserved_##m##_map[gpio_bank(i)]
#define is_reserved(m, i, e) (map_entry(m, i) & gpio_bit(i))
#define reserve(m, i)        (map_entry(m, i) |= gpio_bit(i))
#define unreserve(m, i)      (map_entry(m, i) &= ~gpio_bit(i))
#define DECLARE_RESERVED_MAP(m, c) static unsigned short reserved_##m##_map[c]
#else
#define is_reserved(m, i, e) (!(e))
#define reserve(m, i)
#define unreserve(m, i)
#define DECLARE_RESERVED_MAP(m, c)
#define gpio_error(gpio)
#define set_label(...)
#define get_label(...) ""
#define cmp_label(...) 1
#endif

DECLARE_RESERVED_MAP(gpio, GPIO_BANK_NUM);
DECLARE_RESERVED_MAP(peri, gpio_bank(MAX_RESOURCES));

inline int check_gpio(unsigned gpio)
{
	if (gpio >= MAX_BLACKFIN_GPIOS)
		return -EINVAL;
	return 0;
}

static void port_setup(unsigned gpio, unsigned short usage)
{
#if defined(BF538_FAMILY)
	/*
	 * BF538/9 Port C,D and E are special.
	 * Inverted PORT_FER polarity on CDE and no PORF_FER on F
	 * Regular PORT F GPIOs are handled here, CDE are exclusively
	 * managed by GPIOLIB
	 */

	if (gpio < MAX_BLACKFIN_GPIOS || gpio >= MAX_RESOURCES)
		return;

	gpio -= MAX_BLACKFIN_GPIOS;

	if (usage == GPIO_USAGE)
		*port_fer[gpio_bank(gpio)] |= gpio_bit(gpio);
	else
		*port_fer[gpio_bank(gpio)] &= ~gpio_bit(gpio);
	SSYNC();
	return;
#endif

	if (check_gpio(gpio))
		return;

#if defined(CONFIG_BF52x) || defined(BF537_FAMILY) || defined(CONFIG_BF51x) || \
    defined(CONFIG_BF50x)
	if (usage == GPIO_USAGE)
		*port_fer[gpio_bank(gpio)] &= ~gpio_bit(gpio);
	else
		*port_fer[gpio_bank(gpio)] |= gpio_bit(gpio);
	SSYNC();
#endif
}

#ifdef BF537_FAMILY
static struct {
	unsigned short res;
	unsigned short offset;
} port_mux_lut[] = {
	{.res = P_PPI0_D13, .offset = 11},
	{.res = P_PPI0_D14, .offset = 11},
	{.res = P_PPI0_D15, .offset = 11},
	{.res = P_SPORT1_TFS, .offset = 11},
	{.res = P_SPORT1_TSCLK, .offset = 11},
	{.res = P_SPORT1_DTPRI, .offset = 11},
	{.res = P_PPI0_D10, .offset = 10},
	{.res = P_PPI0_D11, .offset = 10},
	{.res = P_PPI0_D12, .offset = 10},
	{.res = P_SPORT1_RSCLK, .offset = 10},
	{.res = P_SPORT1_RFS, .offset = 10},
	{.res = P_SPORT1_DRPRI, .offset = 10},
	{.res = P_PPI0_D8, .offset = 9},
	{.res = P_PPI0_D9, .offset = 9},
	{.res = P_SPORT1_DRSEC, .offset = 9},
	{.res = P_SPORT1_DTSEC, .offset = 9},
	{.res = P_TMR2, .offset = 8},
	{.res = P_PPI0_FS3, .offset = 8},
	{.res = P_TMR3, .offset = 7},
	{.res = P_SPI0_SSEL4, .offset = 7},
	{.res = P_TMR4, .offset = 6},
	{.res = P_SPI0_SSEL5, .offset = 6},
	{.res = P_TMR5, .offset = 5},
	{.res = P_SPI0_SSEL6, .offset = 5},
	{.res = P_UART1_RX, .offset = 4},
	{.res = P_UART1_TX, .offset = 4},
	{.res = P_TMR6, .offset = 4},
	{.res = P_TMR7, .offset = 4},
	{.res = P_UART0_RX, .offset = 3},
	{.res = P_UART0_TX, .offset = 3},
	{.res = P_DMAR0, .offset = 3},
	{.res = P_DMAR1, .offset = 3},
	{.res = P_SPORT0_DTSEC, .offset = 1},
	{.res = P_SPORT0_DRSEC, .offset = 1},
	{.res = P_CAN0_RX, .offset = 1},
	{.res = P_CAN0_TX, .offset = 1},
	{.res = P_SPI0_SSEL7, .offset = 1},
	{.res = P_SPORT0_TFS, .offset = 0},
	{.res = P_SPORT0_DTPRI, .offset = 0},
	{.res = P_SPI0_SSEL2, .offset = 0},
	{.res = P_SPI0_SSEL3, .offset = 0},
};

static void portmux_setup(unsigned short per)
{
	u16 y, offset, muxreg, mask;
	u16 function = P_FUNCT2MUX(per);

	for (y = 0; y < ARRAY_SIZE(port_mux_lut); y++) {
		if (port_mux_lut[y].res == per) {

			/* SET PORTMUX REG */

			offset = port_mux_lut[y].offset;
			muxreg = bfin_read_PORT_MUX();

			if (offset == 1)
				mask = 3;
			else
				mask = 1;

			muxreg &= ~(mask << offset);
			muxreg |= ((function & mask) << offset);
			bfin_write_PORT_MUX(muxreg);
		}
	}
}
#elif defined(CONFIG_BF52x) || defined(CONFIG_BF51x)
inline void portmux_setup(unsigned short per)
{
	u16 pmux, ident = P_IDENT(per), function = P_FUNCT2MUX(per);
	u8 offset = pmux_offset[gpio_bank(ident)][gpio_sub_n(ident)];

	pmux = *port_mux[gpio_bank(ident)];
	pmux &= ~(3 << offset);
	pmux |= (function & 3) << offset;
	*port_mux[gpio_bank(ident)] = pmux;
	SSYNC();
}
#else
# define portmux_setup(...)  do { } while (0)
#endif

/***********************************************************
*
* FUNCTIONS: Blackfin General Purpose Ports Access Functions
*
* INPUTS/OUTPUTS:
* gpio - GPIO Number between 0 and MAX_BLACKFIN_GPIOS
*
*
* DESCRIPTION: These functions abstract direct register access
*              to Blackfin processor General Purpose
*              Ports Regsiters
*
* CAUTION: These functions do not belong to the GPIO Driver API
*************************************************************
* MODIFICATION HISTORY :
**************************************************************/

/* Set a specific bit */

#define SET_GPIO(name) \
void set_gpio_ ## name(unsigned gpio, unsigned short arg) \
{ \
	unsigned long flags; \
	local_irq_save(flags); \
	if (arg) \
		gpio_array[gpio_bank(gpio)]->name |= gpio_bit(gpio); \
	else \
		gpio_array[gpio_bank(gpio)]->name &= ~gpio_bit(gpio); \
	AWA_DUMMY_READ(name); \
	local_irq_restore(flags); \
}

SET_GPIO(dir)   /* set_gpio_dir() */
SET_GPIO(inen)  /* set_gpio_inen() */
SET_GPIO(polar) /* set_gpio_polar() */
SET_GPIO(edge)  /* set_gpio_edge() */
SET_GPIO(both)  /* set_gpio_both() */


#define SET_GPIO_SC(name) \
void set_gpio_ ## name(unsigned gpio, unsigned short arg) \
{ \
	unsigned long flags; \
	if (ANOMALY_05000311 || ANOMALY_05000323) \
		local_irq_save(flags); \
	if (arg) \
		gpio_array[gpio_bank(gpio)]->name ## _set = gpio_bit(gpio); \
	else \
		gpio_array[gpio_bank(gpio)]->name ## _clear = gpio_bit(gpio); \
	if (ANOMALY_05000311 || ANOMALY_05000323) { \
		AWA_DUMMY_READ(name); \
		local_irq_restore(flags); \
	} \
}

SET_GPIO_SC(maska)
SET_GPIO_SC(maskb)
SET_GPIO_SC(data)

void set_gpio_toggle(unsigned gpio)
{
	unsigned long flags;
	if (ANOMALY_05000311 || ANOMALY_05000323)
		local_irq_save(flags);
	gpio_array[gpio_bank(gpio)]->toggle = gpio_bit(gpio);
	if (ANOMALY_05000311 || ANOMALY_05000323) {
		AWA_DUMMY_READ(toggle);
		local_irq_restore(flags);
	}
}

/* Set current PORT date (16-bit word) */

#define SET_GPIO_P(name) \
void set_gpiop_ ## name(unsigned gpio, unsigned short arg) \
{ \
	unsigned long flags; \
	if (ANOMALY_05000311 || ANOMALY_05000323) \
		local_irq_save(flags); \
	gpio_array[gpio_bank(gpio)]->name = arg; \
	if (ANOMALY_05000311 || ANOMALY_05000323) { \
		AWA_DUMMY_READ(name); \
		local_irq_restore(flags); \
	} \
}

SET_GPIO_P(data)
SET_GPIO_P(dir)
SET_GPIO_P(inen)
SET_GPIO_P(polar)
SET_GPIO_P(edge)
SET_GPIO_P(both)
SET_GPIO_P(maska)
SET_GPIO_P(maskb)

/* Get a specific bit */
#define GET_GPIO(name) \
unsigned short get_gpio_ ## name(unsigned gpio) \
{ \
	unsigned long flags; \
	unsigned short ret; \
	if (ANOMALY_05000311 || ANOMALY_05000323) \
		local_irq_save(flags); \
	ret = 0x01 & (gpio_array[gpio_bank(gpio)]->name >> gpio_sub_n(gpio)); \
	if (ANOMALY_05000311 || ANOMALY_05000323) { \
		AWA_DUMMY_READ(name); \
		local_irq_restore(flags); \
	} \
	return ret; \
}

GET_GPIO(data)
GET_GPIO(dir)
GET_GPIO(inen)
GET_GPIO(polar)
GET_GPIO(edge)
GET_GPIO(both)
GET_GPIO(maska)
GET_GPIO(maskb)

/* Get current PORT date (16-bit word) */

#define GET_GPIO_P(name) \
unsigned short get_gpiop_ ## name(unsigned gpio) \
{ \
	unsigned long flags; \
	unsigned short ret; \
	if (ANOMALY_05000311 || ANOMALY_05000323) \
		local_irq_save(flags); \
	ret = (gpio_array[gpio_bank(gpio)]->name); \
	if (ANOMALY_05000311 || ANOMALY_05000323) { \
		AWA_DUMMY_READ(name); \
		local_irq_restore(flags); \
	} \
	return ret; \
}

GET_GPIO_P(data)
GET_GPIO_P(dir)
GET_GPIO_P(inen)
GET_GPIO_P(polar)
GET_GPIO_P(edge)
GET_GPIO_P(both)
GET_GPIO_P(maska)
GET_GPIO_P(maskb)

/***********************************************************
*
* FUNCTIONS:	Blackfin Peripheral Resource Allocation
*		and PortMux Setup
*
* INPUTS/OUTPUTS:
* per	Peripheral Identifier
* label	String
*
* DESCRIPTION: Blackfin Peripheral Resource Allocation and Setup API
*
* CAUTION:
*************************************************************
* MODIFICATION HISTORY :
**************************************************************/

int peripheral_request(unsigned short per, const char *label)
{
	unsigned short ident = P_IDENT(per);

	/*
	 * Don't cares are pins with only one dedicated function
	 */

	if (per & P_DONTCARE)
		return 0;

	if (!(per & P_DEFINED))
		return -ENODEV;

	BUG_ON(ident >= MAX_RESOURCES);

	/* If a pin can be muxed as either GPIO or peripheral, make
	 * sure it is not already a GPIO pin when we request it.
	 */
	if (unlikely(!check_gpio(ident) && is_reserved(gpio, ident, 1))) {
		printf("%s: Peripheral %d is already reserved as GPIO by %s !\n",
		       __func__, ident, get_label(ident));
		return -EBUSY;
	}

	if (unlikely(is_reserved(peri, ident, 1))) {

		/*
		 * Pin functions like AMC address strobes my
		 * be requested and used by several drivers
		 */

		if (!(per & P_MAYSHARE)) {
			/*
			 * Allow that the identical pin function can
			 * be requested from the same driver twice
			 */

			if (cmp_label(ident, label) == 0)
				goto anyway;

			printf("%s: Peripheral %d function %d is already reserved by %s !\n",
			       __func__, ident, P_FUNCT2MUX(per), get_label(ident));
			return -EBUSY;
		}
	}

 anyway:
	reserve(peri, ident);

	portmux_setup(per);
	port_setup(ident, PERIPHERAL_USAGE);

	set_label(ident, label);

	return 0;
}

int peripheral_request_list(const unsigned short per[], const char *label)
{
	u16 cnt;
	int ret;

	for (cnt = 0; per[cnt] != 0; cnt++) {

		ret = peripheral_request(per[cnt], label);

		if (ret < 0) {
			for ( ; cnt > 0; cnt--)
				peripheral_free(per[cnt - 1]);

			return ret;
		}
	}

	return 0;
}

void peripheral_free(unsigned short per)
{
	unsigned short ident = P_IDENT(per);

	if (per & P_DONTCARE)
		return;

	if (!(per & P_DEFINED))
		return;

	if (unlikely(!is_reserved(peri, ident, 0)))
		return;

	if (!(per & P_MAYSHARE))
		port_setup(ident, GPIO_USAGE);

	unreserve(peri, ident);

	set_label(ident, "free");
}

void peripheral_free_list(const unsigned short per[])
{
	u16 cnt;
	for (cnt = 0; per[cnt] != 0; cnt++)
		peripheral_free(per[cnt]);
}

/***********************************************************
*
* FUNCTIONS: Blackfin GPIO Driver
*
* INPUTS/OUTPUTS:
* gpio	PIO Number between 0 and MAX_BLACKFIN_GPIOS
* label	String
*
* DESCRIPTION: Blackfin GPIO Driver API
*
* CAUTION:
*************************************************************
* MODIFICATION HISTORY :
**************************************************************/

int gpio_request(unsigned gpio, const char *label)
{
	if (check_gpio(gpio) < 0)
		return -EINVAL;

	/*
	 * Allow that the identical GPIO can
	 * be requested from the same driver twice
	 * Do nothing and return -
	 */

	if (cmp_label(gpio, label) == 0)
		return 0;

	if (unlikely(is_reserved(gpio, gpio, 1))) {
		printf("bfin-gpio: GPIO %d is already reserved by %s !\n",
		       gpio, get_label(gpio));
		return -EBUSY;
	}
	if (unlikely(is_reserved(peri, gpio, 1))) {
		printf("bfin-gpio: GPIO %d is already reserved as Peripheral by %s !\n",
		       gpio, get_label(gpio));
		return -EBUSY;
	}
	else {	/* Reset POLAR setting when acquiring a gpio for the first time */
		set_gpio_polar(gpio, 0);
	}

	reserve(gpio, gpio);
	set_label(gpio, label);

	port_setup(gpio, GPIO_USAGE);

	return 0;
}

int gpio_free(unsigned gpio)
{
	if (check_gpio(gpio) < 0)
		return -1;

	if (unlikely(!is_reserved(gpio, gpio, 0))) {
		gpio_error(gpio);
		return -1;
	}

	unreserve(gpio, gpio);

	set_label(gpio, "free");

	return 0;
}

#ifdef ADI_SPECIAL_GPIO_BANKS
DECLARE_RESERVED_MAP(special_gpio, gpio_bank(MAX_RESOURCES));

int special_gpio_request(unsigned gpio, const char *label)
{
	/*
	 * Allow that the identical GPIO can
	 * be requested from the same driver twice
	 * Do nothing and return -
	 */

	if (cmp_label(gpio, label) == 0)
		return 0;

	if (unlikely(is_reserved(special_gpio, gpio, 1))) {
		printf("bfin-gpio: GPIO %d is already reserved by %s !\n",
		       gpio, get_label(gpio));
		return -EBUSY;
	}
	if (unlikely(is_reserved(peri, gpio, 1))) {
		printf("bfin-gpio: GPIO %d is already reserved as Peripheral by %s !\n",
		       gpio, get_label(gpio));

		return -EBUSY;
	}

	reserve(special_gpio, gpio);
	reserve(peri, gpio);

	set_label(gpio, label);
	port_setup(gpio, GPIO_USAGE);

	return 0;
}

void special_gpio_free(unsigned gpio)
{
	if (unlikely(!is_reserved(special_gpio, gpio, 0))) {
		gpio_error(gpio);
		return;
	}

	unreserve(special_gpio, gpio);
	unreserve(peri, gpio);
	set_label(gpio, "free");
}
#endif

static inline void __gpio_direction_input(unsigned gpio)
{
	gpio_array[gpio_bank(gpio)]->dir &= ~gpio_bit(gpio);
	gpio_array[gpio_bank(gpio)]->inen |= gpio_bit(gpio);
}

int gpio_direction_input(unsigned gpio)
{
	unsigned long flags;

	if (!is_reserved(gpio, gpio, 0)) {
		gpio_error(gpio);
		return -EINVAL;
	}

	local_irq_save(flags);
	__gpio_direction_input(gpio);
	AWA_DUMMY_READ(inen);
	local_irq_restore(flags);

	return 0;
}

int gpio_set_value(unsigned gpio, int arg)
{
	if (arg)
		gpio_array[gpio_bank(gpio)]->data_set = gpio_bit(gpio);
	else
		gpio_array[gpio_bank(gpio)]->data_clear = gpio_bit(gpio);

	return 0;
}

int gpio_direction_output(unsigned gpio, int value)
{
	unsigned long flags;

	if (!is_reserved(gpio, gpio, 0)) {
		gpio_error(gpio);
		return -EINVAL;
	}

	local_irq_save(flags);

	gpio_array[gpio_bank(gpio)]->inen &= ~gpio_bit(gpio);
	gpio_set_value(gpio, value);
	gpio_array[gpio_bank(gpio)]->dir |= gpio_bit(gpio);

	AWA_DUMMY_READ(dir);
	local_irq_restore(flags);

	return 0;
}

int gpio_get_value(unsigned gpio)
{
	unsigned long flags;

	if (unlikely(get_gpio_edge(gpio))) {
		int ret;
		local_irq_save(flags);
		set_gpio_edge(gpio, 0);
		ret = get_gpio_data(gpio);
		set_gpio_edge(gpio, 1);
		local_irq_restore(flags);
		return ret;
	} else
		return get_gpio_data(gpio);
}

/* If we are booting from SPI and our board lacks a strong enough pull up,
 * the core can reset and execute the bootrom faster than the resistor can
 * pull the signal logically high.  To work around this (common) error in
 * board design, we explicitly set the pin back to GPIO mode, force /CS
 * high, and wait for the electrons to do their thing.
 *
 * This function only makes sense to be called from reset code, but it
 * lives here as we need to force all the GPIO states w/out going through
 * BUG() checks and such.
 */
void bfin_reset_boot_spi_cs(unsigned short pin)
{
	unsigned short gpio = P_IDENT(pin);
	port_setup(gpio, GPIO_USAGE);
	gpio_array[gpio_bank(gpio)]->data_set = gpio_bit(gpio);
	AWA_DUMMY_READ(data_set);
	udelay(1);
}

void gpio_labels(void)
{
	int c, gpio;

	for (c = 0; c < MAX_RESOURCES; c++) {
		gpio = is_reserved(gpio, c, 1);
		if (!check_gpio(c) && gpio)
			printf("GPIO_%d:\t%s\tGPIO %s\n", c,
				get_label(c),
				get_gpio_dir(c) ? "OUTPUT" : "INPUT");
		else if (is_reserved(peri, c, 1))
			printf("GPIO_%d:\t%s\tPeripheral\n", c, get_label(c));
		else
			continue;
	}
}
#else
struct gpio_port_t * const gpio_array[] = {
	(struct gpio_port_t *)PORTA_FER,
	(struct gpio_port_t *)PORTB_FER,
	(struct gpio_port_t *)PORTC_FER,
	(struct gpio_port_t *)PORTD_FER,
	(struct gpio_port_t *)PORTE_FER,
	(struct gpio_port_t *)PORTF_FER,
	(struct gpio_port_t *)PORTG_FER,
#if defined(CONFIG_BF54x)
	(struct gpio_port_t *)PORTH_FER,
	(struct gpio_port_t *)PORTI_FER,
	(struct gpio_port_t *)PORTJ_FER,
#endif
};
#endif
