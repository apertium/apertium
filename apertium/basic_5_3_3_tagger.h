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

#ifndef BASIC_5_3_3_TAGGER_H
#define BASIC_5_3_3_TAGGER_H

#include "i.h"
#include "lemma.h"

#include <cstddef>
#include <map>
#include <utility>

namespace Apertium {
class basic_5_3_3_Tagger {
protected:
  std::pair<std::map<i, std::map<Lemma, std::size_t> >,
            std::pair<std::map<i, std::map<Lemma, std::size_t> >,
                      std::map<Lemma, std::map<i, std::size_t> > > > Model;
};
}

#endif // BASIC_5_3_3_TAGGER_H
