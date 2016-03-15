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

#ifndef _RDS_H_
#define _RDS_H_

#include <stdint.h>

#define RDS_RADIO_NAME_MAX_LENGTH 8
#define RDS_RADIO_TEXT_MAX_LENGTH 64

#define RDS_DATA_TYPE_MUSIC 0
#define RDS_DATA_TYPE_TRAFFIC 1
#define RDS_DATA_TYPE_SPEECH 2

typedef struct Rds {
  char radio_name[RDS_RADIO_NAME_MAX_LENGTH + 1]; /* Nom actuel de la radio. */
  char new_radio_name[RDS_RADIO_NAME_MAX_LENGTH + 1]; /* Nom de la radio en cours de parsing. */

  char radio_text[RDS_RADIO_TEXT_MAX_LENGTH + 1]; /* Texte actuel de la radio. */
  char new_radio_text[RDS_RADIO_TEXT_MAX_LENGTH + 1]; /* Texte de la radio en cours de parsing. */

  uint16_t bit_fields;
} Rds;

/* Initialise les valeurs par défaut d'une structure Rds. */
void rds_init (Rds *rds);

void rds_decode (Rds *rds, uint16_t blocks[]);

int rds_get_data_type (Rds *rds);

#endif /* _RDS_H_ INCLUDED */
