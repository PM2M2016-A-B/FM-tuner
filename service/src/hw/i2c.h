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

#ifndef _I2C_H_
#define _I2C_H_

#include <sys/types.h>

/* Donne l'accès à un adaptateur I2C situé sur le
   bus /dev/i2c-(bus_id) à l'adresse (addr).
   Retourne -1 en cas d'échec, sinon 0. */
int i2c_open (unsigned int bus_id, char addr);

/* Ferme un descripteur obtenu par l'appel de i2c_open.
   Retourne -1 en cas d'échec, sinon 0. */
int i2c_close (int fd);

/* Ecrire/Lire des données d'un adaptateur I2C.
   Retourne -1 en cas d'échec sinon le nombre de bytes lus/écrits. */
ssize_t i2c_write (int fd, void *buf, size_t count);
ssize_t i2c_read (int fd, void *buf, size_t count);

#endif /* _I2C_H_ INCLUDED */
