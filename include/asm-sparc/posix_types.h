/*
 * (C) Copyright 2000 - 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2007, taken from asm-ppc/posix_types.h
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

#ifndef __SPARC_POSIX_TYPES_H__
#define __SPARC_POSIX_TYPES_H__

/*
 * This file is generally used by user-level software, so you need to
 * be a little careful about namespace pollution etc.  Also, we cannot
 * assume GCC is being used.
 */

typedef unsigned int __kernel_dev_t;
typedef unsigned int __kernel_ino_t;
typedef unsigned int __kernel_mode_t;
typedef unsigned short __kernel_nlink_t;
typedef long __kernel_off_t;
typedef int __kernel_pid_t;
typedef unsigned int __kernel_uid_t;
typedef unsigned int __kernel_gid_t;
typedef unsigned int __kernel_size_t;
typedef int __kernel_ssize_t;
typedef long __kernel_ptrdiff_t;
typedef long __kernel_time_t;
typedef long __kernel_suseconds_t;
typedef long __kernel_clock_t;
typedef int __kernel_daddr_t;
typedef char *__kernel_caddr_t;
typedef short __kernel_ipc_pid_t;
typedef unsigned short __kernel_uid16_t;
typedef unsigned short __kernel_gid16_t;
typedef unsigned int __kernel_uid32_t;
typedef unsigned int __kernel_gid32_t;

typedef unsigned int __kernel_old_uid_t;
typedef unsigned int __kernel_old_gid_t;

#ifdef __GNUC__
typedef long long __kernel_loff_t;
#endif

typedef struct {
	int val[2];
} __kernel_fsid_t;

#ifndef __GNUC__

#define	__FD_SET(d, set)	((set)->fds_bits[__FDELT(d)] |= __FDMASK(d))
#define	__FD_CLR(d, set)	((set)->fds_bits[__FDELT(d)] &= ~__FDMASK(d))
#define	__FD_ISSET(d, set)	((set)->fds_bits[__FDELT(d)] & __FDMASK(d))
#define	__FD_ZERO(set)	\
  ((void) memset ((__ptr_t) (set), 0, sizeof (__kernel_fd_set)))

#else				/* __GNUC__ */

#if defined(__KERNEL__) || !defined(__GLIBC__) || (__GLIBC__ < 2) \
    || (__GLIBC__ == 2 && __GLIBC_MINOR__ == 0)
/* With GNU C, use inline functions instead so args are evaluated only once: */

#undef __FD_SET
static __inline__ void __FD_SET(unsigned long fd, __kernel_fd_set * fdsetp)
{
	unsigned long _tmp = fd / __NFDBITS;
	unsigned long _rem = fd % __NFDBITS;
	fdsetp->fds_bits[_tmp] |= (1UL << _rem);
}

#undef __FD_CLR
static __inline__ void __FD_CLR(unsigned long fd, __kernel_fd_set * fdsetp)
{
	unsigned long _tmp = fd / __NFDBITS;
	unsigned long _rem = fd % __NFDBITS;
	fdsetp->fds_bits[_tmp] &= ~(1UL << _rem);
}

#undef __FD_ISSET
static __inline__ int __FD_ISSET(unsigned long fd, __kernel_fd_set * p)
{
	unsigned long _tmp = fd / __NFDBITS;
	unsigned long _rem = fd % __NFDBITS;
	return (p->fds_bits[_tmp] & (1UL << _rem)) != 0;
}

/*
 * This will unroll the loop for the normal constant case (8 ints,
 * for a 256-bit fd_set)
 */
#undef __FD_ZERO
static __inline__ void __FD_ZERO(__kernel_fd_set * p)
{
	unsigned int *tmp = (unsigned int *)p->fds_bits;
	int i;

	if (__builtin_constant_p(__FDSET_LONGS)) {
		switch (__FDSET_LONGS) {
		case 8:
			tmp[0] = 0;
			tmp[1] = 0;
			tmp[2] = 0;
			tmp[3] = 0;
			tmp[4] = 0;
			tmp[5] = 0;
			tmp[6] = 0;
			tmp[7] = 0;
			return;
		}
	}
	i = __FDSET_LONGS;
	while (i) {
		i--;
		*tmp = 0;
		tmp++;
	}
}

#endif				/* defined(__KERNEL__) || !defined(__GLIBC__) || (__GLIBC__ < 2) */
#endif				/* __GNUC__ */
#endif				/* _SPARC_POSIX_TYPES_H */
