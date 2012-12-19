/*
 * (C) Copyright 2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <environment.h>
#include <serial.h>
#include <stdio_dev.h>
#include <post.h>
#include <linux/compiler.h>
#include <errno.h>

DECLARE_GLOBAL_DATA_PTR;

static struct serial_device *serial_devices;
static struct serial_device *serial_current;
/*
 * Table with supported baudrates (defined in config_xyz.h)
 */
static const unsigned long baudrate_table[] = CONFIG_SYS_BAUDRATE_TABLE;
#define	N_BAUDRATES (sizeof(baudrate_table) / sizeof(baudrate_table[0]))

/**
 * serial_null() - Void registration routine of a serial driver
 *
 * This routine implements a void registration routine of a serial
 * driver. The registration routine of a particular driver is aliased
 * to this empty function in case the driver is not compiled into
 * U-Boot.
 */
static void serial_null(void)
{
}

/**
 * on_baudrate() - Update the actual baudrate when the env var changes
 *
 * This will check for a valid baudrate and only apply it if valid.
 */
static int on_baudrate(const char *name, const char *value, enum env_op op,
	int flags)
{
	int i;
	int baudrate;

	switch (op) {
	case env_op_create:
	case env_op_overwrite:
		/*
		 * Switch to new baudrate if new baudrate is supported
		 */
		baudrate = simple_strtoul(value, NULL, 10);

		/* Not actually changing */
		if (gd->baudrate == baudrate)
			return 0;

		for (i = 0; i < N_BAUDRATES; ++i) {
			if (baudrate == baudrate_table[i])
				break;
		}
		if (i == N_BAUDRATES) {
			if ((flags & H_FORCE) == 0)
				printf("## Baudrate %d bps not supported\n",
					baudrate);
			return 1;
		}
		if ((flags & H_INTERACTIVE) != 0) {
			printf("## Switch baudrate to %d"
				" bps and press ENTER ...\n", baudrate);
			udelay(50000);
		}

		gd->baudrate = baudrate;
#if defined(CONFIG_PPC) || defined(CONFIG_MCF52x2)
		gd->bd->bi_baudrate = baudrate;
#endif

		serial_setbrg();

		udelay(50000);

		if ((flags & H_INTERACTIVE) != 0)
			while (1) {
				if (getc() == '\r')
					break;
			}

		return 0;
	case env_op_delete:
		printf("## Baudrate may not be deleted\n");
		return 1;
	default:
		return 0;
	}
}
U_BOOT_ENV_CALLBACK(baudrate, on_baudrate);

/**
 * serial_initfunc() - Forward declare of driver registration routine
 * @name:	Name of the real driver registration routine.
 *
 * This macro expands onto forward declaration of a driver registration
 * routine, which is then used below in serial_initialize() function.
 * The declaration is made weak and aliases to serial_null() so in case
 * the driver is not compiled in, the function is still declared and can
 * be used, but aliases to serial_null() and thus is optimized away.
 */
#define serial_initfunc(name)					\
	void name(void)						\
		__attribute__((weak, alias("serial_null")));

serial_initfunc(mpc8xx_serial_initialize);
serial_initfunc(ns16550_serial_initialize);
serial_initfunc(pxa_serial_initialize);
serial_initfunc(s3c24xx_serial_initialize);
serial_initfunc(s5p_serial_initialize);
serial_initfunc(zynq_serial_initalize);
serial_initfunc(bfin_serial_initialize);
serial_initfunc(bfin_jtag_initialize);
serial_initfunc(mpc512x_serial_initialize);
serial_initfunc(uartlite_serial_initialize);
serial_initfunc(au1x00_serial_initialize);
serial_initfunc(asc_serial_initialize);
serial_initfunc(jz_serial_initialize);
serial_initfunc(mpc5xx_serial_initialize);
serial_initfunc(mpc8220_serial_initialize);
serial_initfunc(mpc8260_scc_serial_initialize);
serial_initfunc(mpc8260_smc_serial_initialize);
serial_initfunc(mpc85xx_serial_initialize);
serial_initfunc(iop480_serial_initialize);
serial_initfunc(leon2_serial_initialize);
serial_initfunc(leon3_serial_initialize);
serial_initfunc(marvell_serial_initialize);
serial_initfunc(amirix_serial_initialize);
serial_initfunc(bmw_serial_initialize);
serial_initfunc(cogent_serial_initialize);
serial_initfunc(cpci750_serial_initialize);
serial_initfunc(evb64260_serial_initialize);
serial_initfunc(ml2_serial_initialize);
serial_initfunc(sconsole_serial_initialize);
serial_initfunc(p3mx_serial_initialize);
serial_initfunc(altera_jtag_serial_initialize);
serial_initfunc(altera_serial_initialize);
serial_initfunc(atmel_serial_initialize);
serial_initfunc(lpc32xx_serial_initialize);
serial_initfunc(mcf_serial_initialize);
serial_initfunc(ns9750_serial_initialize);
serial_initfunc(oc_serial_initialize);
serial_initfunc(s3c64xx_serial_initialize);
serial_initfunc(sandbox_serial_initialize);
serial_initfunc(clps7111_serial_initialize);
serial_initfunc(imx_serial_initialize);
serial_initfunc(ixp_serial_initialize);
serial_initfunc(ks8695_serial_initialize);
serial_initfunc(lh7a40x_serial_initialize);
serial_initfunc(max3100_serial_initialize);
serial_initfunc(mxc_serial_initialize);
serial_initfunc(pl01x_serial_initialize);
serial_initfunc(s3c44b0_serial_initialize);
serial_initfunc(sa1100_serial_initialize);
serial_initfunc(sh_serial_initialize);

/**
 * serial_register() - Register serial driver with serial driver core
 * @dev:	Pointer to the serial driver structure
 *
 * This function registers the serial driver supplied via @dev with
 * serial driver core, thus making U-Boot aware of it and making it
 * available for U-Boot to use. On platforms that still require manual
 * relocation of constant variables, relocation of the supplied structure
 * is performed.
 */
void serial_register(struct serial_device *dev)
{
#ifdef CONFIG_NEEDS_MANUAL_RELOC
	if (dev->start)
		dev->start += gd->reloc_off;
	if (dev->stop)
		dev->stop += gd->reloc_off;
	if (dev->setbrg)
		dev->setbrg += gd->reloc_off;
	if (dev->getc)
		dev->getc += gd->reloc_off;
	if (dev->tstc)
		dev->tstc += gd->reloc_off;
	if (dev->putc)
		dev->putc += gd->reloc_off;
	if (dev->puts)
		dev->puts += gd->reloc_off;
#endif

	dev->next = serial_devices;
	serial_devices = dev;
}

/**
 * serial_initialize() - Register all compiled-in serial port drivers
 *
 * This function registers all serial port drivers that are compiled
 * into the U-Boot binary with the serial core, thus making them
 * available to U-Boot to use. Lastly, this function assigns a default
 * serial port to the serial core. That serial port is then used as a
 * default output.
 */
void serial_initialize(void)
{
	mpc8xx_serial_initialize();
	ns16550_serial_initialize();
	pxa_serial_initialize();
	s3c24xx_serial_initialize();
	s5p_serial_initialize();
	mpc512x_serial_initialize();
	bfin_serial_initialize();
	bfin_jtag_initialize();
	uartlite_serial_initialize();
	zynq_serial_initalize();
	au1x00_serial_initialize();
	asc_serial_initialize();
	jz_serial_initialize();
	mpc5xx_serial_initialize();
	mpc8220_serial_initialize();
	mpc8260_scc_serial_initialize();
	mpc8260_smc_serial_initialize();
	mpc85xx_serial_initialize();
	iop480_serial_initialize();
	leon2_serial_initialize();
	leon3_serial_initialize();
	marvell_serial_initialize();
	amirix_serial_initialize();
	bmw_serial_initialize();
	cogent_serial_initialize();
	cpci750_serial_initialize();
	evb64260_serial_initialize();
	ml2_serial_initialize();
	sconsole_serial_initialize();
	p3mx_serial_initialize();
	altera_jtag_serial_initialize();
	altera_serial_initialize();
	atmel_serial_initialize();
	lpc32xx_serial_initialize();
	mcf_serial_initialize();
	ns9750_serial_initialize();
	oc_serial_initialize();
	s3c64xx_serial_initialize();
	sandbox_serial_initialize();
	clps7111_serial_initialize();
	imx_serial_initialize();
	ixp_serial_initialize();
	ks8695_serial_initialize();
	lh7a40x_serial_initialize();
	max3100_serial_initialize();
	mxc_serial_initialize();
	pl01x_serial_initialize();
	s3c44b0_serial_initialize();
	sa1100_serial_initialize();
	sh_serial_initialize();

	serial_assign(default_serial_console()->name);
}

/**
 * serial_stdio_init() - Register serial ports with STDIO core
 *
 * This function generates a proxy driver for each serial port driver.
 * These proxy drivers then register with the STDIO core, making the
 * serial drivers available as STDIO devices.
 */
void serial_stdio_init(void)
{
	struct stdio_dev dev;
	struct serial_device *s = serial_devices;

	while (s) {
		memset(&dev, 0, sizeof(dev));

		strcpy(dev.name, s->name);
		dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_INPUT;

		dev.start = s->start;
		dev.stop = s->stop;
		dev.putc = s->putc;
		dev.puts = s->puts;
		dev.getc = s->getc;
		dev.tstc = s->tstc;

		stdio_register(&dev);

		s = s->next;
	}
}

/**
 * serial_assign() - Select the serial output device by name
 * @name:	Name of the serial driver to be used as default output
 *
 * This function configures the serial output multiplexing by
 * selecting which serial device will be used as default. In case
 * the STDIO "serial" device is selected as stdin/stdout/stderr,
 * the serial device previously configured by this function will be
 * used for the particular operation.
 *
 * Returns 0 on success, negative on error.
 */
int serial_assign(const char *name)
{
	struct serial_device *s;

	for (s = serial_devices; s; s = s->next) {
		if (strcmp(s->name, name))
			continue;
		serial_current = s;
		return 0;
	}

	return -EINVAL;
}

/**
 * serial_reinit_all() - Reinitialize all compiled-in serial ports
 *
 * This function reinitializes all serial ports that are compiled
 * into U-Boot by calling their serial_start() functions.
 */
void serial_reinit_all(void)
{
	struct serial_device *s;

	for (s = serial_devices; s; s = s->next)
		s->start();
}

/**
 * get_current() - Return pointer to currently selected serial port
 *
 * This function returns a pointer to currently selected serial port.
 * The currently selected serial port is altered by serial_assign()
 * function.
 *
 * In case this function is called before relocation or before any serial
 * port is configured, this function calls default_serial_console() to
 * determine the serial port. Otherwise, the configured serial port is
 * returned.
 *
 * Returns pointer to the currently selected serial port on success,
 * NULL on error.
 */
static struct serial_device *get_current(void)
{
	struct serial_device *dev;

	if (!(gd->flags & GD_FLG_RELOC))
		dev = default_serial_console();
	else if (!serial_current)
		dev = default_serial_console();
	else
		dev = serial_current;

	/* We must have a console device */
	if (!dev) {
#ifdef CONFIG_SPL_BUILD
		puts("Cannot find console\n");
		hang();
#else
		panic("Cannot find console\n");
#endif
	}

	return dev;
}

/**
 * serial_init() - Initialize currently selected serial port
 *
 * This function initializes the currently selected serial port. This
 * usually involves setting up the registers of that particular port,
 * enabling clock and such. This function uses the get_current() call
 * to determine which port is selected.
 *
 * Returns 0 on success, negative on error.
 */
int serial_init(void)
{
	return get_current()->start();
}

/**
 * serial_setbrg() - Configure baud-rate of currently selected serial port
 *
 * This function configures the baud-rate of the currently selected
 * serial port. The baud-rate is retrieved from global data within
 * the serial port driver. This function uses the get_current() call
 * to determine which port is selected.
 *
 * Returns 0 on success, negative on error.
 */
void serial_setbrg(void)
{
	get_current()->setbrg();
}

/**
 * serial_getc() - Read character from currently selected serial port
 *
 * This function retrieves a character from currently selected serial
 * port. In case there is no character waiting on the serial port,
 * this function will block and wait for the character to appear. This
 * function uses the get_current() call to determine which port is
 * selected.
 *
 * Returns the character on success, negative on error.
 */
int serial_getc(void)
{
	return get_current()->getc();
}

/**
 * serial_tstc() - Test if data is available on currently selected serial port
 *
 * This function tests if one or more characters are available on
 * currently selected serial port. This function never blocks. This
 * function uses the get_current() call to determine which port is
 * selected.
 *
 * Returns positive if character is available, zero otherwise.
 */
int serial_tstc(void)
{
	return get_current()->tstc();
}

/**
 * serial_putc() - Output character via currently selected serial port
 * @c:	Single character to be output from the serial port.
 *
 * This function outputs a character via currently selected serial
 * port. This character is passed to the serial port driver responsible
 * for controlling the hardware. The hardware may still be in process
 * of transmitting another character, therefore this function may block
 * for a short amount of time. This function uses the get_current()
 * call to determine which port is selected.
 */
void serial_putc(const char c)
{
	get_current()->putc(c);
}

/**
 * serial_puts() - Output string via currently selected serial port
 * @s:	Zero-terminated string to be output from the serial port.
 *
 * This function outputs a zero-terminated string via currently
 * selected serial port. This function behaves as an accelerator
 * in case the hardware can queue multiple characters for transfer.
 * The whole string that is to be output is available to the function
 * implementing the hardware manipulation. Transmitting the whole
 * string may take some time, thus this function may block for some
 * amount of time. This function uses the get_current() call to
 * determine which port is selected.
 */
void serial_puts(const char *s)
{
	get_current()->puts(s);
}

/**
 * default_serial_puts() - Output string by calling serial_putc() in loop
 * @s:	Zero-terminated string to be output from the serial port.
 *
 * This function outputs a zero-terminated string by calling serial_putc()
 * in a loop. Most drivers do not support queueing more than one byte for
 * transfer, thus this function precisely implements their serial_puts().
 *
 * To optimize the number of get_current() calls, this function only
 * calls get_current() once and then directly accesses the putc() call
 * of the &struct serial_device .
 */
void default_serial_puts(const char *s)
{
	struct serial_device *dev = get_current();
	while (*s)
		dev->putc(*s++);
}

#if CONFIG_POST & CONFIG_SYS_POST_UART
static const int bauds[] = CONFIG_SYS_BAUDRATE_TABLE;

/**
 * uart_post_test() - Test the currently selected serial port using POST
 * @flags:	POST framework flags
 *
 * Do a loopback test of the currently selected serial port. This
 * function is only useful in the context of the POST testing framwork.
 * The serial port is firstly configured into loopback mode and then
 * characters are sent through it.
 *
 * Returns 0 on success, value otherwise.
 */
/* Mark weak until post/cpu/.../uart.c migrate over */
__weak
int uart_post_test(int flags)
{
	unsigned char c;
	int ret, saved_baud, b;
	struct serial_device *saved_dev, *s;
	bd_t *bd = gd->bd;

	/* Save current serial state */
	ret = 0;
	saved_dev = serial_current;
	saved_baud = bd->bi_baudrate;

	for (s = serial_devices; s; s = s->next) {
		/* If this driver doesn't support loop back, skip it */
		if (!s->loop)
			continue;

		/* Test the next device */
		serial_current = s;

		ret = serial_init();
		if (ret)
			goto done;

		/* Consume anything that happens to be queued */
		while (serial_tstc())
			serial_getc();

		/* Enable loop back */
		s->loop(1);

		/* Test every available baud rate */
		for (b = 0; b < ARRAY_SIZE(bauds); ++b) {
			bd->bi_baudrate = bauds[b];
			serial_setbrg();

			/*
			 * Stick to printable chars to avoid issues:
			 *  - terminal corruption
			 *  - serial program reacting to sequences and sending
			 *    back random extra data
			 *  - most serial drivers add in extra chars (like \r\n)
			 */
			for (c = 0x20; c < 0x7f; ++c) {
				/* Send it out */
				serial_putc(c);

				/* Make sure it's the same one */
				ret = (c != serial_getc());
				if (ret) {
					s->loop(0);
					goto done;
				}

				/* Clean up the output in case it was sent */
				serial_putc('\b');
				ret = ('\b' != serial_getc());
				if (ret) {
					s->loop(0);
					goto done;
				}
			}
		}

		/* Disable loop back */
		s->loop(0);

		/* XXX: There is no serial_stop() !? */
		if (s->stop)
			s->stop();
	}

 done:
	/* Restore previous serial state */
	serial_current = saved_dev;
	bd->bi_baudrate = saved_baud;
	serial_reinit_all();
	serial_setbrg();

	return ret;
}
#endif
