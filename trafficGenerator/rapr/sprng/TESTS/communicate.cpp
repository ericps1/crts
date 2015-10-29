#ifdef SPRNG_MPI
#include <mpi.h>
#endif

#include <cstdlib>
#include <cstring>
#include "util.h"

using namespace std;

#define SPRNG_MPI_KS 0

extern int proc_rank, init_nprocs, nsubsequences;

void getKSdata (double *V, long ntests)
{
  int nprocs, myid, i, nsets;
  long first, last;
  
#ifdef SPRNG_MPI
  MPI_Status status;
  
  myid = proc_rank;
  nprocs = init_nprocs;
  nsets = ntests/nsubsequences;
  
  first = (nsets*myid/nprocs)*nsubsequences;
  last = (nsets*(myid+1)/nprocs)*nsubsequences - 1;
  
  if(myid != 0)
    MPI_Send(V, last-first+1, MPI_DOUBLE, 0, SPRNG_MPI_KS, MPI_COMM_WORLD);
  else
    for(i=1; i<nprocs; i++)
      MPI_Recv(V+(nsets*i/nprocs)*nsubsequences,
	       (nsets*(i+1)/nprocs - nsets*i/nprocs)*nsubsequences,
	       MPI_DOUBLE, i, SPRNG_MPI_KS, MPI_COMM_WORLD, &status);
  
  /*MPI_Finalize();*/
#endif
}


void  reduce_sum(double *array,int n)
{
  double *temp;
  
#ifdef SPRNG_MPI
  temp = new double[n];

  MPI_Reduce(array, temp, n, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  
  if(proc_rank == 0)
    memcpy(array,temp,n*sizeof(double));

  delete [] temp;
#endif
}

void  reduce_sum_long(long *array,long n)
{
  long *temp;
  
#ifdef SPRNG_MPI
  temp = new long[n];

  MPI_Reduce(array, temp, n, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
  
  if(proc_rank == 0)
    memcpy(array,temp,n*sizeof(long));

  delete [] temp;
#endif
}

  
