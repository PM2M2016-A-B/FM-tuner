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

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

#include "hw/i2c.h"
#include "hw/pin.h"
#include "utils/error.h"
#include "utils/pmath.h"
#include "utils/ptime.h"

#include "fm_tuner.h"

#if defined EUROPE_VERSION && defined AMERICAN_VERSION
  #error "Two defined program versions."
#elif defined EUROPE_VERSION || defined AMERICAN_VERSION
  /* Yeah! */
#else
  #error "Undefined program version."
#endif

#define MASK_CHANNEL 0x03FF
#define MASK_DE_EMPHASIS 0x0800
#define MASK_ENABLE_RDS 0x1000
#define MASK_STC 0x4000
#define MASK_TEST_RDS 0x8000
#define MASK_TUNE 0x8000
#define MASK_VOLUME 0x000F

#define CHANNEL_OFFSET 875

#define VAL_OSCILLATOR 0x8100
#define VAL_POWER_ON 0x4001
#define VAL_POWER_OFF 0x0041

static inline void __set_volume(Fm_tuner *fm_tuner, int volume) {
  /* Suppose que volume est dans l'intervalle
     [ FM_TUNER_VOLUME_MIN, FM_TUNER_VOLUME_MAX ]. */
  fm_tuner->regs[REG_SYSCONFIG2] &= ~MASK_VOLUME;
  fm_tuner->regs[REG_SYSCONFIG2] |= volume;

  return;
}

/* Documentation: "doc/AN230.pdf", page 12. */
int fm_tuner_init (Fm_tuner *fm_tuner, Fm_tuner_conf *conf) {
  int pins[] = { conf->pin_rst, conf->pin_sdio };
  int i;

  /* Ouverture des pins en mode OUT et LOW. */
  for (i = 0; i < 2; i++) {
    if (pin_open(pins[i]) == -1)
      return error("Unable to open the %d pin.", pins[i]);

    if (pin_set_direction(pins[i], PIN_OUT) == -1)
      return error("Unable to set the direction of the %d pin.", pins[i]);

    if (pin_set_value(pins[i], PIN_LOW) == -1)
      return error("Unable to set the value of the %d pin.", pins[i]);
  }

  sleep_m(1);

  /* Reset du tuner. */
  if (pin_set_value(conf->pin_rst, PIN_HIGH) == -1)
    return error("Unable to reset the fm tuner.");

  sleep_m(1);

  /* Accès au bus. */
  if ((fm_tuner->bus = i2c_open(conf->i2c_id, conf->tuner_addr)) == -1)
    return error("Unable to open the bus.");

  if (fm_tuner_read_registers(fm_tuner) == -1)
    goto err;

  /* Activation de l'oscillateur. */
  fm_tuner->regs[REG_TEST1] = VAL_OSCILLATOR;

  if (fm_tuner_write_registers(fm_tuner) == -1)
    goto err;

  sleep_m(500);

  if (fm_tuner_read_registers(fm_tuner) == -1)
    goto err;

  /* Power up. */
  fm_tuner->regs[REG_POWERCFG] = VAL_POWER_ON;

  #ifdef EUROPE_VERSION
    fm_tuner->regs[REG_SYSCONFIG1] |= MASK_DE_EMPHASIS;
    fm_tuner->regs[REG_SYSCONFIG2] |= 0x0010;
  #endif

  /* Activation RDS. */
  fm_tuner->regs[REG_SYSCONFIG1] |= MASK_ENABLE_RDS;

  /* Volume au minimum. */
  __set_volume(fm_tuner, 1);

  if (fm_tuner_write_registers(fm_tuner) == -1)
    goto err;

  sleep_m(110);
  return 0;

 err:
  i2c_close(fm_tuner->bus);
  return -1;
}

/* Documentation: "doc/AN230.pdf", page 13. */
int fm_tuner_close (Fm_tuner *fm_tuner) {
  if (fm_tuner_read_registers(fm_tuner) == -1)
    return -1;

  fm_tuner->regs[REG_POWERCFG] = VAL_POWER_OFF;

  if (fm_tuner_write_registers(fm_tuner) == -1)
    return -1;

  if (i2c_close(fm_tuner->bus) == -1)
    return error("Unable to close the bus.");

  return 0;
}

/* Documentation: "doc/Si4702-03-C19-1.pdf", page 19. */
int fm_tuner_write_registers (Fm_tuner *fm_tuner) {
  const ssize_t size = 6 * FM_TUNER_REGISTER_SIZE;
  uint16_t regs[6];
  int i, j;

  /* Ecriture des registres 0x02 à 0x07. */
  for (i = 0x02, j = 0; i <= 0x07; i++, j++) {
    regs[j] = htons(fm_tuner->regs[i]);
  }

  if (i2c_write(fm_tuner->bus, (void *)regs, size) != size) {
    error("Unable to write registers.");
    return -1;
  }

  return 0;
}

/* Documentation: "doc/Si4702-03-C19-1.pdf", page 18. */
int fm_tuner_read_registers (Fm_tuner *fm_tuner) {
  const ssize_t size = FM_TUNER_REGISTERS_N * FM_TUNER_REGISTER_SIZE;
  uint16_t regs[FM_TUNER_REGISTERS_N];
  int i, j;

  /* Lecture de tous les registres, de 0x0A à 0x0F puis de 0x00 à 0x09. */
  if (i2c_read(fm_tuner->bus, (void *)regs, size) != size) {
    error("Unable to read registers.");
    return -1;
  }

  /* Attention: données en entrée en big-endian ! */
  for (i = 0x0A, j = 0; i <= 0x0F; i++, j++)
    fm_tuner->regs[i] = ntohs(regs[j]);

  for (i = 0x00; i <= 0x09; i++, j++)
    fm_tuner->regs[i] = ntohs(regs[j]);

  return 0;
}

void fm_tuner_print_registers (Fm_tuner *fm_tuner) {
  static const char *reg_names[] = {
    "DEVICEID  ", "CHIPID    ", "POWERCFG  ", "CHANNEL   ",
    "SYSCONFIG1", "SYSCONFIG2", "SYSCONFIG3", "TEST1     ",
    "TEST2     ", "BOOTCONFIG", "STATUSRSSI", "READCHAN  ",
    "RDSA      ", "RDSB      ", "RDSC      ", "RDSD      "
  };
  static char buf[8 * FM_TUNER_REGISTER_SIZE + 1];
  int i;

  for (i = 0; i < FM_TUNER_REGISTERS_N; i++)
    printf("%s    0x%04X    0b%s\n", reg_names[i], fm_tuner->regs[i],
           bytes_to_binary(fm_tuner->regs[i], FM_TUNER_REGISTER_SIZE, buf));

  return;
}

/* Documentation: "doc/Si4702-03-C19-1.pdf", page 28. */
int fm_tuner_set_volume (Fm_tuner *fm_tuner, int volume) {
  if (fm_tuner_read_registers(fm_tuner) == -1)
    return -1;

  /* Clamping du volume. */
  if (volume > FM_TUNER_VOLUME_MAX)
    volume = FM_TUNER_VOLUME_MAX;
  else if (volume < FM_TUNER_VOLUME_MIN)
    volume = FM_TUNER_VOLUME_MIN;

  /* Mise à jour du volume.*/
  __set_volume(fm_tuner, volume);

  return fm_tuner_write_registers(fm_tuner);
}

/* Documentation: "doc/Si4702-03-C19-1.pdf", page 28. */
int fm_tuner_get_volume (Fm_tuner *fm_tuner) {
  if (fm_tuner_read_registers(fm_tuner) == -1)
    return -1;

  return fm_tuner->regs[REG_SYSCONFIG2] & MASK_VOLUME;
}

/* Documentation: "doc/AN230.pdf", page 22. */
int fm_tuner_set_channel (Fm_tuner *fm_tuner, int channel) {
  channel = channel - CHANNEL_OFFSET;

  #ifdef AMERICAN_VERSION
    channel /= 2;
  #endif

  channel &= MASK_CHANNEL;

  if (fm_tuner_read_registers(fm_tuner) == -1)
    return -1;

  /* Ecriture du channel choisi. */
  fm_tuner->regs[REG_CHANNEL] &= ~MASK_CHANNEL;
  fm_tuner->regs[REG_CHANNEL] |= channel;

  /* Mise à 1 du bit TUNE. */
  fm_tuner->regs[REG_CHANNEL] |= MASK_TUNE;

  if (fm_tuner_write_registers(fm_tuner) == -1)
    return -1;

  /* On attend que le tune soit pris en compte. */
  for (;;) {
    if (fm_tuner_read_registers(fm_tuner) == -1)
      return -1;
    if (fm_tuner->regs[REG_STATUSRSSI] & MASK_STC)
      break;

    sleep_m(10);
  }

  /* Remise à 0 du bit TUNE. */
  fm_tuner->regs[REG_CHANNEL] &= ~MASK_TUNE;

  if (fm_tuner_write_registers(fm_tuner) == -1)
    return -1;

  for (;;) {
    if (fm_tuner_read_registers(fm_tuner) == -1)
      return -1;
    if (!(fm_tuner->regs[REG_STATUSRSSI] & MASK_STC))
      break;

    sleep_m(10);
  }

  return 0;
}

/* Documentation: "doc/AN230.pdf", page 22. */
int fm_tuner_get_channel (Fm_tuner *fm_tuner) {
  int channel;

  if (fm_tuner_read_registers(fm_tuner) == -1)
    return -1;

  channel = fm_tuner->regs[REG_READCHAN] & MASK_CHANNEL;

  #ifdef AMERICAN_VERSION
    channel *= 2;
  #endif

  return channel + CHANNEL_OFFSET;
}

int fm_tuner_read_rds (Fm_tuner *fm_tuner, uint16_t blocks[4], int *data_exists) {
  int i;

  if (fm_tuner_read_registers(fm_tuner) == -1)
    return -1;

  if (fm_tuner->regs[REG_STATUSRSSI] & MASK_TEST_RDS) {
    for (i = 0; i < 4; i++)
      blocks[i] = fm_tuner->regs[REG_RDSA + i];

    *data_exists = 1;
  }
  else
    *data_exists = 0;

  return 0;
}
