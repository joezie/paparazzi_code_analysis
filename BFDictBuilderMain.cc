#include <clang-c/Index.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include "headers/BFDict.h"
//#include "headers/VisitorStruct.h"
//#include "headers/CursorWrappers.h"
//#include "headers/ASTAnalysis.h"
//#include "headers/CallbackFuncs.h"
#include "headers/HelperFuncs.h"
#include "headers/LibToolAnalysis.h"

using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace std;

/* global variables start */

CXTranslationUnit tu;
//string src_file;
vector<BFDict> dict;
unordered_map<string, string> bfNameMap; // (bounding function name, function type)
unordered_map<string, vector<int>> bvReadLineMap; // (bounded variable, read lines in src file)
unordered_map<string, vector<int>> bvWriteLineMap; // (bounded variable, write lines in src file)
unordered_map<string, int> bvDeclLineMap; // (bounded variable, declaration line in src file)
unordered_set<string> bvSet; // set of bounded variables

/* global variables end */


int main( int argc, const char** argv )
{
  if( argc < 2 ) {
    printf("Usage: %s <src_files> [options]\n", argv[0]);
    return -1;
  }

  //src_file = argv[1];
  initBFNameMap();

/*
  CXIndex index = clang_createIndex( 0, 1 );
  tu = clang_createTranslationUnit( index, argv[1] );


  if( !tu ) {
    printf("%s translation unit failed!\n", argv[1]);
    return -1;
  }


  CXCursor rootCursor  = clang_getTranslationUnitCursor( tu );

  // 1. traverse the AST to build dict
  VisitorStruct vs {0, false};
  clang_visitChildren( rootCursor, buildDict, &vs );
  printDict();


  // 2. traverse the AST to build bvREadLineMap/bvWriteLineMap/bvDeclLineMap
  bool foundBV = false;
  clang_visitChildren( rootCursor, buildBVLineMap, &foundBV );
  printBVLineMap();


  // 3. modify declarations of BV in the copied src file
  string src_file_cp = src_file + "_cp";
  modifyArgDecl(src_file, src_file_cp);
*/

  // 2. update bvWriteLineMap
  static llvm::cl::OptionCategory ToolingSampleCategory("Tooling Sample");

  //int argc_new = 2;
  //const char* argv_new[] = {argv[0], argv[1]}; // exclude the src file arg

  /* Used to parse the command line option into the form of CommonOptionsParser
   * e.g. Input: argc = 5 argv = {foo, src, -Ixxx, -lxxx, -Dxxx}
   * my_argv: {foo, src, -extra-arg=-Ixxx, -extra-arg=-lxxx, -extra-arg=-Dxxx}
   */
/*
   const char **my_argv = new const char*[argc];
   string *my_argv_strs = new string[argc];

   for (int i = 0; i < argc; i++) {
        my_argv_strs[i] = "";
        if (argv[i][0] == '-') // only concern the options with prefix -
            my_argv_strs[i] = "-extra-arg=";
        my_argv_strs[i] += string(argv[i]);
        my_argv[i] = my_argv_strs[i].c_str();
   }
*/
  CommonOptionsParser op(argc, argv, ToolingSampleCategory);
  ClangTool Tool(op.getCompilations(), op.getSourcePathList());
  //Tool.run(newFrontendActionFactory<WrittenBVDetector>().get());
  Tool.run(newFrontendActionFactory<BFDictBuilder>().get());

/*
  // 3. insert bounding functions after where BV are written, and then store result in a copied src file
  string src_file_cp = src_file + "_cp";
  modifyArgWritten(src_file, src_file_cp);


  // 4. move the copied src file to a new dir
  string newDir = "Modified_Src_Files";
  createDir(newDir); // create a new folder
  string copiedFilePath = newDir + "/" + retrieveFileName(src_file_cp);
  mvFile(src_file_cp, copiedFilePath); // move the copied file to the new dir
*/

  // 5. free space
  //clang_disposeTranslationUnit( tu );
  //clang_disposeIndex( index );
  //delete[] my_argv;
  //delete[] my_argv_strs;

  // 3. Output dict
  /* TODO: output to a file instead of to screen */
  if (!dict.empty()) {
    for (BFDict entry : dict)
      cout << entry;
  }

  return 0;
}

