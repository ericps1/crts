#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "info.h"
#include "bignum.h"
#include "bigrat.h"

#define LINEAR_SEARCH_LIMIT 5000
#define MOST 18

typedef struct DATATYPE1 {
  long power;
  BigNum *valid;
  struct BIGNUM_ARRAY_TYPE denom;
  struct BIGNUM_ARRAY_TYPE the_primes;
  struct BIGNUM_ARRAY_TYPE kill_us;
  BigNum *prim;
  BigRat *magic;
} REL_PRIME_TABLE;


void free_bn_array(struct BIGNUM_ARRAY_TYPE *array)
{
  long i;
  
  for(i=0; i<array->size; i++)
    (array->list[i]).b_clear();
  
  delete [] array->list;
}


/* This function merges two arrays */
extern struct BIGNUM_ARRAY_TYPE join(struct BIGNUM_ARRAY_TYPE first, struct BIGNUM_ARRAY_TYPE second)
{
  struct BIGNUM_ARRAY_TYPE result;
  long count1 = 0; 
  long count2 = 0;
  BigNum absfirst, abssecond;

  result.size = first.size + second.size;
  //  result.list = static_cast<BigNum *>(malloc(result.size * sizeof(BigNum)));
  result.list = new BigNum[result.size];

  if ((count1 < first.size) && (count2 < second.size)) {
    absfirst = b_abs(first.list[count1]);
    abssecond = b_abs(second.list[count2]);
  }

  while ((count1 < first.size) && (count2 < second.size)) {
    if (b_cmp(absfirst, abssecond) > 0) {
      result.list[count1+count2] = second.list[count2];
      ++count2;

      if (count2 < second.size) 
	abssecond = b_abs(second.list[count2]);
    }
    else {
      result.list[count1+count2] = first.list[count1];
      ++count1;

      if (count1 < first.size)
	absfirst = b_abs(first.list[count1]);
    }
  }

  absfirst.b_clear();
  abssecond.b_clear();

  for (;count1<first.size;count1++) 
    result.list[count1+count2] = first.list[count1];

  for (;count2<second.size;count2++) 
    result.list[count1+count2] = second.list[count2];

  return(result);
}

/* This function multiplies all elements by -1 */
extern struct BIGNUM_ARRAY_TYPE minus(BigNum * list, long size)
{
  long count;
  struct BIGNUM_ARRAY_TYPE result;

  result.size = size;
  //  result.list = static_cast<BigNum *>(malloc(size * sizeof(BigNum)));
  result.list = new BigNum[size];

  for (count=0;count<size;count++) {
    result.list[count] = 0ul;
    result.list[count] = b_neg(list[count]);
  }

  return(result);
}

/* This function multiplies and reduces an array */
extern struct BIGNUM_ARRAY_TYPE mult_reduce(BigNum * number, struct BIGNUM_ARRAY_TYPE set, BigNum * limit)
{
  struct BIGNUM_ARRAY_TYPE result;
  long count, count2;
  BigNum plussed/*, temp*/;
  
#if 0
  number = b_neg(number);       /* multiply by -number, except with 1 */
  temp = b_abs(number);
#endif

  //  result.list = static_cast<BigNum *>(malloc(set.size * sizeof(BigNum)));
  result.list = new BigNum[set.size];
  count2 = 0;

  if (set.size > 0) {
    result.list[count2] = 0ul;

    for (count=0;count<set.size;count++) 
      {
	/*if(mpz_cmp_ui(set.list[count],1U) != 0)*/
	  result.list[count2] = set.list[count] * (*number);
	/*else
	  result.list[count2]) = set.list[count] * temp; */
      	  plussed = b_abs(result.list[count2]);

	  if (b_cmp(plussed, *limit) < 0) {
	    ++count2;
	    
	    if (count2 < set.size)
	      result.list[count2] = 0ul;
	  }
      }
  }

  result.size = count2;
  plussed.b_clear();
  /* temp.b_clear(); */

  return(result);
}


/* This makes a list of denominators out of a list of primes */
extern struct BIGNUM_ARRAY_TYPE find_denom(BigNum * the_primes, long maxsize, BigNum *limit)
{
  struct BIGNUM_ARRAY_TYPE newones, oldones, answer;
  int count;

  if (maxsize > 1)
    oldones = find_denom(&(the_primes[1]), maxsize-1, limit);
  else {
    oldones.size = 1;
    //    oldones.list = static_cast<BigNum *>(malloc(sizeof(BigNum)));
    oldones.list = new BigNum[1];
    oldones.list[0] = 1ul;
  }

  newones = mult_reduce(the_primes, oldones, limit);
  answer = join(oldones, newones);
    
  free_bn_array(&newones);
  free_bn_array(&oldones);

  return(answer);
}

/* Evaluates Mu function, given denominators */
extern void mu_eval(BigNum * result, BigNum * x, struct BIGNUM_ARRAY_TYPE denominators)
{
  BigNum val;
  long count = 0;

  *result = 0ul;
  val = 1ul;

  while ((count < denominators.size) && (b_cmp(val, static_cast<unsigned long int>(0)) != 0)) {
    val = *x / denominators.list[count];
    *result = *result + val;
    count++;
  }

  val.b_clear();
}

/* Checks if x == 0 mod y, returns true if so */
extern short divisible(BigNum * x, BigNum * y)
{
  return (*x % *y == static_cast<unsigned long int>(0));
}

/* Checks if any primes divide a number */
extern short any_divide(BigNum * number, BigNum * the_primes, long no_of_primes)
{
  if (no_of_primes == 1) {
    return(divisible(number, &(the_primes[0])));
  }
  else {
    if (divisible(number, &(the_primes[no_of_primes-1]))) {
      return(1);
    }
    else {
      return(any_divide(number, the_primes, no_of_primes - 1));
    }
  }
}

extern void incr(BigNum * x, long num)
{
  *x = *x + num;
}

extern void decr(BigNum * x, long num)
{
  *x = *x - num;
}

/* Uses a linear search to find inverse value given guess, mu(guess) */
extern BigNum linear_find(BigNum * y, BigNum * current_mu, BigNum * guess, struct BIGNUM_ARRAY_TYPE the_primes)
{
  while (b_cmp(*current_mu, *y) < 0) {
    incr(guess, 1L);
    //    cout << *y << *current_mu << *guess;
        
    if (any_divide(guess, the_primes.list, the_primes.size)) {
    }
    else {
      incr(current_mu, 1L);
    }
  }

  while (b_cmp(*current_mu, *y) > 0) {
    if (any_divide(guess, the_primes.list, the_primes.size))
      decr(guess, 1L);
    else {
      decr(guess, 1L);
      decr(current_mu, 1L);
    }
  }

  while (any_divide(guess, the_primes.list, the_primes.size)) {
    decr(guess, 1L);
  }
  
  return(*guess);  
}

/* This guesses at mu_inverse */
extern BigNum guess_mu_inverse(BigNum * y, BigRat * magic)
{
  BigNum temp, guess;

  temp = magic->br_get_num();
  guess = (*y) * temp;
  temp = magic->br_get_den();
  guess = guess / temp;
  temp.b_clear();
  
  return(guess);
}   


/* Efficient caluculation of mu */
extern void Mu(BigNum * result, BigNum * x, struct BIGNUM_ARRAY_TYPE left_to_kill, struct BIGNUM_ARRAY_TYPE the_final_denom)
{
  BigNum subout, first, second;
  struct BIGNUM_ARRAY_TYPE new_kill;

  if (left_to_kill.size <= 0) {
    mu_eval(result, x, the_final_denom);
  }
  else {
    subout = (*x) / left_to_kill.list[0];
    new_kill.size = left_to_kill.size - 1;
    new_kill.list = &(left_to_kill.list[1]);
    Mu(&first, x, new_kill, the_final_denom);
    Mu(&second, &subout, new_kill, the_final_denom);
    *result = first - second;
    first.b_clear();
    second.b_clear();
    subout.b_clear();
  }
}

/* finds the desired result, given y, the primes to be killed, the denominators,
   the full set of initial primes, and the density of relative primes */
extern BigNum find_M(BigNum * guess, BigNum * y, struct BIGNUM_ARRAY_TYPE left_to_kill,
		     struct BIGNUM_ARRAY_TYPE denom, struct BIGNUM_ARRAY_TYPE initial_primes,
		     BigRat *magic)
{
  BigNum temp, temp2, current_mu;

#if 0
  Mu(&current_mu, guess, left_to_kill, denom); 
  temp = y - current_mu;
  temp2 = b_abs(temp);

  while (b_cmp(temp2, LINEAR_SEARCH_LIMIT) > 0) {
    temp2 = guess_mu_inverse(&temp, magic);
    (*guess) = (*guess) + temp2;

    Mu(&current_mu, guess, left_to_kill, denom);

    temp = y - current_mu;
    temp2 = b_abs(temp);
  }

  temp = b_clear();
  temp2 = b_clear();

#else

  *guess = 11ul;
  current_mu = 1ul;
#endif
  return(linear_find(y, &current_mu, guess, initial_primes));
} 

/* setup. Note: 'int param' must be another argument if modulii other than 2^61-1 will be used */
extern void init_rel_prime(REL_PRIME_TABLE * data, BigNum * maxval)
{
  /* modified by cmdavis */
  BigNum limit, num, denom;
  struct BIGNUM_ARRAY_TYPE the_primes;
	
  /* initialize 'data.power' */ 
  (*data).power = POWER_N;

  /* initialize 'data.valid' */
  (*data).valid = new BigNum[1];
  (*data).valid = maxval;

  /* initialize 'data.prim'  */
  (*data).prim = new BigNum[1];
  *((*data).prim) = Set_ui(PRIM);

  /* initialize 'data.magic' (density of relative primes) */
  (*data).magic = new BigRat[1];
  num = (char *)MAGIC_NUM;
  denom = (char *)MAGIC_DEN;
  (*((*data).magic)).br_set_num(num);
  (*((*data).magic)).br_set_den(denom);

  /* initialize 'data.the_primes[]' */
  (*data).the_primes = init_factors();

  /* initialize 'data.kill_us[]' */	
  (*data).kill_us.size = (*data).the_primes.size - MOST;

  if ((*data).kill_us.size < 0)           
    (*data).kill_us.size = 0;

  (*data).kill_us.list = (*data).the_primes.list;
 
  /* initialize 'data.denom[]' */ 
  the_primes = minus(&((*data).the_primes.list[(*data).kill_us.size]),
		     (*data).the_primes.size - (*data).kill_us.size); 

  /* define maximum value for which setup is valid */
  limit = guess_mu_inverse(maxval, (*data).magic);
  limit = limit * 2;
  (*data).denom = find_denom(the_primes.list, the_primes.size, &limit);

  limit.b_clear();
  num.b_clear();
  denom.b_clear();
  free_bn_array(&the_primes);
}

void free_rel_prime(REL_PRIME_TABLE *data)
{
  (data->valid)->b_clear();
  (data->prim)->b_clear();
  delete [] data->magic;
  free_bn_array(&(data->the_primes));
  free_bn_array(&(data->denom));
}

/* find nth number relatively prime, given data */
extern int rel_prime(BigNum * result, BigNum * number, REL_PRIME_TABLE data)
{
  BigNum guess;
  int i;
  REL_PRIME_TABLE newdata;
  *result = 0ul;

  if (b_cmp(*(data.valid), *number) >= 0) {
    guess = (*(data.magic)).br_get_num();
    guess = guess_mu_inverse(number, data.magic);

    *result = find_M(&guess, number, 
		     data.kill_us, data.denom, 
		     data.the_primes, data.magic);
  }
  else {
    init_rel_prime(&newdata, number);
    rel_prime(result, number, newdata);
    free_rel_prime(&newdata);
  }

  /* Do NOT clear guess, since result uses its component array */

  return 0;
}

/* find nth primitive element, given data */
extern void prim_elt(BigNum * result, BigNum * number, REL_PRIME_TABLE data)
{
  BigNum x, pow,temp;
  x = 2ul;
  x = b_pow(x, data.power);
  x = x - 1;
  temp = *number;
  temp = temp + 5UL;             /* first few streams are bad; so offset by 5 streams */
  rel_prime(&pow, &temp, data);
  *result = b_powm(*(data.prim), pow, x);  

  pow.b_clear();
  temp.b_clear();
}

