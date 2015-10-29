/****************************************************************************/
/*               ____Demonstrates the use make_sprng_seed____               */
/* 'make_sprng_seed' is used to produce a new seed each time the program is */
/* run. Then a few random numbers are printed.                              */
/****************************************************************************/

#include <iostream>
#include <cstdio>

#include "sprng_cpp.h"          /* SPRNG header file                       */

using namespace std;


int main()
{
  int streamnum, nstreams, seed, i, j, scan_ret;
  Sprng *stream;
  double rn;
  int gtype;  /*---    */


  /*--- reading in a generator type */
#include "gen_types_menu.h"
  printf("Type in a generator type (integers: 0,1,2,3,4,5):  ");
  scan_ret = scanf("%d", &gtype);
  if (scan_ret != 1) {
    printf("Generator type entered is not valid\n");
    exit(1);
  }

  /*
for(j = 0; j < 6; j++){
  */
  /************************** Initialization *******************************/

  streamnum = 0;
  nstreams = 1;

  seed = make_sprng_seed();	/* make new seed each time program is run  */
  stream = SelectType(gtype);

  stream->init_sprng(streamnum,nstreams,seed,SPRNG_DEFAULT); /*initialize stream*/
  printf(" Printing information about new stream\n");
  stream->print_sprng();

  /************************ print random numbers ***************************/

  printf(" Printing 3 random numbers in [0,1):\n");

  for (i=0;i<3;i++)
  {
    rn = stream->sprng();		/* generate double precision random number */
    printf("%f\n", rn);
  }

  stream->free_sprng();		/* free memory used to store stream state  */

  return 0;
}
