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

	map["à"_u] = "`a"_u; // Grave accent
	map["è"_u] = "`e"_u;
	map["ì"_u] = "`\\i"_u;
	map["ò"_u] = "`o"_u;
	map["ù"_u] = "`u"_u;
	map["ỳ"_u] = "`y"_u;
	map["À"_u] = "`A"_u;
	map["È"_u] = "`E"_u;
	map["Ì"_u] = "`I"_u;
	map["Ò"_u] = "`O"_u;
	map["Ù"_u] = "`U"_u;
	map["Ỳ"_u] = "`Y"_u;
	map["á"_u] = "'a"_u; // Acute accent
	map["é"_u] = "'e"_u;
	map["í"_u] = "'\\i"_u;
	map["ó"_u] = "'o"_u;
	map["ú"_u] = "'u"_u;
	map["ý"_u] = "'y"_u;
	map["Á"_u] = "'A"_u;
	map["É"_u] = "'E"_u;
	map["Í"_u] = "'I"_u;
	map["Ó"_u] = "'O"_u;
	map["Ú"_u] = "'U"_u;
	map["Ý"_u] = "'Y"_u;
	map["â"_u] = "^a"_u; // Circumflex
	map["ê"_u] = "^e"_u;
	map["î"_u] = "^\\i"_u;
	map["ô"_u] = "^o"_u;
	map["û"_u] = "^u"_u;
	map["ŷ"_u] = "^y"_u;
	map["Â"_u] = "^A"_u;
	map["Ê"_u] = "^E"_u;
	map["Î"_u] = "^I"_u;
	map["Ô"_u] = "^O"_u;
	map["Û"_u] = "^U"_u;
	map["Ŷ"_u] = "^Y"_u;
	map["ä"_u] = "\"a"_u;    // Umlaut or dieresis
	map["ë"_u] = "\"e"_u;
	map["ï"_u] = "\"\\i"_u;
	map["ö"_u] = "\"o"_u;
	map["ü"_u] = "\"u"_u;
	map["ÿ"_u] = "\"y"_u;
	map["Ä"_u] = "\"A"_u;
	map["Ë"_u] = "\"E"_u;
	map["Ï"_u] = "\"I"_u;
	map["Ö"_u] = "\"O"_u;
	map["Ü"_u] = "\"U"_u;
	map["Ÿ"_u] = "\"Y"_u;

	map["ñ"_u] = "~n"_u;
	map["Ñ"_u] = "~N"_u;

	map["ç"_u] = "cc"_u;   // Cedilla
	map["Ç"_u] = "cC"_u;


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
	map["à"_u] = "\\`a"_u; # Grave accent
	map["è"_u] = "\\`e"_u;
	map["ì"_u] = "\\`\\i"_u;
	map["ò"_u] = "\\`o"_u;
	map["ù"_u] = "\\`u"_u;
	map["ỳ"_u] = "\\`y"_u;
	map["À"_u] = "\\`A"_u;
	map["È"_u] = "\\`E"_u;
	map["Ì"_u] = "\\`\\I"_u;
	map["Ò"_u] = "\\`O"_u;
	map["Ù"_u] = "\\`U"_u;
	map["Ỳ"_u] = "\\`Y"_u;
	map["á"_u] = "\\'a"_u; # Acute accent
	map["é"_u] = "\\'e"_u;
	map["í"_u] = "\\'\\i"_u;
	map["ó"_u] = "\\'o"_u;
	map["ú"_u] = "\\'u"_u;
	map["ý"_u] = "\\'y"_u;
	map["Á"_u] = "\\'A"_u;
	map["É"_u] = "\\'E"_u;
	map["Í"_u] = "\\'\\I"_u;
	map["Ó"_u] = "\\'O"_u;
	map["Ú"_u] = "\\'U"_u;
	map["Ý"_u] = "\\'Y"_u;
	map["â"_u] = "\\^a"_u; # Circumflex
	map["ê"_u] = "\\^e"_u;
	map["î"_u] = "\\^\\i"_u;
	map["ô"_u] = "\\^o"_u;
	map["û"_u] = "\\^u"_u;
	map["ŷ"_u] = "\\^y"_u;
	map["Â"_u] = "\\^A"_u;
	map["Ê"_u] = "\\^E"_u;
	map["Î"_u] = "\\^\\I"_u;
	map["Ô"_u] = "\\^O"_u;
	map["Û"_u] = "\\^U"_u;
	map["Ŷ"_u] = "\\^Y"_u;
	map["ä"_u] = "\\\"a"_u;    # Umlaut or dieresis
	map["ë"_u] = "\\\"e"_u;
	map["ï"_u] = "\\\"\\i"_u;
	map["ö"_u] = "\\\"o"_u;
	map["ü"_u] = "\\\"u"_u;
	map["ÿ"_u] = "\\\"y"_u;
	map["Ä"_u] = "\\\"A"_u;
	map["Ë"_u] = "\\\"E"_u;
	map["Ï"_u] = "\\\"\\I"_u;
	map["Ö"_u] = "\\\"O"_u;
	map["Ü"_u] = "\\\"U"_u;
	map["Ÿ"_u] = "\\\"Y"_u;
	map["ç"_u] = "\\c{c}"_u;   # Cedilla
	map["Ç"_u] = "\\c{C}"_u;
	map["œ"_u] = "{\\oe}"_u;   # Ligatures
	map["Œ"_u] = "{\\OE}"_u;
	map["æ"_u] = "{\\ae}"_u;
	map["Æ"_u] = "{\\AE}"_u;
	map["å"_u] = "{\\aa}"_u;
	map["Å"_u] = "{\\AA}"_u;
	map["–"_u] = "--"_u;   # Dashes
	map["—"_u] = "---"_u;
	map["ø"_u] = "{\\o}"_u;    # Misc latin-1 letters
	map["Ø"_u] = "{\\O}"_u;
	map["ß"_u] = "{\\ss}"_u;
	map["¡"_u] = "{!`}"_u;
	map["¿"_u] = "{?`}"_u;
	map["\\"_u] = "\\\\"_u;    # Characters that should be quoted
	map["~"_u] = "\\~"_u;
	map["&"_u] = "\\&"_u;
	map["$"_u] = "\\$"_u;
	map["{"_u] = "\\{"_u;
	map["}"_u] = "\\}"_u;
	map["%"_u] = "\\%"_u;
	map["#"_u] = "\\#"_u;
	map["_"_u] = "\\_"_u;
	map["≥"_u] = "$\\ge$"_u;   # Math operators
	map["≤"_u] = "$\\le$"_u;
	map["≠"_u] = "$\\neq$"_u;
	map["©"_u] = "\copyright"_u; # Misc
	map["ı"_u] = "{\\i}"_u;
	map["µ"_u] = "$\\mu$"_u;
	map["°"_u] = "$\\deg$"_u;
	map["‘"_u] = "`"_u;    #Quotes
	map["’"_u] = "'"_u;
	map["“"_u] = "``"_u;
	map["”"_u] = "''"_u;
	map["‚"_u] = ","_u;
	map["„"_u] = ",,"_u;
]*/


void fputus(const UString& s, FILE* out)
{
  string temp;
  temp.reserve(s.size()*2);
  utf8::utf16to8(s.begin(), s.end(), std::back_inserter(temp));
  fputs(temp.c_str(), out);
}
