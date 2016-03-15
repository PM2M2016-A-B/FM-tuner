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

#include <stdio.h>
#include <string.h>

#include "utils/error.h"

#include "rds.h"

#define RDSA 0
#define RDSB 1
#define RDSC 2
#define RDSD 3

#define BIT_NAME 0
#define BIT_TEXT 2
#define BIT_MS 6
#define BIT_TA 7
#define BIT_TP 8

#define MASK_NAME 0x03
#define MASK_TEXT 0x3C
#define MASK_MS (1 << BIT_MS)
#define MASK_TA (1 << BIT_TA)
#define MASK_TP (1 << BIT_TP)

void rds_init (Rds *rds) {
  memset(rds, 0, sizeof *rds);
  return;
}

static inline void __get_group_type (uint16_t blocks[], int *id, int *version) {
  *id = (blocks[RDSB] & 0xF000) >> 12;
  *version = !!(blocks[RDSB] & 0x0800);

  return;
}

static inline void __decode_basic_tuning_and_switching_info(Rds *rds, uint16_t blocks[]) {
  int off = blocks[RDSB] & 0x0003;
  int chars;

  rds->bit_fields &= ~MASK_MS;
  rds->bit_fields |= (!!(blocks[RDSB] & 0x0008)) << BIT_MS;
  rds->bit_fields &= ~MASK_TA;
  rds->bit_fields |= (!!(blocks[RDSB] & 0x0010)) << BIT_TA;

  if ((rds->bit_fields & MASK_NAME) >> BIT_NAME != off) {
    rds->bit_fields &= ~MASK_NAME;
    return;
  }

  chars = off * 2;
  rds->new_radio_name[chars] = (blocks[RDSD] & 0xFF00) >> 8;
  rds->new_radio_name[chars + 1] = blocks[RDSD] & 0x00FF;

  if (off < 3) {
    rds->bit_fields &= ~MASK_NAME;
    rds->bit_fields |= (off + 1) << BIT_NAME;
    return;
  }

  #ifdef DEBUG
    if (strcmp(rds->radio_name, rds->new_radio_name))
      debug("Radio name: '%s'\n", rds->new_radio_name);
  #endif

  rds->bit_fields &= ~MASK_NAME;
  memcpy(rds->radio_name, rds->new_radio_name, RDS_RADIO_NAME_MAX_LENGTH);
  memset(rds->new_radio_name, 0, RDS_RADIO_NAME_MAX_LENGTH);

  return;
}

static inline void __decode_radio_text (Rds *rds, uint16_t blocks[], int version) {
  int off = blocks[RDSB] & 0x000F;
  int chars;
  int i, completed;

  if ((rds->bit_fields & MASK_TEXT) >> BIT_TEXT != off) {
    rds->bit_fields &= ~MASK_TEXT;
    return;
  }

  chars = off * (!version + 1) * 2;

  if (!version) {
    rds->new_radio_text[chars] = (blocks[RDSC] & 0xFF00) >> 8;
    rds->new_radio_text[chars + 1] = blocks[RDSC] & 0x00FF;
    rds->new_radio_text[chars + 2] = (blocks[RDSD] & 0xFF00) >> 8;
    rds->new_radio_text[chars + 3] = blocks[RDSD] & 0x00FF;
  }
  else {
    rds->new_radio_text[chars] = (blocks[RDSD] & 0xFF00) >> 8;
    rds->new_radio_text[chars + 1] = blocks[RDSD] & 0x00FF;
  }

  for (i = 0; i < 4; i++)
    if ((completed = rds->new_radio_text[chars + i]) == 13) {
      rds->new_radio_text[chars + i] = '\0';
      break;
    }

  if (off < 15) {
    rds->bit_fields &= ~MASK_TEXT;
    rds->bit_fields |= (off + 1) << BIT_TEXT;
    return;
  }

  #ifdef DEBUG
    if (strcmp(rds->radio_text, rds->new_radio_text))
      debug("Radio text: '%s'\n", rds->new_radio_text);
  #endif

  rds->bit_fields &= ~MASK_TEXT;
  memcpy(rds->radio_text, rds->new_radio_text, RDS_RADIO_TEXT_MAX_LENGTH);
  memset(rds->new_radio_text, 0, RDS_RADIO_TEXT_MAX_LENGTH);

  return;
}

void rds_decode (Rds *rds, uint16_t blocks[]) {
  int id, version;

  __get_group_type(blocks, &id, &version);
  rds->bit_fields &= ~MASK_TP;
  rds->bit_fields |= (!!(blocks[RDSB] & 0x0800)) << BIT_TP;

  printf("TYPE %d%c\n", id, !version ? 'A' : 'B');

  switch (id) {
    case 0:
      __decode_basic_tuning_and_switching_info(rds, blocks);
      break;
    case 2:
      __decode_radio_text(rds, blocks, version);
      break;
    default:
      debug("Unsupported group type: %d%c.\n", id, !version ? 'A' : 'B');
  }

  return;
}

int rds_get_data_type (Rds *rds) {
  if (rds->bit_fields & (MASK_TA | MASK_TP))
    return RDS_DATA_TYPE_TRAFFIC;
  if (rds->bit_fields & (MASK_MS))
    return RDS_DATA_TYPE_MUSIC;

  return RDS_DATA_TYPE_SPEECH;
}
