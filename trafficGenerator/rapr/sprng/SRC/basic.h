void free_bn_array(struct BIGNUM_ARRAY_TYPE *array);

/* This function merges two arrays */
extern struct BIGNUM_ARRAY_TYPE join(struct BIGNUM_ARRAY_TYPE first, struct BIGNUM_ARRAY_TYPE second);

/* This function multiplies all elements by -1 */
extern struct BIGNUM_ARRAY_TYPE minus(BigNum * list, long size);

/* This function multiplies and reduces an array */
extern struct BIGNUM_ARRAY_TYPE mult_reduce(BigNum * number, struct BIGNUM_ARRAY_TYPE set, BigNum * limit);

/* This makes a list of denominators out of a list of primes */
extern struct BIGNUM_ARRAY_TYPE find_denom(BigNum * the_primes, long maxsize, BigNum *limit);

/* Evaluates Mu function, given denominators */
extern void mu_eval(BigNum * result, BigNum * x, struct BIGNUM_ARRAY_TYPE denominators);

/* Checks if x == 0 mod y, returns true if so */
extern short divisible(BigNum * x, BigNum * y);

/* Checks if any primes divide a number */
extern short any_divide(BigNum * number, BigNum * the_primes, long no_of_primes);

extern void incr(BigNum * x, long num);
extern void decr(BigNum * x, long num);

/* Uses a linear search to find inverse value given guess, mu(guess) */
extern BigNum linear_find(BigNum * y, BigNum * current_mu, BigNum * guess, struct BIGNUM_ARRAY_TYPE the_primes);

/* This guesses at mu_inverse */
extern BigNum guess_mu_inverse(BigNum * y, BigRat * magic);

/* Efficient caluculation of mu */
extern void Mu(BigNum * result, BigNum * x, struct BIGNUM_ARRAY_TYPE left_to_kill, struct BIGNUM_ARRAY_TYPE the_final_denom);

/* finds the desired result, given y, the primes to be killed, the denominators,
   the full set of initial primes, and the density of relative primes */
extern BigNum find_M(BigNum * guess, BigNum * y, struct BIGNUM_ARRAY_TYPE left_to_kill,
		     struct BIGNUM_ARRAY_TYPE denom, struct BIGNUM_ARRAY_TYPE initial_primes,
		     BigRat *magic);

/* setup. Note: 'int param' must be another argument if modulii other than 2^61-1 will be used */
extern void init_rel_prime(REL_PRIME_TABLE * data, BigNum * maxval);

void free_rel_prime(REL_PRIME_TABLE *data);

/* find nth number relatively prime, given data */
extern int rel_prime(BigNum * result, BigNum * number, REL_PRIME_TABLE data);

/* find nth primitive element, given data */
extern void prim_elt(BigNum * result, BigNum * number, REL_PRIME_TABLE data);


