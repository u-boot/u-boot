# SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
"""
Generic document construction classes.

These classes are templates for creating documents that are not bound
to a specific usage or data model.

Copyright (C) 2022 Intel Corporation <www.intel.com>

Author: Lee, Kah Jing <kah.jing.lee@intel.com>
"""

class document(object):
    """
    An abstract document class which does not dictate
    how a document should be constructed or manipulated.

    It's sole purpose is to describe the entire document
    in smaller units
    """

    class entry(object):
        """
        An entry is the smallest unit
        """

        def __init__(self, parent):
            """ entry initialization """
            if parent != None:
                parent.add(self)

    class block(entry):
        """
        A block is the smallest collection unit
        consists of entries and blocks.
        """

        def __init__(self, parent):
            """ block initialization """
            super(document.block, self).__init__(parent)
            self.entries = []

        def add(self, entry):
            """ add entry to block """
            self.entries.append(entry)


    def __init__(self):
        """ document initialization """
        self.entries = []

    def add(self, entry):
        """ add entry to entry list """
        self.entries.append(entry)


class text(document):
    """
    A simple text document implementation
    """

    class string(document.entry):
        """
        The smallest unit of a text file is a string
        """

        def __init__(self, parent, stringString=None):
            """ string initialization """
            super(text.string, self).__init__(parent)
            self.stringString = stringString

        def __str__(self):
            """ convert None to empty string """
            if (self.stringString != None):
                return self.stringString
            else:
                return ""


    class line(string):
        """
        A line is a string with EOL character
        """

        def __str__(self):
            """ convert string with newline """
            return super(text.line, self).__str__() + "\n"

    class block(document.block):
        """
        A block of text which can be made up of
        strings or lines
        """

        def __str__(self):
            """ concatenate strings or lines """
            blockString = ""

            for entry in self.entries:
                blockString += str(entry)

            return blockString


    def __str__(self):
        """ concatenate strings or lines """
        textString = ""

        for entry in self.entries:
            textString += str(entry)

        return textString


class c_source(text):
    """
    A simple C header document implementation
    """

    class define(text.string):
        """
        C header define
        """

        def __init__(self, parent, id, token=None):
            """ c header constructor initialization """
            super(c_source.define, self).__init__(parent, id)
            self.token = token

        def __str__(self):
            """ c header to strings """
            defineString = "#define" + " " + super(c_source.define, self).__str__()

            if self.token != None:
                defineString += " " + self.token

            defineString += "\n"

            return defineString

    class comment_string(text.string):
        """
        C header comment
        """

        def __str__(self):
            """ c comment """
            return "/*" + " " + super(c_source.comment_string, self).__str__() + " " + "*/"

    class comment_line(comment_string):
        """
        C header comment with newline
        """

        def __str__(self):
            """ c comment with newline """
            return super(c_source.comment_line, self).__str__() + "\n"

    class block(text.block):
        """
        A simple C block string implementation
        """

        def __init__(self, parent, prologue=None, epilogue=None):
            """ ifdef block string implementation """
            super(c_source.block, self).__init__(parent)

            self.prologue = None
            self.epilogue = None

            if prologue != None:
                self.prologue = prologue

            if epilogue != None:
                self.epilogue = epilogue

        def __str__(self):
            """ convert ifdef to string """
            blockString = ""

            if self.prologue != None:
                blockString += str(self.prologue)

            blockString += super(c_source.block, self).__str__()

            if self.epilogue != None:
                blockString += str(self.epilogue)

            return blockString

    class comment_block(block):
        """
        A simple C header block comment implementation
        """

        def __init__(self, parent, comments):
            """ block comment initialization """
            super(c_source.comment_block, self).__init__(parent, "/*\n", " */\n")
            for comment in comments.split("\n"):
                self.add(comment)

        def add(self, entry):
            """ add line to block comment """
            super(c_source.block, self).add(" * " + entry + "\n")

    class ifndef_block(block):
        """
        A simple C header ifndef implementation
        """

        def __init__(self, parent, id):
            """ ifndef block initialization """
            prologue = text.line(None, "#ifndef" + " " + id)
            epilogue = text.block(None)
            text.string(epilogue, "#endif")
            text.string(epilogue, " ")
            c_source.comment_line(epilogue, id)
            super(c_source.ifndef_block, self).__init__(parent, prologue, epilogue)


class generated_c_source(c_source):
    """
    Caller to generate c format files using the helper classes
    """

    def __init__(self, filename):
        """ Generate c header file with license, copyright, comment,
        ifdef block
        """
        super(generated_c_source, self).__init__()

        self.entries.append(c_source.comment_line(None, "SPDX-License-Identifier: BSD-3-Clause"))
        self.entries.append(c_source.comment_block(None, "Copyright (C) 2022 Intel Corporation <www.intel.com>"))
        self.entries.append(c_source.comment_block(None, "Altera SoCFPGA Clock and PLL configuration"))
        self.entries.append(text.line(None))

        self.body = c_source.ifndef_block(None, filename)
        self.body.add(c_source.define(None, filename))
        self.entries.append(self.body)

    def add(self, entry):
        """ add content to be written into c header file """
        self.body.add(entry)
