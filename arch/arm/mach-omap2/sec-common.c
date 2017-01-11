/*
 *
 * Common security related functions for OMAP devices
 *
 * (C) Copyright 2016
 * Texas Instruments, <www.ti.com>
 *
 * Daniel Allred <d-allred@ti.com>
 * Andreas Dannenberg <dannenberg@ti.com>
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

/* Index for signature verify ROM API */
#ifdef CONFIG_AM33XX
#define API_HAL_KM_VERIFYCERTIFICATESIGNATURE_INDEX	(0x0000000C)
#else
#define API_HAL_KM_VERIFYCERTIFICATESIGNATURE_INDEX	(0x0000000E)
#endif

static uint32_t secure_rom_call_args[5] __aligned(ARCH_DMA_MINALIGN);

u32 secure_rom_call(u32 service, u32 proc_id, u32 flag, ...)
{
	int i;
	u32 num_args;
	va_list ap;

	va_start(ap, flag);

	num_args = va_arg(ap, u32);

	if (num_args > 4)
		return 1;

	/* Copy args to aligned args structure */
	for (i = 0; i < num_args; i++)
		secure_rom_call_args[i + 1] = va_arg(ap, u32);

	secure_rom_call_args[0] = num_args;

	va_end(ap);

	/* if data cache is enabled, flush the aligned args structure */
	flush_dcache_range(
		(unsigned int)&secure_rom_call_args[0],
		(unsigned int)&secure_rom_call_args[0] +
		roundup(sizeof(secure_rom_call_args), ARCH_DMA_MINALIGN));

	return omap_smc_sec(service, proc_id, flag, secure_rom_call_args);
}

static u32 find_sig_start(char *image, size_t size)
{
	char *image_end = image + size;
	char *sig_start_magic = "CERT_";
	int magic_str_len = strlen(sig_start_magic);
	char *ch;

	while (--image_end > image) {
		if (*image_end == '_') {
			ch = image_end - magic_str_len + 1;
			if (!strncmp(ch, sig_start_magic, magic_str_len))
				return (u32)ch;
		}
	}
	return 0;
}

int secure_boot_verify_image(void **image, size_t *size)
{
	int result = 1;
	u32 cert_addr, sig_addr;
	size_t cert_size;

	/* Perform cache writeback on input buffer */
	flush_dcache_range(
		(u32)*image,
		(u32)*image + roundup(*size, ARCH_DMA_MINALIGN));

	cert_addr = (uint32_t)*image;
	sig_addr = find_sig_start((char *)*image, *size);

	if (sig_addr == 0) {
		printf("No signature found in image!\n");
		result = 1;
		goto auth_exit;
	}

	*size = sig_addr - cert_addr;	/* Subtract out the signature size */
	cert_size = *size;

	/* Check if image load address is 32-bit aligned */
	if (!IS_ALIGNED(cert_addr, 4)) {
		printf("Image is not 4-byte aligned!\n");
		result = 1;
		goto auth_exit;
	}

	/* Image size also should be multiple of 4 */
	if (!IS_ALIGNED(cert_size, 4)) {
		printf("Image size is not 4-byte aligned!\n");
		result = 1;
		goto auth_exit;
	}

	/* Call ROM HAL API to verify certificate signature */
	debug("%s: load_addr = %x, size = %x, sig_addr = %x\n", __func__,
	      cert_addr, cert_size, sig_addr);

	result = secure_rom_call(
		API_HAL_KM_VERIFYCERTIFICATESIGNATURE_INDEX, 0, 0,
		4, cert_addr, cert_size, sig_addr, 0xFFFFFFFF);
auth_exit:
	if (result != 0) {
		printf("Authentication failed!\n");
		printf("Return Value = %08X\n", result);
		hang();
	}

	/*
	 * Output notification of successful authentication as well the name of
	 * the signing certificate used to re-assure the user that the secure
	 * code is being processed as expected. However suppress any such log
	 * output in case of building for SPL and booting via YMODEM. This is
	 * done to avoid disturbing the YMODEM serial protocol transactions.
	 */
	if (!(IS_ENABLED(CONFIG_SPL_BUILD) &&
	      IS_ENABLED(CONFIG_SPL_YMODEM_SUPPORT) &&
	      spl_boot_device() == BOOT_DEVICE_UART))
		printf("Authentication passed: %s\n", (char *)sig_addr);

	return result;
}
