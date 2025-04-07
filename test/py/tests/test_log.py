# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016, Google Inc.
#
# U-Boot Verified Boot Test

"""
This tests U-Boot logging. It uses the 'log test' command with various options
and checks that the output is correct.
"""

import pytest

@pytest.mark.buildconfigspec('cmd_log')
def test_log_format(ubman):
    """Test the 'log format' and 'log rec' commands"""
    def run_with_format(fmt, expected_output):
        """Set up the log format and then write a log record

        Args:
            fmt: Format to use for 'log format'
            expected_output: Expected output from the 'log rec' command
        """
        output = ubman.run_command('log format %s' % fmt)
        assert output == ''
        output = ubman.run_command('log rec arch notice file.c 123 func msg')
        assert output == expected_output

    with ubman.log.section('format'):
        pad = int(ubman.config.buildconfig.get('config_logf_func_pad'))
        padding = ' ' * (pad - len('func'))

        run_with_format('all', f'NOTICE.arch,file.c:123-{padding}func() msg')
        output = ubman.run_command('log format')
        assert output == 'Log format: clFLfm'

        run_with_format('fm', f'{padding}func() msg')
        run_with_format('clfm', f'NOTICE.arch,{padding}func() msg')
        run_with_format('FLfm', f'file.c:123-{padding}func() msg')
        run_with_format('lm', 'NOTICE. msg')
        run_with_format('m', 'msg')

@pytest.mark.buildconfigspec('debug_uart')
@pytest.mark.boardspec('sandbox')
def test_log_dropped(ubman):
    """Test dropped 'log' message when debug_uart is activated"""

    ubman.restart_uboot()
    output = ubman.get_spawn_output().replace('\r', '')
    assert (not 'debug: main' in output)
