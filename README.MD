Toolchain
=========
  * gcc-arm-embedded toolchain >= 4.9-2014-q4
    * https://launchpad.net/gcc-arm-embedded

  * Other tools (use cygwin for windows)
    * python2
    * make

  * Initial flash programming
    * STM32 ST-LINK utility (windows)
    * https://github.com/texane/stlink (linux)

  * Flash programming with DFU bootloader
    * STM32 DfuSe utility (windows)
    * Openmoko dfu-util (linux)


Building
========
```bash
make boot    # Build USB DFU bootloader
make all     # Build actual keyboard application
```

Initial flash programming
=========================
Unfortunately, the STM32L052 does not have a built-in USB bootloader. So the
initial programming has to be done using a SWD adapter. A cheap STM32 discovery
board will do fine.

Windows
-------
Use the ST-LINK utility to flash `obj_boot/f00b00t.hex` and
`obj_app/nucular_kb.hex`.
   
Hint: You can also copy the ST-LINK utility to Tools/st-link and use `make`
to do the flashing:

```bash
make boot_flash
make flash
```

Linux
-----
Use Texane's stlink utility to do the flashing:

```bash
sudo st-flash --reset write obj_boot/f00b00t.bin 0x08000000
sudo st-flash --reset write obj_app/drquad.bin 0x08010000
```

Hint: Read the st-flash documentation and install udev rules
to flash without root privileges.


Flash programming with DFU bootloader
=====================================
After the USB DFU bootloader has been flashed, the application can be updated
without any external tools. Press FN+POWER on your keyboard to enter the DFU-
bootloader mode.

Windows
-------
Use ST's DfuSe Application to flash `obj_app/nucular_kb.dfu`

Linux
-----
Use Openmoko's dfu-util to flash the application:

```bash
sudo dfu-util -D obj_app/nucular_kb.dfu
```

printf()-Debugging
==================
The application uses a generic HID endpoint to print debug messages. This
endpoint is fully compatible with PJRC's [hid_listen] 
(https://www.pjrc.com/teensy/hid_listen.html) program.

The program (source and binaries) is included in the Tools directory.


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
