/*
* Copyright (C) 2021 Apertium
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

#pragma once
#ifndef APERTIUM_FILESYSTEM_HPP_
#define APERTIUM_FILESYSTEM_HPP_

#include "apertium_config.h"
#include "string_view.h"

#ifdef HAVE_FILESYSTEM
	#include <filesystem>
#else
	#include <experimental/filesystem>

	namespace std {
		namespace filesystem {
			using namespace ::std::experimental::filesystem;
		}
	}
#endif

inline std::filesystem::path path(std::string_view sv) {
	std::filesystem::path rv(sv.begin(), sv.end());
	return rv;
}

namespace fs = ::std::filesystem;

#endif
