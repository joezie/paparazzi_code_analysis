#ifndef HELPERFUNCS_H
#define HELPERFUNCS_H

#include <unistd.h>
#include <sys/wait.h>
using namespace std;

/* Some helper functions
 */

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

void initBFNameMap() { // initialize bfNameMap
  string commonBounds[] = {"constrain", "isInRange"};
  string absBounds[] = {"_constrainAbs"};
  string upBounds[] = {"_constrainOneSide"};
  string noBounds[] = {"constrainFloatToInt16"};
  string splBounds[] = {"constrainXY"};

  for (string str : commonBounds) // (_x, _min, _max)
    bfNameMap[str] = "commonBounds";
  for (string str : absBounds) // (_x, _max)
    bfNameMap[str] = "absBounds";
  for (string str : upBounds) // (_x, _max)
    bfNameMap[str] = "upBounds";
  for (string str : noBounds) // (_x)
    bfNameMap[str] = "noBounds";
  for (string str : splBounds) // (_x1, _x2, _max)
    bfNameMap[str] = "splBounds";
}

void printDict() { // print dict
  cout << "\nBOUNDING FUNCTION DICTIONARY" << endl;
  for (BFDict entry : dict)
    cout << entry;
}

void printBVLineMap() { // print bvReadLineMap and bvWriteLineMap
  cout << "\nBOUNDING VARIABLE READ LINE MAP" << endl;
  for (auto entry : bvReadLineMap) {
    cout << entry.first << ": ";
    for (int lineNum : entry.second)
      cout << lineNum << " ";
    cout << endl;
  }

  cout << "\nBOUNDING VARIABLE WRITE LINE MAP" << endl;
  for (auto entry : bvWriteLineMap) {
    cout << entry.first << ": ";
    for (int lineNum : entry.second)
      cout << lineNum << " ";
    cout << endl;
  }

  cout << "\nBOUNDING VARIABLE DECLARATION LINE MAP" << endl;
  for (auto entry : bvDeclLineMap) {
    cout << entry.first << ": " << entry.second << endl;
  }
}

bool forkFunc(string cmd, vector<string> args) { // wrapper function for fork
  int childExitStatus = -1, status = -1, argNum = args.size();
  pid_t pid = fork();

  if (pid == 0) { /* child */
    if (argNum == 2) // usage case: cp src dest
      execl(cmd.c_str(), cmd.c_str(), args[0].c_str(), args[1].c_str(), (char *)0);
    else if (argNum == 1) // usage case: mkdir dirName
      execl(cmd.c_str(), cmd.c_str(), args[0].c_str(), (char *)0);
    else
      cout << "Not support arg num greater than 2" << endl;
  }
  else if (pid < 0) {
    cout << "fork() failed!" << endl;
    return false;
  }
  else { /* parent */
    pid_t ws = waitpid( pid, &childExitStatus, WNOHANG);
    if (ws == -1) {
      cout << "waitpid() failed!" << endl;
      return false;
    }

    if( WIFEXITED(childExitStatus)) { /* exit code in childExitStatus */
      status = WEXITSTATUS(childExitStatus); /* zero is normal exit */
      if (status) {
        cout << "Abnormal exit of child process!" << endl;
        return false;
      }
    }
    else if (WIFSIGNALED(childExitStatus)) { /* killed */
      cout << "Child process is killed!" << endl;
      return false;
    }
    else if (WIFSTOPPED(childExitStatus)) { /* stopped */
      cout << "Child process is stopped!" << endl;
      return false;
    }
  }
  return true;
}

bool copyFile(string source, string dest) { // copy file from path source to dest
  if (source == "" || dest == "") {
    cout << "Invalid argument(s) for copyFile()" << endl;
    return false;
  }

  vector<string> args;
  args.push_back(source);
  args.push_back(dest);

  return forkFunc("/bin/cp", args);
}

bool createDir(string dirName) { // create a dir if it doesn't exist
  if (access(dirName.c_str(), W_OK) == 0) // already exists
    return true;

  return forkFunc("/bin/mkdir", vector<string>(1, dirName));
}

bool mvFile(string source, string dest) { // move file from path source to dest
  if (source == "" || dest == "") {
    cout << "Invalid argument(s) for mvFile()" << endl;
    return false;
  }

  vector<string> args;
  args.push_back(source);
  args.push_back(dest);

  return forkFunc("/bin/mv", args);
}

string retrieveFileName(string path) { // retrieve file name from the end of the path
  size_t slashPos = path.find_last_of('/');
  size_t startPos = (slashPos == string::npos) ? 0 : slashPos + 1;
  return path.substr(startPos);
}

string getInsertedBFStr(string argName) { // get corresponding bounding function calling string of this argName
    BFDict bfEntry;
    for (BFDict entry : dict) {
      if (entry.argName == argName) {
        bfEntry = entry;
        break;
      }
    }
    if (bfEntry.bfLineNum == -1) { // error
      cout << "ERROR: cannot find bounded variable " << argName << " in dict!" << endl;
      return "";
    }
    string insertedBF = "", bfName = bfEntry.bfName, lbName = bfEntry.lbName, ubName = bfEntry.ubName, bfType = bfNameMap[bfName];
    if (bfType == "commonBounds")
      insertedBF = bfName + "(" + argName + ", " + lbName + ", " + ubName + ")";
    else if (bfType == "absBounds" || bfType == "upBounds")
      insertedBF = bfName + "(" + argName + ", " + ubName + ")";
    else if (bfNameMap[bfName] == "noBounds")
      insertedBF = bfName + "(" + argName + ")";
    else { // "splBounds"
      if (bfEntry.otherArgs.empty()) { // error
        cout << "ERROR: bounded function " << bfName << " of type splBounds don't contain otherArgs!" << endl;
        return "";
      }
      insertedBF = bfName + "(" + argName + ", " + bfEntry.otherArgs[0] + ", " + ubName + ")";
    }

    return insertedBF;
}

string getIndent(string line) { // get indentation string at the beginning of this line
  size_t pos = line.find_first_not_of(" \t");
  return line.substr(0, pos);
}

#endif
