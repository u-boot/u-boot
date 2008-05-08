/*
 * (C) Copyright 2008 Ilya Yanok, EmCraft Systems, yanok@emcraft.com
 *
 * Developed for DENX Software Engineering GmbH
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

/*
 * This test attempts to verify on-chip memory (OCM). Result is written
 * to the scratch register and if test succeed it won't be run till next
 * power on.
 */

#include <post.h>

#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#define OCM_TEST_PATTERN1	0x55555555
#define OCM_TEST_PATTERN2	0xAAAAAAAA

#if CONFIG_POST & CFG_POST_OCM

static uint ocm_status_read(void)
{
	return in_be32((void *)CFG_OCM_STATUS_ADDR) &
		CFG_OCM_STATUS_MASK;
}

static void ocm_status_write(uint value)
{
	out_be32((void *)CFG_OCM_STATUS_ADDR, value |
		(in_be32((void *)CFG_OCM_STATUS_ADDR) &
			~CFG_OCM_STATUS_MASK));
}

static inline int ocm_test_word(uint value, uint *address)
{
	uint read_value;

	*address = value;
	sync();
	read_value = *address;

	return (read_value != value);
}

int ocm_post_test(int flags)
{
	uint   old_value;
	int    ret = 0;
	uint  *address = (uint*)CFG_OCM_BASE;

	if (ocm_status_read() == CFG_OCM_STATUS_OK)
		return 0;
	for (; address < (uint*)(CFG_OCM_BASE + CFG_OCM_SIZE); address++) {
		old_value = *address;
		if (ocm_test_word(OCM_TEST_PATTERN1, address) ||
				ocm_test_word(OCM_TEST_PATTERN2, address)) {
			ret = 1;
			*address = old_value;
			printf("OCM POST failed at %p!\n", address);
			break;
		}
		*address = old_value;
	}
	ocm_status_write(ret ? CFG_OCM_STATUS_FAIL : CFG_OCM_STATUS_OK);
	return ret;
}
#endif /* CONFIG_POST & CFG_POST_OCM */
