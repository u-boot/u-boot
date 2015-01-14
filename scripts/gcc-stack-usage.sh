#!/bin/sh
# Test for gcc '-fstack-usage' support
# Copyright (C) 2013, Masahiro Yamada <yamada.m@jp.panasonic.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

TMP="$$"

cat <<END | $@ -Werror -fstack-usage -x c - -c -o $TMP >/dev/null 2>&1 \
							&& echo "y"
int main(void)
{
	return 0;
}
END

rm -f $TMP $TMP.su
