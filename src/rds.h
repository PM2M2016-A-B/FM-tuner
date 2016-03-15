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

typedef struct Rds Rds;

/* Crée un objet Rds. */
Rds *rds_new (void);

/* Libère un objet Rds. */
void rds_free (Rds *rds);

/* Décode le contenu de blocks RDS et le stocke dans rds. */
void rds_decode (Rds *rds, uint16_t blocks[]);

int rds_get_data_type (Rds *rds);
const char *rds_get_radio_name (Rds *rds);
const char *rds_get_radio_text (Rds *rds);

#endif /* _RDS_H_ INCLUDED */
