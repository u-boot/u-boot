/*
 * NOTE:        DAVICOM DM9000 ethernet driver interface
 *
 * Authors:     Remy Bohmer <linux@bohmer.net>
 *
 *              This program is free software; you can redistribute it and/or
 *              modify it under the terms of the GNU General Public License
 *              as published by the Free Software Foundation; either version
 *              2 of the License, or (at your option) any later version.
 */
#ifndef __DM9000_H__
#define __DM9000_H__

/******************  function prototypes **********************/
#if !defined(CONFIG_DM9000_NO_SROM)
void dm9000_write_srom_word(int offset, u16 val);
void dm9000_read_srom_word(int offset, u8 *to);
#endif

#endif /* __DM9000_H__ */
