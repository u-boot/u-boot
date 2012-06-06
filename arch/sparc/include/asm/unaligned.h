#ifndef _ASM_SPARC_UNALIGNED_H
#define _ASM_SPARC_UNALIGNED_H

/*
 * The SPARC can not do unaligned accesses, it must be split into multiple
 * byte accesses. The SPARC is in big endian mode.
 */
#include <asm-generic/unaligned.h>

#endif	/* _ASM_SPARC_UNALIGNED_H */
