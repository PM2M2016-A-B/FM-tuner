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
#include <stdlib.h>

#include "utils/alloc.h"
#include "utils/error.h"
#include "utils/ptime.h"

#include "seek.h"

#define RSSI_TRIES 10

typedef struct Channel_data {
  int channel;
  int rssi;
} Channel_data;

int channel_data_cmp (const void *a, const void *b) {
  return ((Channel_data *)a)->rssi - ((Channel_data *)b)->rssi;
}

void seek_utils (Fm_tuner *fm_tuner) {
  int channel, success;
  float rssi;
  int i, n;
  int n_data = 0;
  Channel_data *data = NULL;

  fm_tuner_set_channel(fm_tuner, FM_TUNER_CHANNEL_START);

  while ((channel = fm_tuner_seek(fm_tuner, FM_TUNER_SEEKUP, &success)) != -1 &&
         channel != FM_TUNER_CHANNEL_START) {
    rssi = 0;

    printf("Channel %d...\n", channel);

    for (i = 0; i < RSSI_TRIES; i++) {
      if ((n = fm_tuner_get_rssi(fm_tuner)) == -1)
        fatal_error("Unable to get RSSI.");

      rssi += n;
      sleep_m(1);
    }

    rssi /= RSSI_TRIES;

    if (n_data % 10 == 0)
      prealloc(data, (n_data + 10) * sizeof *data);

    if (success) {
      data[n_data].channel = channel;
      data[n_data].rssi = rssi * 100 / (float)FM_TUNER_RSSI_MAX;
      n_data++;
    }
  }

  qsort(data, n_data, sizeof *data, channel_data_cmp);

  for (i = 0; i < n_data; i++)
    printf("RSSI=%d%%, channel=%d\n", data[i].rssi, data[i].channel);

  free(data);

  return;
}
