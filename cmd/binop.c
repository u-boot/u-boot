// SPDX-License-Identifier: GPL-2.0+

#include <command.h>
#include <env.h>
#include <errno.h>
#include <hexdump.h>
#include <malloc.h>
#include <mapmem.h>
#include <vsprintf.h>
#include <linux/ctype.h>

enum {
	OP_ID_XOR,
	OP_ID_AND,
	OP_ID_OR,
};

static int write_to_env_var(char *varname, u8 *result, ulong len)
{
	char *str_output;
	char *str_ptr;
	int i, ret;

	str_output = malloc(len * 2 + 1);
	if (!str_output)
		return -ENOMEM;
	str_ptr = str_output;

	for (i = 0; i < len; i++) {
		sprintf(str_ptr, "%02x", result[i]);
		str_ptr += 2;
	}
	*str_ptr = '\0';
	ret = env_set(varname, str_output);
	free(str_output);

	return ret;
}

static int read_from_env_var(char *varname, u8 *result, ulong len)
{
	char *str_value;
	size_t slen;
	bool is_env;

	str_value = env_get(varname);
	is_env = !!str_value;
	if (!is_env)
		str_value = varname;

	/* Shorter hex strings leave the (zeroed) tail of result untouched */
	slen = strlen(str_value);
	if ((slen & 1) || slen / 2 > len || hex2bin(result, str_value, slen / 2)) {
		if (is_env)
			printf("Error: env var '%s' value '%s' is not a hex string of up to %lu bytes\n",
			       varname, str_value, len);
		else
			printf("Error: '%s' is not an env var or hex string of up to %lu bytes\n",
			       varname, len);
		return -EINVAL;
	}

	return 0;
}

static int read_from_mem(char *varname, u8 *result, ulong len)
{
	ulong addr;
	u8 *src;

	if (strict_strtoul(varname, 16, &addr)) {
		printf("Error: invalid address '%s'\n", varname);
		return -EINVAL;
	}
	src = map_sysmem(addr, len);
	memcpy(result, src, len);
	unmap_sysmem(src);

	return 0;
}

static int write_to_mem(char *varname, u8 *result, ulong len)
{
	ulong addr;
	u8 *buf;

	if (strict_strtoul(varname, 16, &addr)) {
		printf("Error: invalid address '%s'\n", varname);
		return -EINVAL;
	}
	buf = map_sysmem(addr, len);
	memcpy(buf, result, len);
	unmap_sysmem(buf);

	return 0;
}

static int read_src(char *arg, u8 *result, ulong len)
{
	if (*arg == '*')
		return read_from_mem(arg + 1, result, len); /* Skip asterisk */

	return read_from_env_var(arg, result, len);
}

static int do_binop(struct cmd_tbl *cmdtp, int flag, int argc,
		    char *const argv[])
{
	ulong len;
	u8 *result, *src1, *src2;
	char *oparg, *lenarg, *src1arg, *src2arg, *destarg;
	int i, op, ret = CMD_RET_FAILURE;

	if (argc < 5)
		return CMD_RET_USAGE;

	oparg = argv[1];
	lenarg = argv[2];
	src1arg = argv[3];
	src2arg = argv[4];

	if (!strcmp(oparg, "xor"))
		op = OP_ID_XOR;
	else if (!strcmp(oparg, "or"))
		op = OP_ID_OR;
	else if (!strcmp(oparg, "and"))
		op = OP_ID_AND;
	else
		return CMD_RET_USAGE;

	if (strict_strtoul(lenarg, 10, &len) || !len)
		return CMD_RET_USAGE;

	src1 = calloc(1, len);
	src2 = calloc(1, len);
	result = malloc(len);
	if (!src1 || !src2 || !result ||
	    read_src(src1arg, src1, len) || read_src(src2arg, src2, len))
		goto exit;

	switch (op) {
	case OP_ID_XOR:
		for (i = 0; i < len; i++)
			result[i] = src1[i] ^ src2[i];
		break;
	case OP_ID_OR:
		for (i = 0; i < len; i++)
			result[i] = src1[i] | src2[i];
		break;
	case OP_ID_AND:
		for (i = 0; i < len; i++)
			result[i] = src1[i] & src2[i];
		break;
	}

	if (argc == 5) {
		for (i = 0; i < len; i++) {
			printf("%02x ", result[i]);
			if (i % 16 == 15 || i == len - 1)
				puts("\n");
		}

		ret = CMD_RET_SUCCESS;
		goto exit;
	}

	destarg = argv[5];

	if (*destarg == '*')
		ret = write_to_mem(destarg + 1, result, len); /* Skip asterisk */
	else
		ret = write_to_env_var(destarg, result, len);
exit:
	free(result);
	free(src2);
	free(src1);

	return ret ? CMD_RET_FAILURE : CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	binop,	6,	1,	do_binop,
	"compute binary operation",
	"op count [*]src1 [*]src2 [[*]dest]\n"
		"    - compute binary operation of data at/in src1 and\n      src2 (either *memaddr, env var name or hex string)\n      and store result in/at dest, where op is one of\n      xor, or, and."
);
