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

static struct serial_device *serial_devices = NULL;
static struct serial_device *serial_current = NULL;

void serial_register(struct serial_device *dev)
{
#ifdef CONFIG_NEEDS_MANUAL_RELOC
	dev->init += gd->reloc_off;
	dev->setbrg += gd->reloc_off;
	dev->getc += gd->reloc_off;
	dev->tstc += gd->reloc_off;
	dev->putc += gd->reloc_off;
	dev->puts += gd->reloc_off;
#endif

	dev->next = serial_devices;
	serial_devices = dev;
}

void serial_initialize (void)
{
#if defined(CONFIG_8xx_CONS_SMC1) || defined(CONFIG_8xx_CONS_SMC2)
	serial_register (&serial_smc_device);
#endif
#if defined(CONFIG_8xx_CONS_SCC1) || defined(CONFIG_8xx_CONS_SCC2) \
 || defined(CONFIG_8xx_CONS_SCC3) || defined(CONFIG_8xx_CONS_SCC4)
	serial_register (&serial_scc_device);
#endif

#if defined(CONFIG_SYS_NS16550_SERIAL)
#if defined(CONFIG_SYS_NS16550_COM1)
	serial_register(&eserial1_device);
#endif
#if defined(CONFIG_SYS_NS16550_COM2)
	serial_register(&eserial2_device);
#endif
#if defined(CONFIG_SYS_NS16550_COM3)
	serial_register(&eserial3_device);
#endif
#if defined(CONFIG_SYS_NS16550_COM4)
	serial_register(&eserial4_device);
#endif
#endif /* CONFIG_SYS_NS16550_SERIAL */
#if defined (CONFIG_FFUART)
	serial_register(&serial_ffuart_device);
#endif
#if defined (CONFIG_BTUART)
	serial_register(&serial_btuart_device);
#endif
#if defined (CONFIG_STUART)
	serial_register(&serial_stuart_device);
#endif
#if defined(CONFIG_S3C2410)
	serial_register(&s3c24xx_serial0_device);
	serial_register(&s3c24xx_serial1_device);
	serial_register(&s3c24xx_serial2_device);
#endif
#if defined(CONFIG_S5P)
	serial_register(&s5p_serial0_device);
	serial_register(&s5p_serial1_device);
	serial_register(&s5p_serial2_device);
	serial_register(&s5p_serial3_device);
#endif
#if defined(CONFIG_MPC512X)
#if defined(CONFIG_SYS_PSC1)
	serial_register(&serial1_device);
#endif
#if defined(CONFIG_SYS_PSC3)
	serial_register(&serial3_device);
#endif
#if defined(CONFIG_SYS_PSC4)
	serial_register(&serial4_device);
#endif
#if defined(CONFIG_SYS_PSC6)
	serial_register(&serial6_device);
#endif
#endif
#if defined(CONFIG_SYS_BFIN_UART)
	serial_register_bfin_uart();
#endif
	serial_assign (default_serial_console ()->name);
}

void serial_stdio_init (void)
{
	struct stdio_dev dev;
	struct serial_device *s = serial_devices;

	while (s) {
		memset (&dev, 0, sizeof (dev));

		strcpy (dev.name, s->name);
		dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_INPUT;

		dev.start = s->init;
		dev.stop = s->uninit;
		dev.putc = s->putc;
		dev.puts = s->puts;
		dev.getc = s->getc;
		dev.tstc = s->tstc;

		stdio_register (&dev);

		s = s->next;
	}
}

int serial_assign (char *name)
{
	struct serial_device *s;

	for (s = serial_devices; s; s = s->next) {
		if (strcmp (s->name, name) == 0) {
			serial_current = s;
			return 0;
		}
	}

	return 1;
}

void serial_reinit_all (void)
{
	struct serial_device *s;

	for (s = serial_devices; s; s = s->next) {
		s->init ();
	}
}

int serial_init (void)
{
	if (!(gd->flags & GD_FLG_RELOC) || !serial_current) {
		struct serial_device *dev = default_serial_console ();

		return dev->init ();
	}

	return serial_current->init ();
}

void serial_setbrg (void)
{
	if (!(gd->flags & GD_FLG_RELOC) || !serial_current) {
		struct serial_device *dev = default_serial_console ();

		dev->setbrg ();
		return;
	}

	serial_current->setbrg ();
}

int serial_getc (void)
{
	if (!(gd->flags & GD_FLG_RELOC) || !serial_current) {
		struct serial_device *dev = default_serial_console ();

		return dev->getc ();
	}

	return serial_current->getc ();
}

int serial_tstc (void)
{
	if (!(gd->flags & GD_FLG_RELOC) || !serial_current) {
		struct serial_device *dev = default_serial_console ();

		return dev->tstc ();
	}

	return serial_current->tstc ();
}

void serial_putc (const char c)
{
	if (!(gd->flags & GD_FLG_RELOC) || !serial_current) {
		struct serial_device *dev = default_serial_console ();

		dev->putc (c);
		return;
	}

	serial_current->putc (c);
}

void serial_puts (const char *s)
{
	if (!(gd->flags & GD_FLG_RELOC) || !serial_current) {
		struct serial_device *dev = default_serial_console ();

		dev->puts (s);
		return;
	}

	serial_current->puts (s);
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

		/* XXX: There is no serial_uninit() !? */
		if (s->uninit)
			s->uninit();
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
