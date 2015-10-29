/***************************************************************************/
/*            ____Demonstrates converting code to SPRNG____                */
/* The original random number call is to 'myrandom'. We change it to call  */
/* SPRNG by defining a macro.                                 */

/* The lines between the '#ifdef CONVERT' and the '#else' are the
   newly added lines. Those lines between the '#else' and the '#endif'
   are theoriginal lines that need to be deleted.
*/
/***************************************************************************/

#include <cstdio>
#define CONVERT        /* used to set on the macro 'myrandom --> sprng'    */ 
#ifdef CONVERT
#define SIMPLE_SPRNG   /* simple interface                        */
#include "sprng_cpp.h" /* SPRNG header file                       */

using namespace std;


#define myrandom sprng /* we define this macro to make SPRNG calls*/
#endif

#define SEED 985456376

double myrandom();

int main()
{
  int seed, i, ret;
  double rn;
  int gtype;  /*---    */

#ifdef CONVERT
  /*--- reading in a generator type */
#include "gen_types_menu.h"
  printf("Type in a generator type (integers: 0,1,2,3,4,5):  ");
  ret = scanf("%d", &gtype);

  /************************** Initialization *******************************/
  /******* We add the following optional initialization lines **************/
  init_sprng(SEED,SPRNG_DEFAULT, gtype);  /* initialize stream   */
  printf("Print information about random number stream:\n");
  print_sprng();
#else
  /* Old initialization lines*/
#endif    
        
  /*********************** print random numbers ****************************/

  printf("Printing 3 random numbers in [0,1):\n");
  for (i=0;i<3;i++)
    {
      rn = myrandom();/* generate double precision random number */
      printf("%f\n", rn);
    }

  return 0;
}

