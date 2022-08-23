/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * K3: Architecture common definitions
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#include <asm/armv7_mpu.h>
#include <asm/hardware.h>

#define J721E  0xbb64
#define J7200  0xbb6d

struct fwl_data {
	const char *name;
	u16 fwl_id;
	u16 regions;
};

enum k3_device_type {
	K3_DEVICE_TYPE_BAD,
	K3_DEVICE_TYPE_GP,
	K3_DEVICE_TYPE_TEST,
	K3_DEVICE_TYPE_EMU,
	K3_DEVICE_TYPE_HS_FS,
	K3_DEVICE_TYPE_HS_SE,
};

void setup_k3_mpu_regions(void);
int early_console_init(void);
void disable_linefill_optimization(void);
void remove_fwl_configs(struct fwl_data *fwl_data, size_t fwl_data_size);
int load_firmware(char *name_fw, char *name_loadaddr, u32 *loadaddr);
void k3_sysfw_print_ver(void);
void spl_enable_dcache(void);
void mmr_unlock(phys_addr_t base, u32 partition);
bool is_rom_loaded_sysfw(struct rom_extended_boot_data *data);
enum k3_device_type get_device_type(void);
void ti_secure_image_post_process(void **p_image, size_t *p_size);
