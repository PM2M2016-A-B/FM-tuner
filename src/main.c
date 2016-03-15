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

#include "hw/led.h"
#include "rds.h"
#include "utils/error.h"
#include "utils/ptime.h"

#include "fm_tuner.h"

static inline void __disable_leds (void) {
  int i;

  debug("Disable leds.\n");

  for (i = 0; i < LEDS_N; i++)
    led_set_state(i, LED_INACTIVE);

  return;
}

static inline void __enable_tuner (Fm_tuner *fm_tuner) {
  Fm_tuner_conf conf = {
    .i2c_id = 1,
    .pin_rst = 45,
    .pin_sdio = 12,
    .tuner_addr = 0x10
  };

  debug("Init FM tuner.\n");

  if (fm_tuner_init(fm_tuner, &conf) == -1)
    exit(EXIT_FAILURE);

  #ifdef DEBUG
    debug("Registers data at init:\n");
    fm_tuner_print_registers(fm_tuner);
  #endif

  return;
}

static int __set_volume (Fm_tuner *fm_tuner, int value) {
  debug("Set volume.\n");
  return fm_tuner_set_volume(fm_tuner, value);
}

static int __set_channel (Fm_tuner *fm_tuner, int value) {
  debug("Set channel: %d\n", value);
  return fm_tuner_set_channel(fm_tuner, value);
}

int main (int argc, char *argv[]) {
  Fm_tuner fm_tuner;
  uint16_t blocks[4];
  int data_exists;
  Rds rds;

  (void)argc;
  (void)argv;

  __disable_leds();
  __enable_tuner(&fm_tuner);

  if (__set_volume(&fm_tuner, 5) == -1)
    goto err;

  /* Nostalgie: 933. */
  /* Rire et chanson: 950. */
  /* Beur FM: 978. */
  /* Fun radio: 988. */
  /* Radio classique: 1024. */
  /* NRJ: 1032. */
  /* RMC: 1042. */
  /* Europe 1: 1046. */
  /* Chérie FM: 1058. */
  if (__set_channel(&fm_tuner, 933) == -1)
    goto err;

  rds_init(&rds);

  for (;;) {
    fm_tuner_read_rds(&fm_tuner, blocks, &data_exists);

    if (data_exists) {
      rds_decode(&rds, blocks);
      printf("data type: %d\n", rds_get_data_type(&rds));
    }
    sleep_m(40);
  }

  if (fm_tuner_close(&fm_tuner) == -1)
    exit(EXIT_FAILURE);

  exit(EXIT_SUCCESS);

 err:
  fm_tuner_close(&fm_tuner);
  exit(EXIT_FAILURE);
}
