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

#ifndef _SOCKET_H_
#define _SOCKET_H_

#if defined __unix__ /* UNIX */
  #include <arpa/inet.h> /* in_addr_t, in_port_t */
  #include <sys/select.h> /* select & fd_set */
  typedef int Socket;
#else
  #error "Socket is not compatible with this platform."
#endif

/* Une IP. Vraiment! */
typedef struct IP {
  in_addr_t host; /* Host - IPv4. */
  in_port_t port; /* Port. */
} IP;

/* Un ensemble de sockets. */
typedef struct Socket_set Socket_set;

typedef fd_set Sockets_states;

/* ---------------------------------------------------------------------- */

/* Sérialise des données. */
char *serialize_uint8 (char *buf, uint8_t value);
char *serialize_uint16 (char *buf, uint16_t value);
char *serialize_uint32 (char *buf, uint32_t value);

/* Désérialise des données. */
char *deserialize_uint8 (char *buf, uint8_t *value);
char *deserialize_uint16 (char *buf, uint16_t *value);
char *deserialize_uint32 (char *buf, uint32_t *value);

/* ---------------------------------------------------------------------- */

/* Remplit une structure IP avec un hostname et un port.
   Retourne -1 en cas d'échec, 0 sinon. */
int resolve_host (IP *ip, const char *hostname, in_port_t port);

/* ---------------------------------------------------------------------- */

/* Crée un nouvel ensemble de sockets de taille n.
   Retourne un nouvel ensemble ou NULL en cas d'erreur. */
Socket_set *socket_set_new (int n);

/* Libère un ensemble de sockets. */
void socket_set_free (Socket_set *ss);

/* Ajoute un socket à un ensemble de sockets.
   Retourne l'emplacement du socket ajouté ou -1 en cas d'erreur. */
int socket_set_add (Socket_set *ss, Socket sock);

/* Supprime un socket d'un ensemble de sockets.
   Retourne l'emplacement du socket supprimé ou -1 en cas d'erreur. */
int socket_set_remove (Socket_set *ss, Socket sock);

/* Retourne une socket en position id ou -1 sinon. */
Socket socket_set_get (Socket_set *ss, int id);

/* Applique la fonction select à un ensemble de sockets.
   Retourne -1 en cas d'erreur, ou sinon les sockets ayant reçus des données. */
int socket_set_select (Socket_set *ss, Sockets_states *states, int *stdin, int timeout);
int socket_is_ready (Socket socket, Sockets_states *states);

/* Donne la taille actuelle d'un ensemble de sockets.
   Retourne -1 en cas d'erreur, le nombre de sockets sinon. */
int socket_set_get_size (Socket_set *ss);

/* Donne la taille max d'un ensemble de sockets.
   Retourne -1 en cas d'erreur, le nombre de sockets max sinon. */
int socket_set_get_max_size (Socket_set *ss);

/* ---------------------------------------------------------------------- */

/* Obtenir un socket relative à une ip. Que ce soit un socket cliente ou serveur.
   Retourne -1 en cas d'échec ou un socket sinon. */
Socket tcp_get (IP *ip);

/* Accepte une connexion TCP.
   Retourne un socket ou -1 en cas d'erreur. */
Socket tcp_accept (Socket server);

/* Envoit des données à partir d'un socket.
   Retourne -1 en cas d'échec ou le nombre d'octets envoyés. */
int tcp_send (Socket sock, void *data, int len);

/* Reçoit des données à partir d'un socket.
   Retourne -1 en cas d'échec ou le nombre d'octets reçus. */
int tcp_recv (Socket sock, void *data, int len);

/* Ferme un socket. */
void tcp_close (Socket sock);

#endif /* _SOCKET_H_ INCLUDED */
