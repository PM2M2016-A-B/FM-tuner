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

#ifndef _FM_TUNER_
#define _FM_TUNER_

#include "rds.h"

/* Intervalle du volume du tuner. */
#define FM_TUNER_VOLUME_MIN 0
#define FM_TUNER_VOLUME_MAX 15

/* Directions possibles du seek. */
#define FM_TUNER_SEEKDOWN 0
#define FM_TUNER_SEEKUP 1

typedef struct Fm_tuner Fm_tuner;

/* Configuration du tuner. */
typedef struct Fm_tuner_conf {
  /* Pins utilisés: /sys/class/gpio/gpioXX/ */
  int pin_sdio;
  int pin_rst;

  /* Identifiant du bus relié au tuner. */
  int i2c_id;

  /* Adresse du tuner. */
  int tuner_addr;
} Fm_tuner_conf;

/* Crée et donne l'accès à un tuner. */
Fm_tuner *fm_tuner_new (Fm_tuner_conf *conf);

/* Libère un tuner. */
void fm_tuner_free (Fm_tuner *fm_tuner);

/* Ecrit les registres contenus dans fm_tuner sur le tuner physique.
   Retourne -1 en cas d'échec, sinon 0. */
int fm_tuner_write_registers (Fm_tuner *fm_tuner);

/* Lit les registres contenus dans le tuner physique et stocke
   leurs valeurs dans fm_tuner.
   Retourne -1 en cas d'échec, sinon 0. */
int fm_tuner_read_registers (Fm_tuner *fm_tuner);

/* Affiche le contenu des registres d'un tuner sur la
   sortie standard. */
void fm_tuner_print_registers (Fm_tuner *fm_tuner);

/* Règle le volume d'un tuner dans l'intervalle
   [ FM_TUNER_VOLUME_MIN, FM_TUNER_VOLUME_MAX ].
   Retourne -1 en cas d'échec, sinon le volume. */
int fm_tuner_set_volume (Fm_tuner *fm_tuner, int volume);

/* Donne la valeur actuelle du volume du tuner.
   Retourne -1 en cas d'échec, sinon le volume. */
int fm_tuner_get_volume (Fm_tuner *fm_tuner);

/* Règle le channel du tuner.
   Retourne -1 en cas d'échec, sinon le channel. */
int fm_tuner_set_channel (Fm_tuner *fm_tuner, int channel);

/* Donne le channel actuelle du tuner.
   Retourne -1 en cas d'échec, sinon le channel. */
int fm_tuner_get_channel (Fm_tuner *fm_tuner);

/* Change de station (gauche/droite).
   success est utilisé pour indiqué si le changement
   de station a pu se faire. Retourne -1 en cas d'échec,
   sinon le channel. */
int fm_tuner_seek (Fm_tuner *fm_tuner, int direction, int *success);

/* Stocke dans blocks des données rds si elles existent.
   Dans le cas où elles existent, data_exists vaut 1 sinon 0.
   Retourne -1 en cas d'échec, sinon 0. */
int fm_tuner_read_rds (Fm_tuner *fm_tuner, uint16_t blocks[static RDS_BLOCKS_N], int *data_exists);

/* Retourne le RSSI actuel ou -1 en cas d'erreur.
   Max: 75dBuV. */
int fm_tuner_get_rssi (Fm_tuner *fm_tuner);

#endif /* _FM_TUNER_ INCLUDED */
