/* A Clang plugin for building a bounding function dictionary */

#include <string>
#include <iostream>
#include <fstream>
#include <unordered_set>
#include <clang-c/Index.h>
#include <cstdio>
#include <map>
#include <unordered_map>
#include "../headers/BFDict.h"
#include "../headers/HelperFuncs.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/raw_ostream.h"
#include "clang/Frontend/FrontendPluginRegistry.h"

using namespace clang;
using namespace llvm;
//using namespace clang::driver;
//using namespace clang::tooling;
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

ofstream writeToFile;


// CAUTION: please modify this path to the path on your computer
string debugFileName = "/home/joezie/Drones/paparazzi_code_analysis/myplugins/output.txt";
string dictFileName = "/home/joezie/Drones/paparazzi_code_analysis/myplugins/dict_info.txt";
/* global variables end */


//static llvm::cl::OptionCategory ToolingSampleCategory("Tooling Sample");

//std::string src_file_name_test = "../mysrc/complexAssign.cc";


/* BF Dict Builder Part Start */ 

// By overriding function VisitStmt of RecursiveASTVisitor, we can detect
// Bounding Functions, and also build dict

class BFVisitor : public RecursiveASTVisitor<BFVisitor> {
public:
  //BFVisitor(Rewriter &R, ofstream &os) : TheRewriter(R), writeToFile(os) {}
  BFVisitor(SourceManager &R) : srcMgr(R) {}

  bool VisitStmt(Stmt *s) {
    // Detect Bounding Functions and add new entries in dict

    // 1. check if its type is CallExpr
    if (!isa<CallExpr>(s))
      return true;

    // 2. get bounding function name and bounded variable name
    CallExpr *callExp = cast<CallExpr>(s);
    FunctionDecl *funcDecl = callExp -> getDirectCallee();
    if (!funcDecl)
      return true;
    
    string bfName = funcDecl -> getNameInfo().getAsString();     
    //writeToFile << "***func name: " << bfName << "***" << endl;
    
    if (bfNameMap.find(bfName) == bfNameMap.end()) // not a bounding function
      return true;
    writeToFile << "***func name: " << bfName << "***(NOT analyze yet, just found it)" << endl; // joezie 2019/10/29
      


    //unsigned startLine = TheRewriter.getSourceMgr().getSpellingLineNumber(callExp -> getBeginLoc());
    //unsigned endLine = TheRewriter.getSourceMgr().getSpellingLineNumber(callExp -> getEndLoc());
    //string srcFile = TheRewriter.getSourceMgr().getFilename(callExp -> getBeginLoc()).str();
    
    //unsigned startLine = 2540;
    //unsigned endLine = 2540;
    //string srcFile = "<placeholder>";

    unsigned startLine = srcMgr.getSpellingLineNumber(callExp -> getBeginLoc());
    unsigned endLine = srcMgr.getSpellingLineNumber(callExp -> getEndLoc());
    string srcFile = srcMgr.getFilename(callExp -> getBeginLoc()).str();



    Expr **args = callExp -> getArgs();
    unsigned argNum = callExp -> getNumArgs();
    Expr *bvExpr = args[0]; // bounded variable 
    string bvStr = getVarName(bvExpr);

    // 3. match bounding function name with bfNameMap
    BFDict entry_op; // (temporary)entry used for output to screen
    if (bfNameMap[bfName] == "commonBounds") { // (_x, _min, _max)
      if (argNum != 3) {
        writeToFile << "Error: argument number of " << bfName << " should be 3 instead of " << argNum << endl;
        return true;
	    }
      string args_str[3] = {bvStr, "", ""};
      
	    args_str[1] = getVarType(args[1]); // bound values
	    args_str[2] = getVarType(args[2]); // bound values
      
      BFDict newEntry(srcFile, bfName, args_str[0], args_str[1], args_str[2], (int)startLine);
      bvSet.insert(args_str[0]);
      dict.push_back(newEntry);
      entry_op = newEntry;
    }
    else if (bfNameMap[bfName] == "absBounds") { // (_x, _max)
      if (argNum != 2) {
        writeToFile << "Error: argument number of " << bfName << " should be 2 instead of " << argNum << endl;
        return true;
	    }
      string args_str[2] = {bvStr, ""};
      
	    args_str[1] = getVarType(args[1]); // bound values
	      
      BFDict newEntry(srcFile, bfName, args_str[0], "opposite(" + args_str[1] + ")", args_str[1], (int)startLine);
      bvSet.insert(args_str[0]);
      dict.push_back(newEntry);
      entry_op = newEntry;
    }
    else if (bfNameMap[bfName] == "upBounds") { // (_x, _max)
      if (argNum != 2) {
        writeToFile << "Error: argument number of " << bfName << " should be 2 instead of " << argNum << endl;
        return true;
	    }
      string args_str[2] = {bvStr, ""};
      
	  
	    args_str[1] = getVarType(args[1]); // bound values
	      
      BFDict newEntry(srcFile, bfName, args_str[0], "NULL", args_str[1], (int)startLine);
      bvSet.insert(args_str[0]);
      dict.push_back(newEntry);
      entry_op = newEntry;
    }
    else if (bfNameMap[bfName] == "noBounds") { // (_x)	      
      if (argNum != 1) {
        writeToFile << "Error: argument number of " << bfName << " should be 1 instead of " << argNum << endl;
        return true;
	    }
      string lbName = "", ubName = "";
      if (bfName == "FLOAT_ANGLE_NORMALIZE" || bfName == "INT32_ANGLE_NORMALIZE" ||
          bfName == "NormRadAngle") {
        lbName = "-pi";
        ubName = "pi";
      } else if (bfName == "INT32_COURSE_NORMALIZE") {
        lbName = "0";
        ubName = "2*pi";
      } else {
        lbName = "Invallid noBound Type Bounding Function";
        ubName = lbName;
      }

      BFDict newEntry(srcFile, bfName, bvStr, lbName, ubName, (int)startLine);
      bvSet.insert(bvStr);
      dict.push_back(newEntry);
      entry_op = newEntry;
    }
    else { // (_x1, _x2, _max)
      if (argNum != 3) {
        writeToFile << "Error: argument number of " << bfName << " should be 3 instead of " << argNum << endl;
        return true;
	    }
      string args_str[3] = {bvStr, "", ""};
	    Expr *extArgExpr = args[1]; // extra argument's expression

      args_str[1] = getVarName(extArgExpr); // extra argument
	    args_str[2] = getVarType(args[2]); // bound values
	   
      BFDict newEntry(srcFile, bfName, args_str[0], "opposite(" + args_str[2] + ")", args_str[2], (int)startLine);
	    newEntry.otherArgs.push_back(args_str[1]);
      bvSet.insert(args_str[0]);
      dict.push_back(newEntry);
      entry_op = newEntry;
    }
       
    // 4. output this entry to file
    writeToFile << "***FOUND BF***" << endl; // output debug info
    writeToFile << entry_op;


    ofstream writeDictToFile;

    writeDictToFile.open(dictFileName, ios::app); // output dict info

    writeDictToFile << entry_op;

    // close stream to output file
    writeDictToFile.close();


    return true;
  }


private:
  //Rewriter &TheRewriter;

  //ofstream &writeToFile;

  SourceManager &srcMgr;

  string getVarName(Expr* e) { // for bounded variables or extra arguments
	  string varName = "";

      while (isa<ImplicitCastExpr>(e)) { // perform implicit cast
        ImplicitCastExpr *tmp = cast<ImplicitCastExpr>(e);
	    e = tmp -> getSubExpr();
	  }
		
	  if (isa<DeclRefExpr>(e)) {
	    DeclRefExpr *declExp = cast<DeclRefExpr>(e);
	    varName = declExp -> getNameInfo().getAsString();
	  }
	  else if (isa<MemberExpr>(e)) { // structure member
	    MemberExpr *memExp = cast<MemberExpr>(e);
	    Expr *baseExp = memExp -> getBase();
	    while (isa<ImplicitCastExpr>(baseExp)) { // perform implicit cast
              ImplicitCastExpr *tmp = cast<ImplicitCastExpr>(baseExp);
	      baseExp = tmp -> getSubExpr();
	    }
	    if (isa<DeclRefExpr>(baseExp)) { 
		    DeclRefExpr *declExp = cast<DeclRefExpr>(baseExp);
		    varName = declExp -> getNameInfo().getAsString() + ". ";
	    }
	    else if (isa<MemberExpr>(baseExp)) { // three-level structure member
	      MemberExpr *memExp2 = cast<MemberExpr>(baseExp);
	      Expr *baseExp2 = memExp2 -> getBase();
	      while (isa<ImplicitCastExpr>(baseExp2)) { // perform implicit cast
          ImplicitCastExpr *tmp = cast<ImplicitCastExpr>(baseExp2);
	        baseExp2 = tmp -> getSubExpr();
	      }
	      if (isa<DeclRefExpr>(baseExp2)) { 
		      DeclRefExpr *declExp2 = cast<DeclRefExpr>(baseExp2);
		      varName = declExp2 -> getNameInfo().getAsString() + ". ";
	      }
	      else {
		      // TODO: maximum three-level structure member now
		      string baseType2 = baseExp2 -> getType().getAsString();
		      varName = "<" + baseType2 + ">. ";
	      }

	      varName += memExp2 -> getMemberNameInfo().getAsString() + ". ";
      }
	    else {
		  // TODO: handle base types other than MemberExpr
		  string baseType = baseExp -> getType().getAsString();
		  varName = "<" + baseType + ">. ";
	    }
	    varName += memExp -> getMemberNameInfo().getAsString();
	  }
	  else if (isa<ArraySubscriptExpr>(e)) { // array element
	    ArraySubscriptExpr *arrExp = cast<ArraySubscriptExpr>(e);
	    Expr *baseExp = arrExp -> getBase(); // array name
	    while (isa<ImplicitCastExpr>(baseExp)) { // perform implicit cast
              ImplicitCastExpr *tmp = cast<ImplicitCastExpr>(baseExp);
	      baseExp = tmp -> getSubExpr();
	    }
	    if (isa<DeclRefExpr>(baseExp)) { 
		  DeclRefExpr *declExp = cast<DeclRefExpr>(baseExp);
		  varName = declExp -> getNameInfo().getAsString();
	    }
	    else if (isa<MemberExpr>(baseExp)) { // structure member
	      MemberExpr *memExp = cast<MemberExpr>(baseExp);
	      Expr *memBaseExp = memExp -> getBase();
	      while (isa<ImplicitCastExpr>(memBaseExp)) { // perform implicit cast
                ImplicitCastExpr *tmp = cast<ImplicitCastExpr>(memBaseExp);
	        memBaseExp = tmp -> getSubExpr();
	      }
	      if (isa<DeclRefExpr>(memBaseExp)) { 
		      DeclRefExpr *declExp = cast<DeclRefExpr>(memBaseExp);
		      varName = declExp -> getNameInfo().getAsString() + ". ";
	      }
	      else {
		      // TODO: handle base types other than MemberExpr
		      string baseType = baseExp -> getType().getAsString();
		      varName = "<" + baseType + ">. ";
	      }
	      varName += memExp -> getMemberNameInfo().getAsString();
	    }
	    else {
		  // TODO: handle base types other than DeclRefExpr or MemberExpr
		  string baseType = baseExp -> getType().getAsString();
		  varName = "<" + baseType + ">";
	    }

	    Expr *idxExp = arrExp -> getIdx(); // index
	    while (isa<ImplicitCastExpr>(idxExp)) { // perform implicit cast
              ImplicitCastExpr *tmp = cast<ImplicitCastExpr>(idxExp);
	      idxExp = tmp -> getSubExpr();
	    }
	    string idxStr = "";
	    if (isa<IntegerLiteral>(idxExp)) {
	       IntegerLiteral *intIdxExp = cast<IntegerLiteral>(idxExp);
	       uint64_t intIdx = intIdxExp -> getValue().getLimitedValue();
	       idxStr = ('0' + intIdx);
	    }
	    else if (isa<DeclRefExpr>(idxExp)) { 
	       DeclRefExpr *declIdxExp = cast<DeclRefExpr>(idxExp);
	       idxStr = declIdxExp -> getNameInfo().getAsString();
	    }
      else {
		  // TODO: handle index types other than IntegerLiteral or DeclRefExpr
		  string idxType = idxExp -> getType().getAsString();
		  varName = "<" + idxType + ">. ";
	    }
	    varName += "[" + idxStr + "]";
	  }
    else if (isa<UnaryOperator>(e)) { // unary operator statement
      // recursively get subexpr via one of the following paths until DeclRefExpr:
      // 1. UnaryOperator->CStyleCastExpr->ImplicitCastExpr->DeclRefExpr
      // 2. UnaryOperator->CStyleCastExpr->UnaryOperator->DeclRefExpr
      // 3. UnaryOperator->CStyleCastExpr->UnaryOperator->MemberExpr->ImplicitCastExpr->DeclRefExpr

      UnaryOperator *unaryExp = cast<UnaryOperator>(e);
      Expr* derefExp = unaryExp -> getSubExpr();
      if (!isa<CStyleCastExpr>(derefExp)) {
        return "unary exception: BV not CStyleCastExp";
      }
      CStyleCastExpr* castExp = cast<CStyleCastExpr>(derefExp);
      Expr* precastExp = castExp -> getSubExpr();

      if (isa<ImplicitCastExpr>(precastExp)) { // path 1
        while (isa<ImplicitCastExpr>(precastExp)) { // perform implicit cast
            ImplicitCastExpr *tmp = cast<ImplicitCastExpr>(precastExp);
            precastExp = tmp -> getSubExpr();
        }
        if (!isa<DeclRefExpr>(precastExp)) {
          return "unary exception: Pre-cast BV not DeclRefExpr";
        }
        DeclRefExpr *declExp = cast<DeclRefExpr>(precastExp);
        varName = declExp -> getNameInfo().getAsString();
      } else if (isa<UnaryOperator>(precastExp)) {
        UnaryOperator* refExp = cast<UnaryOperator>(precastExp);
        Expr* varExp = refExp -> getSubExpr();

        if (isa<DeclRefExpr>(varExp)) { // path 2
          DeclRefExpr *declExp = cast<DeclRefExpr>(varExp);
          varName = declExp -> getNameInfo().getAsString();
        } else if (isa<MemberExpr>(varExp)) { // path 3
          MemberExpr *memExp = cast<MemberExpr>(varExp);
          Expr *memBaseExp = memExp -> getBase();
          while (isa<ImplicitCastExpr>(memBaseExp)) { // perform implicit cast
            ImplicitCastExpr *tmp = cast<ImplicitCastExpr>(memBaseExp);
            memBaseExp = tmp -> getSubExpr();
          }
          if (isa<DeclRefExpr>(memBaseExp)) { 
            DeclRefExpr *declExp = cast<DeclRefExpr>(memBaseExp);
            varName = declExp -> getNameInfo().getAsString() + "-> ";
          }
          else {
            // TODO: handle base types other than MemberExpr
            string baseType = memBaseExp -> getType().getAsString();
            varName = "<" + baseType + ">-> ";
          }
          varName += memExp -> getMemberNameInfo().getAsString();
        } else {
          return "unary exception: Referenced BV not DeclRefExpr or MemberExpr";
        }
      } else {
        string precastExpType = precastExp -> getType().getAsString();
        return "unary exception: Pre-cast BV (" + precastExpType + ") not DeclRefExpr or UnaryOperator";
      }
    }

	  return varName;
  }

  string getVarType(Expr* e) { // for bound values 
      //TODO: get the name(for variables) or value(for literals) instead of just type of the argument
	  while (isa<ImplicitCastExpr>(e)) { // perform implicit cast
		ImplicitCastExpr *tmp = cast<ImplicitCastExpr>(e);
		e = tmp -> getSubExpr();
	  }
	  string argType = e -> getType().getAsString();
	  return "<" + argType + ">";
  }

};

// By overriding function HandleTopLevelDecl of ASTConsumer, we can apply BFVisitor.TraverseDecl to 
// each top-level declarations of the AST, and inside it BFVisitor.VisitStmt is called so that we can
// detect Bounding Functions

class BFConsumer : public ASTConsumer {
public:
  //BFConsumer(Rewriter &R, ofstream &writeToFile) : Visitor(R, writeToFile) {}
  BFConsumer(SourceManager &R) : Visitor(R) {}

  // Override the method that gets called for each parsed top-level
  // declaration.
  bool HandleTopLevelDecl(DeclGroupRef DR) override {

    writeToFile.open(debugFileName, ios::app); // debug output
  

    for (DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e; ++b) {
      // Traverse the declaration using our AST visitor.
      Visitor.TraverseDecl(*b);
      #ifdef DUMP
        (*b)->dump(); // dump() output the AST to command line window
      #endif
    }

    // close stream to output file
    writeToFile.close();

    return true;
  }

private:
  BFVisitor Visitor;
 
};

// By overriding function CreateASTConsumer of ASTFrontendAction, we create a BFConsumer when
// ClangTool.run() is called, so as to trigger the action to detect Bounding Functions

class BFDictBuilder : public PluginASTAction {

protected:
  // this gets called by Clang when it invokes our Plugin
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef file) override {
    initBFNameMap();



    // open stream to output file
    writeToFile.open(debugFileName, ios::app);
  
    writeToFile << "** Creating AST consumer for: " << file.str() << "\n";
    
    // close stream to output file
    writeToFile.close();



    //TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    //return llvm::make_unique<BFConsumer>(TheRewriter, writeToFile);

    //SourceManager srcMgr = CI.getSourceManager();
    return llvm::make_unique<BFConsumer>(CI.getSourceManager());
  }

  // implement this function if you want to parse custom cmd-line args
  bool ParseArgs(const CompilerInstance &CI, const vector<string> &args) override {
    return true;
  }


private:
  //Rewriter TheRewriter;

  //ofstream writeToFile;

  //SourceManager srcMgr;
};

static FrontendPluginRegistry::Add<BFDictBuilder> X("bfdictbuilder", "build bounding function dict");
