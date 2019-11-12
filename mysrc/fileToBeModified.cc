#include "foo.h"
template <class T> bool f( T x )
{
  bool res = false;
  int y, z;
  [joezie 1997]int y = 1;
  z = y;
  func(res, x, y + z);
  return res;
}

