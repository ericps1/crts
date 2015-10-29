#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "tests.h"

#ifdef SPRNG_MPI
#include <mpi.h>
#endif

void init_coupon (long n, int t, int d);
double coupon (long n, int t, int d);
double stirling (int n, int m);

int *occurs, Bins_used=0;
long *count;
double *probability;


int main(int argc, char *argv[])
{
  long ntests, n, i;
  double *V, result;
  int t, d;
  
  if(argc != N_STREAM_PARAM + 4)
  {
    fprintf(stderr,"USAGE: %s (... %d arguments)\n",argv[0], N_STREAM_PARAM+3);
    exit(1);
  }
  
  ntests = init_tests(argc,argv);

  V = new double[NTESTS];  
 
  n = atol(argv[N_STREAM_PARAM+1]);
  t = atoi(argv[N_STREAM_PARAM+2]);
  d = atoi(argv[N_STREAM_PARAM+3]);
  
  if(t<=d)
  {
    fprintf(stderr,"ERROR: t = %d must be greater than d = %d\n", t, d);
    exit(1);
  }

  init_coupon(n,t,d);
  
  for(i=0; i<ntests; i++)
  {
    V[i] = coupon(n,t,d);
    
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
  delete [] occurs;
  delete [] count;
  delete [] probability;

#if defined(SPRNG_MPI)
  MPI_Finalize();
#endif

  return 0;
}

void init_coupon(long n, int t, int d)
{
  int i, r;
  double prod;
  
  occurs = new int[d];
  count = new long[t-d+1];
  probability = new double[t-d+1];
  
  prod = 1;

  for(i=1; i<d; i++)
    prod *= static_cast<double>(i)/ static_cast<double>(d);
  
  for(r=d; r<t; r++)
  {
    probability[r-d] = prod*stirling(r-1,d-1);
    prod /= static_cast<double>(d);
  }
  
  prod *= static_cast<double>(d);
  probability[t-d] = 1.0 - prod*stirling(t-1,d);
  
}

double coupon(long n, int t, int d)
{
  int r, q;
  long s;
  int temp;
  double answer;
  
  memset(count, 0, (t-d+1)*sizeof(long));
  
  for(s=0; s<n; s++)
  {
    r = 0;
    memset(occurs,0,d*sizeof(int));
    
    for(q=0; q<d; q++)
    {
      while( occurs[(temp = (int) (d*get_rn()))] != 0 )
	r++;
      
      occurs[temp] = 1;
    }
    
    if(r >= t-d)
      count[t-d]++;
    else
      count[r]++; 
    
  }
  
  answer = chisquare(count,probability,n, t-d+1, &Bins_used);
  
  /*printf("\tChisquare for stream = %f, %% = %f\n", answer, 
	 chipercent(answer,t-d+1));*/
  
  return answer;
}
