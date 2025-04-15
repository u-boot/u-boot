# SPDX-License-Identifier:      GPL-2.0+

""" Unit test for xxd command
"""

import pytest
from subprocess import call, check_call, CalledProcessError
from tests import fs_helper

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_xxd')
def test_xxd(ubman):
    """ Unit test for xxd

    Args:
        ubman -- U-Boot console
    """
    try:
        scratch_dir = ubman.config.persistent_data_dir + '/scratch'

        check_call('mkdir -p %s' % scratch_dir, shell=True)

        with open(scratch_dir + '/hello', 'w', encoding = 'ascii') as file:
            file.write('hello world\n\x00\x01\x02\x03\x04\x05')

        xxd_data = fs_helper.mk_fs(ubman.config, 'vfat', 0x100000,
                                   'test_xxd', scratch_dir)
        response = ubman.run_command_list([ f'host bind 0 {xxd_data}',
                                                    'xxd host 0 hello'])

        assert '00000000: 68 65 6c 6c 6f 20 77 6f 72 6c 64 0a 00 01 02 03  hello world.....\r\r\n' + \
               '00000010: 04 05                                            ..' \
               in response
    except CalledProcessError as err:
        pytest.skip('Preparing test_xxd image failed')
        call('rm -f %s' % xxd_data, shell=True)
        return
    finally:
        call('rm -rf %s' % scratch_dir, shell=True)
        call('rm -f %s' % xxd_data, shell=True)
