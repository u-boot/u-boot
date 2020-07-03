#ifndef __LINUX_KCONFIG_H
#define __LINUX_KCONFIG_H

#include <generated/autoconf.h>

/*
 * Helper macros to use CONFIG_ options in C/CPP expressions. Note that
 * these only work with boolean and tristate options.
 */

/*
 * Getting something that works in C and CPP for an arg that may or may
 * not be defined is tricky.  Here, if we have "#define CONFIG_BOOGER 1"
 * we match on the placeholder define, insert the "0," for arg1 and generate
 * the triplet (0, 1, 0).  Then the last step cherry picks the 2nd arg (a one).
 * When CONFIG_BOOGER is not defined, we generate a (... 1, 0) pair, and when
 * the last step cherry picks the 2nd arg, we get a zero.
 */
#define __ARG_PLACEHOLDER_1 0,
#define config_enabled(cfg) _config_enabled(cfg)
#define _config_enabled(value) __config_enabled(__ARG_PLACEHOLDER_##value)
#define __config_enabled(arg1_or_junk) ___config_enabled(arg1_or_junk 1, 0)
#define ___config_enabled(__ignored, val, ...) val

/*
 * IS_ENABLED(CONFIG_FOO) evaluates to 1 if CONFIG_FOO is set to 'y',
 * 0 otherwise.
 *
 */
#define IS_ENABLED(option) \
	(config_enabled(option))

/*
 * U-Boot add-on: Helper macros to reference to different macros
 * (CONFIG_ or CONFIG_SPL_ prefixed), depending on the build context.
 */

#if defined(CONFIG_TPL_BUILD)
#define _CONFIG_PREFIX TPL_
#elif defined(CONFIG_SPL_BUILD)
#define _CONFIG_PREFIX SPL_
#else
#define _CONFIG_PREFIX
#endif

#define   config_val(cfg)       _config_val(_CONFIG_PREFIX, cfg)
#define  _config_val(pfx, cfg) __config_val(pfx, cfg)
#define __config_val(pfx, cfg) CONFIG_ ## pfx ## cfg

/*
 * CONFIG_VAL(FOO) evaluates to the value of
 *  CONFIG_FOO if CONFIG_SPL_BUILD is undefined,
 *  CONFIG_SPL_FOO if CONFIG_SPL_BUILD is defined.
 *  CONFIG_TPL_FOO if CONFIG_TPL_BUILD is defined.
 */
#define CONFIG_VAL(option)  config_val(option)

/*
 * Count number of arguments to a variadic macro. Currently only need
 * it for 1, 2 or 3 arguments.
 */
#define __arg6(a1, a2, a3, a4, a5, a6, ...) a6
#define __count_args(...) __arg6(dummy, ##__VA_ARGS__, 4, 3, 2, 1, 0)

#define __concat(a, b)   ___concat(a, b)
#define ___concat(a, b)  a ## b

#define __unwrap(...) __VA_ARGS__
#define __unwrap1(case1, case0) __unwrap case1
#define __unwrap0(case1, case0) __unwrap case0

#define __CONFIG_IS_ENABLED_1(option)        __CONFIG_IS_ENABLED_3(option, (1), (0))
#define __CONFIG_IS_ENABLED_2(option, case1) __CONFIG_IS_ENABLED_3(option, case1, ())
#define __CONFIG_IS_ENABLED_3(option, case1, case0) \
	__concat(__unwrap, config_enabled(CONFIG_VAL(option))) (case1, case0)

/*
 * CONFIG_IS_ENABLED(FOO) expands to
 *  1 if CONFIG_SPL_BUILD is undefined and CONFIG_FOO is set to 'y',
 *  1 if CONFIG_SPL_BUILD is defined and CONFIG_SPL_FOO is set to 'y',
 *  1 if CONFIG_TPL_BUILD is defined and CONFIG_TPL_FOO is set to 'y',
 *  0 otherwise.
 *
 * CONFIG_IS_ENABLED(FOO, (abc)) expands to
 *  abc if CONFIG_SPL_BUILD is undefined and CONFIG_FOO is set to 'y',
 *  abc if CONFIG_SPL_BUILD is defined and CONFIG_SPL_FOO is set to 'y',
 *  abc if CONFIG_TPL_BUILD is defined and CONFIG_TPL_FOO is set to 'y',
 *  nothing otherwise.
 *
 * CONFIG_IS_ENABLED(FOO, (abc), (def)) expands to
 *  abc if CONFIG_SPL_BUILD is undefined and CONFIG_FOO is set to 'y',
 *  abc if CONFIG_SPL_BUILD is defined and CONFIG_SPL_FOO is set to 'y',
 *  abc if CONFIG_TPL_BUILD is defined and CONFIG_TPL_FOO is set to 'y',
 *  def otherwise.
 *
 * The optional second and third arguments must be parenthesized; that
 * allows one to include a trailing comma, e.g. for use in
 *
 * CONFIG_IS_ENABLED(ACME, ({.compatible = "acme,frobnozzle"},))
 *
 * which adds an entry to the array being defined if CONFIG_ACME (or
 * CONFIG_SPL_ACME/CONFIG_TPL_ACME, depending on build context) is
 * set, and nothing otherwise.
 */

#define CONFIG_IS_ENABLED(option, ...)					\
	__concat(__CONFIG_IS_ENABLED_, __count_args(option, ##__VA_ARGS__)) (option, ##__VA_ARGS__)


#endif /* __LINUX_KCONFIG_H */
