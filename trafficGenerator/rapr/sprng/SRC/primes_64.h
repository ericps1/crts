#ifndef _primes_64_h_
#define _primes_64_h_

int getprime_64 (int need, unsigned int *prime_array, int offset);
 
#define MAXPRIME 3037000501U  /* largest odd # < sqrt(2)*2^31+2 */
#define MINPRIME 55108   /* sqrt(MAXPRIME) */
#define MAXPRIMEOFFSET 146138719U /* Total number of available primes */

#endif
