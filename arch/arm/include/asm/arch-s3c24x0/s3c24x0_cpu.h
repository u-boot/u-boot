/*
 * (C) Copyright 2009
 * Kevin Morfitt, Fearnside Systems Ltd, <kevin.morfitt@fearnside-systems.co.uk>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifdef CONFIG_S3C2400
	#include <asm/arch/s3c2400.h>
#elif defined CONFIG_S3C2410
	#include <asm/arch/s3c2410.h>
#elif defined CONFIG_S3C2440
	#include <asm/arch/s3c2440.h>
#else
	#error Please define the s3c24x0 cpu type
#endif
