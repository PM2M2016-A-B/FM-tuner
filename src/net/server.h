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

#ifndef _SERVER_H_
#define _SERVER_H_

#include "../utils/socket.h"

typedef int (*Fun_client_event)(Socket sock, int id, char *buffer, int len, void *user_value);
typedef void *(*Fun_client_join)(Socket sock, int id, void *user_value);
typedef void (*Fun_client_quit)(Socket sock, int id, void *user_value);
typedef void (*Fun_server_loop)(Socket_set *ss, void *user_value);

typedef struct Server_handlers {
  Fun_client_event event;
  Fun_client_join join;
  Fun_client_quit quit;
  Fun_server_loop loop;
} Server_handlers;

typedef struct Server_conf {
  in_port_t port;
  unsigned int max_clients;
  Server_handlers handlers;
  void *user_value;
} Server_conf;

void server_run (Server_conf *conf, int timeout);

#endif /* _SERVER_H_ INCLUDED */
