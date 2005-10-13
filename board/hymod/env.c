/*
 * (C) Copyright 2003
 * Murray Jensen, CSIRO-MIT, <Murray.Jensen@csiro.au>
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

/* imports from fetch.c */
extern int fetch_and_parse (char *, ulong, int (*)(uchar *, uchar *));

/* this is relative to the root of the server's tftp directory */
static char *def_global_env_path = "/hymod/global_env";

static int
env_callback (uchar *name, uchar *value)
{
	DECLARE_GLOBAL_DATA_PTR;

	hymod_conf_t *cp = &gd->bd->bi_hymod_conf;
	char ov[CFG_CBSIZE], nv[CFG_CBSIZE], *p, *q, *nn, c, *curver, *newver;
	int override = 1, append = 0, remove = 0, nnl, ovl, nvl;

	nn = (char *)name;

	if (*nn == '-') {
		override = 0;
		nn++;
	}

	while (*nn == ' ' || *nn == '\t')
		nn++;

	if ((nnl = strlen (nn)) == 0) {
		printf ("Empty name in global env file\n");
		return (0);
	}

	if ((c = nn[nnl - 1]) == '+' || c == '-') {
		if (c == '+')
			append = 1;
		else
			remove = 1;
		nn[--nnl] = '\0';
	}

	while (nnl > 0 && ((c = nn[nnl - 1]) == ' ' || c == '\t'))
		nn[--nnl] = '\0';
	if (nnl == 0) {
		printf ("Empty name in global env file\n");
		return (0);
	}

	p = (char *)value;
	q = nv;

	while ((c = *p) == ' ' || c == '\t')
		p++;

	nvl = strlen (p);
	while (nvl > 0 && ((c = p[nvl - 1]) == ' ' || c == '\t'))
		p[--nvl] = '\0';

	while ((*q = *p++) != '\0') {
		if (*q == '%') {
			switch (*p++) {

			case '\0':	/* whoops - back up */
				p--;
				break;

			case '%':	/* a single percent character */
				q++;
				break;

			case 's':	/* main board serial number as string */
				q += sprintf (q, "%010lu",
					cp->main.eeprom.serno);
				break;

			case 'S':	/* main board serial number as number */
				q += sprintf (q, "%lu", cp->main.eeprom.serno);
				break;

			default:	/* ignore any others */
				break;
			}
		}
		else
			q++;
	}

	if ((nvl = q - nv) == 0) {
		setenv (nn, NULL);
		return (1);
	}

	if ((curver = getenv ("global_env_version")) == NULL)
		curver = "unknown";

	if ((newver = getenv ("new_genv_version")) == NULL || \
	    strcmp (curver, newver) == 0) {
		if (strcmp (nn, "version") == 0)
			setenv ("new_genv_version", nv);
		return (1);
	}

	if ((p = getenv (nn)) != NULL) {

		strcpy (ov, p);
		ovl = strlen (ov);

		if (append) {

			if (strstr (ov, nv) == NULL) {

				printf ("Appending '%s' to env var '%s'\n",
					nv, nn);

				while (nvl >= 0) {
					nv[ovl + 1 + nvl] = nv[nvl];
					nvl--;
				}

				nv[ovl] = ' ';

				while (--ovl >= 0)
					nv[ovl] = ov[ovl];

				setenv (nn, nv);
			}

			return (1);
		}

		if (remove) {

			if (strstr (ov, nv) != NULL) {

				printf ("Removing '%s' from env var '%s'\n",
					nv, nn);

				while ((p = strstr (ov, nv)) != NULL) {
					q = p + nvl;
					if (*q == ' ')
						q++;
					strcpy(p, q);
				}

				setenv (nn, ov);
			}

			return (1);
		}

		if (!override || strcmp (ov, nv) == 0)
			return (1);

		printf ("Re-setting env cmd '%s' from '%s' to '%s'\n",
			nn, ov, nv);
	}
	else
		printf ("Setting env cmd '%s' to '%s'\n", nn, nv);

	setenv (nn, nv);
	return (1);
}

void
hymod_check_env (void)
{
	char *p, *path, *curver, *newver;
	int firsttime = 0, needsave = 0;

	if (getenv ("global_env_loaded") == NULL) {
		puts ("*** global environment has never been loaded\n");
		puts ("*** fetching from server");
		firsttime = 1;
	}
	else if ((p = getenv ("always_check_env")) != NULL &&
	    strcmp (p, "yes") == 0)
		puts ("*** checking for updated global environment");
	else
		return;

	puts (" (Control-C to Abort)\n");

	if ((path = getenv ("global_env_path")) == NULL || *path == '\0')
		path = def_global_env_path;

	if (fetch_and_parse (path, CFG_LOAD_ADDR, env_callback) == 0) {
		puts ("*** Fetch of global environment failed!\n");
		return;
	}

	if ((newver = getenv ("new_genv_version")) == NULL) {
		puts ("*** Version number not set - contents ignored!\n");
		return;
	}

	if ((curver = getenv ("global_env_version")) == NULL || \
	    strcmp (curver, newver) != 0) {
		setenv ("global_env_version", newver);
		needsave = 1;
	}
	else
		printf ("*** Global environment up-to-date (ver %s)\n", curver);

	setenv ("new_genv_version", NULL);

	if (firsttime) {
		setenv ("global_env_loaded", "yes");
		needsave = 1;
	}

	if (needsave)
		puts ("\n*** Remember to run the 'saveenv' "
			"command to save the changes\n\n");
}
