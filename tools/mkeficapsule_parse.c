// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2023 Linaro Limited
 */

/*
 * The code in this file adds parsing ability to the mkeficapsule
 * tool. This allows specifying parameters needed to build the capsule
 * through the config file instead of specifying them on the command-line.
 * Parameters can be specified for more than one payload, generating the
 * corresponding capsule files.
 *
 * The parameters are specified in a "key:value" pair. All the parameters
 * that are currently supported by the mkeficapsule tool can be specified
 * in the config file.
 *
 * The example below shows four payloads. The first payload is an example
 * of generating a signed capsule. The second payload is an example of
 * generating an unsigned capsule. The third payload is an accept empty
 * capsule, while the fourth payload is the revert empty capsule, used
 * for the multi-bank firmware update feature.
 *
 * This functionality can be easily extended to generate a single capsule
 * comprising multiple payloads.

	{
	    image-guid: 02f4d760-cfd5-43bd-8e2d-a42acb33c660
	    hardware-instance: 0
	    monotonic-count: 1
	    payload: u-boot.bin
	    fw-version: 2
	    image-index: 1
	    private-key: /path/to/priv/key
	    pub-key-cert: /path/to/pub/key
	    capsule: u-boot.capsule
	}
	{
	    image-guid: 4ce292da-1dd8-428d-a1c2-77743ef8b96e
	    hardware-instance: 0
	    payload: u-boot.itb
	    image-index: 2
	    fw-version: 10
	    oemflags: 0x8000
	    capsule: fit.capsule
	}
	{
	    capsule-type: accept
	    image-guid: 4ce292da-1dd8-428d-a1c2-77743ef8b96e
	    capsule: accept.capsule
	}
	{
	    capsule-type: revert
	    capsule: revert.capsule
	}
*/

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <uuid/uuid.h>

#include "eficapsule.h"

#define PARAMS_START	"{"
#define PARAMS_END	"}"

#define PSTART		2
#define PEND		3

#define MALLOC_FAIL_STR		"Unable to allocate memory\n"

#define ARRAY_SIZE(x)		(sizeof(x) / sizeof((x)[0]))

const char *capsule_params[] = {
	"image-guid", "image-index", "private-key",
	"pub-key-cert", "payload", "capsule",
	"hardware-instance", "monotonic-count",
	"capsule-type",	"oemflags", "fw-version" };

static unsigned char params_start;
static unsigned char params_end;

static void print_and_exit(const char *str)
{
	fprintf(stderr, "%s", str);
	exit(EXIT_FAILURE);
}

static int param_delim_checks(char *line, unsigned char *token)
{
	if (!strcmp(line, PARAMS_START)) {
		if (params_start || !params_end) {
			fprintf(stderr, "Earlier params processing still in progress. ");
			fprintf(stderr, "Can't start processing a new params.\n");
			exit(EXIT_FAILURE);
		} else {
			params_start = 1;
			params_end = 0;
			*token = PSTART;
			return 1;
		}
	} else if (!strcmp(line, PARAMS_END)) {
		if (!params_start) {
			fprintf(stderr, "Cannot put end braces without start braces. ");
			fprintf(stderr, "Please check the documentation for reference config file syntax\n");
			exit(EXIT_FAILURE);
		} else {
			params_start = 0;
			params_end = 1;
			*token = PEND;
			return 1;
		}
	} else if (!params_start) {
		fprintf(stderr, "Params should be passed within braces. ");
		fprintf(stderr, "Please check the documentation for reference config file syntax\n");
		exit(EXIT_FAILURE);
	}

	return 0;
}

static void add_guid(efi_guid_t **guid_param, char *guid)
{
	unsigned char uuid_buf[16];

	*guid_param = malloc(sizeof(efi_guid_t));
	if (!*guid_param)
		print_and_exit(MALLOC_FAIL_STR);

	if (uuid_parse(guid, uuid_buf))
		print_and_exit("Wrong guid format\n");

	convert_uuid_to_guid(uuid_buf);
	memcpy(*guid_param, uuid_buf, sizeof(efi_guid_t));
}

static void add_string(char **dst, char *val)
{
	*dst = strdup(val);
	if (!*dst)
		print_and_exit(MALLOC_FAIL_STR);
}

static void match_and_populate_param(char *key, char *val,
				     struct efi_capsule_params *param)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(capsule_params); i++) {
		if (!strcmp(key, capsule_params[i])) {
			switch (i) {
			case 0:
				add_guid(&param->image_guid, val);
				return;
			case 1:
				param->image_index = strtoul(val, NULL, 0);
				if (param->image_index == ULONG_MAX)
					print_and_exit("Enter a valid value of index bewtween 1-255");
				return;
			case 2:
				add_string(&param->privkey_file, val);
				return;
			case 3:
				add_string(&param->cert_file, val);
				return;
			case 4:
				add_string(&param->input_file, val);
				return;
			case 5:
				add_string(&param->capsule_file, val);
				return;
			case 6:
				param->hardware_instance = strtoul(val, NULL, 0);
				if (param->hardware_instance == ULONG_MAX)
					print_and_exit("Enter a valid hardware instance value");
				return;
			case 7:
				param->monotonic_count = strtoull(val, NULL, 0);
				if (param->monotonic_count == ULLONG_MAX)
					print_and_exit("Enter a valid monotonic count value");
				return;
			case 8:
				if (!strcmp(val, "normal"))
					param->capsule = CAPSULE_NORMAL_BLOB;
				else if (!strcmp(val, "accept"))
					param->capsule = CAPSULE_ACCEPT;
				else if (!strcmp(val, "revert"))
					param->capsule = CAPSULE_REVERT;
				else
					print_and_exit("Invalid type of capsule");

				return;
			case 9:
				param->oemflags = strtoul(val, NULL, 0);
				if (param->oemflags > 0xffff)
					print_and_exit("OemFlags must be between 0x0 and 0xffff\n");
				return;
			case 10:
				param->fmp.fw_version = strtoul(val, NULL, 0);
				param->fmp.have_header = true;
				return;
			}
		}
	}

	fprintf(stderr, "Undefined param %s specified. ", key);
	fprintf(stderr, "Please check the documentation for reference config file syntax\n");
	exit(EXIT_FAILURE);
}

static int get_capsule_params(char *line, struct efi_capsule_params *params)
{
	char *key = NULL;
	char *val = NULL;
	unsigned char token;

	if (param_delim_checks(line, &token))
		return token;

	key = strtok(line, ":");
	if (key)
		val = strtok(NULL, "\0");
	else
		print_and_exit("Expect the params in a key:value pair\n");

	match_and_populate_param(key, val, params);

	return 0;
}

static char *skip_whitespace(char *line)
{
	char *ptr, *newline;

	ptr = malloc(strlen(line) + 1);
	if (!ptr)
		print_and_exit(MALLOC_FAIL_STR);

	for (newline = ptr; *line; line++)
		if (!isblank(*line))
			*ptr++ = *line;
	*ptr = '\0';
	return newline;
}

static int parse_capsule_payload_params(FILE *fp, struct efi_capsule_params *params)
{
	char *line = NULL;
	char *newline;
	size_t n = 0;
	ssize_t len;

	while ((len = getline(&line, &n, fp)) != -1) {
		if (len == 1 && line[len - 1] == '\n')
			continue;

		line[len - 1] = '\0';

		newline = skip_whitespace(line);

		if (newline[0] == '#')
			continue;

		if (get_capsule_params(newline, params) == PEND)
			return 0;
	}

	if (errno == EINVAL || errno == ENOMEM) {
		fprintf(stderr, "getline() returned an error %s reading the line\n",
			strerror(errno));
		exit(EXIT_FAILURE);
	} else if (params_start == 1 || params_end == 0) {
		fprintf(stderr, "Params should be passed within braces. ");
		fprintf(stderr, "Please check the documentation for reference config file syntax\n");
		exit(EXIT_FAILURE);
	} else {
		return -1;
	}
}

static void params_dependency_check(struct efi_capsule_params *params)
{
	/* check necessary parameters */
	if ((params->capsule == CAPSULE_NORMAL_BLOB &&
	     ((!params->input_file || !params->capsule_file ||
	       !params->image_guid) ||
	      ((params->privkey_file && !params->cert_file) ||
	       (!params->privkey_file && params->cert_file)))) ||
	    (params->capsule != CAPSULE_NORMAL_BLOB &&
	     (!params->capsule_file ||
	      (params->capsule == CAPSULE_ACCEPT && !params->image_guid) ||
	      (params->capsule == CAPSULE_REVERT && params->image_guid)))) {
		print_usage();
		exit(EXIT_FAILURE);
	}
}

static void generate_capsule(struct efi_capsule_params *params)
{
	if (params->capsule != CAPSULE_NORMAL_BLOB) {
		if (create_empty_capsule(params->capsule_file,
					 params->image_guid,
					 params->capsule ==
					 CAPSULE_ACCEPT) < 0)
			print_and_exit("Creating empty capsule failed\n");
	} else if (create_fwbin(params->capsule_file, params->input_file,
				params->image_guid, params->image_index,
				params->hardware_instance,
				&params->fmp,
				params->monotonic_count,
				params->privkey_file,
				params->cert_file,
				(uint16_t)params->oemflags) < 0) {
		print_and_exit("Creating firmware capsule failed\n");
	}
}

/**
 * capsule_with_cfg_file() - Generate capsule from config file
 * @cfg_file: Path to the config file
 *
 * Parse the capsule parameters from the config file and use the
 * parameters for generating one or more capsules.
 *
 * Return: None
 *
 */
void capsule_with_cfg_file(const char *cfg_file)
{
	FILE *fp;
	struct efi_capsule_params params = { 0 };

	fp = fopen(cfg_file, "r");
	if (!fp) {
		fprintf(stderr, "Unable to open the capsule config file %s\n",
			cfg_file);
		exit(EXIT_FAILURE);
	}

	params_start = 0;
	params_end = 1;

	while (parse_capsule_payload_params(fp, &params) != -1) {
		params_dependency_check(&params);
		generate_capsule(&params);

		memset(&params, 0, sizeof(struct efi_capsule_params));
	}
}
