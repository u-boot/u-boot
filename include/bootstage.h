/*
 * This file implements recording of each stage of the boot process. It is
 * intended to implement timing of each stage, reporting this information
 * to the user and passing it to the OS for logging / further analysis.
 *
 * Copyright (c) 2011 The Chromium OS Authors.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _BOOTSTAGE_H
#define _BOOTSTAGE_H

/*
 * A list of boot stages that we know about. Each of these indicates the
 * state that we are at, and the action that we are about to perform. For
 * errors, we issue an error for an item when it fails. Therefore the
 * normal sequence is:
 *
 * progress action1
 * progress action2
 * progress action3
 *
 * and an error condition where action 3 failed would be:
 *
 * progress action1
 * progress action2
 * progress action3
 * error on action3
 */
enum bootstage_id {
	BOOTSTAGE_ID_RUN_OS	= 15,	/* Exiting U-Boot, entering OS */
};

/*
 * Board code can implement show_boot_progress() if needed.
 *
 * @param val	Progress state (enum bootstage_id), or -id if an error
 *		has occurred.
 */
void show_boot_progress(int val);
static inline void show_boot_error(int val)
{
	show_boot_progress(-val);
}

#endif
