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

#include <errno.h>
#include <stdlib.h>
#include <string.h> /* memcpy */

#include "socket.h"

#if defined __unix__
  #include <fcntl.h> /* fcntl & O_NONBLOCK */
  #include <netdb.h> /* gethostbyname */
  #include <unistd.h> /* close */
#endif

#ifndef h_addr
  /* Pour compatibilité... */
  #define h_addr h_addr_list[0]
#endif

struct Socket_set {
  Socket *socks;
  int n;
  int max_n;
  int max_fd;
};

/* --------------------------------------------------------------------- */

char *serialize_uint8 (char *buf, uint8_t value) {
  buf[0] = value;
  return buf + 1;
}

char *serialize_uint16 (char *buf, uint16_t value) {
  buf[0] = (value >> 8) & 0xFF;
  buf[1] = value & 0xFF;

  return buf + 2;
}

char *serialize_uint32 (char *buf, uint32_t value) {
  buf[0] = (value >> 24) & 0xFF;
  buf[1] = (value >> 16) & 0xFF;
  buf[2] = (value >> 8) & 0xFF;
  buf[3] = value & 0xFF;

  return buf + 4;
}

char *deserialize_uint8 (char *buf, uint8_t *value) {
  *value = buf[0];
  return buf + 1;
}

char *deserialize_uint16 (char *buf, uint16_t *value) {
  *value = (buf[0] << 8 | buf[1]);
  return buf + 2;
}

char *deserialize_uint32 (char *buf, uint32_t *value) {
  *value = (buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3]);
  return buf + 4;
}

/* --------------------------------------------------------------------- */

int resolve_host (IP *ip, const char *hostname, in_port_t port) {
  struct hostent *h;

  /* Si host non spécifié, on utilise n'importe quelle adresse valable de la machine.
     Il s'agit donc d'une éventuelle création de serveur et non de client. */
  if (hostname == NULL)
    ip->host = INADDR_ANY;
  /* Sinon, récupération de l'ip à utiliser pour le client. */
  else if ((ip->host = inet_addr(hostname)) == INADDR_NONE) {
    if ((h = gethostbyname(hostname)) == NULL)
      return -1; /* Echec de la récupération des informations sur l'host. */

    memcpy(&ip->host, h->h_addr, h->h_length);
  }

  /* Port. */
  ip->port = htons(port);

  return 0;
}

/* --------------------------------------------------------------------- */

Socket_set *socket_set_new (int n) {
  Socket_set *ss = malloc(sizeof *ss);
  int i;

  if (ss == NULL)
    return NULL;

  if ((ss->socks = malloc(n * sizeof(Socket))) == NULL) {
    free(ss);
    return NULL;
  }

  ss->max_n = n;
  ss->n = 0;
  ss->max_fd = 0;

  for (i = 0; i < ss->max_n; i++)
    ss->socks[i] = -1;

  return ss;
}

void socket_set_free (Socket_set *ss) {
  int i;

  if (ss == NULL)
    return;

  for (i = 0; i < ss->max_n; i++)
    if (ss->socks[i] != -1)
      close(ss->socks[i]);

  free(ss->socks);
  free(ss);

  return;
}

int socket_set_add (Socket_set *ss, Socket sock) {
  int i;

  if (sock < 0 || ss->n == ss->max_n)
    return -1;

  for (i = 0; i < ss->max_n && ss->socks[i] != -1; i++);

  ss->socks[i] = sock;
  ss->n++;

  if (sock > ss->max_fd)
    ss->max_fd = sock;

  return i;
}

int socket_set_remove (Socket_set *ss, Socket sock) {
  int i;

  if (sock < 0)
    return -1;

  for (i = 0; i < ss->max_n && ss->socks[i] != sock; i++);

  /* Non trouvé. */
  if (i == ss->max_n)
    return -1;

  /* Trouvé ! */
  ss->socks[i] = -1;
  ss->n--;

  /* On met à jour le max_fd. */
  if (sock == ss->max_fd) {
    ss->max_fd = 0;

    for (i = 0; i < ss->max_n; i++)
      if (ss->socks[i] > ss->max_fd)
        ss->max_fd = ss->socks[i];
  }

  return i;
}

Socket socket_set_get (Socket_set *ss, int id) {
  return (id >= 0 && id < ss->max_n) ? ss->socks[id] : -1;
}

int socket_set_select (Socket_set *ss, Sockets_states *states, int *stdin, int timeout) {
  int i, ret;
  struct timeval tv;

  /* Mise à 0 de l'ensemble. */
  FD_ZERO(&*states);

  /* Ajout de stdin si demandé. */
  if (stdin != NULL) {
    FD_SET(STDIN_FILENO, &*states);
    *stdin = 0;
  }

  /* Ajout des sockets dans l'ensemble. */
  for (i = 0; i < ss->max_n; i++)
    if (ss->socks[i] != -1)
      FD_SET(ss->socks[i], &*states);

  /* Timeout. */
  tv.tv_sec = 0;
  tv.tv_usec = timeout;

  /* Select. */
  do {
    ret = select(ss->max_fd + 1, &*states, NULL, NULL, (timeout > 0) ? &tv : NULL);
  } while (errno == EINTR);

  if (stdin != NULL && FD_ISSET(STDIN_FILENO, &*states))
    *stdin = 1;

  return ret;
}

int socket_is_ready (Socket socket, Sockets_states *states) {
  return FD_ISSET(socket, &*states);
}

int socket_set_get_size (Socket_set *ss) {
  return (ss == NULL) ? -1 : ss->n;
}

int socket_set_get_max_size (Socket_set *ss) {
  return (ss == NULL) ? -1 : ss->max_n;
}

/* --------------------------------------------------------------------- */

Socket tcp_get (IP *ip) {
  struct sockaddr_in addr;
  Socket sock;
  int opt = 1; /* Utilisé pour bloquer le EADDRINUSE et mettre en place TCP_NODELAY */

  /* Création d'un socket fondé sur le protocole IP, en mode connecté. */
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    return -1;

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = ip->port;

  /* Demande client. */
  if (ip->host != INADDR_ANY && ip->host != INADDR_NONE) {
    /* Mise en place de l'host à joindre. */
    addr.sin_addr.s_addr = ip->host;

    /* Connexion... */
    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
      close(sock);
      return -1;
    }
  }
  /* Demande serveur. */
  else {
    /* Adresse de la machine courante. */
    addr.sin_addr.s_addr = INADDR_ANY;

    /* Evite un eventuel bind: Address already in use */
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    /* Affectation de l'adresse. */
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0 ||
       listen(sock, 5) < 0) {
      close(sock);
      return -1;
    }

    /* Mode non bloquant pour accept. */
    #ifdef O_NONBLOCK
      fcntl(sock, F_SETFL, O_NONBLOCK);
    #else
      #error "Unable to set O_NONBLOCK."
    #endif
  }

  return sock;
}

static inline int __socket_accept_connection (Socket sock) {
  int optval;
  socklen_t optlen = sizeof(optval);

  if (getsockopt(sock, SOL_SOCKET, SO_ACCEPTCONN, &optval, &optlen) == -1)
    return 0;

  return !!optval;
}

Socket tcp_accept (Socket server) {
  struct sockaddr_in addr;
  socklen_t len;
  Socket sock;

  /* La socket donnée n'est pas un socket serveur. */
  if (!__socket_accept_connection(server))
    return -1;

  /* Acceptation d'une connexion TCP. */
  len = sizeof(addr);

  if ((sock = accept(server, (struct sockaddr *)&addr, &len)) < 0)
    return -1;

  return sock;
}

int tcp_send (Socket sock, void *data, int len) {
  char *p = data;
  int len_s = 0, len_t;

  if (__socket_accept_connection(sock))
    return -1;

  do {
    if ((len_t = send(sock, p, len, 0)) > 0) {
      len -= len_t;
      len_s += len_t;
      p += len;
    }

    /* On envoie tant qu'il reste des données à envoyer et que
       rien de terrible ne se soit passé autre qu'un signal EINTR. */
  } while (len > 0 && (errno == EINTR || len_t > 0));

  return len_s;
}

int tcp_recv (Socket sock, void *data, int len) {
  int len_r;

  if (__socket_accept_connection(sock))
    return -1;

  do {
    len_r = recv(sock, data, len, 0);
  } while (errno == EINTR);

  return len_r;
}

void tcp_close (Socket sock) {
  if (sock > 0)
    close(sock);

  return;
}
