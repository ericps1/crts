/***************************************************************************/
/*         ____Demonstrates use of the single precision generator____      */
/* One stream is maintained per processor. Each processor prints a few     */
/* single precision random numbers.                                        */
/***************************************************************************/

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <mpi.h>                /* MPI header file                         */


/* Uncomment the following line to get the interface with pointer checking */
/*#define CHECK_POINTERS                                                   */
 
#define FLOAT_GEN	  /* make 'sprng()' return single precision numbers*/
#include "sprng_cpp.h"              /* SPRNG header file                       */

using namespace std;

#define SEED 985456376

int main(int argc, char *argv[])
{
  int streamnum, nstreams;
  Sprng *stream;
  float rn;
  int i, myid, nprocs;
  int gtype;  /*---    */

  /************************** MPI calls ************************************/
            
  MPI_Init(&argc, &argv);       /* Initialize MPI                          */
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);	/* find process id                 */
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs); /* find number of processes      */

  /************************* Initialization ********************************/
            
  streamnum = myid;
  nstreams = nprocs;		/* one stream per processor                */
  /*--- node 0 is reading in a generator type */
  if(myid == 0)
  {
#include "gen_types_menu.h"
    printf("Type in a generator type (integers: 0,1,2,3,4,5):  ");
    i = scanf("%d", &gtype);
    if (i != 1)
	    exit(1);
  }
  MPI_Bcast(&gtype,1,MPI_INT,0,MPI_COMM_WORLD ); /*--- broadcast gen type */

  stream = SelectType(gtype);
  stream->init_sprng(streamnum,nstreams,SEED,SPRNG_DEFAULT);	/*initialize stream*/
  printf("Process %d: Print information about stream:\n",myid);
  stream->print_sprng();

  /*********************** print random numbers ****************************/
            
  for (i=0;i<3;i++)
  {
    rn = stream->sprng();	/* generate single precision random number */
    printf("Process %d, random number %d: %f\n", myid, i+1, rn);
  }

  /*************************** free memory *********************************/
            
  stream->free_sprng();         /* free memory used to store stream state  */

  MPI_Finalize();		/* Terminate MPI                           */

  return 0;
}
