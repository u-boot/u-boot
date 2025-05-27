# SPDX-License-Identifier: GPL-2.0+
"""
Shell command ease-ups for Python

Copyright (c) 2011 The Chromium OS Authors.
"""

import subprocess

from u_boot_pylib import cros_subprocess

# This permits interception of RunPipe for test purposes. If it is set to
# a function, then that function is called with the pipe list being
# executed. Otherwise, it is assumed to be a CommandResult object, and is
# returned as the result for every run_pipe() call.
# When this value is None, commands are executed as normal.
TEST_RESULT = None


class CommandExc(Exception):
    """Reports an exception to the caller"""
    def __init__(self, msg, result):
        """Set up a new exception object

        Args:
            result (CommandResult): Execution result so far
        """
        super().__init__(msg)
        self.result = result


class CommandResult:
    """A class which captures the result of executing a command.

    Members:
        stdout (bytes): stdout obtained from command, as a string
        stderr (bytes): stderr obtained from command, as a string
        combined (bytes): stdout and stderr interleaved
        return_code (int): Return code from command
        exception (Exception): Exception received, or None if all ok
        output (str or None): Returns output as a single line if requested
    """
    def __init__(self, stdout='', stderr='', combined='', return_code=0,
                 exception=None):
        self.stdout = stdout
        self.stderr = stderr
        self.combined = combined
        self.return_code = return_code
        self.exception = exception
        self.output = None

    def to_output(self, binary):
        """Converts binary output to its final form

        Args:
            binary (bool): True to report binary output, False to use strings
        Returns:
            self
        """
        if not binary:
            self.stdout = self.stdout.decode('utf-8')
            self.stderr = self.stderr.decode('utf-8')
            self.combined = self.combined.decode('utf-8')
        return self


def run_pipe(pipe_list, infile=None, outfile=None, capture=False,
             capture_stderr=False, oneline=False, raise_on_error=True, cwd=None,
             binary=False, output_func=None, **kwargs):
    """
    Perform a command pipeline, with optional input/output filenames.

    Args:
        pipe_list (list of list): List of command lines to execute. Each command
            line is piped into the next, and is itself a list of strings. For
            example [ ['ls', '.git'] ['wc'] ] will pipe the output of
            'ls .git' into 'wc'.
        infile (str): File to provide stdin to the pipeline
        outfile (str): File to store stdout
        capture (bool): True to capture output
        capture_stderr (bool): True to capture stderr
        oneline (bool): True to strip newline chars from output
        raise_on_error (bool): True to raise on an error, False to return it in
            the CommandResult
        cwd (str or None): Directory to run the command in
        binary (bool): True to report binary output, False to use strings
        output_func (function): Output function to call with each output
            fragment (if it returns True the function terminates)
        **kwargs: Additional keyword arguments to cros_subprocess.Popen()
    Returns:
        CommandResult object
    Raises:
        CommandExc if an exception happens
    """
    if TEST_RESULT:
        if hasattr(TEST_RESULT, '__call__'):
            # pylint: disable=E1102
            result = TEST_RESULT(pipe_list=pipe_list)
            if result:
                return result
        else:
            return TEST_RESULT
        # No result: fall through to normal processing
    result = CommandResult(b'', b'', b'')
    last_pipe = None
    pipeline = list(pipe_list)
    user_pipestr = '|'.join([' '.join(pipe) for pipe in pipe_list])
    kwargs['stdout'] = None
    kwargs['stderr'] = None
    while pipeline:
        cmd = pipeline.pop(0)
        if last_pipe is not None:
            kwargs['stdin'] = last_pipe.stdout
        elif infile:
            kwargs['stdin'] = open(infile, 'rb')
        if pipeline or capture:
            kwargs['stdout'] = cros_subprocess.PIPE
        elif outfile:
            kwargs['stdout'] = open(outfile, 'wb')
        if capture_stderr:
            kwargs['stderr'] = cros_subprocess.PIPE

        try:
            last_pipe = cros_subprocess.Popen(cmd, cwd=cwd, **kwargs)
        except Exception as err:
            result.exception = err
            if raise_on_error:
                raise CommandExc(f"Error running '{user_pipestr}': {err}",
                                 result) from err
            result.return_code = 255
            return result.to_output(binary)

    if capture:
        result.stdout, result.stderr, result.combined = (
                last_pipe.communicate_filter(output_func))
        if result.stdout and oneline:
            result.output = result.stdout.rstrip(b'\r\n')
    result.return_code = last_pipe.wait()
    if raise_on_error and result.return_code:
        raise CommandExc(f"Error running '{user_pipestr}'", result)
    return result.to_output(binary)


def output(*cmd, **kwargs):
    """Run a command and return its output

    Args:
        *cmd (list of str): Command to run
        **kwargs (dict of args): Extra arguments to pass in

    Returns:
        str: command output
    """
    kwargs['raise_on_error'] = kwargs.get('raise_on_error', True)
    return run_pipe([cmd], capture=True, **kwargs).stdout


def output_one_line(*cmd, **kwargs):
    """Run a command and output it as a single-line string

    The command is expected to produce a single line of output

    Args:
        *cmd (list of str): Command to run
        **kwargs (dict of args): Extra arguments to pass in

    Returns:
        str: output of command with all newlines removed
    """
    raise_on_error = kwargs.pop('raise_on_error', True)
    result = run_pipe([cmd], capture=True, oneline=True,
                      raise_on_error=raise_on_error, **kwargs).stdout.strip()
    return result


def run(*cmd, **kwargs):
    """Run a command

    Note that you must add 'capture' to kwargs to obtain non-empty output

    Args:
        *cmd (list of str): Command to run
        **kwargs (dict of args): Extra arguments to pass in

    Returns:
        str: output of command
    """
    return run_pipe([cmd], **kwargs).stdout


def run_one(*cmd, **kwargs):
    """Run a single command

    Note that you must add 'capture' to kwargs to obtain non-empty output

    Args:
        *cmd (list of str): Command to run
        **kwargs (dict of args): Extra arguments to pass in

    Returns:
        CommandResult: output of command
    """
    return run_pipe([cmd], **kwargs)


def run_list(cmd, **kwargs):
    """Run a command and return its output

    Args:
        cmd (list of str): Command to run

    Returns:
        str: output of command
        **kwargs (dict of args): Extra arguments to pass in
    """
    return run_pipe([cmd], capture=True, **kwargs).stdout


def stop_all():
    """Stop all subprocesses initiated with cros_subprocess"""
    cros_subprocess.stay_alive = False
