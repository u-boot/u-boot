// SPDX-License-Identifier: GPL-2.0+

#ifndef TURRIS_ATSHA_OTP_H
#define TURRIS_ATSHA_OTP_H

int turris_atsha_otp_init_mac_addresses(int first_idx);
int turris_atsha_otp_get_serial_number(u32 *version_num, u32 *serial_num);

#endif
