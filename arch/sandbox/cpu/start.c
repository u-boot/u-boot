/*
 * Copyright (c) 2011-2012 The Chromium OS Authors.
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <os.h>
#include <asm/getopt.h>
#include <asm/io.h>
#include <asm/sections.h>
#include <asm/state.h>

DECLARE_GLOBAL_DATA_PTR;

int sandbox_early_getopt_check(void)
{
	struct sandbox_state *state = state_get_current();
	struct sandbox_cmdline_option **sb_opt = __u_boot_sandbox_option_start;
	size_t num_options = __u_boot_sandbox_option_count();
	size_t i;
	int max_arg_len, max_noarg_len;

	/* parse_err will be a string of the faulting option */
	if (!state->parse_err)
		return 0;

	if (strcmp(state->parse_err, "help")) {
		printf("u-boot: error: failed while parsing option: %s\n"
			"\ttry running with --help for more information.\n",
			state->parse_err);
		os_exit(1);
	}

	printf(
		"u-boot, a command line test interface to U-Boot\n\n"
		"Usage: u-boot [options]\n"
		"Options:\n");

	max_arg_len = 0;
	for (i = 0; i < num_options; ++i)
		max_arg_len = max(strlen(sb_opt[i]->flag), max_arg_len);
	max_noarg_len = max_arg_len + 7;

	for (i = 0; i < num_options; ++i) {
		struct sandbox_cmdline_option *opt = sb_opt[i];

		/* first output the short flag if it has one */
		if (opt->flag_short >= 0x100)
			printf("      ");
		else
			printf("  -%c, ", opt->flag_short);

		/* then the long flag */
		if (opt->has_arg)
			printf("--%-*s <arg> ", max_arg_len, opt->flag);
		else
			printf("--%-*s", max_noarg_len, opt->flag);

		/* finally the help text */
		printf("  %s\n", opt->help);
	}

	os_exit(0);
}

static int sandbox_cmdline_cb_help(struct sandbox_state *state, const char *arg)
{
	/* just flag to sandbox_early_getopt_check to show usage */
	return 1;
}
SANDBOX_CMDLINE_OPT_SHORT(help, 'h', 0, "Display help");

int sandbox_main_loop_init(void)
{
	struct sandbox_state *state = state_get_current();

	/* Execute command if required */
	if (state->cmd) {
		run_command_list(state->cmd, -1, 0);
		if (!state->interactive)
			os_exit(state->exit_type);
	}

	return 0;
}

static int sandbox_cmdline_cb_command(struct sandbox_state *state,
				      const char *arg)
{
	state->cmd = arg;
	return 0;
}
SANDBOX_CMDLINE_OPT_SHORT(command, 'c', 1, "Execute U-Boot command");

static int sandbox_cmdline_cb_fdt(struct sandbox_state *state, const char *arg)
{
	state->fdt_fname = arg;
	return 0;
}
SANDBOX_CMDLINE_OPT_SHORT(fdt, 'd', 1, "Specify U-Boot's control FDT");

static int sandbox_cmdline_cb_interactive(struct sandbox_state *state,
					  const char *arg)
{
	state->interactive = true;
	return 0;
}

SANDBOX_CMDLINE_OPT_SHORT(interactive, 'i', 0, "Enter interactive mode");

static int sandbox_cmdline_cb_jump(struct sandbox_state *state,
				   const char *arg)
{
	/* Remember to delete this U-Boot image later */
	state->jumped_fname = arg;

	return 0;
}
SANDBOX_CMDLINE_OPT_SHORT(jump, 'j', 1, "Jumped from previous U-Boot");

static int sandbox_cmdline_cb_memory(struct sandbox_state *state,
				     const char *arg)
{
	int err;

	/* For now assume we always want to write it */
	state->write_ram_buf = true;
	state->ram_buf_fname = arg;

	if (os_read_ram_buf(arg)) {
		printf("Failed to read RAM buffer\n");
		return err;
	}

	return 0;
}
SANDBOX_CMDLINE_OPT_SHORT(memory, 'm', 1,
			  "Read/write ram_buf memory contents from file");

static int sandbox_cmdline_cb_rm_memory(struct sandbox_state *state,
					const char *arg)
{
	state->ram_buf_rm = true;

	return 0;
}
SANDBOX_CMDLINE_OPT(rm_memory, 0, "Remove memory file after reading");

static int sandbox_cmdline_cb_state(struct sandbox_state *state,
				    const char *arg)
{
	state->state_fname = arg;
	return 0;
}
SANDBOX_CMDLINE_OPT_SHORT(state, 's', 1, "Specify the sandbox state FDT");

static int sandbox_cmdline_cb_read(struct sandbox_state *state,
				   const char *arg)
{
	state->read_state = true;
	return 0;
}
SANDBOX_CMDLINE_OPT_SHORT(read, 'r', 0, "Read the state FDT on startup");

static int sandbox_cmdline_cb_write(struct sandbox_state *state,
				    const char *arg)
{
	state->write_state = true;
	return 0;
}
SANDBOX_CMDLINE_OPT_SHORT(write, 'w', 0, "Write state FDT on exit");

static int sandbox_cmdline_cb_ignore_missing(struct sandbox_state *state,
					     const char *arg)
{
	state->ignore_missing_state_on_read = true;
	return 0;
}
SANDBOX_CMDLINE_OPT_SHORT(ignore_missing, 'n', 0,
			  "Ignore missing state on read");

static int sandbox_cmdline_cb_show_lcd(struct sandbox_state *state,
				       const char *arg)
{
	state->show_lcd = true;
	return 0;
}
SANDBOX_CMDLINE_OPT_SHORT(show_lcd, 'l', 0,
			  "Show the sandbox LCD display");

static const char *term_args[STATE_TERM_COUNT] = {
	"raw-with-sigs",
	"raw",
	"cooked",
};

static int sandbox_cmdline_cb_terminal(struct sandbox_state *state,
				       const char *arg)
{
	int i;

	for (i = 0; i < STATE_TERM_COUNT; i++) {
		if (!strcmp(arg, term_args[i])) {
			state->term_raw = i;
			return 0;
		}
	}

	printf("Unknown terminal setting '%s' (", arg);
	for (i = 0; i < STATE_TERM_COUNT; i++)
		printf("%s%s", i ? ", " : "", term_args[i]);
	puts(")\n");

	return 1;
}
SANDBOX_CMDLINE_OPT_SHORT(terminal, 't', 1,
			  "Set terminal to raw/cooked mode");

int main(int argc, char *argv[])
{
	struct sandbox_state *state;
	gd_t data;
	int ret;

	ret = state_init();
	if (ret)
		goto err;

	state = state_get_current();
	if (os_parse_args(state, argc, argv))
		return 1;

	ret = sandbox_read_state(state, state->state_fname);
	if (ret)
		goto err;

	/* Remove old memory file if required */
	if (state->ram_buf_rm && state->ram_buf_fname)
		os_unlink(state->ram_buf_fname);

	memset(&data, '\0', sizeof(data));
	gd = &data;
#ifdef CONFIG_SYS_MALLOC_F_LEN
	gd->malloc_base = CONFIG_MALLOC_F_ADDR;
#endif

	/* Do pre- and post-relocation init */
	board_init_f(0);

	board_init_r(gd->new_gd, 0);

	/* NOTREACHED - board_init_r() does not return */
	return 0;

err:
	printf("Error %d\n", ret);
	return 1;
}
