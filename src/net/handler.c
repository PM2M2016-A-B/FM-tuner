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

#include <string.h>

#include "../utils/error.h"
#include "../utils/ptime.h"

#include "handler.h"

#define SLEEP_DELAY 40

#define RDS_BLOCKS_SIZE (4 * sizeof(uint16_t))

static int __set_volume (Fm_tuner *fm_tuner, int value) {
  debug("Set volume.\n");
  return fm_tuner_set_volume(fm_tuner, value);
}

static int __set_channel (Fm_tuner *fm_tuner, int value) {
  debug("Set channel: %d\n", value);
  return fm_tuner_set_channel(fm_tuner, value);
}

/* --------------------------------------------------------------------- */

int handler_event (Socket sock, int id, char *buffer, int len, void *user_value) {
  (void)sock;
  (void)id;
  (void)buffer;
  (void)len;
  (void)user_value;

  return 0;
}

void *handler_join (Socket sock, int id, void *user_value) {
  (void)sock;
  (void)id;
  (void)user_value;

  return 0;
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

  (void)ss;

  time_get_cur(&t_cur);
  sleep_time = SLEEP_DELAY - time_diff(&t_prev, &t_cur);

  // TMP
  // debug("Sleep %ldms.\n", sleep_time);

  sleep_m(sleep_time);
  time_get_cur(&t_prev);

  fm_tuner_read_rds(value->fm_tuner, blocks, &data_exists);

  /* Parsing du RDS si possible. */
  if (data_exists && memcmp(blocks, prev_blocks, RDS_BLOCKS_SIZE))
    rds_decode(value->rds, blocks);

  memcpy(prev_blocks, blocks, RDS_BLOCKS_SIZE);

  return;
}
