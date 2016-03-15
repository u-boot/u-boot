/* bin2header.c - program to convert binary file into a C structure
 * definition to be included in a header file.
 *
 * (C) Copyright 2008 by Harald Welte <laforge@openmoko.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "%s needs one argument: the structure name\n",
			argv[0]);
		exit(1);
	}

	printf("/* bin2header output - automatically generated */\n");
	printf("unsigned char %s[] = {\n", argv[1]);

	while (1) {
		int i, nread;
		unsigned char buf[10];
		nread = read(0, buf, sizeof(buf));
		if (nread <= 0)
			break;

		printf("\t");
		for (i = 0; i < nread - 1; i++)
			printf("0x%02x, ", buf[i]);

		printf("0x%02x,\n", buf[nread-1]);
	}

	printf("};\n");

	exit(0);
}
