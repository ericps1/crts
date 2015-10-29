/*********************************************************************
 *								     *
 *			  isingwolff.c				     *
 *								     *
 *********************************************************************/
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include "sprng_cpp.h"

using namespace std;


/*#define PARALLEL*/

int lattice_size, *spin, *stack, nsites;
Sprng **genptr;			/* array of random number generators */
int exponent, mask; /* Used for efficiency purposes */
double prob;
double Energy[10][10], Cv[10][10], J=0.4406868;
double exact_energy=-1.4530649029, exact_Cv=1.4987048885; /***** This is correct ONLY for a 16x16 lattice!!****** */

int RNG()	 /* Random number generator used for initializations alone */
{
  static int seed=17;
  seed=16807*(seed%127773)-(seed/127773)*2836;
  if(seed<0) 
    seed+=2147483647;
  else if(seed > 2147483647)
  {
    seed--;
    seed -= 2147483647;
  }
  
  return seed;
}


void Single_Cluster_Update(void) /* update lattice spins: a single sweep */
{
  static int nSite[4], Ipt=-1;
  int i, j, nnJ, ix, iy;
  
  i = RNG()>>(31-(exponent<<1)); 
  if(i<0) 
    i += nsites; 
  spin[i] = -spin[i];  
  
  while(i >= 0) 
  {
    /* ix=I/size; iy=I%size; */
    ix =i>>exponent; iy=i&mask;
    /* printf("ix=%d, iy=%d) ", ix, iy); */
    if(iy==0) nSite[0]=(ix<<exponent)+mask; /* nSite[0]=ix*size+size-1; */
    else nSite[0]=i-1;
    
    if(iy==lattice_size-1) nSite[2]=ix<<exponent; /* nSite[2]=ix*size; */
    else nSite[2]=i+1;
    
    if(ix==0) nSite[1]=(mask<<exponent)+iy; /* nSite[1]=(size-1)*size+iy; */
    else nSite[1]=i-lattice_size;
    
    if(ix==lattice_size-1) nSite[3]=iy;
    else nSite[3]=i+lattice_size;
    
    for(j=0; j<4; j++)
      {
	nnJ=nSite[j];
	if(spin[i]==spin[nnJ]) 
	  continue;
	/* notice prog should be scaled to MAXIMUM if necessary */
	if (genptr[i]->sprng() > prob)
	  continue;
	spin[nnJ]=-spin[nnJ];
	stack[++Ipt]=nnJ;
      }
    
    if(Ipt>=0)
      {
	i=stack[Ipt];
	Ipt--;
      }
    else i=-1;
  }
}


int System_Energy()		/* Compute energy of lattice */
{
  int E =0;
  int i, j, s;

  for(i=0, s=0;  i<lattice_size-1;  i++, s+=lattice_size)
    for(j=s; j<s+lattice_size; j++)
      if(spin[j]==spin[lattice_size+j])
	E-=1; 
      else
	E+=1;  
  
  for(i=0, s=0; i<lattice_size; i++, s+=lattice_size)
    for(j=s; j<s+lattice_size-1; j++)
      if(spin[j]==spin[j+1])
	E-=1; 
      else
	E+=1; 

  s=lattice_size*(lattice_size-1);
  for(i=0; i<lattice_size; i++)
    if(spin[i]==spin[s+i])
	E-=1;
    else
	E+=1;

  for(i=0, s=0; i<lattice_size; i++, s+=lattice_size)
    if(spin[s]==spin[s+lattice_size-1])
	E-=1;
    else
	E+=1;
  
  return E;
}



void compute(int i)		/* print results */
{
  int j;
  double average_energy, average_Cv, energy_error, Cv_error;

  average_energy = average_Cv = energy_error = Cv_error = 0.0;
  
  for(j=0; j<10; j++)
  {
    Cv[i][j] = J*J*(Cv[i][j] - Energy[i][j]*Energy[i][j]
		    *nsites*nsites)/nsites;
    average_Cv += Cv[i][j];
    Cv_error += Cv[i][j]*Cv[i][j];
    average_energy += Energy[i][j];
    energy_error += Energy[i][j]*Energy[i][j];
  }
 
  energy_error = sqrt((energy_error/10.0-Energy[i+1][0]*Energy[i+1][0])/9.0);
  average_Cv /= 10.0;
  Cv_error = sqrt((Cv_error/10.0 - average_Cv*average_Cv)/9.0);

  printf("%9d.\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\n",
	 i, Energy[i+1][0], fabs(Energy[i+1][0]-exact_energy), 
	 energy_error, average_Cv, fabs(average_Cv-exact_Cv), Cv_error);
}




void wolff(int block_size, int use_blocks)
{
  int i, j, k, row, col, old_row, energy, divisor;
  double average_E, average_Cv;
 
  printf("         \tEnergy\t\tEnergy_error\tSigma_Energy\tCv\t\tCv_error\tSigma_Cv\n");

  for(i=old_row=row=0,divisor=1; i<use_blocks; i++)
  {
    for(j=average_E=average_Cv=0; j<block_size; j++)
    {
      Single_Cluster_Update();	/* update lattice spins */
      energy = System_Energy(); 
      average_E += energy;
      average_Cv += energy*energy; 
    }

    average_E /= (double) block_size*nsites; /*compute average of quantities */
    average_Cv /= (double) block_size; 

    if(i>=10*divisor)	/* make scale logarithmic for printing results */
    {
      divisor *= 10;
      row++;
    }
    
    col = i/divisor;
    average_E /= divisor;
    average_Cv /= divisor;
    Energy[row][col] += average_E;
    Cv[row][col] += average_Cv;

    for(k=row+1; k<10; k++)
    {
      average_E /= 10;
      average_Cv /= 10;
      Energy[k][0] += average_E;
      Cv[k][0] += average_Cv;
    }
    
    if(old_row != row)
    {
      compute(old_row);
      old_row = row;
    }
  }
      compute(old_row);
}

/*--- in order not to duplicate with initialize ---*/
/*--- change from initialize() to minitialize() ---*/
void minitialize(int rng_type, int seed, int param, int use_blocks)
{
  int i, j, temp;
  
  nsites = lattice_size*lattice_size;
  prob = 1 - exp(-2.0*J);

  for(i=0; i<10; i++)
    for(j=0; j<10; j++)
      Energy[i][j] = Cv[i][j] = 0.0;
  

  spin = static_cast<int *>(malloc(nsites*sizeof(int)));
  stack = static_cast<int *>(malloc(nsites*sizeof(int)));
  if(!spin || !stack)
  {
    printf("\n\tMemory allocation failure, program exits!\n");
    exit(-1);
  }

  
  for(i=0; i<nsites; i++)   /* randomly initialize system */
    spin[i]=(RNG()>prob)?1:-1;

  /* here assume that expo is integer exponent of 2 */
  temp = mask=lattice_size-1;
  exponent = 0;			/* expo = log_2(lattice_size) */
  while(temp)
  {
    exponent++;
    temp >>= 1;
  }
  
  /* initialize generator */
  genptr = (Sprng **) malloc(nsites*sizeof(Sprng *));
  genptr[0] = SelectType(rng_type);
  genptr[0]->init_sprng(0,nsites,seed,param); /*--- add rng_type ---*/
  genptr[0]->print_sprng();
  for(i=1; i<nsites; i++)
#ifdef PARALLEL
    genptr[i] = SelectType(rng_type);
    genptr[i]->init_sprng(i,nsites,seed,param); /*--- add rng_type ---*/
#else
    genptr[i]=genptr[0]; 
#endif
}


void check_arguments(int lattice_size, int block_size, int discard_blocks, 
		     int use_blocks)
{
  if(lattice_size<=0)
  {
    printf("ERROR: lattice_size %d should be > 0\n", lattice_size);
    exit(-1);
  }
  if(block_size<=0)
  {
    printf("ERROR: Block_size %d should be > 0\n", block_size);
    exit(-1);
  }
  if(discard_blocks<=0)
  {
    printf("ERROR: discard_blocks %d should be > 0\n", discard_blocks);
    exit(-1);
  }
  if(use_blocks<=0)
  {
    printf("ERROR: use_blocks %d should be > 0\n", use_blocks);
    exit(-1);
  }
  if((lattice_size&(lattice_size-1)) != 0) /* check if lattice_size = 2^n */
  {
    printf("ERROR: lattice_size %d should be a positive power of 2\n", 
	   lattice_size);
    exit(-1);
  }
  if(lattice_size!=16)
  {
    printf("WARNING: The current code gives error values correctly only for a 16x16 lattice.\n\t... The Energy_error and Cv_error columns are incorrect.\n\t... Please use the Energy and Cv values and compute error from the exact solution.\n");
  }
  while(use_blocks)		/* check if use_blocks is a power of 10 */
  {
    if(use_blocks%10 != 0 && use_blocks!=1)
    {
      printf("ERROR: use_blocks %d should be a power of 10\n", use_blocks);
      exit(-1);
    }
    use_blocks /= 10;
  }
}



  /************** 'Thermalize' system so that results are not influenced
    by the initial onditions                              *************/
void thermalize(int block_size, int discard_blocks)
{
  int i, j;
  
  for(i=0; i<discard_blocks; i++)
    for(j=0; j<block_size; j++)
      Single_Cluster_Update();
}




/* block_size*use_blocks sweeps through a lattice of size 
   lattice_size*lattice_size using the Wolff algorithm for the Ising model */

int main(int argc, char **argv)
{
  /*--- Add rng_type as the argument to the new interface ---*/
  int rng_type;
  int seed, param, block_size, discard_blocks, use_blocks;
  
  /****************** Read and check Arguments ********************/
  if(argc==8 ) /*--- increase argc by 1 ---*/
  {
    argv++;
    rng_type = atoi(*argv++); /*--- get rng_type ---*/
    seed = atoi(*argv++);
    param = atoi(*argv++);
    lattice_size = atoi(*argv++);
    block_size = atoi(*argv++);
    discard_blocks = atoi(*argv++);
    use_blocks = atoi(*argv++);
    check_arguments(lattice_size, block_size, discard_blocks, use_blocks);
#ifdef PARALLEL
    printf("Wolff Algorithm with Parallel RNG\n");
#else
    printf("Wolff Algorithm with Serial RNG\n");
#endif
    printf("lattice_size = %d, block_size = %d, discard_blocks = %d, use_blocks = %d\n", lattice_size, block_size, discard_blocks, use_blocks);
  }
  else
  {
    printf("USAGE: %s rng_type seed param lattice_size block_size discard_blocks use_blocks\n", argv[0]);
    exit(-1);
  }


  minitialize(rng_type, seed, param, use_blocks); /* initalize data  */
  

  /************** 'Thermalize' system so that results are not influenced
    by the initial onditions                              *************/
  thermalize(block_size, discard_blocks);
  
  /********** Perform the actual Wolff algorithm calculations *********/
  wolff(block_size, use_blocks);

  return 0;
}
