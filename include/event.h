/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Events provide a general-purpose way to react to / subscribe to changes
 * within U-Boot
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __event_h
#define __event_h

/**
 * enum event_t - Types of events supported by U-Boot
 *
 * @EVT_DM_PRE_PROBE: Device is about to be probed
 */
enum event_t {
	EVT_NONE,
	EVT_TEST,

	/* Events related to driver model */
	EVT_DM_POST_INIT,
	EVT_DM_PRE_PROBE,
	EVT_DM_POST_PROBE,
	EVT_DM_PRE_REMOVE,
	EVT_DM_POST_REMOVE,

	/* Init hooks */
	EVT_MISC_INIT_F,

	EVT_COUNT
};

union event_data {
	/**
	 * struct event_data_test  - test data
	 *
	 * @signal: A value to update the state with
	 */
	struct event_data_test {
		int signal;
	} test;

	/**
	 * struct event_dm - driver model event
	 *
	 * @dev: Device this event relates to
	 */
	struct event_dm {
		struct udevice *dev;
	} dm;
};

/**
 * struct event - an event that can be sent and received
 *
 * @type: Event type
 * @data: Data for this particular event
 */
struct event {
	enum event_t type;
	union event_data data;
};

/** Function type for event handlers */
typedef int (*event_handler_t)(void *ctx, struct event *event);

/**
 * struct evspy_info - information about an event spy
 *
 * @func: Function to call when the event is activated (must be first)
 * @type: Event type
 * @id: Event id string
 */
struct evspy_info {
	event_handler_t func;
	enum event_t type;
#if CONFIG_IS_ENABLED(EVENT_DEBUG)
	const char *id;
#endif
};

/* Declare a new event spy */
#if CONFIG_IS_ENABLED(EVENT_DEBUG)
#define _ESPY_REC(_type, _func)   { _func, _type, #_func, }
#else
#define _ESPY_REC(_type, _func)   { _func, _type, }
#endif

static inline const char *event_spy_id(struct evspy_info *spy)
{
#if CONFIG_IS_ENABLED(EVENT_DEBUG)
	return spy->id;
#else
	return "?";
#endif
}

/*
 * It seems that LTO will drop list entries if it decides they are not used,
 * although the conditions that cause this are unclear.
 *
 * The example found is the following:
 *
 * static int sandbox_misc_init_f(void *ctx, struct event *event)
 * {
 *    return sandbox_early_getopt_check();
 * }
 * EVENT_SPY(EVT_MISC_INIT_F, sandbox_misc_init_f);
 *
 * where EVENT_SPY uses ll_entry_declare()
 *
 * In this case, LTO decides to drop the sandbox_misc_init_f() function
 * (which is fine) but then drops the linker-list entry too. This means
 * that the code no longer works, in this case sandbox no-longer checks its
 * command-line arguments properly.
 *
 * Without LTO, the KEEP() command in the .lds file is enough to keep the
 * entry around. But with LTO it seems that the entry has already been
 * dropped before the link script is considered.
 *
 * The only solution I can think of is to mark linker-list entries as 'used'
 * using an attribute. This should be safe, since we don't actually want to drop
 * any of these. However this does slightly limit LTO's optimisation choices.
 */
#define EVENT_SPY(_type, _func) \
	static __attribute__((used)) ll_entry_declare(struct evspy_info, \
						      _type, evspy_info) = \
	_ESPY_REC(_type, _func)

/**
 * event_register - register a new spy
 *
 * @id: Spy ID
 * @type: Event type to subscribe to
 * @func: Function to call when the event is sent
 * @ctx: Context to pass to the function
 * @return 0 if OK, -ve on error
 */
int event_register(const char *id, enum event_t type, event_handler_t func,
		   void *ctx);

/** event_show_spy_list( - Show a list of event spies */
void event_show_spy_list(void);

#if CONFIG_IS_ENABLED(EVENT)
/**
 * event_notify() - notify spies about an event
 *
 * It is possible to pass in union event_data here but that may not be
 * convenient if the data is elsewhere, or is one of the members of the union.
 * So this uses a void * for @data, with a separate @size.
 *
 * @type: Event type
 * @data: Event data to be sent (e.g. union_event_data)
 * @size: Size of data in bytes
 * @return 0 if OK, -ve on error
 */
int event_notify(enum event_t type, void *data, int size);

/**
 * event_notify_null() - notify spies about an event
 *
 * Data is NULL and the size is 0
 *
 * @type: Event type
 * @return 0 if OK, -ve on error
 */
int event_notify_null(enum event_t type);
#else
static inline int event_notify(enum event_t type, void *data, int size)
{
	return 0;
}

static inline int event_notify_null(enum event_t type)
{
	return 0;
}
#endif

#if CONFIG_IS_ENABLED(EVENT_DYNAMIC)
/**
 * event_uninit() - Clean up dynamic events
 *
 * This removes all dynamic event handlers
 */
int event_uninit(void);

/**
 * event_uninit() - Set up dynamic events
 *
 * Init a list of dynamic event handlers, so that these can be added as
 * needed
 */
int event_init(void);
#else
static inline int event_uninit(void)
{
	return 0;
}

static inline int event_init(void)
{
	return 0;
}
#endif

#endif
