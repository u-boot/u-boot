// SPDX-License-Identifier: GPL-2.0+
/* Copyright (C) 2024-2025 Linaro Ltd. */

#include <command.h>
#include <env.h>
#include <image.h>
#include <net.h>
#include <lwip/altcp_tls.h>

U_BOOT_CMD(wget, 4, 1, do_wget,
	   "boot image via network using HTTP/HTTPS protocol"
#if defined(CONFIG_WGET_CACERT)
	   "\nwget cacert - configure wget root certificates"
#endif
	   ,
	   "[loadAddress] url\n"
	   "wget [loadAddress] [host:]path\n"
	   "    - load file"
#if defined(CONFIG_WGET_CACERT)
	   "\nwget cacert <address> <length>\n"
	   "    - provide CA certificates (0 0 to remove current)"
	   "\nwget cacert none|optional|required\n"
	   "    - set server certificate verification mode (default: optional)"
#if defined(CONFIG_WGET_BUILTIN_CACERT)
	   "\nwget cacert builtin\n"
	   "    - use the builtin CA certificates"
#endif
#endif
);

#if CONFIG_IS_ENABLED(WGET_CACERT) || CONFIG_IS_ENABLED(WGET_BUILTIN_CACERT)
char *cacert;
size_t cacert_size;
enum auth_mode cacert_auth_mode = AUTH_OPTIONAL;

#if CONFIG_IS_ENABLED(WGET_BUILTIN_CACERT)
extern const char builtin_cacert[];
extern const size_t builtin_cacert_size;
bool cacert_initialized;
#endif

static int _set_cacert(const void *addr, size_t sz)
{
	mbedtls_x509_crt crt;
	void *p;
	int ret;

	if (cacert)
		free(cacert);

	if (!addr) {
		cacert = NULL;
		cacert_size = 0;
		return CMD_RET_SUCCESS;
	}

	p = malloc(sz);
	if (!p)
		return CMD_RET_FAILURE;
	cacert = p;
	cacert_size = sz;

	memcpy(cacert, (void *)addr, sz);

	mbedtls_x509_crt_init(&crt);
	ret = mbedtls_x509_crt_parse(&crt, cacert, cacert_size);
	if (ret) {
		if (!wget_info->silent)
			printf("Could not parse certificates (%d)\n", ret);
		free(cacert);
		cacert = NULL;
		cacert_size = 0;
		return CMD_RET_FAILURE;
	}

#if CONFIG_IS_ENABLED(WGET_BUILTIN_CACERT)
	cacert_initialized = true;
#endif
	return CMD_RET_SUCCESS;
}

#if CONFIG_IS_ENABLED(WGET_BUILTIN_CACERT)
int set_cacert_builtin(void)
{
	cacert_auth_mode = AUTH_REQUIRED;
	return _set_cacert(builtin_cacert, builtin_cacert_size);
}
#endif
#endif  /* CONFIG_WGET_CACERT || CONFIG_WGET_BUILTIN_CACERT */

#if CONFIG_IS_ENABLED(WGET_CACERT)
static int set_auth(enum auth_mode auth)
{
	cacert_auth_mode = auth;

	return CMD_RET_SUCCESS;
}

static int set_cacert(char * const saddr, char * const ssz)
{
	ulong addr, sz;

	addr = hextoul(saddr, NULL);
	sz = hextoul(ssz, NULL);

	return _set_cacert((void *)addr, sz);
}
#endif

/*
 * Legacy syntax support
 * Convert [<server_name_or_ip>:]filename into a URL if needed
 */
static int parse_legacy_arg(char *arg, char *nurl, size_t rem)
{
	char *p = nurl;
	size_t n;
	char *col = strchr(arg, ':');
	char *env;
	char *server;
	char *path;

	if (strstr(arg, "http") == arg) {
		n = snprintf(nurl, rem, "%s", arg);
		if (n < 0 || n > rem)
			return -1;
		return 0;
	}

	n = snprintf(p, rem, "%s", "http://");
	if (n < 0 || n > rem)
		return -1;
	p += n;
	rem -= n;

	if (col) {
		n = col - arg;
		server = arg;
		path = col + 1;
	} else {
		env = env_get("httpserverip");
		if (!env)
			env = env_get("serverip");
		if (!env) {
			log_err("error: httpserver/serverip has to be set\n");
			return -1;
		}
		n = strlen(env);
		server = env;
		path = arg;
	}

	if (rem < n)
		return -1;
	strncpy(p, server, n);
	p += n;
	rem -= n;
	if (rem < 1)
		return -1;
	*p = '/';
	p++;
	rem--;
	n = strlen(path);
	if (rem < n)
		return -1;
	strncpy(p, path, n);
	p += n;
	rem -= n;
	if (rem < 1)
		return -1;
	*p = '\0';

	return 0;
}

int do_wget(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	char *end;
	char *url;
	ulong dst_addr;
	char nurl[1024];

#if CONFIG_IS_ENABLED(WGET_CACERT)
	if (argc == 4 && !strncmp(argv[1], "cacert", strlen("cacert")))
		return set_cacert(argv[2], argv[3]);
	if (argc == 3 && !strncmp(argv[1], "cacert", strlen("cacert"))) {
#if CONFIG_IS_ENABLED(WGET_BUILTIN_CACERT)
		if (!strncmp(argv[2], "builtin", strlen("builtin")))
			return set_cacert_builtin();
#endif
		if (!strncmp(argv[2], "none", strlen("none")))
			return set_auth(AUTH_NONE);
		if (!strncmp(argv[2], "optional", strlen("optional")))
			return set_auth(AUTH_OPTIONAL);
		if (!strncmp(argv[2], "required", strlen("required")))
			return set_auth(AUTH_REQUIRED);
		return CMD_RET_USAGE;
	}
#endif

	if (argc < 2 || argc > 3)
		return CMD_RET_USAGE;

	dst_addr = hextoul(argv[1], &end);
	if (end == (argv[1] + strlen(argv[1]))) {
		if (argc < 3)
			return CMD_RET_USAGE;
		url = argv[2];
	} else {
		dst_addr = image_load_addr;
		url = argv[1];
	}

	if (parse_legacy_arg(url, nurl, sizeof(nurl)))
		return CMD_RET_FAILURE;

	wget_info = &default_wget_info;
	if (wget_do_request(dst_addr, nurl))
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}
