# Copyright (c) 2015 Stephen Warren
# Copyright (c) 2015-2016, NVIDIA CORPORATION. All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0

# Test operation of shell commands relating to environment variables.

import pytest

# FIXME: This might be useful for other tests;
# perhaps refactor it into ConsoleBase or some other state object?
class StateTestEnv(object):
    """Container that represents the state of all U-Boot environment variables.
    This enables quick determination of existant/non-existant variable
    names.
    """

    def __init__(self, u_boot_console):
        """Initialize a new StateTestEnv object.

        Args:
            u_boot_console: A U-Boot console.

        Returns:
            Nothing.
        """

        self.u_boot_console = u_boot_console
        self.get_env()
        self.set_var = self.get_non_existent_var()

    def get_env(self):
        """Read all current environment variables from U-Boot.

        Args:
            None.

        Returns:
            Nothing.
        """

        if self.u_boot_console.config.buildconfig.get(
                'config_version_variable', 'n') == 'y':
            with self.u_boot_console.disable_check('main_signon'):
                response = self.u_boot_console.run_command('printenv')
        else:
            response = self.u_boot_console.run_command('printenv')
        self.env = {}
        for l in response.splitlines():
            if not '=' in l:
                continue
            (var, value) = l.strip().split('=', 1)
            self.env[var] = value

    def get_existent_var(self):
        """Return the name of an environment variable that exists.

        Args:
            None.

        Returns:
            The name of an environment variable.
        """

        for var in self.env:
            return var

    def get_non_existent_var(self):
        """Return the name of an environment variable that does not exist.

        Args:
            None.

        Returns:
            The name of an environment variable.
        """

        n = 0
        while True:
            var = 'test_env_' + str(n)
            if var not in self.env:
                return var
            n += 1

ste = None
@pytest.fixture(scope='function')
def state_test_env(u_boot_console):
    """pytest fixture to provide a StateTestEnv object to tests."""

    global ste
    if not ste:
        ste = StateTestEnv(u_boot_console)
    return ste

def unset_var(state_test_env, var):
    """Unset an environment variable.

    This both executes a U-Boot shell command and updates a StateTestEnv
    object.

    Args:
        state_test_env: The StateTestEnv object to update.
        var: The variable name to unset.

    Returns:
        Nothing.
    """

    state_test_env.u_boot_console.run_command('setenv %s' % var)
    if var in state_test_env.env:
        del state_test_env.env[var]

def set_var(state_test_env, var, value):
    """Set an environment variable.

    This both executes a U-Boot shell command and updates a StateTestEnv
    object.

    Args:
        state_test_env: The StateTestEnv object to update.
        var: The variable name to set.
        value: The value to set the variable to.

    Returns:
        Nothing.
    """

    state_test_env.u_boot_console.run_command('setenv %s "%s"' % (var, value))
    state_test_env.env[var] = value

def validate_empty(state_test_env, var):
    """Validate that a variable is not set, using U-Boot shell commands.

    Args:
        var: The variable name to test.

    Returns:
        Nothing.
    """

    response = state_test_env.u_boot_console.run_command('echo $%s' % var)
    assert response == ''

def validate_set(state_test_env, var, value):
    """Validate that a variable is set, using U-Boot shell commands.

    Args:
        var: The variable name to test.
        value: The value the variable is expected to have.

    Returns:
        Nothing.
    """

    # echo does not preserve leading, internal, or trailing whitespace in the
    # value. printenv does, and hence allows more complete testing.
    response = state_test_env.u_boot_console.run_command('printenv %s' % var)
    assert response == ('%s=%s' % (var, value))

def test_env_echo_exists(state_test_env):
    """Test echoing a variable that exists."""

    var = state_test_env.get_existent_var()
    value = state_test_env.env[var]
    validate_set(state_test_env, var, value)

def test_env_echo_non_existent(state_test_env):
    """Test echoing a variable that doesn't exist."""

    var = state_test_env.set_var
    validate_empty(state_test_env, var)

def test_env_printenv_non_existent(state_test_env):
    """Test printenv error message for non-existant variables."""

    var = state_test_env.set_var
    c = state_test_env.u_boot_console
    with c.disable_check('error_notification'):
        response = c.run_command('printenv %s' % var)
    assert(response == '## Error: "%s" not defined' % var)

def test_env_unset_non_existent(state_test_env):
    """Test unsetting a nonexistent variable."""

    var = state_test_env.get_non_existent_var()
    unset_var(state_test_env, var)
    validate_empty(state_test_env, var)

def test_env_set_non_existent(state_test_env):
    """Test set a non-existant variable."""

    var = state_test_env.set_var
    value = 'foo'
    set_var(state_test_env, var, value)
    validate_set(state_test_env, var, value)

def test_env_set_existing(state_test_env):
    """Test setting an existant variable."""

    var = state_test_env.set_var
    value = 'bar'
    set_var(state_test_env, var, value)
    validate_set(state_test_env, var, value)

def test_env_unset_existing(state_test_env):
    """Test unsetting a variable."""

    var = state_test_env.set_var
    unset_var(state_test_env, var)
    validate_empty(state_test_env, var)

def test_env_expansion_spaces(state_test_env):
    """Test expanding a variable that contains a space in its value."""

    var_space = None
    var_test = None
    try:
        var_space = state_test_env.get_non_existent_var()
        set_var(state_test_env, var_space, ' ')

        var_test = state_test_env.get_non_existent_var()
        value = ' 1${%(var_space)s}${%(var_space)s} 2 ' % locals()
        set_var(state_test_env, var_test, value)
        value = ' 1   2 '
        validate_set(state_test_env, var_test, value)
    finally:
        if var_space:
            unset_var(state_test_env, var_space)
        if var_test:
            unset_var(state_test_env, var_test)
