// SPDX-License-Identifier: GPL-2.0 OR MIT

#include <test/lib.h>
#include <test/ut.h>
#include <slre.h>

struct re_test {
	const char *str;
	const char *re;
	int match;
};

static const struct re_test re_test[] = {
	{ "123", "^\\d+$", 1},
	{ "x23", "^\\d+$", 0},
	{ "banana", "^([bn]a)*$", 1},
	{ "panama", "^([bn]a)*$", 0},
	{ "xby", "^a|b", 1},
	{ "xby", "b|^a", 1},
	{ "xby", "b|c$", 1},
	{ "xby", "c$|b", 1},
	{ "", "x*$", 1},
	{ "", "^x*$", 1},
	{ "yy", "x*$", 1},
	{ "yy", "^x*$", 0},
	{ "Gadsby", "^[^eE]*$", 1},
	{ "Ernest", "^[^eE]*$", 0},
	{ "6d41f0a39d6", "^[0123456789abcdef]*$", 1 },
	/* DIGIT is 17 */
	{ "##\x11%%\x11", "^[#%\\d]*$", 0 },
	{ "##23%%45", "^[#%\\d]*$", 1 },
	{ "U-Boot", "^[B-Uo-t]*$", 0 },
	{ "U-Boot", "^[A-Zm-v-]*$", 1 },
	{ "U-Boot", "^[-A-Za-z]*$", 1 },
	/* The range --C covers both - and B. */
	{ "U-Boot", "^[--CUot]*$", 1 },
	{ "U-Boot", "^[^0-9]*$", 1 },
	{ "U-Boot", "^[^0-9<->]*$", 1 },
	{ "U-Boot", "^[^0-9<\\->]*$", 0 },
	{}
};

static int lib_slre(struct unit_test_state *uts)
{
	const struct re_test *t;

	for (t = re_test; t->str; t++) {
		struct slre slre;

		ut_assert(slre_compile(&slre, t->re));
		ut_assertf(!!slre_match(&slre, t->str, strlen(t->str), NULL) == t->match,
			   "'%s' unexpectedly %s '%s'\n", t->str,
			   t->match ? "didn't match" : "matched", t->re);
	}

	return 0;
}
LIB_TEST(lib_slre, 0);
