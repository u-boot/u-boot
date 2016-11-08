/*
 *
 * Security related functions for OMAP5 class devices
 *
 * (C) Copyright 2016
 * Texas Instruments, <www.ti.com>
 *
 * Daniel Allred <d-allred@ti.com>
 * Harinarayan Bhatta <harinarayan@ti.com>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <stdarg.h>

#include <asm/arch/sys_proto.h>
#include <asm/omap_common.h>
#include <asm/omap_sec_common.h>
#include <asm/spl.h>
#include <spl.h>

/* Index for signature PPA-based TI HAL APIs */
#define PPA_HAL_SERVICES_START_INDEX        (0x200)
#define PPA_SERV_HAL_SETUP_SEC_RESVD_REGION (PPA_HAL_SERVICES_START_INDEX + 25)
#define PPA_SERV_HAL_SETUP_EMIF_FW_REGION   (PPA_HAL_SERVICES_START_INDEX + 26)
#define PPA_SERV_HAL_LOCK_EMIF_FW           (PPA_HAL_SERVICES_START_INDEX + 27)

static u32 get_sec_mem_start(void)
{
	u32 sec_mem_start = CONFIG_TI_SECURE_EMIF_REGION_START;
	u32 sec_mem_size = CONFIG_TI_SECURE_EMIF_TOTAL_REGION_SIZE;
	/*
	 * Total reserved region is all contiguous with protected
	 * region coming first, followed by the non-secure region.
	 * If 0x0 start address is given, we simply put the reserved
	 * region at the end of the external DRAM.
	 */
	if (sec_mem_start == 0)
		sec_mem_start =
			(CONFIG_SYS_SDRAM_BASE +
			(omap_sdram_size() - sec_mem_size));
	return sec_mem_start;
}

int secure_emif_firewall_setup(uint8_t region_num, uint32_t start_addr,
			       uint32_t size, uint32_t access_perm,
			       uint32_t initiator_perm)
{
	int result = 1;

	/*
	 * Call PPA HAL API to do any other general firewall
	 * configuration for regions 1-6 of the EMIF firewall.
	 */
	debug("%s: regionNum = %x, startAddr = %x, size = %x", __func__,
	      region_num, start_addr, size);

	result = secure_rom_call(
			PPA_SERV_HAL_SETUP_EMIF_FW_REGION, 0, 0, 4,
			(start_addr & 0xFFFFFFF0) | (region_num & 0x0F),
			size, access_perm, initiator_perm);

	if (result != 0) {
		puts("Secure EMIF Firewall Setup failed!\n");
		debug("Return Value = %x\n", result);
	}

	return result;
}

#if	(CONFIG_TI_SECURE_EMIF_TOTAL_REGION_SIZE <  \
	CONFIG_TI_SECURE_EMIF_PROTECTED_REGION_SIZE)
#error	"TI Secure EMIF: Protected size cannot be larger than total size."
#endif
int secure_emif_reserve(void)
{
	int result = 1;
	u32 sec_mem_start = get_sec_mem_start();
	u32 sec_prot_size = CONFIG_TI_SECURE_EMIF_PROTECTED_REGION_SIZE;

	/* If there is no protected region, there is no reservation to make */
	if (sec_prot_size == 0)
		return 0;

	/*
	 * Call PPA HAL API to reserve a chunk of EMIF SDRAM
	 * for secure world use. This region should be carved out
	 * from use by any public code. EMIF firewall region 7
	 * will be used to protect this block of memory.
	 */
	result = secure_rom_call(
			PPA_SERV_HAL_SETUP_SEC_RESVD_REGION,
			0, 0, 2, sec_mem_start, sec_prot_size);

	if (result != 0) {
		puts("SDRAM Firewall: Secure memory reservation failed!\n");
		debug("Return Value = %x\n", result);
	}

	return result;
}

int secure_emif_firewall_lock(void)
{
	int result = 1;

	/*
	 * Call PPA HAL API to lock the EMIF firewall configurations.
	 * After this API is called, none of the PPA HAL APIs for
	 * configuring the EMIF firewalls will be usable again (that
	 * is, calls to those APIs will return failure and have no
	 * effect).
	 */

	result = secure_rom_call(
			PPA_SERV_HAL_LOCK_EMIF_FW,
			0, 0, 0);

	if (result != 0) {
		puts("Secure EMIF Firewall Lock failed!\n");
		debug("Return Value = %x\n", result);
	}

	return result;
}
