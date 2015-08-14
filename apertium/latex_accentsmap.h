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

#include <map>
#include <iostream>
#include <cwchar>
#include <string>
#include <cstring>
#include <locale>
#include <lttoolbox/ltstr.h>

using namespace std;

/*struct Ltstr // Already in lttoolbox/ltstr.h
{
  bool operator()(wstring const &s1, wstring const &s2) const
  {
    return wcscmp(s1.c_str(), s2.c_str()) < 0;
  }
};
*/

class AccentsMap {
	typedef std::map<wstring, wstring, Ltstr> acmap;
	private:
		acmap           map; // Accent to character
		acmap::iterator it;  // Iterator for searching

		void init_acmap();
		void init_camap();
	public:
		AccentsMap(bool char2accent); // the direction
		~AccentsMap();

		// Optionally
		void init_locale(); 

		// The getter for both directions depending on init.
		wstring get(wstring input);
};

