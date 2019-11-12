#ifndef BFDICT_H
#define BFDICT_H

#include <string>
#include <vector>
#include <iostream>
using namespace std;

/* Definition of bounding function dictionary class
 */

class BFDict { // bounding function dictionary class
public:
  BFDict(): srcFile(""), bfName(""), argName(""), lbName(""), ubName(""), bfLineNum(-1) {} // default constructor
  BFDict(string src_file_name, string bounding_function_name, string argument_name, string lower_bound_name, string upper_bound_name, int lineNum): srcFile(src_file_name), bfName(bounding_function_name), argName(argument_name), lbName(lower_bound_name), ubName(upper_bound_name), bfLineNum(lineNum) {} // value constructor

  
  string srcFile; // name of source file
  string bfName; // name of bounding function
  string argName; // name of argument to be bounded
  string lbName; // name of lower bound
  string ubName; // name of upper bound
  vector<string> otherArgs; // other arguments
  int bfLineNum; // line number in src file

  friend ostream & operator << (ostream & os, const BFDict & bfDict) {
    os << bfDict.srcFile << "\t" << bfDict.bfName << "\t" << bfDict.argName << "\t" << bfDict.lbName << "\t" << bfDict.ubName;
    for (string arg : bfDict.otherArgs)
      os << "\t" << arg;
    os << "\t" << bfDict.bfLineNum << endl;
    return os;
  }
};

#endif
