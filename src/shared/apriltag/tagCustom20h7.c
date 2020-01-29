#include <stdlib.h>
#include "apriltag.h"

apriltag_family_t *tagCustom20h7_create()
{
   apriltag_family_t *tf = calloc(1, sizeof(apriltag_family_t));
   tf->name = strdup("tagCustom20h7");
   tf->h = 7;
   tf->ncodes = 26;
   tf->codes = calloc(26, sizeof(uint64_t));
   tf->codes[0] = 0x0000000000094dddUL;
   tf->codes[1] = 0x0000000000075967UL;
   tf->codes[2] = 0x0000000000065f2cUL;
   tf->codes[3] = 0x0000000000046ab6UL;
   tf->codes[4] = 0x0000000000017c05UL;
   tf->codes[5] = 0x00000000000d9319UL;
   tf->codes[6] = 0x000000000009aa2dUL;
   tf->codes[7] = 0x00000000000abf1dUL;
   tf->codes[8] = 0x00000000000f0459UL;
   tf->codes[9] = 0x0000000000041ceaUL;
   tf->codes[10] = 0x0000000000022874UL;
   tf->codes[11] = 0x000000000008208bUL;
   tf->codes[12] = 0x00000000000493efUL;
   tf->codes[13] = 0x00000000000d1236UL;
   tf->codes[14] = 0x000000000006dad7UL;
   tf->codes[15] = 0x000000000009070aUL;
   tf->codes[16] = 0x000000000008c56fUL;
   tf->codes[17] = 0x00000000000b1f81UL;
   tf->codes[18] = 0x00000000000e4025UL;
   tf->codes[19] = 0x0000000000013129UL;
   tf->codes[20] = 0x000000000008dcc3UL;
   tf->codes[21] = 0x00000000000e8e12UL;
   tf->codes[22] = 0x0000000000045a0bUL;
   tf->codes[23] = 0x00000000000def03UL;
   tf->codes[24] = 0x00000000000ef14dUL;
   tf->codes[25] = 0x00000000000d9c0fUL;
   tf->nbits = 20;
   tf->bit_x = calloc(20, sizeof(uint32_t));
   tf->bit_y = calloc(20, sizeof(uint32_t));
   tf->bit_x[0] = 0;
   tf->bit_y[0] = -2;
   tf->bit_x[1] = 1;
   tf->bit_y[1] = -2;
   tf->bit_x[2] = 2;
   tf->bit_y[2] = -2;
   tf->bit_x[3] = 3;
   tf->bit_y[3] = -2;
   tf->bit_x[4] = 1;
   tf->bit_y[4] = 1;
   tf->bit_x[5] = 5;
   tf->bit_y[5] = 0;
   tf->bit_x[6] = 5;
   tf->bit_y[6] = 1;
   tf->bit_x[7] = 5;
   tf->bit_y[7] = 2;
   tf->bit_x[8] = 5;
   tf->bit_y[8] = 3;
   tf->bit_x[9] = 2;
   tf->bit_y[9] = 1;
   tf->bit_x[10] = 3;
   tf->bit_y[10] = 5;
   tf->bit_x[11] = 2;
   tf->bit_y[11] = 5;
   tf->bit_x[12] = 1;
   tf->bit_y[12] = 5;
   tf->bit_x[13] = 0;
   tf->bit_y[13] = 5;
   tf->bit_x[14] = 2;
   tf->bit_y[14] = 2;
   tf->bit_x[15] = -2;
   tf->bit_y[15] = 3;
   tf->bit_x[16] = -2;
   tf->bit_y[16] = 2;
   tf->bit_x[17] = -2;
   tf->bit_y[17] = 1;
   tf->bit_x[18] = -2;
   tf->bit_y[18] = 0;
   tf->bit_x[19] = 1;
   tf->bit_y[19] = 2;
   tf->width_at_border = 4;
   tf->total_width = 8;
   tf->reversed_border = false;
   return tf;
}

void tagCustom20h7_destroy(apriltag_family_t *tf)
{
   free(tf->codes);
   free(tf->bit_x);
   free(tf->bit_y);
   free(tf->name);
   free(tf);
}
