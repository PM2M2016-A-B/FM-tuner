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

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "led.h"

#define BUFFER_SIZE 128
#define PATH_LEDS "/sys/class/leds/beaglebone:green:usr"

static int __set_attribute (int id, const char *filename, const char *value) {
  int fd;
  char buf[BUFFER_SIZE];

  if (id < 0 || id > LEDS_N)
    return -1;

  if (snprintf(buf, BUFFER_SIZE, PATH_LEDS "%d/%s", id, filename) >= BUFFER_SIZE)
    return -1;

  if ((fd = open(buf, O_WRONLY)) == -1)
    return -1;

  if (write(fd, value, strlen(value)) == -1) {
    close(fd);
    return -1;
  }

  return close(fd);
}

int led_set_state (int id, int state) {
  if (__set_attribute(id, "trigger", "none") == -1)
    return -1;

  return __set_attribute(id, "brightness", state == LED_ACTIVE ? "1" : "0");
}
