# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2015-2016, NVIDIA CORPORATION. All rights reserved.

"""
Logic to spawn a sub-process and interact with its stdio.
"""

import io
import os
import re
import pty
import pytest
import signal
import select
import sys
import termios
import time
import traceback

# Character to send (twice) to exit the terminal
EXIT_CHAR = 0x1d    # FS (Ctrl + ])

class Timeout(Exception):
    """An exception sub-class that indicates that a timeout occurred."""

class BootFail(Exception):
    """An exception sub-class that indicates that a boot failure occurred.

    This is used when a bad pattern is seen when waiting for the boot prompt.
    It is regarded as fatal, to avoid trying to boot the again and again to no
    avail.
    """

class Unexpected(Exception):
    """An exception sub-class that indicates that unexpected test was seen."""


def handle_exception(ubconfig, console, log, err, name, fatal, output=''):
    """Handle an exception from the console

    Exceptions can occur when there is unexpected output or due to the board
    crashing or hanging. Some exceptions are likely fatal, where retrying will
    just chew up time to no available. In those cases it is best to cause
    further tests be skipped.

    Args:
        ubconfig (ArbitraryAttributeContainer): ubconfig object
        log (Logfile): Place to log errors
        console (ConsoleBase): Console to clean up, if fatal
        err (Exception): Exception which was thrown
        name (str): Name of problem, to log
        fatal (bool): True to abort all tests
        output (str): Extra output to report on boot failure. This can show the
           target's console output as it tried to boot
    """
    msg = f'{name}: '
    if fatal:
        msg += 'Marking connection bad - no other tests will run'
    else:
        msg += 'Assuming that lab is healthy'
    print(msg)
    log.error(msg)
    log.error(f'Error: {err}')

    if output:
        msg += f'; output {output}'

    if fatal:
        ubconfig.connection_ok = False
        console.cleanup_spawn()
        pytest.exit(msg)


class Spawn:
    """Represents the stdio of a freshly created sub-process. Commands may be
    sent to the process, and responses waited for.

    Members:
        output: accumulated output from expect()
    """

    def __init__(self, args, cwd=None, decode_signal=False):
        """Spawn (fork/exec) the sub-process.

        Args:
            args: array of processs arguments. argv[0] is the command to
              execute.
            cwd: the directory to run the process in, or None for no change.
            decode_signal (bool): True to indicate the exception number when
                something goes wrong

        Returns:
            Nothing.
        """
        self.decode_signal = decode_signal
        self.waited = False
        self.exit_code = 0
        self.exit_info = ''
        self.buf = ''
        self.output = ''
        self.logfile_read = None
        self.before = ''
        self.after = ''
        self.timeout = None
        # http://stackoverflow.com/questions/7857352/python-regex-to-match-vt100-escape-sequences
        self.re_vt100 = re.compile(r'(\x1b\[|\x9b)[^@-_]*[@-_]|\x1b[@-_]', re.I)

        (self.pid, self.fd) = pty.fork()
        if self.pid == 0:
            try:
                # For some reason, SIGHUP is set to SIG_IGN at this point when
                # run under "go" (www.go.cd). Perhaps this happens under any
                # background (non-interactive) system?
                signal.signal(signal.SIGHUP, signal.SIG_DFL)
                if cwd:
                    os.chdir(cwd)
                os.execvp(args[0], args)
            except:
                print('CHILD EXECEPTION:')
                traceback.print_exc()
            finally:
                os._exit(255)

        old = None
        try:
            isatty = False
            try:
                isatty = os.isatty(sys.stdout.fileno())

            # with --capture=tee-sys we cannot call fileno()
            except io.UnsupportedOperation as exc:
                pass
            if isatty:
                new = termios.tcgetattr(self.fd)
                old = new
                new[3] = new[3] & ~(termios.ICANON | termios.ISIG)
                new[3] = new[3] & ~termios.ECHO
                new[6][termios.VMIN] = 0
                new[6][termios.VTIME] = 0
                termios.tcsetattr(self.fd, termios.TCSANOW, new)

            self.poll = select.poll()
            self.poll.register(self.fd, select.POLLIN | select.POLLPRI | select.POLLERR |
                               select.POLLHUP | select.POLLNVAL)
        except:
            if old:
                termios.tcsetattr(self.fd, termios.TCSANOW, old)
            self.close()
            raise

    def kill(self, sig):
        """Send unix signal "sig" to the child process.

        Args:
            sig: The signal number to send.

        Returns:
            Nothing.
        """

        os.kill(self.pid, sig)

    def checkalive(self):
        """Determine whether the child process is still running.

        Returns:
            tuple:
                True if process is alive, else False
                0 if process is alive, else exit code of process
                string describing what happened ('' or 'status/signal n')
        """

        if self.waited:
            return False, self.exit_code, self.exit_info

        w = os.waitpid(self.pid, os.WNOHANG)
        if w[0] == 0:
            return True, 0, 'running'
        status = w[1]

        if os.WIFEXITED(status):
            self.exit_code = os.WEXITSTATUS(status)
            self.exit_info = 'status %d' % self.exit_code
        elif os.WIFSIGNALED(status):
            signum = os.WTERMSIG(status)
            self.exit_code = -signum
            self.exit_info = 'signal %d (%s)' % (signum, signal.Signals(signum).name)
        self.waited = True
        return False, self.exit_code, self.exit_info

    def isalive(self):
        """Determine whether the child process is still running.

        Args:
            None.

        Returns:
            Boolean indicating whether process is alive.
        """
        return self.checkalive()[0]

    def send(self, data):
        """Send data to the sub-process's stdin.

        Args:
            data: The data to send to the process.

        Returns:
            Nothing.
        """

        os.write(self.fd, data.encode(errors='replace'))

    def receive(self, num_bytes):
        """Receive data from the sub-process's stdin.

        Args:
            num_bytes (int): Maximum number of bytes to read

        Returns:
            str: The data received

        Raises:
            ValueError if U-Boot died
        """
        try:
            c = os.read(self.fd, num_bytes).decode(errors='replace')
        except OSError as err:
            # With sandbox, try to detect when U-Boot exits when it
            # shouldn't and explain why. This is much more friendly than
            # just dying with an I/O error
            if self.decode_signal and err.errno == 5:  # I/O error
                alive, _, info = self.checkalive()
                if alive:
                    raise err
                raise ValueError('U-Boot exited with %s' % info)
            raise
        return c

    def expect(self, patterns):
        """Wait for the sub-process to emit specific data.

        This function waits for the process to emit one pattern from the
        supplied list of patterns, or for a timeout to occur.

        Args:
            patterns: A list of strings or regex objects that we expect to
                see in the sub-process' stdout.

        Returns:
            The index within the patterns array of the pattern the process
            emitted.

        Notable exceptions:
            Timeout, if the process did not emit any of the patterns within
            the expected time.
        """

        for pi in range(len(patterns)):
            if type(patterns[pi]) == type(''):
                patterns[pi] = re.compile(patterns[pi])

        tstart_s = time.time()
        try:
            while True:
                earliest_m = None
                earliest_pi = None
                for pi in range(len(patterns)):
                    pattern = patterns[pi]
                    m = pattern.search(self.buf)
                    if not m:
                        continue
                    if earliest_m and m.start() >= earliest_m.start():
                        continue
                    earliest_m = m
                    earliest_pi = pi
                if earliest_m:
                    pos = earliest_m.start()
                    posafter = earliest_m.end()
                    self.before = self.buf[:pos]
                    self.after = self.buf[pos:posafter]
                    self.output += self.buf[:posafter]
                    self.buf = self.buf[posafter:]
                    return earliest_pi
                tnow_s = time.time()
                if self.timeout:
                    tdelta_ms = (tnow_s - tstart_s) * 1000
                    poll_maxwait = self.timeout - tdelta_ms
                    if tdelta_ms > self.timeout:
                        raise Timeout()
                else:
                    poll_maxwait = None
                events = self.poll.poll(poll_maxwait)
                if not events:
                    raise Timeout()
                c = self.receive(1024)
                if self.logfile_read:
                    self.logfile_read.write(c)
                self.buf += c
                # count=0 is supposed to be the default, which indicates
                # unlimited substitutions, but in practice the version of
                # Python in Ubuntu 14.04 appears to default to count=2!
                self.buf = self.re_vt100.sub('', self.buf, count=1000000)
        finally:
            if self.logfile_read:
                self.logfile_read.flush()

    def close(self):
        """Close the stdio connection to the sub-process.

        This also waits a reasonable time for the sub-process to stop running.

        Args:
            None.

        Returns:
            str: Type of closure completed
        """
        # For Labgrid-sjg, ask it is exit gracefully, so it can transition the
        # board to the final state (like 'off') before exiting.
        if os.environ.get('USE_LABGRID_SJG'):
            self.send(chr(EXIT_CHAR) * 2)

            # Wait about 10 seconds for Labgrid to close and power off the board
            for _ in range(100):
                if not self.isalive():
                    return 'normal'
                time.sleep(0.1)

        # That didn't work, so try closing the PTY
        os.close(self.fd)
        for _ in range(100):
            if not self.isalive():
                return 'break'
            time.sleep(0.1)

        return 'timeout'

    def get_expect_output(self):
        """Return the output read by expect()

        Returns:
            The output processed by expect(), as a string.
        """
        return self.output
