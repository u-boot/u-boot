/*
 * (C) Copyright 2002-2007
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
#include <stdio_dev.h>
#include <post.h>
#include <logbuff.h>

DECLARE_GLOBAL_DATA_PTR;

/* Local prototypes */
static void logbuff_putc(const char c);
static void logbuff_puts(const char *s);
static int logbuff_printk(const char *line);

static char buf[1024];

/* This combination will not print messages with the default loglevel */
static unsigned console_loglevel = 3;
static unsigned default_message_loglevel = 4;
static unsigned log_version = 1;
#ifdef CONFIG_ALT_LB_ADDR
static volatile logbuff_t *log;
#else
static logbuff_t *log;
#endif
static char *lbuf;

unsigned long __logbuffer_base(void)
{
	return CONFIG_SYS_SDRAM_BASE + gd->ram_size - LOGBUFF_LEN;
}
unsigned long logbuffer_base(void)
__attribute__((weak, alias("__logbuffer_base")));

void logbuff_init_ptrs(void)
{
	unsigned long tag, post_word;
	char *s;

#ifdef CONFIG_ALT_LB_ADDR
	log = (logbuff_t *)CONFIG_ALT_LH_ADDR;
	lbuf = (char *)CONFIG_ALT_LB_ADDR;
#else
	log = (logbuff_t *)(logbuffer_base()) - 1;
	lbuf = (char *)log->buf;
#endif

	/* Set up log version */
	if ((s = getenv ("logversion")) != NULL)
		log_version = (int)simple_strtoul(s, NULL, 10);

	if (log_version == 2)
		tag = log->v2.tag;
	else
		tag = log->v1.tag;
	post_word = post_word_load();
#ifdef CONFIG_POST
	/* The post routines have setup the word so we can simply test it */
	if (tag != LOGBUFF_MAGIC || (post_word & POST_COLDBOOT))
		logbuff_reset();
#else
	/* No post routines, so we do our own checking                    */
	if (tag != LOGBUFF_MAGIC || post_word != LOGBUFF_MAGIC) {
		logbuff_reset ();
		post_word_store (LOGBUFF_MAGIC);
	}
#endif
	if (log_version == 2 && (long)log->v2.start > (long)log->v2.con)
		log->v2.start = log->v2.con;

	/* Initialize default loglevel if present */
	if ((s = getenv ("loglevel")) != NULL)
		console_loglevel = (int)simple_strtoul(s, NULL, 10);

	gd->flags |= GD_FLG_LOGINIT;
}

void logbuff_reset(void)
{
#ifndef CONFIG_ALT_LB_ADDR
	memset(log, 0, sizeof(logbuff_t));
#endif
	if (log_version == 2) {
		log->v2.tag = LOGBUFF_MAGIC;
#ifdef CONFIG_ALT_LB_ADDR
		log->v2.start = 0;
		log->v2.con = 0;
		log->v2.end = 0;
		log->v2.chars = 0;
#endif
	} else {
		log->v1.tag = LOGBUFF_MAGIC;
#ifdef CONFIG_ALT_LB_ADDR
		log->v1.dummy = 0;
		log->v1.start = 0;
		log->v1.size = 0;
		log->v1.chars = 0;
#endif
	}
}

int drv_logbuff_init(void)
{
	struct stdio_dev logdev;
	int rc;

	/* Device initialization */
	memset (&logdev, 0, sizeof (logdev));

	strcpy (logdev.name, "logbuff");
	logdev.ext   = 0;			/* No extensions */
	logdev.flags = DEV_FLAGS_OUTPUT;	/* Output only */
	logdev.putc  = logbuff_putc;		/* 'putc' function */
	logdev.puts  = logbuff_puts;		/* 'puts' function */

	rc = stdio_register(&logdev);

	return (rc == 0) ? 1 : rc;
}

static void logbuff_putc(const char c)
{
	char buf[2];
	buf[0] = c;
	buf[1] = '\0';
	logbuff_printk(buf);
}

static void logbuff_puts(const char *s)
{
	logbuff_printk (s);
}

void logbuff_log(char *msg)
{
	if ((gd->flags & GD_FLG_LOGINIT)) {
		logbuff_printk(msg);
	} else {
		/*
		 * Can happen only for pre-relocated errors as logging
		 * at that stage should be disabled
		 */
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
int do_log(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *s;
	unsigned long i, start, size;

	if (strcmp(argv[1], "append") == 0) {
		/* Log concatenation of all arguments separated by spaces */
		for (i = 2; i < argc; i++) {
			logbuff_printk(argv[i]);
			logbuff_putc((i < argc - 1) ? ' ' : '\n');
		}
		return 0;
	}

	switch (argc) {

	case 2:
		if (strcmp(argv[1], "show") == 0) {
			if (log_version == 2) {
				start = log->v2.start;
				size = log->v2.end - log->v2.start;
			} else {
				start = log->v1.start;
				size = log->v1.size;
			}
			if (size > LOGBUFF_LEN)
				size = LOGBUFF_LEN;
			for (i = 0; i < size; i++) {
				s = lbuf + ((start + i) & LOGBUFF_MASK);
				putc(*s);
			}
			return 0;
		} else if (strcmp(argv[1], "reset") == 0) {
			logbuff_reset();
			return 0;
		} else if (strcmp(argv[1], "info") == 0) {
			printf("Logbuffer   at  %08lx\n", (unsigned long)lbuf);
			if (log_version == 2) {
				printf("log_start    =  %08lx\n",
					log->v2.start);
				printf("log_end      =  %08lx\n", log->v2.end);
				printf("log_con      =  %08lx\n", log->v2.con);
				printf("logged_chars =  %08lx\n",
					log->v2.chars);
			}
			else {
				printf("log_start    =  %08lx\n",
					log->v1.start);
				printf("log_size     =  %08lx\n",
					log->v1.size);
				printf("logged_chars =  %08lx\n",
					log->v1.chars);
			}
			return 0;
		}
		return CMD_RET_USAGE;

	default:
		return CMD_RET_USAGE;
	}
}

U_BOOT_CMD(
	log,     255,	1,	do_log,
	"manipulate logbuffer",
	"info   - show pointer details\n"
	"log reset  - clear contents\n"
	"log show   - show contents\n"
	"log append <msg> - append <msg> to the logbuffer"
);

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
			} else {
				msg += 3;
			}
			msg_level = p[1] - '0';
		}
		line_feed = 0;
		for (; p < buf_end; p++) {
			if (log_version == 2) {
				lbuf[log->v2.end & LOGBUFF_MASK] = *p;
				log->v2.end++;
				if (log->v2.end - log->v2.start > LOGBUFF_LEN)
					log->v2.start++;
				log->v2.chars++;
			} else {
				lbuf[(log->v1.start + log->v1.size) &
					 LOGBUFF_MASK] = *p;
				if (log->v1.size < LOGBUFF_LEN)
					log->v1.size++;
				else
					log->v1.start++;
				log->v1.chars++;
			}
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
