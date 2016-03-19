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

#ifndef _LED_H_
#define _LED_H_

/* Nombre de leds du BeagleBone Black. */
#define LEDS_N 4

/* Etats possibles d'une led. */
#define LED_ACTIVE 0
#define LED_INACTIVE 1

/* Active/Désactive une led de la carte.
   Retourne -1 en cas d'échec, sinon 0. */
int led_set_state (int id, int state);

#endif /* _LED_H_ INCLUDED */
