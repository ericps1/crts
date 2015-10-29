/*--- Modified by J. Ren: Sept 2006 */
/*--- Chris S.: June 10, 1999   */
/*---  reads in a generator type as an integer */
/*---adding 'int gentype' and read in + error handling */
/*--- generator numeral range is fix-set to 0-5 ?  */
/*--- should 5 be change to some variable MAX_GEN_NUMBER ? */

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include "sprng_cpp.h"
#include "cputime.h"

#define TIMING_TRIAL_SIZE 1000000
#define PARAM 0

using namespace std;


int main()
{

  int i;
  Sprng *gen;
  double temp1, temp2, temp3, temp4;
  double temp_mult = TIMING_TRIAL_SIZE/1.0e6;
/*---   */
  int gentype = 3;
  
  //  scanf("%d\n", &gentype);
  gen = SelectType(gentype);

/*---   */  

  gen->init_rng(0,1,0,PARAM);

/*--- Printing generator and stream information */
/*  gen->print_sprng(); */

  temp1 = cputime();

  for(i=0; i<TIMING_TRIAL_SIZE; i++)
    gen->get_rn_int();
  
  temp2 = cputime();
  

  for(i=0; i<TIMING_TRIAL_SIZE; i++)
    gen->get_rn_flt();
  
  temp3 = cputime();
  

  for(i=0; i<TIMING_TRIAL_SIZE; i++)
    gen->get_rn_dbl();
  
  temp4 = cputime();
  
  if(temp2-temp1 < 1.0e-15 || temp3-temp2 < 1.0e-15 ||  temp4-temp3 < 1.0e-15)
  {
    printf("Timing Information not available/not accurate enough.\n\t...Exiting\n");
    exit(1);
  }

  /* The next line is just used to ensure that the optimization does not remove the call to the RNG. Nothing is really printed.             */
  fprintf(stderr,"Last random number generated\n", gen->get_rn_dbl());

  printf("\nUser + System time Information (Note: MRS = Million Random Numbers Per Second)\n");
  printf("\tInteger generator:\t Time = %7.3f seconds => %8.4f MRS\n", 
	 temp2-temp1, temp_mult/(temp2-temp1));
  printf("\tFloat generator:\t Time = %7.3f seconds => %8.4f MRS\n", 
	 temp3-temp2, temp_mult/(temp3-temp2));
  printf("\tDouble generator:\t Time = %7.3f seconds => %8.4f MRS\n", 
	 temp4-temp3, temp_mult/(temp4-temp3));
  putchar('\n');

  return 0;  
}
