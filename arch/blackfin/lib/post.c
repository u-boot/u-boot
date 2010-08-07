/*
 * (C) Copyright 2002
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
#include <stdio_dev.h>
#include <watchdog.h>
#include <post.h>

#ifdef CONFIG_LOGBUFFER
#include <logbuff.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#define POST_MAX_NUMBER		32

#define BOOTMODE_MAGIC	0xDEAD0000

int post_init_f(void)
{
	int res = 0;
	unsigned int i;

	for (i = 0; i < post_list_size; i++) {
		struct post_test *test = post_list + i;

		if (test->init_f && test->init_f()) {
			res = -1;
		}
	}

	gd->post_init_f_time = post_time_ms(0);
	if (!gd->post_init_f_time) {
		printf
		    ("post/post.c: post_time_ms seems not to be implemented\n");
	}

	return res;
}

void post_bootmode_init(void)
{
	int bootmode = post_bootmode_get(0);
	int newword;

	if (post_hotkeys_pressed() && !(bootmode & POST_POWERTEST)) {
		newword = BOOTMODE_MAGIC | POST_SLOWTEST;
	} else if (bootmode == 0) {
		newword = BOOTMODE_MAGIC | POST_POWERON;
	} else if (bootmode == POST_POWERON || bootmode == POST_SLOWTEST) {
		newword = BOOTMODE_MAGIC | POST_NORMAL;
	} else {
		/* Use old value */
		newword = post_word_load() & ~POST_COLDBOOT;
	}

	if (bootmode == 0) {
		/* We are booting after power-on */
		newword |= POST_COLDBOOT;
	}

	post_word_store(newword);

	/* Reset activity record */
	gd->post_log_word = 0;
}

int post_bootmode_get(unsigned int *last_test)
{
	unsigned long word = post_word_load();
	int bootmode;

	if ((word & 0xFFFF0000) != BOOTMODE_MAGIC) {
		return 0;
	}

	bootmode = word & 0x7F;

	if (last_test && (bootmode & POST_POWERTEST)) {
		*last_test = (word >> 8) & 0xFF;
	}

	return bootmode;
}

/* POST tests run before relocation only mark status bits .... */
static void post_log_mark_start(unsigned long testid)
{
	gd->post_log_word |= (testid) << 16;
}

static void post_log_mark_succ(unsigned long testid)
{
	gd->post_log_word |= testid;
}

/* ... and the messages are output once we are relocated */
void post_output_backlog(void)
{
	int j;

	for (j = 0; j < post_list_size; j++) {
		if (gd->post_log_word & (post_list[j].testid << 16)) {
			post_log("POST %s ", post_list[j].cmd);
			if (gd->post_log_word & post_list[j].testid)
				post_log("PASSED\n");
			else {
				post_log("FAILED\n");
				show_boot_progress (-31);
			}
		}
	}
}

static void post_bootmode_test_on(unsigned int last_test)
{
	unsigned long word = post_word_load();

	word |= POST_POWERTEST;

	word |= (last_test & 0xFF) << 8;

	post_word_store(word);
}

static void post_bootmode_test_off(void)
{
	unsigned long word = post_word_load();

	word &= ~POST_POWERTEST;

	post_word_store(word);
}

static void post_get_flags(int *test_flags)
{
	int flag[] = { POST_POWERON, POST_NORMAL, POST_SLOWTEST };
	char *var[] = { "post_poweron", "post_normal", "post_slowtest" };
	int varnum = sizeof(var) / sizeof(var[0]);
	char list[128];		/* long enough for POST list */
	char *name;
	char *s;
	int last;
	int i, j;

	for (j = 0; j < post_list_size; j++) {
		test_flags[j] = post_list[j].flags;
	}

	for (i = 0; i < varnum; i++) {
		if (getenv_f(var[i], list, sizeof(list)) <= 0)
			continue;

		for (j = 0; j < post_list_size; j++) {
			test_flags[j] &= ~flag[i];
		}

		last = 0;
		name = list;
		while (!last) {
			while (*name && *name == ' ')
				name++;
			if (*name == 0)
				break;
			s = name + 1;
			while (*s && *s != ' ')
				s++;
			if (*s == 0)
				last = 1;
			else
				*s = 0;

			for (j = 0; j < post_list_size; j++) {
				if (strcmp(post_list[j].cmd, name) == 0) {
					test_flags[j] |= flag[i];
					break;
				}
			}

			if (j == post_list_size) {
				printf("No such test: %s\n", name);
			}

			name = s + 1;
		}
	}

	for (j = 0; j < post_list_size; j++) {
		if (test_flags[j] & POST_POWERON) {
			test_flags[j] |= POST_SLOWTEST;
		}
	}
}

static int post_run_single(struct post_test *test,
			   int test_flags, int flags, unsigned int i)
{
	if ((flags & test_flags & POST_ALWAYS) &&
	    (flags & test_flags & POST_MEM)) {
		WATCHDOG_RESET();

		if (!(flags & POST_REBOOT)) {
			if ((test_flags & POST_REBOOT)
			    && !(flags & POST_MANUAL)) {
				post_bootmode_test_on(i);
			}

			if (test_flags & POST_PREREL)
				post_log_mark_start(test->testid);
			else
				post_log("POST %s ", test->cmd);
		}

		if (test_flags & POST_PREREL) {
			if ((*test->test) (flags) == 0)
				post_log_mark_succ(test->testid);
		} else {
			if ((*test->test) (flags) != 0) {
				post_log("FAILED\n");
				show_boot_progress (-32);
			} else
				post_log("PASSED\n");
		}

		if ((test_flags & POST_REBOOT) && !(flags & POST_MANUAL)) {
			post_bootmode_test_off();
		}

		return 0;
	} else {
		return -1;
	}
}

int post_run(char *name, int flags)
{
	unsigned int i;
	int test_flags[POST_MAX_NUMBER];

	post_get_flags(test_flags);

	if (name == NULL) {
		unsigned int last;

		if (post_bootmode_get(&last) & POST_POWERTEST) {
			if (last < post_list_size &&
			    (flags & test_flags[last] & POST_ALWAYS) &&
			    (flags & test_flags[last] & POST_MEM)) {

				post_run_single(post_list + last,
						test_flags[last],
						flags | POST_REBOOT, last);

				for (i = last + 1; i < post_list_size; i++) {
					post_run_single(post_list + i,
							test_flags[i],
							flags, i);
				}
			}
		} else {
			for (i = 0; i < post_list_size; i++) {
				post_run_single(post_list + i,
						test_flags[i], flags, i);
			}
		}

		return 0;
	} else {
		for (i = 0; i < post_list_size; i++) {
			if (strcmp(post_list[i].cmd, name) == 0)
				break;
		}

		if (i < post_list_size) {
			return post_run_single(post_list + i,
					       test_flags[i], flags, i);
		} else {
			return -1;
		}
	}
}

static int post_info_single(struct post_test *test, int full)
{
	if (test->flags & POST_MANUAL) {
		if (full)
			printf("%s - %s\n"
			       "  %s\n", test->cmd, test->name, test->desc);
		else
			printf("  %-15s - %s\n", test->cmd, test->name);

		return 0;
	} else {
		return -1;
	}
}

int post_info(char *name)
{
	unsigned int i;

	if (name == NULL) {
		for (i = 0; i < post_list_size; i++) {
			post_info_single(post_list + i, 0);
		}

		return 0;
	} else {
		for (i = 0; i < post_list_size; i++) {
			if (strcmp(post_list[i].cmd, name) == 0)
				break;
		}

		if (i < post_list_size) {
			return post_info_single(post_list + i, 1);
		} else {
			return -1;
		}
	}
}

int post_log(char *format, ...)
{
	va_list args;
	uint i;
	char printbuffer[CONFIG_SYS_PBSIZE];

	va_start(args, format);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vsprintf(printbuffer, format, args);
	va_end(args);

#ifdef CONFIG_LOGBUFFER
	/* Send to the logbuffer */
	logbuff_log(printbuffer);
#else
	/* Send to the stdout file */
	puts(printbuffer);
#endif

	return 0;
}

void post_reloc(void)
{
	unsigned int i;

	/*
	 * We have to relocate the test table manually
	 */
	for (i = 0; i < post_list_size; i++) {
		ulong addr;
		struct post_test *test = post_list + i;

		if (test->name) {
			addr = (ulong) (test->name) + gd->reloc_off;
			test->name = (char *)addr;
		}

		if (test->cmd) {
			addr = (ulong) (test->cmd) + gd->reloc_off;
			test->cmd = (char *)addr;
		}

		if (test->desc) {
			addr = (ulong) (test->desc) + gd->reloc_off;
			test->desc = (char *)addr;
		}

		if (test->test) {
			addr = (ulong) (test->test) + gd->reloc_off;
			test->test = (int (*)(int flags))addr;
		}

		if (test->init_f) {
			addr = (ulong) (test->init_f) + gd->reloc_off;
			test->init_f = (int (*)(void))addr;
		}

		if (test->reloc) {
			addr = (ulong) (test->reloc) + gd->reloc_off;
			test->reloc = (void (*)(void))addr;

			test->reloc();
		}
	}
}

/*
 * Some tests (e.g. SYSMON) need the time when post_init_f started,
 * but we cannot use get_timer() at this point.
 *
 * On PowerPC we implement it using the timebase register.
 */
unsigned long post_time_ms(unsigned long base)
{
	return (unsigned long)get_ticks() / (get_tbclk() / CONFIG_SYS_HZ) - base;
}
