/*
 * U-boot - linkage.h
 *
 * Copyright (c) 2005-2007 Analog Devices Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _LINUX_LINKAGE_H
#define _LINUX_LINKAGE_H

#include <asm/linkage.h>
#include <linux/config.h>

#ifdef __cplusplus
#define CPP_ASMLINKAGE		extern "C"
#else
#define CPP_ASMLINKAGE
#endif

#define asmlinkage CPP_ASMLINKAGE

#define SYMBOL_NAME_STR(X)	#X
#define SYMBOL_NAME(X)		X
#ifdef __STDC__
#define SYMBOL_NAME_LABEL(X)	X##:
#else
#define SYMBOL_NAME_LABEL(X)	X:
#endif

#ifndef __ALIGN
#define __ALIGN .align		4
#endif

#ifndef __ALIGN_STR
#define __ALIGN_STR		".align 4"
#endif

#ifdef __ASSEMBLY__

#define ALIGN			__ALIGN
#define ALIGN_STR		__ALIGN_STR

#define LENTRY(name) \
	ALIGN; \
	SYMBOL_NAME_LABEL(name)

#define ENTRY(name) \
	.globl SYMBOL_NAME(name); \
	LENTRY(name)

#ifndef END
#define END(name) \
	.size name, .-name
#endif

#ifndef ENDPROC
#define ENDPROC(name) \
	.type name STT_FUNC; \
	END(name)
#endif

#endif

#endif
