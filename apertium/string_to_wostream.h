// Include string_utils.h instead if you're linking against apertium

#ifndef __STRING_TO_WOSTREAM_H_
#define __STRING_TO_WOSTREAM_H_

#include <iterator>

std::wostream & operator<<(std::wostream & ostr, std::string const & str) {
  std::copy(
    str.begin(), str.end(),
    std::ostream_iterator<char, wchar_t>(ostr));
  return ostr;
}

#endif
