#!/bin/sh

# Test for U-Boot cli including command repeat

BASE="$(dirname $0)"
. $BASE/common.sh

run_test() {
	./${OUTPUT_DIR}/u-boot <<END
setenv ctrlc_ignore y
md 0

reset
END
}
check_results() {
	echo "Check results"

	grep -q 00000100 ${tmp} || fail "Command did not repeat"
}

echo "Test CLI repeat"
echo
tmp="$(tempfile)"
build_uboot
run_test >${tmp}
check_results ${tmp}
rm ${tmp}
echo "Test passed"
