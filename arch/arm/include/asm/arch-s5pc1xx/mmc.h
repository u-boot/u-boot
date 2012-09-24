/*
 * (C) Copyright 2009 SAMSUNG Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef __ASM_ARCH_MMC_H_
#define __ASM_ARCH_MMC_H_

#define SDHCI_CONTROL2		0x80
#define SDHCI_CONTROL3		0x84
#define SDHCI_CONTROL4		0x8C

#define SDHCI_CTRL2_ENSTAASYNCCLR	(1 << 31)
#define SDHCI_CTRL2_ENCMDCNFMSK		(1 << 30)
#define SDHCI_CTRL2_CDINVRXD3		(1 << 29)
#define SDHCI_CTRL2_SLCARDOUT		(1 << 28)

#define SDHCI_CTRL2_FLTCLKSEL_MASK	(0xf << 24)
#define SDHCI_CTRL2_FLTCLKSEL_SHIFT	(24)
#define SDHCI_CTRL2_FLTCLKSEL(_x)	((_x) << 24)

#define SDHCI_CTRL2_LVLDAT_MASK		(0xff << 16)
#define SDHCI_CTRL2_LVLDAT_SHIFT	(16)
#define SDHCI_CTRL2_LVLDAT(_x)		((_x) << 16)

#define SDHCI_CTRL2_ENFBCLKTX		(1 << 15)
#define SDHCI_CTRL2_ENFBCLKRX		(1 << 14)
#define SDHCI_CTRL2_SDCDSEL		(1 << 13)
#define SDHCI_CTRL2_SDSIGPC		(1 << 12)
#define SDHCI_CTRL2_ENBUSYCHKTXSTART	(1 << 11)

#define SDHCI_CTRL2_DFCNT_MASK(_x)	((_x) << 9)
#define SDHCI_CTRL2_DFCNT_SHIFT		(9)

#define SDHCI_CTRL2_ENCLKOUTHOLD	(1 << 8)
#define SDHCI_CTRL2_RWAITMODE		(1 << 7)
#define SDHCI_CTRL2_DISBUFRD		(1 << 6)
#define SDHCI_CTRL2_SELBASECLK_MASK(_x)	((_x) << 4)
#define SDHCI_CTRL2_SELBASECLK_SHIFT	(4)
#define SDHCI_CTRL2_PWRSYNC		(1 << 3)
#define SDHCI_CTRL2_ENCLKOUTMSKCON	(1 << 1)
#define SDHCI_CTRL2_HWINITFIN		(1 << 0)

#define SDHCI_CTRL3_FCSEL3		(1 << 31)
#define SDHCI_CTRL3_FCSEL2		(1 << 23)
#define SDHCI_CTRL3_FCSEL1		(1 << 15)
#define SDHCI_CTRL3_FCSEL0		(1 << 7)

#define SDHCI_CTRL4_DRIVE_MASK(_x)	((_x) << 16)
#define SDHCI_CTRL4_DRIVE_SHIFT		(16)

int s5p_sdhci_init(u32 regbase, u32 max_clk, u32 min_clk, u32 quirks);

static inline unsigned int s5p_mmc_init(int index, int bus_width)
{
	unsigned int base = samsung_get_base_mmc() + (0x10000 * index);
	return s5p_sdhci_init(base, 52000000, 400000, index);
}
#endif
