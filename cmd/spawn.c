// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#include <command.h>
#include <console.h>
#include <env.h>
#include <malloc.h>
#include <vsprintf.h>
#include <uthread.h>

/* Spawn arguments and job index  */
struct spa {
	int argc;
	char **argv;
	unsigned int job_idx;
};

/*
 * uthread group identifiers for each running job
 * 0: job slot available, != 0: uthread group id
 * Note that job[0] is job_id 1, job[1] is job_id 2 etc.
 */
static unsigned int job[CONFIG_CMD_SPAWN_NUM_JOBS];
/* Return values of the commands run as jobs */
static enum command_ret_t job_ret[CONFIG_CMD_SPAWN_NUM_JOBS];

static void spa_free(struct spa *spa)
{
	int i;

	if (!spa)
		return;

	for (i = 0; i < spa->argc; i++)
		free(spa->argv[i]);
	free(spa->argv);
	free(spa);
}

static struct spa *spa_create(int argc, char *const argv[])
{
	struct spa *spa;
	int i;

	spa = calloc(1, sizeof(*spa));
	if (!spa)
		return NULL;
	spa->argc = argc;
	spa->argv = malloc(argc * sizeof(char *));
	if (!spa->argv)
		goto err;
	for (i = 0; i < argc; i++) {
		spa->argv[i] = strdup(argv[i]);
		if (!spa->argv[i])
			goto err;
	}
	return spa;
err:
	spa_free(spa);
	return NULL;
}

static void spawn_thread(void *arg)
{
	struct spa *spa = (struct spa *)arg;
	ulong cycles = 0;
	int repeatable = 0;

	job_ret[spa->job_idx] = cmd_process(0, spa->argc, spa->argv,
					    &repeatable, &cycles);
	spa_free(spa);
}

static unsigned int next_job_id(void)
{
	int i;

	for (i = 0; i < CONFIG_CMD_SPAWN_NUM_JOBS; i++)
		if (!job[i])
			return i + 1;

	/* No job available */
	return 0;
}

static void refresh_jobs(void)
{
	int i;

	for (i = 0; i < CONFIG_CMD_SPAWN_NUM_JOBS; i++)
		if (job[i] && uthread_grp_done(job[i]))
			job[i] = 0;

}

static int do_spawn(struct cmd_tbl *cmdtp, int flag, int argc,
		    char *const argv[])
{
	unsigned int id;
	unsigned int idx;
	struct spa *spa;
	int ret;

	if (argc == 1)
		return CMD_RET_USAGE;

	spa = spa_create(argc - 1, argv + 1);
	if (!spa)
		return CMD_RET_FAILURE;

	refresh_jobs();

	id = next_job_id();
	if (!id)
		return CMD_RET_FAILURE;
	idx = id - 1;

	job[idx] = uthread_grp_new_id();

	ret = uthread_create(NULL, spawn_thread, spa, 0, job[idx]);
	if (ret) {
		job[idx] = 0;
		return CMD_RET_FAILURE;
	}

	ret = env_set_ulong("job_id", id);
	if (ret)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(spawn, CONFIG_SYS_MAXARGS, 0, do_spawn,
	   "run commands and summarize execution time",
	   "command [args...]\n");

static enum command_ret_t wait_job(unsigned int idx)
{
	int prev = disable_ctrlc(false);

	while (!uthread_grp_done(job[idx])) {
		if (ctrlc()) {
			puts("<INTERRUPT>\n");
			disable_ctrlc(prev);
			return CMD_RET_FAILURE;
		}
		uthread_schedule();
	}

	job[idx] = 0;
	disable_ctrlc(prev);

	return job_ret[idx];
}

static int do_wait(struct cmd_tbl *cmdtp, int flag, int argc,
		   char *const argv[])
{
	enum command_ret_t ret = CMD_RET_SUCCESS;
	unsigned long id;
	unsigned int idx;
	int i;

	if (argc == 1) {
		for (i = 0; i < CONFIG_CMD_SPAWN_NUM_JOBS; i++)
			if (job[i])
				ret = wait_job(i);
	} else {
		for (i = 1; i < argc; i++) {
			id = dectoul(argv[i], NULL);
			if (id < 1 || id > CONFIG_CMD_SPAWN_NUM_JOBS)
				return CMD_RET_USAGE;
			idx = (int)id - 1;
			ret = wait_job(idx);
		}
	}

	return ret;
}

U_BOOT_CMD(wait, CONFIG_SYS_MAXARGS, 0, do_wait,
	   "wait for one or more jobs to complete",
	   "[job_id ...]\n"
	   "    - Wait until all specified jobs have exited and return the\n"
	   "      exit status of the last job waited for. When no job_id is\n"
	   "      given, wait for all the background jobs.\n");
