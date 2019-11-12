#include "foo.h"
template <class T> bool f( T x )
{
  bool res = false;
  int y, z;
  y = 1;
  z = y;
  func(res, x, y + z);
  return res;
}

