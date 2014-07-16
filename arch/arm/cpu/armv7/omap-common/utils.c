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

void __weak usb_fake_mac_from_die_id(u32 *id)
{
	uint8_t device_mac[6];

	if (!getenv("usbethaddr")) {
		/*
		 * create a fake MAC address from the processor ID code.
		 * first byte is 0x02 to signify locally administered.
		 */
		device_mac[0] = 0x02;
		device_mac[1] = id[3] & 0xff;
		device_mac[2] = id[2] & 0xff;
		device_mac[3] = id[1] & 0xff;
		device_mac[4] = id[0] & 0xff;
		device_mac[5] = (id[0] >> 8) & 0xff;

		eth_setenv_enetaddr("usbethaddr", device_mac);
	}
}
