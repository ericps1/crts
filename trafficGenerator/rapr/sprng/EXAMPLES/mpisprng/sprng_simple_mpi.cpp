/***************************************************************************/
/*           Demonstrates sprng use with one stream per process            */
/* A distinct stream is created on each process, then prints a few         */
/* random numbers.                                                         */
/***************************************************************************/

#include <iostream>
#include <cstdio>
#include <mpi.h>                /* MPI header file                         */

#define SIMPLE_SPRNG		/* simple interface                        */
#define USE_MPI			/* use MPI to find number of processes     */
#include "sprng.h"

#define SEED 985456376

using namespace std;

int main(int argc, char *argv[])
{
  double rn;
  int i, myid;
  int gtype;  /*---    */

  /*************************** MPI calls ***********************************/
            
  MPI::Init(argc, argv);       /* Initialize MPI                          */
  myid = MPI::COMM_WORLD.Get_rank(); /* find process id                 */

  /************************** Initialization *******************************/
  
  /*--- node 0 is reading in a generator type */
  if(myid == 0)
  {
#include "gen_types_menu.h"
    printf("Type in a generator type (integers: 0,1,2,3,4,5):  ");
    i = scanf("%d", &gtype);
  }


  MPI::COMM_WORLD.Bcast(&gtype,1,MPI::INT,0);
  //  MPI_Bcast(&gtype,1,MPI_INT,0,MPI_COMM_WORLD );

  init_sprng(SEED,SPRNG_DEFAULT,gtype);	/* initialize stream        */
  printf("\n\nProcess %d, print information about stream:\n", myid);
  print_sprng();

  /************************ print random numbers ***************************/
            
  for (i=0;i<3;i++)
  {
    rn = sprng();		/* generate double precision random number */
    printf("Process %d, random number %d: %.14f\n", myid, i+1, rn);
  }

  MPI::Finalize();		/* Terminate MPI                           */

  return 0;
}
