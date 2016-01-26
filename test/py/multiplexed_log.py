# Copyright (c) 2015 Stephen Warren
# Copyright (c) 2015-2016, NVIDIA CORPORATION. All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0

# Generate an HTML-formatted log file containing multiple streams of data,
# each represented in a well-delineated/-structured fashion.

import cgi
import os.path
import shutil
import subprocess

mod_dir = os.path.dirname(os.path.abspath(__file__))

class LogfileStream(object):
    """A file-like object used to write a single logical stream of data into
    a multiplexed log file. Objects of this type should be created by factory
    functions in the Logfile class rather than directly."""

    def __init__(self, logfile, name, chained_file):
        """Initialize a new object.

        Args:
            logfile: The Logfile object to log to.
            name: The name of this log stream.
            chained_file: The file-like object to which all stream data should be
            logged to in addition to logfile. Can be None.

        Returns:
            Nothing.
        """

        self.logfile = logfile
        self.name = name
        self.chained_file = chained_file

    def close(self):
        """Dummy function so that this class is "file-like".

        Args:
            None.

        Returns:
            Nothing.
        """

        pass

    def write(self, data, implicit=False):
        """Write data to the log stream.

        Args:
            data: The data to write tot he file.
            implicit: Boolean indicating whether data actually appeared in the
                stream, or was implicitly generated. A valid use-case is to
                repeat a shell prompt at the start of each separate log
                section, which makes the log sections more readable in
                isolation.

        Returns:
            Nothing.
        """

        self.logfile.write(self, data, implicit)
        if self.chained_file:
            self.chained_file.write(data)

    def flush(self):
        """Flush the log stream, to ensure correct log interleaving.

        Args:
            None.

        Returns:
            Nothing.
        """

        self.logfile.flush()
        if self.chained_file:
            self.chained_file.flush()

class RunAndLog(object):
    """A utility object used to execute sub-processes and log their output to
    a multiplexed log file. Objects of this type should be created by factory
    functions in the Logfile class rather than directly."""

    def __init__(self, logfile, name, chained_file):
        """Initialize a new object.

        Args:
            logfile: The Logfile object to log to.
            name: The name of this log stream or sub-process.
            chained_file: The file-like object to which all stream data should
                be logged to in addition to logfile. Can be None.

        Returns:
            Nothing.
        """

        self.logfile = logfile
        self.name = name
        self.chained_file = chained_file

    def close(self):
        """Clean up any resources managed by this object."""
        pass

    def run(self, cmd, cwd=None, ignore_errors=False):
        """Run a command as a sub-process, and log the results.

        Args:
            cmd: The command to execute.
            cwd: The directory to run the command in. Can be None to use the
                current directory.
            ignore_errors: Indicate whether to ignore errors. If True, the
                function will simply return if the command cannot be executed
                or exits with an error code, otherwise an exception will be
                raised if such problems occur.

        Returns:
            Nothing.
        """

        msg = "+" + " ".join(cmd) + "\n"
        if self.chained_file:
            self.chained_file.write(msg)
        self.logfile.write(self, msg)

        try:
            p = subprocess.Popen(cmd, cwd=cwd,
                stdin=None, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            (stdout, stderr) = p.communicate()
            output = ''
            if stdout:
                if stderr:
                    output += 'stdout:\n'
                output += stdout
            if stderr:
                if stdout:
                    output += 'stderr:\n'
                output += stderr
            exit_status = p.returncode
            exception = None
        except subprocess.CalledProcessError as cpe:
            output = cpe.output
            exit_status = cpe.returncode
            exception = cpe
        except Exception as e:
            output = ''
            exit_status = 0
            exception = e
        if output and not output.endswith('\n'):
            output += '\n'
        if exit_status and not exception and not ignore_errors:
            exception = Exception('Exit code: ' + str(exit_status))
        if exception:
            output += str(exception) + '\n'
        self.logfile.write(self, output)
        if self.chained_file:
            self.chained_file.write(output)
        if exception:
            raise exception

class SectionCtxMgr(object):
    """A context manager for Python's "with" statement, which allows a certain
    portion of test code to be logged to a separate section of the log file.
    Objects of this type should be created by factory functions in the Logfile
    class rather than directly."""

    def __init__(self, log, marker):
        """Initialize a new object.

        Args:
            log: The Logfile object to log to.
            marker: The name of the nested log section.

        Returns:
            Nothing.
        """

        self.log = log
        self.marker = marker

    def __enter__(self):
        self.log.start_section(self.marker)

    def __exit__(self, extype, value, traceback):
        self.log.end_section(self.marker)

class Logfile(object):
    """Generates an HTML-formatted log file containing multiple streams of
    data, each represented in a well-delineated/-structured fashion."""

    def __init__(self, fn):
        """Initialize a new object.

        Args:
            fn: The filename to write to.

        Returns:
            Nothing.
        """

        self.f = open(fn, "wt")
        self.last_stream = None
        self.blocks = []
        self.cur_evt = 1
        shutil.copy(mod_dir + "/multiplexed_log.css", os.path.dirname(fn))
        self.f.write("""\
<html>
<head>
<link rel="stylesheet" type="text/css" href="multiplexed_log.css">
</head>
<body>
<tt>
""")

    def close(self):
        """Close the log file.

        After calling this function, no more data may be written to the log.

        Args:
            None.

        Returns:
            Nothing.
        """

        self.f.write("""\
</tt>
</body>
</html>
""")
        self.f.close()

    # The set of characters that should be represented as hexadecimal codes in
    # the log file.
    _nonprint = ("%" + "".join(chr(c) for c in range(0, 32) if c not in (9, 10)) +
                 "".join(chr(c) for c in range(127, 256)))

    def _escape(self, data):
        """Render data format suitable for inclusion in an HTML document.

        This includes HTML-escaping certain characters, and translating
        control characters to a hexadecimal representation.

        Args:
            data: The raw string data to be escaped.

        Returns:
            An escaped version of the data.
        """

        data = data.replace(chr(13), "")
        data = "".join((c in self._nonprint) and ("%%%02x" % ord(c)) or
                       c for c in data)
        data = cgi.escape(data)
        return data

    def _terminate_stream(self):
        """Write HTML to the log file to terminate the current stream's data.

        Args:
            None.

        Returns:
            Nothing.
        """

        self.cur_evt += 1
        if not self.last_stream:
            return
        self.f.write("</pre>\n")
        self.f.write("<div class=\"stream-trailer\" id=\"" +
                     self.last_stream.name + "\">End stream: " +
                     self.last_stream.name + "</div>\n")
        self.f.write("</div>\n")
        self.last_stream = None

    def _note(self, note_type, msg):
        """Write a note or one-off message to the log file.

        Args:
            note_type: The type of note. This must be a value supported by the
                accompanying multiplexed_log.css.
            msg: The note/message to log.

        Returns:
            Nothing.
        """

        self._terminate_stream()
        self.f.write("<div class=\"" + note_type + "\">\n<pre>")
        self.f.write(self._escape(msg))
        self.f.write("\n</pre></div>\n")

    def start_section(self, marker):
        """Begin a new nested section in the log file.

        Args:
            marker: The name of the section that is starting.

        Returns:
            Nothing.
        """

        self._terminate_stream()
        self.blocks.append(marker)
        blk_path = "/".join(self.blocks)
        self.f.write("<div class=\"section\" id=\"" + blk_path + "\">\n")
        self.f.write("<div class=\"section-header\" id=\"" + blk_path +
                     "\">Section: " + blk_path + "</div>\n")

    def end_section(self, marker):
        """Terminate the current nested section in the log file.

        This function validates proper nesting of start_section() and
        end_section() calls. If a mismatch is found, an exception is raised.

        Args:
            marker: The name of the section that is ending.

        Returns:
            Nothing.
        """

        if (not self.blocks) or (marker != self.blocks[-1]):
            raise Exception("Block nesting mismatch: \"%s\" \"%s\"" %
                            (marker, "/".join(self.blocks)))
        self._terminate_stream()
        blk_path = "/".join(self.blocks)
        self.f.write("<div class=\"section-trailer\" id=\"section-trailer-" +
                     blk_path + "\">End section: " + blk_path + "</div>\n")
        self.f.write("</div>\n")
        self.blocks.pop()

    def section(self, marker):
        """Create a temporary section in the log file.

        This function creates a context manager for Python's "with" statement,
        which allows a certain portion of test code to be logged to a separate
        section of the log file.

        Usage:
            with log.section("somename"):
                some test code

        Args:
            marker: The name of the nested section.

        Returns:
            A context manager object.
        """

        return SectionCtxMgr(self, marker)

    def error(self, msg):
        """Write an error note to the log file.

        Args:
            msg: A message describing the error.

        Returns:
            Nothing.
        """

        self._note("error", msg)

    def warning(self, msg):
        """Write an warning note to the log file.

        Args:
            msg: A message describing the warning.

        Returns:
            Nothing.
        """

        self._note("warning", msg)

    def info(self, msg):
        """Write an informational note to the log file.

        Args:
            msg: An informational message.

        Returns:
            Nothing.
        """

        self._note("info", msg)

    def action(self, msg):
        """Write an action note to the log file.

        Args:
            msg: A message describing the action that is being logged.

        Returns:
            Nothing.
        """

        self._note("action", msg)

    def status_pass(self, msg):
        """Write a note to the log file describing test(s) which passed.

        Args:
            msg: A message describing passed test(s).

        Returns:
            Nothing.
        """

        self._note("status-pass", msg)

    def status_skipped(self, msg):
        """Write a note to the log file describing skipped test(s).

        Args:
            msg: A message describing passed test(s).

        Returns:
            Nothing.
        """

        self._note("status-skipped", msg)

    def status_fail(self, msg):
        """Write a note to the log file describing failed test(s).

        Args:
            msg: A message describing passed test(s).

        Returns:
            Nothing.
        """

        self._note("status-fail", msg)

    def get_stream(self, name, chained_file=None):
        """Create an object to log a single stream's data into the log file.

        This creates a "file-like" object that can be written to in order to
        write a single stream's data to the log file. The implementation will
        handle any required interleaving of data (from multiple streams) in
        the log, in a way that makes it obvious which stream each bit of data
        came from.

        Args:
            name: The name of the stream.
            chained_file: The file-like object to which all stream data should
                be logged to in addition to this log. Can be None.

        Returns:
            A file-like object.
        """

        return LogfileStream(self, name, chained_file)

    def get_runner(self, name, chained_file=None):
        """Create an object that executes processes and logs their output.

        Args:
            name: The name of this sub-process.
            chained_file: The file-like object to which all stream data should
                be logged to in addition to logfile. Can be None.

        Returns:
            A RunAndLog object.
        """

        return RunAndLog(self, name, chained_file)

    def write(self, stream, data, implicit=False):
        """Write stream data into the log file.

        This function should only be used by instances of LogfileStream or
        RunAndLog.

        Args:
            stream: The stream whose data is being logged.
            data: The data to log.
            implicit: Boolean indicating whether data actually appeared in the
                stream, or was implicitly generated. A valid use-case is to
                repeat a shell prompt at the start of each separate log
                section, which makes the log sections more readable in
                isolation.

        Returns:
            Nothing.
        """

        if stream != self.last_stream:
            self._terminate_stream()
            self.f.write("<div class=\"stream\" id=\"%s\">\n" % stream.name)
            self.f.write("<div class=\"stream-header\" id=\"" + stream.name +
                         "\">Stream: " + stream.name + "</div>\n")
            self.f.write("<pre>")
        if implicit:
            self.f.write("<span class=\"implicit\">")
        self.f.write(self._escape(data))
        if implicit:
            self.f.write("</span>")
        self.last_stream = stream

    def flush(self):
        """Flush the log stream, to ensure correct log interleaving.

        Args:
            None.

        Returns:
            Nothing.
        """

        self.f.flush()
