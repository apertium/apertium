#ifndef _APERTIUM_RE_
#define _APERTIUM_RE_

#include <pcre.h>
#include <cstdio>
#include <string>

using namespace std;

class ApertiumRE
{
private:
  bool empty;
  pcre *re;
  pcre_extra *extra;
public:
  ApertiumRE();
  ~ApertiumRE();
  void read(FILE *);
  void write(FILE *) const;
  string match(string const &str) const;
  void replace(string &str, string const &value) const;
  void compile(string const &str);
};

#endif
