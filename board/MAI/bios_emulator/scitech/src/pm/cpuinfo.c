/****************************************************************************
*
*                   SciTech OS Portability Manager Library
*
*  ========================================================================
*
*    The contents of this file are subject to the SciTech MGL Public
*    License Version 1.0 (the "License"); you may not use this file
*    except in compliance with the License. You may obtain a copy of
*    the License at http://www.scitechsoft.com/mgl-license.txt
*
*    Software distributed under the License is distributed on an
*    "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
*    implied. See the License for the specific language governing
*    rights and limitations under the License.
*
*    The Original Code is Copyright (C) 1991-1998 SciTech Software, Inc.
*
*    The Initial Developer of the Original Code is SciTech Software, Inc.
*    All Rights Reserved.
*
*  ========================================================================
*
* Language:     ANSI C
* Environment:  Any
*
* Description:  Main module to implement the Zen Timer support functions.
*
****************************************************************************/

#include "ztimer.h"
#include "pmapi.h"
#include "oshdr.h"
#if !defined(__WIN32_VXD__) && !defined(__OS2_VDD__) && !defined(__NT_DRIVER__)
#include <stdio.h>
#include <string.h>
#endif

/*----------------------------- Implementation ----------------------------*/

/* External Intel assembler functions */
#ifdef  __INTEL__
/* {secret} */
ibool   _ASMAPI _CPU_haveCPUID(void);
/* {secret} */
ibool   _ASMAPI _CPU_check80386(void);
/* {secret} */
ibool   _ASMAPI _CPU_check80486(void);
/* {secret} */
uint    _ASMAPI _CPU_checkCPUID(void);
/* {secret} */
uint    _ASMAPI _CPU_getCPUIDModel(void);
/* {secret} */
uint    _ASMAPI _CPU_getCPUIDStepping(void);
/* {secret} */
uint    _ASMAPI _CPU_getCPUIDFeatures(void);
/* {secret} */
uint    _ASMAPI _CPU_getCacheSize(void);
/* {secret} */
uint    _ASMAPI _CPU_have3DNow(void);
/* {secret} */
ibool   _ASMAPI _CPU_checkClone(void);
/* {secret} */
void    _ASMAPI _CPU_readTimeStamp(CPU_largeInteger *time);
/* {secret} */
void    _ASMAPI _CPU_runBSFLoop(ulong iterations);
/* {secret} */
ulong   _ASMAPI _CPU_mulDiv(ulong a,ulong b,ulong c);
/* {secret} */
void ZTimerQuickInit(void);
#define CPU_HaveMMX     0x00800000
#define CPU_HaveRDTSC   0x00000010
#define CPU_HaveSSE     0x02000000
#endif

#if     defined(__SMX32__)
#include "smx/cpuinfo.c"
#elif   defined(__RTTARGET__)
#include "rttarget/cpuinfo.c"
#elif   defined(__REALDOS__)
#include "dos/cpuinfo.c"
#elif   defined(__NT_DRIVER__)
#include "ntdrv/cpuinfo.c"
#elif   defined(__WIN32_VXD__)
#include "vxd/cpuinfo.c"
#elif   defined(__WINDOWS32__)
#include "win32/cpuinfo.c"
#elif   defined(__OS2_VDD__)
#include "vdd/cpuinfo.c"
#elif   defined(__OS2__)
#include "os2/cpuinfo.c"
#elif   defined(__LINUX__)
#include "linux/cpuinfo.c"
#elif   defined(__QNX__)
#include "qnx/cpuinfo.c"
#elif   defined(__BEOS__)
#include "beos/cpuinfo.c"
#else
#error  CPU library not ported to this platform yet!
#endif

/*------------------------ Public interface routines ----------------------*/

/****************************************************************************
REMARKS:
Read an I/O port location.
****************************************************************************/
static uchar rdinx(
    int port,
    int index)
{
    PM_outpb(port,(uchar)index);
    return PM_inpb(port+1);
}

/****************************************************************************
REMARKS:
Write an I/O port location.
****************************************************************************/
static void wrinx(
    ushort port,
    ushort index,
    ushort value)
{
    PM_outpb(port,(uchar)index);
    PM_outpb(port+1,(uchar)value);
}

/****************************************************************************
REMARKS:
Enables the Cyrix CPUID instruction to properly detect MediaGX and 6x86
processors.
****************************************************************************/
static void _CPU_enableCyrixCPUID(void)
{
    uchar   ccr3;

    PM_init();
    ccr3 = rdinx(0x22,0xC3);
    wrinx(0x22,0xC3,(uchar)(ccr3 | 0x10));
    wrinx(0x22,0xE8,(uchar)(rdinx(0x22,0xE8) | 0x80));
    wrinx(0x22,0xC3,ccr3);
}

/****************************************************************************
DESCRIPTION:
Returns the type of processor in the system.

HEADER:
ztimer.h

RETURNS:
Numerical identifier for the installed processor

REMARKS:
Returns the type of processor in the system. Note that if the CPU is an
unknown Pentium family processor that we don't have an enumeration for,
the return value will be greater than or equal to the value of CPU_UnkPentium
(depending on the value returned by the CPUID instruction).

SEE ALSO:
CPU_getProcessorSpeed, CPU_haveMMX, CPU_getProcessorName
****************************************************************************/
uint ZAPI CPU_getProcessorType(void)
{
#if     defined(__INTEL__)
    uint            cpu,vendor,model,cacheSize;
    static ibool    firstTime = true;

    if (_CPU_haveCPUID()) {
	cpu = _CPU_checkCPUID();
	vendor = cpu & ~CPU_mask;
	if (vendor == CPU_Intel) {
	    /* Check for Intel processors */
	    switch (cpu & CPU_mask) {
		case 4: cpu = CPU_i486;         break;
		case 5: cpu = CPU_Pentium;      break;
		case 6:
		    if ((model = _CPU_getCPUIDModel()) == 1)
			cpu = CPU_PentiumPro;
		    else if (model <= 6) {
			cacheSize = _CPU_getCacheSize();
			if ((model == 5 && cacheSize == 0) ||
			    (model == 5 && cacheSize == 256) ||
			    (model == 6 && cacheSize == 128))
			    cpu = CPU_Celeron;
			else
			    cpu = CPU_PentiumII;
			}
		    else if (model >= 7) {
			/* Model 7 == Pentium III */
			/* Model 8 == Celeron/Pentium III Coppermine */
			cacheSize = _CPU_getCacheSize();
			if ((model == 8 && cacheSize == 128))
			    cpu = CPU_Celeron;
			else
			    cpu = CPU_PentiumIII;
			}
		    break;
		default:
		    cpu = CPU_UnkIntel;
		}
	    }
	else if (vendor == CPU_Cyrix) {
	    /* Check for Cyrix processors */
	    switch (cpu & CPU_mask) {
		case 4:
		    if ((model = _CPU_getCPUIDModel()) == 4)
			cpu = CPU_CyrixMediaGX;
		    else
			cpu = CPU_UnkCyrix;
		    break;
		case 5:
		    if ((model = _CPU_getCPUIDModel()) == 2)
			cpu = CPU_Cyrix6x86;
		    else if (model == 4)
			cpu = CPU_CyrixMediaGXm;
		    else
			cpu = CPU_UnkCyrix;
		    break;
		case 6:
		    if ((model = _CPU_getCPUIDModel()) <= 1)
			cpu = CPU_Cyrix6x86MX;
		    else
			cpu = CPU_UnkCyrix;
		    break;
		default:
		    cpu = CPU_UnkCyrix;
		}
	    }
	else if (vendor == CPU_AMD) {
	    /* Check for AMD processors */
	    switch (cpu & CPU_mask) {
		case 4:
		    if ((model = _CPU_getCPUIDModel()) == 0)
			cpu = CPU_AMDAm5x86;
		    else
			cpu = CPU_AMDAm486;
		    break;
		case 5:
		    if ((model = _CPU_getCPUIDModel()) <= 3)
			cpu = CPU_AMDK5;
		    else if (model <= 7)
			cpu = CPU_AMDK6;
		    else if (model == 8)
			cpu = CPU_AMDK6_2;
		    else if (model == 9)
			cpu = CPU_AMDK6_III;
		    else if (model == 13) {
			if (_CPU_getCPUIDStepping() <= 3)
			    cpu = CPU_AMDK6_IIIplus;
			else
			    cpu = CPU_AMDK6_2plus;
			}
		    else
			cpu = CPU_UnkAMD;
		    break;
		case 6:
		    if ((model = _CPU_getCPUIDModel()) == 3)
			cpu = CPU_AMDDuron;
		    else
			cpu = CPU_AMDAthlon;
		    break;
		default:
		    cpu = CPU_UnkAMD;
		}
	    }
	else if (vendor == CPU_IDT) {
	    /* Check for IDT WinChip processors */
	    switch (cpu & CPU_mask) {
		case 5:
		    if ((model = _CPU_getCPUIDModel()) <= 4)
			cpu = CPU_WinChipC6;
		    else if (model == 8)
			cpu = CPU_WinChip2;
		    else
			cpu = CPU_UnkIDT;
		    break;
		default:
		    cpu = CPU_UnkIDT;
		}
	    }
	else {
	    /* Assume a Pentium compatible Intel clone */
	    cpu = CPU_Pentium;
	    }
	return cpu | vendor | (_CPU_getCPUIDStepping() << CPU_steppingShift);
	}
    else {
	if (_CPU_check80386())
	    cpu = CPU_i386;
	else  if (_CPU_check80486()) {
	    /* If we get here we may have a Cyrix processor so we can try
	     * enabling the CPUID instruction and trying again.
	     */
	    if (firstTime) {
		firstTime = false;
		_CPU_enableCyrixCPUID();
		return CPU_getProcessorType();
		}
	    cpu = CPU_i486;
	    }
	else
	    cpu = CPU_Pentium;
	if (!_CPU_checkClone())
	    return cpu | CPU_Intel;
	return cpu;
	}
#elif   defined(__ALPHA__)
    return CPU_Alpha;
#elif   defined(__MIPS__)
    return CPU_Mips;
#elif   defined(__PPC__)
    return CPU_PowerPC;
#endif
}

/****************************************************************************
DESCRIPTION:
Returns true if the processor supports Intel MMX extensions.

HEADER:
ztimer.h

RETURNS:
True if MMX is available, false if not.

REMARKS:
This function determines if the processor supports the Intel MMX extended
instruction set.

SEE ALSO:
CPU_getProcessorType, CPU_getProcessorSpeed, CPU_have3DNow, CPU_haveSSE,
CPU_getProcessorName
****************************************************************************/
ibool ZAPI CPU_haveMMX(void)
{
#ifdef  __INTEL__
    if (_CPU_haveCPUID())
	return (_CPU_getCPUIDFeatures() & CPU_HaveMMX) != 0;
    return false;
#else
    return false;
#endif
}

/****************************************************************************
DESCRIPTION:
Returns true if the processor supports AMD 3DNow! extensions.

HEADER:
ztimer.h

RETURNS:
True if 3DNow! is available, false if not.

REMARKS:
This function determines if the processor supports the AMD 3DNow! extended
instruction set.

SEE ALSO:
CPU_getProcessorType, CPU_getProcessorSpeed, CPU_haveMMX, CPU_haveSSE,
CPU_getProcessorName
****************************************************************************/
ibool ZAPI CPU_have3DNow(void)
{
#ifdef  __INTEL__
    if (_CPU_haveCPUID())
	return _CPU_have3DNow();
    return false;
#else
    return false;
#endif
}

/****************************************************************************
DESCRIPTION:
Returns true if the processor supports Intel KNI extensions.

HEADER:
ztimer.h

RETURNS:
True if Intel KNI is available, false if not.

REMARKS:
This function determines if the processor supports the Intel KNI extended
instruction set.

SEE ALSO:
CPU_getProcessorType, CPU_getProcessorSpeed, CPU_haveMMX, CPU_have3DNow,
CPU_getProcessorName
****************************************************************************/
ibool ZAPI CPU_haveSSE(void)
{
#ifdef  __INTEL__
    if (_CPU_haveCPUID())
	return (_CPU_getCPUIDFeatures() & CPU_HaveSSE) != 0;
    return false;
#else
    return false;
#endif
}

/****************************************************************************
RETURNS:
True if the RTSC instruction is available, false if not.

REMARKS:
This function determines if the processor supports the Intel RDTSC
instruction, for high precision timing. If the processor is not an Intel or
Intel clone CPU, this function will always return false.

DESCRIPTION:
Returns true if the processor supports RDTSC extensions.

HEADER:
ztimer.h

RETURNS:
True if RTSC is available, false if not.

REMARKS:
This function determines if the processor supports the RDTSC instruction
for reading the processor time stamp counter.

SEE ALSO:
CPU_getProcessorType, CPU_getProcessorSpeed, CPU_haveMMX, CPU_have3DNow,
CPU_getProcessorName
****************************************************************************/
ibool ZAPI CPU_haveRDTSC(void)
{
#ifdef  __INTEL__
    if (_CPU_haveCPUID())
	return (_CPU_getCPUIDFeatures() & CPU_HaveRDTSC) != 0;
    return false;
#else
    return false;
#endif
}

#ifdef  __INTEL__

#define ITERATIONS      16000
#define SAMPLINGS       2
#define INNER_LOOPS     400

/****************************************************************************
REMARKS:
If processor does not support time stamp reading, but is at least a 386 or
above, utilize method of timing a loop of BSF instructions which take a
known number of cycles to run on i386(tm), i486(tm), and Pentium(R)
processors.
****************************************************************************/
static ulong GetBSFCpuSpeed(
    ulong cycles)
{
    CPU_largeInteger t0,t1,count_freq;
    ulong   ticks;              /* Microseconds elapsed during test     */
    ulong   current;            /* Variable to store time elapsed       */
    int     i,j,iPriority;
    ulong   lowest  = (ulong)-1;

    iPriority = SetMaxThreadPriority();
    GetCounterFrequency(&count_freq);
    for (i = 0; i < SAMPLINGS; i++) {
	GetCounter(&t0);
	for (j = 0; j < INNER_LOOPS; j++)
	    _CPU_runBSFLoop(ITERATIONS);
	GetCounter(&t1);
	current = t1.low - t0.low;
	if (current < lowest)
	    lowest = current;
	}
    RestoreThreadPriority(iPriority);

    /* Compute frequency */
    ticks = _CPU_mulDiv(lowest,1000000,count_freq.low);
    if ((ticks % count_freq.low) > (count_freq.low/2))
	ticks++;            /* Round up if necessary */
    if (ticks == 0)
	return 0;
    return ((cycles*INNER_LOOPS)/ticks);
}

#define TOLERANCE       1

/****************************************************************************
REMARKS:
On processors supporting the Read Time Stamp opcode, compare elapsed
time on the High-Resolution Counter with elapsed cycles on the Time
Stamp Register.

The inner loop runs up to 20 times oruntil the average of the previous
three calculated frequencies is within 1 MHz of each of the individual
calculated frequencies. This resampling increases the accuracy of the
results since outside factors could affect this calculation.
****************************************************************************/
static ulong GetRDTSCCpuSpeed(
    ibool accurate)
{
    CPU_largeInteger    t0,t1,s0,s1,count_freq;
    u64                 stamp0, stamp1, ticks0, ticks1;
    u64                 total_cycles, cycles, hz, freq;
    u64                 total_ticks, ticks;
    int                 tries,iPriority;
    ulong               maxCount;

    PM_set64_32(total_cycles,0);
    PM_set64_32(total_ticks,0);
    maxCount = accurate ? 600000 : 30000;
    iPriority = SetMaxThreadPriority();
    GetCounterFrequency(&count_freq);
    PM_set64(freq,count_freq.high,count_freq.low);
    for (tries = 0; tries < 3; tries++) {
	/* Loop until 100 ticks have passed since last read of hi-res
	 * counter. This accounts for overhead later.
	 */
	GetCounter(&t0);
	t1.low = t0.low;
	t1.high = t0.high;
	while ((t1.low - t0.low) < 100) {
	    GetCounter(&t1);
	    _CPU_readTimeStamp(&s0);
	    }

	/* Loop until 30000 ticks have passed since last read of hi-res counter.
	 * This allows for elapsed time for sampling. For a hi-res frequency
	 * of 1MHz, this is about 0.03 of a second. The frequency reported
	 * by the OS dependent code should be tuned to provide a good
	 * sample period depending on the accuracy of the OS timers (ie:
	 * if the accuracy is lower, lower the frequency to spend more time
	 * in the inner loop to get better accuracy).
	 */
	t0.low = t1.low;
	t0.high = t1.high;
	while ((t1.low - t0.low) < maxCount) {
	    GetCounter(&t1);
	    _CPU_readTimeStamp(&s1);
	    }

	/* Find the difference during the timing loop */
	PM_set64(stamp0,s0.high,s0.low);
	PM_set64(stamp1,s1.high,s1.low);
	PM_set64(ticks0,t0.high,t0.low);
	PM_set64(ticks1,t1.high,t1.low);
	PM_sub64(cycles,stamp1,stamp0);
	PM_sub64(ticks,ticks1,ticks0);

	/* Sum up the results */
	PM_add64(total_ticks,total_ticks,ticks);
	PM_add64(total_cycles,total_cycles,cycles);
	}
    RestoreThreadPriority(iPriority);

    /* Compute frequency in Hz */
    PM_mul64(hz,total_cycles,freq);
    PM_div64(hz,hz,total_ticks);
    return PM_64to32(hz);
}

#endif  /* __INTEL__ */

/****************************************************************************
DESCRIPTION:
Returns the speed of the processor in MHz.

HEADER:
ztimer.h

PARAMETERS:
accurate    - True of the speed should be measured accurately

RETURNS:
Processor speed in MHz.

REMARKS:
This function returns the speed of the CPU in MHz. Note that if the speed
cannot be determined, this function will return 0.

If the accurate parameter is set to true, this function will spend longer
profiling the speed of the CPU, and will not round the CPU speed that is
reported. This is important for highly accurate timing using the Pentium
RDTSC instruction, but it does take a lot longer for the profiling to
produce accurate results.

SEE ALSO:
CPU_getProcessorSpeedInHz, CPU_getProcessorType, CPU_haveMMX,
CPU_getProcessorName
****************************************************************************/
ulong ZAPI CPU_getProcessorSpeed(
    ibool accurate)
{
#if defined(__INTEL__)
    /* Number of cycles needed to execute a single BSF instruction on i386+
     * processors.
     */
    ulong   cpuSpeed;
    uint    i;
    static  ulong intel_cycles[] = {
	115,47,43,
	};
    static  ulong cyrix_cycles[] = {
	38,38,52,52,
	};
    static  ulong amd_cycles[] = {
	49,
	};
    static  ulong known_speeds[] = {
	1000,950,900,850,800,750,700,650,600,550,500,450,433,400,350,
	333,300,266,233,200,166,150,133,120,100,90,75,66,60,50,33,20,0,
	};

    if (CPU_haveRDTSC()) {
	cpuSpeed = (GetRDTSCCpuSpeed(accurate) + 500000) / 1000000;
	}
    else {
	int type = CPU_getProcessorType();
	int processor = type & CPU_mask;
	int vendor = type & CPU_familyMask;
	if (vendor == CPU_Intel)
	    cpuSpeed = GetBSFCpuSpeed(ITERATIONS * intel_cycles[processor - CPU_i386]);
	else if (vendor == CPU_Cyrix)
	    cpuSpeed = GetBSFCpuSpeed(ITERATIONS * cyrix_cycles[processor - CPU_Cyrix6x86]);
	else if (vendor == CPU_AMD)
	    cpuSpeed = GetBSFCpuSpeed(ITERATIONS * amd_cycles[0]);
	else
	    return 0;
	}

    /* Now normalise the results given known processors speeds, if the
     * speed we measure is within 2MHz of the expected values
     */
    if (!accurate) {
	for (i = 0; known_speeds[i] != 0; i++) {
	    if (cpuSpeed >= (known_speeds[i]-3) && cpuSpeed <= (known_speeds[i]+3)) {
		return known_speeds[i];
		}
	    }
	}
    return cpuSpeed;
#else
    return 0;
#endif
}

/****************************************************************************
DESCRIPTION:
Returns the speed of the processor in Hz.

HEADER:
ztimer.h

RETURNS:
Accurate processor speed in Hz.

REMARKS:
This function returns the accurate speed of the CPU in Hz. Note that if the
speed cannot be determined, this function will return 0.

This function is similar to the CPU_getProcessorSpeed function, except that
it attempts to accurately measure the CPU speed in Hz. This is used
internally in the Zen Timer libraries to provide accurate real world timing
information. This is important for highly accurate timing using the Pentium
RDTSC instruction, but it does take a lot longer for the profiling to
produce accurate results.

SEE ALSO:
CPU_getProcessorSpeed, CPU_getProcessorType, CPU_haveMMX,
CPU_getProcessorName
****************************************************************************/
ulong ZAPI CPU_getProcessorSpeedInHZ(
    ibool accurate)
{
#if defined(__INTEL__)
    if (CPU_haveRDTSC()) {
	return GetRDTSCCpuSpeed(accurate);
	}
    return CPU_getProcessorSpeed(false) * 1000000;
#else
    return 0;
#endif
}

/****************************************************************************
DESCRIPTION:
Returns a string defining the speed and name of the processor.

HEADER:
ztimer.h

RETURNS:
Processor name string.

REMARKS:
This function returns an English string describing the speed and name of the
CPU.

SEE ALSO:
CPU_getProcessorType, CPU_haveMMX, CPU_getProcessorName
****************************************************************************/
char * ZAPI CPU_getProcessorName(void)
{
#if defined(__INTEL__)
    static int  cpu,speed = -1;
    static char name[80];

    if (speed == -1) {
	cpu = CPU_getProcessorType();
	speed = CPU_getProcessorSpeed(false);
	}
    sprintf(name,"%d MHz ", speed);
    switch (cpu & CPU_mask) {
	case CPU_i386:
	    strcat(name,"Intel i386 processor");
	    break;
	case CPU_i486:
	    strcat(name,"Intel i486 processor");
	    break;
	case CPU_Pentium:
	    strcat(name,"Intel Pentium processor");
	    break;
	case CPU_PentiumPro:
	    strcat(name,"Intel Pentium Pro processor");
	    break;
	case CPU_PentiumII:
	    strcat(name,"Intel Pentium II processor");
	    break;
	case CPU_Celeron:
	    strcat(name,"Intel Celeron processor");
	    break;
	case CPU_PentiumIII:
	    strcat(name,"Intel Pentium III processor");
	    break;
	case CPU_UnkIntel:
	    strcat(name,"Unknown Intel processor");
	    break;
	case CPU_Cyrix6x86:
	    strcat(name,"Cyrix 6x86 processor");
	    break;
	case CPU_Cyrix6x86MX:
	    strcat(name,"Cyrix 6x86MX processor");
	    break;
	case CPU_CyrixMediaGX:
	    strcat(name,"Cyrix MediaGX processor");
	    break;
	case CPU_CyrixMediaGXm:
	    strcat(name,"Cyrix MediaGXm processor");
	    break;
	case CPU_UnkCyrix:
	    strcat(name,"Unknown Cyrix processor");
	    break;
	case CPU_AMDAm486:
	    strcat(name,"AMD Am486 processor");
	    break;
	case CPU_AMDAm5x86:
	    strcat(name,"AMD Am5x86 processor");
	    break;
	case CPU_AMDK5:
	    strcat(name,"AMD K5 processor");
	    break;
	case CPU_AMDK6:
	    strcat(name,"AMD K6 processor");
	    break;
	case CPU_AMDK6_2:
	    strcat(name,"AMD K6-2 processor");
	    break;
	case CPU_AMDK6_III:
	    strcat(name,"AMD K6-III processor");
	    break;
	case CPU_AMDK6_2plus:
	    strcat(name,"AMD K6-2+ processor");
	    break;
	case CPU_AMDK6_IIIplus:
	    strcat(name,"AMD K6-III+ processor");
	    break;
	case CPU_UnkAMD:
	    strcat(name,"Unknown AMD processor");
	    break;
	case CPU_AMDAthlon:
	    strcat(name,"AMD Athlon processor");
	    break;
	case CPU_AMDDuron:
	    strcat(name,"AMD Duron processor");
	    break;
	case CPU_WinChipC6:
	    strcat(name,"IDT WinChip C6 processor");
	    break;
	case CPU_WinChip2:
	    strcat(name,"IDT WinChip 2 processor");
	    break;
	case CPU_UnkIDT:
	    strcat(name,"Unknown IDT processor");
	    break;
	default:
	    strcat(name,"Unknown processor");
	}
    if (CPU_haveMMX())
	strcat(name," with MMX(R)");
    if (CPU_have3DNow())
	strcat(name,", 3DNow!(R)");
    if (CPU_haveSSE())
	strcat(name,", SSE(R)");
    return name;
#else
    return "Unknown";
#endif
}
