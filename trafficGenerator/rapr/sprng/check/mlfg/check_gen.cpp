//#define USE_MPI		/*	 Uncomment to test with MPI */

#ifdef USE_MPI
#include <mpi.h>
#endif

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "sprng_cpp.h"

using namespace std;


#ifdef VERBOSE
#define report printf
#else
#define report ignore
#endif

#define PARAM SPRNG_DEFAULT

#define YES 1
#define NO 0

#define diff(a,b) (((a)>(b))?((a)-(b)):((b)-(a)))

int gtype = 4;

void ignore(const char *s, ...)
{
}


int check_gen()			/* Check generator with correct parameters   */
{ 
  Sprng *gen1, *gen2, *gen3, *gen4, **gen5, **newgen1, **newgen2;
  int i, size, ret;
  char *s;
  int tempi, tempi2, correct, result = YES;
  int ngens, seed, nsp;
  float tempf1, tempf2;
  double tempd1;
  
  ngens = 3;
  seed = 985456376;

  gen1 = SelectType(gtype);
  gen2 = SelectType(gtype);
  gen3 = SelectType(gtype);
  gen4 = SelectType(gtype);

  gen1->init_sprng(0,ngens,seed,PARAM); /* initiallize generators           */  
  gen2->init_sprng(1,ngens,seed,PARAM);
  gen3->init_sprng(2,ngens,seed,PARAM);

  
  /* ____________________ Check arithmetic ___________________________       */

#ifdef CREATE_DATA
  for(i=0; i<500; i++) {       	/* generate integers                         */
    printf("%d\n", gen1->isprng());
  }

  for(i=0; i<500; i++)		/* generate floats                           */
    printf("%8.6f\n", gen1->get_rn_flt());

  for(i=0; i<500; i++)
    printf("%16.14f\n", gen1->sprng()); /* generate double precision numbers   */

#else
  correct = YES;

  for(i=0; i<500; i++)		/* check integer arithmetic                  */
  {
    tempi2 = gen1->isprng();
    ret = scanf("%d\n", &tempi);

    if(tempi != tempi2)
    {
      printf("%d. %d, %d\n", i, tempi, tempi2);
      result = correct = NO;
    }
  }
  
  if(correct == NO)
    printf("FAILED: Integer generator does not reproduce correct stream.\n\tArithmetic on this machine may not be compatible with this generator.\n");
  else
    report("PASSED: Integer generator passed the reproducibility test\n");

  correct = YES;

  for(i=0; i<500; i++)		/* check float arithmetic                    */
  {
    tempf1 = gen1->get_rn_flt();
    ret = scanf("%f\n", &tempf2);

    if(diff(tempf1,tempf2) > 1.0e-6)
    {
      printf("%d. %8.6f %8.6f\n", i, tempf2, tempf1);
      
      result = correct = NO;
    }
  }

  if(correct == NO)
    printf("FAILED: Float generator does not reproduce correct stream\n\tArithmetic on this machine may not be compatible with this generator.\n");
  else
    report("PASSED: Float generator passed the reproducibility test\n");

  correct = YES;

  for(i=0; i<500; i++)		/* check double precision arithmetic         */
  {
    double tempd2;
    
    ret = scanf("%lf\n", &tempd1);
    tempd2 = gen1->sprng();
    if(diff(tempd1,tempd2)>1.0e-14)
    {
      printf("%d. %18.15f %18.15f\n", i, tempd1, tempd2);
      
      result = correct = NO;
    }
    
  }

  if(correct == NO)
    printf("FAILED: Double generator does not reproduce correct stream.\n\tArithmetic on this machine may not be compatible with this generator.\n");
  else
    report("PASSED: Double generator spawns correctly\n");
#endif

  /* ____________________ Check spawning ___________________                 */

  nsp = 0;
  nsp += gen2->spawn_sprng(2, &newgen1); /* spawn new generators              */
  nsp += newgen1[1]->spawn_sprng(2,&newgen2);


#ifdef CREATE_DATA
  for(i=0; i<50; i++)	      /* generate numbers from new stream            */
    printf("%d\n", newgen2[1]->isprng());
#else
  if(nsp != 4)		     /* check if spawn_sprng returned correct value  */
  {
    result = NO;
    printf("FAILED: Generator was unable to spawn\n");
  }
  
  correct = YES;

  for(i=0; i<50; i++)	     /* check new stream                             */
  {
    int irn;
    ret = scanf("%d\n", &tempi);
    irn = newgen2[1]->isprng();

    if(tempi != irn) {
      printf("%d. %x %x\n", i, tempi, irn);
      result = correct = NO;
    }
  }

  if(correct == NO)
    printf("FAILED: Generator does not reproduce correct stream after spawning\n\tThis is probably an error in spawning the generators\n");
  else
    report("PASSED: Generator spawns correctly\n");
#endif
  

  /* _____________________ Pack and unpack generator ______________________  */

#ifdef CREATE_DATA
  for(i=0; i<50; i++)		/* generate from original stream             */
    printf("%d\n", newgen2[1]->isprng());

  newgen2[1]->spawn_sprng(1,&gen5); /* spawn from original stream             */

  for(i=0; i<50; i++)
    printf("%d\n", gen5[0]->isprng());
#else
  size = newgen2[1]->pack_sprng(&s); /* pack the original stream             */

  if(size == 0)			/* check if pack_sprng succeeded             */
  {
    result = NO;
    printf("FAILED: Generator was unable to pack\n");
  }

  gen4->unpack_sprng(s);	/* unpack generator                          */
 
  correct = YES;

  for(i=0; i<50; i++)	        /* check if unpacked stream = original stream*/
  {
    ret = scanf("%d\n", &tempi);

    if(tempi != gen4->isprng())
      result = correct = NO;
  }

 if(correct == NO)
    printf("FAILED: Generator does not reproduce correct stream after packing and unpacking\n\tThis is probably an error in packing/unpacking the generators\n");
  else
    report("PASSED: Generator packs and unpacks stream correctly\n");

  correct = YES;
  gen4->spawn_sprng(1,&gen5);	/* spawn from unpacked stream                */
  
  for(i=0; i<50; i++)		/* check if spawned stream is correct        */
  {
    ret = scanf("%d\n", &tempi);
    if(tempi != gen5[0]->isprng())
      result = correct = NO;
  }
  if(correct == NO)
    printf("FAILED: Generator does not spawn correct stream after packing and unpacking\n\tThis is probably an error in packing/unpacking the generators\n");
  else
    report("PASSED: Generator packs and unpacks spawning information correctly\n");
#endif


  /* _______________ Free generators ___________________                     */

#ifndef CREATE_DATA
  report("Checking free_sprng for integer generator ...\n");
  nsp = gen1->free_sprng();
  nsp = gen2->free_sprng();
  nsp = gen3->free_sprng(); 

  if(nsp != 6)	   /* check if free rng returns # of available generators    */
  {
    result = NO;
    printf("FAILED: Free returns %d instead of %d\n", nsp,6);
  }

  nsp = gen4->free_sprng();
  nsp = gen5[0]->free_sprng();
  nsp = newgen1[0]->free_sprng();
  nsp = newgen1[1]->free_sprng();
  nsp = newgen2[0]->free_sprng();
  nsp = newgen2[1]->free_sprng();

  if(nsp != 0)
  {
    result = NO;
    printf("FAILED: Free returns %d instead of %d\n", nsp,0);
  }
#endif
 
#ifndef CREATE_DATA
  report("\n... Completed checking generator \n\n");
#endif

  return result;
}


     /* Check if generator meets specifications in handling errors           */
int check_errors()
{
  Sprng *gen1, **gen2;
  int i, ret;
  int val;
  int tempi, correct, result = YES;
  int seed, nsp, size;
  char s[MAX_PACKED_LENGTH];
  
  seed = 985456376;
  
  /* ___________ ngens incorrect in init_sprng _____________                 */

  gen1 = SelectType(gtype);
  
#ifdef CREATE_DATA
  gen1->init_sprng(0,1,seed,PARAM); /* take ngens = 1 */

  for(i=0; i<50; i++)
    printf("%d\n", gen1->isprng());
#else
  correct = YES;
  fprintf(stderr,"Expect SPRNG WARNING: ngens <= 0.\n");
  gen1->init_sprng(0,0,seed,PARAM);
  for(i=0; i<50; i++)	 /* ngens should be reset to 1   */
  {
    ret = scanf("%d\n", &tempi);

    if(tempi != gen1->isprng())
      result = correct = NO;
  }

  if(correct == NO)
    printf("FAILED: Generator does not produce expected stream when ngens is 0 during initialization.\n");
  else
    report("PASSED: Generator produces expected stream when ngens is 0 during initialization.\n");

  nsp = gen1->free_sprng();     /* check if only one stream had been produced  */

  if(nsp != 0)
  {
    result = NO;
    printf("FAILED: Generator produces %d streams instead of 1 when ngens is 0 during initialization.\n",nsp+1);
  }
  else
    report("PASSED: Generator produces the correct number of streams when ngens is 0 during initialization.\n");
#endif

  /* _______________ invalid range for gennum _______________                */

#ifndef CREATE_DATA
  correct = YES;
  fprintf(stderr,"Expect SPRNG ERROR: gennum not in range\n");

  gen1 = SelectType(gtype);
  val = gen1->init_sprng(-1,1,seed,PARAM); /* negative gennum */

  if(val != 0)
  {
    gen1->free_sprng();
    result = correct = NO;
  }
  else
    gen1->free_sprng();
  
  fprintf(stderr,"Expect SPRNG ERROR: gennum not in range\n");

  gen1 = SelectType(gtype);

  val = gen1->init_sprng(2,1,seed,PARAM); /* gennum >= ngens */

  if(val != 0)
  {
    gen1->free_sprng();
    result = correct = NO;
  }
  
  if(correct == NO)
    printf("FAILED: Generator does not return NULL when gennum is incorrect during initialization.\n");
  else
    report("PASSED: Generator returns NULL when gennum is incorrect during initialization.\n");

#endif

  /* _______________ Invalid parameter ______________________________        */

#ifdef CREATE_DATA
  gen1->init_sprng(0,1,seed,SPRNG_DEFAULT); /* use default parameter */

  for(i=0; i<50; i++)
    printf("%d\n", gen1->isprng());
#else
  correct = YES;
  fprintf(stderr,"Expect SPRNG WARNING: Invalid parameter\n");

  gen1 = SelectType(gtype);
  gen1->init_sprng(0,1,seed,1<<30);

  for(i=0; i<50; i++)		/* check if default parameter is used ...    */
  {				/* ... when an invalid parameter is passed.  */
    ret = scanf("%d\n", &tempi);
    if(tempi != gen1->isprng())
      result = correct = NO;
  }
#endif

  /* _____________________ Check spawn with invalid ngens _________________ */
#ifdef CREATE_DATA
  gen_1->spawn_sprng(1,&gen2);	/* spawn one generator */

  for(i=0; i<50; i++)
    printf("%d\n", gen2[0]->isprng());
#else
  report("Checking spawn with incorrect nspawned\n");
  
  fprintf(stderr,"Expect SPRNG WARNING: nspawned <= 0.\n");
  nsp = gen1->spawn_sprng(0,&gen2);
  gen1->free_sprng();


  if(nsp != 1)			/* check if one generator was spawned */
  {
    result = NO;
    printf("FAILED: Spawn returned %d streams instead of 1 when nspawned was greater than permitted.\n", nsp);
  }
    
  
  for(i=0; i<50; i++)		/* check spawned stream */
  {
    ret = scanf("%d\n", &tempi);
    if(tempi != gen2[0]->isprng())
      result = correct = NO;
  }
  
  gen2[0]->free_sprng();
  
  if(correct == NO)
    printf("FAILED: Generator does not spawn correct stream when nspawned was 0.\n");
  else
    report("PASSED: Generator spawns correctly when nspawned was 0.\n");
#endif

  /* ____________________ Unpack invalid string _____________________ */

#ifndef CREATE_DATA
  memset(s,0,MAX_PACKED_LENGTH); /* set string to 0's */
  
  fprintf(stderr,"Expect SPRNG ERROR: packed string invalid\n");

  gen1 = SelectType(gtype);
  val = gen1->unpack_sprng(s);

  if(val != 0)	    /* NULL should be returned for invalid string */
  {
    result = NO;
    printf("FAILED: Generator unpacks invalid string\n");
  }
  else
    report("PASSED: Generator detected invalid string while unpacking\n");
#endif

  return result;
}



#ifdef USE_MPI
int check_mpi_seed(unsigned int seed)
{
  int nprocs, myid, result = YES, i, tag=0;
  MPI_Status status;
  unsigned int temp;
  
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  if(myid != 0)
    MPI_Send(&seed, 1, MPI_UNSIGNED, 0, tag, MPI_COMM_WORLD);
  else
    for(i=1; i<nprocs; i++)
    {
      MPI_Recv(&temp,1, MPI_UNSIGNED, i, tag, MPI_COMM_WORLD, &status);
      if(temp != seed)
	result = NO;
    }
  
  if(result == NO)
    printf("FAILED: Seeds returned by make_seed differ on different processors\n");
  else
    report("PASSED: Seeds returned my make_seed on all processes are equal.\n");
  
  return result;
}
#endif


main(int argc, char *argv[])
{
  int result=YES;
  
#ifndef CREATE_DATA
  int temp, myid;
  unsigned int seed1, seed2;
 
  report("Checking make_sprng_seed ...  ");

#ifdef USE_MPI
  MPI_Init(&argc, &argv);
#endif
  seed1 = make_sprng_seed();
#ifdef USE_MPI
  result = check_mpi_seed(seed1);
#endif

  seed2 = make_sprng_seed();
#ifdef USE_MPI
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);
  if(myid==0)
  {
#endif
  if(seed1 != seed2)
    report("  ... Checked make_sprng_seed\n");
  else
  {
    result = NO;
    printf("\nERROR: make_sprng_seed does not return unique seeds\n");
  }
#endif

  if(check_gen() != YES)
    result = NO;
  
  if(check_errors() != YES)
    result = NO;

#ifndef CREATE_DATA
  if(result == YES)
    printf("\nResult:\t PASSED\n\n");
  else
    printf("\nResult:\t FAILED\n\n");
#endif

#ifdef USE_MPI
}
  MPI_Finalize();
#endif
}

