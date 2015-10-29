/*************************************************************************/
/*************************************************************************/
/*           Parallel 64-bit Linear Congruential Generator               */
/*                                                                       */ 
/* Modifed by: J. Ren                                                    */
/*             Florida State University                                  */
/*             Email: ren@csit.fsu.edu                                   */
/*                                                                       */
/* Based on the implementation by:                                       */
/*             Ashok Srinivasan (May 1998)                               */
/*                                                                       */ 
/* Note: The modulus is 2^64                                             */
/*                                                                       */
/* Disclaimer: NCSA expressly disclaims any and all warranties, expressed*/
/* or implied, concerning the enclosed software.  The intent in sharing  */
/* this software is to promote the productive interchange of ideas       */
/* throughout the research community. All software is furnished on an    */
/* "as is" basis. No further updates to this software should be          */
/* expected. Although this may occur, no commitment exists. The authors  */
/* certainly invite your comments as well as the reporting of any bugs.  */
/* NCSA cannot commit that any or all bugs will be fixed.                */
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
#include <limits.h>
#define NDEBUG
#include <assert.h>
#include "memory.h"
#include "sprng.h"
#include "lcg64.h"
#include "primes_64.h"
#include "store.h"

#define MAX_STREAMS lcg64_MAX_STREAMS
#define NGENS lcg64_NGENS
#define PARAMLIST lcg64_PARAMLIST

#define VERSION "00"
/*** Name for Generator ***/
#define GENTYPE VERSION "64 bit Linear Congruential Generator with Prime Addend"
int MAX_STREAMS = (146138719);  /*** Maximum number of independent streams ***/
#define NPARAMS 3		/*** number of valid parameters ***/

#if LONG_MAX > 2147483647L 
#if LONG_MAX > 35184372088831L 
#if LONG_MAX >= 9223372036854775807L 
#define LONG_SPRNG
#define LONG64 long		/* 64 bit long */
#define store_long64 store_long
#define store_long64array store_longarray
#define load_long64 load_long
#define load_long64array load_longarray
#endif
#endif
#endif

#if !defined(LONG_SPRNG) &&  defined(_LONG_LONG)
#define LONG64 long long
#define store_long64 store_longlong
#define store_long64array store_longlongarray
#define load_long64 load_longlong
#define load_long64array load_longlongarray
#endif


unsigned int PARAMLIST[NPARAMS][2] = {{0x87b0b0fdU, 0x27bb2ee6U}, 
				      {0xe78b6955U,0x2c6fe96eU},
				      {0x31a53f85U,0x369dea0fU}};


/*************************************************************************/
/* You should not need to look at the next few lines!                    */


#define INIT_SEED1 0x2bc6ffffU
#define INIT_SEED0 0x8cfe166dU

#define TWO_M22  2.384185791015625e-07 /* 2^(-22) */
#define TWO_P22  4194304  /* 2^(22) */
#define TWO_M20  9.5367431640625e-07 /* 2^(-20) */
#define TWO_P20  1048576 /* 2^(20) */
#define TWO_M42  2.273736754432321e-13 /* 2^(-42) */
#define TWO_M64 5.4210108624275222e-20 /* 2^(-64) */

/*                                                                      */
/************************************************************************/

int NGENS=0;		  /* number of random streams in current process */



/* Initialize random number stream */

int LCG64::init_rng(int gn, int tg, int s, int p)
{
/*      gives back one stream (node gennum) with updated spawning         */
/*      info; should be called total_gen times, with different value      */
/*      of gennum in [0,total_gen) each call                              */
  int i;
  double tempdbl;

  if (tg <= 0) /* Is total_gen valid ? */
  {
    tg = 1;
    fprintf(stderr,"WARNING - lcg64 init_rng: Total_gen <= 0. Default value of 1 used for total_gen\n");
  }

  if (gn >= MAX_STREAMS) /* check if gen_num is valid    */
    fprintf(stderr,"WARNING - lcg64 init_rng: gennum: %d > maximum number of independent streams: %d\n\tIndependence of streams cannot be guranteed.\n",
	    gn, MAX_STREAMS); 
  
  if (gn < 0 || gn >= tg) /* check if gen_num is valid    */
  {
    fprintf(stderr,"ERROR - lcg64 init_rng: gennum %d out of range [%d,%d).\n",
	    gn, 0, tg); 
    return 0;
  }

  if (p < 0 || p >= NPARAMS)     /* check if parameter is valid */
  {
    fprintf(stderr,"WARNING - lcg64 init_rng: parameter not valid. Using Default parameter.\n");
    p = 0;
  }

  if ((LCG64 *) mymalloc(1*sizeof(LCG64)) == NULL)
    return 0;
  
  /* Initiallize data structure variables */
  rng_type = 2;
  gentype = (char *)GENTYPE;
  stream_number = gn;
  nstreams = tg;
  init_seed = s & 0x7fffffff;  /* Only 31 LSB of seed considered */
  parameter = p;
  spawn_offset = tg;
  
  /*** Change the next line depending on your generators data needs ***/
  narrays = 0;		/* number of arrays needed by your generator */

  if(narrays > 0)
  {
    //    array_sizes = (int *) mymalloc(narrays*sizeof(int));
    //    arrays = (int **) mymalloc(narrays*sizeof(int *));
    array_sizes = new int[narrays];
    arrays = new int * [narrays];
  
    if(array_sizes == NULL || arrays == NULL)
      return 0;
  }
  else
  {
    array_sizes = NULL;
    arrays = NULL;
  }
  
  /*** Change the next line depending on your generators data needs ***/
  		/* initiallize ...array_sizes to the sizes of the arrays */


  for(i=0; i<narrays; i++)
  {
    //    arrays[i] = (int *) mymalloc(array_sizes[i]*sizeof(int)); 
    arrays[i] = new int[array_sizes[i]];

    if (arrays[i] == NULL)  /* check if memory allocated for data structure */
      return 0;
  }
  
  /*** Add initialization statements for your data in the arrays and other 
    variables you have defined ***/
  getprime_64(1,&prime,gn);
#ifdef LONG64
  multiplier = ((unsigned LONG64) PARAMLIST[p][1])<<32 | ((unsigned LONG64) PARAMLIST[p][0]);
  state = (((unsigned LONG64) INIT_SEED1)<<32 | INIT_SEED0) ^(((unsigned LONG64) s<<33)|gn);
#else
  multiplier[0] = (double) (PARAMLIST[p][0]&0x3fffff);
  multiplier[1] = (double) (PARAMLIST[p][0]>>22 | (PARAMLIST[p][1]&0xfff)<<10);
  multiplier[2] = (double) (PARAMLIST[p][1]>>12);
  state[0] = (double) ((INIT_SEED0^gn)&0x3fffff);
  state[1] = (double) ((INIT_SEED0^gn)>>22 | ((INIT_SEED1 ^ (unsigned)s<<1)&0xfff)<<10);
  state[2] = (double) ((INIT_SEED1 ^ (unsigned)s<<1)>>12);
#endif  

  for(i=0; i<127*stream_number; i++)
    tempdbl = get_rn_dbl();
  
  NGENS++;			/* NGENS = # of streams */

  return 1;
} 

LCG64::LCG64()
{
  rng_type = 2;
  gentype = NULL;
  stream_number = 0;
  nstreams = 0;
  init_seed = 0;
  parameter = 0;
  narrays = 0;
  array_sizes = NULL;
  arrays = NULL;
  spawn_offset = 0;
  prime = 0;

#ifdef LONG64
  state = 0;
  multiplier = 0;
#else
  state[0] = 0;
  state[1] = 0;
  state[2] = 0;
  multiplier[0] = 0;
  multiplier[1] = 0;
  multiplier[2] = 0;

#endif
}

LCG64::~LCG64()
{
  free_rng();
}

LCG64::LCG64(const LCG64 &c)
{
  rng_type = c.rng_type;
  gentype = c.gentype;
  stream_number = c.stream_number;
  nstreams = c.nstreams;
  init_seed = c.init_seed;
  parameter = c.parameter;
  narrays = c.narrays;
  array_sizes = c.array_sizes;
  arrays = c.arrays;
  spawn_offset = c.spawn_offset;
  prime = c.prime;

#ifdef LONG64
  state = c.state;
  multiplier = c.multiplier;
#else
  state[0] = c.state[0];
  state[1] = c.state[1];
  state[2] = c.state[2];
  multiplier[0] = c.multiplier[0];
  multiplier[1] = c.multiplier[1];
  multiplier[2] = c.multiplier[2];
#endif
}

LCG64 & LCG64::operator= (const LCG64 &c)
{
  if (this != &c) {
    this->free_rng();

    rng_type = c.rng_type;
    gentype = c.gentype;
    stream_number = c.stream_number;
    nstreams = c.nstreams;
    init_seed = c.init_seed;
    parameter = c.parameter;
    narrays = c.narrays;
    array_sizes = c.array_sizes;
    arrays = c.arrays;
    spawn_offset = c.spawn_offset;
    prime = c.prime;
    
#ifdef LONG64
    state = c.state;
    multiplier = c.multiplier;
#else
    state[0] = c.state[0];
    state[1] = c.state[1];
    state[2] = c.state[2];
    multiplier[0] = c.multiplier[0];
    multiplier[1] = c.multiplier[1];
    multiplier[2] = c.multiplier[2];
#endif
  }
}

void LCG64::advance_state()
{
#ifdef LONG64
  state = state*multiplier + prime;
#else
  double t0, t1, t2, t3, st0, st1, st2;
  t0 = state[0]*multiplier[0] + prime;
  t1 = (double) (int) (t0*TWO_M22); 
  st0 = t0 - TWO_P22*t1; 
  assert( (int) st0 == st0); 
  t1 += state[1]*multiplier[0] + state[0]*multiplier[1]; 
  t2 = (double) (int) (t1*TWO_M22); 
  st1 = t1 - TWO_P22*t2; 
  assert( (int) st1 == st1); 
  t2 += state[2]*multiplier[0] + state[1]*multiplier[1] + state[0]*multiplier[2];
  t3 = (double) (int) (t2*TWO_M20); 
  st2 = t2 - TWO_P20*t3; 
  assert( (int) st2 == st2); 
  state[0] = st0; 
  state[1] = st1; 
  state[2] = st2;
#endif
}

double LCG64::get_rn_dbl()
{
#ifdef LONG64
#ifdef _LONG_LONG
#define EXPO 0x3ff0000000000000ULL
#else
#define EXPO 0x3ff0000000000000UL
#endif
  static double dtemp[1] = {0.0};

  advance_state();	/* next state in sequence */
#if defined(CONVEX) || defined(O2K) || defined(SGI) || defined(GENERIC)
  *((unsigned LONG64 *) dtemp) = (state>>12) | EXPO;
  return *dtemp - (double) 1.0;
#else
  return state*TWO_M64;
#endif

#else  /* 32 bit machine */
#define EXPO 0x3ff00000
#ifdef LittleEndian
#define MSB 1
#else
#define MSB 0
#endif
#define LSB (1-MSB)
  double ans;
  unsigned int ist0, ist1, ist2;
  static double temp[1] = {0.0};

  advance_state();	/* next state in sequence */

  ist0 = static_cast<unsigned int>(state[0]);
  ist1 = static_cast<unsigned int>(state[1]);
  ist2 = static_cast<unsigned int>(state[2]);
  
#if defined(HP) || defined(SUN) || defined(SOLARIS) || defined(GENERIC)
/*IEEE mantissa is 52 bits. */
  ((unsigned int *)temp)[LSB] = ist1<<10 | ist0>>12;
  ((unsigned int *)temp)[MSB] = EXPO | ist2;
  return *temp - (double) 1.0;
#else
  return state[2]*TWO_M20 + state[1]*TWO_M42 + state[0]*TWO_M64;
#endif

#endif		
}

int LCG64::get_rn_int()
{
#ifdef LONG64
  advance_state();	/* next state in sequence */
  return state>>33;
#else
  return (int) (get_rn_dbl()*0x80000000U);
#endif

} 


/* Return a single precision random number */

float LCG64::get_rn_flt()
{
  /* If you have a more efficient way of computing the random integer,
     then please replace the statement below with your scheme.        */

    return (float) get_rn_dbl();
}


/*************************************************************************/
/*************************************************************************/
/*                  SPAWN_RNG: spawns new generators                     */
/*************************************************************************/
/*************************************************************************/

int LCG64::spawn_rng(int nspawned, Sprng ***newgens)
{
  LCG64 ** genptr;
  int i, j;
  
  if (nspawned <= 0) /* is nspawned valid ? */
  {
    nspawned = 1;
    fprintf(stderr,"WARNING - spawn_rng: nspawned <= 0. Default value of 1 used for nspawned\n");
  }
  
  //  genptr = (LCG64 **) mymalloc(nspawned*sizeof(LCG64 *));
  genptr = new LCG64 * [nspawned];

  if(genptr == NULL)	   /* allocate memory for pointers to structures */
  {
    *newgens = NULL;
    return 0;
  }
  
  for(i=0; i<nspawned; i++)	/* create nspawned new streams */
  {
    int s, gn;
    
    gn = stream_number + spawn_offset*(i+1);
  
    if(gn > MAX_STREAMS)   /* change seed to avoid repeating sequence */
      s = (init_seed)^gn; 
    else
      s = init_seed;
    
    /* Initialize a stream. This stream has incorrect spawning information.
       But we will correct it below. */

    genptr[i] = new LCG64;
    genptr[i]->init_rng (gn, gn+1, s, parameter);
  
    if(genptr[i] == NULL)	/* Was generator initiallized? */
    {
      nspawned = i;
      break;
    }
    genptr[i]->spawn_offset = (nspawned+1)*spawn_offset;
  }
  
  spawn_offset *= (nspawned+1);
  *newgens = (Sprng **) genptr;
 
  return nspawned;
}


/* Free memory allocated for data structure associated with stream */

int LCG64::free_rng()
{
  int i;
  
  assert(this != NULL);

  for(i=0; i<narrays; i++)
    delete [] arrays[i];

  if(narrays > 0)
  {
    delete [] array_sizes;
    delete [] arrays;
  }

  //  free(this);
  
  NGENS--;
  return NGENS;
}


int LCG64::pack_rng(char **buffer)
{
  unsigned char *p, *initp;
  int size, i;
  unsigned int temp, m[2];

  size = 4 + 48 + strlen(gentype)+1;
  
  initp = p = (unsigned char *) mymalloc(size); /* allocate memory */
  /* The new load/store routines make using sizeof unnecessary. Infact, */
  /* using sizeof could be erroneous. */
  if(p == NULL)
  {
    *buffer = NULL;
    return 0;
  }
  
  
  p += store_int(rng_type,4,p);
  strcpy((char *)p,gentype);
  p += strlen(gentype)+1;
  p += store_int(stream_number,4,p);
  p += store_int(nstreams,4,p);
  p += store_int(init_seed,4,p);
  p += store_int(parameter,4,p);
  p += store_int(narrays,4,p);
  p += store_int(spawn_offset,4,p);
  p += store_int(prime,4,p);
#ifdef LONG64			/* 64 bit integer types */
  p += store_long64(state,8,p);
  p += store_long64(multiplier,8,p);
#else  /* No 64 bit type available */
  m[0] = static_cast<unsigned int>(state[2]); 
  temp = static_cast<unsigned int>(state[1]);
  m[0]=(m[0]<<12)|(temp>>10);
  m[1] = static_cast<unsigned int>(state[1]); 
  temp = static_cast<unsigned int>(state[0]);
  m[1]=(m[1]<<22)|(temp);
  p += store_intarray(m,2,4,p);
  m[0] = static_cast<unsigned int>(multiplier[2]); 
  temp = static_cast<unsigned int>(multiplier[1]);
  m[0]=(m[0]<<12)|(temp>>10);
  m[1] = static_cast<unsigned int>(multiplier[1]); 
  temp = static_cast<unsigned int>(multiplier[0]);
  m[1]=(m[1]<<22)|(temp);
  p += store_intarray(m,2,4,p);
#endif
  
  *buffer =  (char *) initp;
  assert(p-initp == size);
  return p-initp;
}


int LCG64::unpack_rng( char *packed)
{
  unsigned int i, m[2];
  unsigned char *p;

  p = (unsigned char *) packed;

  if(this == NULL) 
    return 0;

  p += load_int(p,4,(unsigned int *)&rng_type);

  if(strcmp((char *)p,GENTYPE) != 0)
    {
      fprintf(stderr,"ERROR: Unpacked ' %.24s ' instead of ' %s '\n",  
	      p, GENTYPE); 
      return 0;
    }
  else
    gentype = (char *)GENTYPE;

  p += strlen(gentype)+1;

  p += load_int(p,4,(unsigned int *)&stream_number);
  p += load_int(p,4,(unsigned int *)&nstreams);
  p += load_int(p,4,(unsigned int *)&init_seed);
  p += load_int(p,4,(unsigned int *)&parameter);
  p += load_int(p,4,(unsigned int *)&narrays);
  p += load_int(p,4,(unsigned int *)&spawn_offset);
  p += load_int(p,4,&prime);
#ifdef LONG64                   /* 64 bit integer types */
  p += load_long64(p,8,&state);
  p += load_long64(p,8,&multiplier);
#else  /* No 64 bit type available */
  p += load_intarray(p,2,4,m);
  state[0] = (double) (m[1]&0x3fffff);
  state[1] = (double) ((m[1]>>22) | (m[0]&0xfff)<<10);
  state[2] = (double) (m[0]>>12);
  p += load_intarray(p,2,4,m);
  multiplier[0] = (double) (m[1]&0x3fffff);
  multiplier[1] = (double) ((m[1]>>22) | (m[0]&0xfff)<<10);
  multiplier[2] = (double) (m[0]>>12);
#endif

    
  array_sizes = NULL;
  arrays = NULL;
    
  NGENS++;

  return 1;
}

      

int LCG64::get_seed_rng()
{
  return init_seed;
}



int LCG64::print_rng()
{
  printf("\n%s\n", GENTYPE+2);
  printf("\n \tseed = %d, stream_number = %d\tparameter = %d\n\n", init_seed, stream_number, parameter);

  return 1;
}

