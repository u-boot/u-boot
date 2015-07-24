/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:      GPL-2.0+
 *
 * Ethernet Switch commands
 */

#include <common.h>
#include <command.h>
#include <errno.h>
#include <ethsw.h>

static const char *ethsw_name;

#define ETHSW_PORT_STATS_HELP "ethsw [port <port_no>] statistics " \
"{ [help] | [clear] } - show an l2 switch port's statistics"

static int ethsw_port_stats_help_key_func(struct ethsw_command_def *parsed_cmd)
{
	printf(ETHSW_PORT_STATS_HELP"\n");

	return CMD_RET_SUCCESS;
}

static struct keywords_to_function {
	enum ethsw_keyword_id cmd_keyword[ETHSW_MAX_CMD_PARAMS];
	int cmd_func_offset;
	int (*keyword_function)(struct ethsw_command_def *parsed_cmd);
} ethsw_cmd_def[] = {
		{
			.cmd_keyword = {
					ethsw_id_enable,
					ethsw_id_key_end,
			},
			.cmd_func_offset = offsetof(struct ethsw_command_func,
						    port_enable),
			.keyword_function = NULL,
		}, {
			.cmd_keyword = {
					ethsw_id_disable,
					ethsw_id_key_end,
			},
			.cmd_func_offset = offsetof(struct ethsw_command_func,
						    port_disable),
			.keyword_function = NULL,
		}, {
			.cmd_keyword = {
					ethsw_id_show,
					ethsw_id_key_end,
			},
			.cmd_func_offset = offsetof(struct ethsw_command_func,
						    port_show),
			.keyword_function = NULL,
		}, {
			.cmd_keyword = {
					ethsw_id_statistics,
					ethsw_id_help,
					ethsw_id_key_end,
			},
			.cmd_func_offset = -1,
			.keyword_function = &ethsw_port_stats_help_key_func,
		}, {
			.cmd_keyword = {
					ethsw_id_statistics,
					ethsw_id_key_end,
			},
			.cmd_func_offset = offsetof(struct ethsw_command_func,
						    port_stats),
			.keyword_function = NULL,
		}, {
			.cmd_keyword = {
					ethsw_id_statistics,
					ethsw_id_clear,
					ethsw_id_key_end,
			},
			.cmd_func_offset = offsetof(struct ethsw_command_func,
						    port_stats_clear),
			.keyword_function = NULL,
		},
};

struct keywords_optional {
	int cmd_keyword[ETHSW_MAX_CMD_PARAMS];
} cmd_opt_def[] = {
		{
				.cmd_keyword = {
						ethsw_id_port,
						ethsw_id_port_no,
						ethsw_id_key_end,
				},
		},
};

static int keyword_match_gen(enum ethsw_keyword_id key_id, int argc, char
			     *const argv[], int *argc_nr,
			     struct ethsw_command_def *parsed_cmd);
static int keyword_match_port(enum ethsw_keyword_id key_id, int argc,
			      char *const argv[], int *argc_nr,
			      struct ethsw_command_def *parsed_cmd);

/*
 * Define properties for each keyword;
 * keep the order synced with enum ethsw_keyword_id
 */
struct keyword_def {
	const char *keyword_name;
	int (*match)(enum ethsw_keyword_id key_id, int argc, char *const argv[],
		     int *argc_nr, struct ethsw_command_def *parsed_cmd);
} keyword[] = {
		{
				.keyword_name = "help",
				.match = &keyword_match_gen,
		}, {
				.keyword_name = "show",
				.match = &keyword_match_gen,
		}, {
				.keyword_name = "port",
				.match = &keyword_match_port
		},  {
				.keyword_name = "enable",
				.match = &keyword_match_gen,
		}, {
				.keyword_name = "disable",
				.match = &keyword_match_gen,
		}, {
				.keyword_name = "statistics",
				.match = &keyword_match_gen,
		}, {
				.keyword_name = "clear",
				.match = &keyword_match_gen,
		},
};

/*
 * Function used by an Ethernet Switch driver to set the functions
 * that must be called by the parser when an ethsw command is given
 */
int ethsw_define_functions(const struct ethsw_command_func *cmd_func)
{
	int i;
	void **aux_p;
	int (*cmd_func_aux)(struct ethsw_command_def *);

	if (!cmd_func->ethsw_name)
		return -EINVAL;

	ethsw_name = cmd_func->ethsw_name;

	for (i = 0; i < ARRAY_SIZE(ethsw_cmd_def); i++) {
		/*
		 * get the pointer to the function send by the Ethernet Switch
		 * driver that corresponds to the proper ethsw command
		 */
		if (ethsw_cmd_def[i].keyword_function)
			continue;

		aux_p = (void *)cmd_func + ethsw_cmd_def[i].cmd_func_offset;

		cmd_func_aux = (int (*)(struct ethsw_command_def *)) *aux_p;
		ethsw_cmd_def[i].keyword_function = cmd_func_aux;
	}

	return 0;
}

/* Generic function used to match a keyword only by a string */
static int keyword_match_gen(enum ethsw_keyword_id key_id, int argc,
			     char *const argv[], int *argc_nr,
			     struct ethsw_command_def *parsed_cmd)
{
	if (strcmp(argv[*argc_nr], keyword[key_id].keyword_name) == 0) {
		parsed_cmd->cmd_to_keywords[*argc_nr] = key_id;

		return 1;
	}
	return 0;
}

/* Function used to match the command's port */
static int keyword_match_port(enum ethsw_keyword_id key_id, int argc,
			      char *const argv[], int *argc_nr,
			      struct ethsw_command_def *parsed_cmd)
{
	unsigned long val;

	if (!keyword_match_gen(key_id, argc, argv, argc_nr, parsed_cmd))
		return 0;

	if (*argc_nr + 1 >= argc)
		return 0;

	if (strict_strtoul(argv[*argc_nr + 1], 10, &val) != -EINVAL) {
		parsed_cmd->port = val;
		(*argc_nr)++;
		parsed_cmd->cmd_to_keywords[*argc_nr] = ethsw_id_port_no;
		return 1;
	}

	return 0;
}

/* Finds optional keywords and modifies *argc_va to skip them */
static void cmd_keywords_opt_check(const struct ethsw_command_def *parsed_cmd,
				   int *argc_val)
{
	int i;
	int keyw_opt_matched;
	int argc_val_max;
	int const *cmd_keyw_p;
	int const *cmd_keyw_opt_p;

	/* remember the best match */
	argc_val_max = *argc_val;

	/*
	 * check if our command's optional keywords match the optional
	 * keywords of an available command
	 */
	for (i = 0; i < ARRAY_SIZE(ethsw_cmd_def); i++) {
		keyw_opt_matched = 0;
		cmd_keyw_p = &parsed_cmd->cmd_to_keywords[keyw_opt_matched];
		cmd_keyw_opt_p = &cmd_opt_def[i].cmd_keyword[keyw_opt_matched];

		/*
		 * increase the number of keywords that
		 * matched with a command
		 */
		while (keyw_opt_matched + *argc_val <
		       parsed_cmd->cmd_keywords_nr &&
		       *cmd_keyw_opt_p != ethsw_id_key_end &&
		       *(cmd_keyw_p + *argc_val) == *cmd_keyw_opt_p) {
			keyw_opt_matched++;
			cmd_keyw_p++;
			cmd_keyw_opt_p++;
		}

		/*
		 * if all our optional command's keywords perfectly match an
		 * optional pattern, then we can move to the next defined
		 * keywords in our command; remember the one that matched the
		 * greatest number of keywords
		 */
		if (keyw_opt_matched + *argc_val <=
		    parsed_cmd->cmd_keywords_nr &&
		    *cmd_keyw_opt_p == ethsw_id_key_end &&
		    *argc_val + keyw_opt_matched > argc_val_max)
			argc_val_max = *argc_val + keyw_opt_matched;
	}

	*argc_val = argc_val_max;
}

/*
 * Finds the function to call based on keywords and
 * modifies *argc_va to skip them
 */
static void cmd_keywords_check(struct ethsw_command_def *parsed_cmd,
			       int *argc_val)
{
	int i;
	int keyw_matched;
	int *cmd_keyw_p;
	int *cmd_keyw_def_p;

	/*
	 * check if our command's keywords match the
	 * keywords of an available command
	 */
	for (i = 0; i < ARRAY_SIZE(ethsw_cmd_def); i++) {
		keyw_matched = 0;
		cmd_keyw_p = &parsed_cmd->cmd_to_keywords[keyw_matched];
		cmd_keyw_def_p = &ethsw_cmd_def[i].cmd_keyword[keyw_matched];

		/*
		 * increase the number of keywords that
		 * matched with a command
		 */
		while (keyw_matched + *argc_val < parsed_cmd->cmd_keywords_nr &&
		       *cmd_keyw_def_p != ethsw_id_key_end &&
		       *(cmd_keyw_p + *argc_val) == *cmd_keyw_def_p) {
			keyw_matched++;
			cmd_keyw_p++;
			cmd_keyw_def_p++;
		}

		/*
		 * if all our command's keywords perfectly match an
		 * available command, then we get the function we need to call
		 * to configure the Ethernet Switch
		 */
		if (keyw_matched && keyw_matched + *argc_val ==
		    parsed_cmd->cmd_keywords_nr &&
		    *cmd_keyw_def_p == ethsw_id_key_end) {
			*argc_val += keyw_matched;
			parsed_cmd->cmd_function =
					ethsw_cmd_def[i].keyword_function;
			return;
		}
	}
}

/* find all the keywords in the command */
static int keywords_find(int argc, char * const argv[],
			 struct ethsw_command_def *parsed_cmd)
{
	int i;
	int j;
	int argc_val;
	int rc = CMD_RET_SUCCESS;

	for (i = 1; i < argc; i++) {
		for (j = 0; j < ethsw_id_count; j++) {
			if (keyword[j].match(j, argc, argv, &i, parsed_cmd))
				break;
		}
	}

	/* if there is no keyword match for a word, the command is invalid */
	for (i = 1; i < argc; i++)
		if (parsed_cmd->cmd_to_keywords[i] == ethsw_id_key_end)
			rc = CMD_RET_USAGE;

	parsed_cmd->cmd_keywords_nr = argc;
	argc_val = 1;

	/* get optional parameters first */
	cmd_keywords_opt_check(parsed_cmd, &argc_val);

	if (argc_val == parsed_cmd->cmd_keywords_nr)
		return CMD_RET_USAGE;

	/*
	 * check the keywords and if a match is found,
	 * get the function to call
	 */
	cmd_keywords_check(parsed_cmd, &argc_val);

	/* error if not all commands' parameters were matched */
	if (argc_val == parsed_cmd->cmd_keywords_nr) {
		if (!parsed_cmd->cmd_function) {
			printf("Command not available for: %s\n", ethsw_name);
			rc = CMD_RET_FAILURE;
		}
	} else {
		rc = CMD_RET_USAGE;
	}

	return rc;
}

static void command_def_init(struct ethsw_command_def *parsed_cmd)
{
	int i;

	for (i = 0; i < ETHSW_MAX_CMD_PARAMS; i++)
		parsed_cmd->cmd_to_keywords[i] = ethsw_id_key_end;

	parsed_cmd->port = ETHSW_CMD_PORT_ALL;
	parsed_cmd->cmd_function = NULL;
}

/* function to interpret commands starting with "ethsw " */
static int do_ethsw(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct ethsw_command_def parsed_cmd;
	int rc = CMD_RET_SUCCESS;

	if (argc == 1 || argc >= ETHSW_MAX_CMD_PARAMS)
		return CMD_RET_USAGE;

	command_def_init(&parsed_cmd);

	rc = keywords_find(argc, argv, &parsed_cmd);

	if (rc == CMD_RET_SUCCESS)
		rc = parsed_cmd.cmd_function(&parsed_cmd);

	return rc;
}

#define ETHSW_PORT_CONF_HELP "[port <port_no>] { enable | disable | show } " \
"- enable/disable a port; show shows a port's configuration"

U_BOOT_CMD(ethsw, ETHSW_MAX_CMD_PARAMS, 0, do_ethsw,
	   "Ethernet l2 switch commands",
	   ETHSW_PORT_CONF_HELP"\n"
	   ETHSW_PORT_STATS_HELP"\n"
);
