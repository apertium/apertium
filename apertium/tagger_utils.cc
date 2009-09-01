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
#include <apertium/tagger_utils.h>

#include <stdio.h>
#include <apertium/string_utils.h>
#ifdef _MSC_VER
#define wcstok wcstok_s
#endif
#ifdef __MINGW32__

wchar_t *_wcstok(wchar_t *wcs, const wchar_t *delim, wchar_t **ptr) {
  (void)ptr;
  return wcstok(wcs, delim);
}

#define wcstok _wcstok
#endif

using namespace Apertium;


void tagger_utils::fatal_error (wstring const &s) {
  wcerr<<L"Error: "<<s<<L"\n";
  exit(1);
}

void tagger_utils::file_name_error (string const &s) { 
  cerr << "Error: " << s << endl;
  exit(1);
}

char * tagger_utils::itoa(int i) {                 
  static char buf[512];
  sprintf(buf,"%d",i);
  return buf;
}

void tagger_utils::clear_array_double(double a[], int l) {
  for(int i=0; i<l; i++)
    a[i]=0.0;
}

void tagger_utils::clear_array_vector(vector<TTag> v[], int l) {
  for(int i=0; i<l; i++)
    v[i].clear();
}

int tagger_utils::ntokens_multiword(wstring const &s) 
{
   wchar_t *news = new wchar_t[s.size()+1];
   wcscpy(news, s.c_str());
   news[s.size()] = 0;
   wcerr << news << endl;
   
   wchar_t const *delim = L"_";
   wchar_t *ptr;
   int n=0;
   
   if (wcstok(news, delim, &ptr))
     n++;  
   while (wcstok(NULL, delim, &ptr))
     n++;
     
   delete[] news;
     
   return n;   
}
 
int tagger_utils::nguiones_fs(wstring const & s) {
   wchar_t *news = new wchar_t[s.size()+1];
   wcscpy(news, s.c_str());
   news[s.size()] = 0;
   wcerr << news << endl;   
   wchar_t const *delim = L"-";
   wchar_t *ptr;
   int n=0;
   
   if (wcstok(news, delim, &ptr))
     n++;  
   while (wcstok(NULL, delim, &ptr))
     n++;
     
   delete[] news;
     
   return n;   
} 

wstring tagger_utils::trim(wstring s) 
{
  if (s.length()==0)
    return L"";
      
  for (unsigned int i=0; i<(s.length()-1); i++) {
    if ((s.at(i)==L' ')&&(s.at(i+1)==L' ')) {
      s.erase(i,1);
      i--;
    }
  }
                              
  if ((s.length()>0)&&(s.at(s.length()-1)==L' '))
    s.erase(s.length()-1,1);
  if ((s.length()>0)&&(s.at(0)==L' '))
    s.erase(0,1);  

  return s;
}
  
template <class T>
ostream& operator<< (ostream& os, const map <int, T> & f){
  typename map <int, T>::const_iterator it;
  os<<f.size();
  for (it=f.begin(); it!=f.end(); it++) 
    os<<' '<<it->first<<' '<<it->second;
  return os;
}

template <class T>
istream& operator>> (istream& is, map <int, T> & f) {
  int n, i, k;
  f.clear();
  is>>n; 
  for (k=0; k<n; k++) {
    is>>i;     // warning: does not work if both
    is>>f[i];  // lines merged in a single one
  }
  if (is.bad()) tagger_utils::fatal_error(L"reading map");
  return is;
}

template <class T>
ostream& operator<< (ostream& os, const set<T>& s) {
  typename set<T>::iterator it = s.begin();
  os<<'{';
  if (it!=s.end()) {
    os<<*it;
    while (++it!=s.end()) os<<','<<*it;
  }
  os<<'}';
  return os;
}

