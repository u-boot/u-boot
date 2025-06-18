/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * K3: Architecture common definitions
 *
 * Copyright (C) 2018-2024 Texas Instruments Incorporated - https://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#include <asm/armv7_mpu.h>
#include <asm/hardware.h>
#include <mach/security.h>

/* keep ram_top in the 32-bit address space */
#define CFG_MAX_MEM_MAPPED		0x100000000

#define K3_FIREWALL_BACKGROUND_BIT (8)

struct fwl_data {
	const char *name;
	u16 fwl_id;
	u16 regions;
};

enum k3_firewall_region_type {
	K3_FIREWALL_REGION_FOREGROUND,
	K3_FIREWALL_REGION_BACKGROUND
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
int remove_fwl_region(struct fwl_data *fwl);
void remove_fwl_configs(struct fwl_data *fwl_data, size_t fwl_data_size);
int load_firmware(char *name_fw, char *name_loadaddr, u32 *loadaddr);
void k3_sysfw_print_ver(void);
void k3_dm_print_ver(void);
void spl_enable_cache(void);
void mmr_unlock(uintptr_t base, u32 partition);
bool is_rom_loaded_sysfw(struct rom_extended_boot_data *data);
enum k3_device_type get_device_type(void);
struct ti_sci_handle *get_ti_sci_handle(void);
void do_board_detect(void);
void ti_secure_image_check_binary(void **p_image, size_t *p_size);
int shutdown_mcu_r5_core1(void);

#if (IS_ENABLED(CONFIG_K3_QOS))
void setup_qos(void);
#else
static inline void setup_qos(void)
{
}
#endif
