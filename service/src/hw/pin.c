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

#include "pin.h"

#define PATH_PIN_PREFIX "/sys/class/gpio/gpio"
#define PATH_EXPORT "/sys/class/gpio/export"

/* Documentation: "doc/GPIO_Programming_on_the_Beaglebone.pdf" */

static inline int __get_pin_directory (char *dest, int pin) {
  return sprintf(dest, PATH_PIN_PREFIX "%d/", pin & 0xFF);
}

int pin_open (int pin) {
  char buf[128];
  char pin_s[4];
  int fd;

  __get_pin_directory(buf, pin);

  if (!access(buf, F_OK))
    return 0;

  if ((fd = open(PATH_EXPORT, O_WRONLY)) == -1)
    return -1;

  if (write(fd, pin_s, sprintf(pin_s, "%d", pin & 0xFF)) == -1) {
    close(fd);
    return -1;
  }

  return close(fd);
}

static int __set_attribute (int pin, const char *attribute, const char *value) {
  char buf[128];
  int fd;
  int end = __get_pin_directory(buf, pin);

  sprintf(buf + end, attribute);

  if ((fd = open(buf, O_WRONLY)) == -1)
    return -1;

  if (write(fd, value, strlen(value)) == -1) {
    close(fd);
    return -1;
  }

  return close(fd);
}

int pin_set_direction (int pin, int direction) {
  if (direction != PIN_IN && direction != PIN_OUT)
    return -1;

  return __set_attribute(pin, "direction", direction == PIN_IN ? "in" : "out");
}

int pin_set_value (int pin, int value) {
  if (value != PIN_LOW && value != PIN_HIGH)
    return -1;

  return __set_attribute(pin, "value", value == PIN_LOW ? "0" : "1");
}
