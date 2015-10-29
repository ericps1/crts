/***************************************************************************/
/*           Demonstrates sprng use with one stream per process            */
/* A distinct stream is created on each process, then prints a few         */
/* random numbers.                                                         */
/***************************************************************************/

#if __GNUC__ > 3
 #include <string.h>
 #include <iostream>
#else
 #include <iostream.h>
#endif
#include <stdio.h>
#include <mpi.h>		/* MPI header file                         */

#include "sprng_cpp.h"

#define SEED 985456376


int main(int argc, char *argv[])
{
  int streamnum, nstreams;
  Sprng *stream;
  double rn;
  int i, myid, nprocs;
  int gtype;  /*---    */
  /*************************** MPI calls ***********************************/

  MPI_Init(&argc, &argv);	/* Initialize MPI                          */
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);	/* find process id                 */
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs); /* find number of processes      */

  /************************** Initialization *******************************/

  streamnum = myid;	
  nstreams = nprocs;		/* one stream per processor                */
/*--- node 0 is reading in a generator type */
  if(myid == 0)
  {
#include "gen_types_menu.h"
    printf("Type in a generator type (integers: 0,1,2,3,4,5):  ");
    i = scanf("%d", &gtype);
  }

  MPI_Bcast(&gtype,1,MPI_INT,0,MPI_COMM_WORLD );
    
  stream = SelectType(gtype);

  stream->init_sprng(streamnum,nstreams,SEED,SPRNG_DEFAULT);	/* initialize stream */
  printf("\n\nProcess %d, print information about stream:\n", myid);
  stream->print_sprng();

  /*********************** print random numbers ****************************/

  for (i=0;i<3;i++)
  {
    rn = stream->sprng();		/* generate double precision random number */
    printf("Process %d, random number %d: %.14f\n", myid, i+1, rn);
  }

  /*************************** free memory *********************************/

  stream->free_sprng();		/* free memory used to store stream state  */

  MPI_Finalize();		/* Terminate MPI                           */

  return 0;
}
