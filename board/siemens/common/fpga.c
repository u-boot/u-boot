/*
 * (C) Copyright 2002
 * Wolfgang Grandegger, DENX Software Engineering, wg@denx.de.
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


#include <common.h>
#include <command.h>
#include <linux/ctype.h>
#include <common.h>

#include "fpga.h"

int  power_on_reset(void);

/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */


static int fpga_get_version(fpga_t* fpga, char* name)
{
    char vname[12];
    /*
     * Net-list string format:
     *     "vvvvvvvvddddddddn...".
     *     Version Date    Name
     *     "0000000322042002PUMA" = PUMA version 3 from 22.04.2002.
     */
    if (strlen(name) < (16 + strlen(fpga->name)))
	goto failure;
    /* Check FPGA name */
    if (strcmp(&name[16], fpga->name) != 0)
	goto failure;
    /* Get version number */
    memcpy(vname, name, 8);
    vname[8] = '\0';
    return simple_strtoul(vname, NULL, 16);

 failure:
    printf("Image name %s is invalid\n", name);
    return -1;
}

/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */

static fpga_t* fpga_get(char* fpga_name)
{
    char name[FPGA_NAME_LEN];
    int i;

    if (strlen(fpga_name) >= FPGA_NAME_LEN)
	goto failure;
    for (i = 0; i < strlen(fpga_name); i++)
	name[i] = toupper(fpga_name[i]);
    name[i] = '\0';
    for (i = 0; i < fpga_count; i++) {
	if (strcmp(name, fpga_list[i].name) == 0)
	    return &fpga_list[i];
    }
 failure:
    printf("FPGA: name %s is invalid\n", fpga_name);
    return NULL;
}

/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */

static void fpga_status (fpga_t* fpga)
{
    /* Check state */
    if (fpga_control(fpga, FPGA_DONE_IS_HIGH))
	printf ("%s is loaded (%08lx)\n",
		fpga->name, fpga_control(fpga, FPGA_GET_ID));
    else
	printf ("%s is NOT loaded\n", fpga->name);
}

/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */

#define FPGA_RESET_TIMEOUT 100 /* = 10 ms */

static int fpga_reset (fpga_t* fpga)
{
    int i;

    /* Set PROG to low and wait til INIT goes low */
    fpga_control(fpga, FPGA_PROG_SET_LOW);
    for (i = 0; i < FPGA_RESET_TIMEOUT; i++) {
	udelay (100);
	if (!fpga_control(fpga, FPGA_INIT_IS_HIGH))
	    break;
    }
    if (i == FPGA_RESET_TIMEOUT)
	goto failure;

    /* Set PROG to high and wait til INIT goes high */
    fpga_control(fpga, FPGA_PROG_SET_HIGH);
    for (i = 0; i < FPGA_RESET_TIMEOUT; i++) {
	udelay (100);
	if (fpga_control(fpga, FPGA_INIT_IS_HIGH))
	    break;
    }
    if (i == FPGA_RESET_TIMEOUT)
	goto failure;

    return 0;
 failure:
    return 1;
}

/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */

#define FPGA_LOAD_TIMEOUT 100 /* = 10 ms */

static int fpga_load (fpga_t* fpga, ulong addr, int checkall)
{
    volatile uchar *fpga_addr = (volatile uchar *)fpga->conf_base;
    image_header_t *hdr = (image_header_t *)addr;
    ulong len;
    uchar *data;
    char msg[32];
    int verify, i;

#if defined(CONFIG_FIT)
    if (genimg_get_format ((void *)hdr) != IMAGE_FORMAT_LEGACY) {
	puts ("Non legacy image format not supported\n");
	return -1;
    }
#endif

    /*
     * Check the image header and data of the net-list
     */
    if (!image_check_magic (hdr)) {
	strcpy (msg, "Bad Image Magic Number");
	goto failure;
    }

    if (!image_check_hcrc (hdr)) {
	strcpy (msg, "Bad Image Header CRC");
	goto failure;
    }

    data = (uchar*)image_get_data (hdr);
    len  = image_get_data_size (hdr);

    verify = getenv_yesno ("verify");
    if (verify) {
	if (!image_check_dcrc (hdr)) {
	    strcpy (msg, "Bad Image Data CRC");
	    goto failure;
	}
    }

    if (checkall && fpga_get_version(fpga, image_get_name (hdr)) < 0)
	return 1;

    /* align length */
    if (len & 1)
	++len;

    /*
     * Reset FPGA and wait for completion
     */
    if (fpga_reset(fpga)) {
	strcpy (msg, "Reset Timeout");
	goto failure;
    }

    printf ("(%s)... ", image_get_name (hdr));
    /*
     * Copy data to FPGA
     */
    fpga_control (fpga, FPGA_LOAD_MODE);
    while (len--) {
	*fpga_addr = *data++;
    }
    fpga_control (fpga, FPGA_READ_MODE);

    /*
     * Wait for completion and check error status if timeout
     */
    for (i = 0; i < FPGA_LOAD_TIMEOUT; i++) {
	udelay (100);
	if (fpga_control (fpga, FPGA_DONE_IS_HIGH))
	    break;
    }
    if (i == FPGA_LOAD_TIMEOUT) {
	if (fpga_control(fpga, FPGA_INIT_IS_HIGH))
	    strcpy(msg, "Invalid Size");
	else
	    strcpy(msg, "CRC Error");
	goto failure;
    }

    printf("done\n");
    return 0;

 failure:

    printf("ERROR: %s\n", msg);
    return 1;
}

#if defined(CONFIG_CMD_BSP)

/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */

int do_fpga (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    ulong addr = 0;
    int i;
    fpga_t* fpga;

    if (argc < 2)
	goto failure;

    if (strncmp(argv[1], "stat", 4) == 0) {		/* status */
	if (argc == 2) {
	    for (i = 0; i < fpga_count; i++) {
		fpga_status (&fpga_list[i]);
	    }
	}
	else if (argc == 3) {
	    if ((fpga = fpga_get(argv[2])) == 0)
		goto failure;
	    fpga_status (fpga);
	}
	else
	    goto failure;
    }
    else if (strcmp(argv[1],"load") == 0) {		/* load */
	if (argc == 3 && fpga_count == 1) {
	    fpga = &fpga_list[0];
	}
	else if (argc == 4) {
	    if ((fpga = fpga_get(argv[2])) == 0)
		goto failure;
	}
	else
	    goto failure;

	addr = simple_strtoul(argv[argc-1], NULL, 16);

	printf ("FPGA load %s: addr %08lx: ",
		fpga->name, addr);
	fpga_load (fpga, addr, 1);

    }
    else if (strncmp(argv[1], "rese", 4) == 0) {	/* reset */
	if (argc == 2 && fpga_count == 1) {
	    fpga = &fpga_list[0];
	}
	else if (argc == 3) {
	    if ((fpga = fpga_get(argv[2])) == 0)
		goto failure;
	}
	else
	    goto failure;

	printf ("FPGA reset %s: ", fpga->name);
	if (fpga_reset(fpga))
	    printf ("ERROR: Timeout\n");
	else
	    printf ("done\n");
    }
    else
	goto failure;

    return 0;

 failure:
    cmd_usage(cmdtp);
    return 1;
}

U_BOOT_CMD(
	fpga,	4,	1,	do_fpga,
	"access FPGA(s)",
	"fpga status [name] - print FPGA status\n"
	"fpga reset  [name] - reset FPGA\n"
	"fpga load [name] addr - load FPGA configuration data"
);

#endif

/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */

int fpga_init (void)
{
    ulong addr;
    ulong new_id, old_id = 0;
    image_header_t *hdr;
    fpga_t* fpga;
    int do_load, i, j;
    char name[16], *s;

    /*
     *  Port setup for FPGA control
     */
    for (i = 0; i < fpga_count; i++) {
	fpga_control(&fpga_list[i], FPGA_INIT_PORTS);
    }

    /*
     * Load FPGA(s): a new net-list is loaded if the FPGA is
     * empty, Power-on-Reset or the old one is not up-to-date
     */
    for (i = 0; i < fpga_count; i++) {
	fpga = &fpga_list[i];
	printf ("%s:  ", fpga->name);

	for (j = 0; j < strlen(fpga->name); j++)
	    name[j] = tolower(fpga->name[j]);
	name[j] = '\0';
	sprintf(name, "%s_addr", name);
	addr = 0;
	if ((s = getenv(name)) != NULL)
	    addr = simple_strtoul(s, NULL, 16);

	if (!addr) {
	    printf ("env. variable %s undefined\n", name);
	    return 1;
	}

	hdr = (image_header_t *)addr;
#if defined(CONFIG_FIT)
	if (genimg_get_format ((void *)hdr) != IMAGE_FORMAT_LEGACY) {
	   puts ("Non legacy image format not supported\n");
	   return -1;
	}
#endif

	if ((new_id = fpga_get_version(fpga, image_get_name (hdr))) == -1)
	    return 1;

	do_load = 1;

	if (!power_on_reset() && fpga_control(fpga, FPGA_DONE_IS_HIGH)) {
	    old_id = fpga_control(fpga, FPGA_GET_ID);
	    if (new_id == old_id)
		do_load = 0;
	}

	if (do_load) {
	    printf ("loading ");
	    fpga_load (fpga, addr, 0);
	} else {
	    printf ("loaded (%08lx)\n", old_id);
	}
    }

    return 0;
}
