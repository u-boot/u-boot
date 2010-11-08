/* SPARC Processor specifics
 * taken from the SPARC port of Linux (ptrace.h).
 *
 * (C) Copyright 2007
 * Daniel Hellstrom, Gaisler Research, daniel@gaisler.com.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

#ifndef __ASM_SPARC_PROCESSOR_H
#define __ASM_SPARC_PROCESSOR_H

#include <asm/arch/asi.h>

#ifdef CONFIG_LEON

/* All LEON processors supported */
#include <asm/leon.h>

#else
/* other processors */
#error Unknown SPARC Processor
#endif

#ifndef __ASSEMBLY__

/* flush data cache */
static __inline__ void sparc_dcache_flush_all(void)
{
      __asm__ __volatile__("sta %%g0, [%%g0] %0\n\t"::"i"(ASI_DFLUSH):"memory");
}

/* flush instruction cache */
static __inline__ void sparc_icache_flush_all(void)
{
      __asm__ __volatile__("sta %%g0, [%%g0] %0\n\t"::"i"(ASI_IFLUSH):"memory");
}

/* do a cache miss load */
static __inline__ unsigned long long sparc_load_reg_cachemiss_qword(unsigned
								    long paddr)
{
	unsigned long long retval;
	__asm__ __volatile__("ldda [%1] %2, %0\n\t":
			     "=r"(retval):"r"(paddr), "i"(ASI_CACHEMISS));
	return retval;
}

static __inline__ unsigned long sparc_load_reg_cachemiss(unsigned long paddr)
{
	unsigned long retval;
	__asm__ __volatile__("lda [%1] %2, %0\n\t":
			     "=r"(retval):"r"(paddr), "i"(ASI_CACHEMISS));
	return retval;
}

static __inline__ unsigned short sparc_load_reg_cachemiss_word(unsigned long
							       paddr)
{
	unsigned short retval;
	__asm__ __volatile__("lduha [%1] %2, %0\n\t":
			     "=r"(retval):"r"(paddr), "i"(ASI_CACHEMISS));
	return retval;
}

static __inline__ unsigned char sparc_load_reg_cachemiss_byte(unsigned long
							      paddr)
{
	unsigned char retval;
	__asm__ __volatile__("lduba [%1] %2, %0\n\t":
			     "=r"(retval):"r"(paddr), "i"(ASI_CACHEMISS));
	return retval;
}

/* do a physical address bypass write, i.e. for 0x80000000 */
static __inline__ void sparc_store_reg_bypass(unsigned long paddr,
					      unsigned long value)
{
	__asm__ __volatile__("sta %0, [%1] %2\n\t"::"r"(value), "r"(paddr),
			     "i"(ASI_BYPASS):"memory");
}

static __inline__ unsigned long sparc_load_reg_bypass(unsigned long paddr)
{
	unsigned long retval;
	__asm__ __volatile__("lda [%1] %2, %0\n\t":
			     "=r"(retval):"r"(paddr), "i"(ASI_BYPASS));
	return retval;
}

/* Macros for bypassing cache when reading */
#define SPARC_NOCACHE_READ_DWORD(address) sparc_load_reg_cachemiss_qword((unsigned int)(address))
#define SPARC_NOCACHE_READ(address)       sparc_load_reg_cachemiss((unsigned int)(address))
#define SPARC_NOCACHE_READ_HWORD(address) sparc_load_reg_cachemiss_word((unsigned int)(address))
#define SPARC_NOCACHE_READ_BYTE(address)  sparc_load_reg_cachemiss_byte((unsigned int)(address))

#define SPARC_BYPASS_READ(address)        sparc_load_reg_bypass((unsigned int)(address))
#define SPARC_BYPASS_WRITE(address,value) sparc_store_reg_bypass((unsigned int)(address),(unsigned int)(value))

#endif

#endif				/* __ASM_SPARC_PROCESSOR_H */
