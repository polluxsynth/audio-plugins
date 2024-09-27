// C program to convert the original BLEP and BLAMP data (which are
// B_OVERSAMPLED times oversampled BLEP/BLAMPs) to be more
// localized given the usage, stressing the cache less.
//
// We also take the opportunity to change the sign of selected
// parts of that data, so that it only requires simple addition
// when mixing in with the trivial waveform.
//
// For the BLEP tables, this means changing the sign of the first
// half of the table. This avoids having two loops to mix in the impulse
// center.
//
// For the BLAMP tables, the sign of the whole table is changed. There
// is already a single loop to add in the BLAMP.
//
// Usage: ./blepnew > BlepDataNew.h
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
#define TABLESIZE (BLEPLEN * (64 + 1)) // BLEPLEN * (B_OVERSAMPLING + 1)

void reformat(const char *name, const float *buf, int size, FILE *outfile, int blep)
{
  printf("// %s table: %d entries\n", name, size);
  printf("const float %s[] = {\n", name);
  for (int i = 0; i < B_OVERSAMPLING; i++) {
    for (int j = 0; j < BLEPLEN; j++) {
      // For BLEPs the sign is changed for the first half of the table
      // For BLAMPs the sign of the whole table is changed
      int sign = blep ? ((j < BLEPLEN / 2) ? -1 : 1) : -1;
      fprintf(outfile, "\t%ef,\n", sign * buf[i + j  * B_OVERSAMPLING]);
    }
    fprintf(outfile, "\n");
  }
  // In the original table, there's an extra entry to act as an adjacent
  // sample for interpolating against when accessing the final entry.
  // Here due to the reformatting we need to have a whole BLEPLEN of entries,
  // which is actually a repeat of the first subsampled blep, but shifted
  // one step up, skipping the first entry and bringing in the final
  // entry from the original table.
  for (int i = 0; i < BLEPLEN; i++) {
    int sign = blep ? (((i + 1) < BLEPLEN / 2) ? -1 : 1) : -1;
    fprintf(outfile, "\t%ef", sign * buf[(i + 1) * B_OVERSAMPLING]);
    if (i != BLEPLEN - 1) fprintf(outfile, ",");
    fprintf(outfile, "\n");
  }
  printf("};\n");
}

int main(int argc, char **argv)
{
  printf("// Rather than simply store the bleps and blamps directly in their\n"
         "// oversampled form, we note that when in use they are subsampled\n"
         "// by B_OVERSAMPLING, thus when reading out the waveform, we read\n"
         "// every B_OVERSAMPLING:th sample, linearly interpolated with the\n"
         "// adjacent sample. This essentially needs the whole table in\n"
         "// the cache every time. To optimize this, instead store the waveform\n"
         "// so that each subsampled waveform is linearly accessible, with the\n"
         "// adjacent subsampled waveform to be used for interpolation being\n"
         "// a mere 2 * Samples entries away. Thus, every time the table is\n"
         "// used, we need roughly 2 * 2 * Samples = 64 entries in the cache\n"
         "// rather than the whole table of 2048 entries.\n"
         "// A small downside is that instead of an extra entry to act as an\n"
         "// interpolation neighbor for the final waveform point, we need\n"
         "// a whole 2 * Samples worth of them.\n//\n"
         "// Additionally, we store the data in a form that will require\n"
         "// simple addition when mixing with the trivial waveform, rather\n"
         "// than requiring a mixture of additions and subtractions\n");
 
  printf("// Sizeof blep: %d = %d floats\n\n", sizeof blep, sizeof blep / sizeof(float));
  reformat("blep", blep, TABLESIZE, stdout, 1);
  fprintf(stdout, "\n");

  printf("// Sizeof blepd2: %d = %d floats\n\n", sizeof blepd2, sizeof blepd2 / sizeof(float));
  reformat("blepd2", blepd2, TABLESIZE, stdout, 1);
  fprintf(stdout, "\n");


  printf("// Sizeof blamp: %d = %d floats\n\n", sizeof blamp, sizeof blamp / sizeof(float));
  reformat("blamp", blamp, TABLESIZE, stdout, 0);
  fprintf(stdout, "\n");

  printf("// Sizeof blampd2: %d = %d floats\n\n", sizeof blampd2, sizeof blampd2 / sizeof(float));
  reformat("blampd2", blampd2, TABLESIZE, stdout, 0);

  return 0;
}
