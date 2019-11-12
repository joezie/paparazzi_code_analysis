#ifndef LIBTOOLANALYSIS_H
#define LIBTOOLANALYSIS_H
/* bounding function dictionary builder using libTools
 */

#include <string>
#include <iostream>
#include <fstream>
#include <unordered_set>
#include "BFDict.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace std;


/* extern variables start */

extern CXTranslationUnit tu;
extern string src_file;
extern vector<BFDict> dict;
extern unordered_map<string, string> bfNameMap; // (bounding function name, function type)
extern unordered_map<string, vector<int>> bvReadLineMap; // (bounded variable, read lines in src file)
extern unordered_map<string, vector<int>> bvWriteLineMap; // (bounded variable, write lines in src file)
extern unordered_map<string, int> bvDeclLineMap; // (bounded variable, declaration line in src file)
extern unordered_set<string> bvSet; // set of bounded variables

/* extern variables end */


//static llvm::cl::OptionCategory ToolingSampleCategory("Tooling Sample");

//std::string src_file_name_test = "../mysrc/complexAssign.cc";

/* BV Detector Part Start */

// By overriding function VisitStmt of RecursiveASTVisitor, we can detect where
// our Bounded Variables are written, and also update bvWriteLineMap
class WrittenBVVisitor : public RecursiveASTVisitor<WrittenBVVisitor> {
public:
  WrittenBVVisitor(Rewriter &R) : TheRewriter(R) {}

  bool VisitStmt(Stmt *s) {
    // Detect where our Bounded Variable is written and insert bounding function after the assignment

    //bool modified = false; // mark whether the source code is modified below

    if (isa<BinaryOperator>(s)) {
      BinaryOperator *binOp = cast<BinaryOperator>(s);
      std::string operSign = binOp -> getOpcodeStr(binOp -> getOpcode());
      if (operSign == "=") { // if it's an assignment, then get the lhs and rhs
        Expr* binLHS = binOp -> getLHS();
        Expr* binRHS = binOp -> getRHS();
        if (DeclRefExpr* DRE = dyn_cast<DeclRefExpr>(binLHS)) {
          // Try to cast binLHS to DeclRefExpr pointer. If sucess, match it with bounded
          // variables in bvSet
          std::string lhsName = (DRE -> getNameInfo()).getAsString();
          std::cout << "Found LHS " << lhsName << std::endl;

          if (bvSet.find(lhsName) != bvSet.end()) { // insert bounding function after the assignment
            // use rewriter to modify
            //modified = true;

            // get next line to the current statement
            SourceManager &SM = TheRewriter.getSourceMgr();
            FileID fileId = SM.getMainFileID();
            SourceLocation locOp = binOp -> getExprLoc(); // it's indeed the location of operator '='
            if (locOp.isMacroID()) { // now we consider macro as those expanded bounding functions' calling instances
                return true;
            }

            // update bvWriteLineMap
            unsigned lineNum = SM.getSpellingLineNumber(locOp);
            if (bvWriteLineMap.find(lhsName) == bvWriteLineMap.end())
                bvWriteLineMap[lhsName] = vector<int>();
            bvWriteLineMap[lhsName].push_back(lineNum);


            /*
            SourceLocation loc_nextLine = SM.translateLineCol(fileId, lineNum_current + 1, 0);
            TheRewriter.InsertTextBefore(loc_nextLine, "BEGIN:" + getInsertedBFStr(lhsName));
            */

            /*
            std::string newRHS = "boundcheck( " + convertExprToStr(binRHS) + " , _min, _max )";
            TheRewriter.ReplaceText(binRHS -> getSourceRange(), newRHS);
            */

          }
        }
      }

    }

    //if (modified) { // If modified, then rewrite the new version to the source code
    //  SourceManager &SM = TheRewriter.getSourceMgr();
    //  const RewriteBuffer *RewriteBuf = TheRewriter.getRewriteBufferFor(SM.getMainFileID());
    //  std::ofstream output(src_file_name_test);
    //  output << std::string(RewriteBuf -> begin(), RewriteBuf -> end());
    //  output.close();
    //}

    return true;
  }


private:
  Rewriter &TheRewriter;

  std::string convertExprToStr(Expr *E) { // retrieve the expr through source code
    SourceManager &SM = TheRewriter.getSourceMgr();
    clang::LangOptions lopt;
    SourceLocation startLoc = E -> getLocStart();
    SourceLocation _endLoc = E -> getLocEnd();
    SourceLocation endLoc = clang::Lexer::getLocForEndOfToken(_endLoc, 0, SM, lopt);

    return std::string(SM.getCharacterData(startLoc), SM.getCharacterData(endLoc) - SM.getCharacterData(startLoc));
  }

  std::string getInsertedBFStr(std::string argName) { // get corresponding bounding function calling string of this argName
    BFDict bfEntry;
    for (BFDict entry : dict) {
      if (entry.argName == argName) {
        bfEntry = entry;
        break;
      }
    }
    if (bfEntry.bfLineNum == -1) { // error
      std::cout << "ERROR: cannot find bounded variable " << argName << " in dict!" << std::endl;
      return "";
    }
    std::string insertedBF = "", bfName = bfEntry.bfName, lbName = bfEntry.lbName, ubName = bfEntry.ubName, bfType = bfNameMap[bfName];
    if (bfType == "commonBounds")
      insertedBF = bfName + "(" + argName + ", " + lbName + ", " + ubName + ")\n";
    else if (bfType == "absBounds" || bfType == "upBounds")
      insertedBF = bfName + "(" + argName + ", " + ubName + ")\n";
    else if (bfNameMap[bfName] == "noBounds")
      insertedBF = bfName + "(" + argName + ")\n";
    else { // "splBounds"
      if (bfEntry.otherArgs.empty()) { // error
        std::cout << "ERROR: bounded function " << bfName << " of type splBounds don't contain otherArgs!" << std::endl;
        return "";
      }
      insertedBF = bfName + "(" + argName + ", " + bfEntry.otherArgs[0] + ", " + ubName + ")\n";
    }

    return insertedBF;
  }
};

// By overriding function HanfleTopLevelDecl of ASTConsumer, we can apply WrittenBVVisitor.TraverseDecl to 
// each top-level declarations of the AST, and inside it WrittenBVVisitor.VisitStmt is called so that we can
// detect where Bounded Variable is written

class WrittenBVConsumer : public ASTConsumer {
public:
  WrittenBVConsumer(Rewriter &R) : Visitor(R) {}

  // Override the method that gets called for each parsed top-level
  // declaration.
  bool HandleTopLevelDecl(DeclGroupRef DR) override {
    for (DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e; ++b) {
      // Traverse the declaration using our AST visitor.
      Visitor.TraverseDecl(*b);
      #if DUMP
        (*b)->dump(); // dump() output the AST to command line window
      #endif
    }
    return true;
  }

private:
  WrittenBVVisitor Visitor;
};

// By overriding function CreateASTConsumer of ASTFrontendAction, we create a WrittenBVConsumer when
// ClangTool.run() is called, so as to trigger the action to detect written bounded variables

class WrittenBVDetector : public ASTFrontendAction {
public:
  WrittenBVDetector() {}
  void EndSourceFileAction() override {
    SourceManager &SM = TheRewriter.getSourceMgr();
    llvm::errs() << "** EndSourceFileAction for: "
                 << SM.getFileEntryForID(SM.getMainFileID())->getName() << "\n";

    // Now emit the rewritten buffer.
    //TheRewriter.getEditBuffer(SM.getMainFileID()).write(llvm::outs()); // this statement output the rewriter buffer (i.e the modified src code)
  }

  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef file) override {
    llvm::errs() << "** Creating AST consumer for: " << file << "\n";
    TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return llvm::make_unique<WrittenBVConsumer>(TheRewriter);
  }

private:
  Rewriter TheRewriter;
};

/* BV Detector Part End */

/* BF Dict Builder Part Start */ 

// By overriding function VisitStmt of RecursiveASTVisitor, we can detect
// Bounding Functions, and also build dict

class BFVisitor : public RecursiveASTVisitor<BFVisitor> {
public:
  BFVisitor(Rewriter &R) : TheRewriter(R) {}

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
    
    
    if (bfNameMap.find(bfName) == bfNameMap.end()) // not a bounding function
      return true;
      
    unsigned startLine = TheRewriter.getSourceMgr().getSpellingLineNumber(callExp -> getBeginLoc());
    unsigned endLine = TheRewriter.getSourceMgr().getSpellingLineNumber(callExp -> getEndLoc());
	string srcFile = TheRewriter.getSourceMgr().getFilename(callExp -> getBeginLoc()).str();
    
	Expr **args = callExp -> getArgs();
	unsigned argNum = callExp -> getNumArgs();
	Expr *bvExpr = args[0]; // bounded variable 
	string bvStr = getVarName(bvExpr);

    // 3. match bounding function name with bfNameMap
    if (bfNameMap[bfName] == "commonBounds") { // (_x, _min, _max)
	  if (argNum != 3) {
        cout << "Error: argument number of " << bfName << " should be 3 instead of " << argNum << endl;
        return true;
	  }
      string args_str[3] = {bvStr, "", ""};
      
	  args_str[1] = getVarType(args[1]); // bound values
	  args_str[2] = getVarType(args[2]); // bound values
      
      BFDict newEntry(srcFile, bfName, args_str[0], args_str[1], args_str[2], (int)startLine);
      bvSet.insert(args_str[0]);
      dict.push_back(newEntry);
    }
    else if (bfNameMap[bfName] == "absBounds") { // (_x, _max)
	  if (argNum != 2) {
        cout << "Error: argument number of " << bfName << " should be 2 instead of " << argNum << endl;
        return true;
	  }
      string args_str[2] = {bvStr, ""};
      
	  args_str[1] = getVarType(args[1]); // bound values
	      
      BFDict newEntry(srcFile, bfName, args_str[0], "opposite(" + args_str[1] + ")", args_str[1], (int)startLine);
      bvSet.insert(args_str[0]);
      dict.push_back(newEntry);
    }
    else if (bfNameMap[bfName] == "upBounds") { // (_x, _max)
	  if (argNum != 2) {
        cout << "Error: argument number of " << bfName << " should be 2 instead of " << argNum << endl;
        return true;
	  }
      string args_str[2] = {bvStr, ""};
      
	  
	  args_str[1] = getVarType(args[1]); // bound values
	      
      BFDict newEntry(srcFile, bfName, args_str[0], "NULL", args_str[1], (int)startLine);
      bvSet.insert(args_str[0]);
      dict.push_back(newEntry);
    }
    else if (bfNameMap[bfName] == "noBounds") { // (_x)	      
	  if (argNum != 1) {
        cout << "Error: argument number of " << bfName << " should be 1 instead of " << argNum << endl;
        return true;
	  }
      BFDict newEntry(srcFile, bfName, bvStr, "-pi", "pi", (int)startLine);
      bvSet.insert(bvStr);
      dict.push_back(newEntry);
    }
    else { // (_x1, _x2, _max)
	  if (argNum != 3) {
        cout << "Error: argument number of " << bfName << " should be 3 instead of " << argNum << endl;
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
    }
       

    return true;
  }


private:
  Rewriter &TheRewriter;

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
		  varName = declExp -> getNameInfo().getAsString() + ".";
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
		  varName = declExp2 -> getNameInfo().getAsString() + ".";
	      }
	      else {
		  // TODO: maximum three-level structure memebr now
		  string baseType2 = baseExp2 -> getType().getAsString();
		  varName = "<" + baseType2 + ">.";
	      }
	      varName += memExp2 -> getMemberNameInfo().getAsString() + ".";
            }
	    else {
		  // TODO: handle base types other than MemberExpr
		  string baseType = baseExp -> getType().getAsString();
		  varName = "<" + baseType + ">.";
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
		varName = declExp -> getNameInfo().getAsString() + ".";
	      }
	      else {
		  // TODO: handle base types other than MemberExpr
		  string baseType = baseExp -> getType().getAsString();
		  varName = "<" + baseType + ">.";
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
		  varName = "<" + idxType + ">.";
	    }
	    varName += "[" + idxStr + "]";
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
  BFConsumer(Rewriter &R) : Visitor(R) {}

  // Override the method that gets called for each parsed top-level
  // declaration.
  bool HandleTopLevelDecl(DeclGroupRef DR) override {
    for (DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e; ++b) {
      // Traverse the declaration using our AST visitor.
      Visitor.TraverseDecl(*b);
      #if DUMP
        (*b)->dump(); // dump() output the AST to command line window
      #endif
    }
    return true;
  }

private:
  BFVisitor Visitor;
 
};

// By overriding function CreateASTConsumer of ASTFrontendAction, we create a BFConsumer when
// ClangTool.run() is called, so as to trigger the action to detect Bounding Functions

class BFDictBuilder : public ASTFrontendAction {
public:
  BFDictBuilder() {}
  void EndSourceFileAction() override {
    SourceManager &SM = TheRewriter.getSourceMgr();
    llvm::errs() << "** EndSourceFileAction for: "
                 << SM.getFileEntryForID(SM.getMainFileID())->getName() << "\n";

    // Now emit the rewritten buffer.
    //TheRewriter.getEditBuffer(SM.getMainFileID()).write(llvm::outs()); // this statement output the rewriter buffer (i.e the modified src code)
  }

  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef file) override {
    llvm::errs() << "** Creating AST consumer for: " << file << "\n";
    TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return llvm::make_unique<BFConsumer>(TheRewriter);
  }

private:
  Rewriter TheRewriter;
};

/* BF Dict Builder Part End */

/*
int main(int argc, const char **argv) {
  CommonOptionsParser op(argc, argv, ToolingSampleCategory);
  ClangTool Tool(op.getCompilations(), op.getSourcePathList());

  // ClangTool::run accepts a FrontendActionFactory, which is then used to
  // create new objects implementing the FrontendAction interface. Here we use
  // the helper newFrontendActionFactory to create a default factory that will
  // return a new MyFrontendAction object every time.
  // To further customize this, we could create our own factory class.
  return Tool.run(newFrontendActionFactory<MyFrontendAction>().get());
}
*/













#endif

