#ifndef _ERRNO_H

#include <asm-generic/errno.h>

extern int errno;

#define __set_errno(val) do { errno = val; } while (0)

#endif /* _ERRNO_H */
