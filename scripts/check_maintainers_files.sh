#!/bin/sh
# SPDX-License-Identifier: GPL-2.0
#
# Check that every "F:" file reference in MAINTAINERS exists.
#
# Each F: entry is treated as a shell glob (relative to the repo root),
# matching the way MAINTAINERS wildcards work. If a pattern matches no
# existing file or directory, it is reported and the script exits non-zero.

set -u

MAINTAINERS="${1:-MAINTAINERS}"

if [ ! -f "$MAINTAINERS" ]; then
	echo "error: cannot find $MAINTAINERS" >&2
	exit 2
fi

rc=0

while IFS= read -r pattern; do
	# Strip the "F:" prefix and surrounding whitespace.
	pattern=$(printf '%s\n' "$pattern" | sed -e 's/^F:[[:space:]]*//' -e 's/[[:space:]]*$//')
	[ -n "$pattern" ] || continue

	# Expand the pattern as a glob; if nothing matches the glob stays literal.
	matched=0
	for path in $pattern; do
		if [ -e "$path" ]; then
			matched=1
			break
		fi
	done

	if [ "$matched" -eq 0 ]; then
		echo "error: MAINTAINERS references non-existing file: $pattern" >&2
		rc=1
	fi
done <<EOF
$(grep '^F:' "$MAINTAINERS")
EOF

if [ "$rc" -eq 0 ]; then
	echo "All MAINTAINERS F: entries exist."
fi

exit $rc
