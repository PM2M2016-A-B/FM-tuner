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

#include <signal.h>
#include <stdlib.h>

#include "hw/led.h"
#include "net/handler.h"
#include "net/server.h"
#include "utils/error.h"

#include "fm_tuner.h"

#define SERVER_MAX_CLIENTS 10
#define SERVER_DEFAULT_PORT 9502

static Fm_tuner fm_tuner;

static void __disable_signals (void) {
  sigset_t set;

  sigfillset(&set);
  sigprocmask(SIG_BLOCK, &set, NULL);

  return;
}

static void __disable_leds (void) {
  int i;

  for (i = 0; i < LEDS_N; i++)
    led_set_state(i, LED_INACTIVE);

  return;
}

static void __enable_tuner (void) {
  Fm_tuner_conf conf = {
    .i2c_id = 1,
    .pin_rst = 45,
    .pin_sdio = 12,
    .tuner_addr = 0x10
  };

  debug("Init FM tuner.\n");

  if (fm_tuner_init(&fm_tuner, &conf) == -1)
    exit(EXIT_FAILURE);

  #ifdef DEBUG
    debug("Registers data at init:\n");
    fm_tuner_print_registers(&fm_tuner);
  #endif

  return;
}

static void __disable_tuner (void) {
  fm_tuner_close(&fm_tuner);
  return;
}

/* --------------------------------------------------------------------- */

int main (void) {
  Handler_value handler_value = {
    .fm_tuner = &fm_tuner,
    .rds = rds_new()
  };
  Server_conf conf = {
    .port = SERVER_DEFAULT_PORT,
    .max_clients = SERVER_MAX_CLIENTS,
    .user_value = &handler_value,
    .handlers = {
      .event = handler_event,
      .join = handler_join,
      .quit = handler_quit,
      .loop = handler_loop
    }
  };

  __disable_signals();
   __disable_leds();
   __enable_tuner();
  atexit(__disable_tuner);

  // TMP
  printf("volume init %d\n", fm_tuner_set_volume(&fm_tuner, 5));
  printf("channel init %d\n", fm_tuner_set_channel(&fm_tuner, 933));

  server_run(&conf, 500);

  rds_free(handler_value.rds);

  exit(EXIT_SUCCESS);
}
