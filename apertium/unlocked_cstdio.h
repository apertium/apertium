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

#ifndef _APERTIUM_UNLOCKED_CSTDIO_
#define _APERTIUM_UNLOCKED_CSTDIO_

#include <cstdio>

#if !HAVE_DECL_FPUTS_UNLOCKED
#define fputs_unlocked fputs
#endif

#if !HAVE_DECL_FGETC_UNLOCKED
#define fgetc_unlocked fgetc
#endif

#if !HAVE_DECL_FPUTC_UNLOCKED
#define fputc_unlocked fputc
#endif

#if !HAVE_DECL_FWRITE_UNLOCKED
#define fwrite_unlocked fwrite
#endif

#if !HAVE_DECL_FREAD_UNLOCKED
#define fread_unlocked fread
#endif

#endif
