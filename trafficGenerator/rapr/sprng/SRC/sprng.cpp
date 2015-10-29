#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include "memory.h"
#include "communicate.h"
#include "sprng.h"
#include "sprng_cpp.h"

using namespace std;


Sprng *defaultgen = NULL;
int junk;                       /* pass useless pointer at times */
int junkmpi;

/********************* SIMPLE ************************/

int * init_rng_simple(int seed, int mult, int gtype /* = 0 */)
{
  int myid=0, nprocs=1;
  int val;
  Sprng * temp;

  temp = SelectType(gtype);

  val = temp->init_rng(myid,nprocs,seed,mult);

  if(val == 0)
    return NULL;
  else {
    if(defaultgen != NULL)
      defaultgen->free_rng();

    defaultgen = temp;

    return &junk;               /* return "garbage" value */
  }
}

int get_rn_int_simple()
{
  if(defaultgen == NULL) {
    if(init_rng_simple(0,0) == NULL) {
      return static_cast<int>(-1.0);
    }
  }

  return defaultgen->get_rn_int();
} 

float get_rn_flt_simple()
{
  if(defaultgen == NULL)
    if(init_rng_simple(0,0) == NULL)
      return -1.0;
  
  return defaultgen->get_rn_flt();
} 


double get_rn_dbl_simple()
{
  if(defaultgen == NULL)
    if(init_rng_simple(0,0) == NULL)
      return -1.0;
  
  return defaultgen->get_rn_dbl();
} 

int pack_rng_simple(char **buffer)
{
  if(defaultgen == NULL)
    return 0;
  
  return defaultgen->pack_rng(buffer);
}

int * unpack_rng_simple(char *packed, int gtype)
{
  Sprng * temp;
  int val;
  
  temp = SelectType(gtype);
  val = temp->unpack_rng(packed);
  
  if(val == 0)
    return NULL;
  else {
    if(defaultgen != NULL)
      defaultgen->free_rng();
      
    defaultgen = temp;
    return &junk;               /* return "garbage" value  */
  }
}

int print_rng_simple()
{
  if(defaultgen == NULL) {
    fprintf(stderr,"WARNING: No generator initialized so far\n");
    return 0;
  }
  
  return defaultgen->print_rng();
}

int make_new_seed()
{
  time_t tp;
  struct tm *temp;
  unsigned int temp2, temp3;
  static unsigned int temp4 = 0xe0e1;
  
  time(&tp);
  temp = localtime(&tp);
  
  temp2 = (temp->tm_sec<<26)+(temp->tm_min<<20)+(temp->tm_hour<<15)+
    (temp->tm_mday<<10)+(temp->tm_mon<<6);
  temp3 = (temp->tm_year<<13)+(temp->tm_wday<<10)+(temp->tm_yday<<1)+
    temp->tm_isdst;
  temp2 ^= clock()^temp3;

  temp4 = (temp4*0xeeee)%0xffff;
  temp2 ^= temp4<<16;
  temp4 = (temp4*0xaeee)%0xffff;
  temp2 ^= temp4;
  
  temp2 &= 0x7fffffff;

  return temp2;
}

/********************* SIMPLE_MPI ************************/

//#ifdef SPRNG_MPI

int * init_rng_simple_mpi(int seed,  int mult, int gtype /* = 0 */)
{
  int myid=0, nprocs=1;
  Sprng *temp;
  
  get_proc_info_mpi(&myid,&nprocs);

  temp = SelectType(gtype);
  temp->init_rng(myid,nprocs,seed,mult);

  if(temp == NULL)
    return NULL;
  else {
    if(defaultgen != NULL)
      defaultgen->free_rng();
    
    defaultgen = temp;

    return &junkmpi;            /* return "garbage" value */
  }
} 

int get_rn_int_simple_mpi()
{
  if(defaultgen == NULL)
    if(init_rng_simple_mpi(0,0) == NULL)
      return static_cast<int>(-1.0);
  
  return defaultgen->get_rn_int();
} 

float get_rn_flt_simple_mpi()
{
  if(defaultgen == NULL)
    if(init_rng_simple_mpi(0,0) == NULL)
      return -1.0;
  
  return defaultgen->get_rn_flt();
} 

double get_rn_dbl_simple_mpi()
{
  if(defaultgen == NULL)
    if(init_rng_simple_mpi(0,0) == NULL)
      return -1.0;
  
  return defaultgen->get_rn_dbl();
}

//#endif

#include "fwrap.cpp"

