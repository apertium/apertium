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
* along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <regex>
#include <cctype>
#include "apertium_config.h"

void trim(std::string& str) {
	while (!str.empty() && isspace(str.back())) {
		str.pop_back();
	}
	size_t h = 0;
	for (; h < str.size() && isspace(str[h]); ++h) {
	}
	str.erase(0, h);
}

int main(int argc, char* argv[]) {
	bool add_z = false;
  bool add_normalize = true;

  int argn = 1;
  while (argc > argn && argv[argn][0] == '-') {
    if (argv[argn][1] == 'z' && argv[argn][2] == 0) {
      add_z = true;
      argn++;
    } else if (argv[argn][1] == 'n' && argv[argn][2] == 0) {
      add_normalize = false;
      argn++;
    }
  }

	// Input can come either as a passed file or from stdin
	std::unique_ptr<std::istream> _in;
	std::istream *in = &std::cin;
	if (argc > argn) {
		_in.reset(new std::ifstream(argv[argn], std::ios::binary));
		in = _in.get();
	}

	std::string mode;
	std::string tmp;
	while (std::getline(*in, tmp)) {
		mode += tmp;
	}
	trim(mode);

	// Don't change the mode twice
	bool has_helpers = (mode.find("apertium-wblank-") != std::string::npos);

  // apertium-restore-caps detaches wblanks
  bool has_restore_caps = (mode.find("apertium-restore-caps") != std::string::npos);

  bool has_normalize = ((mode.find("apertium-normalize") != std::string::npos) || (mode.find("uconv") != std::string::npos));

	// Convert old-style transfer to new-style
	if (mode.find("lt-proc -b") == std::string::npos) {
		mode = std::regex_replace(mode, std::regex(R"X(apertium-transfer\s+'([^']+)'\s+'([^']+)'\s+'([^']+autobil\.bin)')X"), "lt-proc -b '$3' | apertium-transfer -b '$1' '$2'");
	}

	std::string new_mode;
	size_t b = 0;
	size_t e = 0;
  if (add_normalize && !has_normalize) {
    new_mode += "apertium-normalize ";
    new_mode += DATADIR;
    new_mode += "/nfcn.nrm | ";
  }
	do {
		b = mode.find('|', e);
		auto l = mode.begin() + b;
		if (b == std::string::npos) {
			l = mode.end();
		}
		tmp.assign(mode.begin() + e, l);
		trim(tmp);

    if (!add_normalize && ((tmp.find("apertium-normalize") != std::string::npos) || (tmp.find("uconv") != std::string::npos))) {
      // drop existing normalization
      e = b + 1;
      continue;
    }

		if (add_z && tmp.find(" -z") == std::string::npos) {
			auto s = tmp.find_first_of(" \t\r\n");
			if (s != std::string::npos) {
				tmp.insert(s, " -z");
			}
			else {
				tmp += " -z";
			}
		}

		if (!has_helpers) {
			if (tmp.find("automorf.bin") != std::string::npos || tmp.find("automorf.hfst") != std::string::npos) {
				tmp += " | apertium-wblank-attach";
			}
			else if (!has_restore_caps && (tmp.find("autogen.bin") != std::string::npos || tmp.find("autogen.hfst") != std::string::npos)) {
				tmp.insert(0, "apertium-wblank-detach | ");
			}
		}

		new_mode += tmp;
		if (b != std::string::npos) {
			new_mode += " | ";
		}
		e = b + 1;
	} while (b != std::string::npos);

	std::cout << new_mode << std::endl;
}
