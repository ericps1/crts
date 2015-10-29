#ifndef _mlfg_h
#define _mlfg_h

//#include "int64.h"

extern "C" {
class MLFG : public Sprng
{
 public:

  MLFG();
  int init_rng(int, int, int, int);
  ~MLFG();
  MLFG(const MLFG &);
  MLFG & operator= (const MLFG &);

  int get_rn_int();
  float get_rn_flt();
  double get_rn_dbl();
  int spawn_rng(int, Sprng ***);
  int get_seed_rng();
  int free_rng();
  int pack_rng(char **);
  int unpack_rng (char *);
  int print_rng ();

  int rng_type;
  char *gentype;
  int stream_number;
  int nstreams;
  int init_seed;
  int parameter;
  int narrays;
  int *array_sizes;
  int **arrays;

#ifdef LONG64
  typedef unsigned LONG64 uint64;
#else
  typedef unsigned int uint64[2];
#endif
  
  uint64 *lags;
  uint64 *si;
  int hptr;          /* integer pointer into fill */
  int lval, kval, seed;
 
  inline void advance_state();
};
}
#endif

