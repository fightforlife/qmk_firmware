// Small rand() implementation
//
// The libc rand implementation takes to much flash space, so an smaller rand() function is used. 
#include "quantum.h"

int rand(void)
{
   // static unsigned int z4;
   unsigned int z1, z2, z3, z4;
   int r;
   unsigned int b;

   z1 = timer_read32();
   z2 = 12345;
   z3 = 12345;
   z4 = z1 << 8;

   b  = ((z1 << 6) ^ z1) >> 13;
   z1 = ((z1 & 4294967294U) << 18) ^ b;
   b  = ((z2 << 2) ^ z2) >> 27;
   z2 = ((z2 & 4294967288U) << 2) ^ b;
   b  = ((z3 << 13) ^ z3) >> 21;
   z3 = ((z3 & 4294967280U) << 7) ^ b;
   b  = ((z4 << 3) ^ z4) >> 12;
   z4 = ((z4 & 4294967168U) << 13) ^ b;

   r = z1 ^ z2 ^ z3 ^ z4;
   // z4 = r;

   return r;
}