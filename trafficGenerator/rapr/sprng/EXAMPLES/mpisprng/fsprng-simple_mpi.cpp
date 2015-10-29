/***************************************************************************/
/*         ____Demonstrates use of the single precision generator____      */
/* One stream is maintained per processor. Each processor prints a few     */
/* single precision random numbers.                                        */
/***************************************************************************/

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <mpi.h>                /* MPI header file                         */

#define SIMPLE_SPRNG		/* simple interface                        */
#define USE_MPI			/* use MPI to find number of processes     */
#define FLOAT_GEN	  /* make 'sprng()' return single precision numbers*/
#include "sprng_cpp.h"          /* SPRNG header file                       */

using namespace std;

#define SEED 985456376

int main(int argc, char *argv[])
{
  int i, myid;
  float rn;
  int gtype;  /*---    */

  /************************** MPI calls ***********************************/
            
  MPI_Init(&argc, &argv);       /* Initialize MPI                         */
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);	/* find process id                */

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

  init_sprng(SEED,SPRNG_DEFAULT,gtype);	/* initialize stream              */
  printf("Process %d: Print information about stream:\n",myid);
  print_sprng();

  /*********************** print random numbers ***************************/
            
  for (i=0;i<3;i++)
  {
    rn = sprng();		/*generate single precision random number */
    printf("Process %d, random number %d: %f\n", myid, i+1, rn);
  }

  MPI_Finalize();		/* Terminate MPI */

  return 0;
}
