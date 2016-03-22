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
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "fm_tuner.h"
#include "hw/led.h"
#include "net/handler.h"
#include "net/server.h"
#include "seek.h" /* seek_utils. */
#include "utils/error.h"

#define DEFAULT_I2C_ID 1
#define DEFAULT_MAX_CLIENTS 10
#define DEFAULT_PIN_RST 45
#define DEFAULT_PIN_SDIO 12
#define DEFAULT_PORT 9502

#define MODE_SERVER 0
#define MODE_SEEK 1

static Fm_tuner *fm_tuner;

/* --------------------------------------------------------------------- */

static void __disable_leds (void) {
  int i;

  for (i = 0; i < LEDS_N; i++)
    led_set_state(i, LED_INACTIVE);

  return;
}

static void __create_tuner (Fm_tuner_conf *conf) {
  fm_tuner = fm_tuner_new(conf);

  debug("Init FM tuner.\n");

  #ifdef DEBUG
    debug("Registers data at init:\n");
    fm_tuner_print_registers(fm_tuner);
  #endif

  return;
}

static void __delete_tuner (void) {
  fm_tuner_free(fm_tuner);
  return;
}

static void __usage (const char *progname) {
  printf("Usage: %s [OPTION]...\n", progname);
  printf("  -h, --help           Print this helper.\n");
  printf("  -i, --i2c-id=ID      Set the i2c bus id. Default: %d.\n", DEFAULT_I2C_ID);
  printf("  -m, --max-clients=N  Set the number max of server clients. Default: %d.\n", DEFAULT_MAX_CLIENTS);
  printf("  -p, --port=PORT      Set the server port. Default: %d.\n", DEFAULT_PORT);
  printf("  -r, --reset-pin=PIN  Set the reset pin number of the fm tuner. Default: %d.\n", DEFAULT_PIN_RST);
  printf("  -s, --sdio-pin=PIN   Set the sdio pin number of the fm tuner. Default: %d.\n", DEFAULT_PIN_SDIO);
  printf("      --seek           Seek to locate radio stations.\n");

  exit(EXIT_SUCCESS);
}

/* --------------------------------------------------------------------- */

static int __parse_arguments (int argc, char *argv[], Server_conf *server_conf, Fm_tuner_conf *fm_tuner_conf) {
  static const char *opts = "hm:p:i:r:s:";
  static struct option long_opts[] = {
    { "help", no_argument, NULL, 'h' },
    { "i2c-id", required_argument, NULL, 'i' },
    { "max-clients", required_argument, NULL, 'm' },
    { "port", required_argument, NULL, 'p' },
    { "reset-pin", required_argument, NULL, 'r' },
    { "sdio-pin", required_argument, NULL, 's' },
    { "seek", no_argument, NULL, 'l' },
    { 0, 0, 0, 0}
  };

  int opt;
  int opt_index;
  long value;
  char *endptr;

  int mode = MODE_SERVER;

  errno = 0;

  while ((opt = getopt_long(argc, argv, opts, long_opts, &opt_index)) != -1) {
    if (opt == 'h' || opt == '?')
      __usage(*argv);

    if (opt == 'l') {
      mode = MODE_SEEK;
      continue;
    }

    if ((value = strtol(optarg, &endptr, 10)) < 0 || errno != 0 || optarg == endptr) {
      fprintf(stderr, "error: %s must be an valid unsigned integer.\n", long_opts[opt_index].name);
      exit(EXIT_FAILURE);
    }

    switch (opt) {
      case 'm':
        server_conf->max_clients = value;
        break;
      case 'p':
        server_conf->port = value;
        break;
      case 'i':
        fm_tuner_conf->i2c_id = value;
        break;
      case 'r':
        fm_tuner_conf->pin_rst = value;
        break;
      case 's':
        fm_tuner_conf->pin_sdio = value;
        break;
    }
  }

  return mode;
}

int main (int argc, char *argv[]) {
  static Handler_value handler_value = {
    .to_set = 0
  };
  static Server_conf server_conf = {
    .port = DEFAULT_PORT,
    .max_clients = DEFAULT_MAX_CLIENTS,
    .user_value = &handler_value,
    .handlers = {
      .event = handler_event,
      .join = handler_join,
      .quit = handler_quit,
      .loop = handler_loop
    }
  };
  static Fm_tuner_conf fm_tuner_conf = {
    .i2c_id = DEFAULT_I2C_ID,
    .pin_rst = DEFAULT_PIN_RST,
    .pin_sdio = DEFAULT_PIN_SDIO,
    .tuner_addr = 0x10
  };

  int mode = __parse_arguments(argc, argv, &server_conf, &fm_tuner_conf);

  __create_tuner(&fm_tuner_conf);
  atexit(__delete_tuner);
  __disable_leds();

  if (mode == MODE_SEEK)
    seek_utils(fm_tuner);
  else {
    handler_value.fm_tuner = fm_tuner;
    handler_value.rds = rds_new();
    server_run(&server_conf, 500);
    rds_free(handler_value.rds);
  }

  exit(EXIT_SUCCESS);
}
