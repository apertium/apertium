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

string trim(string s) {
  if (s.length()==0)
    return s;

  for (unsigned int i=0; i<(s.length()-1); i++) {
    if ((s.at(i)==' ')&&(s.at(i+1)==' ')) {
      s.erase(i,1);
      i--;
    }
  }

  if ((s.length()>0)&&(s.at(s.length()-1)==' '))
    s.erase(s.length()-1,1);
  if ((s.length()>0)&&(s.at(0)==' '))
    s.erase(0,1);

  return s;
}
