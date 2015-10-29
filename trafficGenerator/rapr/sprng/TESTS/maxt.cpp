#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "tests.h"
#include <math.h>

#ifdef SPRNG_MPI
#include <mpi.h>
#endif

double xt (double x);
double maxt (long n, int t);
void set_t (int t);
double *V2;

int xt_t = 1;


int main(int argc, char *argv[])
{
  long ntests, n, i;
  double *V, result;
  int t;
  
  if(argc != N_STREAM_PARAM + 3)
  {
    fprintf(stderr,"USAGE: %s (... %d arguments)\n",argv[0], N_STREAM_PARAM+2);
    exit(1);
  }
  
  ntests = init_tests(argc,argv);
  
  n = atol(argv[N_STREAM_PARAM+1]);
  t = atoi(argv[N_STREAM_PARAM+2]);
  
  V = new double[NTESTS];
  V2 = new double[n];
  
  for(i=0; i<ntests; i++)
  {
    V[i] = maxt(n,t);
    
    next_stream();
  }
  
  set_KS_n(NTESTS);

#if defined(SPRNG_MPI)
  getKSdata(V,NTESTS);
#endif
  
  if(proc_rank == 0)
  {
    result = KS(V,NTESTS,KSF);
    printf("\nResult: KS value = %f", result);
    result = KSpercent(result,NTESTS);
    printf("\t %% = %.2f\n\n", result*100.0);
  }
  

#if defined(SPRNG_MPI)
  MPI_Finalize();
#endif

  return 0;
}

double xt(double x)
{
  return pow(x,(double) xt_t);
}

void set_t(int t)
{
  xt_t = t;
}

double maxt(long n, int t)
{
  double *V=V2, temp;
  int j;
  long i;
  
  for(i=0; i<n; i++)
  {
    V[i] = 0.0;
    for(j=0; j<t; j++)
    {
      temp=get_rn();
      
      if( temp > V[i])
	V[i] = temp;
    }
  }
  
  set_t(t);
  temp = KS(V,n,xt);
  /*printf("\tKS for stream = %f, %% = %f\n", temp, KSpercent(temp,n));*/
  
  /*delete [] V;*/
  
  return temp;
}
