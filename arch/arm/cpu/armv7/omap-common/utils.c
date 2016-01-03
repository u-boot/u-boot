/*
 * Copyright 2011 Linaro Limited
 * Aneesh V <aneesh@ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <asm/arch/sys_proto.h>
static void do_cancel_out(u32 *num, u32 *den, u32 factor)
{
	while (1) {
		if (((*num)/factor*factor == (*num)) &&
		   ((*den)/factor*factor == (*den))) {
			(*num) /= factor;
			(*den) /= factor;
		} else
			break;
	}
}

/*
 * Cancel out the denominator and numerator of a fraction
 * to get smaller numerator and denominator.
 */
void cancel_out(u32 *num, u32 *den, u32 den_limit)
{
	do_cancel_out(num, den, 2);
	do_cancel_out(num, den, 3);
	do_cancel_out(num, den, 5);
	do_cancel_out(num, den, 7);
	do_cancel_out(num, den, 11);
	do_cancel_out(num, den, 13);
	do_cancel_out(num, den, 17);
	while ((*den) > den_limit) {
		*num /= 2;
		/*
		 * Round up the denominator so that the final fraction
		 * (num/den) is always <= the desired value
		 */
		*den = (*den + 1) / 2;
	}
}

__weak void omap_die_id(unsigned int *die_id)
{
	die_id[0] = die_id[1] = die_id[2] = die_id[3] = 0;
}

void omap_die_id_serial(void)
{
	unsigned int die_id[4] = { 0 };
	char serial_string[17] = { 0 };

	omap_die_id((unsigned int *)&die_id);

	if (!getenv("serial#")) {
		snprintf(serial_string, sizeof(serial_string),
			"%08x%08x", die_id[0], die_id[3]);

		setenv("serial#", serial_string);
	}
}

void omap_die_id_get_board_serial(struct tag_serialnr *serialnr)
{
	char *serial_string;
	unsigned long long serial;

	serial_string = getenv("serial#");

	if (serial_string) {
		serial = simple_strtoull(serial_string, NULL, 16);

		serialnr->high = (unsigned int) (serial >> 32);
		serialnr->low = (unsigned int) (serial & 0xffffffff);
	} else {
		serialnr->high = 0;
		serialnr->low = 0;
	}
}

void omap_die_id_usbethaddr(void)
{
	unsigned int die_id[4] = { 0 };
	unsigned char mac[6] = { 0 };

	omap_die_id((unsigned int *)&die_id);

	if (!getenv("usbethaddr")) {
		/*
		 * Create a fake MAC address from the processor ID code.
		 * First byte is 0x02 to signify locally administered.
		 */
		mac[0] = 0x02;
		mac[1] = die_id[3] & 0xff;
		mac[2] = die_id[2] & 0xff;
		mac[3] = die_id[1] & 0xff;
		mac[4] = die_id[0] & 0xff;
		mac[5] = (die_id[0] >> 8) & 0xff;

		eth_setenv_enetaddr("usbethaddr", mac);
	}
}

void omap_die_id_display(void)
{
	unsigned int die_id[4] = { 0 };

	omap_die_id(die_id);

	printf("OMAP die ID: %08x%08x%08x%08x\n", die_id[0], die_id[1],
		die_id[2], die_id[3]);
}
