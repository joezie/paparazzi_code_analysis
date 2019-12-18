* BFDictBuilderPlugin.cpp: "plugin" version of bounding function dictionary builder
* BFDictBuilderPlugin_PX4.cpp: modified version for PX4

1. Run "make" to generate "no_dump" version of BFDictBuilderPlugin
   Before running "make", you need to modify "LLVM_SRC_PATH" & "LLVM_BUILD_PATH" in Makefile. Please follow the instruction in Makefile.

   * Other make options:
   	- make dump_version: generate "dump" version of BFDictBuilderPlugin
   	- make px4_version: generate "PX4" version of BFDictBuilderPlugin

2. Run "bash run.sh <cfile>" to apply this plugin to the c file to be analyzed.
