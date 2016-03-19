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

#ifndef _ERROR_H_
#define _ERROR_H_

/* Affiche un message d'erreur sur stderr ainsi que errno,
   puis quitte le programme. */
void fatal_error (const char *msg, ...);

/* Affiche un message d'erreur sur stderr ainsi que errno.
   Retourne toujours -1. */
int error (const char *msg, ...);

#ifdef DEBUG
  /* Affiche un message de debug sur stderr. */
  void debug (const char *format, ...);
#else
  #define debug()
#endif

#endif /* _ERROR_H_ INCLUDED */
