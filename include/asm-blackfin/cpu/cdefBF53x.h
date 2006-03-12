/************************************************************************
 *
 * cdefBF53x.h
 *
 * (c) Copyright 2002-2003 Analog Devices, Inc.  All rights reserved.
 *
 ************************************************************************/

#ifndef _CDEFBF53x_H
#define _CDEFBF53x_H

#if defined(__ADSPBF531__)
	#include <asm/cpu/cdefBF531.h>
#elif defined(__ADSPBF532__)
	#include <asm/cpu/cdefBF532.h>
#elif defined(__ADSPBF533__)
	#include <asm/cpu/cdefBF533.h>
#elif defined(__ADSPBF561__)
	#include <asm/cpu/cdefBF561.h>
#elif defined(__ADSPBF535__)
	#include <asm/cpu/cdefBF535.h>
#elif defined(__AD6532__)
	#include <sam/cpu/cdefAD6532.h>
#else
	#if defined(__ADSPLPBLACKFIN__)
		#include <asm/cpu/cdefBF532.h>
	#else
		#include <asm/cpu/cdefBF535.h>
	#endif
#endif

#endif	/* _CDEFBF53x_H */
