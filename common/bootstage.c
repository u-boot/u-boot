/*
 * Copyright (c) 2011, Google Inc. All rights reserved.
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
 * This module records the progress of boot and arbitrary commands, and
 * permits accurate timestamping of each.
 *
 * TBD: Pass timings to kernel in the FDT
 */

#include <common.h>
#include <libfdt.h>

DECLARE_GLOBAL_DATA_PTR;

struct bootstage_record {
	ulong time_us;
	uint32_t start_us;
	const char *name;
	int flags;		/* see enum bootstage_flags */
	enum bootstage_id id;
};

static struct bootstage_record record[BOOTSTAGE_ID_COUNT] = { {1} };
static int next_id = BOOTSTAGE_ID_USER;

ulong bootstage_add_record(enum bootstage_id id, const char *name,
			   int flags, ulong mark)
{
	struct bootstage_record *rec;

	if (flags & BOOTSTAGEF_ALLOC)
		id = next_id++;

	if (id < BOOTSTAGE_ID_COUNT) {
		rec = &record[id];

		/* Only record the first event for each */
		if (!rec->time_us) {
			rec->time_us = mark;
			rec->name = name;
			rec->flags = flags;
			rec->id = id;
		}
	}

	/* Tell the board about this progress */
	show_boot_progress(flags & BOOTSTAGEF_ERROR ? -id : id);
	return mark;
}


ulong bootstage_mark(enum bootstage_id id)
{
	return bootstage_add_record(id, NULL, 0, timer_get_boot_us());
}

ulong bootstage_error(enum bootstage_id id)
{
	return bootstage_add_record(id, NULL, BOOTSTAGEF_ERROR,
				    timer_get_boot_us());
}

ulong bootstage_mark_name(enum bootstage_id id, const char *name)
{
	int flags = 0;

	if (id == BOOTSTAGE_ID_ALLOC)
		flags = BOOTSTAGEF_ALLOC;
	return bootstage_add_record(id, name, flags, timer_get_boot_us());
}

uint32_t bootstage_start(enum bootstage_id id, const char *name)
{
	struct bootstage_record *rec = &record[id];

	rec->start_us = timer_get_boot_us();
	rec->name = name;
	return rec->start_us;
}

uint32_t bootstage_accum(enum bootstage_id id)
{
	struct bootstage_record *rec = &record[id];
	uint32_t duration;

	duration = (uint32_t)timer_get_boot_us() - rec->start_us;
	rec->time_us += duration;
	return duration;
}

static void print_time(unsigned long us_time)
{
	char str[15], *s;
	int grab = 3;

	/* We don't seem to have %'d in U-Boot */
	sprintf(str, "%12lu", us_time);
	for (s = str + 3; *s; s += grab) {
		if (s != str + 3)
			putc(s[-1] != ' ' ? ',' : ' ');
		printf("%.*s", grab, s);
		grab = 3;
	}
}

/**
 * Get a record name as a printable string
 *
 * @param buf	Buffer to put name if needed
 * @param len	Length of buffer
 * @param rec	Boot stage record to get the name from
 * @return pointer to name, either from the record or pointing to buf.
 */
static const char *get_record_name(char *buf, int len,
				   struct bootstage_record *rec)
{
	if (rec->name)
		return rec->name;
	else if (rec->id >= BOOTSTAGE_ID_USER)
		snprintf(buf, len, "user_%d", rec->id - BOOTSTAGE_ID_USER);
	else
		snprintf(buf, len, "id=%d", rec->id);

	return buf;
}

static uint32_t print_time_record(enum bootstage_id id,
			struct bootstage_record *rec, uint32_t prev)
{
	char buf[20];

	if (prev == -1U) {
		printf("%11s", "");
		print_time(rec->time_us);
	} else {
		print_time(rec->time_us);
		print_time(rec->time_us - prev);
	}
	printf("  %s\n", get_record_name(buf, sizeof(buf), rec));

	return rec->time_us;
}

static int h_compare_record(const void *r1, const void *r2)
{
	const struct bootstage_record *rec1 = r1, *rec2 = r2;

	return rec1->time_us > rec2->time_us ? 1 : -1;
}

#ifdef CONFIG_OF_LIBFDT
/**
 * Add all bootstage timings to a device tree.
 *
 * @param blob	Device tree blob
 * @return 0 on success, != 0 on failure.
 */
static int add_bootstages_devicetree(struct fdt_header *blob)
{
	int bootstage;
	char buf[20];
	int id;
	int i;

	if (!blob)
		return 0;

	/*
	 * Create the node for bootstage.
	 * The address of flat device tree is set up by the command bootm.
	 */
	bootstage = fdt_add_subnode(blob, 0, "bootstage");
	if (bootstage < 0)
		return -1;

	/*
	 * Insert the timings to the device tree in the reverse order so
	 * that they can be printed in the Linux kernel in the right order.
	 */
	for (id = BOOTSTAGE_ID_COUNT - 1, i = 0; id >= 0; id--, i++) {
		struct bootstage_record *rec = &record[id];
		int node;

		if (id != BOOTSTAGE_ID_AWAKE && rec->time_us == 0)
			continue;

		node = fdt_add_subnode(blob, bootstage, simple_itoa(i));
		if (node < 0)
			break;

		/* add properties to the node. */
		if (fdt_setprop_string(blob, node, "name",
				get_record_name(buf, sizeof(buf), rec)))
			return -1;

		/* Check if this is a 'mark' or 'accum' record */
		if (fdt_setprop_cell(blob, node,
				rec->start_us ? "accum" : "mark",
				rec->time_us))
			return -1;
	}

	return 0;
}

int bootstage_fdt_add_report(void)
{
	if (add_bootstages_devicetree(working_fdt))
		puts("bootstage: Failed to add to device tree\n");

	return 0;
}
#endif

void bootstage_report(void)
{
	struct bootstage_record *rec = record;
	int id;
	uint32_t prev;

	puts("Timer summary in microseconds:\n");
	printf("%11s%11s  %s\n", "Mark", "Elapsed", "Stage");

	/* Fake the first record - we could get it from early boot */
	rec->name = "reset";
	rec->time_us = 0;
	prev = print_time_record(BOOTSTAGE_ID_AWAKE, rec, 0);

	/* Sort records by increasing time */
	qsort(record, ARRAY_SIZE(record), sizeof(*rec), h_compare_record);

	for (id = 0; id < BOOTSTAGE_ID_COUNT; id++, rec++) {
		if (rec->time_us != 0 && !rec->start_us)
			prev = print_time_record(rec->id, rec, prev);
	}
	if (next_id > BOOTSTAGE_ID_COUNT)
		printf("(Overflowed internal boot id table by %d entries\n"
			"- please increase CONFIG_BOOTSTAGE_USER_COUNT\n",
		       next_id - BOOTSTAGE_ID_COUNT);

	puts("\nAccumulated time:\n");
	for (id = 0, rec = record; id < BOOTSTAGE_ID_COUNT; id++, rec++) {
		if (rec->start_us)
			prev = print_time_record(id, rec, -1);
	}
}

ulong __timer_get_boot_us(void)
{
	static ulong base_time;

	/*
	 * We can't implement this properly. Return 0 on the first call and
	 * larger values after that.
	 */
	if (base_time)
		return get_timer(base_time) * 1000;
	base_time = get_timer(0);
	return 0;
}

ulong timer_get_boot_us(void)
	__attribute__((weak, alias("__timer_get_boot_us")));
