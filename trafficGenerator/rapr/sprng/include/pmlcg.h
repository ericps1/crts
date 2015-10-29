#ifndef _pmlcg_h
#define _pmlcg_h

#include "bignum.h"

extern "C" {
class PMLCG : public Sprng
{
 public:

  PMLCG();
  int init_rng(int, int, int, int);
  ~PMLCG();
  PMLCG(const PMLCG &);
  PMLCG & operator= (const PMLCG &);

  int get_rn_int();
  float get_rn_flt();
  double get_rn_dbl();
  int spawn_rng(int, Sprng ***);
  int get_seed_rng();
  int free_rng();
  int pack_rng(char **);
  int unpack_rng (char *);
  int print_rng ();

  void iterate();

  int rng_type;
  char *gentype;
  int stream_number;
  int nstreams;
  int init_seed;
  int parameter;
  int narrays;
  /*** declare other variables here ***/
#ifdef LONG64
  unsigned LONG64 mult, x;
#else
  unsigned long r[2], a[2];
  int a_size;               /* length of array 'a' */
#endif
  BigNum k, si;         
};
}

#endif
