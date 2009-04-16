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

#include <apertium/tagger.h>
#include <apertium/tsx_reader.h>
#include <lttoolbox/match_exe.h>
#include <lttoolbox/match_state.h>
#include <lttoolbox/lt_locale.h>
#include <iostream>

#include <cstdlib>
#include <apertium/string_utils.h>

using namespace Apertium;
using namespace std;

int main(int argc, char *argv[])
{
  LtLocale::tryToSetLocale();
  Tagger t;
  t.main(argc, argv);

  return EXIT_SUCCESS;
}
