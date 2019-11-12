#ifndef ASTANALYSIS_H
#define ASTANALYSIS_H
/* Some functions related to AST analysis
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

/* extern functions start */

extern string getInsertedBFStr(string);
extern string getIndent(string);

/* extern functions end */

bool detectBF(CXCursor cursor) { // detect bounding functions and update dict
  // 1. check if its type is CXCursor_CompoundStmt
  CXCursorKind cursorKind = clang_getCursorKind( cursor );
  if ( cursorKind != CXCursor_CompoundStmt )
      return false;

  // 2. check if the source code block corresponding to this cursor contains a bounding function token
  CXToken *tokens_ptr = NULL;
  unsigned numTokens = 0;
  CXSourceRange cs_range = clang_getCursorExtent( cursor );
  clang_tokenize( tu, cs_range, &tokens_ptr, &numTokens );


  bool hasBound = false;
  for (int i = 0; i < numTokens; i++) {
    string token_name = clang_getCString( clang_getTokenSpelling( tu, tokens_ptr[i] ) );
    if (bfNameMap.find(token_name) != bfNameMap.end()) {
	hasBound = true;
	break;
    }
  }
  clang_disposeTokens( tu, tokens_ptr, numTokens );

  if (!hasBound) // no Bound token
    return false;

  // 3. open source file and jump to corresponding start line
  unsigned int startLine = 0, endLine = 0;
  CXSourceLocation startLocation = clang_getRangeStart( cs_range );
  CXSourceLocation endLocation = clang_getRangeEnd( cs_range );
  clang_getSpellingLocation( startLocation, NULL, &startLine, NULL, NULL );
  clang_getSpellingLocation( endLocation, NULL, &endLine, NULL, NULL );
  ifstream src_fstr(src_file);
  string tmp;
  for (int i = 1; i < startLine; i++)
    getline(src_fstr, tmp);

  // 4. read and match bounding function name
  string pattern("abcdefghijklmnopqrstuvwxyz"
                 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                 "0123456789_");
  string curLine = "", codeStr = "", bfName = "";

  for (int i = startLine; i <= endLine; i++) {
    getline(src_fstr, curLine);
    codeStr += curLine;
  }
  size_t token_head = codeStr.find_first_not_of(" "); // trim spaces
  size_t token_end = codeStr.find_first_not_of(pattern, token_head);
  if (token_head != string::npos && token_end != string::npos)
    bfName = codeStr.substr(token_head, token_end - token_head);

  // 5. interpret based on bounding function name
  if (bfNameMap.find(bfName) == bfNameMap.end()) // not a bounding function
    return false;

  token_head = codeStr.find_first_not_of(" ", token_end); // point to (
  token_head = codeStr.find_first_not_of(" ", token_head + 1); // trim spaces and point to the first token
  if (bfNameMap[bfName] == "commonBounds") { // (_x, _min, _max)
    string args[3];
    for (int i = 0; i < 3; i++) {
      token_end = codeStr.find_first_of(",) ", token_head); // point to end of the token (pass the end)
      args[i] = codeStr.substr(token_head, token_end - token_head);
      token_head = codeStr.find_first_not_of(",) ", token_end); // trim spaces and point to the next token
    }
    BFDict newEntry(src_file, bfName, args[0], args[1], args[2], startLine);
    bvSet.insert(args[0]);
    dict.push_back(newEntry);
  }
  else if (bfNameMap[bfName] == "absBounds") { // (_x, _max)
    string args[2];
    for (int i = 0; i < 2; i++) {
      token_end = codeStr.find_first_of(",) ", token_head); // point to end of the token (pass the end)
      args[i] = codeStr.substr(token_head, token_end - token_head);
      token_head = codeStr.find_first_not_of(",) ", token_end); // trim spaces and point to the next token
    }
    BFDict newEntry(src_file, bfName, args[0], "oposite(" + args[1] + ")", args[1], startLine);
    bvSet.insert(args[0]);
    dict.push_back(newEntry);
  }
  else if (bfNameMap[bfName] == "upBounds") { // (_x, _max)
    string args[2];
    for (int i = 0; i < 2; i++) {
      token_end = codeStr.find_first_of(",) ", token_head); // point to end of the token (pass the end)
      args[i] = codeStr.substr(token_head, token_end - token_head);
      token_head = codeStr.find_first_not_of(",) ", token_end); // trim spaces and point to the next token
    }
    BFDict newEntry(src_file, bfName, args[0], "NULL", args[1], startLine);
    bvSet.insert(args[0]);
    dict.push_back(newEntry);
  }
  else if (bfNameMap[bfName] == "noBounds") { // (_x)
    token_end = codeStr.find_first_of(") ", token_head); // point to end of the token (pass the end)
    string arg = codeStr.substr(token_head, token_end - token_head);
    BFDict newEntry(src_file, bfName, arg, "-pi", "pi", startLine);
    bvSet.insert(arg);
    dict.push_back(newEntry);
  }
  else { // (_x1, _x2, _max)
    string args[3];
    for (int i = 0; i < 3; i++) {
      token_end = codeStr.find_first_of(",) ", token_head); // point to end of the token (pass the end)
      args[i] = codeStr.substr(token_head, token_end - token_head);
      token_head = codeStr.find_first_not_of(",) ", token_end); // trim spaces and point to the next token
    }
    BFDict newEntry(src_file, bfName, args[0], "oposite(" + args[2] + ")", args[2], startLine);
    newEntry.otherArgs.push_back(args[1]);
    bvSet.insert(args[0]);
    dict.push_back(newEntry);
  }


  /*20190310 start*/
  /*
  if (foundBound) {
    cout << "************" << getCursorSpelling( cursor ) << "'s bound starts************";
    for (int i = 0; i < numTokens; i++) {
      string token_name = clang_getCString( clang_getTokenSpelling( tu, tokens_ptr[i] ) );
      cout << token_name << endl;
    }
    cout << "************" << getCursorSpelling( cursor ) << "'s bound ends************";
  }
  */
  /*20190310 end*/

  return true;
}

bool detectBVToken(string str, int rwMode, int lineNum) { // detect bounded variables' token in a string and update bvReadLineMap / bvWriteLineMap; rwMode = 0/1 indicates that str represents the read/write side
  string pattern("abcdefghijklmnopqrstuvwxyz"
                 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                 "0123456789_");
  bool foundBV = false;
  for (string bvToken : bvSet) {
    size_t tokenStartPos = str.find(bvToken);
    if (tokenStartPos != string::npos) { // possibly found a BV token, then double check it: whether it's a standalone name
      size_t tokenEndPos = tokenStartPos + bvToken.length();
      char prevChar = (tokenStartPos == 0) ? '\0' : str[tokenStartPos - 1]; // the character before substring bvToken in str
      char nextChar = (tokenEndPos + 1 >= string::npos) ? '\0' : str[tokenEndPos + 1]; // the character after substring bvToken in str
      if (pattern.find(prevChar) == string::npos && pattern.find(nextChar) == string::npos) { // it's a standalone name, then update bvReadLineMap / bvWriteLineMap
        foundBV = true;
        if (rwMode == 0) { // read side
          if (bvReadLineMap.find(bvToken) == bvReadLineMap.end())
            bvReadLineMap[bvToken] = vector<int>();
          bvReadLineMap[bvToken].push_back(lineNum);
        }
        else { // write side
          if (bvWriteLineMap.find(bvToken) == bvWriteLineMap.end())
            bvWriteLineMap[bvToken] = vector<int>();
          bvWriteLineMap[bvToken].push_back(lineNum);
        }
      }
    }
  }
  return foundBV;
}

bool detectBV(CXCursor cursor) { // detect bounded variables' read/write/declaration occurence in a cursor
  // 1. check if its type is CXCursor_BinaryOperator or CXCursor_VarDecl
  CXCursorKind cursorKind = clang_getCursorKind( cursor );
  if ( cursorKind != CXCursor_BinaryOperator && cursorKind != CXCursor_VarDecl )
      return false;

  if ( cursorKind == CXCursor_BinaryOperator ) {
    // 2. detect read/write occurence

    // 2.1 check if the source code block corresponding to this cursor contains a bounded variable's token
    CXToken *tokens_ptr = NULL;
    unsigned numTokens = 0;
    CXSourceRange cs_range = clang_getCursorExtent( cursor );
    clang_tokenize( tu, cs_range, &tokens_ptr, &numTokens );

    bool hasBV = false;
    for (int i = 0; i < numTokens; i++) {
      string token_name = clang_getCString( clang_getTokenSpelling( tu, tokens_ptr[i] ) );
      if (bvSet.find(token_name) != bvSet.end()) {
	hasBV = true;
	break;
      }
    }
    clang_disposeTokens( tu, tokens_ptr, numTokens );

    if (!hasBV) // no Bounded Variable's token
      return false;

    // 2.2 open source file and jump to corresponding start line
    unsigned int startLine = 0, endLine = 0;
    CXSourceLocation startLocation = clang_getRangeStart( cs_range );
    CXSourceLocation endLocation = clang_getRangeEnd( cs_range );
    clang_getSpellingLocation( startLocation, NULL, &startLine, NULL, NULL );
    clang_getSpellingLocation( endLocation, NULL, &endLine, NULL, NULL );
    ifstream src_fstr(src_file);
    string tmp;
    for (int i = 1; i < startLine; i++)
      getline(src_fstr, tmp);

    // 2.3 read and match bounding function name
    string curLine = "", codeStr = "", bfName = "";

    for (int i = startLine; i <= endLine; i++) {
      getline(src_fstr, curLine);
      codeStr += curLine;
    }
    size_t eqPos = codeStr.find('='); // the position of '='
    if (eqPos == string::npos)
      return false;
    string lhs = codeStr.substr(0, eqPos); // left hand side of '='
    string rhs = codeStr.substr(eqPos + 1); // right hand side of '='

    // 2.4 match BV in dict and update bvReadLineMap/bvWriteLineMap
    if (detectBVToken(lhs, 1, startLine) || detectBVToken(rhs, 0, startLine))
      return true;
    else
      return false;
  }
  else {
    // 3. detect declaration occurence

    // 3.1 look up bvSet to match argName
    string cursorName = getCursorSpelling(cursor);
    if (bvSet.find(cursorName) == bvSet.end()) // not a BV
      return false;

    // 3.2 update bvDeclLineMap
    CXSourceRange cs_range = clang_getCursorExtent( cursor );
    CXSourceLocation startLocation = clang_getRangeStart( cs_range );
    unsigned int startLine = 0;
    clang_getSpellingLocation( startLocation, NULL, &startLine, NULL, NULL );

    bvDeclLineMap[cursorName] = startLine;
    return true;
  }
}


void modifyArgDecl(string src, string dest) { // modify the declarations of BV in src file and store the results in dest file
  // 1. sort bvDeclLineMap based on line num
  map< int, vector<string> > bvDeclLineMap_r; // (lineNum, BV(s) at that line)
  for (pair<string, int> p : bvDeclLineMap) {
    string bvToken = p.first;
    int lineNum = p.second;
    if (bvDeclLineMap_r.find(lineNum) == bvDeclLineMap_r.end())
      bvDeclLineMap_r[lineNum] = vector<string>();
    bvDeclLineMap_r[lineNum].push_back(bvToken);
  }

  // 2. open file
  ifstream in;
  char curLine[1024] = {'\0'};
  in.open(src.c_str());
  if (in.fail()) {
    cout << "Open file " << src << " in read mode failed!" << endl;
    return;
  }

  int lineCnt = 0, nextModLineNum = -1; // line num of the next to-be-modified line
  string wholeFile = ""; // the output string to the modified file
  vector<string> bvTokens; // BV tokens corresponding to the next to-be-modified line
  map< int, vector<string> >::iterator iter = bvDeclLineMap_r.begin();
  if (iter != bvDeclLineMap_r.end()) {
    nextModLineNum = iter -> first;
    bvTokens = iter -> second;
  }

  // 3. add constraints to corresponding BV(s) according to BF type
  while (in.getline(curLine, sizeof(curLine))) {
    lineCnt++;
    if (lineCnt != nextModLineNum)
      wholeFile += string(curLine);
    else { // modify this line
      string curLine_str = string(curLine);

      for (string bvToken : bvTokens) {
        // 3.1 add constraints to each BV token at this line
        size_t tokenPos = curLine_str.find(bvToken);
        if (tokenPos == string::npos) {
          cout << "Token " << bvToken << " not in line " << lineCnt << "!" << endl;
          return;
        }

        BFDict matchedEntry;
        bool foundEntry = false;
        for (BFDict entry : dict) {
          if (entry.argName == bvToken) {
            foundEntry = true;
            matchedEntry = entry;
            break;
          }
        }

        if (!foundEntry) {
          cout << "Token " << bvToken << " isn't a valid token in dict!";
          return;
        }

        string bvInfo = " [ " + matchedEntry.lbName + ", " + matchedEntry.ubName + " ]";

        if (bfNameMap[matchedEntry.bfName] == "splBounds") {
          // 3.2 need to add extra argument info
          bvInfo += " --other args( ";
          for (string arg : matchedEntry.otherArgs)
            bvInfo = bvInfo + arg + " ";
          bvInfo += ")";
        }

        // 3.3 update curLine_str
        curLine_str.insert(tokenPos, bvInfo);

        // 3.4 update nextModLineNum and bvTokens
        iter++;
        if (iter != bvDeclLineMap_r.end()) {
          nextModLineNum = iter-> first;
          bvTokens = iter-> second;
        }
        else
          nextModLineNum = -1;
      }

      wholeFile += curLine_str;
    }
    wholeFile += '\n';
  }
  in.close();

  // 4. output to the file
  ofstream out;
  out.open(dest.c_str()); // would create one if not exists
  if (out.fail()) {
    cout << "Open file " << dest << " in write mode failed!" << endl;
    return;
  }

  out.flush();
  out << wholeFile;
  out.close();
}

bool findBoundArg( CXCursor cursor ) {
  // check if its CXCursorKind is CXCursor_DeclRefExpr
  if ( clang_getCursorKind( cursor ) == CXCursor_DeclRefExpr ) {
      cout << "@@Arg " << getCursorSpelling( cursor ) << "@@\n";
      return true;
  }
  else
    return false;
}

void modifyArgWritten(string src, string dest) { // insert bounding functions after where BV are written, and store the results in dest file
  // 1. sort bvWriteLineMap based on line num
  map<int, string> bvWriteLineMap_r; // (lineNum, BV at that line)
  for (pair<string, vector<int> > p : bvWriteLineMap) {
    string bvToken = p.first;
    vector<int> lineNums = p.second;
    for (int lineNum : lineNums)
        bvWriteLineMap_r[lineNum] = bvToken;
  }

  // 2. open file
  ifstream in;
  char curLine[1024] = {'\0'};
  in.open(src.c_str());
  if (in.fail()) {
    cout << "Open file " << src << " in read mode failed!" << endl;
    return;
  }

  int lineCnt = 0, nextModLineNum = -1; // line num of the next to-be-modified line
  string wholeFile = ""; // the output string to the modified file
  string bvToken; // BV token corresponding to the next to-be-modified line
  map<int, string>::iterator iter = bvWriteLineMap_r.begin();
  if (iter != bvWriteLineMap_r.end()) {
    nextModLineNum = iter -> first;
    bvToken = iter -> second;
  }

  // 3. insert bounding functions after the end of current line
  while (in.getline(curLine, sizeof(curLine))) {
    lineCnt++;
    if (lineCnt != nextModLineNum)
      wholeFile = wholeFile + string(curLine) + '\n';
    else { // insert after this line
      // 3.1 search for ';' (i.e the end of this statement)
      string curLine_str = string(curLine);
      wholeFile = wholeFile + string(curLine) + '\n';
      while (curLine_str.find(';') == string::npos) { // this statement doesn't end at this line, then keep searching for the end
        in.getline(curLine, sizeof(curLine));
        curLine_str = string(curLine);
        wholeFile = wholeFile + curLine_str + '\n';
        lineCnt++;
      }

      // 3.2 insert bounding function after current line
      wholeFile = wholeFile + getIndent(curLine_str) + getInsertedBFStr(bvToken) + '\n';

      // 3.3 update nextModLineNum and bvToken
      iter++;
      if (iter != bvWriteLineMap_r.end()) {
        nextModLineNum = iter-> first;
        bvToken = iter-> second;
      }
      else {
        nextModLineNum = -1;
      }
    }
  }
  in.close();

  // 4. output to the file
  ofstream out;
  out.open(dest.c_str()); // would create one if not exists
  if (out.fail()) {
    cout << "Open file " << dest << " in write mode failed!" << endl;
    return;
  }

  out.flush();
  out << wholeFile;
  out.close();
}

#endif
