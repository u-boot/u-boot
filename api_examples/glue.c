/*
 * (C) Copyright 2007 Semihalf
 *
 * Written by: Rafal Jaworowski <raj@semihalf.com>
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
 *
 */

#include <common.h>
#include <linux/types.h>
#include <api_public.h>

#include "glue.h"

static int valid_sig(struct api_signature *sig)
{
	uint32_t checksum;
	struct api_signature s;

	if (sig == NULL)
		return 0;
	/*
	 * Clear the checksum field (in the local copy) so as to calculate the
	 * CRC with the same initial contents as at the time when the sig was
	 * produced
	 */
	s = *sig;
	s.checksum = 0;

	checksum = crc32(0, (unsigned char *)&s, sizeof(struct api_signature));

	if (checksum != sig->checksum)
		return 0;

	return 1;
}

/*
 * Searches for the U-Boot API signature
 *
 * returns 1/0 depending on found/not found result
 */
int api_search_sig(struct api_signature **sig) {

	unsigned char *sp;

	if (sig == NULL)
		return 0;

	sp = (unsigned char *)API_SEARCH_START;

	while ((sp + (int)API_SIG_MAGLEN) < (unsigned char *)API_SEARCH_END) {
		if (!memcmp(sp, API_SIG_MAGIC, API_SIG_MAGLEN)) {
			*sig = (struct api_signature *)sp;
			if (valid_sig(*sig))
				return 1;
		}
		sp += API_SIG_MAGLEN;
	}

	*sig = NULL;
	return 0;
}

/****************************************
 *
 * console
 *
 ****************************************/

int ub_getc(void)
{
	int c;

	if (!syscall(API_GETC, NULL, (uint32_t)&c))
		return -1;

	return c;
}

int ub_tstc(void)
{
	int t;

	if (!syscall(API_TSTC, NULL, (uint32_t)&t))
		return -1;

	return t;
}

void ub_putc(char c)
{
	syscall(API_PUTC, NULL, (uint32_t)&c);
}

void ub_puts(const char *s)
{
	syscall(API_PUTS, NULL, (uint32_t)s);
}

/****************************************
 *
 * system
 *
 ****************************************/

void ub_reset(void)
{
	syscall(API_RESET, NULL);
}

#define MR_MAX 5
static struct mem_region mr[MR_MAX];
static struct sys_info si;

struct sys_info * ub_get_sys_info(void)
{
	int err = 0;

	memset(&si, 0, sizeof(struct sys_info));
	si.mr = mr;
	si.mr_no = MR_MAX;
	memset(&mr, 0, sizeof(mr));

	if (!syscall(API_GET_SYS_INFO, &err, (u_int32_t)&si))
		return NULL;

	return ((err) ? NULL : &si);
}

/****************************************
 *
 * timing
 *
 ****************************************/
 
void ub_udelay(unsigned long usec)
{
	syscall(API_UDELAY, NULL, &usec);
}

unsigned long ub_get_timer(unsigned long base)
{
	unsigned long cur;

	if (!syscall(API_GET_TIMER, NULL, &cur, &base))
		return 0;

	return cur;
}


/****************************************************************************
 *
 * devices
 *
 * Devices are identified by handles: numbers 0, 1, 2, ..., MAX_DEVS-1
 *
 ***************************************************************************/

#define MAX_DEVS 6

static struct device_info devices[MAX_DEVS];

struct device_info * ub_dev_get(int i)
{
	return ((i < 0 || i >= MAX_DEVS) ? NULL : &devices[i]);
}

/*
 * Enumerates the devices: fills out device_info elements in the devices[]
 * array.
 *
 * returns:		number of devices found
 */
int ub_dev_enum(void)
{
	struct device_info *di;
	int n = 0;

	memset(&devices, 0, sizeof(struct device_info) * MAX_DEVS);
	di = &devices[0];

	if (!syscall(API_DEV_ENUM, NULL, di))
		return 0;

	while (di->cookie != NULL) {

		if (++n >= MAX_DEVS)
			break;

		/* take another device_info */
		di++;

		/* pass on the previous cookie */
		di->cookie = devices[n - 1].cookie;

		if (!syscall(API_DEV_ENUM, NULL, di))
			return 0;
	}

	return n;
}

/*
 * handle:	0-based id of the device
 *
 * returns:	0 when OK, err otherwise
 */
int ub_dev_open(int handle)
{
	struct device_info *di;
	int err = 0;

	if (handle < 0 || handle >= MAX_DEVS)
		return API_EINVAL;

	di = &devices[handle];

	if (!syscall(API_DEV_OPEN, &err, di))
		return -1;

	return err;
}

int ub_dev_close(int handle)
{
	struct device_info *di;

	if (handle < 0 || handle >= MAX_DEVS)
		return API_EINVAL;

	di = &devices[handle];
	if (!syscall(API_DEV_CLOSE, NULL, di))
		return -1;

	return 0;
}

/*
 *
 * Validates device for read/write, it has to:
 *
 * - have sane handle
 * - be opened
 *
 * returns:	0/1 accordingly
 */
static int dev_valid(int handle)
{
	if (handle < 0 || handle >= MAX_DEVS)
		return 0;

	if (devices[handle].state != DEV_STA_OPEN)
		return 0;

	return 1;
}

static int dev_stor_valid(int handle)
{
	if (!dev_valid(handle))
		return 0;

	if (!(devices[handle].type & DEV_TYP_STOR))
		return 0;

	return 1;
}

int ub_dev_read(int handle, void *buf, lbasize_t len, lbastart_t start)
{
	struct device_info *di;
	lbasize_t act_len;
	int err = 0;

	if (!dev_stor_valid(handle))
		return API_ENODEV;

	di = &devices[handle];
	if (!syscall(API_DEV_READ, &err, di, buf, &len, &start, &act_len))
		return -1;

	if (err) 
		return err;

	if (act_len != len)
		return API_EIO;

	return 0;
}

static int dev_net_valid(int handle)
{
	if (!dev_valid(handle))
		return 0;

	if (devices[handle].type != DEV_TYP_NET)
		return 0;

	return 1;
}

int ub_dev_recv(int handle, void *buf, int len)
{
	struct device_info *di;
	int err = 0, act_len;

	if (!dev_net_valid(handle))
		return API_ENODEV;

	di = &devices[handle];
	if (!syscall(API_DEV_READ, &err, di, buf, &len, &act_len))
		return -1;

	if (err)
		return -1;

	return act_len;
}

int ub_dev_send(int handle, void *buf, int len)
{
	struct device_info *di;
	int err = 0;

	if (!dev_net_valid(handle))
		return API_ENODEV;

	di = &devices[handle];
	if (!syscall(API_DEV_WRITE, &err, di, buf, &len))
		return -1;

	return err;
}

/****************************************
 *
 * env vars
 *
 ****************************************/

char * ub_env_get(const char *name)
{
	char *value;

	if (!syscall(API_ENV_GET, NULL, (uint32_t)name, (uint32_t)&value))
		return NULL;

	return value;
}

void ub_env_set(const char *name, char *value)
{
	syscall(API_ENV_SET, NULL, (uint32_t)name, (uint32_t)value);
}


static char env_name[256];

const char * ub_env_enum(const char *last)
{
	const char *env, *str;
	int i;

	env = NULL;

	/*
	 * It's OK to pass only the name piece as last (and not the whole
	 * 'name=val' string), since the API_ENUM_ENV call uses envmatch()
	 * internally, which handles such case
	 */
	if (!syscall(API_ENV_ENUM, NULL, (uint32_t)last, (uint32_t)&env))
		return NULL;

	if (!env)
		/* no more env. variables to enumerate */
		return NULL;

	/* next enumerated env var */
	memset(env_name, 0, 256);
	for (i = 0, str = env; *str != '=' && *str != '\0';)
		env_name[i++] = *str++;

	env_name[i] = '\0';

	return env_name;
}
