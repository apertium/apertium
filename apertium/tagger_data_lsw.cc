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
#include <apertium/lswpost.h>
#include <apertium/tagger_data_lsw.h>
#include <lttoolbox/compression.h>
#include <apertium/endian_double_util.h>
#include <apertium/string_utils.h>

using namespace Apertium;

void
TaggerDataLSW::destroy()
{
  if (d != NULL) {
    for (int i = 0; i < N; ++i) {
      for (int j = 0; j < N; ++j) {
        delete [] d[i][j];
      }
      delete [] d[i];
    }
  delete [] d;
  }
  d = NULL;
  
  N = 0;
}

TaggerDataLSW::TaggerDataLSW()
{
  d = NULL;
  N = 0;
}

TaggerDataLSW::~TaggerDataLSW()
{
  destroy();
}

TaggerDataLSW::TaggerDataLSW(TaggerDataLSW const &o)
{
  d = NULL;
  N = 0;
  TaggerData::copy(o);
  this->setProbabilities(o.N, o.d);
}

TaggerDataLSW::TaggerDataLSW(TaggerData const &o)
{
  d = NULL;
  N = 0;
  TaggerData::copy(o);
}

TaggerDataLSW &
TaggerDataLSW::operator =(TaggerDataLSW const &o)
{
  if(this != &o)
  {
    destroy();
    TaggerData::copy(o);
    this->setProbabilities(o.N, o.d);
  }
  return *this;
}

void
TaggerDataLSW::setProbabilities(int const myN, double ***myD) {
   this->destroy();
   N = myN;
   if(N != 0) {
     d = new double ** [N];
     for (int i = 0; i < N; ++i) {
       d[i] = new double * [N];
       for (int j = 0; j < N; ++j) {
           d[i][j] = new double [N];
           if (myD != NULL) {
              for (int k = 0; k < N; ++k) {
                d[i][j][k] = myD[i][j][k];
              }
           }
       }
     }
   } else {
     d = NULL;
   }
}

double ***
TaggerDataLSW::getD() {
  return d;
}

int 
TaggerDataLSW::getN()
{  
  return N;
}

void
TaggerDataLSW::read(FILE *in)
{
  destroy();

  // open_class
  int val = 0;
  for(int i = Compression::multibyte_read(in); i != 0; i--)
  {
    val += Compression::multibyte_read(in);
    open_class.insert(val);
  }
  
  // forbid_rules
  for(int i = Compression::multibyte_read(in); i != 0; i--)
  {
    TForbidRule aux;
    aux.tagi = Compression::multibyte_read(in);
    aux.tagj = Compression::multibyte_read(in);
    forbid_rules.push_back(aux);
  }

  
  // array_tags
  for(int i = Compression::multibyte_read(in); i != 0; i--)
  {
    array_tags.push_back(Compression::wstring_read(in));
  }
  
  // tag_index
  for(int i = Compression::multibyte_read(in); i != 0; i--)
  {
    wstring tmp = Compression::wstring_read(in);    
    tag_index[tmp] = Compression::multibyte_read(in);
  }

  // enforce_rules  
  for(int i = Compression::multibyte_read(in); i != 0; i--)
  {
    TEnforceAfterRule aux;
    aux.tagi = Compression::multibyte_read(in);
    for(int j = Compression::multibyte_read(in); j != 0; j--)
    {
      aux.tagsj.push_back(Compression::multibyte_read(in));
    }
    enforce_rules.push_back(aux);
  }

  // prefer_rules
  for(int i = Compression::multibyte_read(in); i != 0; i--)
  {
    prefer_rules.push_back(Compression::wstring_read(in));
  }

  // constants
  constants.read(in);

  // output
  output.read(in); 

  // dimensions
  N = Compression::multibyte_read(in);

  d = new double ** [N];
  for ( int i = 0; i < N; ++i) {
    d[i] = new double * [N];
    for (int j = 0; j < N; ++j) {
      d[i][j] = new double [N];
    }
  }

  // initializing d matrix
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      for (int k = 0; k < N; ++k) {
        d[i][j][k] = 0;
      }
    }
  }

  int nval = Compression::multibyte_read(in);
  for(; nval != 0; nval--) {
    int i = Compression::multibyte_read(in);
    int j = Compression::multibyte_read(in);
    int k = Compression::multibyte_read(in);
    d[i][j][k] = EndianDoubleUtil::read(in);
  }
   
  // read pattern list
  plist.read(in);
    
  // read discards on ambiguity
  discard.clear();

  int limit = Compression::multibyte_read(in);  
  if(feof(in))
  {
    return;
  }
  
  for(int i = 0; i < limit; i++)
  {
    discard.push_back(Compression::wstring_read(in));
  }
}

void
TaggerDataLSW::write(FILE *out)
{
  
  // open_class
  Compression::multibyte_write(open_class.size(), out);  
  int val = 0;
  for(set<TTag>::const_iterator it = open_class.begin(), limit = open_class.end();
      it != limit; it++)
  {
    Compression::multibyte_write(*it-val, out);    
    val = *it;
  }
  
  // forbid_rules
  Compression::multibyte_write(forbid_rules.size(), out);
  for(unsigned int i = 0, limit = forbid_rules.size(); i != limit; i++)
  {
    Compression::multibyte_write(forbid_rules[i].tagi, out);
    Compression::multibyte_write(forbid_rules[i].tagj, out);
  }
  
  // array_tags
  Compression::multibyte_write(array_tags.size(), out);
  for(unsigned int i = 0, limit = array_tags.size(); i != limit; i++)
  {
    Compression::wstring_write(array_tags[i], out);
  }

  // tag_index
  Compression::multibyte_write(tag_index.size(), out);
  for(map<wstring, int, Ltstr>::iterator it = tag_index.begin(), limit = tag_index.end();
      it != limit; it++)
  {
    Compression::wstring_write(it->first, out);
    Compression::multibyte_write(it->second, out);
  }
  
  // enforce_rules
  Compression::multibyte_write(enforce_rules.size(), out);
  for(unsigned int i = 0, limit = enforce_rules.size(); i != limit; i++)
  {
    Compression::multibyte_write(enforce_rules[i].tagi, out);
    Compression::multibyte_write(enforce_rules[i].tagsj.size(), out);
    for(unsigned int j = 0, limit2 = enforce_rules[i].tagsj.size(); j != limit2; j++)
    {
      Compression::multibyte_write(enforce_rules[i].tagsj[j], out);
    }
  }

  // prefer_rules
  Compression::multibyte_write(prefer_rules.size(), out);
  for(unsigned int i = 0, limit = prefer_rules.size(); i != limit; i++)
  {
    Compression::wstring_write(prefer_rules[i], out);
  }
  
  // constants
  constants.write(out);  

  // output
  output.write(out);

  // d matrix
  Compression::multibyte_write(N, out);

  int nval = 0;
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      for (int k = 0; k < N; ++k) {
        if (d[i][j][k] > ZERO) {
          ++nval;
        }
      }
    }
  }
  Compression::multibyte_write(nval, out);

  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      for (int k = 0; k < N; ++k) {
        if (d[i][j][k] > ZERO) {
          Compression::multibyte_write(i, out);
          Compression::multibyte_write(j, out);
          Compression::multibyte_write(k, out);
          EndianDoubleUtil::write(out, d[i][j][k]);
        }
      }
    }
  }
  
  // write pattern list
  plist.write(out);
  
  // write discard list
  
  if(discard.size() != 0)
  {
    Compression::multibyte_write(discard.size(), out);
    for(unsigned int i = 0, limit = discard.size(); i != limit; i++)
    {
      Compression::wstring_write(discard[i], out);
    }
  }  
}

