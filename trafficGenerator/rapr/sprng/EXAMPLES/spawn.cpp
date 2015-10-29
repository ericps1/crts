/***************************************************************************/
/*            ____Demonstrates the use of spawn_sprng____                  */
/* A random number stream is initialized and a few random numbers are      */
/* printed. Then two streams are spawned and a few numbers from one of them*/
/* is printed.                                                            */
/***************************************************************************/

#include <iostream>
#include <cstdio>

#include "sprng_cpp.h"  /* SPRNG header file                               */

#define SEED 985456376

using namespace std;


int main()
{
  int streamnum, nstreams;
  Sprng *stream, **newobj;
  double rn;
  int i, irn, nspawned, scan_ret;
  int gtype;  /*---    */ 

 /*--- reading in a generator type */
#include "gen_types_menu.h"
  printf("Type in a generator type (integers: 0,1,2,3,4,5):  ");
  scan_ret = scanf("%d", &gtype);
  if (scan_ret != 1) {
    printf("Generator type entered is not valid\n");
    exit(1);
  }

  /****************** Initialization values *******************************/
           
  streamnum = 0;
  nstreams = 1;

  stream = SelectType(gtype);
  stream->init_sprng(streamnum,nstreams,SEED,SPRNG_DEFAULT); /* initialize stream */

  printf(" Print information about stream:\n");
  stream->print_sprng();	

  /*********************** print random numbers ***************************/

  printf(" Printing 2 random numbers in [0,1):\n");

  for (i=0;i<2;i++)
  {
    rn = stream->sprng();	/* generate double precision random number*/
    printf("%f\n", rn);
  }

  /**************************** spawn streams *****************************/

  printf(" Spawned two streams\n");
  nspawned = 2;
  nspawned = stream->spawn_sprng(2,&newobj); /* spawn 2 streams               */

  if(nspawned != 2)
  {
    fprintf(stderr,"Error: only %d streams spawned\n", nspawned);
    exit(1);
  }

  printf(" Information on first spawned stream:\n");
  newobj[0]->print_sprng();
  printf(" Information on second spawned stream:\n");
  newobj[1]->print_sprng();
  

  printf(" Printing 2 random numbers from second spawned stream:\n");

  for (i=0;i<2;i++)
  {
    rn = newobj[1]->sprng();	/* generate a random number               */
    printf("%f\n", rn);
  }

  /*************************** free memory ********************************/

  stream->free_sprng();        /* free memory used to store stream state  */
  newobj[0]->free_sprng();     /* free memory used to store stream state  */
  newobj[1]->free_sprng();     /* free memory used to store stream state  */
  //  delete [] newobj;

  return 0;
}

