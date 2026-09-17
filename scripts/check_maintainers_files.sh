#!/bin/sh
# SPDX-License-Identifier: GPL-2.0
#
# Check that every "F:" file reference in the MAINTAINERS files exists.
#
# Every MAINTAINERS file in the tree is checked (the top-level one plus the
# per-board/per-driver MAINTAINERS files). All F: paths are interpreted
# relative to the repository root and are treated as shell globs, matching the
# way MAINTAINERS wildcards work. If a pattern matches no existing file or
# directory it is reported and the script exits non-zero.

set -u

# Move to the repository root so that F: globs (which are root relative)
# resolve correctly regardless of where the script is invoked from.
if root=$(git rev-parse --show-toplevel 2>/dev/null); then
	cd "$root" || exit 2
fi

# Collect the list of MAINTAINERS files to check. Newlines separate entries;
# MAINTAINERS paths never contain spaces.
set -f
IFS='
'
if list=$(git ls-files '*MAINTAINERS' 'MAINTAINERS' 2>/dev/null) && [ -n "$list" ]; then
	set -- $list
else
	# Fall back to a filesystem search if git is unavailable.
	set -- $(find . -name MAINTAINERS -type f | sed 's,^\./,,')
fi
unset IFS
set +f

rc=0

for maint in "$@"; do
	[ -f "$maint" ] || continue

	# Skip Buildman's test fixtures, whose F: paths are relative to the
	# test setup rather than the repository root.
	case "$maint" in
	tools/buildman/*) continue ;;
	esac

	while IFS= read -r pattern; do
		# Strip the "F:" prefix and surrounding whitespace, then drop the
		# backslashes MAINTAINERS uses to escape regex-special characters
		# (e.g. "board/k\+p/") so the pattern can be used as a plain glob.
		pattern=$(printf '%s\n' "$pattern" | sed -e 's/^F:[[:space:]]*//' -e 's/[[:space:]]*$//' -e 's/\\\(.\)/\1/g')
		[ -n "$pattern" ] || continue

		matched=0
		for path in $pattern; do
			if [ -e "$path" ]; then
				matched=1
				break
			fi
		done

		if [ "$matched" -eq 0 ]; then
			echo "error: $maint references non-existing file: $pattern" >&2
			rc=1
		fi
	done <<EOF
$(grep '^F:' "$maint")
EOF
done

if [ "$rc" -eq 0 ]; then
	echo "All MAINTAINERS F: entries exist."
fi

exit $rc
