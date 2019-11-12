LLVM_SRC_PATH := /home/joezie/Drones/Tools/clang+llvm-7.0.1-x86_64-linux-gnu-ubuntu-16.04
LLVM_BUILD_PATH := /home/joezie/Drones/Tools/clang+llvm-7.0.1-x86_64-linux-gnu-ubuntu-16.04
LLVM_BIN_PATH 	:= $(LLVM_BUILD_PATH)/bin
CXX := $(LLVM_BIN_PATH)/clang++
CXXFLAGS := -fno-rtti -O0 -g
PLUGIN_CXXFLAGS := -fpic
LLVM_CXXFLAGS := `$(LLVM_BIN_PATH)/llvm-config --cxxflags`
LLVM_LDFLAGS := `$(LLVM_BIN_PATH)/llvm-config --ldflags --libs --system-libs`
LLVM_LDFLAGS_NOLIBS := `$(LLVM_BIN_PATH)/llvm-config --ldflags`
PLUGIN_LDFLAGS := -shared
CLANG_INCLUDES := \
	-I$(LLVM_SRC_PATH)/tools/clang/include \
	-I$(LLVM_BUILD_PATH)/tools/clang/include
CLANG_LIBS := \
	-Wl,--start-group \
	-lclangAST \
	-lclangASTMatchers \
	-lclangAnalysis \
	-lclangBasic \
	-lclangDriver \
	-lclangEdit \
	-lclangFrontend \
	-lclangFrontendTool \
	-lclangLex \
	-lclangParse \
	-lclangSema \
	-lclangEdit \
	-lclangRewrite \
	-lclangRewriteFrontend \
	-lclangStaticAnalyzerFrontend \
	-lclangStaticAnalyzerCheckers \
	-lclangStaticAnalyzerCore \
	-lclangCrossTU \
	-lclangIndex \
	-lclangSerialization \
	-lclangToolingCore \
	-lclangTooling \
	-lclangFormat \
	-Wl,--end-group

all : no_dump dump parser inserter

OPTIONS=-lclang -std=c++11

no_dump : BFDictBuilderMain_no_dump

dump : BFDictBuilderMain_dump

parser : BFDetectorMain

inserter : BFInserterMain

BFDictBuilderMain_no_dump : BFDictBuilderMain.cc
	$(CXX) $(CXXFLAGS) $(LLVM_CXXFLAGS) $(CLANG_INCLUDES) $^ \
		$(CLANG_LIBS) $(LLVM_LDFLAGS) $(OPTIONS) -o $@

BFDictBuilderMain_dump : BFDictBuilderMain.cc
	$(CXX) $(CXXFLAGS) $(LLVM_CXXFLAGS) $(CLANG_INCLUDES) $^ \
		$(CLANG_LIBS) $(LLVM_LDFLAGS) $(OPTIONS) -DDUMP -o $@

BFInserterMain : BFInserterMain.cc
	$(CXX) $(CXXFLAGS) $(LLVM_CXXFLAGS) $(CLANG_INCLUDES) $^ \
		$(CLANG_LIBS) $(LLVM_LDFLAGS) $(OPTIONS) -o $@

BFDetectorMain : BFDetectorMain.cc
	g++ -o BFDetectorMain BFDetectorMain.cc $(OPTIONS)

clean:
	rm -rf BFDictBuilderMain_no_dump BFDictBuilderMain_dump BFInserterMain BFDetectorMain

