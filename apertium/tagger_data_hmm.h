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
#ifndef _TAGGERDATAHMM_
#define _TAGGERDATAHMM_

#include <apertium/tagger_data.h>

class TaggerDataHMM : public TaggerData
{
private:
  int N;
  int M;
  double **a;
  double **b;

  void destroy();
public:
  TaggerDataHMM();
  virtual ~TaggerDataHMM();
  TaggerDataHMM(TaggerDataHMM const &o);
  TaggerDataHMM(TaggerData const &o);
  TaggerDataHMM & operator =(TaggerDataHMM const &o);
 
  virtual void setProbabilities(int const myN, int const myM, 
                        double **myA = NULL, double **myB = NULL);

  virtual double ** getA();
  virtual double ** getB();
  virtual int getN();
  virtual int getM();
  
  virtual void read(FILE *in);
  virtual void write(FILE *out);
};

#endif
