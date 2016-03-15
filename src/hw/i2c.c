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
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "i2c.h"

#define PATH_I2C "/dev/i2c-"

/* Documentation: https://www.kernel.org/doc/Documentation/i2c/dev-interface */

int i2c_open (unsigned int bus_id, char addr) {
  int fd;
  char filename[128];

  sprintf(filename, PATH_I2C "%d", bus_id);

  if ((fd = open(filename, O_RDWR)) == -1)
    return -1;

  if (ioctl(fd, I2C_SLAVE, addr) == -1 ||
      ioctl(fd, I2C_PEC, 1) == -1) {
    close(fd);
    return -1;
  }

  return fd;
}

int i2c_close (int fd) {
  return close(fd);
}

ssize_t i2c_write (int fd, void *buf, size_t count) {
  return write(fd, buf, count);
}

ssize_t i2c_read (int fd, void *buf, size_t count) {
  return read(fd, buf, count);
}
