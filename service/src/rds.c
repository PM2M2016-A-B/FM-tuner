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

#include <string.h>

#include "utils/alloc.h"
#include "utils/error.h"

#include "rds.h"

#define RDSA 0
#define RDSB 1
#define RDSC 2
#define RDSD 3

#define BIT_GROUP_ID 12
#define BIT_PT 5

/* Masks relatifs à la norme RDS. */
#define MASK_GROUP_ID 0xF000
#define MASK_GROUP_VERSION 0x0800

#define MASK_PSNAME_PART 0x0003
#define MASK_RADIO_TEXT_PART 0x000F

#define MASK_MS 0x0008 /* Music/Speech. */
#define MASK_TA 0x0010 /* Traffic Announcement. */
#define MASK_TP 0x0400 /* Traffic Program. */
#define MASK_PT 0x03E0 /* Program Type. */

/* Bits et masks relatifs à l'attribut bit_fields
   de la structure RDS. */
#define ST_BIT_NAME 0 /* Length: 2. */
#define ST_BIT_TEXT 2 /* Length: 4. */
#define ST_BIT_PT 6   /* Length: 4. */
#define ST_BIT_MS 10  /* Length: 1. */
#define ST_BIT_TA 11  /* Length: 1. */
#define ST_BIT_TP 12  /* Length: 1. */

#define ST_MASK_NAME 0x0003
#define ST_MASK_TEXT 0x003C
#define ST_MASK_PT 0x03C0
#define ST_MASK_MS (1 << ST_BIT_MS)
#define ST_MASK_TA (1 << ST_BIT_TA)
#define ST_MASK_TP (1 << ST_BIT_TP)

/* Fin de ligne. */
#define RDS_CARRIAGE_RETURN 13

struct Rds {
  char radio_name[RDS_RADIO_NAME_MAX_LENGTH + 1]; /* Nom actuel de la radio. */
  char new_radio_name[RDS_RADIO_NAME_MAX_LENGTH + 1]; /* Nom de la radio en cours de parsing. */

  char radio_text[RDS_RADIO_TEXT_MAX_LENGTH + 1]; /* Texte actuel de la radio. */
  char new_radio_text[RDS_RADIO_TEXT_MAX_LENGTH + 1]; /* Texte de la radio en cours de parsing. */

  uint16_t bit_fields; /* Diverses informations RDS. */
};

Rds *rds_new (void) {
  return pnew0(Rds);
}

void rds_free (Rds *rds) {
  if (rds != NULL)
    free(rds);

  return;
}

static void __get_group_type (uint16_t blocks[], int *id, int *version) {
  *id = (blocks[RDSB] & MASK_GROUP_ID) >> BIT_GROUP_ID;
  *version = !!(blocks[RDSB] & MASK_GROUP_VERSION);

  return;
}

static void __decode_basic_tuning_and_switching_info(Rds *rds, uint16_t blocks[]) {
  int off = blocks[RDSB] & MASK_PSNAME_PART;
  int chars;

  #ifdef DEBUG
    int i;
  #endif

  /* Récupération des flags Music/Speech et Traffic Annoucement. */
  rds->bit_fields &= ~ST_MASK_MS;
  rds->bit_fields |= (!!(blocks[RDSB] & MASK_MS)) << ST_BIT_MS;
  rds->bit_fields &= ~ST_MASK_TA;
  rds->bit_fields |= (!!(blocks[RDSB] & MASK_TA)) << ST_BIT_TA;

  /* Vérification de la position de l'offset du nom de la radio.
     S'il n'est pas bon, on reset et on attend une prochaine séquence. */
  if ((rds->bit_fields & ST_MASK_NAME) >> ST_BIT_NAME != off) {
    rds->bit_fields &= ~ST_MASK_NAME;
    return;
  }

  /* Récupération de 2 lettres contenues dans le nom de la radio. */
  chars = off * 2;
  rds->new_radio_name[chars] = (blocks[RDSD] & 0xFF00) >> 8;
  rds->new_radio_name[chars + 1] = blocks[RDSD] & 0x00FF;

  /* Le nom n'est pas complet, on met à jour l'offset. */
  if (off < 3) {
    rds->bit_fields &= ~ST_MASK_NAME;
    rds->bit_fields |= (off + 1) << ST_BIT_NAME;
    return;
  }

  /* Le nom est complet. */
  #ifdef DEBUG
    if (strcmp(rds->radio_name, rds->new_radio_name)) {
      debug("[rds]Radio name: '%s' (", rds->new_radio_name);

      for (i = 0; i < RDS_RADIO_NAME_MAX_LENGTH; i++)
        debug("%d, ", rds->new_radio_name[i]);

      debug("0)\n");
    }
  #endif

  rds->bit_fields &= ~ST_MASK_NAME;
  memcpy(rds->radio_name, rds->new_radio_name, RDS_RADIO_NAME_MAX_LENGTH);
  memset(rds->new_radio_name, 0, RDS_RADIO_NAME_MAX_LENGTH);

  return;
}

static void __decode_radio_text (Rds *rds, uint16_t blocks[], int version) {
  int off = blocks[RDSB] & MASK_RADIO_TEXT_PART;
  int chars;
  int i, n = (!version + 1) * 2;

  /* Vérification de la position de l'offset du text de la radio.
     S'il n'est pas bon, on reset et on attend une prochaine séquence. */
  if ((rds->bit_fields & ST_MASK_TEXT) >> ST_BIT_TEXT != off) {
    rds->bit_fields &= ~ST_MASK_TEXT;
    return;
  }

  /* Attention: 2x plus de lettres transmises en 1 message pour la version A. */
  chars = off * n;

  /* Récupération de 4 lettres pour la version A et de 2 pour la version B. */
  if (n == 4) {
    rds->new_radio_text[chars] = (blocks[RDSC] & 0xFF00) >> 8;
    rds->new_radio_text[chars + 1] = blocks[RDSC] & 0x00FF;
    rds->new_radio_text[chars + 2] = (blocks[RDSD] & 0xFF00) >> 8;
    rds->new_radio_text[chars + 3] = blocks[RDSD] & 0x00FF;
  }
  /* n = 2. */
  else {
    rds->new_radio_text[chars] = (blocks[RDSD] & 0xFF00) >> 8;
    rds->new_radio_text[chars + 1] = blocks[RDSD] & 0x00FF;
  }

  for (i = 0; i < n; i++)
    if (rds->new_radio_text[chars + i] == RDS_CARRIAGE_RETURN) {
      rds->new_radio_text[chars + i] = '\0';
      goto text_completed;
    }

  /* Le nom n'est pas complet, on met à jour l'offset. */
  if (off < 15) {
    rds->bit_fields &= ~ST_MASK_TEXT;
    rds->bit_fields |= (off + 1) << ST_BIT_TEXT;
    return;
  }

 text_completed:

  /* Le nom est complet. */
  #ifdef DEBUG
    if (strcmp(rds->radio_text, rds->new_radio_text)) {
      debug("[rds]Radio text: '%s' (", rds->new_radio_text);

      for (i = 0; i < RDS_RADIO_TEXT_MAX_LENGTH; i++)
        debug("%d, ", rds->new_radio_text[i]);

      debug("0)\n");
    }
  #endif

  rds->bit_fields &= ~ST_MASK_TEXT;
  memcpy(rds->radio_text, rds->new_radio_text, RDS_RADIO_TEXT_MAX_LENGTH);
  memset(rds->new_radio_text, 0, RDS_RADIO_TEXT_MAX_LENGTH);

  return;
}

void rds_decode (Rds *rds, uint16_t blocks[static RDS_BLOCKS_N]) {
  int id, version;

  __get_group_type(blocks, &id, &version);

  /* Récupération du flag Traffic Program.
     Si actif, la radio diffuse des infos routières si le flag TA l'est aussi. */
  rds->bit_fields &= ~ST_MASK_TP;
  rds->bit_fields |= (!!(blocks[RDSB] & MASK_TP)) << ST_BIT_TP;

  /* Récupération du Program Type. */
  rds->bit_fields &= ~ST_MASK_PT;
  rds->bit_fields |= ((blocks[RDSB] & MASK_PT) >> BIT_PT) << ST_BIT_PT;

  switch (id) {
    case 0:
      __decode_basic_tuning_and_switching_info(rds, blocks);
      break;
    case 2:
      __decode_radio_text(rds, blocks, version);
      break;
    default:
      printf("[rds]Unsupported group type: %d%c.\n", id, !version ? 'A' : 'B');
  }

  return;
}

int rds_get_data_type (Rds *rds) {
  if ((rds->bit_fields & (ST_MASK_TA | ST_MASK_TP)) == (ST_MASK_TA | ST_MASK_TP))
    return RDS_DATA_TYPE_TRAFFIC;
  if (rds->bit_fields & ST_MASK_MS)
    return RDS_DATA_TYPE_MUSIC;

  return RDS_DATA_TYPE_SPEECH;
}

const char *rds_get_radio_name (Rds *rds) {
  return rds->radio_name;
}

const char *rds_get_radio_text (Rds *rds) {
  return rds->radio_text;
}

int rds_get_program_type (Rds *rds) {
  int pt = (rds->bit_fields & ST_MASK_PT) >> ST_BIT_PT;

  if (pt > RDS_PT_COLLEGE && pt < RDS_PT_WEATHER)
    return RDS_PT_UNKNOWN;

  return pt;
}
