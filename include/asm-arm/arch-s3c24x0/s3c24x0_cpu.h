/*
 * (C) Copyright 2009
 * Kevin Morfitt, Fearnside Systems Ltd, <kevin.morfitt@fearnside-systems.co.uk>
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

#ifdef CONFIG_S3C2400
	#include <asm/arch/s3c2400.h>
#elif defined CONFIG_S3C2410
	#include <asm/arch/s3c2410.h>
#else
	#error Please define the s3c24x0 cpu type
#endif
