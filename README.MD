
default: calc.exe
HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\AppKey\18


Toolchain
=========
  * gcc-arm-embedded toolchain >= 4.8-2014-q3
    * https://launchpad.net/gcc-arm-embedded

  * Command line tools (use cygwin for windows)
    * make
    * doxygen	>= 1.8.8

  * Flash programming
    * STLINK Utility (windows only)
    * https://github.com/texane/stlink  (for linux)


Eclipse settings
================
  * Download Eclipse Luna and install plug-ins:
    * Programming Languages
      * C/C++ Development Tools
    * Mobile and Device Development
      * C/C++ GCC Cross Compiler Support
      * C/C++ GDB Hardware Debugging

  * Window -> Preferences
    -> C/C++ -> Build -> Console -> Limit console output: 5000
    -> C/C++ -> Editor -> Scalability -> .. number of lines: 50000
    -> C/C++ -> Editor -> Folding -> Enable folding of preprocessor branches
    -> C/C++ -> Code Style -> Formatter -> Import: "Docs/Codestyle.xml"
    -> C/C++ -> Code Analysis -> Uncheck all options
    -> General -> Editors -> Text Editors -> Spelling -> Disable spell checking

  * Project -> Uncheck "Build Automatically" 


Import Project
--------------
  * File -> New -> Makefile Project with Existing Code
    -> Toolchain for Indexer Settigns: Cross GCC

  * Right click Attic folder
    -> Resource Configurations -> Exclude from Build... -> Select All, OK 

  * Project -> Properties -> C/C++ Build -> Environment
    -> Double-Click on PATH,   prepend path to C-Compiler:
       C:\code\gcc-arm-none-eabi-4_8-2014q3\bin;   (or similar)

  * Project -> Properties -> C/C++ General
      -> Preprocessor Include Paths, Macros, etc. -> Providers
        -> CDT GCC Build Output Parser:
           (arm-none-eabi-gcc)|([gc]\+\+)|(clang)
        -> CDT Cross GCC Built-in Compiler Settings:
           arm-none-eabi-gcc -E -P -v -dD "${INPUTS}"

  * Make Target Window, Right Click -> New...
    * Target-name: "all -j8"
    * Target-name: "clean"





