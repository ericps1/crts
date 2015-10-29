#ifndef _sprng_h_
#define _sprng_h_

#define DEFAULT_RNG_TYPE SPRNG_LFG

#define SPRNG_LFG   0
#define SPRNG_LCG   1
#define SPRNG_LCG64 2
#define SPRNG_CMRG  3
#define SPRNG_MLFG  4
#define SPRNG_PMLCG 5

#define SPRNG_DEFAULT 0
#define CRAYLCG 0
#define DRAND48 1
#define FISH1   2
#define FISH2   3
#define FISH3   4
#define FISH4   5
#define FISH5   6
#define LECU1   0
#define LECU2   1
#define LECU3   2
#define LAG1279  0
#define LAG17    1
#define LAG31    2
#define LAG55    3
#define LAG63    4
#define LAG127   5
#define LAG521   6
#define LAG521B  7
#define LAG607   8
#define LAG607B  9
#define LAG1279B 10

#define MAX_PACKED_LENGTH 24000

#ifdef USE_MPI
#define MPINAME(A) A ## _mpi
#else
#define MPINAME(A) A
#endif

#define make_sprng_seed MPINAME(make_new_seed)

#if defined(SIMPLE_SPRNG)

#define pack_sprng pack_rng_simple
#define unpack_sprng unpack_rng_simple
#define isprng  MPINAME(get_rn_int_simple)
#define init_sprng MPINAME(init_rng_simple) 
#define print_sprng print_rng_simple

#ifdef FLOAT_GEN
#define sprng  MPINAME(get_rn_flt_simple)
#else
#define sprng  MPINAME(get_rn_dbl_simple)
#endif

#else

#define free_sprng free_rng
#define pack_sprng pack_rng
#define unpack_sprng unpack_rng
#define isprng  get_rn_int
#define spawn_sprng spawn_rng
#define init_sprng init_rng 
#define print_sprng print_rng

#ifdef FLOAT_GEN
#define sprng  get_rn_flt
#else
#define sprng  get_rn_dbl
#endif

#endif

class Sprng
{
 public:

  virtual ~Sprng(){};
  virtual int init_rng (int, int, int, int) = 0;
  virtual int get_rn_int () = 0;
  virtual float get_rn_flt () = 0;
  virtual double get_rn_dbl () = 0;
  virtual int spawn_rng (int nspawned, Sprng ***newgens) = 0;
  virtual int get_seed_rng () = 0;
  virtual int free_rng () = 0;
  virtual int pack_rng (char **buffer) = 0;
  virtual int print_rng () = 0;
  virtual int unpack_rng(char *packed) = 0;
};

int make_new_seed ();

int *init_rng_simple (int seed, int mult, int gtype = 0); 
int *init_rng_simple_mpi (int seed, int mult, int gtype = 0); 
int get_rn_int_simple ();
int get_rn_int_simple_mpi ();
float get_rn_flt_simple ();
float get_rn_flt_simple_mpi ();
double get_rn_dbl_simple ();
double get_rn_dbl_simple_mpi ();
int pack_rng_simple (char **buffer);
int *unpack_rng_simple (char *packed, int gtype);
int print_rng_simple ();
int make_new_seed_mpi ();


#endif

