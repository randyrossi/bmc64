/*
   plus4emu -- portable Commodore Plus/4 emulator
   Copyright (C) 2003-2008 Istvan Varga <istvanv@users.sourceforge.net>
   http://sourceforge.net/projects/plus4emu/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static const double tabl1[8] = {
  ( 162.8 +  157.5) * 0.5,
  ( 435.9 +  430.6) * 0.5,
  ( 709.0 +  703.3) * 0.5,
  ( 986.7 +  983.1) * 0.5,
  (1273.8 + 1269.5) * 0.5,
  (1560.7 + 1555.0) * 0.5,
  (1850.0 + 1844.5) * 0.5,
  (2118.5 + 2112.7) * 0.5
};

static const double tabl2[8] = {
   321.2,
   874.5,
  1436.3,
  2026.4,
  2633.9,
  3262.5,
  3924.5,
  4570.0
};

static int calcDistortion(int x, double gamma0, double gamma1)
{
  double  xf = (double) x / 300.0;
  double  gamma;
  double  yf;
  xf = (xf > 0.0 ? (xf < 1.0 ? xf : 1.0) : 0.0);
  gamma = gamma0 * pow((gamma1 / gamma0), xf);
  yf = pow(xf, gamma);
  return (int) ((yf * 24576.0) + 0.5);
}

static int calcTEDSound(int x)
{
  switch (x & 48) {
  case 0:
    x = 0;
    break;
  case 16:
  case 32:
    x = (x & 15) * 10 - 4;
    x = (x > 0 ? (x < 75 ? x : 75) : 0);
    break;
  case 48:
    x = (x & 15) * 10 - 4;
    x = (x > 0 ? (x < 75 ? x : 75) : 0);
    x = x * 2;
    break;
  }
  return (x * 2);
}

static double calcError(double gamma0, double gamma1)
{
  double  totalError = 0.0;
  int     i;
  for (i = 0; i < 8; i++) {
    int     x1 = (int) ((tabl1[i] * 24576.0 / tabl2[7]) + 0.5);
    int     x2 = calcDistortion(calcTEDSound(17 + i), gamma0, gamma1);
    double  err = (double) (x2 - x1) / sqrt((double) x1);
    totalError += (err * err);
  }
  for (i = 0; i < 8; i++) {
    int     x1 = (int) ((tabl2[i] * 24576.0 / tabl2[7]) + 0.5);
    int     x2 = calcDistortion(calcTEDSound(49 + i), gamma0, gamma1);
    double  err = (double) (x2 - x1) / sqrt((double) x1);
    totalError += (err * err);
  }
  return totalError;
}

int main(int argc, char **argv)
{
  double  bestError = 1000000000000.0;
  double  bestGamma0 = 0.0;
  double  bestGamma1 = 0.0;
  double  gamma0 = 1.0;
  int     i;
  do {
    double  gamma1 = 1.0;
    do {
      double  err = calcError(gamma0, gamma1);
      if (err < bestError) {
        bestGamma0 = gamma0;
        bestGamma1 = gamma1;
        bestError = err;
      }
      gamma1 = gamma1 + 0.001;
    } while (gamma1 < 1.5);
    gamma0 = gamma0 + 0.001;
  } while (gamma0 < 1.5);
  printf("gamma0 = %f\n", bestGamma0);
  printf("gamma1 = %f\n", bestGamma1);
  printf("err = %f\n", bestError);
  printf("static const int16_t soundVolumeTable[64] = {\n");
  for (i = 0; i < 64; i++) {
    int     tmp2 = calcDistortion(calcTEDSound(i), bestGamma0, bestGamma1);
    if (!(i & 7))
      printf(" ");
    printf("%6d", tmp2);
    if (i != 63)
      printf(",");
    if ((i & 7) == 7)
      printf("\n");
  }
  printf("};\n");
  printf("static const int16_t tedSoundDistortionTable[301] = {\n");
  for (i = 0; i <= 300; i++) {
    int     tmp2 = calcDistortion(i, bestGamma0, bestGamma1);
    if (!(i & 7))
      printf(" ");
    printf("%6d", tmp2);
    if (i != 300)
      printf(",");
    if ((i & 7) == 7 || i == 300)
      printf("\n");
  }
  printf("};\n");
  return 0;
}

