#ifndef CURSORWRAPPERS_H
#define CURSORWRAPPERS_H

#include <string>
#include <clang-c/Index.h>
using namespace std;

/* Wrapper functions for some cursor operations
 */

string getCursorKindName( CXCursorKind cursorKind )
{
  CXString kindName  = clang_getCursorKindSpelling( cursorKind );
  string result = clang_getCString( kindName );

  clang_disposeString( kindName );
  return result;
}

string getCursorSpelling( CXCursor cursor )
{
  CXString cursorSpelling = clang_getCursorSpelling( cursor );
  string result      = clang_getCString( cursorSpelling );

  clang_disposeString( cursorSpelling );
  return result;
}


string getCursorTypeName( CXType cursorType )
{
  CXString typeName  = clang_getTypeSpelling( cursorType );
  string result = clang_getCString( typeName );

  clang_disposeString( typeName );
  return result;
}

#endif
