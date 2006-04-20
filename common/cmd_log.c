/*
 * (C) Copyright 2002
 * Detlev Zundel, DENX Software Engineering, dzu@denx.de.
 *
 * Code used from linux/kernel/printk.c
 * Copyright (C) 1991, 1992  Linus Torvalds
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
 *
 * Comments:
 *
 * After relocating the code, the environment variable "loglevel" is
 * copied to console_loglevel.  The functionality is similar to the
 * handling in the Linux kernel, i.e. messages logged with a priority
 * less than console_loglevel are also output to stdout.
 *
 * If you want messages with the default level (e.g. POST messages) to
 * appear on stdout also, make sure the environment variable
 * "loglevel" is set at boot time to a number higher than
 * default_message_loglevel below.
 */

/*
 * Logbuffer handling routines
 */

#include <common.h>
#include <command.h>
#include <devices.h>
#include <post.h>
#include <logbuff.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_LOGBUFFER)

/* Local prototypes */
static void logbuff_putc (const char c);
static void logbuff_puts (const char *s);
static int logbuff_printk(const char *line);

static char buf[1024];

/* This combination will not print messages with the default loglevel */
static unsigned console_loglevel = 3;
static unsigned default_message_loglevel = 4;
static unsigned char *log_buf = NULL;
static unsigned long *ext_log_size;
static unsigned long *ext_log_start;
static unsigned long *ext_logged_chars;
#define log_size (*ext_log_size)
#define log_start (*ext_log_start)
#define logged_chars (*ext_logged_chars)

/* Forced by code, eh! */
#define LOGBUFF_MAGIC 0xc0de4ced

/* The mapping used here has to be the same as in setup_ext_logbuff ()
   in linux/kernel/printk */
void logbuff_init_ptrs (void)
{
	unsigned long *ext_tag;
	unsigned long post_word;
	char *s;

	log_buf = (unsigned char *)(gd->bd->bi_memsize-LOGBUFF_LEN);
	ext_tag = (unsigned long *)(log_buf)-4;
 	ext_log_start = (unsigned long *)(log_buf)-3;
	ext_log_size = (unsigned long *)(log_buf)-2;
	ext_logged_chars = (unsigned long *)(log_buf)-1;
	post_word = post_word_load();
#ifdef CONFIG_POST
	/* The post routines have setup the word so we can simply test it */
 	if (post_word_load () & POST_COLDBOOT) {
 		logged_chars = log_size = log_start = 0;
		*ext_tag = LOGBUFF_MAGIC;
 	}
#else
	/* No post routines, so we do our own checking                    */
 	if (post_word != LOGBUFF_MAGIC) {
 		logged_chars = log_size = log_start = 0;
		post_word_store (LOGBUFF_MAGIC);
		*ext_tag = LOGBUFF_MAGIC;
 	}
#endif
	/* Initialize default loglevel if present */
	if ((s = getenv ("loglevel")) != NULL)
		console_loglevel = (int)simple_strtoul (s, NULL, 10);

	gd->post_log_word |= LOGBUFF_INITIALIZED;
}

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
	buf[0] = c;
	buf[1] = '\0';
	logbuff_printk (buf);
}

static void logbuff_puts (const char *s)
{
	logbuff_printk (s);
}

void logbuff_log(char *msg)
{
	if ((gd->post_log_word & LOGBUFF_INITIALIZED)) {
		logbuff_printk (msg);
	} else {
		/* Can happen only for pre-relocated errors as logging */
		/* at that stage should be disabled                    */
		puts (msg);
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

	if (strcmp(argv[1],"append") == 0) {
		/* Log concatenation of all arguments separated by spaces */
		for (i=2; i<argc; i++) {
			logbuff_printk (argv[i]);
			logbuff_putc ((i<argc-1) ? ' ' : '\n');
		}
		return 0;
	}

	switch (argc) {

	case 2:
		if (strcmp(argv[1],"show") == 0) {
			for (i=0; i < (log_size&LOGBUFF_MASK); i++) {
				s = (char *)log_buf+((log_start+i)&LOGBUFF_MASK);
				putc (*s);
			}
			return 0;
		} else if (strcmp(argv[1],"reset") == 0) {
			log_start    = 0;
			log_size     = 0;
			logged_chars = 0;
			return 0;
		} else if (strcmp(argv[1],"info") == 0) {
			printf ("Logbuffer   at  %08lx\n", (unsigned long)log_buf);
			printf ("log_start    =  %08lx\n", log_start);
			printf ("log_size     =  %08lx\n", log_size);
			printf ("logged_chars =  %08lx\n", logged_chars);
			return 0;
		}
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;

	default:
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}
}
#if defined(CONFIG_LOGBUFFER)
U_BOOT_CMD(
	log,     255,	1,	do_log,
	"log     - manipulate logbuffer\n",
	"info   - show pointer details\n"
	"log reset  - clear contents\n"
	"log show   - show contents\n"
	"log append <msg> - append <msg> to the logbuffer\n"
);
#endif	/* CONFIG_LOGBUFFER */
static int logbuff_printk(const char *line)
{
	int i;
	char *msg, *p, *buf_end;
	int line_feed;
	static signed char msg_level = -1;

	strcpy (buf + 3, line);
	i = strlen (line);
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
			log_buf[(log_start+log_size) & LOGBUFF_MASK] = *p;
			if (log_size < LOGBUFF_LEN)
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
