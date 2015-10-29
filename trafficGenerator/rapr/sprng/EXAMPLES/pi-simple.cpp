/***************************************************************************/
/*       ____Demonstrates SPRNG use for Monte Carlo integration____        */
/* Compute pi using Monte Carlo integration. Random points are generated   */
/* in a square of size 2. The value of pi is estimated as four times the   */
/* the proportion of samples that fall within a circle of unit radius.     */
/***************************************************************************/


#include <iostream>
#include <cstdio>
#include <cmath>
#include <cstring>

#define SIMPLE_SPRNG		/* simple interface                        */
#include "sprng_cpp.h"

using namespace std;


#define EXACT_PI 3.141592653589793238462643
 
int gtype;  /*---    */

int initialize_function(int * n, int * in_old, int * n_old, char * filename);
int count_in_circle(int n);	
int save_state(char * filename, int  in, int  n);

int main(int argc, char *argv[])
{
  int in, n, in_old, n_old, scan_ret = 0;
  double pi, error, stderror, p=EXACT_PI/4.0;
  char filename[80];
  char strbuf[1024];

  /*--- reading in a generator type */
#include "gen_types_menu.h"
  printf("Type in a generator type (integers: 0,1,2,3,4,5):  ");
  if( fgets(strbuf, sizeof strbuf, stdin) != NULL) {
    scan_ret = sscanf(strbuf, "%d", &gtype);
  }
  if (scan_ret != 1) {
    printf("Generator type entered is not valid, using type 0\n");
    gtype = 0;
  }
  
  initialize_function(&n, &in_old, &n_old, filename);	/* read args & initialize  */
  
  in = count_in_circle(n);	/* count samples in circle                 */
  
  in += in_old;			/* # in circle, in all runs                */
  n += n_old;			/* # of samples, in all runs               */
  pi = (4.0*in)/n;		/* estimate pi                             */
  error = fabs(pi - EXACT_PI);	/* determine error                         */
  stderror = 4*sqrt(p*(1.0-p)/n); /* standard error                        */
  printf( "pi is estimated as %18.16f from %12d samples.\n", pi, n);
  printf( "\tError = %10.8g, standard error = %10.8g\n", error, stderror);

  save_state(filename, in, n);	/* check-point final state                 */

  return 0;
}


int count_in_circle(int n)		/* count # of samples in circle    */
{
  int i, in;
  double x, y;
  
  for (i=in=0; i<n; i++)	/* find number of samples in circle        */
  {
    x = 2*sprng() - 1.0;	/* x coordinate                            */
    y = 2*sprng() - 1.0;	/* y coordinate                            */
    if (x*x + y*y < 1.0)	/* check if point (x,y) is in circle       */
      in++;
  }

  return in;
}


/* Read arguments and initialize stream                                    */
int initialize_function(int * n, int * in_old, int * n_old, char * filename)
{
  int seed, size, new_old, scan_ret, read_ret;
  char buffer[MAX_PACKED_LENGTH];
  FILE *fp;
  
  printf("Enter 9 for a new run, or 2 for the continuation of an old run:\n");
  scan_ret = scanf("%d", &new_old);
  printf("Enter name of file to store/load state of the stream:\n");
  scan_ret = scanf("%s", filename);
  printf("Enter number of new samples:\n");
  scan_ret = scanf("%d", n);

  if(new_old == 9)		/* new set of runs                         */
  {
    seed = make_sprng_seed();	/* make seed from date/time information    */
    
    init_sprng(seed,CRAYLCG,gtype);  /* initialize stream                  */
    print_sprng();		/* print information abour stream          */

    *in_old = 0;
    *n_old = 0;
  }
  else				/* continue from previously stored state   */
  {
    fp = fopen(filename,"r");	/* open file                               */
    if(fp == NULL)
    {
      fprintf(stderr,"ERROR opening file %s\n", filename);
      exit(1);
    }
    
    read_ret = fread(in_old,1,sizeof(int),fp); /* cumulative # in circle previously   */
    read_ret = fread(n_old,1,sizeof(int),fp);  /* cumulative # of samples previously  */
    read_ret = fread(&size,1,sizeof(int),fp);  /* size of stored stream state         */
    read_ret = fread(buffer,1,size,fp);	/* copy stream state to buffer             */
    unpack_sprng(buffer,gtype);	/* retrieve state of the stream            */
    fclose(fp);			/* close file                              */
  }
  
  return 0;
}


int save_state(char * filename, int  in, int  n)	/* store the state  */
{
  FILE *fp;
  char *bytes;
  int size;
  
  fp = fopen(filename,"w");	/* open file to store state                */

  if(fp == NULL)
  {
    fprintf(stderr,"Could not open file %s for writing\nCheck path or permissions\n", filename);
    exit(1);
  }

  fwrite(&in,1,sizeof(int),fp); /* store # in circle in all runs           */
  fwrite(&n,1,sizeof(int),fp);  /* store # of samples in all runs          */

  size = pack_sprng(&bytes);	/* pack stream state into an array         */
  fwrite(&size,1,sizeof(int),fp); /* store # of bytes required for storage */
  fwrite(bytes,1,size,fp);      /* store stream state                      */
  fclose(fp);

  free(bytes);		        /* free memory needed to store stream state*/

  return 0;
}
