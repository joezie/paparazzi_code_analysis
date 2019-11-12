#ifndef CALLBACKFUNCS_H
#define CALLBACKFUNCS_H

#include "ASTAnalysis.h"
/* Callback functions of clang_visitChildren
 */

CXChildVisitResult buildDict( CXCursor cursor, CXCursor /* parent */, CXClientData clientData )
{ // traverse the AST to build dict
  CXSourceLocation location = clang_getCursorLocation( cursor );

  CXCursorKind cursorKind = clang_getCursorKind( cursor );

  VisitorStruct vs = *( reinterpret_cast<VisitorStruct*>( clientData ) );
  unsigned int curLevel = vs.curLevel;
  bool foundBound = vs.foundBound;

  if ( !foundBound ) 
    // if not found yet, then the task is to find Bound
    foundBound = detectBF( cursor );
    
 
  // update VisitorStruct
  vs.curLevel = curLevel + 1;
  vs.foundBound = foundBound;
  
  clang_visitChildren( cursor,
                       buildDict,
                       &vs); 

  return CXChildVisit_Continue;
}

CXChildVisitResult buildBVLineMap( CXCursor cursor, CXCursor /* parent */, CXClientData clientData )
{ // traverse the AST to build bvReadLineMap/bvWriteLineMap/bvDeclLineMap
  CXCursorKind cursorKind = clang_getCursorKind( cursor );

  bool foundBV = *( reinterpret_cast<bool*>( clientData ) );

  if ( !foundBV ) 
    // if not found yet, then the task is to find BV
    foundBV = detectBV( cursor );
      
  clang_visitChildren( cursor,
                       buildBVLineMap,
                       &foundBV); 

  return CXChildVisit_Continue;
}

CXChildVisitResult visitor( CXCursor cursor, CXCursor /* parent */, CXClientData clientData )
{ // traverse the AST and output cursors
  CXSourceLocation location = clang_getCursorLocation( cursor );

  CXCursorKind cursorKind = clang_getCursorKind( cursor );

  unsigned int curLevel = *( reinterpret_cast<unsigned int*>( clientData ) );


  cout << string( curLevel, '-' ) << " " << getCursorKindName(cursorKind ) << " (" << getCursorSpelling( cursor ) << ")\n";
  
  //modifyArgDecl(cursor); // print if it's bounded arguments
  
  unsigned int nextLevel = curLevel + 1; // update level
  
  clang_visitChildren( cursor,
                       visitor,
                       &nextLevel); 

  return CXChildVisit_Continue;
}

#endif
