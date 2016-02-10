// Copyright (C) 2005 Universitat d'Alacant / Universidad de Alicante
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.

#include "linebreak.h"

#include <string>

namespace Apertium {
std::string linebreak::linebreak_(std::string string_,
                                  std::string::size_type col,
                                  const std::string::size_type &wrapmargin) {
  std::string::size_type i_ = 0;

  while (true) {
    if (i_ == string_.size())
      return string_;

    if (col < 79) {
      if (string_.at(i_) == '\n') {
        if (i_ + 1 == string_.size()) {
          string_.erase(i_, 1);
          return string_;
        }

        string_.insert(i_ + 1, wrapmargin, ' ');
        col = wrapmargin;
        i_ += wrapmargin;
        continue;
      }

      ++col;
      ++i_;
      continue;
    }

    if (string_.at(i_) == ' ') {
      std::string::size_type j_ = i_ + 1;

      for (; i_ != 0; --i_) {
        if (string_.at(i_ - 1) != ' ')
          break;
      }

      for (;; ++j_) {
        if (j_ == string_.size()) {
          string_.erase(i_, j_ - i_);
          return string_;
        }

        if (string_.at(j_) != ' ')
          break;
      }

      linebreak_(string_, col, wrapmargin, i_, j_);
      continue;
    }

    std::string::size_type j_ = i_;

    for (; j_ != 0; --j_) {
      if (string_.at(j_ - 1) == ' ')
        break;
    }

    for (i_ = j_; i_ != 0; --i_) {
      if (string_.at(i_ - 1) != ' ')
        break;
    }

    linebreak_(string_, col, wrapmargin, i_, j_);
  }
}

void linebreak::linebreak_(std::string &string_, std::string::size_type &col,
                           const std::string::size_type &wrapmargin,
                           std::string::size_type &i_,
                           const std::string::size_type &j_) {
  string_.replace(i_, j_ - i_, '\n' + std::string(wrapmargin, ' '));
  col = wrapmargin;
  i_ += 1 /* '\n' */ + wrapmargin /* std::string(wrapmargin, ' ') */;
}
}
