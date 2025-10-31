# SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
"""
XML node parser

Copyright (C) 2022 Intel Corporation <www.intel.com>

Author: Lee, Kah Jing <kah.jing.lee@intel.com>
"""
import xml.dom

def isElementNode(XMLNode):
    """ check if the node is element node """
    return XMLNode.nodeType == xml.dom.Node.ELEMENT_NODE

def firstElementChild(XMLNode):
    """ Calling firstChild on an Node of type Element often (always?)
    returns a Node of Text type.  How annoying!  Return the first Element
    child
    """
    child = XMLNode.firstChild
    while child != None and not isElementNode(child):
        child = nextElementSibling(child)
    return child

def nextElementSibling(XMLNode):
    """ nextElementSibling will return the next sibling of XMLNode that is
    an Element Node Type
    """
    sib = XMLNode.nextSibling
    while sib != None and not isElementNode(sib):
        sib = sib.nextSibling
    return sib
