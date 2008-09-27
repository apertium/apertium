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
#ifndef _ENDIANDOUBLEUTIL_
#define _ENDIANDOUBLEUTIL_

#include <cctype>
#include <cstdio>
#include <iostream>

using namespace std;

/**
 * Generic class to process correctly endian-enabled I/O operations
 */
class EndianDoubleUtil
{
public:
  /**
   * Read procedure.
   * @param input the stream to read from.
   * @returns the first element readed from the current position of the stream
   */
  static double read(FILE *input);

  /**
   * Read procedure, C++ I/O version.
   * @param is the stream to read from.
   * @returns the first element readed from the current position of the stream
   */
  static double read(istream &is);
  
  /**
   * Write procedure.
   * @param output the stream to write to
   * @param val the value of the generic object to write to the stream
   */
  static void write(FILE *output, double const &val);
  
  /**
   * Write procedure, C++ I/O version.
   * @param output the stream to write to
   * @param val the value of the generic object to write to the stream
   */
  static void write(ostream &os, double const &val);
};

#endif
