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
#include <devices.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_SERIAL_MULTI)

static struct serial_device *serial_devices = NULL;
static struct serial_device *serial_current = NULL;

#ifndef CONFIG_LWMON
struct serial_device *default_serial_console (void)
{
#if defined(CONFIG_8xx_CONS_SMC1) || defined(CONFIG_8xx_CONS_SMC2)
	return &serial_smc_device;
#elif defined(CONFIG_8xx_CONS_SCC1) || defined(CONFIG_8xx_CONS_SCC2) \
   || defined(CONFIG_8xx_CONS_SCC3) || defined(CONFIG_8xx_CONS_SCC4)
	return &serial_scc_device;
#elif defined(CONFIG_405GP) || defined(CONFIG_405CR) || defined(CONFIG_440) \
   || defined(CONFIG_405EP) || defined(CONFIG_MPC5xxx)
#if defined(CONFIG_CONS_INDEX) && defined(CFG_NS16550_SERIAL)
#if (CONFIG_CONS_INDEX==1)
	return &eserial1_device;
#elif (CONFIG_CONS_INDEX==2)
	return &eserial2_device;
#elif (CONFIG_CONS_INDEX==3)
	return &eserial3_device;
#elif (CONFIG_CONS_INDEX==4)
	return &eserial4_device;
#else
#error "Bad CONFIG_CONS_INDEX."
#endif
#elif defined(CONFIG_UART1_CONSOLE)
		return &serial1_device;
#else
		return &serial0_device;
#endif
#else
#error No default console
#endif
}
#endif

static int serial_register (struct serial_device *dev)
{
	dev->init += gd->reloc_off;
	dev->setbrg += gd->reloc_off;
	dev->getc += gd->reloc_off;
	dev->tstc += gd->reloc_off;
	dev->putc += gd->reloc_off;
	dev->puts += gd->reloc_off;

	dev->next = serial_devices;
	serial_devices = dev;

	return 0;
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

#if defined(CONFIG_405GP) || defined(CONFIG_405CR) || defined(CONFIG_440) \
 || defined(CONFIG_405EP) || defined(CONFIG_MPC5xxx)
	serial_register(&serial0_device);
	serial_register(&serial1_device);
#endif

#if defined(CFG_NS16550_SERIAL)
#if defined(CFG_NS16550_COM1)
	serial_register(&eserial1_device);
#endif
#if defined(CFG_NS16550_COM2)
	serial_register(&eserial2_device);
#endif
#if defined(CFG_NS16550_COM3)
	serial_register(&eserial3_device);
#endif
#if defined(CFG_NS16550_COM4)
	serial_register(&eserial4_device);
#endif
#endif /* CFG_NS16550_SERIAL */
	serial_assign (default_serial_console ()->name);
}

void serial_devices_init (void)
{
	device_t dev;
	struct serial_device *s = serial_devices;

	while (s) {
		memset (&dev, 0, sizeof (dev));

		strcpy (dev.name, s->name);
		dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_INPUT;

		dev.start = s->init;
		dev.putc = s->putc;
		dev.puts = s->puts;
		dev.getc = s->getc;
		dev.tstc = s->tstc;

		device_register (&dev);

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

#endif /* CONFIG_SERIAL_MULTI */
