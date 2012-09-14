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
#include <serial.h>
#include <stdio_dev.h>
#include <post.h>
#include <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

static struct serial_device *serial_devices;
static struct serial_device *serial_current;

static void serial_null(void)
{
}

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
serial_initfunc(s3c4510b_serial_initialize);
serial_initfunc(s3c64xx_serial_initialize);
serial_initfunc(sandbox_serial_initialize);
serial_initfunc(clps7111_serial_initialize);
serial_initfunc(imx_serial_initialize);
serial_initfunc(ixp_serial_initialize);
serial_initfunc(ks8695_serial_initialize);
serial_initfunc(lh7a40x_serial_initialize);
serial_initfunc(lpc2292_serial_initialize);
serial_initfunc(max3100_serial_initialize);
serial_initfunc(mxc_serial_initialize);

void serial_register(struct serial_device *dev)
{
#ifdef CONFIG_NEEDS_MANUAL_RELOC
	dev->start += gd->reloc_off;
	dev->setbrg += gd->reloc_off;
	dev->getc += gd->reloc_off;
	dev->tstc += gd->reloc_off;
	dev->putc += gd->reloc_off;
	dev->puts += gd->reloc_off;
#endif

	dev->next = serial_devices;
	serial_devices = dev;
}

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
	s3c4510b_serial_initialize();
	s3c64xx_serial_initialize();
	sandbox_serial_initialize();
	clps7111_serial_initialize();
	imx_serial_initialize();
	ixp_serial_initialize();
	ks8695_serial_initialize();
	lh7a40x_serial_initialize();
	lpc2292_serial_initialize();
	max3100_serial_initialize();
	mxc_serial_initialize();

	serial_assign(default_serial_console()->name);
}

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

int serial_assign(const char *name)
{
	struct serial_device *s;

	for (s = serial_devices; s; s = s->next) {
		if (strcmp(s->name, name) == 0) {
			serial_current = s;
			return 0;
		}
	}

	return 1;
}

void serial_reinit_all(void)
{
	struct serial_device *s;

	for (s = serial_devices; s; s = s->next)
		s->start();
}

static struct serial_device *get_current(void)
{
	struct serial_device *dev;

	if (!(gd->flags & GD_FLG_RELOC) || !serial_current) {
		dev = default_serial_console();

		/* We must have a console device */
		if (!dev)
			panic("Cannot find console");
	} else
		dev = serial_current;
	return dev;
}

int serial_init(void)
{
	return get_current()->start();
}

void serial_setbrg(void)
{
	get_current()->setbrg();
}

int serial_getc(void)
{
	return get_current()->getc();
}

int serial_tstc(void)
{
	return get_current()->tstc();
}

void serial_putc(const char c)
{
	get_current()->putc(c);
}

void serial_puts(const char *s)
{
	get_current()->puts(s);
}

#if CONFIG_POST & CONFIG_SYS_POST_UART
static const int bauds[] = CONFIG_SYS_BAUDRATE_TABLE;

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
