/*
 * (C) Copyright 2008, 2009 Andreas Pfefferle,
 *     DENX Software Engineering, ap@denx.de.
 * (C) Copyright 2009 Detlev Zundel,
 *     DENX Software Engineering, dzu@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/io.h>
#include <common.h>
#include <config.h>
#include <console.h>
#include <mpc5xxx.h>
#include <pci.h>

#include <command.h>

/* This is needed for the includes in ns16550.h */
#define CONFIG_SYS_NS16550_REG_SIZE 1
#include <ns16550.h>

#define GPIO_BASE		((u_char *)CONFIG_SYS_CS3_START)

#define DIGIN_TOUCHSCR_MASK	0x00003000	/* Inputs 12-13 */
#define DIGIN_KEYB_MASK		0x00010000	/* Input 16 */

#define DIGIN_DRAWER_SW1	0x00400000	/* Input 22 */
#define DIGIN_DRAWER_SW2	0x00800000	/* Input 23 */

#define DIGIO_LED0		0x00000001	/* Output 0 */
#define DIGIO_LED1		0x00000002	/* Output 1 */
#define DIGIO_LED2		0x00000004	/* Output 2 */
#define DIGIO_LED3		0x00000008	/* Output 3 */
#define DIGIO_LED4		0x00000010	/* Output 4 */
#define DIGIO_LED5		0x00000020	/* Output 5 */

#define DIGIO_DRAWER1		0x00000100	/* Output 8 */
#define DIGIO_DRAWER2		0x00000200	/* Output 9 */

#define SERIAL_PORT_BASE	((u_char *)CONFIG_SYS_CS2_START)

#define PSC_OP1_RTS	0x01
#define PSC_OP0_RTS	0x01

/*
 * Table with supported baudrates (defined in inka4x0.h)
 */
static const unsigned long baudrate_table[] = CONFIG_SYS_BAUDRATE_TABLE;
#define	N_BAUDRATES (sizeof(baudrate_table) / sizeof(baudrate_table[0]))

static unsigned int inka_digin_get_input(void)
{
	return in_8(GPIO_BASE + 0) << 0 | in_8(GPIO_BASE + 1) << 8 |
		in_8(GPIO_BASE + 2) << 16 | in_8(GPIO_BASE + 3) << 24;
}

#define LED_HIGH(NUM)							\
	do {								\
		setbits_be32((unsigned *)MPC5XXX_GPT##NUM##_ENABLE, 0x10); \
	} while (0)

#define LED_LOW(NUM)							\
	do {								\
		clrbits_be32((unsigned *)MPC5XXX_GPT##NUM##_ENABLE, 0x10); \
	} while (0)

#define CHECK_LED(NUM) \
    do { \
	    if (state & (1 << NUM)) {		\
		    LED_HIGH(NUM);		\
	    } else {				\
		    LED_LOW(NUM);		\
	    }					\
    } while (0)

static void inka_digio_set_output(unsigned int state, int which)
{
	volatile struct mpc5xxx_gpio *gpio = (struct mpc5xxx_gpio *)MPC5XXX_GPIO;

	if (which == 0) {
		/* other */
		CHECK_LED(0);
		CHECK_LED(1);
		CHECK_LED(2);
		CHECK_LED(3);
		CHECK_LED(4);
		CHECK_LED(5);
	} else {
		if (which == 1) {
			/* drawer1 */
			if (state) {
				clrbits_be32(&gpio->simple_dvo, 0x1000);
				udelay(1);
				setbits_be32(&gpio->simple_dvo, 0x1000);
			} else {
				setbits_be32(&gpio->simple_dvo, 0x1000);
				udelay(1);
				clrbits_be32(&gpio->simple_dvo, 0x1000);
			}
		}
		if (which == 2) {
			/* drawer 2 */
			if (state) {
				clrbits_be32(&gpio->simple_dvo, 0x2000);
				udelay(1);
				setbits_be32(&gpio->simple_dvo, 0x2000);
			} else {
				setbits_be32(&gpio->simple_dvo, 0x2000);
				udelay(1);
				clrbits_be32(&gpio->simple_dvo, 0x2000);
			}
		}
	}
	udelay(1);
}

static int do_inkadiag_io(cmd_tbl_t *cmdtp, int flag, int argc,
			  char * const argv[]) {
	unsigned int state, val;

	switch (argc) {
	case 3:
		/* Write a value */
		val = simple_strtol(argv[2], NULL, 16);

		if (strcmp(argv[1], "drawer1") == 0) {
			inka_digio_set_output(val, 1);
		} else if (strcmp(argv[1], "drawer2") == 0) {
			inka_digio_set_output(val, 2);
		} else if (strcmp(argv[1], "other") == 0)
			inka_digio_set_output(val, 0);
		else {
			printf("Invalid argument: %s\n", argv[1]);
			return -1;
		}
		/* fall through */
	case 2:
		/* Read a value */
		state = inka_digin_get_input();

		if (strcmp(argv[1], "drawer1") == 0) {
			val = (state & DIGIN_DRAWER_SW1) >> (ffs(DIGIN_DRAWER_SW1) - 1);
		} else if (strcmp(argv[1], "drawer2") == 0) {
			val = (state & DIGIN_DRAWER_SW2) >> (ffs(DIGIN_DRAWER_SW2) - 1);
		} else if (strcmp(argv[1], "other") == 0) {
			val = ((state & DIGIN_KEYB_MASK) >> (ffs(DIGIN_KEYB_MASK) - 1))
				| (state & DIGIN_TOUCHSCR_MASK) >> (ffs(DIGIN_TOUCHSCR_MASK) - 2);
		} else {
			printf("Invalid argument: %s\n", argv[1]);
			return -1;
		}
		printf("exit code: 0x%X\n", val);
		return 0;
	default:
		return cmd_usage(cmdtp);
	}

	return -1;
}

DECLARE_GLOBAL_DATA_PTR;

static int ser_init(volatile struct mpc5xxx_psc *psc, int baudrate)
{
	unsigned long baseclk;
	int div;

	/* reset PSC */
	out_8(&psc->command, PSC_SEL_MODE_REG_1);

	/* select clock sources */

	out_be16(&psc->psc_clock_select, 0);
	baseclk = (gd->arch.ipb_clk + 16) / 32;

	/* switch to UART mode */
	out_be32(&psc->sicr, 0);

	/* configure parity, bit length and so on */

	out_8(&psc->mode, PSC_MODE_8_BITS | PSC_MODE_PARNONE);
	out_8(&psc->mode, PSC_MODE_ONE_STOP);

	/* set up UART divisor */
	div = (baseclk + (baudrate / 2)) / baudrate;
	out_8(&psc->ctur, (div >> 8) & 0xff);
	out_8(&psc->ctlr, div & 0xff);

	/* disable all interrupts */
	out_be16(&psc->psc_imr, 0);

	/* reset and enable Rx/Tx */
	out_8(&psc->command, PSC_RST_RX);
	out_8(&psc->command, PSC_RST_TX);
	out_8(&psc->command, PSC_RX_ENABLE | PSC_TX_ENABLE);

	return 0;
}

static void ser_putc(volatile struct mpc5xxx_psc *psc, const char c)
{
	/* Wait 1 second for last character to go. */
	int i = 0;

	while (!(psc->psc_status & PSC_SR_TXEMP) && (i++ < 1000000/10))
		udelay(10);
	psc->psc_buffer_8 = c;

}

static int ser_getc(volatile struct mpc5xxx_psc *psc)
{
	/* Wait for a character to arrive. */
	int i = 0;

	while (!(in_be16(&psc->psc_status) & PSC_SR_RXRDY) && (i++ < 1000000/10))
		udelay(10);

	return in_8(&psc->psc_buffer_8);
}

static int do_inkadiag_serial(cmd_tbl_t *cmdtp, int flag, int argc,
			      char * const argv[]) {
	volatile struct NS16550 *uart;
	volatile struct mpc5xxx_psc *psc;
	unsigned int num, mode;
	int combrd, baudrate, i, j, len;
	int address;

	if (argc < 5)
		return cmd_usage(cmdtp);

	argc--;
	argv++;

	num = simple_strtol(argv[0], NULL, 0);
	if (num < 0 || num > 11) {
		printf("invalid argument for num: %d\n", num);
		return -1;
	}

	mode = simple_strtol(argv[1], NULL, 0);

	combrd = 0;
	baudrate = simple_strtoul(argv[2], NULL, 10);
	for (i=0; i<N_BAUDRATES; ++i) {
		if (baudrate == baudrate_table[i])
			break;
	}
	if (i == N_BAUDRATES) {
		printf("## Baudrate %d bps not supported\n",
		       baudrate);
		return 1;
	}
	combrd = 115200 / baudrate;

	uart = (struct NS16550 *)(SERIAL_PORT_BASE + (num << 3));

	printf("Testing uart %d.\n\n", num);

	if ((num >= 0) && (num <= 7)) {
		if (mode & 1) {
			/* turn on 'loopback' mode */
			out_8(&uart->mcr, UART_MCR_LOOP);
		} else {
			/*
			 * establish the UART's operational parameters
			 * set DLAB=1, so rbr accesses DLL
			 */
			out_8(&uart->lcr, UART_LCR_DLAB);
			/* set baudrate */
			out_8(&uart->rbr, combrd);
			/* set data-format: 8-N-1 */
			out_8(&uart->lcr, UART_LCR_WLS_8);
		}

		if (mode & 2) {
			/* set request to send */
			out_8(&uart->mcr, UART_MCR_RTS);
			udelay(10);
			/* check clear to send */
			if ((in_8(&uart->msr) & UART_MSR_CTS) == 0x00)
				return -1;
		}
		if (mode & 4) {
			/* set data terminal ready */
			out_8(&uart->mcr, UART_MCR_DTR);
			udelay(10);
			/* check data set ready and carrier detect */
			if ((in_8(&uart->msr) & (UART_MSR_DSR | UART_MSR_DCD))
			    != (UART_MSR_DSR | UART_MSR_DCD))
				return -1;
		}

		/* write each message-character, read it back, and display it */
		for (i = 0, len = strlen(argv[3]); i < len; ++i) {
			j = 0;
			while ((in_8(&uart->lsr) & UART_LSR_THRE) ==	0x00) {
				if (j++ > CONFIG_SYS_HZ)
					break;
				udelay(10);
			}
			out_8(&uart->rbr, argv[3][i]);
			j = 0;
			while ((in_8(&uart->lsr) & UART_LSR_DR) == 0x00) {
				if (j++ > CONFIG_SYS_HZ)
					break;
				udelay(10);
			}
			printf("%c", in_8(&uart->rbr));
		}
		printf("\n\n");
		out_8(&uart->mcr, 0x00);
	} else {
		address = 0;

		switch (num) {
		case 8:
			address = MPC5XXX_PSC6;
			break;
		case 9:
			address = MPC5XXX_PSC3;
			break;
		case 10:
			address = MPC5XXX_PSC2;
			break;
		case 11:
			address = MPC5XXX_PSC1;
			break;
		}
		psc = (struct mpc5xxx_psc *)address;
		ser_init(psc, simple_strtol(argv[2], NULL, 0));
		if (mode & 2) {
			/* set request to send */
			out_8(&psc->op0, PSC_OP0_RTS);
			udelay(10);
			/* check clear to send */
			if ((in_8(&psc->ip) & PSC_IPCR_CTS) == 0)
				return -1;
		}
		len = strlen(argv[3]);
		for (i = 0; i < len; ++i) {
			ser_putc(psc, argv[3][i]);
			printf("%c", ser_getc(psc));
		}
		printf("\n\n");
	}
	return 0;
}

#define BUZZER_GPT	(MPC5XXX_GPT + 0x60)	/* GPT6 */
static void buzzer_turn_on(unsigned int freq)
{
	volatile struct mpc5xxx_gpt *gpt = (struct mpc5xxx_gpt *)(BUZZER_GPT);

	const u32 prescale = gd->arch.ipb_clk / freq / 128;
	const u32 count = 128;
	const u32 width = 64;

	gpt->cir = (prescale << 16) | count;
	gpt->pwmcr = width << 16;
	gpt->emsr = 3;		/* Timer enabled for PWM */
}

static void buzzer_turn_off(void)
{
	volatile struct mpc5xxx_gpt *gpt = (struct mpc5xxx_gpt *)(BUZZER_GPT);

	gpt->emsr = 0;
}

static int do_inkadiag_buzzer(cmd_tbl_t *cmdtp, int flag, int argc,
			      char * const argv[]) {

	unsigned int period, freq;
	int prev, i;

	if (argc != 3)
		return cmd_usage(cmdtp);

	argc--;
	argv++;

	period = simple_strtol(argv[0], NULL, 0);
	if (!period)
		printf("Zero period is senseless\n");
	argc--;
	argv++;

	freq = simple_strtol(argv[0], NULL, 0);
	/* avoid zero prescale in buzzer_turn_on() */
	if (freq > gd->arch.ipb_clk / 128) {
		printf("%dHz exceeds maximum (%ldHz)\n", freq,
		       gd->arch.ipb_clk / 128);
	} else if (!freq)
		printf("Zero frequency is senseless\n");
	else
		buzzer_turn_on(freq);

	clear_ctrlc();
	prev = disable_ctrlc(0);

	printf("Buzzing for %d ms. Type ^C to abort!\n\n", period);

	i = 0;
	while (!ctrlc() && (i++ < CONFIG_SYS_HZ))
		udelay(period);

	clear_ctrlc();
	disable_ctrlc(prev);

	buzzer_turn_off();

	return 0;
}

static int do_inkadiag_help(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);

cmd_tbl_t cmd_inkadiag_sub[] = {
	U_BOOT_CMD_MKENT(io, 1, 1, do_inkadiag_io, "read digital input",
	 "<drawer1|drawer2|other> [value] - get or set specified signal"),
	U_BOOT_CMD_MKENT(serial, 4, 1, do_inkadiag_serial, "test serial port",
	 "<num> <mode> <baudrate> <msg>  - test uart num [0..11] in mode\n"
	 "and baudrate with msg"),
	U_BOOT_CMD_MKENT(buzzer, 2, 1, do_inkadiag_buzzer, "activate buzzer",
	 "<period> <freq> - turn buzzer on for period ms with freq hz"),
	U_BOOT_CMD_MKENT(help, 4, 1, do_inkadiag_help, "get help",
	 "[command] - get help for command"),
};

static int do_inkadiag_help(cmd_tbl_t *cmdtp, int flag,
			    int argc, char * const argv[]) {
	extern int _do_help (cmd_tbl_t *cmd_start, int cmd_items,
			     cmd_tbl_t *cmdtp, int flag,
			     int argc, char * const argv[]);
	/* do_help prints command name - we prepend inkadiag to our subcommands! */
#ifdef CONFIG_SYS_LONGHELP
	puts ("inkadiag ");
#endif
	return _do_help(&cmd_inkadiag_sub[0],
		ARRAY_SIZE(cmd_inkadiag_sub), cmdtp, flag, argc, argv);
}

static int do_inkadiag(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[]) {
	cmd_tbl_t *c;

	c = find_cmd_tbl(argv[1], &cmd_inkadiag_sub[0], ARRAY_SIZE(cmd_inkadiag_sub));

	if (c) {
		argc--;
		argv++;
		return c->cmd(c, flag, argc, argv);
	} else {
		/* Unrecognized command */
		return cmd_usage(cmdtp);
	}
}

U_BOOT_CMD(inkadiag, 6, 1, do_inkadiag,
	   "inkadiag - inka diagnosis\n",
	   "[inkadiag what ...]\n"
	   "    - perform a diagnosis on inka hardware\n"
	   "'inkadiag' performs hardware tests.");
