/* Copyright MontaVista Software Incorporated, 2000 */
#include <config.h>
	.section .resetvec,"ax"
#if defined(CONFIG_440)
	b _start_440
#else
#if defined(CONFIG_BOOT_PCI) && (defined(CONFIG_TARGET_MIP405) \
				 || defined(CONFIG_TARGET_MIP405T))
	b _start_pci
#else
	b _start
#endif
#endif
