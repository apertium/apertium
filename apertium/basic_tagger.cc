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

#include "basic_tagger.h"

namespace Apertium {
basic_Tagger::Flags::Flags()
    : Debug(false), First(false), Mark(false), ShowSuperficial(false),
      NullFlush(false) {}

bool basic_Tagger::Flags::getDebug() const { return Debug; }

void basic_Tagger::Flags::setDebug(const bool &Debug_) { Debug = Debug_; }

bool basic_Tagger::Flags::getSentSeg() const { return SentSeg; }

void basic_Tagger::Flags::setSentSeg(const bool &SentSeg_) { SentSeg = SentSeg_; }

bool basic_Tagger::Flags::getSkipErrors() const { return SkipErrors; }

void basic_Tagger::Flags::setSkipErrors(const bool &SkipErrors_) { SkipErrors = SkipErrors_; }

bool basic_Tagger::Flags::getFirst() const { return First; }

void basic_Tagger::Flags::setFirst(const bool &First_) { First = First_; }

bool basic_Tagger::Flags::getMark() const { return Mark; }

void basic_Tagger::Flags::setMark(const bool &Mark_) { Mark = Mark_; }

bool basic_Tagger::Flags::getShowSuperficial() const { return ShowSuperficial; }

void basic_Tagger::Flags::setShowSuperficial(const bool &ShowSuperficial_) {
  ShowSuperficial = ShowSuperficial_;
}

bool basic_Tagger::Flags::getNullFlush() const { return NullFlush; }

void basic_Tagger::Flags::setNullFlush(const bool &NullFlush_) {
  NullFlush = NullFlush_;
}

basic_Tagger::basic_Tagger() : TheFlags() {}

basic_Tagger::basic_Tagger(const Flags &Flags_) : TheFlags(Flags_) {}
}
