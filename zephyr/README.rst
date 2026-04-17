.. _NxpTouchREADME:
.. NXP Touch library module

#################
NXP Touch library for Zephyr
#################
* Based on and tested with Zephyr v4.2.0

.. contents::
   :local:
   :depth: 2


**************************
NXP Touch library environment setup
**************************

Follow these steps to set up a development environment on your machine.

#. If you haven't already, please follow `this guide <https://docs.zephyrproject.org/latest/develop/getting_started/index.html>`_ to set up a Zephyr development environment and its dependencies first.

#. Get the NXP Touch library. You can pick either of the options listed below.

    * Freestanding nxp_touch module - pulls in only the dependencies it needs including Zephyr itself.

         Run::

            1. west init -m <nxp_touch_repository_URL> --mr <nxp_touch_revision_or_branch> <folder_name>
            2. cd <folder_name>
            3. west update

    * nxp_touch as a Zephyr module - if you already have your Zephyr environment set up.

        To include nxp_touch into Zephyr, update Zephyr's ``west.yml`` file::

            projects:
            - name: nxp_touch
              url: <nxp_touch_repository_url>
              revision: <nxp_touch_revision_or_branch>
              path: modules/lib/nxp_touch

        Then run ``west update nxp_touch`` command.

*****************************
Build and run frdm_touch example
*****************************

See Zephyr's `Building, Flashing and Debugging <https://docs.zephyrproject.org/latest/develop/west/build-flash-debug.html>`_ guide if you aren't familiar with it yet.

#. To **build** a project, run:

    ::

        west build -b <board> -<path to example> -p

    For example, this compiles the frdm_touch example for a frdm-ke15z board::

        1. cd modules/lib/nxp_touch/zephyr
        2. west build -b frdm_ke15z samples/frdm_touch -p

#. To **run** a project, run:

    ::

        west flash

****************
Folder structure
****************

::

    nxp_touch/
    ├─── ...
    └─── zephyr/                        All Zephyr related files.
        └─── samples/                   Sample examples.
            └─── frdm_touch/            frdm touch library files
                ├── boards/             Board specific overlay files,containing the tsi0 & tsi1 nodes enabled.
                ├── src/                Zephyr specific library source code.
                |   └─── boards/        Board specific touch lib. config files
                ├── module.yml          Defines module name, Cmake and Kconfig locations.
                ├── CMakeList.txt       Defines module's build process.
                ├── Kconfig             Defines module's configuration.
                └── ...