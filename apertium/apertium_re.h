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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

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
