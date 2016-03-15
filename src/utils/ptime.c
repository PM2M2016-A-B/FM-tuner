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

#include <stdlib.h>
#include <time.h>

#include "time.h"

void sleep_m (unsigned int nb_millisec) {
  struct timespec req;

  req.tv_sec = nb_millisec / 1000;
  req.tv_nsec = (long)(nb_millisec % 1000) * 1000000;

  nanosleep(&req, NULL);

  return;
}
