#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "tests.h"
#include <math.h>

#ifdef SPRNG_MPI
#include <mpi.h>
#endif

using namespace std;

void init_poker (long n, int k, int d);
double poker (long n, int k, int d);
double stirling (int n, int m);

static int ncatagories = 0, *bins, *poker_index, Bins_used=0;
long *actual;
double *probability;


int main(int argc, char *argv[])
{
  long ntests, n, i;
  double *V, result;
  int k, d;
  
  if(argc != N_STREAM_PARAM + 4)
  {
    fprintf(stderr,"USAGE: %s (... %d arguments)\n",argv[0], N_STREAM_PARAM+3);
    exit(1);
  }
  
  ntests = init_tests(argc,argv);
  
  V = new double[NTESTS];
 
  n = atol(argv[N_STREAM_PARAM+1]);
  k = atoi(argv[N_STREAM_PARAM+2]);
  d = atoi(argv[N_STREAM_PARAM+3]);
  
  init_poker(n,k,d);
  
  for(i=0; i<ntests; i++)
  {
    V[i] = poker(n,k,d);
    
    next_stream();
  }
  
#if defined(SPRNG_MPI)
  getKSdata(V,NTESTS);
#endif
  
  if(proc_rank == 0)
  {
    set_d_of_f(Bins_used-1);
    result = KS(V,NTESTS,chiF);
    printf("\nResult: KS value = %f", result);
    result = KSpercent(result,NTESTS);
    printf("\t %% = %.2f\n\n", result*100.0);
  }
  
  delete [] V;
  delete [] actual;
  delete [] bins;
  delete [] probability;
  delete [] poker_index;

#if defined(SPRNG_MPI)
  MPI_Finalize();
#endif

  return 0;
}


void init_poker(long n, int k, int d)
{
  int i;
  double *pr, temp;
  long sum;
  
  bins = new int[d];
  poker_index = new int[k+1];
  pr = new double[k+1];
  temp = pow((double) d, - (double) k);
  
  for(i=1; i<=k; i++)
  {
    temp *= d-i+1;
    pr[i] = temp*stirling(k,i);
  }
  
  ncatagories = 0;
  sum = 0;
  for(i=1; i<=k; i++)
  {
    poker_index[i] = ncatagories;
    sum += static_cast<long>(n*pr[i]);
    
    if(sum > 5 && i < k)
    {
      sum = 0;
      ncatagories++;
    }
  }
  
  ncatagories++;

  actual = new long[ncatagories];
  probability = new double[ncatagories];

  for(i=0; i< ncatagories; i++)
    probability[i] = 0.0;
  
  for(i=1; i<=k; i++)
    probability[poker_index[i]] += pr[i];
  
  free(pr);
}

double poker(long n, int k, int d)
{
  double temp;
  int j, sum;
  long i;
  
  memset(actual, 0, ncatagories*sizeof(long));
  
  for(i=0; i<n; i++)
  {
    memset(bins,0,d*sizeof(int));
    
    for(j=0; j<k; j++)
    {
      bins[(int) (d*get_rn())] = 1;
    }
    
    sum = 0;
    for(j=0; j<d; j++)
      sum += bins[j];
    
    actual[poker_index[sum]]++;
  }
  
  temp = chisquare(actual,probability,n, ncatagories, &Bins_used);
  /*printf("\tChisquare for stream = %f, %% = %f\n", temp, 
	 chipercent(temp,ncatagories-1));*/
  
  return temp;
}
