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

#ifndef _PIN_H_
#define _PIN_H_

/* Directions possibles d'un pin. */
#define PIN_IN 0
#define PIN_OUT 1

/* Valeurs possibles d'un pin. */
#define PIN_LOW 0
#define PIN_HIGH 1

/* Ouvre le répertoire sous /sys/class/gpio/gpioXX d'un pin
   du BeagleBone.
   Retourne -1 en cas d'échec, sinon 0. */
int pin_open (int pin);

/* Définit la direction d'un pin: IN ou OUT.
   Retourne -1 en cas d'échec, sinon 0. */
int pin_set_direction (int pin, int direction);

/* Définit la valeur d'un pin: LOW ou HIGH.
   Retourne -1 en cas d'échec, sinon 0. */
int pin_set_value (int pin, int value);

#endif /* _PIN_H_ INCLUDED */
