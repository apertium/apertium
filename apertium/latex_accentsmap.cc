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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#include <apertium/latex_accentsmap.h>

using namespace std;


AccentsMap::AccentsMap(bool char2latex) {
	if(char2latex)
		init_camap();
	else
		init_acmap();
}

AccentsMap::~AccentsMap(){
}

void AccentsMap::init_acmap() {
	init_camap();
	for (acmap::iterator i = map.begin();
		i != map.end();
		++i)
	{
		map[i->second] = i->first;
	}
}

void AccentsMap::init_camap() {

	map[u"à"_u] = "`a"_u; // Grave accent
	map[u"è"_u] = "`e"_u;
	map[u"ì"_u] = "`\\i"_u;
	map[u"ò"_u] = "`o"_u;
	map[u"ù"_u] = "`u"_u;
	map[u"ỳ"_u] = "`y"_u;
	map[u"À"_u] = "`A"_u;
	map[u"È"_u] = "`E"_u;
	map[u"Ì"_u] = "`I"_u;
	map[u"Ò"_u] = "`O"_u;
	map[u"Ù"_u] = "`U"_u;
	map[u"Ỳ"_u] = "`Y"_u;
	map[u"á"_u] = "'a"_u; // Acute accent
	map[u"é"_u] = "'e"_u;
	map[u"í"_u] = "'\\i"_u;
	map[u"ó"_u] = "'o"_u;
	map[u"ú"_u] = "'u"_u;
	map[u"ý"_u] = "'y"_u;
	map[u"Á"_u] = "'A"_u;
	map[u"É"_u] = "'E"_u;
	map[u"Í"_u] = "'I"_u;
	map[u"Ó"_u] = "'O"_u;
	map[u"Ú"_u] = "'U"_u;
	map[u"Ý"_u] = "'Y"_u;
	map[u"â"_u] = "^a"_u; // Circumflex
	map[u"ê"_u] = "^e"_u;
	map[u"î"_u] = "^\\i"_u;
	map[u"ô"_u] = "^o"_u;
	map[u"û"_u] = "^u"_u;
	map[u"ŷ"_u] = "^y"_u;
	map[u"Â"_u] = "^A"_u;
	map[u"Ê"_u] = "^E"_u;
	map[u"Î"_u] = "^I"_u;
	map[u"Ô"_u] = "^O"_u;
	map[u"Û"_u] = "^U"_u;
	map[u"Ŷ"_u] = "^Y"_u;
	map[u"ä"_u] = "\"a"_u;    // Umlaut or dieresis
	map[u"ë"_u] = "\"e"_u;
	map[u"ï"_u] = "\"\\i"_u;
	map[u"ö"_u] = "\"o"_u;
	map[u"ü"_u] = "\"u"_u;
	map[u"ÿ"_u] = "\"y"_u;
	map[u"Ä"_u] = "\"A"_u;
	map[u"Ë"_u] = "\"E"_u;
	map[u"Ï"_u] = "\"I"_u;
	map[u"Ö"_u] = "\"O"_u;
	map[u"Ü"_u] = "\"U"_u;
	map[u"Ÿ"_u] = "\"Y"_u;

	map[u"ñ"_u] = "~n"_u;
	map[u"Ñ"_u] = "~N"_u;

	map[u"ç"_u] = "cc"_u;   // Cedilla
	map[u"Ç"_u] = "cC"_u;


}

UString AccentsMap::get(UString input){
	it = map.find(input);
	if(it == map.end())
		return ""_u;
	else
		return (*it).second;
}

//Optionally:
void AccentsMap::init_locale(){
	char *locale = setlocale(LC_ALL, "");
	std::locale lollocale(locale);
	cout.imbue(lollocale);
}



/*latexAccents = [
	map[u"à"_u] = "\\`a"_u; # Grave accent
	map[u"è"_u] = "\\`e"_u;
	map[u"ì"_u] = "\\`\\i"_u;
	map[u"ò"_u] = "\\`o"_u;
	map[u"ù"_u] = "\\`u"_u;
	map[u"ỳ"_u] = "\\`y"_u;
	map[u"À"_u] = "\\`A"_u;
	map[u"È"_u] = "\\`E"_u;
	map[u"Ì"_u] = "\\`\\I"_u;
	map[u"Ò"_u] = "\\`O"_u;
	map[u"Ù"_u] = "\\`U"_u;
	map[u"Ỳ"_u] = "\\`Y"_u;
	map[u"á"_u] = "\\'a"_u; # Acute accent
	map[u"é"_u] = "\\'e"_u;
	map[u"í"_u] = "\\'\\i"_u;
	map[u"ó"_u] = "\\'o"_u;
	map[u"ú"_u] = "\\'u"_u;
	map[u"ý"_u] = "\\'y"_u;
	map[u"Á"_u] = "\\'A"_u;
	map[u"É"_u] = "\\'E"_u;
	map[u"Í"_u] = "\\'\\I"_u;
	map[u"Ó"_u] = "\\'O"_u;
	map[u"Ú"_u] = "\\'U"_u;
	map[u"Ý"_u] = "\\'Y"_u;
	map[u"â"_u] = "\\^a"_u; # Circumflex
	map[u"ê"_u] = "\\^e"_u;
	map[u"î"_u] = "\\^\\i"_u;
	map[u"ô"_u] = "\\^o"_u;
	map[u"û"_u] = "\\^u"_u;
	map[u"ŷ"_u] = "\\^y"_u;
	map[u"Â"_u] = "\\^A"_u;
	map[u"Ê"_u] = "\\^E"_u;
	map[u"Î"_u] = "\\^\\I"_u;
	map[u"Ô"_u] = "\\^O"_u;
	map[u"Û"_u] = "\\^U"_u;
	map[u"Ŷ"_u] = "\\^Y"_u;
	map[u"ä"_u] = "\\\"a"_u;    # Umlaut or dieresis
	map[u"ë"_u] = "\\\"e"_u;
	map[u"ï"_u] = "\\\"\\i"_u;
	map[u"ö"_u] = "\\\"o"_u;
	map[u"ü"_u] = "\\\"u"_u;
	map[u"ÿ"_u] = "\\\"y"_u;
	map[u"Ä"_u] = "\\\"A"_u;
	map[u"Ë"_u] = "\\\"E"_u;
	map[u"Ï"_u] = "\\\"\\I"_u;
	map[u"Ö"_u] = "\\\"O"_u;
	map[u"Ü"_u] = "\\\"U"_u;
	map[u"Ÿ"_u] = "\\\"Y"_u;
	map[u"ç"_u] = "\\c{c}"_u;   # Cedilla
	map[u"Ç"_u] = "\\c{C}"_u;
	map[u"œ"_u] = "{\\oe}"_u;   # Ligatures
	map[u"Œ"_u] = "{\\OE}"_u;
	map[u"æ"_u] = "{\\ae}"_u;
	map[u"Æ"_u] = "{\\AE}"_u;
	map[u"å"_u] = "{\\aa}"_u;
	map[u"Å"_u] = "{\\AA}"_u;
	map[u"–"_u] = "--"_u;   # Dashes
	map[u"—"_u] = "---"_u;
	map[u"ø"_u] = "{\\o}"_u;    # Misc latin-1 letters
	map[u"Ø"_u] = "{\\O}"_u;
	map[u"ß"_u] = "{\\ss}"_u;
	map[u"¡"_u] = "{!`}"_u;
	map[u"¿"_u] = "{?`}"_u;
	map[u"\\"_u] = "\\\\"_u;    # Characters that should be quoted
	map[u"~"_u] = "\\~"_u;
	map[u"&"_u] = "\\&"_u;
	map[u"$"_u] = "\\$"_u;
	map[u"{"_u] = "\\{"_u;
	map[u"}"_u] = "\\}"_u;
	map[u"%"_u] = "\\%"_u;
	map[u"#"_u] = "\\#"_u;
	map[u"_"_u] = "\\_"_u;
	map[u"≥"_u] = "$\\ge$"_u;   # Math operators
	map[u"≤"_u] = "$\\le$"_u;
	map[u"≠"_u] = "$\\neq$"_u;
	map[u"©"_u] = "\copyright"_u; # Misc
	map[u"ı"_u] = "{\\i}"_u;
	map[u"µ"_u] = "$\\mu$"_u;
	map[u"°"_u] = "$\\deg$"_u;
	map[u"‘"_u] = "`"_u;    #Quotes
	map[u"’"_u] = "'"_u;
	map[u"“"_u] = "``"_u;
	map[u"”"_u] = "''"_u;
	map[u"‚"_u] = ","_u;
	map[u"„"_u] = ",,"_u;
]*/


void fputus(const UString& s, FILE* out)
{
  string temp;
  temp.reserve(s.size()*2);
  utf8::utf16to8(s.begin(), s.end(), std::back_inserter(temp));
  fputs(temp.c_str(), out);
}
