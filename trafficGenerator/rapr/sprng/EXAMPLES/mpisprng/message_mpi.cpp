/***************************************************************************/
/*        ____Demonstrates passing a stream to another process____         */
/* Process 0 initializes a random number stream and prints a few random    */
/* numbers. It then passes this stream to process 1, which recieves it     */
/* and prints a few random numbers from this stream.                       */ 
/***************************************************************************/

#if __GNUC__ > 3
 #include <string.h>
 #include <iostream>
#else
 #include <iostream.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>		/* MPI header file                         */

#include "sprng_cpp.h"          /* SPRNG header file                       */

#define SEED 985456376


int main(int argc, char *argv[])
{
  int streamnum, nstreams;
  Sprng *stream;
  double rn;
  int i, myid, nprocs, len;
  MPI_Status  status;
  char *packed;
  int gtype;  
            
  MPI_Init(&argc, &argv);	
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  if(nprocs < 2)
  {
    fprintf(stderr,"ERROR: At least 2 processes required\n");
    MPI_Finalize();
    exit(1);
  }

  if(myid == 0)
  {
#include "gen_types_menu.h"
    printf("Type in a generator type (integers: 0,1,2,3,4,5):  ");
    i = scanf("%d", &gtype);
    if (i != 1)
	    exit(1);
  }
  MPI_Bcast(&gtype,1,MPI_INT,0,MPI_COMM_WORLD );

  if (myid==0)	
  {
    streamnum = 0;
    nstreams = 1;
    stream = SelectType(gtype);
    stream->init_sprng(streamnum,nstreams,SEED,SPRNG_DEFAULT);
    printf("\n\nProcess %d: Print information about stream:\n",myid);
    stream->print_sprng();

    printf("Process %d: Print 2 random numbers in [0,1):\n", myid);
    for (i=0;i<2;i++)
    {
      rn = stream->sprng();	
      printf("Process %d: %f\n", myid, rn);
    }

    len = stream->pack_sprng(&packed); 
    MPI_Send(&len, 1, MPI_INT, 1, 0, MPI_COMM_WORLD); 
    MPI_Send(packed, len, MPI_BYTE, 1, 0, MPI_COMM_WORLD); 

    free(packed);
    nstreams = stream->free_sprng();

    printf(" Process 0 sends stream to process 1\n");
    printf(" %d generators now exist on process 0\n", nstreams);
  }
  else if(myid == 1)
  { 
    MPI_Recv(&len, 1, MPI_INT, 0, MPI_ANY_TAG,
             MPI_COMM_WORLD, &status);

    if ((packed = (char *) malloc(len)) == NULL)
    {
      fprintf(stderr,"ERROR: process %d: Cannot allocate memory\n", myid);
      MPI_Finalize();
      exit(1);
    }

    MPI_Recv(packed, len, MPI_BYTE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status); 
    
    stream = SelectType(gtype);
    stream->unpack_sprng(packed); 

    printf(" Process 1 has received the packed stream\n");
    printf("Process %d: Print information about stream:\n",myid);
    stream->print_sprng();
    free(packed);

    printf(" Process 1 prints 2 numbers from received stream:\n");
    for (i=0;i<2;i++)		
      {
	rn = stream->sprng();
	printf("Process %d: %f\n", myid, rn);
      }
   
      stream->free_sprng();   
  }

  MPI_Finalize();		

  return 0;
}
