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

#include <stdint.h>

/* Nombre de registres du tuner. */
#define FM_TUNER_REGISTERS_N 16

/* Taille en bytes d'un registre du tuner. */
#define FM_TUNER_REGISTER_SIZE 2

/* Registres du tuner. */
#define REG_DEVICEID 0x00
#define REG_CHIPID  0x01
#define REG_POWERCFG 0x02
#define REG_CHANNEL 0x03
#define REG_SYSCONFIG1 0x04
#define REG_SYSCONFIG2 0x05
#define REG_SYSCONFIG3 0x06
#define REG_TEST1 0x07
#define REG_TEST2 0x08
#define REG_BOOTCONFIG 0x09
#define REG_STATUSRSSI 0x0A
#define REG_READCHAN 0x0B
#define REG_RDSA 0x0C
#define REG_RDSB 0x0D
#define REG_RDSC 0x0E
#define REG_RDSD 0x0F

/* Intervalle du volume du tuner. */
#define FM_TUNER_VOLUME_MIN 0
#define FM_TUNER_VOLUME_MAX 15

typedef struct Fm_tuner {
  int bus;
  uint16_t regs[FM_TUNER_REGISTERS_N];
} Fm_tuner;

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

/* Initialise et donne l'accès à un tuner.
   Retourne -1 en cas d'échec, sinon 0. */
int fm_tuner_init (Fm_tuner *fm_tuner, Fm_tuner_conf *conf);

/* Ferme l'accès à un tuner.
   Retourne -1 en cas d'échec, sinon 0. */
int fm_tuner_close (Fm_tuner *fm_tuner);

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
   Retourne -1 en cas d'échec, sinon 0. */
int fm_tuner_set_volume (Fm_tuner *fm_tuner, int volume);

/* Donne la valeur actuelle du volume du tuner.
   Retourne -1 en cas d'échec, sinon le volume. */
int fm_tuner_get_volume (Fm_tuner *fm_tuner);

/* Règle la channel du tuner.
   Retourne -1 en cas d'échec, sinon 0. */
int fm_tuner_set_channel (Fm_tuner *fm_tuner, int channel);

/* Donne la channel actuelle du tuner.
   Retourne -1 en cas d'échec, sinon la channel. */
int fm_tuner_get_channel (Fm_tuner *fm_tuner);

/* Stocke dans blocks des données rds si elles existent.
   Dans le cas où elles existent, data_exists vaut 1 sinon 0.
   Retourne -1 en cas d'échec, sinon 0. */
int fm_tuner_read_rds (Fm_tuner *fm_tuner, uint16_t blocks[], int *data_exists);

#endif /* _FM_TUNER_ INCLUDED */
