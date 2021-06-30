/*
 * Copyright (C) 2005 Universitat d'Alacant / Universidad de Alicante
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#ifndef _APERTIUM_RE_
#define _APERTIUM_RE_

#include <cstdio>
#include <unicode/regex.h>
#include <lttoolbox/ustring.h>

using namespace std;

class ApertiumRE
{
private:
  icu::RegexPattern* re = nullptr;
public:
  ApertiumRE();
  ~ApertiumRE();
  void read(FILE *);
  void write(FILE *) const;
  UString match(UString const &str) const;
  bool replace(UString &str, UString const &value) const;
  void compile(UString const &str);
};

#endif
