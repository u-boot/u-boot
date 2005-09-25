/*
 * (C) Copyright 2004
 * Jian Zhang, Texas Instruments, jzhang@ti.com.

 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>

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

/* #define DEBUG */

#include <common.h>

#if defined(CFG_ENV_IS_IN_NAND) /* Environment is in Nand Flash */

#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#include <linux/mtd/nand.h>

#if ((CONFIG_COMMANDS&(CFG_CMD_ENV|CFG_CMD_NAND)) == (CFG_CMD_ENV|CFG_CMD_NAND))
#define CMD_SAVEENV
#endif

#if defined(CFG_ENV_SIZE_REDUND)
#error CFG_ENV_SIZE_REDUND  not supported yet
#endif

#if defined(CFG_ENV_ADDR_REDUND)
#error CFG_ENV_ADDR_REDUND and CFG_ENV_IS_IN_NAND not supported yet
#endif


#ifdef CONFIG_INFERNO
#error CONFIG_INFERNO not supported yet
#endif

/* references to names in cmd_nand.c */
#define NANDRW_READ		0x01
#define NANDRW_WRITE	0x00
#define NANDRW_JFFS2	0x02
extern struct nand_chip nand_dev_desc[];
int nand_rw (struct nand_chip* nand, int cmd,
	    size_t start, size_t len,
	    size_t * retlen, u_char * buf);
int nand_erase(struct nand_chip* nand, size_t ofs,
				size_t len, int clean);

/* references to names in env_common.c */
extern uchar default_environment[];
extern int default_environment_size;

char * env_name_spec = "NAND";


#ifdef ENV_IS_EMBEDDED
extern uchar environment[];
env_t *env_ptr = (env_t *)(&environment[0]);
#else /* ! ENV_IS_EMBEDDED */
env_t *env_ptr = 0;
#endif /* ENV_IS_EMBEDDED */


/* local functions */
static void use_default(void);


uchar env_get_char_spec (int index)
{
	DECLARE_GLOBAL_DATA_PTR;

	return ( *((uchar *)(gd->env_addr + index)) );
}


/* this is called before nand_init()
 * so we can't read Nand to validate env data.
 * Mark it OK for now. env_relocate() in env_common.c
 * will call our relocate function which will does
 * the real validation.
 */
int env_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

  	gd->env_addr  = (ulong)&default_environment[0];
	gd->env_valid = 1;

	return (0);
}

#ifdef CMD_SAVEENV
int saveenv(void)
{
	int	total, ret = 0;
 	puts ("Erasing Nand...");
 	if (nand_erase(nand_dev_desc + 0, CFG_ENV_OFFSET, CFG_ENV_SIZE, 0))
 		return 1;

	puts ("Writing to Nand... ");
	ret = nand_rw(nand_dev_desc + 0,
				  NANDRW_WRITE | NANDRW_JFFS2, CFG_ENV_OFFSET, CFG_ENV_SIZE,
			      &total, (u_char*)env_ptr);
  	if (ret || total != CFG_ENV_SIZE)
		return 1;

 	puts ("done\n");
  	return ret;
}
#endif /* CMD_SAVEENV */


void env_relocate_spec (void)
{
#if !defined(ENV_IS_EMBEDDED)
	int ret, total;

	ret = nand_rw(nand_dev_desc + 0,
				  NANDRW_READ | NANDRW_JFFS2, CFG_ENV_OFFSET, CFG_ENV_SIZE,
			      &total, (u_char*)env_ptr);
  	if (ret || total != CFG_ENV_SIZE)
		return use_default();

	if (crc32(0, env_ptr->data, ENV_SIZE) != env_ptr->crc)
		return use_default();
#endif /* ! ENV_IS_EMBEDDED */

}

static void use_default()
{
	DECLARE_GLOBAL_DATA_PTR;

	puts ("*** Warning - bad CRC or NAND, using default environment\n\n");

  	if (default_environment_size > CFG_ENV_SIZE){
		puts ("*** Error - default environment is too large\n\n");
		return;
	}

	memset (env_ptr, 0, sizeof(env_t));
	memcpy (env_ptr->data,
			default_environment,
			default_environment_size);
	env_ptr->crc = crc32(0, env_ptr->data, ENV_SIZE);
 	gd->env_valid = 1;

}

#endif /* CFG_ENV_IS_IN_NAND */
