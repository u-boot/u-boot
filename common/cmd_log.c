/*
 * (C) Copyright 2002
 * Detlev Zundel, DENX Software Engineering, dzu@denx.de.
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

/*
 * Logbuffer handling routines
 */

#include <common.h>
#include <command.h>
#include <devices.h>
#include <logbuff.h>

#if defined(CONFIG_LOGBUFFER)

#define LOG_BUF_LEN	(16384)
#define LOG_BUF_MASK	(LOG_BUF_LEN-1)

/* Local prototypes */
static void logbuff_putc (const char c);
static void logbuff_puts (const char *s);
static int logbuff_printk(const char *line);

static char buf[1024];

static unsigned console_loglevel = 3;
static unsigned default_message_loglevel = 4;
static unsigned long log_size;
static unsigned char *log_buf=NULL;
static unsigned long *ext_log_start, *ext_logged_chars;
#define log_start (*ext_log_start)
#define logged_chars (*ext_logged_chars)

/* Forced by code, eh! */
#define LOGBUFF_MAGIC 0xc0de4ced

int drv_logbuff_init (void)
{
	device_t logdev;
	int rc;

	/* Device initialization */
	memset (&logdev, 0, sizeof (logdev));

	strcpy (logdev.name, "logbuff");
	logdev.ext   = 0;			/* No extensions */
	logdev.flags = DEV_FLAGS_OUTPUT;	/* Output only */
	logdev.putc  = logbuff_putc;		/* 'putc' function */
	logdev.puts  = logbuff_puts;		/* 'puts' function */

	rc = device_register (&logdev);

	return (rc == 0) ? 1 : rc;
}

static void logbuff_putc (const char c)
{
	char buf[2];
	buf[0]=c;
	buf[1]='\0';
	logbuff_printk(buf);
}

static void logbuff_puts (const char *s)
{
	char buf[512];

	sprintf(buf, "%s\n", s);
	logbuff_printk(buf);
}

void logbuff_log(char *msg)
{
	DECLARE_GLOBAL_DATA_PTR;

	if (gd->flags & GD_FLG_RELOC) {
		logbuff_printk(msg);
	} else {
		puts(msg);
	}
}

void logbuff_reset (void)
{
	char *s;
	unsigned long *ext_tag;

	if ((s = getenv ("logstart")) != NULL) {
		log_buf = (unsigned char *)simple_strtoul(s, NULL, 16);
		ext_tag=(unsigned long *)(log_buf)-3;
		ext_log_start=(unsigned long *)(log_buf)-2;
		ext_logged_chars=(unsigned long *)(log_buf)-1;
//		if (*ext_tag!=LOGBUFF_MAGIC) {
			logged_chars=log_start=0;
			*ext_tag=LOGBUFF_MAGIC;
//		}
		log_size=logged_chars;
	}
}

/*
 * Subroutine:  do_log
 *
 * Description: Handler for 'log' command..
 *
 * Inputs:	argv[1] contains the subcommand
 *
 * Return:      None
 *
 */
int do_log (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char *s;
	unsigned long i;

	if (log_buf==NULL) {
		printf ("No logbuffer defined!  Set 'logstart' to use this feature.\n");
		return 1;
	}

	switch (argc) {

	case 2:
		if (strcmp(argv[1],"show") == 0) {
			for (i=0; i<logged_chars; i++) {
				s=log_buf+((log_start+i)&LOG_BUF_MASK);
				putc(*s);
			}
			return 0;
		} else if (strcmp(argv[1],"reset") == 0) {
			log_start=0;
			logged_chars=0;
			log_size=0;
			return 0;
		}
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;

	case 3:
		if (strcmp(argv[1],"append") == 0) {
			logbuff_puts(argv[2]);
			return 0;

		}
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;

	default:
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}
}

static int logbuff_printk(const char *line)
{
	int i;
	char *msg, *p, *buf_end;
	int line_feed;
	static signed char msg_level = -1;

	strcpy(buf + 3, line);
	i = strlen(line);
	buf_end = buf + 3 + i;
	for (p = buf + 3; p < buf_end; p++) {
		msg = p;
		if (msg_level < 0) {
			if (
				p[0] != '<' ||
				p[1] < '0' ||
				p[1] > '7' ||
				p[2] != '>'
			) {
				p -= 3;
				p[0] = '<';
				p[1] = default_message_loglevel + '0';
				p[2] = '>';
			} else
				msg += 3;
			msg_level = p[1] - '0';
		}
		line_feed = 0;
		for (; p < buf_end; p++) {
			log_buf[(log_start+log_size) & LOG_BUF_MASK] = *p;
			if (log_size < LOG_BUF_LEN)
				log_size++;
			else
				log_start++;

			logged_chars++;
			if (*p == '\n') {
				line_feed = 1;
				break;
			}
		}
		if (msg_level < console_loglevel) {
			printf("%s", msg);
		}
		if (line_feed)
			msg_level = -1;
	}
	return i;
}

#endif /* (CONFIG_LOGBUFFER) */
