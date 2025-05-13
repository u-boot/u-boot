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
