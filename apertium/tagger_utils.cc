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

namespace tagger_utils {

void fatal_error (wstring const &s) {
  wcerr<<L"Error: "<<s<<L"\n";
  exit(1);
}

void file_name_error (string s) { 
  cerr << "Error: " << s << endl;
  exit(1);
}

char *itoa(int i) {                 
  static char buf[512];
  sprintf(buf,"%d",i);
  return buf;
}

void clear_array_double(double a[], int l) {
  for(int i=0; i<l; i++)
    a[i]=0.0;
}

void clear_array_vector(vector<TTag> v[], int l) {
  for(int i=0; i<l; i++)
    v[i].clear();
}

int ntokens_multiword(string s) {
   char *news = strdup((char*) s.c_str());
   char *delim ="_";
   int n=0;
   
   if (strtok(news,delim))
     n++;  
   while (strtok(NULL, delim))
     n++;
     
   return n;   
}
 
int nguiones_fs(string s) {
   char *news = strdup((char*) s.c_str());
   char *delim ="-";
   int n=0;

   if (strtok(news,delim))
     n++;  
   while (strtok(NULL, delim))
     n++;
   return n;   
} 

wstring trim(wstring s) 
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
  
};  

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

