# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc

import pytest
import u_boot_utils as util

OF_PLATDATA_OUTPUT = '''
of-platdata probe:
bool 1
byte 05
bytearray 06 00 00
int 1
intarray 2 3 4 0
longbytearray 09 0a 0b 0c 0d 0e 0f 10 11
string message
stringarray "multi-word" "message" ""
of-platdata probe:
bool 0
byte 08
bytearray 01 23 34
int 3
intarray 5 0 0 0
longbytearray 09 00 00 00 00 00 00 00 00
string message2
stringarray "another" "multi-word" "message"
of-platdata probe:
bool 0
byte 00
bytearray 00 00 00
int 0
intarray 0 0 0 0
longbytearray 00 00 00 00 00 00 00 00 00
string <NULL>
stringarray "one" "" ""
of-platdata probe:
bool 0
byte 00
bytearray 00 00 00
int 0
intarray 0 0 0 0
longbytearray 00 00 00 00 00 00 00 00 00
string <NULL>
stringarray "spl" "" ""
'''

@pytest.mark.buildconfigspec('spl_of_platdata')
def test_ofplatdata(u_boot_console):
    """Test that of-platdata can be generated and used in sandbox"""
    cons = u_boot_console
    cons.restart_uboot_with_flags(['--show_of_platdata'])
    output = cons.get_spawn_output().replace('\r', '')
    assert OF_PLATDATA_OUTPUT in output

@pytest.mark.buildconfigspec('spl_of_platdata')
def test_spl_devicetree(u_boot_console):
    """Test content of spl device-tree"""
    cons = u_boot_console
    dtb = cons.config.build_dir + '/spl/u-boot-spl.dtb'
    fdtgrep = cons.config.build_dir + '/tools/fdtgrep'
    output = util.run_and_log(cons, [fdtgrep, '-l', dtb])

    assert "u-boot,dm-pre-reloc" not in output
    assert "u-boot,dm-pre-proper" not in output
    assert "u-boot,dm-spl" not in output
    assert "u-boot,dm-tpl" not in output

    assert "spl-test4" in output
    assert "spl-test5" not in output
    assert "spl-test6" not in output
    assert "spl-test7" in output
