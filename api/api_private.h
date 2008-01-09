/*
 * (C) Copyright 2007 Semihalf
 *
 * Written by: Rafal Jaworowski <raj@semihalf.com>
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

#ifndef _API_PRIVATE_H_
#define _API_PRIVATE_H_

void	api_init(void);
void	platform_set_mr(struct sys_info *, unsigned long, unsigned long, int);
int	platform_sys_info(struct sys_info *);

void	dev_enum_reset(void);
int	dev_enum_storage(struct device_info *);
int	dev_enum_net(struct device_info *);

int	dev_open_stor(void *);
int	dev_open_net(void *);
int	dev_close_stor(void *);
int	dev_close_net(void *);

lbasize_t	dev_read_stor(void *, void *, lbasize_t, lbastart_t);
int		dev_read_net(void *, void *, int);
int		dev_write_net(void *, void *, int);

void dev_stor_init(void);

#endif /* _API_PRIVATE_H_ */
