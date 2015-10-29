#ifndef _lfg_h
#define _lfg_h

extern "C" {
class LFG : public Sprng
{
 public:
  
  LFG();
  int init_rng (int, int, int, int);
  ~LFG();
  LFG (const LFG &);
  LFG & operator= (const LFG &);

  int get_rn_int();
  float get_rn_flt ();
  double get_rn_dbl ();
  int spawn_rng (int nspawned, Sprng ***);
  int get_seed_rng ();
  int free_rng ();
  int pack_rng (char **);
  int unpack_rng (char *);
  int print_rng ();

  /* data members public for static linkage */

  int rng_type;
  char *gentype;
  unsigned *si;      /* sets next branch seed  */
  unsigned *r0;      /* pointer to the even generator */
  unsigned *r1;      /* pointer to the odd generator */
  int stream_number;
  int hptr;          /* integer pointer into fill */
  int seed;
  int init_seed;
  int lval, kval;
  int param;

 private:
  int get_hptr_rng();
  int * get_fill_rng();
  int * get_next_index_rng();
  int * get_node_index_rng();
};
}

#endif
