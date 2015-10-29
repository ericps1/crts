/*************************************************************************/
/*************************************************************************/
/* A PARALLEL RANDOM NUMBER GENERATION SYSTEM IN SPRNG FORMAT            */
/*                                                                       */
/* Check for more changes needed to add a new rng in DOCS                */
/*                                                                       */ 
/* Modifed by J.Ren                                                      */
/*      Florida State University                                         */
/* Email: ren@csit.fsu.edu                                               */
/* Based on implentation by                                              */
/* Ashok Srinivasan  (Apr 13, 1998)                                      */
/*                                                                       */ 
/*                                                                       */
/* Note: Data stored by 'pack_rng' is NOT in a machine independent       */
/*       format. For that, please take a look at some SPRNG examples     */
/*       (lcg/lcg.cpp, lfg/lfg.cpp, etc).                                */
/*************************************************************************/
/*************************************************************************/

#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <assert.h>
#include "memory.h"
#include "interface.h"

using namespace std;


/*** Change this to the type of generator you are implementing ***/
#define GENTYPE "Sample Generator"


#define MAX_STREAMS MyRng_MAX_STREAMS
#define NGENS MyRng_NGENS

#define NPARAMS 1		/*** number of valid parameters ***/
int MAX_STREAMS = (1<<19);      /*** Maximum number of independent streams ***/

int NGENS=0;		        /* number of random streams in current process */


MyRng::MyRng()  /* default constructor */
{
  /* modify if necessary */
  gentype = NULL;
  stream_number = 0;
  nstreams = 0;
  init_seed = 0;
  parameter = 0;
  narrays = 0;
  array_size = NULL;
  arrays = NULL;
  spawn_offset = 0;

  /* add other variables here */
  double x = 0.0; /* please remove this in your implementation */
}


/* Initialize random number stream */

int MyRng::init_rng(int gn, int tg, int s, int p)
{
/*      gives back one stream (node gennum) with updated spawning         */
/*      info; should be called total_gen times, with different value      */
/*      of gennum in [0,total_gen) each call                              */
  int i;
  
  if (tg <= 0) /* Is total_gen valid ? */
  {
    tg = 1;
    fprintf(stderr,"WARNING - init_rng: Total_gen <= 0. Default value of 1 used for total_gen\n");
  }

  if (gn >= MAX_STREAMS) /* check if gen_num is valid    */
    fprintf(stderr,"WARNING - init_rng: gennum: %d > maximum number of independent streams: %d\n\tIndependence of streams cannot be guranteed.\n",
	    gn, MAX_STREAMS); 

  if (gn < 0 || gn >= tg) /* check if gen_num is valid    */
  {
    fprintf(stderr,"ERROR - init_rng: gennum %d out of range [%d,%d).\n",
	    gn, 0, tg);

    return 0;
  }

  if (p < 0 || p >= NPARAMS)     /* check if parameter is valid */
  {
    fprintf(stderr,"WARNING - init_rng: parameter not valid. Using Default parameter.\n");
    p = 0;
  }
  	
   /* check if memory allocated for data structure */
  if ((MyRng *) mymalloc(1*sizeof(MyRng) == NULL) {
    cerr << "Memory allocation error: init_rng\n";
    return 0;
  }
  
  /* Initiallize data structure variables */
  gentype = GENTYPE;
  stream_number = gn;
  nstreams = tg;
  init_seed = s & 0x7fffffff;  /* Only 31 LSB of seed considered */
  parameter = p;
  spawn_offset = tg;
  
  /*** Change the next line depending on your generators data needs ***/
  narrays = 0;		/* number of arrays needed by your generator */

  if(narrays > 0)
  {
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
    arrays[i] = new int[array_sizes[i]];

    if(arrays[i] == NULL)  /* check if memory allocated for data structure */
      return 0;
  }
  
  /*** Add initialization statements for your data in the arrays and other 
    variables you have defined ***/
  x = 0.1;  /* Please remove this statement in your implementation */
  
  NGENS++;			/* NGENS = # of streams */
  
  return 1;
} 

MyRng::~MyRng()
{
  free_rng();
}

/* Copy constructor */
MyRng::MyRng(const MyRng & c)
{
  /* modify if necessary */
  gentype = c.gentype;
  stream_number = c.stream_number;
  nstreams = c.nstreams;
  init_seed = c.init_seed;
  parameter = c.parameter;
  narrays = c.narrays;
  array_size = c.array_size;
  arrays = c.arrays;
  spawn_offset = c.spawnoffset;

  /* add other variables here */
  double x = c.x; /* please remove this in your implementation */   
}

/* Assignment operator */
MyRng & MyRng::operator= (const MyRng &c)
{
  if (this != &c) {
    /* copy assignments of variables from copy constructor */       
           
    /* modify if necessary */
    gentype = c.gentype;
    stream_number = c.stream_number;
    nstreams = c.nstreams;
    init_seed = c.init_seed;
    parameter = c.parameter;
    narrays = c.narrays;
    array_size = c.array_size;
    arrays = c.arrays;
    spawn_offset = c.spawnoffset;

    /* add other variables here */
    double x = c.x; /* please remove this in your implementation */   
  }
  
  return *this;
}


/* Returns a double precision random number */

double MyRng::get_rn_dbl();
{
    struct rngen *genptr = (struct rngen *) igenptr;

    /*** Use the data in the structure genptr to update your state.
      Replace the statements below by those that return a double precision
      number in [0,1).  ***/

    if(x>0.85)	        /* the statements below are here just for    */
      x = 0.0;	        /* testing purposes. Please _do_  remove */
    else                /* them in your implementation.          */ 
      x += 0.1;
    
    return x;			
} 



/* Return a random integer */

int MyRng::get_rn_int()
{
  /* If you have a more efficient way of computing the random integer in
     [0,2^31), then please replace the statement below with your scheme. */

  return (int) (get_rn_dbl()*0x80000000);
} 



/* Return a single precision random number */


float MyRng::get_rn_flt()
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

int MyRng::spawn_rng(int nspawned, Sprng ***newgens, int checkid)
{
  MyRng ** genptr;
  int i, j;
  
  if (nspawned <= 0) /* is nspawned valid ? */
  {
    nspawned = 1;
    fprintf(stderr,"WARNING - spawn_rng: nspawned <= 0. Default value of 1 used for nspawned\n");
  }

  genptr = (MyRng **) mymalloc(nspawned *sizeof(MyRng *));
  
  if(genptr == NULL)	   /* allocate memory for pointers to structures */
  {
    *newgens = NULL;
    return 0;
  }
  
  for(i=0; i<nspawned; i++)	/* create nspawned new streams */
  {
    int seed, gennum;
    
    gennum = stream_number + spawn_offset*(i+1);
  
    if(gennum > MAX_STREAMS)   /* change seed to avoid repeating sequence */
      seed = (init_seed)^gennum; 
     else
      seed = init_seed;
   
    /* Initialize a stream. This stream has incorrect spawning information.
       But we will correct it below. */

    genptr[i] = new MyRng;
    genptr[i]->init_rng(gennum, gennum+1, seed, parameter);    
  
    if(genptr[i] == NULL)	/* Was generator initiallized? */
    {
      nspawned = i;
      break;
    }
    
    genptr[i]->spawn_offset = (nspawned+1)*spawn_offset;
  }
  
  tempptr->spawn_offset *= (nspawned+1);
  *newgens = (Sprng **) genptr;
  

  if(checkid != 0)
    for(i=0; i<nspawned; i++)
      if(addID(( int *) genptr[i]) == NULL)
	return i;
  
  return nspawned;
}


/* Free memory allocated for data structure associated with stream */

int MyRng::free_rng()
{
  int i;
  
  assert(this != NULL);
  
  for(i=0; i<narrays; i++)
    delete [] array[i];

  if(narrays > 0)
  {
    delete [] array_sizes];
    delete [] arrays;
  }

  NGENS--;
  return NGENS;
}


int MyRng::pack_rng(char **buffer)
{
  char *temp_buffer;
  int size, i;
  int pos;

  size = sizeof(MyRng) + narrays*sizeof(int) + strlen(gentype)+1;
  
  for(i=0; i<narrays; i++)
    size += array_sizes[i]*sizeof(int);
  
  temp_buffer = (char *) mymalloc(size); /* allocate memory */
 
  if(temp_buffer == NULL)
  {
    *buffer = NULL;
    return 0;
  }
  
  memcpy(temp_buffer,q,sizeof(MyRng));
  pos = sizeof(MyRng);
  strcpy(temp_buffer+pos,gentype);
  pos += strlen(gentype)+1;
  
  if(q->narrays > 0)
  {
    memcpy(temp_buffer+pos,array_sizes,narrays*sizeof(int));
    pos += narrays*sizeof(int);
    for(i=0; i<narrays; i++)
    {
      memcpy(temp_buffer+pos,arrays[i],array_sizes[i]*sizeof(int));
      pos += array_sizes[i]*sizeof(int);
    }
  }
  
  assert(pos == size);
  
  *buffer = temp_buffer;
  return size;
}


int MyRng::unpack_rng( char *packed)
{
  int i;
  int pos;

  if(this == NULL)
    return 0;

  memcpy(this,packed,sizeof(MyRng));
  pos = sizeof(MyRng);

  if(strcmp(packed+pos,GENTYPE) != 0)
  {
    fprintf(stderr,"ERROR: Unpacked ' %.24s ' instead of ' %s '\n",  
	    packed+pos, GENTYPE); 
    return 0; 
  }
  else
    gentype = GENTYPE;
  
  pos += strlen(gentype)+1;
    
  if(narrays > 0)
  {
    array_sizes = new int[narrays];
    arrays = new int *[narrays];
    
    if(array_sizes == NULL || arrays == NULL)
      return 0;

    memcpy(array_sizes,packed+pos,narrays*sizeof(int));
    pos += narrays*sizeof(int);
  
    for(i=0; i<narrays; i++)
    {
      arrays[i] = new int[array_sizes[i]];
      
      if(arrays[i] == NULL)
     	return 0;
      
      memcpy(arrays[i],packed+pos,array_sizes[i]*sizeof(int));
      pos += array_sizes[i]*sizeof(int);
    }   
  }
  else				/* narrays == 0 */
  {
    array_sizes = NULL;
    arrays = NULL;
  }
    
  NGENS++;
  
  return 1;
}

      
int MyRng::get_seed_rng()
{
  return init_seed;
}


int MyRng::print_rng()
{
  
  printf("\n%s\n", GENTYPE);
  
  printf("\n \tseed = %d, stream_number = %d\tparameter = %d\n\n", init_seed, stream_number, parameter);

  return 1;
}

