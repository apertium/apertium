/*
* Copyright (C) 2020 Apertium (https://apertium.org/)
* Developed by Tino Didriksen <mail@tinodidriksen.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <string>
#include <array>

int main(int argc, char* argv[]) {
	// Ignore -z, but anything else just show what this tool does
	if (argc > 1 && argv[1][1] != 'z') {
		std::cout << "Closes all word-bound blanks, turning [[...]]^...$ into [[...]]^...$[[/]]\n";
		std::cout << "This tool does not merge across whitespace or do any other heuristics wrt. which word-bound blanks should have their spans combined.\n";
		return 0;
	}

	std::ios::sync_with_stdio(false);
	std::cin.tie(nullptr);

	std::array<char, 4096> inbuf{};
	std::cin.rdbuf()->pubsetbuf(inbuf.data(), inbuf.size());

	std::string blank;
	std::string unesc;

	bool in_token = false;
	bool in_blank = false;
	bool in_wblank = false;
	bool had_wblank = false;

	char c = 0;
	while (std::cin.get(c)) {
		if (c == '\\' && std::cin.peek()) {
			auto n = static_cast<char>(std::cin.get());
			if (in_blank) {
				blank += c;
				blank += n;
				unesc += n;
			}
			else {
				std::cout.put(c);
				std::cout.put(n);
			}
			continue;
		}

		if (c == '\0') {
			in_token = in_blank = in_wblank = had_wblank = false;
			if (!blank.empty()) {
				std::cout << blank;
				blank.clear();
				unesc.clear();
			}
			std::cout.put(c);
			std::cout.flush();
			continue;
		}

		if (!in_token && c == '[') {
			if (in_blank) {
				in_wblank = true;
			}
			in_blank = true;
		}
		else if (in_wblank && c == ']') {
			// Do nothing
		}
		else if (in_blank && c == ']') {
			// Do nothing
		}
		else if (!in_blank && c == '^') {
			in_token = true;
		}
		else if (!in_blank && c == '$') {
			in_token = false;
		}

		if (in_blank) {
			blank += c;
			unesc += c;
		}
		else {
			std::cout.put(c);
		}

		if (in_wblank && c == ']') {
			in_wblank = false;
		}
		else if (in_blank && c == ']') {
			in_blank = false;
			if (unesc[0] == '[' && unesc[1] == '[') {
				had_wblank = true;
			}
			std::cout << blank;
			blank.clear();
			unesc.clear();
		}
		else if (had_wblank && !in_blank && c == '$') {
			std::cout << "[[/]]";
			had_wblank = false;
		}
	}
}
