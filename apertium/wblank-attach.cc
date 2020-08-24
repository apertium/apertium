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
#include <vector>
#include <algorithm>
#include <array>

void trim_wb(std::string& wb) {
	while (!wb.empty() && (wb.back() == ';' || wb.back() == ' ')) {
		wb.pop_back();
	}
	size_t h = 0;
	for (; h < wb.size() && (wb[h] == ';' || wb[h] == ' '); ++h) {
	}
	wb.erase(0, h);
}

int main(int argc, char* argv[]) {
	// Ignore -z, but anything else just show what this tool does
	if (argc > 1 && argv[1][1] != 'z') {
		std::cout << "Distributes word-bound blanks across all tokens they encompass, turning [[A]]^...$^...$[[/]] into [[A]]^...$[[A]]^...$\n";
		std::cout << "Also merges word-bound blanks, turning [[A]][[B]]^...$^...$[[/]][[/]] into [[A; B]]^...$\n";
		std::cout << "Word-bound blanks will be deduplicated, but order will be preserved amongst unique elements.\n";
		return 0;
	}

	std::ios::sync_with_stdio(false);
	std::cin.tie(nullptr);

	std::array<char, 4096> inbuf{};
	std::cin.rdbuf()->pubsetbuf(inbuf.data(), inbuf.size());

	std::vector<std::string> wbs;
	std::string blank;
	std::string unesc;

	bool in_token = false;
	bool in_blank = false;
	bool in_wblank = false;

	char c = 0;
	size_t line = 1;
	while (std::cin.get(c)) {
		if (c == '\n') {
			++line;
		}

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
			in_token = in_blank = false;
			if (!wbs.empty()) {
				std::cerr << "tf-apertium-spread warning: Null-flush found, but had open word-bound blanks on line " << line << ":";
				for (auto& wb : wbs) {
					std::cerr << ' ' << wb;
				}
				std::cerr << std::endl;
				wbs.clear();
			}
			if (!blank.empty()) {
				std::cerr << "tf-apertium-spread warning: Null-flush found, but had an open blank on line " << line << std::endl;
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
			if (!wbs.empty()) {
				std::cout << "[[";
				for (size_t i = 0; i < wbs.size(); ++i) {
					std::cout << wbs[i];
					if (i < wbs.size() - 1) {
						std::cout << "; ";
					}
				}
				std::cout << "]]";
			}
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
			if (unesc[0] == '[' && unesc[1] == '[' && unesc[2] == '/' && unesc[3] == ']' && unesc[4] == ']') {
				if (wbs.empty()) {
					std::cerr << "tf-apertium-spread warning: Too many [[/]] on line " << line << std::endl;
				}
				else {
					wbs.pop_back();
				}
			}
			else if (unesc[0] == '[' && unesc[1] == '[') {
				blank.assign(unesc.begin() + 2, unesc.end() - 2);
				size_t b = 0;
				while (b < blank.size()) {
					size_t e = blank.find(';', b);
					unesc.assign(blank, b, e - b);
					trim_wb(unesc);
					// Deduplicate
					if (!unesc.empty() && std::find(wbs.begin(), wbs.end(), unesc) == wbs.end()) {
						wbs.push_back(unesc);
					}
					b = std::max(e, e + 1);
				}
			}
			else {
				// A normal blank, so just output it and move on
				std::cout << blank;
			}
			blank.clear();
			unesc.clear();
		}
	}

	if (!wbs.empty()) {
		std::cerr << "tf-apertium-spread warning: End of input reached, but had open word-bound blanks:";
		for (auto& wb : wbs) {
			std::cerr << ' ' << wb;
		}
		std::cerr << std::endl;
	}
	if (!blank.empty()) {
		std::cerr << "tf-apertium-spread warning: End of input reached, but had an open blank." << std::endl;
		std::cout << blank;
	}
}
