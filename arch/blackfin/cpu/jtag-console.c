/*
 * jtag-console.c - console driver over Blackfin JTAG
 *
 * Copyright (c) 2008-2010 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <malloc.h>
#include <stdio_dev.h>
#include <asm/blackfin.h>

#ifdef DEBUG
# define dprintf(...) serial_printf(__VA_ARGS__)
#else
# define dprintf(...) do { if (0) printf(__VA_ARGS__); } while (0)
#endif

static inline void dprintf_decode(const char *s, uint32_t len)
{
	uint32_t i;
	for (i = 0; i < len; ++i)
		if (s[i] < 0x20 || s[i] >= 0x7f)
			dprintf("\\%o", s[i]);
		else
			dprintf("%c", s[i]);
}

static inline uint32_t bfin_write_emudat(uint32_t emudat)
{
	__asm__ __volatile__("emudat = %0;" : : "d"(emudat));
	return emudat;
}

static inline uint32_t bfin_read_emudat(void)
{
	uint32_t emudat;
	__asm__ __volatile__("%0 = emudat;" : "=d"(emudat));
	return emudat;
}

#ifndef CONFIG_JTAG_CONSOLE_TIMEOUT
# define CONFIG_JTAG_CONSOLE_TIMEOUT 500
#endif

/* The Blackfin tends to be much much faster than the JTAG hardware. */
static bool jtag_write_emudat(uint32_t emudat)
{
	static bool overflowed = false;
	ulong timeout = get_timer(0) + CONFIG_JTAG_CONSOLE_TIMEOUT;
	while (bfin_read_DBGSTAT() & 0x1) {
		if (overflowed)
			return overflowed;
		if (timeout < get_timer(0))
			overflowed = true;
	}
	overflowed = false;
	bfin_write_emudat(emudat);
	return overflowed;
}
/* Transmit a buffer.  The format is:
 * [32bit length][actual data]
 */
static void jtag_send(const char *raw_str, uint32_t len)
{
	const char *cooked_str;
	uint32_t i, ex;

	if (len == 0)
		return;

	/* Ugh, need to output \r after \n */
	ex = 0;
	for (i = 0; i < len; ++i)
		if (raw_str[i] == '\n')
			++ex;
	if (ex) {
		char *c = malloc(len + ex);
		cooked_str = c;
		for (i = 0; i < len; ++i) {
			*c++ = raw_str[i];
			if (raw_str[i] == '\n')
				*c++ = '\r';
		}
		len += ex;
	} else
		cooked_str = raw_str;

	dprintf("%s(\"", __func__);
	dprintf_decode(cooked_str, len);
	dprintf("\", %i)\n", len);

	/* First send the length */
	if (jtag_write_emudat(len))
		goto done;

	/* Then send the data */
	for (i = 0; i < len; i += 4) {
		uint32_t emudat =
			(cooked_str[i + 0] <<  0) |
			(cooked_str[i + 1] <<  8) |
			(cooked_str[i + 2] << 16) |
			(cooked_str[i + 3] << 24);
		if (jtag_write_emudat(emudat)) {
			bfin_write_emudat(0);
			goto done;
		}
	}

 done:
	if (cooked_str != raw_str)
		free((char *)cooked_str);
}
static void jtag_putc(const char c)
{
	jtag_send(&c, 1);
}
static void jtag_puts(const char *s)
{
	jtag_send(s, strlen(s));
}

static size_t inbound_len, leftovers_len;

/* Lower layers want to know when jtag has data */
static int jtag_tstc_dbg(void)
{
	int ret = (bfin_read_DBGSTAT() & 0x2);
	if (ret)
		dprintf("%s: ret:%i\n", __func__, ret);
	return ret;
}

/* Higher layers want to know when any data is available */
static int jtag_tstc(void)
{
	return jtag_tstc_dbg() || leftovers_len;
}

/* Receive a buffer.  The format is:
 * [32bit length][actual data]
 */
static uint32_t leftovers;
static int jtag_getc(void)
{
	int ret;
	uint32_t emudat;

	dprintf("%s: inlen:%zu leftlen:%zu left:%x\n", __func__,
		inbound_len, leftovers_len, leftovers);

	/* see if any data is left over */
	if (leftovers_len) {
		--leftovers_len;
		ret = leftovers & 0xff;
		leftovers >>= 8;
		return ret;
	}

	/* wait for new data ! */
	while (!jtag_tstc_dbg())
		continue;
	emudat = bfin_read_emudat();

	if (inbound_len == 0) {
		/* grab the length */
		inbound_len = emudat;
	} else {
		/* store the bytes */
		leftovers_len = min(4, inbound_len);
		inbound_len -= leftovers_len;
		leftovers = emudat;
	}

	return jtag_getc();
}

int drv_jtag_console_init(void)
{
	struct stdio_dev dev;
	int ret;

	memset(&dev, 0x00, sizeof(dev));
	strcpy(dev.name, "jtag");
	dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_INPUT | DEV_FLAGS_SYSTEM;
	dev.putc = jtag_putc;
	dev.puts = jtag_puts;
	dev.tstc = jtag_tstc;
	dev.getc = jtag_getc;

	ret = stdio_register(&dev);
	return (ret == 0 ? 1 : ret);
}

#ifdef CONFIG_UART_CONSOLE_IS_JTAG
/* Since the JTAG is always available (at power on), allow it to fake a UART */
void serial_set_baud(uint32_t baud) {}
void serial_setbrg(void)            {}
int serial_init(void)               { return 0; }
void serial_putc(const char c)      __attribute__((alias("jtag_putc")));
void serial_puts(const char *s)     __attribute__((alias("jtag_puts")));
int serial_tstc(void)               __attribute__((alias("jtag_tstc")));
int serial_getc(void)               __attribute__((alias("jtag_getc")));
#endif
