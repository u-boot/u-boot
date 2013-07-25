/*
 * (C) Copyright 2008 Ilya Yanok, EmCraft Systems, yanok@emcraft.com
 *
 * Developed for DENX Software Engineering GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
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

#if CONFIG_POST & CONFIG_SYS_POST_OCM

static uint ocm_status_read(void)
{
	return in_be32((void *)CONFIG_SYS_OCM_STATUS_ADDR) &
		CONFIG_SYS_OCM_STATUS_MASK;
}

static void ocm_status_write(uint value)
{
	out_be32((void *)CONFIG_SYS_OCM_STATUS_ADDR, value |
		(in_be32((void *)CONFIG_SYS_OCM_STATUS_ADDR) &
			~CONFIG_SYS_OCM_STATUS_MASK));
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
	uint  *address = (uint*)CONFIG_SYS_OCM_BASE;

	if (ocm_status_read() == CONFIG_SYS_OCM_STATUS_OK)
		return 0;
	for (; address < (uint*)(CONFIG_SYS_OCM_BASE + CONFIG_SYS_OCM_SIZE); address++) {
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
	ocm_status_write(ret ? CONFIG_SYS_OCM_STATUS_FAIL : CONFIG_SYS_OCM_STATUS_OK);
	return ret;
}
#endif /* CONFIG_POST & CONFIG_SYS_POST_OCM */
