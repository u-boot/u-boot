# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2015-2016, NVIDIA CORPORATION. All rights reserved.

# Test basic shell functionality, such as commands separate by semi-colons.

import pytest

pytestmark = pytest.mark.buildconfigspec('cmd_echo')

def test_shell_execute(ubman):
    """Test any shell command."""

    response = ubman.run_command('echo hello')
    assert response.strip() == 'hello'

def test_shell_semicolon_two(ubman):
    """Test two shell commands separate by a semi-colon."""

    cmd = 'echo hello; echo world'
    response = ubman.run_command(cmd)
    # This validation method ignores the exact whitespace between the strings
    assert response.index('hello') < response.index('world')

def test_shell_semicolon_three(ubman):
    """Test three shell commands separate by a semi-colon, with variable
    expansion dependencies between them."""

    cmd = 'setenv list 1; setenv list ${list}2; setenv list ${list}3; ' + \
        'echo ${list}'
    response = ubman.run_command(cmd)
    assert response.strip() == '123'
    ubman.run_command('setenv list')

def test_shell_run(ubman):
    """Test the "run" shell command."""

    ubman.run_command('setenv foo \'setenv monty 1; setenv python 2\'')
    ubman.run_command('run foo')
    response = ubman.run_command('echo ${monty}')
    assert response.strip() == '1'
    response = ubman.run_command('echo ${python}')
    assert response.strip() == '2'
    ubman.run_command('setenv foo')
    ubman.run_command('setenv monty')
    ubman.run_command('setenv python')
