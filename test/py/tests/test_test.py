# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2019, Xilinx Inc.

import pytest
import random

MAX_SIZE = 0x7FFFFFFF

def test_eq_test(u_boot_console):
	val1 = random.randint(0, MAX_SIZE)
	output = u_boot_console.run_command('test %d -eq %d && echo "true"' % (val1, val1))
	if not "true" in output:
		pytest.fail('test -eq failed')
	output = u_boot_console.run_command('test %d -eq %d && echo "true"' % (val1, ~val1))
	if "true" in output:
		pytest.fail('test -eq failed')

def test_neq_test(u_boot_console):
	val1 = random.randint(0, MAX_SIZE)
	output = u_boot_console.run_command('test %d -ne %d && echo "true"' % (val1, ~val1))
	if not "true" in output:
		pytest.fail('test -ne failed')
	output = u_boot_console.run_command('test %d -ne %d && echo "true"' % (val1, val1))
	if "true" in output:
		pytest.fail('test -ne failed')

def test_gt_test(u_boot_console):
	val1 = 0
	val2 = 0
	while val1 <= val2:
		val1 = random.randint(0, MAX_SIZE)
		val2 = random.randint(0, MAX_SIZE)
# Test hexa decimal numbers
	output = u_boot_console.run_command('test 0x%x -gt 0x%x && echo "true"' % (val1, val2))
	if not "true" in output:
		pytest.fail('test -gt failed')
# Test case negative
	output = u_boot_console.run_command('test 0x%x -gt 0x%x && echo "true"' % (val2, val1))
	if "true" in output:
		pytest.fail('test -gt failed')
# Test decimal numbers
	output = u_boot_console.run_command('test %d -gt %d && echo "true"' % (val1, val2))
	if not "true" in output:
		pytest.fail('test -gt failed')
#Test case >=
	output = u_boot_console.run_command('test 0x%x -ge 0x%x && echo "true"' % (val1, val1))
	if not "true" in output:
		pytest.fail('test -ge failed')

def test_lt_test(u_boot_console):
	val1 = 0
	val2 = 0
	while val1 >= val2:
		val1 = random.randint(0, MAX_SIZE)
		val2 = random.randint(0, MAX_SIZE)
# Test hexa decimal numbers
	output = u_boot_console.run_command('test 0x%x -lt 0x%x && echo "true"' % (val1, val2))
	if not "true" in output:
		pytest.fail('test -lt failed')
# Negative test case
	output = u_boot_console.run_command('test 0x%x -lt 0x%x && echo "true"' % (val2, val1))
	if "true" in output:
		pytest.fail('test -lt failed')
# Test decimal numbers
	output = u_boot_console.run_command('test %d -lt %d && echo "true"' % (val1, val2))
	if not "true" in output:
		pytest.fail('test -lt failed')
# Test case <=
	output = u_boot_console.run_command('test 0x%x -le 0x%x && echo "true"' % (val1, val1))
	if not "true" in output:
		pytest.fail('test -le failed')

def test_str_test(u_boot_console):
	str1 = "hello"
	str2 = "bye"
#str compare test
	output = u_boot_console.run_command('test %s > %s && echo "true"' % (str1, str2))
	if not "true" in output:
		pytest.fail('test strlen > compare failed')
	output = u_boot_console.run_command('test %s < %s && echo "true"' % (str1, str2))
	if "true" in output:
		pytest.fail('test strlen < compare failed')
	output = u_boot_console.run_command('test %s = %s && echo "true"' % (str1, str1))
	if not "true" in output:
		pytest.fail('test strlen = compare failed')
	output = u_boot_console.run_command('test %s != %s && echo "true"' % (str1, str2))
	if not "true" in output:
		pytest.fail('test strlen != compare failed')
	output = u_boot_console.run_command('test -z %s && echo "true"' % (str1))
	if "true" in output:
		pytest.fail('test strlen empty failed')
	output = u_boot_console.run_command('test -n %s && echo "true"' % (str1))
	if not "true" in output:
		pytest.fail('test strlen non empty failed')
