# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2016, NVIDIA CORPORATION. All rights reserved.

""" Test for bind command """

import re
import pytest

def in_tree(response, name, uclass, drv, depth, last_child):
    lines = [x.strip() for x in response.splitlines()]
    leaf = ''
    if depth != 0:
        leaf = '   ' + '    ' * (depth - 1)
        if not last_child:
            leaf = leaf + r'\|'
        else:
            leaf = leaf + '`'

    leaf = leaf + '-- ' + name
    line = (r' *{:10.10} *[0-9]*  \[ [ +] \]   {:20.20}  [` |]{}$'
            .format(uclass, drv, leaf))
    prog = re.compile(line)
    for l in lines:
        if prog.match(l):
            return True
    return False

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_bind')
def test_bind_unbind_with_node(ubman):

    tree = ubman.run_command('dm tree')
    assert in_tree(tree, 'bind-test', 'simple_bus', 'simple_bus', 0, True)
    assert in_tree(tree, 'bind-test-child1', 'phy', 'phy_sandbox', 1, False)
    assert in_tree(tree, 'bind-test-child2', 'simple_bus', 'simple_bus', 1, True)

    #bind usb_ether driver (which has no compatible) to usb@1 node.
    ##New entry usb_ether should appear in the dm tree
    response = ubman.run_command('bind  /usb@1 usb_ether')
    assert response == ''
    tree = ubman.run_command('dm tree')
    assert in_tree(tree, 'usb@1', 'ethernet', 'usb_ether', 1, True)

    #Unbind child #1. No error expected and all devices should be there except for bind-test-child1
    response = ubman.run_command('unbind  /bind-test/bind-test-child1')
    assert response == ''
    tree = ubman.run_command('dm tree')
    assert in_tree(tree, 'bind-test', 'simple_bus', 'simple_bus', 0, True)
    assert 'bind-test-child1' not in tree
    assert in_tree(tree, 'bind-test-child2', 'simple_bus', 'simple_bus', 1, True)

    #bind child #1. No error expected and all devices should be there
    response = ubman.run_command('bind  /bind-test/bind-test-child1 phy_sandbox')
    assert response == ''
    tree = ubman.run_command('dm tree')
    assert in_tree(tree, 'bind-test', 'simple_bus', 'simple_bus', 0, True)
    assert in_tree(tree, 'bind-test-child1', 'phy', 'phy_sandbox', 1, True)
    assert in_tree(tree, 'bind-test-child2', 'simple_bus', 'simple_bus', 1, False)

    #Unbind child #2. No error expected and all devices should be there except for bind-test-child2
    response = ubman.run_command('unbind  /bind-test/bind-test-child2')
    assert response == ''
    tree = ubman.run_command('dm tree')
    assert in_tree(tree, 'bind-test', 'simple_bus', 'simple_bus', 0, True)
    assert in_tree(tree, 'bind-test-child1', 'phy', 'phy_sandbox', 1, True)
    assert 'bind-test-child2' not in tree


    #Bind child #2. No error expected and all devices should be there
    response = ubman.run_command('bind /bind-test/bind-test-child2 simple_bus')
    assert response == ''
    tree = ubman.run_command('dm tree')
    assert in_tree(tree, 'bind-test', 'simple_bus', 'simple_bus', 0, True)
    assert in_tree(tree, 'bind-test-child1', 'phy', 'phy_sandbox', 1, False)
    assert in_tree(tree, 'bind-test-child2', 'simple_bus', 'simple_bus', 1, True)

    #Unbind parent. No error expected. All devices should be removed and unbound
    response = ubman.run_command('unbind  /bind-test')
    assert response == ''
    tree = ubman.run_command('dm tree')
    assert 'bind-test' not in tree
    assert 'bind-test-child1' not in tree
    assert 'bind-test-child2' not in tree

    #try binding invalid node with valid driver
    response = ubman.run_command('bind  /not-a-valid-node simple_bus')
    assert response != ''
    tree = ubman.run_command('dm tree')
    assert 'not-a-valid-node' not in tree

    #try binding valid node with invalid driver
    response = ubman.run_command('bind  /bind-test not_a_driver')
    assert response != ''
    tree = ubman.run_command('dm tree')
    assert 'bind-test' not in tree

    #bind /bind-test. Device should come up as well as its children
    response = ubman.run_command('bind  /bind-test simple_bus')
    assert response == ''
    tree = ubman.run_command('dm tree')
    assert in_tree(tree, 'bind-test', 'simple_bus', 'simple_bus', 0, True)
    assert in_tree(tree, 'bind-test-child1', 'phy', 'phy_sandbox', 1, False)
    assert in_tree(tree, 'bind-test-child2', 'simple_bus', 'simple_bus', 1, True)

    response = ubman.run_command('unbind  /bind-test')
    assert response == ''

def get_next_line(tree, name):
    treelines = [x.strip() for x in tree.splitlines() if x.strip()]
    child_line = ''
    for idx, line in enumerate(treelines):
        if '-- ' + name in line:
            try:
                child_line = treelines[idx+1]
            except:
                pass
            break
    return child_line

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_bind')
@pytest.mark.singlethread
def test_bind_unbind_with_uclass(ubman):
    #bind /bind-test
    response = ubman.run_command('bind  /bind-test simple_bus')
    assert response == ''

    #make sure bind-test-child2 is there and get its uclass/index pair
    tree = ubman.run_command('dm tree')
    child2_line = [x.strip() for x in tree.splitlines() if '-- bind-test-child2' in x]
    assert len(child2_line) == 1

    child2_uclass = child2_line[0].split()[0]
    child2_index = int(child2_line[0].split()[1])

    #bind simple_bus as a child of bind-test-child2
    response = ubman.run_command(
                    'bind  {} {} simple_bus'.format(child2_uclass, child2_index))

    #check that the child is there and its uclass/index pair is right
    tree = ubman.run_command('dm tree')

    child_of_child2_line = get_next_line(tree, 'bind-test-child2')
    assert child_of_child2_line
    child_of_child2_index = int(child_of_child2_line.split()[1])
    assert in_tree(tree, 'simple_bus', 'simple_bus', 'simple_bus', 2, True)
    assert child_of_child2_index == child2_index + 1

    #unbind the child and check it has been removed
    response = ubman.run_command('unbind  simple_bus {}'.format(child_of_child2_index))
    assert response == ''
    tree = ubman.run_command('dm tree')
    assert in_tree(tree, 'bind-test-child2', 'simple_bus', 'simple_bus', 1, True)
    assert not in_tree(tree, 'simple_bus', 'simple_bus', 'simple_bus', 2, True)
    child_of_child2_line = get_next_line(tree, 'bind-test-child2')
    assert child_of_child2_line == ''

    #bind simple_bus as a child of bind-test-child2
    response = ubman.run_command(
                    'bind  {} {} simple_bus'.format(child2_uclass, child2_index))

    #check that the child is there and its uclass/index pair is right
    tree = ubman.run_command('dm tree')
    treelines = [x.strip() for x in tree.splitlines() if x.strip()]

    child_of_child2_line = get_next_line(tree, 'bind-test-child2')
    assert child_of_child2_line
    child_of_child2_index = int(child_of_child2_line.split()[1])
    assert in_tree(tree, 'simple_bus', 'simple_bus', 'simple_bus', 2, True)
    assert child_of_child2_index == child2_index + 1

    #unbind the child and check it has been removed
    response = ubman.run_command(
                    'unbind  {} {} simple_bus'.format(child2_uclass, child2_index))
    assert response == ''

    tree = ubman.run_command('dm tree')
    assert in_tree(tree, 'bind-test-child2', 'simple_bus', 'simple_bus', 1, True)

    child_of_child2_line = get_next_line(tree, 'bind-test-child2')
    assert child_of_child2_line == ''

    #unbind the child again and check it doesn't change the tree
    tree_old = ubman.run_command('dm tree')
    response = ubman.run_command(
                    'unbind  {} {} simple_bus'.format(child2_uclass, child2_index))
    tree_new = ubman.run_command('dm tree')

    assert response == ''
    assert tree_old == tree_new

    response = ubman.run_command('unbind  /bind-test')
    assert response == ''
