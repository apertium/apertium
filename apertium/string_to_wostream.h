// Include string_utils.h instead if you're linking against apertium

#ifndef __STRING_TO_WOSTREAM_H_
#define __STRING_TO_WOSTREAM_H_

#include <iterator>

std::wostream & operator<<(std::wostream & ostr, std::string const & str) {
  ostr << str.c_str();
  return ostr;
}

#endif
