/*
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _PMATH_H_
#define _PMATH_H_

/* Convertit les n bytes de poids faible de l'entier x
   au format binaire, et met le résultat dans buf.
   Retourne buf. */
char *bytes_to_binary_text(int x, int n, char *buf);

#endif /* _PMATH_H_ INCLUDED */
