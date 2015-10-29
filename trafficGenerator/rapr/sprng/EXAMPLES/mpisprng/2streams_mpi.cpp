/***************************************************************************/
/*         ____Demonstrates use of shared and non-shared streams____       */
/* Each process has two streams.  One stream is common to all the          */
/* processes. The other stream is different on each process.               */
/***************************************************************************/

#include <stdio.h>
#include <mpi.h>                 /* MPI header file                        */

#include "sprng_cpp.h"           /* SPRNG header file                      */

#define SEED 985456376



int main(int argc, char *argv[])
{
  int streamnum, commNum, nstreams;
  Sprng *stream, *commonStream;
  double rn;
  int i, myid, nprocs;
  int gtype;  /*---    */

  /************************** MPI calls ***********************************/
            
  MPI_Init(&argc, &argv);       /* Initialize MPI                         */
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);	/* find process id                */
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs); /* find number of processes     */

  /****************** Initialization values *******************************/
            
  streamnum = myid;		/*This stream is different on each process*/
  commNum = nprocs;	        /* This stream is common to all processes */
  nstreams = nprocs + 1;	/* extra stream is common to all processes*/

  /*********************** Initialize streams *****************************/
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
          
  /* This stream is different on each process                             */
  stream = SelectType(gtype);
  stream->init_sprng(streamnum,nstreams,SEED,SPRNG_DEFAULT);
  printf("Process %d: Print information about new stream\n", myid);
  stream->print_sprng();

  /* This stream is identical on each process                             */
  commonStream = SelectType(gtype);
  commonStream->init_sprng(commNum,nstreams,SEED,SPRNG_DEFAULT);
  printf("Process %d: This stream is identical on all processes\n", myid);
  commonStream->print_sprng();

  /*********************** print random numbers ***************************/
            
  for (i=0;i<2;i++)		/* random numbers from distinct stream    */
  {
    rn = stream->sprng();      	/* generate double precision random number*/
    printf("Process %d, random number (distinct stream) %d: %f\n",
	   myid, i+1, rn);
  }

  for (i=0;i<2;i++)		/* random number from common stream       */
  {
    rn = commonStream->sprng();	/*generate double precision random number */
    printf("Process %d, random number (shared stream) %d: %f\n", myid, i+1, rn);
  }

  /*************************** free memory ********************************/
            
  stream->free_sprng();        /* free memory used to store stream state  */
  commonStream->free_sprng();

  MPI_Finalize();              /* terminate MPI                           */

  return 0;
}
