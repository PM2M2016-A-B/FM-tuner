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

#include <pthread.h>
#include <signal.h>
#include <string.h>

#include "../utils/alloc.h"
#include "../utils/error.h"
#include "server.h"

#define CLIENT_BUFFER_SIZE 128

typedef struct Client {
  char buf[CLIENT_BUFFER_SIZE];
  int pos;
} Client;

typedef struct Server {
  Socket sock;
  Socket_set *ss;
  Client **clients;
  pthread_mutex_t lock_run;
  char run;
} Server;

/* ---------------------------------------------------------------------- */

static void __sig_handler (int sig) {
  (void)sig;
  return;
}

static void *__thread_exit (void *arg) {
  struct sigaction action;
  sigset_t set;
  Server *server = arg;

  action.sa_handler = __sig_handler;
  sigemptyset(&(action.sa_mask));
  action.sa_flags = 0;

  if (sigaction(SIGINT, &action, NULL) != 0)
    fatal_error("Sigaction failed.");

  sigfillset(&set);
  sigdelset(&set, SIGINT);

  /* Attend le signal de fermeture. */
  sigsuspend(&set);

  printf("[server]Stopping server...\n");

  pthread_mutex_lock(&server->lock_run);
  server->run = 0;
  pthread_mutex_unlock(&server->lock_run);

  return NULL;
}

/* ---------------------------------------------------------------------- */

static void __server_init (Server *server, Server_conf *conf) {
  unsigned int i;
  IP ip;

  pmalloc(server->clients, conf->max_clients * sizeof *server->clients);

  for (i = 0; i < conf->max_clients; i++)
    pmalloc(server->clients[i], sizeof **server->clients);

  server->run = 1;

  if (resolve_host(&ip, NULL, conf->port) == -1)
    fatal_error("Unable to make server on port: %d.", conf->port);

  if ((server->sock = tcp_get(&ip)) == -1)
    fatal_error("Unable to get ip.");

  /* socket_set contient sockets clients + socket server. */
  if ((server->ss = socket_set_new(conf->max_clients + 1)) == NULL)
    fatal_error("Unable to make socket_set.");

  socket_set_add(server->ss, server->sock);
  pthread_mutex_init(&server->lock_run, NULL);

  return;
}

static void __server_close (Server *server, Server_conf *conf) {
  unsigned int i;

  socket_set_free(server->ss);

  for (i = 0; i < conf->max_clients; i++)
    free(server->clients[i]);

  free(server->clients);
  pthread_mutex_destroy(&server->lock_run);

  return;
}

/* --------------------------------------------------------------------- */

static void __handle_server (Server *server, Server_conf *conf) {
  Socket sock = tcp_accept(server->sock);
  int id;

  /* Mauvaise connexion. */
  if (sock == -1)
    return;

  /* On déconnecte le nouveau client s'il y a trop de monde. */
  if ((id = socket_set_add(server->ss, sock)) == -1) {
    printf("[server]No enough place for a new client.\n");
    tcp_close(sock);

    return;
  }

  printf("[server]New client %d!\n", id);
  memset(server->clients[id - 1], 0, sizeof(Client));
  conf->handlers.join(sock, id, conf->user_value);

  return;
}

static void __handle_client(Server *server, Server_conf *conf, Socket sock, int id) {
  Client *client = server->clients[id - 1];
  unsigned int len;
  int ret;

  /* Normalement si le protocole de gestion des clients est bien fait,
     ce cas ne devrait jamais arriver. */
  if (client->pos == CLIENT_BUFFER_SIZE)
    printf("[server]Warning: buffer is full for client %d!\n", id);

  /* Déconnexion d'un client. */
  if ((len = tcp_recv(sock, client->buf + client->pos, CLIENT_BUFFER_SIZE - client->pos)) <= 0) {
    socket_set_remove(server->ss, sock);
    printf("[server]Bye client %d!\n", id);
  }

  /* Réception d'un message. */
  else {
    printf("[server]New message for client %d!\n", id);
    client->pos += len;

    /* Déplacement du pointeur de lecture. */
    ret = conf->handlers.event(sock, id, client->buf, client->pos, conf->user_value);

    if (ret > client->pos)
      ret = client->pos;
    else if (ret < 0)
      ret = 0;

    memmove(client->buf, client->buf + ret, client->pos - ret);
    client->pos -= ret;
  }

  return;
}

/* --------------------------------------------------------------------- */

void server_run (Server_conf *conf, int timeout) {
  Server server;
  Sockets_states states;
  Socket sock;
  pthread_t thread_exit;
  unsigned int i;

  __server_init(&server, conf);

  if (pthread_create(&thread_exit, NULL, __thread_exit, &server) != 0)
    fatal_error("Unable to create thread_exit.");

  for (;;) {
    pthread_mutex_lock(&server.lock_run);

    if (!server.run)
      break;

    pthread_mutex_unlock(&server.lock_run);

    if (socket_set_select(server.ss, &states, NULL, timeout) == -1)
      error("Select error.");

    /* Socket serveur. */
    if (socket_is_ready(server.sock, &states))
      __handle_server(&server, conf);

    /* Sockets clients. */
    for (i = 1; i <= conf->max_clients; i++) {
      sock = socket_set_get(server.ss, i);

      if (socket_is_ready(sock, &states))
        __handle_client(&server, conf, sock, i);
    }

    conf->handlers.loop(server.ss, conf->user_value);
  }

  pthread_mutex_unlock(&server.lock_run);
  pthread_join(thread_exit, NULL);

  __server_close(&server, conf);

  return;
}
