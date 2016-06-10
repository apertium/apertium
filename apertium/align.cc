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

#include "align.h"

#include "linebreak.h"

#include <iomanip>
#include <ios>
#include <iostream>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace Apertium {
void align::align_(
    const std::vector<std::pair<std::string, std::string> > &string_) {
  const std::streamsize width_ = col(string_) + 2;

  for (std::vector<std::pair<std::string, std::string> >::const_iterator i_ =
           string_.begin();
       i_ != string_.end(); ++i_) {
    std::cerr << "  " << std::setw(width_) << std::left << i_->first
              << std::setw(0)
              << linebreak::linebreak_(i_->second, width_ + 2, width_ + 4)
              << '\n';
  }
}

std::string::size_type
align::col(const std::vector<std::pair<std::string, std::string> > &string_) {
  std::string::size_type col_ = 0;

  for (std::vector<std::pair<std::string, std::string> >::const_iterator i_ =
           string_.begin();
       i_ != string_.end(); ++i_) {
    if (i_->first.size() > col_)
      col_ = i_->first.size();
  }

  return col_;
}
}
