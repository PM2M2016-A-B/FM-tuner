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

#include "pmath.h"

char *bytes_to_binary_text(int x, int n, char *buf) {
  int i = 0, j;

  for (j = 0x80 << 8 * (n - 1); j > 0; j >>= 1)
    buf[i++] = ((x & j) == j) ? '1' : '0';

  buf[i] = '\0';

  return buf;
}
