/*
 * (C) Copyright 2002
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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
 *
 */

#include <common.h>
#include <asm/processor.h>
#include <asm/mmu.h>

int write_bat (ppc_bat_t bat, unsigned long upper, unsigned long lower)
{
	switch (bat) {
	case IBAT0:
		mtspr (IBAT0L, lower);
		mtspr (IBAT0U, upper);
		break;

	case IBAT1:
		mtspr (IBAT1L, lower);
		mtspr (IBAT1U, upper);
		break;

	case IBAT2:
		mtspr (IBAT2L, lower);
		mtspr (IBAT2U, upper);
		break;

	case IBAT3:
		mtspr (IBAT3L, lower);
		mtspr (IBAT3U, upper);
		break;

	case DBAT0:
		mtspr (DBAT0L, lower);
		mtspr (DBAT0U, upper);
		break;

	case DBAT1:
		mtspr (DBAT1L, lower);
		mtspr (DBAT1U, upper);
		break;

	case DBAT2:
		mtspr (DBAT2L, lower);
		mtspr (DBAT2U, upper);
		break;

	case DBAT3:
		mtspr (DBAT3L, lower);
		mtspr (DBAT3U, upper);
		break;

	default:
		return (-1);
	}

	return (0);
}

int read_bat (ppc_bat_t bat, unsigned long *upper, unsigned long *lower)
{
	unsigned long register u;
	unsigned long register l;

	switch (bat) {
	case IBAT0:
		l = mfspr (IBAT0L);
		u = mfspr (IBAT0U);
		break;

	case IBAT1:
		l = mfspr (IBAT1L);
		u = mfspr (IBAT1U);
		break;

	case IBAT2:
		l = mfspr (IBAT2L);
		u = mfspr (IBAT2U);
		break;

	case IBAT3:
		l = mfspr (IBAT3L);
		u = mfspr (IBAT3U);
		break;

	case DBAT0:
		l = mfspr (DBAT0L);
		u = mfspr (DBAT0U);
		break;

	case DBAT1:
		l = mfspr (DBAT1L);
		u = mfspr (DBAT1U);
		break;

	case DBAT2:
		l = mfspr (DBAT2L);
		u = mfspr (DBAT2U);
		break;

	case DBAT3:
		l = mfspr (DBAT3L);
		u = mfspr (DBAT3U);
		break;

	default:
		return (-1);
	}

	*upper = u;
	*lower = l;

	return (0);
}
