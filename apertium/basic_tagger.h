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

#ifndef BASIC_TAGGER_H
#define BASIC_TAGGER_H

namespace Apertium {
class basic_Tagger {
public:
  class Flags {
  public:
    Flags();
    bool getDebug() const;
    void setDebug(const bool &Debug_);
    bool getSentSeg() const;
    void setSentSeg(const bool &SentSeg);
    bool getSkipErrors() const;
    void setSkipErrors(const bool &SkipErrors_);
    bool getFirst() const;
    void setFirst(const bool &First_);
    bool getMark() const;
    void setMark(const bool &Mark_);
    bool getShowSuperficial() const;
    void setShowSuperficial(const bool &ShowSuperficial_);
    bool getNullFlush() const;
    void setNullFlush(const bool &NullFlush_);
    static bool (Flags::*GetDebug)() const;
    static void (Flags::*SetDebug)(const bool &);
    static bool (Flags::*GetFirst)() const;
    static void (Flags::*SetFirst)(const bool &);
    static bool (Flags::*GetMark)() const;
    static void (Flags::*SetMark)(const bool &);
    static bool (Flags::*GetShowSuperficial)() const;
    static void (Flags::*SetShowSuperficial)(const bool &);
    static bool (Flags::*GetNullFlush)() const;
    static void (Flags::*SetNullFlush)(const bool &);

  private:
    bool Debug : 1;
    bool SentSeg : 1;
    bool SkipErrors : 1;
    bool First : 1;
    bool Mark : 1;
    bool ShowSuperficial : 1;
    bool NullFlush : 1;
  };

protected:
  basic_Tagger();
  basic_Tagger(const Flags &Flags_);
  Flags TheFlags;
};
}

#endif // BASIC_TAGGER_H
