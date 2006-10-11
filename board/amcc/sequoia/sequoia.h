/*
 * (C) Copyright 2006
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * (C) Copyright 2006
 * Jacqueline Pira-Ferriol, AMCC/IBM, jpira-ferriol@fr.ibm.com
 * Alain Saurel,            AMCC/IBM, alain.saurel@fr.ibm.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */


/*----------------------------------------------------------------------------+
  | EBC Configuration Register - EBC0_CFG
  +----------------------------------------------------------------------------*/
/* External Bus Three-State Control */
#define EBC0_CFG_EBTC_DRIVEN	    0x80000000
/* Device-Paced Time-out Disable */
#define EBC0_CFG_PTD_ENABLED	    0x00000000
/* Ready Timeout Count */
#define EBC0_CFG_RTC_MASK	    0x38000000
#define EBC0_CFG_RTC_16PERCLK	    0x00000000
#define EBC0_CFG_RTC_32PERCLK	    0x08000000
#define EBC0_CFG_RTC_64PERCLK	    0x10000000
#define EBC0_CFG_RTC_128PERCLK	    0x18000000
#define EBC0_CFG_RTC_256PERCLK	    0x20000000
#define EBC0_CFG_RTC_512PERCLK	    0x28000000
#define EBC0_CFG_RTC_1024PERCLK	    0x30000000
#define EBC0_CFG_RTC_2048PERCLK	    0x38000000
/* External Master Priority Low */
#define EBC0_CFG_EMPL_LOW	    0x00000000
#define EBC0_CFG_EMPL_MEDIUM_LOW    0x02000000
#define EBC0_CFG_EMPL_MEDIUM_HIGH   0x04000000
#define EBC0_CFG_EMPL_HIGH	    0x06000000
/* External Master Priority High */
#define EBC0_CFG_EMPH_LOW	    0x00000000
#define EBC0_CFG_EMPH_MEDIUM_LOW    0x00800000
#define EBC0_CFG_EMPH_MEDIUM_HIGH   0x01000000
#define EBC0_CFG_EMPH_HIGH	    0x01800000
/* Chip Select Three-State Control */
#define EBC0_CFG_CSTC_DRIVEN	    0x00400000
/* Burst Prefetch */
#define EBC0_CFG_BPF_ONEDW	    0x00000000
#define EBC0_CFG_BPF_TWODW	    0x00100000
#define EBC0_CFG_BPF_FOURDW	    0x00200000
/* External Master Size */
#define EBC0_CFG_EMS_8BIT	    0x00000000
/* Power Management Enable */
#define EBC0_CFG_PME_DISABLED	    0x00000000
#define EBC0_CFG_PME_ENABLED	    0x00020000
/* Power Management Timer */
#define EBC0_CFG_PMT_ENCODE(n)		((((unsigned long)(n))&0x1F)<<12)

#define SDR0_USB0                    0x0320     /* USB Control Register */
