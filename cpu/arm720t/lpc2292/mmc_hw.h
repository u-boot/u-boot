/*
    This module implements a linux character device driver for the 24c256 chip.
    Copyright (C) 2006  Embedded Artists AB (www.embeddedartists.com)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef _MMC_HW_
#define _MMC_HW_

unsigned char mmc_read_csd(unsigned char *Buffer);
unsigned char mmc_read_sector (unsigned long addr,
			       unsigned char *Buffer);
unsigned char mmc_write_sector (unsigned long addr,unsigned char *Buffer);
int mmc_hw_init(void);

#endif /* _MMC_HW_ */
