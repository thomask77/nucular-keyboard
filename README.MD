Toolchain
=========
  * gcc-arm-embedded toolchain >= 4.9-2014-q4
    * https://launchpad.net/gcc-arm-embedded

  * Other tools (use cygwin for windows)
    * python2
    * make

  * Flash programming
    * STM32 ST-LINK utility (windows)
    * https://github.com/texane/stlink (linux)

Eclipse settings
================
  * Download and configure Eclipse Luna:
    * Help -> Install new Software -> Add
      * Name: `http://pydev.org/updates`
      * Location: `http://pydev.org/updates`
    * Install the following plugins:
      * Programming Languages
        * C/C++ Development Tools
      * Mobile and Device Development
        * C/C++ GCC Cross Compiler Support
        * C/C++ GDB Hardware Debugging
     * PyDev
        * PyDev for Eclipse
      * Collaboration
        * Eclipse Git Team Provider

  * Window -> Preferences
    * C/C++
      * Build -> Console -> Limit console output: `5000`
      * Editor -> Scalability -> .. number of lines: `50000`
      * Editor -> Folding -> Enable folding of preprocessor branches
      * Code Style -> Formatter -> Import: `Docs/Codestyle.xml`
      * Code Analysis -> Uncheck all options
    * General -> Editors -> Text Editors -> Spelling -> Disable spell checking

Import Project
--------------
  * File -> New -> Makefile Project with Existing Code
    -> Toolchain for Indexer Settigns: Cross GCC

  * Project -> Uncheck "Build Automatically" 
  
  * Project -> Properties -> C/C++ General
    * Preprocessor Include Paths, Macros, etc. -> Providers
      * CDT GCC Build Output Parser: `arm-none-eabi-gcc`
      * CDT Cross GCC Built-in Compiler Settings: `arm-none-eabi-gcc -E -P -v -dD "${INPUTS}"`

  * Make Target Window, Right Click -> New...
    * Target-name: `all -j8`
    * Target-name: `clean`

Flashing
========

Windows
-------
Copy the ST-LINK utility to the Tools/st-link directory.

    make boot_flash
    make flash

Linux
-----
Make sure to install the udev rules, so you can flash without root
privileges.

    st-flash --reset write obj_boot/f00b00t.bin 0x08000000
    st-flash --reset write obj_app/drquad.bin 0x08010000
    
