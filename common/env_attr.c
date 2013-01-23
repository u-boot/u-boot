/*
 * (C) Copyright 2012
 * Joe Hershberger, National Instruments, joe.hershberger@ni.com
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

#ifdef USE_HOSTCC /* Eliminate "ANSI does not permit..." warnings */
#include <stdint.h>
#include <stdio.h>
#include <linux/linux_string.h>
#else
#include <common.h>
#endif

#include <env_attr.h>
#include <errno.h>
#include <linux/string.h>
#include <malloc.h>

/*
 * Iterate through the whole list calling the callback for each found element.
 * "attr_list" takes the form:
 *	attributes = [^,:\s]*
 *	entry = name[:attributes]
 *	list = entry[,list]
 */
int env_attr_walk(const char *attr_list,
	int (*callback)(const char *name, const char *attributes))
{
	const char *entry, *entry_end;
	char *name, *attributes;

	if (!attr_list)
		/* list not found */
		return 1;

	entry = attr_list;
	do {
		char *entry_cpy = NULL;

		entry_end = strchr(entry, ENV_ATTR_LIST_DELIM);
		/* check if this is the last entry in the list */
		if (entry_end == NULL) {
			int entry_len = strlen(entry);

			if (entry_len) {
				/*
				 * allocate memory to copy the entry into since
				 * we will need to inject '\0' chars and squash
				 * white-space before calling the callback
				 */
				entry_cpy = malloc(entry_len + 1);
				if (entry_cpy)
					/* copy the rest of the list */
					strcpy(entry_cpy, entry);
				else
					return -ENOMEM;
			}
		} else {
			int entry_len = entry_end - entry;

			if (entry_len) {
				/*
				 * allocate memory to copy the entry into since
				 * we will need to inject '\0' chars and squash
				 * white-space before calling the callback
				 */
				entry_cpy = malloc(entry_len + 1);
				if (entry_cpy) {
					/* copy just this entry and null term */
					strncpy(entry_cpy, entry, entry_len);
					entry_cpy[entry_len] = '\0';
				} else
					return -ENOMEM;
			}
		}

		/* check if there is anything to process (e.g. not ",,,") */
		if (entry_cpy != NULL) {
			attributes = strchr(entry_cpy, ENV_ATTR_SEP);
			/* check if there is a ':' */
			if (attributes != NULL) {
				/* replace the ':' with '\0' to term name */
				*attributes++ = '\0';
				/* remove white-space from attributes */
				attributes = strim(attributes);
			}
			/* remove white-space from name */
			name = strim(entry_cpy);

			/* only call the callback if there is a name */
			if (strlen(name) != 0) {
				int retval = 0;

				retval = callback(name, attributes);
				if (retval) {
					free(entry_cpy);
					return retval;
				}
			}
		}

		free(entry_cpy);
		entry = entry_end + 1;
	} while (entry_end != NULL);

	return 0;
}

/*
 * Search for the last matching string in another string with the option to
 * start looking at a certain point (i.e. ignore anything beyond that point).
 */
static char *reverse_strstr(const char *searched, const char *search_for,
	const char *searched_start)
{
	char *result = NULL;

	if (*search_for == '\0')
		return (char *)searched;

	for (;;) {
		char *match = strstr(searched, search_for);

		/*
		 * Stop looking if no new match is found or looking past the
		 * searched_start pointer
		 */
		if (match == NULL || (searched_start != NULL &&
		    match + strlen(search_for) > searched_start))
			break;

		result = match;
		searched = match + 1;
	}

	return result;
}

/*
 * Retrieve the attributes string associated with a single name in the list
 * There is no protection on attributes being too small for the value
 */
int env_attr_lookup(const char *attr_list, const char *name, char *attributes)
{
	const char *entry = NULL;

	if (!attributes)
		/* bad parameter */
		return -1;
	if (!attr_list)
		/* list not found */
		return 1;

	entry = reverse_strstr(attr_list, name, NULL);
	while (entry != NULL) {
		const char *prevch = entry - 1;
		const char *nextch = entry + strlen(name);

		/* Skip spaces */
		while (*prevch == ' ')
			prevch--;
		while (*nextch == ' ')
			nextch++;

		/* check for an exact match */
		if ((entry == attr_list ||
		     *prevch == ENV_ATTR_LIST_DELIM) &&
		    (*nextch == ENV_ATTR_SEP ||
		     *nextch == ENV_ATTR_LIST_DELIM ||
		     *nextch == '\0'))
			break;

		entry = reverse_strstr(attr_list, name, entry);
	}
	if (entry != NULL) {
		int len;

		/* skip the name */
		entry += strlen(name);
		/* skip spaces */
		while (*entry == ' ')
			entry++;
		if (*entry != ENV_ATTR_SEP)
			len = 0;
		else {
			const char *delim;
			static const char delims[] = {
				ENV_ATTR_LIST_DELIM, ' ', '\0'};

			/* skip the attr sep */
			entry += 1;
			/* skip spaces */
			while (*entry == ' ')
				entry++;

			delim = strpbrk(entry, delims);
			if (delim == NULL)
				len = strlen(entry);
			else
				len = delim - entry;
			memcpy(attributes, entry, len);
		}
		attributes[len] = '\0';

		/* success */
		return 0;
	}

	/* not found in list */
	return 2;
}
