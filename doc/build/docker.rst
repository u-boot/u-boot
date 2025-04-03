GitLab CI / U-Boot runner container
===================================

In order to have a reproducible and portable build environment for CI we use a container for building in.  This means that developers can also reproduce the CI environment, to a large degree at least, locally.  This file is located in the tools/docker directory.

The docker image supports both amd64 and arm64. Ensure that the
`buildx` Docker CLI plugin is installed. This is often available in your
distribution via the 'docker-buildx' or 'docker-buildx-plugin' package.

You will need a multi-platform container, otherwise this error is shown::

    ERROR: Multi-platform build is not supported for the docker driver.
    Switch to a different driver, or turn on the containerd image store, and try again.

You can add a simple one with:

.. code-block:: bash

    sudo docker buildx create --name multiarch --driver docker-container --use

This will result in a builder that will use QEMU for the non-native
architectures request in a build.  While both amd64 and arm64 happen in
parallel, the non-native part will take considerably longer as it must use QEMU
to emulate the foreign code.  An alternative, if you have accesss to reasonably
fast amd64 (i.e. 64-bit x86) and arm64 machines is:

.. code-block:: bash

    sudo docker buildx create --name multiarch-multinode --node localNode --bootstrap --use
    sudo docker buildx create --name multiarch-multinode --append --node remoteNode --bootstrap ssh://user@host

And this will result in a builder named multiarch-multinode that will build
each platform natively on each node.

To build the image yourself:

.. code-block:: bash

    sudo docker buildx build --platform linux/arm64/v8,linux/amd64 -t your-namespace:your-tag .

Or to use an existing container

.. code-block:: bash

    sudo docker pull trini/u-boot-gitlab-ci-runner:jammy-20240227-14Mar2024
