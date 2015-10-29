#ifdef SPRNG_MPI
#include <mpi.h>
#endif
#include <cstdio>
#include <cstdlib>
/*#define READ_FROM_STDIN*/   /* read random numbers from stdin */
#ifndef READ_FROM_STDIN
#include "../include/sprng_cpp.h"
#endif
#include "util.h"

using namespace std;

#define OFFSET 0		/* set offset to 'k' to start tests
				   from the k th stream onwards */

long NTESTS = 0;
int proc_rank=0, init_nprocs=1;
int nsubsequences=0;

static int init_seed, init_param, init_total;
static int current_subsequence;
static long current_group, first_group, init_ngroups;
static Sprng **gens;
static int current_gen, n_combine, skip;
static int rng_type; /*--- random number type ---*/

long init_streams (int argc, char *argv[]);
void next_stream (void);
double get_rn (void);


#ifdef READ_FROM_STDIN		/* read random numbers from stdin */

int *init_sprng(int a, int b, int c, int d)
{
  return NULL;
}

int free_sprng(int *a)
{
  return 0;
}

double sprng(int *a)
{
  double rn;
  scanf("%lf", &rn);
  return rn;
}
#endif

long init_tests(int argc, char *argv[])
{
  long n;

#ifdef SPRNG_MPI
  MPI_Init(&argc, &argv);
#endif

/*-----------------------------------------------------------------------*/
/* Changed by Yaohang Li to fit for the new interface                    */
/* Adding the rng_type to standard init_sprng interface                  */
/* The Number of the arguments increases by 1                            */
/*-----------------------------------------------------------------------*/
  if(argc < 7+1)
  {
    fprintf(stderr,"Usage: %s n_sets ncombine seed param nsubsequences skip test_arguments\n",
	    argv[0]);
    exit(-1);
  }
   
  if (atoi(argv[1])>5||atoi(argv[1])<0)
  {
    fprintf(stderr,"Error: First command line argument(random number type) should be between 0 to 5\n");
    exit(-1);
  }

  if(atoi(argv[3]) <= 0)
  {
    fprintf(stderr,"Error: Third command line argument should be greater than 0\n");
    exit(-1);
  }
  
  if(atoi(argv[6]) <= 0)
  {
    fprintf(stderr,"Error: Sixth command line argument should be greater than 0\n");
    exit(-1);
  }
/*-------------- End by changing ------------------------------*/
  
  n = init_streams(argc, argv);
  
  return n;
}


long init_streams(int argc, char *argv[])
{
/*------------------------------------------------------------*/
/* Modify by Yaohang Li                                       */
/* Adding rng_type as a new argument                          */
/*------------------------------------------------------------*/
  int seed, param, n, i;
  int myid = 0, nprocs = 1;
  
  rng_type = atoi(argv[1]); /*--- Get the rand type by reading the 1 arg ---*/
  n = atoi(argv[2]);
  n_combine = atoi(argv[3]);
  seed = atoi(argv[4]);
  param = atoi(argv[5]);
  nsubsequences = atoi(argv[6]);
  skip = atoi(argv[7]);
  
#ifdef SPRNG_MPI
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  proc_rank = myid;
  init_nprocs = nprocs;
#endif

   if(proc_rank == 0)
   {
     for(i=0; i<argc; i++)
       printf("%s  ", argv[i]);
     putchar('\n');
   }
   
  first_group = current_group = n*myid/nprocs;
  init_seed = seed;
  init_param = param;
  init_total = n;
  NTESTS = n*nsubsequences;
  current_gen = 0;
  current_subsequence = 0;
  
  gens = new Sprng *[n_combine];

  for(i=0; i<n_combine; i++) {
    gens[i] = SelectType(rng_type);
    gens[i]->init_sprng(n_combine*current_group+i+OFFSET,n_combine*n+OFFSET,seed,init_param);
  }
 
  init_ngroups = n*(myid+1)/nprocs - n*myid/nprocs;
  
  return init_ngroups*nsubsequences;
}

void next_stream(void)
{
  int i;
  double temp;
  
  current_subsequence = (current_subsequence + 1)%nsubsequences;
  
  if(current_subsequence > 0)
    for(i=0; i<skip; i++)
    {
      temp = get_rn();
    }
  else
  {
    current_group++;
    current_gen = 0;
  
    for(i=0; i<n_combine; i++)
      gens[i]->free_sprng();

    if(current_group > first_group && current_group < first_group+init_ngroups)
      for(i=0; i<n_combine; i++) {
	gens[i] = SelectType(rng_type);
	gens[i]->init_sprng(n_combine*current_group+i+OFFSET,n_combine*init_total+OFFSET,init_seed, init_param);
      }
    else if(current_group > first_group && current_group >
	    first_group+init_ngroups)
      printf("ERROR: current_pair = %ld not in allowed range [%d,%ld]\n",
	     current_group,0,init_ngroups-1 ); 
    
  }
  
}

double get_rn(void)
{
  double temp;
  
  temp = gens[current_gen]->sprng();
  current_gen = (current_gen+1)%n_combine;
  
  return temp;
}


#if 0
int main(int argc, char *argv[])
{
  int n, i, j, length;

  n = init_tests(argc,argv);
  /*--- increase argv index by 1 ---*/
  length = atoi(argv[8]);
  
  for(i=0; i<n; i++)
  {
    for(j=0; j<length; j++) 
    { 
      printf("(%d, %d, %d). current_gen = %d, current_group = %d", 
	     proc_rank, i, j, current_gen, current_group);
      printf(" rn = %f\n", get_rn()); 
    } 

    next_stream(); 
  } 

#ifdef SPRNG_MPI 
  MPI_Finalize();
#endif

  return 0;
}
#endif
