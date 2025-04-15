/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef _NPCM_GMAC_H_
#define _NPCM_GMAC_H_

/* PCS registers */
#define PCS_BA			0xF0780000
#define PCS_IND_AC		0x1FE
#define SR_MII_MMD		0x3E0000
#define SR_MII_MMD_CTRL		0x0
#define SR_MII_MMD_STS		0x2
#define VR_MII_MMD		0x3F0000
#define VR_MII_MMD_CTRL1	0x0
#define VR_MII_MMD_AN_CTRL	0x2

#define LINK_UP_TIMEOUT		(3 * CONFIG_SYS_HZ)

#endif
