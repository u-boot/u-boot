/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * Keith Outwater, keith_outwater@mvis.com
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

#define NUM_PORTS	4
#define PORT_BITS	18

/*
 * This structure provides configuration information for one port pin.
 * We include all fields needed to initialize any of the ioports.
 */
typedef struct {
    unsigned char conf:1;	/* If 1, configure this port		*/
    unsigned char ppar:1;	/* Port Pin Assignment Register		*/
    unsigned char psor:1;	/* Port Special Options Register	*/
    unsigned char pdir:1;	/* Port Data Direction Register		*/
    unsigned char podr:1;	/* Port Open Drain Register			*/
    unsigned char pdat:1;	/* Port Data Register				*/
    unsigned char pint:1;	/* Port Interrupt Register			*/
} mpc8xx_iop_conf_t;

extern void config_mpc8xx_ioports(volatile immap_t *immr);
