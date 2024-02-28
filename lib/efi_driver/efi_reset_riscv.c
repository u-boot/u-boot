// SPDX-License-Identifier: GPL-2.0-or-later

#include <efi_loader.h>
#include <asm/sbi.h>

void __efi_runtime EFIAPI efi_reset_system(enum efi_reset_type reset_type,
					   efi_status_t reset_status,
					   unsigned long data_size,
					   void *reset_data)
{
	register unsigned long eid asm("a7") = SBI_EXT_SRST;
	register unsigned long fid asm("a6") = SBI_EXT_SRST_RESET;
	register unsigned long type asm("a0");
	register unsigned long reason asm("a1") = SBI_SRST_RESET_REASON_NONE;

	switch (reset_type) {
	case EFI_RESET_WARM:
		type = SBI_SRST_RESET_TYPE_WARM_REBOOT;
		break;
	case EFI_RESET_SHUTDOWN:
		type = SBI_SRST_RESET_TYPE_SHUTDOWN;
		break;
	default:
		type = SBI_SRST_RESET_TYPE_COLD_REBOOT;
		break;
	}
	asm volatile ("ecall\n"
		      : : "r" (eid), "r" (fid), "r" (type), "r" (reason));
}
