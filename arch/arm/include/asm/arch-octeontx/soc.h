/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#ifndef __SOC_H__
#define __SOC_H__

/* Product PARTNUM */
#define CN81XX	0xA2
#define CN83XX	0xA3

/* Register defines */
#define MIO_FUS_DAT2	0x87E003001410ULL

#define otx_is_altpkg()	read_alt_pkg()
#define otx_is_soc(soc)	(read_partnum() == (soc))
#define otx_is_board(model) (!strcmp(read_board_name(), model))
#define otx_is_platform(platform) (read_platform() == (platform))

typedef enum {
	PLATFORM_HW = 0,
	PLATFORM_EMULATOR = 1,
	PLATFORM_ASIM = 3,
} platform_t;

/**
 * Register (RSL) mio_fus_dat2
 *
 * MIO Fuse Data Register 2
 */
union mio_fus_dat2 {
	u64 u;
	struct mio_fus_dat2_s {
		u64 reserved_0                       : 1;
		u64 ddf_dis                          : 1;
		u64 bgx2_dis                         : 1;
		u64 bgx3_dis                         : 1;
		u64 tim_dis                          : 1;
		u64 lmc_half                         : 1;
		u64 pem_dis                          : 3;
		u64 sata_dis                         : 4;
		u64 bgx_dis                          : 2;
		u64 ocx_dis                          : 1;
		u64 chip_id                          : 8;
		u64 reserved_24                      : 1;
		u64 trustzone_en                     : 1;
		u64 nocrypto                         : 1;
		u64 nomul                            : 1;
		u64 nodfa_cp2                        : 1;
		u64 reserved_29                      : 1;
		u64 lmc_mode32                       : 1;
		u64 reserved_31                      : 1;
		u64 raid_en                          : 1;
		u64 fus318                           : 1;
		u64 dorm_crypto                      : 1;
		u64 power_limit                      : 2;
		u64 rom_info                         : 10;
		u64 fus118                           : 1;
		u64 gbl_pwr_throttle                 : 8;
		u64 run_platform                     : 3;
		u64 reserved_59_63                   : 5;
	} s;
};

platform_t read_platform(void);
u8 read_partnum(void);
const char *read_board_name(void);
bool read_alt_pkg(void);

#endif /* __SOC_H */
