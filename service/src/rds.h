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

/* Type de données transmises. */
#define RDS_DATA_TYPE_MUSIC 0
#define RDS_DATA_TYPE_TRAFFIC 1
#define RDS_DATA_TYPE_SPEECH 2

/* Type de programme. */
#define RDS_PT_NONE 0x00
#define RDS_PT_NEWS 0x01
#define RDS_PT_INFORM 0x02
#define RDS_PT_SPORTS 0x03
#define RDS_PT_TALK 0x04
#define RDS_PT_ROCK 0x05
#define RDS_PT_CLASSIC_ROCK 0x06
#define RDS_PT_ADULT_HITS 0x07
#define RDS_PT_SOFT_ROCK 0x08
#define RDS_PT_TOP_40 0x09
#define RDS_PT_COUNTRY 0x0A
#define RDS_PT_OLDIES 0x0B
#define RDS_PT_SOFT 0x0C
#define RDS_PT_NOSTALGIA 0x0D
#define RDS_PT_JAZZ 0x0E
#define RDS_PT_CLASSICAL 0x0F
#define RDS_PT_RYTHM_AND_BLUES 0x10
#define RDS_PT_SOFT_RYTHM_AND_BLUES 0x11
#define RDS_PT_FOREIGN_LANGUAGE 0x12
#define RDS_PT_RELIGIOUS_MUSIC 0x13
#define RDS_PT_RELIGIOUS_TALK 0x14
#define RDS_PT_PERSONALITY 0x15
#define RDS_PT_PUBLIC 0x16
#define RDS_PT_COLLEGE 0x17
#define RDS_PT_WEATHER 0x1D
#define RDS_PT_EMERGENCY_TEST 0x1E
#define RDS_PT_EMERGENCY 0X1F
#define RDS_PT_UNKNOWN 0x18

typedef struct Rds Rds;

/* Crée un objet Rds. */
Rds *rds_new (void);

/* Libère un objet Rds. */
void rds_free (Rds *rds);

/* Décode le contenu de blocks RDS et le stocke dans rds. */
void rds_decode (Rds *rds, uint16_t blocks[static 4]);

/* Retourne le type de données: MUSIC, TRAFFIC ou SPEECH. */
int rds_get_data_type (Rds *rds);

/* Donne un nom/le texte de radio actuellement en mémoire. */
const char *rds_get_radio_name (Rds *rds);
const char *rds_get_radio_text (Rds *rds);

/* Retourne le type de programme. */
int rds_get_program_type (Rds *rds);

#endif /* _RDS_H_ INCLUDED */
