GitLab CI / U-Boot runner container
===================================

In order to have a reproducible and portable build environment for CI we use a container for building in.  This means that developers can also reproduce the CI environment, to a large degree at least, locally.  This file is located in the tools/docker directory.

The docker image supports both amd64 and arm64. Ensure that the
'docker-buildx' Debian package is installed (or the equivalent on another
distribution).

You will need a multi-platform container, otherwise this error is shown::

    ERROR: Multi-platform build is not supported for the docker driver.
    Switch to a different driver, or turn on the containerd image store, and try again.

You can add one with::

    sudo docker buildx create --name multiarch --driver docker-container --use

Building is supported on both amd64 (i.e. 64-bit x86) and arm64 machines. While
both amd64 and arm64 happen in parallel, the non-native part will take
considerably longer as it must use QEMU to emulate the foreign code.

To build the image yourself::

.. code-block:: bash

    sudo docker buildx build --platform linux/arm64/v8,linux/amd64 -t your-namespace:your-tag .

Or to use an existing container

.. code-block:: bash

    sudo docker pull trini/u-boot-gitlab-ci-runner:jammy-20240227-14Mar2024
