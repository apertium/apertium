// Copyright (C) 2005 Universitat d'Alacant / Universidad de Alicante
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.

#ifndef CONSTRUCTOR_EQ_DELETE_H
#define CONSTRUCTOR_EQ_DELETE_H

namespace Apertium {
class constructor_eq_delete {
protected:
  constructor_eq_delete() {}
  ~constructor_eq_delete() {}

private:
  constructor_eq_delete(const constructor_eq_delete &constructor_eq_delete_);
  constructor_eq_delete &
  operator=(constructor_eq_delete constructor_eq_delete_);
};
}

#endif // CONSTRUCTOR_EQ_DELETE_H
