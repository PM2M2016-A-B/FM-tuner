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

#include <stdio.h>
#include <string.h>

#include "../utils/error.h"
#include "../utils/ptime.h"

#include "handler.h"

/* Temps minimum à attendre avant un nouveau message RDS. */
#define SLEEP_DELAY 40

/* Taille totale en bytes des blocks RDS. */
#define RDS_BLOCKS_SIZE (4 * sizeof(uint16_t))

/* Types d'events clients/serveur. */
#define EVENT_MALFORMED_MESSAGE 0
#define EVENT_VOLUME 1
#define EVENT_CHANNEL 2
#define EVENT_SEEKUP 3
#define EVENT_SEEKDOWN 4

/* Taille des events. size(Id_event) + size(Data_event) en bytes. */
#define EVENT_VOLUME_SIZE 2
#define EVENT_CHANNEL_SIZE 3

/* Message d'erreur renvoyé si un client a émis une mauvaise requête. */
static const char MALFORMED_MESSAGE[] = { 1, EVENT_MALFORMED_MESSAGE };
#define MALFORMED_MESSAGE_SIZE (sizeof(MALFORMED_MESSAGE))

/* Masks utilisés sur Handler_value.to_set. */
#define MASK_VOLUME (1 << (EVENT_VOLUME - 1))
#define MASK_CHANNEL (1 << (EVENT_CHANNEL - 1))
#define MASK_SEEKUP (1 << (EVENT_SEEKUP - 1))
#define MASK_SEEKDOWN (1 << (EVENT_SEEKDOWN - 1))

#define SEND_BUFFER_SIZE 128

/* --------------------------------------------------------------------- */

static inline int __add_volume_to_buf(char *buf, int volume) {
  *buf++ = EVENT_VOLUME;
  serialize_uint8(buf, volume & 0x000000FF);
  return EVENT_VOLUME_SIZE;
}

static int __set_volume (Fm_tuner *fm_tuner, char *buf, int volume) {
  int new_volume = fm_tuner_set_volume(fm_tuner, volume);

  if (new_volume == -1) {
    error("[server]Set volume failed.");
    return 0;
  }

  printf("[server]Set volume: %d.\n", new_volume);
  return __add_volume_to_buf(buf, new_volume);
}

static inline int __add_channel_to_buf(char *buf, int channel) {
  *buf++ = EVENT_CHANNEL;
  serialize_uint16(buf, channel & 0x0000FFFF);
  return EVENT_CHANNEL_SIZE;
}

static int __set_channel (Fm_tuner *fm_tuner, char *buf, int channel) {
  int new_channel = fm_tuner_set_channel(fm_tuner, channel);

  if (new_channel == -1) {
    error("[server]Set channel failed.");
    return 0;
  }

  printf("[server]Set channel: %d.\n", new_channel);
  return __add_channel_to_buf(buf, new_channel);
}

static int __seek (Fm_tuner *fm_tuner, char *buf, int direction) {
  int cur_channel = fm_tuner_get_channel(fm_tuner);
  int new_channel;
  int success;

  if ((new_channel = fm_tuner_seek(fm_tuner, direction, &success)) == -1 || !success) {
    error("[server]Seek failed.");
    return __set_channel(fm_tuner, buf, cur_channel);
  }

  printf("[server]Seek success, channel: %d.\n", new_channel);
  return __add_channel_to_buf(buf, new_channel);
}

/* --------------------------------------------------------------------- */

static int __parse_event (char *buf, int len, Handler_value *value) {
  char to_set = 0;
  uint8_t new_volume;
  uint16_t new_channel;

  if (len <= 0)
    return -1;

  /* Tant que le message n'est pas traité en entier... */
  while (len > 0)
    switch (*buf++) {
      case EVENT_VOLUME:
        if (len < EVENT_VOLUME_SIZE)
          return -1;

        to_set |= MASK_VOLUME;
        buf = deserialize_uint8(buf, &new_volume);
        len -= EVENT_VOLUME_SIZE;
        break;

      case EVENT_CHANNEL:
        if (len < EVENT_CHANNEL_SIZE)
          return -1;

        to_set |= MASK_CHANNEL;
        buf = deserialize_uint16(buf, &new_channel);
        len -= EVENT_CHANNEL_SIZE;
        break;

      case EVENT_SEEKUP:
        to_set |= MASK_SEEKUP;
        buf++;
        len--;
        break;

      case EVENT_SEEKDOWN:
        to_set |= MASK_SEEKDOWN;
        buf++;
        len--;
        break;

      default:
        return -1;
    }

  /* Mise en cache des registres à mettre à jour côté tuner. */
  if (to_set & MASK_VOLUME)
    value->new_volume = new_volume;
  if (to_set & MASK_CHANNEL)
    value->new_channel = new_channel;

  value->to_set |= to_set;

  return 0;
}

/* --------------------------------------------------------------------- */

static void __broadcast (Socket_set *ss, Handler_value *value) {
  static char buf[SEND_BUFFER_SIZE];
  char *p = buf + 1;

  Socket sock;
  int i = socket_set_get_max_size(ss);

  /* Mise à jour des registres. */
  if (value->to_set & MASK_VOLUME)
    p += __set_volume(value->fm_tuner, p, value->new_volume);

  if (value->to_set & MASK_CHANNEL)
    p += __set_channel(value->fm_tuner, p, value->new_channel);
  else if (value->to_set & MASK_SEEKUP)
    p += __seek(value->fm_tuner, p, FM_TUNER_SEEKUP);
  else if (value->to_set & MASK_SEEKDOWN)
    p += __seek(value->fm_tuner, p, FM_TUNER_SEEKDOWN);

  /* TODO: RDS */

  *buf = p - buf;

  /* Broadcast. */
  if (*buf - 1 > 0)
    for (i--; i > 0; i--)
      if ((sock = socket_set_get(ss, i)) != -1)
        tcp_send(sock, buf, *buf);

  /* Reset. */
  value->to_set = 0;

  return;
}

/* --------------------------------------------------------------------- */

int handler_event (Socket sock, int id, char *buf, int len, void *user_value) {
  Handler_value *value = user_value;
  int msg_len = len;
  char *p = buf;

  /* Parse un ensemble de messages clients. */
  while (msg_len > 0 && msg_len >= *buf) {
    printf("[server]Received message of client %d: ", id);

    for (; p - buf < *buf; )
      printf("%02x", *p++);

    printf(" (length=%d)\n", *buf);

    /* Parse un message. */
    if (__parse_event(buf + 1, *buf - 1, value) == -1) {
      printf("[server]Malformed message of client %d.\n", id);

      /* Indique une erreur et deconnecte le client. */
      tcp_send(sock, (void *)MALFORMED_MESSAGE, MALFORMED_MESSAGE_SIZE);
      shutdown(sock, SHUT_RDWR);

      return len;
    }

    msg_len -= *buf;
    buf += *buf;
  }

  return len - msg_len;
}

void handler_join (Socket sock, int id, void *user_value) {
  Handler_value *value = user_value;
  static char buf[SEND_BUFFER_SIZE];
  char *p = buf + 1;

  (void)id;

  /* Envoie les valeurs actuelles du volume/channel. */
  p += __add_volume_to_buf(p, fm_tuner_get_volume(value->fm_tuner));
  p += __add_channel_to_buf(p, fm_tuner_get_channel(value->fm_tuner));
  *buf = p - buf;

  if (*buf - 1 > 0)
    tcp_send(sock, buf, *buf);

  return;
}

void handler_quit (Socket sock, int id, void *user_value) {
  (void)sock;
  (void)id;
  (void)user_value;

  return;
}

void handler_loop (Socket_set *ss, void *user_value) {
  static uint16_t prev_blocks[4]; /* Permet d'éliminer les doublons. */
  static Time t_prev;
  static Time t_cur;

  Handler_value *value = user_value;
  uint16_t blocks[4];
  int data_exists;
  long sleep_time;

  /* Sleep. */
  time_get_cur(&t_cur);

  sleep_time = SLEEP_DELAY - time_diff(&t_prev, &t_cur);
  sleep_m(sleep_time);

  time_get_cur(&t_prev);

  /* Parsing du RDS. */
  fm_tuner_read_rds(value->fm_tuner, blocks, &data_exists);

  if (data_exists && memcmp(blocks, prev_blocks, RDS_BLOCKS_SIZE)) {
    rds_decode(value->rds, blocks);
    memcpy(prev_blocks, blocks, RDS_BLOCKS_SIZE);
  }

  /* Brooooooooadcast. */
  __broadcast(ss, value);

  return;
}
