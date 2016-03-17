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

#ifndef _HANDLER_H_
#define _HANDLER_H_

#include "../fm_tuner.h"
#include "../rds.h"
#include "../utils/socket.h"

typedef struct Handler_value {
  Fm_tuner *fm_tuner;
  Rds *rds;
} Handler_value;

int handler_event (Socket sock, int id, char *buffer, int len, void *user_value);
void *handler_join (Socket sock, int id, void *user_value);
void handler_quit (Socket sock, int id, void *user_value);
void handler_loop (Socket_set *ss, void *user_value);

#endif /* _HANDLER_H_ INCLUDED */
