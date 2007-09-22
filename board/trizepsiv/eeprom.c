/*
 * (C) Copyright 2007
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>

static unsigned char srom[128];
extern u16 read_srom_word(int);
extern void write_srom_word(int offset, u16 val);

static int do_read_dm9000_eeprom ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]) {
	int i;

	for (i=0; i < 0x40; i++) {
		if (!(i % 0x10))
			printf("\n%08lx:", i);
		printf(" %04x", read_srom_word(i));
	}
	printf ("\n");
	return (0);
}

static int do_write_dm9000_eeprom ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]) {
	int offset,value;

	if (argc < 4) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	offset=simple_strtoul(argv[2],NULL,16);
	value=simple_strtoul(argv[3],NULL,16);
	if (offset > 0x40) {
		printf("Wrong offset : 0x%x\n",offset);
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}
	write_srom_word(offset, value);
	return (0);
}

int do_dm9000_eeprom ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]) {
	if (argc < 2) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	if (strcmp (argv[1],"read") == 0) {
		return (do_read_dm9000_eeprom(cmdtp,flag,argc,argv));
	} else if (strcmp (argv[1],"write") == 0) {
		return (do_write_dm9000_eeprom(cmdtp,flag,argc,argv));
	} else {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}
}

U_BOOT_CMD(
	dm9000ee,4,1,do_dm9000_eeprom,
	"dm9000ee- Read/Write eeprom connected to Ethernet Controller\n",
	"\ndm9000ee write <word offset> <value> \n"
	"\tdm9000ee read \n"
	"\tword:\t\t00-02 : MAC Address\n"
	"\t\t\t03-07 : DM9000 Configuration\n"
	"\t\t\t08-63 : User data\n");
