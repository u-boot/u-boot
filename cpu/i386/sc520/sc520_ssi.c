/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB <daniel@omicron.se>.
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
 */

/* stuff specific for the sc520, but independent of implementation */

#include <common.h>
#include <asm/ic/ssi.h>
#include <asm/ic/sc520.h>

int ssi_set_interface(int freq, int lsb_first, int inv_clock, int inv_phase)
{
	u8 temp=0;

	if (freq >= 8192) {
		temp |= CTL_CLK_SEL_4;
	} else if (freq >= 4096) {
		temp |= CTL_CLK_SEL_8;
	} else if (freq >= 2048) {
		temp |= CTL_CLK_SEL_16;
	} else if (freq >= 1024) {
		temp |= CTL_CLK_SEL_32;
	} else if (freq >= 512) {
		temp |= CTL_CLK_SEL_64;
	} else if (freq >= 256) {
		temp |= CTL_CLK_SEL_128;
	} else if (freq >= 128) {
		temp |= CTL_CLK_SEL_256;
	} else {
		temp |= CTL_CLK_SEL_512;
	}

	if (!lsb_first) {
		temp |= MSBF_ENB;
	}

	if (inv_clock) {
		temp |= CLK_INV_ENB;
	}

	if (inv_phase) {
		temp |= PHS_INV_ENB;
	}

	write_mmcr_byte(SC520_SSICTL, temp);

	return 0;
}

u8 ssi_txrx_byte(u8 data)
{
	write_mmcr_byte(SC520_SSIXMIT, data);
	while ((read_mmcr_byte(SC520_SSISTA)) & SSISTA_BSY);
	write_mmcr_byte(SC520_SSICMD, SSICMD_CMD_SEL_XMITRCV);
	while ((read_mmcr_byte(SC520_SSISTA)) & SSISTA_BSY);
	return read_mmcr_byte(SC520_SSIRCV);
}


void ssi_tx_byte(u8 data)
{
	write_mmcr_byte(SC520_SSIXMIT, data);
	while ((read_mmcr_byte(SC520_SSISTA)) & SSISTA_BSY);
	write_mmcr_byte(SC520_SSICMD, SSICMD_CMD_SEL_XMIT);
}

u8 ssi_rx_byte(void)
{
	while ((read_mmcr_byte(SC520_SSISTA)) & SSISTA_BSY);
	write_mmcr_byte(SC520_SSICMD, SSICMD_CMD_SEL_RCV);
	while ((read_mmcr_byte(SC520_SSISTA)) & SSISTA_BSY);
	return read_mmcr_byte(SC520_SSIRCV);
}
