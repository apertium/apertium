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

#include <cstdio>
#include <iostream>
#include <apertium/endian_double_util.h>
#include <apertium/apertium_config.h>
#include <apertium/unlocked_cstdio.h>

using namespace std;

double 
EndianDoubleUtil::read(FILE *input)
{
  double retval;
#ifdef WORDS_BIGENDIAN
  fread_unlocked(&retval, sizeof(double), 1, input);
#else
  char *s = reinterpret_cast<char *>(&retval);

  for(int i = sizeof(double)-1; i != -1; i--)
  {
    if(fread_unlocked(&(s[i]), 1, 1, input)==0)
    {
      return 0;
    }
  } 
#endif
  return retval;
}

double
EndianDoubleUtil::read(istream &is)
{
  double retval;
#ifdef WORDS_BIGENDIAN
  is.read((char *) &retval, sizeof(double));
#else
  char *s = reinterpret_cast<char *>(&retval);

  for(int i = sizeof(double)-1; i != -1; i--)
  {
    is.read(&(s[i]), sizeof(char));
  } 
#endif
  return retval;    
}
  
void 
EndianDoubleUtil::write(FILE *output, double const &val)
{
  double val2 = val;
#ifdef WORDS_BIGENDIAN
  fwrite(&val2, sizeof(double), 1, output);
#else
  char *s = reinterpret_cast<char *>(&val2);
    
  for(int i = sizeof(double)-1; i != -1; i--)
  {
    fwrite(&(s[i]), 1, 1, output);
  }
#endif
}

void 
EndianDoubleUtil::write(ostream &os, double const &val)
{
  double val2 = val;
#ifdef WORDS_BIGENDIAN
  os.write(reinterpret_cast<char *>(&val2), sizeof(double));
#else
  char *s = reinterpret_cast<char *>(&val2);
    
  for(int i = sizeof(double)-1; i != -1; i--)
  {
    os.write(&(s[i]), sizeof(char));
  }
#endif
}
