// C program to convert the original BLEP and BLAMP data (which are
// B_OVERSAMPLED times oversampled BLEP/BLAMPs) so that it only
// requires simple addition when mixing in with the trivial waveform.
//
//  For the BLEP tables, this means changing the sign of the first
//  half of the table. This avoids having two loops to mix in the impulse
//  center.
//  
//  For the BLAMP tables, the sign of the whole table is changed. There
//  is already a single loop to add in the BLAMP.
//
// Usage: ./blepsign > BlepDataNew.h
//
// Copyright 2024 Ricard Wanderlof
//
// This file may be licensed under the terms of of the
// GNU General Public License Version 2 (the ``GPL'').
//
// Software distributed under the License is distributed
// on an ``AS IS'' basis, WITHOUT WARRANTY OF ANY KIND, either
// express or implied. See the GPL for the specific language
// governing rights and limitations.
//
// You should have received a copy of the GPL along with this
// program. If not, go to http://www.gnu.org/licenses/gpl.html
// or write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

#include <stdio.h>

// Note: This is the _old_ version of BlepData.h, _before_ conversion!
#include "../Engine/BlepData.h"

#define BLEPLEN (2 * Samples)
#define TABLESIZE (BLEPLEN * 64 + 1) // BLEPLEN * B_OVERSAMPLING + 1

void reformat(const char *name, const float *buf, int size, FILE *outfile, int sign_of_second_part)
{
  printf("// %s table: %d entries\n", name, size);
  printf("const float %s[] = {\n\t", name);

  for (int i = 0; i < size / 2; i++) {
    fprintf(outfile, "%ef,", -buf[i]);
    if (((i + 1) % 4) == 0) {
      fprintf(outfile, "\n"); 
      if (((i + 1) % 64) == 0) fprintf(outfile, "\n");
      fprintf(outfile, "\t");
    } else {
      fprintf(outfile, " ");
    }
  }

  for (int i = size / 2; i < size; i++) {
    fprintf(outfile, " %ef,", sign_of_second_part * buf[i]);
    if (((i + 1) % 4) == 0) {
      fprintf(outfile, "\n"); 
      if (((i + 1) % 64) == 0) fprintf(outfile, "\n");
      fprintf(outfile, "\t");
    } else {
      fprintf(outfile, " ");
    }
  }

  fprintf(outfile, "\n};\n");
}


int main(int argc, char **argv)
{
  printf("// Sizeof blep: %d = %d floats\n\n", sizeof blep, sizeof blep / sizeof(float));
  reformat("blep", blep, TABLESIZE, stdout, 1);
  fprintf(stdout, "\n");

  printf("// Sizeof blepd2: %d = %d floats\n\n", sizeof blepd2, sizeof blepd2 / sizeof(float));
  reformat("blepd2", blepd2, TABLESIZE, stdout, 1);
  fprintf(stdout, "\n");


  printf("// Sizeof blamp: %d = %d floats\n\n", sizeof blamp, sizeof blamp / sizeof(float));
  reformat("blamp", blamp, TABLESIZE, stdout, -1);
  fprintf(stdout, "\n");

  printf("// Sizeof blampd2: %d = %d floats\n\n", sizeof blampd2, sizeof blampd2 / sizeof(float));
  reformat("blampd2", blampd2, TABLESIZE, stdout, -1);

  return 0;
}
