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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */
#include <apertium/constant_manager.h>
#include <lttoolbox/compression.h>
#include <apertium/string_utils.h>
#include <apertium/serialiser.h>
#include <apertium/deserialiser.h>

using namespace Apertium;
void
ConstantManager::copy(ConstantManager const &o)
{
  constants = o.constants;
}

void
ConstantManager::destroy()
{
}

ConstantManager::ConstantManager()
{
}

ConstantManager::~ConstantManager()
{
  destroy();
}

ConstantManager::ConstantManager(ConstantManager const &o)
{
  copy(o);
}

ConstantManager &
ConstantManager::operator =(ConstantManager const &o)
{
  if(this != &o)
  {
    destroy();
    copy(o);
  }
  return *this;
}
void 
ConstantManager::setConstant(wstring const &constant, int const value)
{
  constants[constant] = value;
}

int 
ConstantManager::getConstant(wstring const &constant)
{
  return constants[constant];
}  

void
ConstantManager::write(FILE *output)
{
  Compression::multibyte_write(constants.size(), output);

  for(map<wstring, int>::const_iterator it = constants.begin(), limit = constants.end();
      it != limit; it++)
  {
    Compression::wstring_write(it->first, output);
    Compression::multibyte_write(it->second, output);
  }
}

void
ConstantManager::read(FILE *input)
{
  constants.clear();
  int size = Compression::multibyte_read(input);
  for(int i = 0; i != size; i++)
  {
    wstring mystr = Compression::wstring_read(input);
    constants[mystr] = Compression::multibyte_read(input);
  }
}

void
ConstantManager::serialise(std::ostream &serialised) const
{
  Serialiser<map<wstring, int> >::serialise(constants, serialised);
}

void
ConstantManager::deserialise(std::istream &serialised)
{
  constants = Deserialiser<map<wstring, int> >::deserialise(serialised);
}
