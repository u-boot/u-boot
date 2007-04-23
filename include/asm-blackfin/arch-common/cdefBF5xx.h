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
	#include <asm/arch-bf533/cdefBF531.h>
#elif defined(__ADSPBF532__)
	#include <asm/arch-bf533/cdefBF532.h>
#elif defined(__ADSPBF533__)
	#include <asm/arch-bf533/cdefBF533.h>
	#include <asm/arch-bf533/defBF533_extn.h>
	#include <asm/arch-bf533/bf533_serial.h>
#elif defined(__ADSPBF537__)
	#include <asm/arch-bf537/cdefBF537.h>
	#include <asm/arch-bf537/defBF537_extn.h>
	#include <asm/arch-bf537/bf537_serial.h>
#elif defined(__ADSPBF561__)
	#include <asm/arch-bf561/cdefBF561.h>
	#include <asm/arch-bf561/defBF561_extn.h>
	#include <asm/arch-bf561/bf561_serial.h>
#elif defined(__ADSPBF535__)
	#include <asm/cpu/cdefBF5d35.h>
#elif defined(__AD6532__)
	#include <asm/cpu/cdefAD6532.h>
#else
	#if defined(__ADSPLPBLACKFIN__)
		#include <asm/arch-bf533/cdefBF532.h>
	#else
		#include <asm/arch-bf533/cdefBF535.h>
	#endif
#endif

#endif	/* _CDEFBF53x_H */
