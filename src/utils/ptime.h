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

#ifndef _PTIME_H_
#define _PTIME_H_

#include <sys/time.h>

typedef struct timeval Time;

/* Endort le processus courant pendant n millisecondes. */
void sleep_m (long nb_millisec);

/* Récupère le temps écoulé depuis le 1er janvier 1970. */
void time_get_cur (Time *time);

/* Retourne la différence entre 2 temps en millisecondes. */
long time_diff (Time *time1, Time *time2);

#endif /* _PTIME_H_ INCLUDED */
