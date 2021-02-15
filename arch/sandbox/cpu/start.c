// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011-2012 The Chromium OS Authors.
 */

#include <common.h>
#include <command.h>
#include <dm/root.h>
#include <efi_loader.h>
#include <errno.h>
#include <init.h>
#include <log.h>
#include <os.h>
#include <cli.h>
#include <sort.h>
#include <asm/getopt.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/malloc.h>
#include <asm/sections.h>
#include <asm/state.h>
#include <linux/ctype.h>

DECLARE_GLOBAL_DATA_PTR;

static char **os_argv;

/* Compare two options so that they can be sorted into alphabetical order */
static int h_compare_opt(const void *p1, const void *p2)
{
	const struct sandbox_cmdline_option *opt1 = p1;
	const struct sandbox_cmdline_option *opt2 = p2;
	const char *str1, *str2;
	char flag1[2], flag2[2];

	opt1 = *(struct sandbox_cmdline_option **)p1;
	opt2 = *(struct sandbox_cmdline_option **)p2;
	flag1[1] = '\0';
	flag2[1] = '\0';

	*flag1 = opt1->flag_short < 0x100 ? opt1->flag_short : '\0';
	*flag2 = opt2->flag_short < 0x100 ? opt2->flag_short : '\0';

	str1 = *flag1 ? flag1 : opt1->flag;
	str2 = *flag2 ? flag2 : opt2->flag;

	/*
	 * Force lower-case flags to come before upper-case ones. We only
	 * support upper-case for short flags.
	 */
	if (isalpha(*str1) && isalpha(*str2) &&
	    tolower(*str1) == tolower(*str2))
		return isupper(*str1) - isupper(*str2);

	return strcasecmp(str1, str2);
}

int sandbox_early_getopt_check(void)
{
	struct sandbox_state *state = state_get_current();
	struct sandbox_cmdline_option **sb_opt = __u_boot_sandbox_option_start;
	size_t num_options = __u_boot_sandbox_option_count();
	size_t i;
	int max_arg_len, max_noarg_len;
	struct sandbox_cmdline_option **sorted_opt;
	int size;

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
		max_arg_len = max((int)strlen(sb_opt[i]->flag), max_arg_len);
	max_noarg_len = max_arg_len + 7;

	/* Sort the options */
	size = sizeof(*sorted_opt) * num_options;
	sorted_opt = malloc(size);
	if (!sorted_opt) {
		printf("No memory to sort options\n");
		os_exit(1);
	}
	memcpy(sorted_opt, sb_opt, size);
	qsort(sorted_opt, num_options, sizeof(*sorted_opt), h_compare_opt);

	for (i = 0; i < num_options; ++i) {
		struct sandbox_cmdline_option *opt = sorted_opt[i];

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

int misc_init_f(void)
{
	return sandbox_early_getopt_check();
}

static int sandbox_cmdline_cb_help(struct sandbox_state *state, const char *arg)
{
	/* just flag to sandbox_early_getopt_check to show usage */
	return 1;
}
SANDBOX_CMDLINE_OPT_SHORT(help, 'h', 0, "Display help");

#ifndef CONFIG_SPL_BUILD
int sandbox_main_loop_init(void)
{
	struct sandbox_state *state = state_get_current();

	/* Execute command if required */
	if (state->cmd || state->run_distro_boot) {
		int retval = 0;

		cli_init();

#ifdef CONFIG_CMDLINE
		if (state->cmd)
			retval = run_command_list(state->cmd, -1, 0);

		if (state->run_distro_boot)
			retval = cli_simple_run_command("run distro_bootcmd",
							0);
#endif
		if (!state->interactive)
			os_exit(retval);
	}

	return 0;
}
#endif

static int sandbox_cmdline_cb_boot(struct sandbox_state *state,
				      const char *arg)
{
	state->run_distro_boot = true;
	return 0;
}
SANDBOX_CMDLINE_OPT_SHORT(boot, 'b', 0, "Run distro boot commands");

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

static int sandbox_cmdline_cb_default_fdt(struct sandbox_state *state,
					  const char *arg)
{
	const char *fmt = "%s.dtb";
	char *fname;
	int len;

	len = strlen(state->argv[0]) + strlen(fmt) + 1;
	fname = malloc(len);
	if (!fname)
		return -ENOMEM;
	snprintf(fname, len, fmt, state->argv[0]);
	state->fdt_fname = fname;

	return 0;
}
SANDBOX_CMDLINE_OPT_SHORT(default_fdt, 'D', 0,
		"Use the default u-boot.dtb control FDT in U-Boot directory");

static int sandbox_cmdline_cb_test_fdt(struct sandbox_state *state,
				       const char *arg)
{
	const char *fmt = "/arch/sandbox/dts/test.dtb";
	char *p;
	char *fname;
	int len;

	len = strlen(state->argv[0]) + strlen(fmt) + 1;
	fname = malloc(len);
	if (!fname)
		return -ENOMEM;
	strcpy(fname, state->argv[0]);
	p = strrchr(fname, '/');
	if (!p)
		p = fname + strlen(fname);
	len -= p - fname;
	snprintf(p, len, fmt);
	state->fdt_fname = fname;

	return 0;
}
SANDBOX_CMDLINE_OPT_SHORT(test_fdt, 'T', 0,
			  "Use the test.dtb control FDT in U-Boot directory");

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

	err = os_read_ram_buf(arg);
	if (err) {
		printf("Failed to read RAM buffer '%s': %d\n", arg, err);
		return err;
	}
	state->ram_buf_read = true;

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

static int sandbox_cmdline_cb_double_lcd(struct sandbox_state *state,
					 const char *arg)
{
	state->double_lcd = true;

	return 0;
}
SANDBOX_CMDLINE_OPT_SHORT(double_lcd, 'K', 0,
			  "Double the LCD display size in each direction");

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

static int sandbox_cmdline_cb_verbose(struct sandbox_state *state,
				      const char *arg)
{
	state->show_test_output = true;
	return 0;
}
SANDBOX_CMDLINE_OPT_SHORT(verbose, 'v', 0, "Show test output");

static int sandbox_cmdline_cb_log_level(struct sandbox_state *state,
					const char *arg)
{
	state->default_log_level = simple_strtol(arg, NULL, 10);

	return 0;
}
SANDBOX_CMDLINE_OPT_SHORT(log_level, 'L', 1,
			  "Set log level (0=panic, 7=debug)");

static int sandbox_cmdline_cb_unittests(struct sandbox_state *state,
					const char *arg)
{
	state->run_unittests = true;

	return 0;
}
SANDBOX_CMDLINE_OPT_SHORT(unittests, 'u', 0, "Run unit tests");

static int sandbox_cmdline_cb_select_unittests(struct sandbox_state *state,
					       const char *arg)
{
	state->select_unittests = arg;

	return 0;
}
SANDBOX_CMDLINE_OPT_SHORT(select_unittests, 'k', 1, "Select unit tests to run");

static void setup_ram_buf(struct sandbox_state *state)
{
	/* Zero the RAM buffer if we didn't read it, to keep valgrind happy */
	if (!state->ram_buf_read)
		memset(state->ram_buf, '\0', state->ram_size);

	gd->arch.ram_buf = state->ram_buf;
	gd->ram_size = state->ram_size;
}

void state_show(struct sandbox_state *state)
{
	char **p;

	printf("Arguments:\n");
	for (p = state->argv; *p; p++)
		printf("%s ", *p);
	printf("\n");
}

void __efi_runtime EFIAPI efi_reset_system(
		enum efi_reset_type reset_type,
		efi_status_t reset_status,
		unsigned long data_size, void *reset_data)
{
	os_fd_restore();
	os_relaunch(os_argv);
}

void sandbox_reset(void)
{
	/* Do this here while it still has an effect */
	os_fd_restore();
	if (state_uninit())
		os_exit(2);

	if (dm_uninit())
		os_exit(2);

	/* Restart U-Boot */
	os_relaunch(os_argv);
}

int main(int argc, char *argv[])
{
	struct sandbox_state *state;
	gd_t data;
	int ret;

	/*
	 * Copy argv[] so that we can pass the arguments in the original
	 * sequence when resetting the sandbox.
	 */
	os_argv = calloc(argc + 1, sizeof(char *));
	if (!os_argv)
		os_exit(1);
	memcpy(os_argv, argv, sizeof(char *) * (argc + 1));

	memset(&data, '\0', sizeof(data));
	gd = &data;
	gd->arch.text_base = os_find_text_base();

	ret = state_init();
	if (ret)
		goto err;

	state = state_get_current();
	if (os_parse_args(state, argc, argv))
		return 1;

	/* Remove old memory file if required */
	if (state->ram_buf_rm && state->ram_buf_fname) {
		os_unlink(state->ram_buf_fname);
		state->write_ram_buf = false;
		state->ram_buf_fname = NULL;
	}

	ret = sandbox_read_state(state, state->state_fname);
	if (ret)
		goto err;

	ret = os_setup_signal_handlers();
	if (ret)
		goto err;

#if CONFIG_VAL(SYS_MALLOC_F_LEN)
	gd->malloc_base = CONFIG_MALLOC_F_ADDR;
#endif
#if CONFIG_IS_ENABLED(LOG)
	gd->default_log_level = state->default_log_level;
#endif
	setup_ram_buf(state);

	/*
	 * Set up the relocation offset here, since sandbox symbols are always
	 * relocated by the OS before sandbox is entered.
	 */
	gd->reloc_off = (ulong)gd->arch.text_base;

	/* sandbox test: log functions called before log_init in board_init_f */
	log_info("sandbox: starting...\n");
	log_debug("debug: %s\n", __func__);

	/* Do pre- and post-relocation init */
	board_init_f(0);

	board_init_r(gd->new_gd, 0);

	/* NOTREACHED - board_init_r() does not return */
	return 0;

err:
	printf("Error %d\n", ret);
	return 1;
}
