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

#include <time.h>

#include "ptime.h"

void sleep_m (long nb_millisec) {
  struct timespec req;

  if (nb_millisec < 0)
    return;

  req.tv_sec = nb_millisec / 1000;
  req.tv_nsec = (long)(nb_millisec % 1000) * 1000000;

  nanosleep(&req, NULL);

  return;
}

void time_get_cur (Time *time) {
  gettimeofday(time, NULL);
  return;
}

long time_diff (Time *time1, Time *time2) {
  return (time2->tv_usec  - time1->tv_usec) / 1000.0 + (time2->tv_sec - time1->tv_sec) * 1000.0;
}
