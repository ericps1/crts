/*************************************************************************/
/*************************************************************************/
/*               Parallel 48 bit Linear Congruential Generator           */
/*                                                                       */ 
/* Modifed by: J. Ren                                                    */
/*             Florida State University                                  */
/*             Email: ren@csit.fsu.edu                                   */
/*                                                                       */
/* Based on the implementation by:                                       */
/*             Ashok Srinivasan (Apr 13, 1998)                           */
/*************************************************************************/
/*************************************************************************/

#if __GNUC__ > 3
 #include <iostream>
#else
 #include <iostream.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "memory.h"
#include "primes_32.h"
#include "sprng.h"
#include "lcg.h"
#include <limits.h>
#define NDEBUG
#include <assert.h>
#include "store.h"

#define MAX_STREAMS lcg_MAX_STREAMS
#define NGENS lcg_NGENS

#define VERSION "00"
#define GENTYPE  VERSION "48 bit Linear Congruential Generator with Prime Addend"

#if LONG_MAX > 2147483647L	/* 32 bit integer */
#if LONG_MAX > 35184372088831L	/* 46 bit integer */
#if LONG_MAX >= 9223372036854775807L /* 64 bit integer */

#define LONG_SPRNG
#define LONG64 long
#define store_long64 store_long
#define store_long64array store_longarray
#define load_long64 load_long
#define load_long64array load_longarray

#define INIT_SEED 0x2bc68cfe166dL
#define MSB_SET 0x3ff0000000000000L
#define LSB48 0xffffffffffffL
#define AN1 0xdadf0ac00001L
#define AN2 0xfefd7a400001L
#define AN3 0x6417b5c00001L
#define AN4 0xcf9f72c00001L
#define AN5 0xbdf07b400001L
#define AN6 0xf33747c00001L
#define AN7 0xcbe632c00001L
#define PMULT1 0xa42c22700000L
#define PMULT2 0xfa858cb00000L
#define PMULT3 0xd0c4ef00000L
#define PMULT4 0xc3cc8e300000L
#define PMULT5 0x11bdbe700000L
#define PMULT6 0xb0f0e9f00000L
#define PMULT7 0x6407de700000L
#define MULT1 0x2875a2e7b175L	/* 44485709377909  */
#define MULT2 0x5deece66dL	/* 1575931494      */
#define MULT3 0x3eac44605265L	/* 68909602460261  */
#define MULT4 0x275b38eb4bbdL	/* 4327250451645   */
#define MULT5 0x1ee1429cc9f5L	/* 33952834046453  */
#define MULT6 0x739a9cb08605L	/* 127107890972165 */
#define MULT7 0x3228d7cc25f5L	/* 55151000561141  */
#endif
#endif
#endif

#if !defined(LONG_SPRNG) &&  defined(_LONG_LONG)
#define LONG64 long long
#define store_long64 store_longlong
#define store_long64array store_longlongarray
#define load_long64 load_longlong
#define load_long64array load_longlongarray

#define INIT_SEED 0x2bc68cfe166dLL
#define MSB_SET 0x3ff0000000000000LL
#define LSB48 0xffffffffffffLL
#define AN1 0xdadf0ac00001LL
#define AN2 0xfefd7a400001LL
#define AN3 0x6417b5c00001LL
#define AN4 0xcf9f72c00001LL
#define AN5 0xbdf07b400001LL
#define AN6 0xf33747c00001LL
#define AN7 0xcbe632c00001LL
#define PMULT1 0xa42c22700000LL
#define PMULT2 0xfa858cb00000LL
#define PMULT3 0xd0c4ef00000LL
#define PMULT4 0x11bdbe700000LL
#define PMULT5 0xc3cc8e300000LL
#define PMULT6 0xb0f0e9f00000LL
#define PMULT7 0x6407de700000LL
#define MULT1 0x2875a2e7b175LL
#define MULT2 0x5deece66dLL
#define MULT3 0x3eac44605265LL
#define MULT4 0x1ee1429cc9f5LL
#define MULT5 0x275b38eb4bbdLL
#define MULT6 0x739a9cb08605LL
#define MULT7 0x3228d7cc25f5LL
#endif

#define TWO_M24 5.96046447753906234e-8
#define TWO_M48 3.5527136788005008323e-15

#ifdef LittleEndian
#define MSB 1
#else
#define MSB 0
#endif
#define LSB (1-MSB)

#define LCGRUNUP 29

int MAX_STREAMS = 1<<19;

#ifndef TOOMANY
#define TOOMANY "generator has branched maximum number of times;\nindependence of streams cannot be guranteed\n"
#endif

#ifdef LONG64

unsigned LONG64 mults[] = {MULT1,MULT2,MULT3,MULT4,MULT5,MULT6,MULT7};
unsigned LONG64 multiplier=0;

#else

int mults[][4] = {{0x175,0xe7b,0x5a2,0x287},{0x66d,0xece,0x5de,0x000},
		  {0x265,0x605,0xc44,0x3ea},{0x9f5,0x9cc,0x142,0x1ee},
		  {0xbbd,0xeb4,0xb38,0x275},{0x605,0xb08,0xa9c,0x739},
		  {0x5f5,0xcc2,0x8d7,0x322}};
int *multiplier=NULL;
#endif

using namespace std;

#define NPARAMS 7

int NGENS=0;

void Plus (int *a, int *b, int *c);
void mult (int *a, int *b, int *c, int size);
void mult_48_32(int *a, int *b, int *c);
void mult_48_64(int *a, int *b, int *c);


int bit_reverse(int n)
{
  int i=31, rev=0;
  
  for(i=30; i>=0; i--)
  {
    rev |= (n&1)<<i;
    n >>= 1;
  }
  
  return rev;
}


void errprint(const char *level, const char *routine, const char *error)
{
#ifdef CRAY
#pragma _CRI guard 63
#endif
      fprintf(stderr,"%s from %s: %s\n",level,routine,error);
#ifdef CRAY
#pragma _CRI endguard 63
#endif
}


int strxncmp(char *s1, char *s2, int n)
{
  int i;
  
  for(i=0; i<n; i++)
    if(s1[i] != s2[i])
      return s1[i]-s2[i];
  
  return 0;			/* First n characters of strings are equal. */
}


LCG::LCG()
{
#ifdef LONG64
  rng_type = 1;
  seed = 0;
  init_seed = 0;
  prime = 0;
  prime_position = 0;
  prime_next = 0;
  gentype = NULL;
  parameter = 0;
  multiplier = 0;

#else
  rng_type = 1;
  seed[0] = 0;
  seed[1] = 1;
  init_seed = 0;
  prime = 0;
  prime_position = 0;
  prime_next = 0;
  gentype = NULL;
  parameter = 0;
  multiplier = NULL;

#endif
}

int LCG::init_rng(int gn, int tg, int s, int m)
{
  /*      gives back one generator (node gennum) with updated spawning     */
  /*      info; should be called total_gen times, with different value     */
  /*      of gennum in [0,total_gen) each call                             */
  int i;

  if (tg <= 0) /* check if total_gen is valid */
    {
      tg = 1;
      errprint("WARNING","init_rng","Total_gen <= 0. Default value of 1 used for total_gen");
    }

  if (gn >= MAX_STREAMS) /* check if gen_num is valid    */
    fprintf(stderr,"WARNING - init_rng: gennum: %d > maximum number of independent streams: %d\n\tIndependence of streams cannot be guranteed.\n",
	    gn, MAX_STREAMS); 

  if (gn < 0 || gn >= tg) /* check if gen_num is valid */
    {
      errprint("ERROR","init_rng","gennum out of range. "); 
      return 0;
    }

  s &= 0x7fffffff;/* Only 31 LSB of seed considered */
  
  if (m < 0 || m >= NPARAMS) 
    {
      errprint("WARNING","init_rng","multiplier not valid. Using Default param");
      m = 0;
    }

#ifdef LONG64

  if(multiplier == 0)
    multiplier = mults[m];
  /*  else
    if(multiplier != mults[m]) 
    fprintf(stderr,"WARNING: init_rng_d: Proposed multiplier does not agree with current multiplier.\n\t Independence of streams is not guaranteed\n");*/

#else
  if(multiplier == NULL)
    multiplier = mults[m];
  /*else
    if(strxncmp((char *) multiplier,(char *) mults[m],4*sizeof(int)) != 0) 
    fprintf(stderr,"WARNING: init_rng_d: Proposed multiplier does not agree with current multiplier.\n\t Independence of streams is not guaranteed\n");*/

#endif

  if ((LCG *) mymalloc(1*sizeof(LCG)) == NULL) {
    cerr << "Memory allocation error: init_rng()\n";
    return 0;
  }

  rng_type = 1;
  gentype = (char *)GENTYPE;
  init_seed = s;
  getprime_32(1, &prime, gn);
  prime_position = gn;
  prime_next = tg;
  parameter = m;
  
#ifdef LONG64

  seed = INIT_SEED;/* initialize generator */
  seed ^= ((unsigned LONG64) s)<<16;
  multiplier = mults[m];
  if (prime == 0) 
    seed |= 1;

#else
  seed[1] = 16651885^((s<<16)&(0xff0000));/* initialize generator */
  seed[0] = 2868876^((s>>8)&(0xffffff));
  multiplier = mults[m];
  if (prime == 0) 
    seed[1] |= 1;

#endif

  for(i=0; i<LCGRUNUP*prime_position; i++)
    get_rn_dbl();

  NGENS++;

  return 1;
}

LCG::~LCG()
{
  free_rng();
}

LCG::LCG (const LCG & c)
{
  rng_type = c.rng_type;
  init_seed = c.init_seed;
  prime = c.prime;
  prime_position = c.prime_position;
  prime_next = c.prime_next;
  gentype = c.gentype;
  parameter = c.parameter;

#ifdef LONG64
  seed = c.seed;
  multiplier = c.multiplier;
#else
  seed[0] = c.seed[0];
  seed[1] = c.seed[1];
  multiplier = c.multiplier;
#endif
}

LCG & LCG::operator= (const LCG & c)
{
  if (this != &c) {
    this->free_rng();

    rng_type = c.rng_type;
    init_seed = c.init_seed;
    prime = c.prime;
    prime_position = c.prime_position;
    prime_next = c.prime_next;
    gentype = c.gentype;
    parameter = c.parameter;
    
#ifdef LONG64
    seed = c.seed;
    multiplier = c.multiplier;
#else
    seed[0] = c.seed[0];
    seed[1] = c.seed[1];
    multiplier = c.multiplier;
#endif
  }

  return * this;
}

/*  On machines with 32 bit integers, */
/*  the Cray's 48 bit integer math is duplicated by breaking the problem into*/
/*  steps.  The algorithm is linear congruential.  M is the multiplier and S*/
/*   is the current seed. The 31 High order bits out of the 48 bits are 
     returned*/
int LCG::get_rn_int()
{
#ifdef LONG64
  multiply();
  
  return ((unsigned LONG64) seed) >> 17;
#else
  int s[4], res[4];
  multiply(multiplier,s,res);
  
  return (seed[0]<<7) | ((unsigned int) seed[1] >> 17);
#endif
} 

float LCG::get_rn_flt()
{
  return (float) get_rn_dbl();
} /* get_rn_float */


double LCG::get_rn_dbl()
{
#ifdef LONG64
  double temp[1];
  unsigned LONG64 *ltemp;
    
  temp[0] = 0.0;
  multiply();
  /* Add defined(O2K) || defined(SGI) if optimization level is -O2 or lower */
#if defined(CONVEX) || defined(GENERIC)
  ltemp = (unsigned LONG64 *) temp;
  *ltemp = (seed<<4) | MSB_SET;
  
  return temp[0] - (double) 1.0;
#else
  return seed*3.5527136788005008e-15;
#endif

#else

    static double equiv[1];
#define iran ((int *)equiv)
#define ran (equiv)

    int expo, s[4], res[4];
    
    multiply(multiplier,s,res);
    
#if defined(HP) || defined(SUN) || defined(SOLARIS) || defined(GENERIC)
    expo = 1072693248;

/*IEEE mantissa is 52 bits.  We have only 48 bits, so we shift our result 4*/
/*  bits left.  32-(24+4) = 4 bits are still blank in the lower word, so we*/
/*  grab the low 4 bits of seedhi to fill these. */
    iran[LSB] = genptr->seed[1] << 4 | genptr->seed[0] << 28;

/* place the remaining (24-4)=20 bits of seedhi in bits 20-0 of ran. */
/* Expo occupies bits 30-20.  Bit 31 (sign) is always zero. */
    iran[MSB] = expo | genptr->seed[0] >> 4;

    return (*ran - (double) 1.0);
#else
    return seed[0]*TWO_M24 + seed[1]*TWO_M48;
#endif  
  
#undef ran
#undef iran
#endif
} /* get_rn_dbl */


/*************************************************************************/
/*************************************************************************/
/*                  SPAWN_RNG: spawns new generators                     */
/*************************************************************************/
/*************************************************************************/

int LCG::spawn_rng(int nspawned, Sprng ***newgens)
{
  LCG ** genptr;
  int i, j;
  
  if (nspawned <= 0) /* check if nspawned is valid */
  {
    nspawned = 1;
    errprint("WARNING","spawn_rng","nspawned <= 0. Default value of 1 used for nspawned");
  }

  //  genptr = (LCG **) mymalloc(nspawned*sizeof(LCG *));
  genptr = new LCG *[nspawned];

  if(genptr == NULL)
  {
    *newgens = NULL;
    return 0;
  }
  
  for(i=0; i<nspawned; i++)
  {
    /*   genptr[i] = (LCG *) mymalloc(sizeof(LCG)); */
    genptr[i] = new LCG;

    if(genptr[i] == NULL)
    {
      nspawned = i;
      break;
    }
    
    genptr[i]->init_seed = init_seed;
    genptr[i]->prime_position = prime_position + prime_next*(i+1);

    if(genptr[i]->prime_position > MAXPRIMEOFFSET)
    {
      fprintf(stderr,"WARNING - spawn_rng: gennum: %d > maximum number of independent streams: %d\n\tIndependence of streams cannot be guranteed.\n",
	      genptr[i]->prime_position, MAX_STREAMS); 
      genptr[i]->prime_position %= MAXPRIMEOFFSET;
    }
    
    genptr[i]->prime_next = (nspawned+1)*prime_next;
    getprime_32(1, &(genptr[i]->prime), genptr[i]->prime_position);
    genptr[i]->multiplier = multiplier;
    genptr[i]->parameter = parameter;
    genptr[i]->gentype = gentype;
    genptr[i]->rng_type = rng_type;
    
#ifdef LONG64
    genptr[i]->seed = INIT_SEED;	/* initialize generator */
    genptr[i]->seed ^= ((unsigned LONG64)init_seed)<<16;	

    if (genptr[i]->prime == 0) 
      genptr[i]->seed |= 1;
#else
    genptr[i]->seed[1] = 16651885^((init_seed<<16)&(0xff0000));
    genptr[i]->seed[0] = 2868876^((init_seed>>8)&(0xffffff));

    if (genptr[i]->prime == 0) 
      genptr[i]->seed[1] |= 1;

#endif

    if(genptr[i]->prime_position > MAXPRIMEOFFSET)
      genptr[i]->advance_seed(); /* advance lcg 10^6 steps from initial seed */

    for(j=0; j<LCGRUNUP*(genptr[i]->prime_position); j++)
      genptr[i]->get_rn_dbl();
  }

  prime_next = (nspawned+1)*prime_next;
  NGENS += nspawned;
  *newgens = (Sprng **) genptr;
  
  return nspawned;
}


/*Compute a + b. a and b are positive 4 digit integers */
/* in base 2^12, modulo 2^48 */
void Plus(int *a, int *b, int *result) 
{
  int temp[5];
  int i;
  
  temp[4] = 0;
  
  for(i=0; i<4; i++)
    temp[i] = a[i] + b[i];
  
  for(i=1; i<5; i++)
  {
    temp[i] += temp[i-1]>>12;
    temp[i-1] &= 4095;
  }
  
  for(i=0; i<4 ; i++)
    result[i] = temp[i];
}



/*multiply two 4 digit numbers in base 2^12 and return 'size' lowest order */
/* digits*/
void mult(int *a, int *b, int *c, int size)
{
  int temp[8];
  int i, j;
  
  for(i=0; i<8; i++)
    temp[i] = 0;
  
  for(i=0; i<4; i++)
    for(j=0; j<4; j++)
      temp[i+j] += a[i]*b[j];
  
  for(i=1; i<8; i++)
  {
    temp[i] += temp[i-1]>>12;
    temp[i-1] &= 4095;
  }
  
  for(i=0; i<size && i<8 ; i++)
    c[i] = temp[i];
}


void mult_48_32(int * a, int * b, int *c)
{
#ifdef LONG64
  cerr << "mult_48_32(int*,int*,int*) error: LONG64" << endl;
#else
  c[0] = a[0]*b[0];
  c[1] = a[1]*b[0]+a[0]*b[1];
  c[2] = a[0]*b[2]+a[1]*b[1]+a[2]*b[0];
  c[3] = a[3]*b[0]+a[2]*b[1]+a[1]*b[2]+a[0]*b[3];
#endif
}

void mult_48_64(unsigned long int * a, unsigned long int * b, unsigned long int *c)
{
#ifdef LONG64
  *c = (*a * *b);
#else
  cerr << "mult_48_64(int*,int*,int) error: not LONG 64" << endl;
#endif
}

#ifdef LONG64
void LCG::multiply()
{
  mult_48_64(&seed,&multiplier,&seed);
  seed +=  prime;
  seed &= LSB48;
}
#else
void LCG::multiply(int * m, int * s, int * res)
{
  s[3] = (unsigned int) seed[0] >> 12;
  s[2] = seed[0] & 4095;
  s[1] = seed[1]  >> 12;
  s[0] = seed[1] & 4095;
  mult_48_32(m, s, res);
  seed[1] = res[0] + ((res[1]&4095) << 12) + prime;
  seed[0] = ( (unsigned int) seed[1] >> 24) + res[2] + ((unsigned int) res[1] >> 12 ) + (res[3] << 12);
  seed[1] &= 16777215;
  seed[0] &= 16777215;
}
#endif

void LCG::advance_seed()
{
#ifdef LONG64
  int i, found;
  unsigned LONG64 an, pmult;
  
  for(i=0, found=0; i<NPARAMS; i++)
    if (multiplier == mults[i])
    {
      found = 1;
      break;
    }
  if(found == 0)
  {
    fprintf(stderr,"WARNING: advance_seed: multiplier not acceptable.\n");
    return ;
  }

  /* a^n, n = 10^6 and pmult =  (a^n-1)/(a-1), n = 10^6 */
  switch(i)
  {
  case 0 :
    an = AN1;
    pmult = PMULT1;
    break;
  case 1 :
    an = AN2;
    pmult = PMULT2;
    break;
  case 2 :
    an = AN3;
    pmult = PMULT3;
    break;
  case 3 :
    an = AN4;
    pmult = PMULT4;
    break;
  case 4 :
    an = AN5;
    pmult = PMULT5;
    break;
  case 5 :
    an = AN6;
    pmult = PMULT6;
    break;
  case 6 :
    an = AN7;
    pmult = PMULT7;
    break;
  default:
    fprintf(stderr,"WARNING: advance_seed parameters for multiplier %d not available\n", i);
    return;
  }
  
  seed = seed*an + pmult*prime;
  seed &= LSB48;
  
#else
  int an[4], pmult[4], x0, x1, temp[4],temp2[4], i, found;
	
  for(i=0, found=0; i<NPARAMS; i++)
    if (strxncmp((char *) multiplier, (char *) (mults[i]), 4*sizeof(int)) == 0)
    {
      found = 1;
      break;
    }
  if(found == 0)
  {
    fprintf(stderr,"WARNING: advance_seed: multiplier not acceptable.\n");
    return ;
  }

  /* a^n, n = 10^6 and pmult =  (a^n-1)/(a-1), n = 10^6 */
  switch(i)
  {
  case 0 :
    an[0] = 0x001; an[1] = 0xc00; an[2] = 0xf0a; an[3] = 0xdad;
    pmult[0] = 0x000; pmult[1] = 0x700; pmult[2] = 0xc22; pmult[3] = 0xa42;
    break;
  case 1 :
    an[0] = 0x001; an[1] = 0x400; an[2] = 0xd7a; an[3] = 0xfef;
    pmult[0] = 0x000; pmult[1] = 0xb00; pmult[2] = 0x58c; pmult[3] = 0xfa8;
    break;
  case 2 :
    an[0] = 0x001; an[1] = 0xc00; an[2] = 0x7b5; an[3] = 0x641;
    pmult[0] = 0x000; pmult[1] = 0xf00; pmult[2] = 0xc4e; pmult[3] = 0x0d0;
    break;
  case 3 :
    an[0] = 0x001; an[1] = 0xc00; an[2] = 0xf72; an[3] = 0xcf9;
    pmult[0] = 0x000; pmult[1] = 0x700; pmult[2] = 0xdbe; pmult[3] = 0x11b;
    break;
  case 4 :
    an[0] = 0x001; an[1] = 0x400; an[2] = 0x07b; an[3] = 0xbdf;
    pmult[0] = 0x000; pmult[1] = 0x300; pmult[2] = 0xc8e; pmult[3] = 0xc3c;
    break;
  case 5 :
    an[0] = 0x001; an[1] = 0xc00; an[2] = 0x747; an[3] = 0xf33;
    pmult[0] = 0x000; pmult[1] = 0xf00; pmult[2] = 0x0e9; pmult[3] = 0xb0f;
    break;
  case 6 :
    an[0] = 0x001; an[1] = 0xc00; an[2] = 0x632; an[3] = 0xcbe;
    pmult[0] = 0x000; pmult[1] = 0x700; pmult[2] = 0x7de; pmult[3] = 0x640;
    break;
  default:
    fprintf(stderr,"WARNING: advance_seed parameters for multiplier %d not available\n", i);
    return;
  }
  
  x0 = seed[0]; x1 = seed[1];
  
  temp[0] = x1&4095; temp[1] = (x1>>12)&4095; temp[2] = x0&4095; /* seed */
  temp[3] = (x0>>12)&4095;

  temp2[0] = prime%(1<<12); temp2[1] = (prime>>12)%(1<<12);
  temp2[2] = (prime>>24)%(1<<12); temp2[3] = 0;	/* prime */

  mult(temp,an,temp,4);
  mult(temp2,pmult,temp2,4);
	
  Plus(temp,temp2,temp);
	
  seed[1] = (temp[1]<<12) + temp[0];
  seed[0] = (temp[3]<<12) + temp[2];
#endif
}


int LCG::free_rng()
{
  free(this);
  NGENS--;
  
  return NGENS;
}


int LCG::pack_rng(char **buffer)
{
  unsigned char *p, *initp;
  unsigned int size, m[2], i;
  
  size = 6*4 /*sizeof(int)*/ + 2*8/*sizeof(unsigned LONG64)*/  + strlen(gentype)+1;
  /* The new load/store routines make using sizeof unnecessary. Infact, */
  /* using sizeof could be erroneous. */
  initp = p = (unsigned char *) mymalloc(size);

  if(p == NULL)
  {
    *buffer = NULL;
    return 0;
  }

  p += store_int(rng_type,4,p);
  strcpy((char *)p,gentype);
  p += strlen(gentype)+1;

#ifdef LONG64
  p += store_long64(seed,8,p);
  p += store_int(init_seed,4,p);
  p += store_int(prime,4,p);
  p += store_int(prime_position,4,p);
  p += store_int(prime_next,4,p);
  p += store_int(parameter,4,p);
  p += store_long64(multiplier,8,p);
#else
  m[0] = seed[0]>>8;
  m[1] = seed[0]<<24 | seed[1];
  p += store_intarray(m,2,4,p);
  p += store_int(init_seed,4,p);
  p += store_int(prime,4,p);
  p += store_int(prime_position,4,p);
  p += store_int(prime_next,4,p);
  p += store_int(parameter,4,p);
  /* The following is done since the multiplier is stored in */
  /* pieces of 12 bits each                               */
  m[1] = multiplier[2]&0xff; 
  m[1] <<= 24; 
  m[1] |= multiplier[1]<<12; 
  m[1] |= multiplier[0];
  m[0] = (multiplier[3]<<4) | (multiplier[2]>>8);
  
  p += store_intarray(m,2,4,p);
#endif     
  *buffer =  (char *) initp;

  assert(p-initp == size);

  return p-initp;
}


int LCG::get_seed_rng()
{
  return init_seed;
}


int LCG::unpack_rng( char *packed)
{
  unsigned char *p;
  unsigned int m[4], m2[2], i;

  p = (unsigned char *) packed;

  if(this == NULL) {
    return 0;
  }
 
  p += load_int(p,4,(unsigned int *)&rng_type);

  if(strcmp((char *)p,GENTYPE) != 0)
  {
    fprintf(stderr,"ERROR: Unpacked ' %.24s ' instead of ' %s '\n", p, GENTYPE); 
    return 0;
  }
  else
    gentype = (char *)GENTYPE;
  p += strlen(gentype)+1;

#ifdef LONG64
  p += load_long64(p,8,&seed);
  p += load_int(p,4,(unsigned int *)&init_seed);
  p += load_int(p,4,(unsigned int *)&prime);
  p += load_int(p,4,(unsigned int *)&prime_position);
  p += load_int(p,4,(unsigned int *)&prime_next);
  p += load_int(p,4,(unsigned int *)&parameter);
  p += load_long64(p,8,&multiplier);

#else
  p += load_intarray(p,2,4,m2);
  seed[1] = m2[1]&0xffffff; seed[0] = m2[1]>>24 | m2[0]<<8;
  p += load_int(p,4,(unsigned int *)&init_seed);
  p += load_int(p,4,(unsigned int *)&prime);
  p += load_int(p,4,(unsigned int *)&prime_position);
  p += load_int(p,4,(unsigned int *)&prime_next);
  p += load_int(p,4,(unsigned int *)&parameter);
  p += load_intarray(p,2,4,m2);
  /* The following is done since the multiplier is stored in */
  /* pieces of 12 bits each                               */
  m[0] = m2[1]&0xfff; 
  m[1] = (m2[1]&0xfff000)>>12;
  m[2] = (m2[1]&0xff000000)>>24 | (m2[0]&0xf)<<8;
  m[3] = (m2[0]&0xfff0)>>4;
  
#endif


  if(parameter < 0 || parameter >= NPARAMS)
  {
    fprintf(stderr,"ERROR: Unpacked parameters not acceptable.\n");
    free(this);
    return 0;
  }
  
  multiplier = mults[parameter];
  
  NGENS++;

  return 1;
}

      
int LCG::print_rng()
{
  printf("\n%s\n", GENTYPE+2);
  printf("\n \tseed = %d, stream_number = %d\tparameter = %d\n\n", init_seed, prime_position, parameter);

  return 1;
}
