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
// along with this program; if not, see <https://www.gnu.org/licenses/>.

#include <apertium/tagger.h>
#include <lttoolbox/lt_locale.h>

#include "getopt_long.h"
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <locale>
#include <sstream>
#include <string>
#include <unistd.h>

int main(int argc, char **argv) {
  LtLocale::tryToSetLocale();
  try {
    Apertium::apertium_tagger(argc, argv);
  } catch (const Apertium::Exception::apertium_tagger::err_Exception &err_Exception_) {
    std::cerr << "Try 'apertium-tagger --help' for more information." << std::endl;
    return 1;
  } catch (...) {
    throw;
  }
}
