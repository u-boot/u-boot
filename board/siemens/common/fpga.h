/*
 * (C) Copyright 2002
 * Wolfgang Grandegger, DENX Software Engineering, wg@denx.de.
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


#ifndef _FPGA_H_
#define _FPGA_H_

#define FPGA_INIT_IS_HIGH   0
#define FPGA_INIT_SET_HIGH  1
#define FPGA_INIT_SET_LOW   2
#define FPGA_PROG_SET_HIGH  3
#define FPGA_PROG_SET_LOW   4
#define FPGA_DONE_IS_HIGH   5
#define	FPGA_READ_MODE      6
#define FPGA_LOAD_MODE      7
#define FPGA_GET_ID         8
#define FPGA_INIT_PORTS     9

#define FPGA_NAME_LEN       8
typedef struct {
    char  name[FPGA_NAME_LEN];
    ulong conf_base;
    uint  init_mask;
    uint  prog_mask;
    uint  done_mask;
} fpga_t;

extern fpga_t fpga_list[];
extern int    fpga_count;

ulong fpga_control (fpga_t* fpga, int cmd);

#endif /* _FPGA_H_ */
