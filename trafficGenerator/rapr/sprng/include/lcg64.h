#ifndef _lcg64_h
#define _lcg64_h

extern "C" {

class LCG64 : public Sprng
{
 public:

  int init_rng(int, int, int, int);
  LCG64();
  ~LCG64();
  LCG64 (const LCG64 &);
  LCG64 & operator = (const LCG64 &);
  int get_rn_int ();
  float get_rn_flt ();
  double get_rn_dbl ();
  int spawn_rng (int, Sprng ***);
  int get_seed_rng ();
  int free_rng ();
  int pack_rng (char **);
  int unpack_rng (char *);
  int print_rng ();

 private:

  int rng_type;
  char *gentype;
  int stream_number;
  int nstreams;
  int init_seed;
  int parameter;
  int narrays;
  int *array_sizes;
  int **arrays;
  int spawn_offset;
  /*** declare other variables here ***/
  unsigned int prime;

#ifdef LONG64                   /* 64 bit integer types */
  unsigned LONG64 state, multiplier;
#else  /* No 64 bit type available, so use array of floats */
  double state[3], multiplier[3];/* simulate 64 bit arithmetic */
#endif

  inline void advance_state();
};
}


#endif
