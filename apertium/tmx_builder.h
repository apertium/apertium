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
#ifndef _TMXBUILDER_
#define _TMXBUILDER_

#include <apertium/transfer_data.h>
#include <lttoolbox/ltstr.h>

#include <string>

using namespace std;

class TMXBuilder
{
private:
  static wstring restOfBlank(FILE *input);
  static wstring nextBlank(FILE *input);
  static bool compatible(FILE *input, FILE *output, bool lazy = false);
public:
  TMXBuilder();
  ~TMXBuilder();
  static bool check(string const &file1, string const &file2, bool lazy = false);
  static void generate(string const &file1, string const &file2, 
                       string const &outfile="");
};

#endif
