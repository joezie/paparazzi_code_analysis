# STATEMENT
* This is a static code analysis tool that work on a modified version of paparazzi codebase (https://github.com/joezie/paparazzi_modified_code). Please pull the modified paparazzi repository if you want to apply this tool. This repository is intended for a private collaboration for research purpose.

# COMPONENT
* Directory "headers": header files in this directory are used by libClang, libTool, and plugin version of code.

* Directory "LibToolTest": just for test using libTool

* Directory "myplugins": "plugin" version of bounding function dictionary builder

* Directory "mysrc": some source files from paparazzi project (for testing)

* Directory "myast": some ast files parsed from paparazzi source files (for testing)

* Directory "Useful_Lists": script for listing files or directories of paparazzi project

* BFDictBuilderMain.cc: "libTool" version of bounding function dictionary builder

* BFDetectorMain.cc: "libClang" version of bounding function detector

* BFInserterMain.cc: "libTool" version of bounding function inserter

* BFDictBuilderMain_batch.sh: script for executing BFDictBuilderMain against the whole code base to build bounding function dictionary

* BFDictBuilderMain_cmd.sh: script for executing BFDictBuilderMain against a source file to build bounding function dictionary

* BFDetectorMain_cmd.sh: a sample script for executing BFDetectorMain

* BFInserterMain_cmd.sh: a sample script for executing BFInserterMain

* AC_COMPILE_SCRIPT.sh: a script to simulate GUI compilation on target 'ap' on all aircrafts

* Makefile: compile BFInserterMain, BFDetectorMain, and "dump" & "no_dump" versions of BFDictBuilderMain
	**Before using "make", please modify "LLVM_SRC_PATH" & "LLVM_BUILD_PATH" to your llvm build path

