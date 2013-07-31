/*
 * (C) Copyright 2000
 * Murray Jensen, CSIRO-MST
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _CONFIG_COGENT_COMMON_H
#define _CONFIG_COGENT_COMMON_H

/*
 * Cogent Motherboard Capabilities
 */
#define CMA_MB_CAP_SERPAR	0x0001	/* has dual serial+parallel (16C552) */
#define CMA_MB_CAP_LCD		0x0002	/* has LCD display (HD44780) */
#define CMA_MB_CAP_FLASH	0x0004	/* has flash (E28F800B or AM29F800BB) */
#define CMA_MB_CAP_RTC		0x0008	/* has RTC+NVRAM (MK48T02) */
#define CMA_MB_CAP_ETHER	0x0010	/* has Ethernet (MB86964) */
#define CMA_MB_CAP_SLOT1	0x0020	/* has CMABus slot 1 */
#define CMA_MB_CAP_SLOT2	0x0040	/* has CMABus slot 2 */
#define CMA_MB_CAP_SLOT3	0x0080	/* has CMABus slot 3 */
#define CMA_MB_CAP_KBM		0x0100	/* has PS/2 keyboard+mouse (HT6542B) */
#define CMA_MB_CAP_SER2		0x0200	/* has 2nd dual serial (16C2552) */
#define CMA_MB_CAP_PCI		0x0400	/* has pci bridge (V360EPC) */
#define CMA_MB_CAP_PCI_EXT	0x0800	/* can access extended pci space  */
#define CMA_MB_CAP_PCI_ETHER	0x1000	/* has 10/100 ether on PCI (GD82559) */
#define CMA_MB_CAP_PCI_VIDEO	0x2000	/* has video int'face on PCI (B69000) */
#define CMA_MB_CAP_PCI_CARDBUS	0x4000	/* has Cardbus Ctlr on PCI (PD6832) */

/*
 * Cogent option sanity checking
 */

#if defined(CONFIG_MPC821) || defined(CONFIG_MPC823) || \
      defined(CONFIG_MPC850) || defined(CONFIG_MPC860)

/*
 * check a PowerPC 8xx cpu module has been selected
 */

# if defined(CONFIG_CMA286_21)

#  define COGENT_CPU_MODULE	"CMA286-21"

# elif defined(CONFIG_CMA286_60_OLD)

#  define COGENT_CPU_MODULE	"CMA286-60 (old)"

# elif defined(CONFIG_CMA286_60)

#  define COGENT_CPU_MODULE	"CMA286-60"

# elif defined(CONFIG_CMA286_60P)

#  define COGENT_CPU_MODULE	"CMA286-60P"

# elif defined(CONFIG_CMA287_21)

#  define COGENT_CPU_MODULE	"CMA287-21"

# elif defined(CONFIG_CMA287_50)

#  define COGENT_CPU_MODULE	"CMA287-50"

# else

#  error Cogent CPU Module must be a PowerPC MPC8xx module

# endif

#elif defined(CONFIG_MPC8260)

/*
 * check a PowerPC 8260 cpu module has been selected
 */

# if defined(CONFIG_CMA282)

#  define COGENT_CPU_MODULE	"CMA282"

# else

#  error Cogent CPU Module must be a PowerPC MPC8260 module

# endif

#else

# error CPU type must be PowerPC 8xx or 8260

#endif

/*
 * check a motherboard has been selected
 * define the motherboard capabilities while we're at it
 */

#if defined(CONFIG_CMA101)

# define COGENT_MOTHERBOARD	"CMA101"
# define CMA_MB_CAPS		(CMA_MB_CAP_SERPAR | CMA_MB_CAP_LCD | \
				 CMA_MB_CAP_RTC | CMA_MB_CAP_ETHER | \
				 CMA_MB_CAP_SLOT1 | CMA_MB_CAP_SLOT2 | \
				 CMA_MB_CAP_SLOT3)
# define CMA_MB_NSLOTS		3

#elif defined(CONFIG_CMA102)

# define COGENT_MOTHERBOARD	"CMA102"
# define CMA_MB_CAPS		(CMA_MB_CAP_SERPAR | CMA_MB_CAP_LCD | \
				 CMA_MB_CAP_RTC | CMA_MB_CAP_SLOT1 | \
				 CMA_MB_CAP_SLOT2 | CMA_MB_CAP_SLOT3)
# define CMA_MB_NSLOTS		3

#elif defined(CONFIG_CMA110)

# define COGENT_MOTHERBOARD	"CMA110"
# define CMA_MB_CAPS		(CMA_MB_CAP_SERPAR | CMA_MB_CAP_LCD | \
				 CMA_MB_CAP_FLASH | CMA_MB_CAP_RTC | \
				 CMA_MB_CAP_KBM | CMA_MB_CAP_PCI)
# define CMA_MB_NSLOTS		0

#elif defined(CONFIG_CMA111)

# define COGENT_MOTHERBOARD	"CMA111"
# define CMA_MB_CAPS		(CMA_MB_CAP_SERPAR | CMA_MB_CAP_LCD | \
				 CMA_MB_CAP_FLASH | CMA_MB_CAP_RTC | \
				 CMA_MB_CAP_SLOT1 | CMA_MB_CAP_KBM | \
				 CMA_MB_CAP_PCI | CMA_MB_CAP_PCI_EXT | \
				 CMA_MB_CAP_PCI_ETHER)
# define CMA_MB_NSLOTS		1

#elif defined(CONFIG_CMA120)

# define COGENT_MOTHERBOARD	"CMA120"
# define CMA_MB_CAPS		(CMA_MB_CAP_SERPAR | CMA_MB_CAP_LCD | \
				 CMA_MB_CAP_FLASH | CMA_MB_CAP_RTC | \
				 CMA_MB_CAP_SLOT1 | CMA_MB_CAP_KBM | \
				 CMA_MB_CAP_SER2 | CMA_MB_CAP_PCI | \
				 CMA_MB_CAP_PCI_EXT | CMA_MB_CAP_PCI_ETHER | \
				 CMA_MB_CAP_PCI_VIDEO | CMA_MB_CAP_PCI_CARDBUS)
# define CMA_MB_NSLOTS		1

#elif defined(CONFIG_CMA150)

# define COGENT_MOTHERBOARD	"CMA150"
# define CMA_MB_CAPS		(CMA_MB_CAP_SERPAR | CMA_MB_CAP_LCD | \
				 CMA_MB_CAP_FLASH | CMA_MB_CAP_RTC | \
				 CMA_MB_CAP_KBM)
# define CMA_MB_NSLOTS		0

#else

# error Cogent Motherboard either unsupported or undefined

#endif

/*
 * check a flash i/o module has been selected if no flash on m/b
 */

#if defined(CONFIG_CMA302)

# define COGENT_FLASH_MODULE	"CMA302"

#elif (CMA_MB_CAPS & CMA_MB_CAP_FLASH) == 0

# error Cogent Flash I/O module (e.g. CMA302) is required with this Motherboard

#endif

/*
 * some further sanity checks
 */

#if (CMA_MB_CAPS & CMA_MB_CAP_PCI) && (CMA_MB_CAPS & CMA_MB_CAP_SLOT2)
#error Cogent Sanity Check: Both Slot2 and PCI are defined
#endif

#if (CMA_MB_CAPS & CMA_MB_CAP_PCI_EXT) && !(CMA_MB_CAPS & CMA_MB_CAP_PCI)
#error Extended PCI capability defined without PCI capability
#endif

#if (CMA_MB_CAPS & CMA_MB_CAP_PCI_ETHER) && !(CMA_MB_CAPS & CMA_MB_CAP_PCI)
#error Motherboard ethernet capability defined without PCI capability
#endif

#if (CMA_MB_CAPS & CMA_MB_CAP_SER2) && !(CMA_MB_CAPS & CMA_MB_CAP_SERPAR)
#error 2nd dual serial capability defined without serial/parallel capability
#endif
#include "../board/cogent/mb.h"
#endif	/* _CONFIG_COGENT_COMMON_H */
