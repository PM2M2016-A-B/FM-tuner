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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"

static inline void __error (const char *msg, va_list ap) {
  vfprintf(stderr, msg, ap);
  fprintf(stderr, " (errno=%s)\n", strerror(errno));

  return;
}

void fatal_error (const char *msg, ...) {
  va_list ap;

  va_start(ap, msg);
  __error(msg, ap);
  va_end(ap);

  exit(EXIT_FAILURE);
}

int error (const char *msg, ...) {
  va_list ap;

  va_start(ap, msg);
  __error(msg, ap);
  va_end(ap);

  return -1;
}

void debug (const char *format, ...) {
  va_list ap;

  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);

  return;
}
