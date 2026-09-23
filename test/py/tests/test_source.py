# SPDX-License-Identifier: GPL-2.0+
# Copyright (C) 2022 Sean Anderson <sean.anderson@seco.com>

import os
import pytest
import utils

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_echo')
@pytest.mark.buildconfigspec('cmd_source')
@pytest.mark.buildconfigspec('fit')
def test_source(ubman):
    # Compile our test script image
    mkimage = os.path.join(ubman.config.build_dir, 'tools/mkimage')
    its = os.path.join(ubman.config.source_dir, 'test/py/tests/source.its')
    fit = os.path.join(ubman.config.build_dir, 'source.itb')
    utils.run_and_log(ubman, (mkimage, '-f', its, fit))
    ubman.run_command(f'host load hostfs - $loadaddr {fit}')

    assert '2' in ubman.run_command('source')
    assert '1' in ubman.run_command('source :')
    assert '1' in ubman.run_command('source :script-1')
    assert '2' in ubman.run_command('source :script-2')
    assert 'Fail' in ubman.run_command('source :not-a-script || echo Fail')
    assert '2' in ubman.run_command('source \\#')
    assert '1' in ubman.run_command('source \\#conf-1')
    assert '2' in ubman.run_command('source \\#conf-2')

    ubman.run_command('fdt addr $loadaddr')
    ubman.run_command('fdt rm /configurations default')
    assert '1' in ubman.run_command('source')
    assert 'Fail' in ubman.run_command('source \\# || echo Fail')

    ubman.run_command('fdt rm /images default')
    assert 'Fail' in ubman.run_command('source || echo Fail')
    assert 'Fail' in ubman.run_command('source \\# || echo Fail')

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_echo')
@pytest.mark.buildconfigspec('cmd_source')
@pytest.mark.buildconfigspec('fit')
@pytest.mark.buildconfigspec('control_dtb_as_fit')
def test_source_control_dtb(ubman):
    output = ubman.run_command('source ${fdtcontroladdr}')
    assert '* default script' in output

    output = ubman.run_command('source ${fdtcontroladdr}:boot')
    assert '* default script' in output

    output = ubman.run_command('source ${fdtcontroladdr}:outer')
    assert '* outer 1' in output
    assert '* inner' in output
    assert '* outer 2' in output

    output = ubman.run_command('source ${fdtcontroladdr}:inner')
    assert '* outer' not in output
    assert '* inner' in output

    assert 'Fail' in ubman.run_command('source ${fdtcontroladdr}:no-such-script || echo Fail')

@pytest.mark.buildconfigspec('cmd_echo')
@pytest.mark.buildconfigspec('cmd_source')
@pytest.mark.buildconfigspec('fit')
@pytest.mark.notbuildconfigspec('control_dtb_as_fit')
def test_source_reject_control_dtb(ubman):
    assert 'Fail' in ubman.run_command('source ${fdtcontroladdr} || echo Fail')
    assert 'Fail' in ubman.run_command('source ${fdtcontroladdr}:boot || echo Fail')
