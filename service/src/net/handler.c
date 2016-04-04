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

#include "../hw/led.h"
#include "../utils/error.h"
#include "../utils/ptime.h"

#include "handler.h"

/* Temps minimum à attendre avant un nouveau message RDS. */
#define SLEEP_DELAY 40

/* Taille totale en bytes des blocks RDS. */
#define RDS_BLOCKS_SIZE (RDS_BLOCKS_N * sizeof(uint16_t))

/* Types d'events clients/serveur. */
#define EVENT_MALFORMED_MESSAGE 0
#define EVENT_VOLUME 1
#define EVENT_CHANNEL 2
#define EVENT_SEEKUP 3
#define EVENT_SEEKDOWN 4
#define EVENT_RADIO_NAME 5
#define EVENT_RADIO_TEXT 6

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

static inline void __print_message (const char *buf) {
  int i;

  for (i = 0; i < *buf; i++)
      printf("%02x", buf[i]);

  printf(" (length=%d)\n", *buf);

  return;
}

/* --------------------------------------------------------------------- */

static inline int __add_uint8_to_buf (char *buf, uint8_t event, uint8_t value) {
  *buf++ = event;
  serialize_uint8(buf, value);
  return 2;
}

static inline int __add_uint16_to_buf (char *buf, uint8_t event, uint16_t value) {
  *buf++ = event;
  serialize_uint16(buf, value);
  return 3;
}

static inline int __add_text_to_buf (char *buf, uint8_t event, const char *s, uint8_t len) {
  *buf++ = event;
  *buf++ = len;
  strncpy(buf, s, len);

  return len + 2;
}

/* --------------------------------------------------------------------- */

static int __add_volume_to_buf (char *buf, Fm_tuner *fm_tuner, int volume) {
  int new_volume = fm_tuner_set_volume(fm_tuner, volume);

  if (new_volume == -1) {
    error("[server]Set volume failed.");
    return 0;
  }

  printf("[server]Set volume: %d.\n", new_volume);
  return __add_uint8_to_buf(buf, EVENT_VOLUME, new_volume);
}

static int __add_channel_to_buf (char *buf, Fm_tuner *fm_tuner, int channel) {
  int new_channel = fm_tuner_set_channel(fm_tuner, channel);

  if (new_channel == -1) {
    error("[server]Set channel failed.");
    return 0;
  }

  printf("[server]Set channel: %d.\n", new_channel);
  return __add_uint16_to_buf(buf, EVENT_CHANNEL, new_channel);
}

static int __add_seek_to_buf (char *buf, Fm_tuner *fm_tuner, int direction) {
  int cur_channel = fm_tuner_get_channel(fm_tuner);
  int new_channel;
  int success;

  if ((new_channel = fm_tuner_seek(fm_tuner, direction, &success)) == -1 || !success) {
    error("[server]Seek failed.");
    return __add_channel_to_buf(buf, fm_tuner, cur_channel);
  }

  printf("[server]Seek success, channel: %d.\n", new_channel);
  return __add_uint16_to_buf(buf, EVENT_CHANNEL, new_channel);
}

static int __add_radio_name_to_buf (char *buf, Rds *rds) {
  static char prev_radio_name[RDS_RADIO_NAME_MAX_LENGTH + 1];
  const char *radio_name = rds_get_radio_name(rds);

  if (strcmp(prev_radio_name, radio_name)) {
    strcpy(prev_radio_name, radio_name);
    return __add_text_to_buf(buf, EVENT_RADIO_NAME, radio_name, strlen(radio_name));
  }

  return 0;
}

static int __add_radio_text_to_buf (char *buf, Rds *rds) {
  static char prev_radio_text[RDS_RADIO_TEXT_MAX_LENGTH + 1];
  const char *radio_text = rds_get_radio_text(rds);

  if (strcmp(prev_radio_text, radio_text)) {
    strcpy(prev_radio_text, radio_text);
    return __add_text_to_buf(buf, EVENT_RADIO_TEXT, radio_text, strlen(radio_text));
  }

  return 0;
}

/* --------------------------------------------------------------------- */

static void __update_leds (Fm_tuner *fm_tuner) {
  float rssi = fm_tuner_get_rssi(fm_tuner) * 100 / (float)FM_TUNER_RSSI_MAX;
  int n = rssi / 20;
  int i;

  for (i = 0; i < n; i++)
    led_set_state(i, LED_ACTIVE);
  for (; i < LEDS_N; i++)
    led_set_state(i, LED_INACTIVE);

  return;
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

int handler_event (Socket sock, int id, char *buf, int len, void *user_value) {
  Handler_value *value = user_value;
  int msg_len = len;

  /* Parse un ensemble de messages clients. */
  while (msg_len > 0 && msg_len >= *buf) {
    printf("[server]Received message of client %d: ", id);
    __print_message(buf);

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

/* --------------------------------------------------------------------- */

void handler_join (Socket sock, int id, void *user_value) {
  Handler_value *value = user_value;
  static char buf[SEND_BUFFER_SIZE];
  char *p = buf + 1;

  const char *radio_name = rds_get_radio_name(value->rds);
  const char *radio_text = rds_get_radio_text(value->rds);

  (void)id;

  /* Envoie les valeurs actuelles du volume/channel, radio name/text. */
  p += __add_uint8_to_buf(p, EVENT_VOLUME, fm_tuner_get_volume(value->fm_tuner));
  p += __add_uint16_to_buf(p, EVENT_CHANNEL, fm_tuner_get_channel(value->fm_tuner));
  p += __add_text_to_buf(p, EVENT_RADIO_NAME, radio_name, strlen(radio_name));
  p += __add_text_to_buf(p, EVENT_RADIO_TEXT, radio_text, strlen(radio_text));
  *buf = p - buf;

  if (*buf - 1 > 0)
    tcp_send(sock, buf, *buf);

  return;
}

/* --------------------------------------------------------------------- */

void handler_quit (Socket sock, int id, void *user_value) {
  (void)sock;
  (void)id;
  (void)user_value;

  return;
}

/* --------------------------------------------------------------------- */

static void __sleep (void) {
  static Time t_prev;
  static Time t_cur;
  long sleep_time;

  time_get_cur(&t_cur);

  sleep_time = SLEEP_DELAY - time_diff(&t_prev, &t_cur);
  sleep_m(sleep_time);

  time_get_cur(&t_prev);

  return;
}

static void __rds_decode (Handler_value *value) {
  static uint16_t prev_blocks[RDS_BLOCKS_N]; /* Permet d'éliminer les doublons. */
  uint16_t blocks[RDS_BLOCKS_N];
  int data_exists;

  fm_tuner_read_rds(value->fm_tuner, blocks, &data_exists);

  if (data_exists && memcmp(blocks, prev_blocks, RDS_BLOCKS_SIZE)) {
    rds_decode(value->rds, blocks);
    memcpy(prev_blocks, blocks, RDS_BLOCKS_SIZE);
  }

  return;
}

static void __broadcast (Socket_set *ss, Handler_value *value) {
  static char buf[SEND_BUFFER_SIZE];
  Socket sock;
  char *p = buf + 1;
  int i = socket_set_get_max_size(ss);

  /* Mise à jour des registres. */
  if (value->to_set & MASK_VOLUME)
    p += __add_volume_to_buf(p, value->fm_tuner, value->new_volume);

  if (value->to_set & MASK_CHANNEL)
    p += __add_channel_to_buf(p, value->fm_tuner, value->new_channel);
  else if (value->to_set & MASK_SEEKUP)
    p += __add_seek_to_buf(p, value->fm_tuner, FM_TUNER_SEEKUP);
  else if (value->to_set & MASK_SEEKDOWN)
    p += __add_seek_to_buf(p, value->fm_tuner, FM_TUNER_SEEKDOWN);

  /* Ajout du RDS. */
  p += __add_radio_name_to_buf(p, value->rds);
  p += __add_radio_text_to_buf(p, value->rds);

  *buf = p - buf;

  /* Broadcast. */
  if (*buf - 1 > 0) {
    printf("[server]Broadcast: ");
    __print_message(buf);

    for (i--; i > 0; i--)
      if ((sock = socket_set_get(ss, i)) != -1)
        tcp_send(sock, buf, *buf);
  }

  /* Reset. */
  value->to_set = 0;

  return;
}

void handler_loop (Socket_set *ss, void *user_value) {
  __sleep();
  __update_leds(((Handler_value *)user_value)->fm_tuner);
  __rds_decode(user_value);
  __broadcast(ss, user_value);

  return;
}
