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
#ifndef _TAGGERDATALSW_
#define _TAGGERDATALSW_

#include <apertium/tagger_data.h>

class TaggerDataLSW : public TaggerData
{
private:
  int N;
  double ***d;
  
  void destroy();

public:
  TaggerDataLSW();
  virtual ~TaggerDataLSW();
  TaggerDataLSW(TaggerDataLSW const &o);
  TaggerDataLSW(TaggerData const &o);
  TaggerDataLSW & operator =(TaggerDataLSW const &o);
  
  void setProbabilities(int const myN, double ***myD = NULL);

  virtual double *** getD();
  virtual int getN();
  
  void read(FILE *in);
  void write(FILE *out);
};

#endif
