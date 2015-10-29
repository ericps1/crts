/***************************************************************************/
/*       ____Demonstrates the use of make_seed with MPI____                */
/* 'make_sprng_seed' is called to produce a new seed each time the program */
/* is run. The same seed is produced on each process.                      */
/***************************************************************************/

#include <cstdio>
#include <mpi.h>                /* MPI header file                         */

#define SIMPLE_SPRNG		/* simple interface                        */
#define USE_MPI                 /* SPRNG makes MPI calls                   */
#include "sprng_cpp.h"          /* SPRNG header file                       */

using namespace std;


int main(int argc, char *argv[])
{
  int seed;
  double rn;
  int myid, i;
  int gtype;  /*---    */


  /*************************** MPI calls ***********************************/
            
  MPI_Init(&argc, &argv);      /* Initialize MPI */
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);	/* find process id */

  /************************** Initialization *******************************/

  seed = make_sprng_seed();	/* make new seed each time program is run  */

  /*--- node 0 is reading in a generator type */
  if(myid == 0)
  {
#include "gen_types_menu.h"
    printf("Type in a generator type (integers: 0,1,2,3,4,5):  ");
    i = scanf("%d", &gtype);
    if (i != 1)
    exit(1);
  }
  MPI_Bcast(&gtype,1,MPI_INT,0,MPI_COMM_WORLD ); /*--- broadcasting gen type */

  /* Seed should be the same on all processes                              */
  printf("\n\nProcess %d: seed = %16d\n", myid, seed);

  init_sprng(seed,SPRNG_DEFAULT,gtype);	/* initialize stream             */
  printf("Process %d: Print information about stream:\n",myid);
  print_sprng();

  /************************ print random numbers ***************************/

  for (i=0;i<3;i++)
  {
    rn = sprng();		/* generate double precision random number */
    printf("process %d, random number %d: %f\n", myid, i+1, rn);
  }

  MPI_Finalize();		/* Terminate MPI                           */

  return 0;
}
