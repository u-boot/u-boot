/*
 * (C) Copyright 2000
 * Murray Jensen <Murray.Jensen@csiro.au>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "error.h"

char *pname;

void
Warning(char *fmt, ...)
{
    va_list args;

    fprintf(stderr, "%s: WARNING: ", pname);

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fprintf(stderr, "\n");
}

void
Error(char *fmt, ...)
{
    va_list args;

    fprintf(stderr, "%s: ERROR: ", pname);

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fprintf(stderr, "\n");

    exit(1);
}

void
Perror(char *fmt, ...)
{
    va_list args;
    int e = errno;
    char *p;

    fprintf(stderr, "%s: ERROR: ", pname);

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    if ((p = strerror(e)) == NULL || *p == '\0')
	fprintf(stderr, ": Unknown Error (%d)\n", e);
    else
	fprintf(stderr, ": %s\n", p);

    exit(1);
}
