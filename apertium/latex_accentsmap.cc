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

	map[L"à"] = L"`a"; // Grave accent
	map[L"è"] = L"`e";
	map[L"ì"] = L"`\\i";
	map[L"ò"] = L"`o";
	map[L"ù"] = L"`u";
	map[L"ỳ"] = L"`y";
	map[L"À"] = L"`A";
	map[L"È"] = L"`E";
	map[L"Ì"] = L"`I";
	map[L"Ò"] = L"`O";
	map[L"Ù"] = L"`U";
	map[L"Ỳ"] = L"`Y";
	map[L"á"] = L"'a"; // Acute accent
	map[L"é"] = L"'e";
	map[L"í"] = L"'\\i";
	map[L"ó"] = L"'o";
	map[L"ú"] = L"'u";
	map[L"ý"] = L"'y";
	map[L"Á"] = L"'A";
	map[L"É"] = L"'E";
	map[L"Í"] = L"'I";
	map[L"Ó"] = L"'O";
	map[L"Ú"] = L"'U";
	map[L"Ý"] = L"'Y";
	map[L"â"] = L"^a"; // Circumflex
	map[L"ê"] = L"^e";
	map[L"î"] = L"^\\i";
	map[L"ô"] = L"^o";
	map[L"û"] = L"^u";
	map[L"ŷ"] = L"^y";
	map[L"Â"] = L"^A";
	map[L"Ê"] = L"^E";
	map[L"Î"] = L"^I";
	map[L"Ô"] = L"^O";
	map[L"Û"] = L"^U";
	map[L"Ŷ"] = L"^Y";
	map[L"ä"] = L"\"a";    // Umlaut or dieresis
	map[L"ë"] = L"\"e";
	map[L"ï"] = L"\"\\i";
	map[L"ö"] = L"\"o";
	map[L"ü"] = L"\"u";
	map[L"ÿ"] = L"\"y";
	map[L"Ä"] = L"\"A";
	map[L"Ë"] = L"\"E";
	map[L"Ï"] = L"\"I";
	map[L"Ö"] = L"\"O";
	map[L"Ü"] = L"\"U";
	map[L"Ÿ"] = L"\"Y";

	map[L"ñ"] = L"~n";
	map[L"Ñ"] = L"~N";
  
	map[L"ç"] = L"cc";   // Cedilla
	map[L"Ç"] = L"cC";


}

wstring AccentsMap::get(wstring input){
	it = map.find(input);
	if(it == map.end())
		return L"";
	else
		return (*it).second;
}

//Optionally:
void AccentsMap::init_locale(){ 
	char *locale = setlocale(LC_ALL, "");
	std::locale lollocale(locale);
	wcout.imbue(lollocale);
}



/*latexAccents = [
	map[L"à"] = L"\\`a"; # Grave accent
	map[L"è"] = L"\\`e";
	map[L"ì"] = L"\\`\\i";
	map[L"ò"] = L"\\`o";
	map[L"ù"] = L"\\`u";
	map[L"ỳ"] = L"\\`y";
	map[L"À"] = L"\\`A";
	map[L"È"] = L"\\`E";
	map[L"Ì"] = L"\\`\\I";
	map[L"Ò"] = L"\\`O";
	map[L"Ù"] = L"\\`U";
	map[L"Ỳ"] = L"\\`Y";
	map[L"á"] = L"\\'a"; # Acute accent
	map[L"é"] = L"\\'e";
	map[L"í"] = L"\\'\\i";
	map[L"ó"] = L"\\'o";
	map[L"ú"] = L"\\'u";
	map[L"ý"] = L"\\'y";
	map[L"Á"] = L"\\'A";
	map[L"É"] = L"\\'E";
	map[L"Í"] = L"\\'\\I";
	map[L"Ó"] = L"\\'O";
	map[L"Ú"] = L"\\'U";
	map[L"Ý"] = L"\\'Y";
	map[L"â"] = L"\\^a"; # Circumflex
	map[L"ê"] = L"\\^e";
	map[L"î"] = L"\\^\\i";
	map[L"ô"] = L"\\^o";
	map[L"û"] = L"\\^u";
	map[L"ŷ"] = L"\\^y";
	map[L"Â"] = L"\\^A";
	map[L"Ê"] = L"\\^E";
	map[L"Î"] = L"\\^\\I";
	map[L"Ô"] = L"\\^O";
	map[L"Û"] = L"\\^U";
	map[L"Ŷ"] = L"\\^Y";
	map[L"ä"] = L"\\\"a";    # Umlaut or dieresis
	map[L"ë"] = L"\\\"e";
	map[L"ï"] = L"\\\"\\i";
	map[L"ö"] = L"\\\"o";
	map[L"ü"] = L"\\\"u";
	map[L"ÿ"] = L"\\\"y";
	map[L"Ä"] = L"\\\"A";
	map[L"Ë"] = L"\\\"E";
	map[L"Ï"] = L"\\\"\\I";
	map[L"Ö"] = L"\\\"O";
	map[L"Ü"] = L"\\\"U";
	map[L"Ÿ"] = L"\\\"Y";
	map[L"ç"] = L"\\c{c}";   # Cedilla
	map[L"Ç"] = L"\\c{C}";
	map[L"œ"] = L"{\\oe}";   # Ligatures
	map[L"Œ"] = L"{\\OE}";
	map[L"æ"] = L"{\\ae}";
	map[L"Æ"] = L"{\\AE}";
	map[L"å"] = L"{\\aa}";
	map[L"Å"] = L"{\\AA}";
	map[L"–"] = L"--";   # Dashes
	map[L"—"] = L"---";
	map[L"ø"] = L"{\\o}";    # Misc latin-1 letters
	map[L"Ø"] = L"{\\O}";
	map[L"ß"] = L"{\\ss}";
	map[L"¡"] = L"{!`}";
	map[L"¿"] = L"{?`}";
	map[L"\\"] = L"\\\\";    # Characters that should be quoted
	map[L"~"] = L"\\~";
	map[L"&"] = L"\\&";
	map[L"$"] = L"\\$";
	map[L"{"] = L"\\{";
	map[L"}"] = L"\\}";
	map[L"%"] = L"\\%";
	map[L"#"] = L"\\#";
	map[L"_"] = L"\\_";
	map[L"≥"] = L"$\\ge$";   # Math operators
	map[L"≤"] = L"$\\le$";
	map[L"≠"] = L"$\\neq$";
	map[L"©"] = L"\copyright"; # Misc
	map[L"ı"] = L"{\\i}";
	map[L"µ"] = L"$\\mu$";
	map[L"°"] = L"$\\deg$";
	map[L"‘"] = L"`";    #Quotes
	map[L"’"] = L"'";
	map[L"“"] = L"``";
	map[L"”"] = L"''";
	map[L"‚"] = L",";
	map[L"„"] = L",,";
]*/






