#ifndef LIBTOOLANALYSIS_H
#define LIBTOOLANALYSIS_H
/* bounding function inserter using libTools
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

// By implementing RecursiveASTVisitor, we can specify in which AST nodes
// our Bounded Variables are written by overriding method VisitStmt
class MyASTVisitor : public RecursiveASTVisitor<MyASTVisitor> {
public:
  MyASTVisitor(Rewriter &R) : TheRewriter(R) {}

  bool VisitStmt(Stmt *s) {
    // Detect where our Bounded Variable is written and update bvWriteLineMap

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
          //std::cout << "Found LHS " << lhsName << std::endl;

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
  /*
  bool shouldTraversePostOrder() const { // set the AST DFS traverse order to post-order
    return true;
  }
  */

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

// Implementation of the ASTConsumer interface for reading an AST produced
// by the Clang parser.
class MyASTConsumer : public ASTConsumer {
public:
  MyASTConsumer(Rewriter &R) : Visitor(R) {}

  // Override the method that gets called for each parsed top-level
  // declaration.
  bool HandleTopLevelDecl(DeclGroupRef DR) override {
    for (DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e; ++b) {
      // Traverse the declaration using our AST visitor.
      Visitor.TraverseDecl(*b);
      //(*b)->dump(); // dump() output the AST to command line window
    }
    return true;
  }

private:
  MyASTVisitor Visitor;
};

// For each source file provided to the tool, a new FrontendAction is created.
class MyFrontendAction : public ASTFrontendAction {
public:
  MyFrontendAction() {}
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
    return llvm::make_unique<MyASTConsumer>(TheRewriter);
  }

private:
  Rewriter TheRewriter;
};

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

