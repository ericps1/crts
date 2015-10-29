/*************************************************************************/
/*************************************************************************/
/* A PARALLEL RANDOM NUMBER GENERATION SYSTEM IN SPRNG FORMAT            */
/* Header file                                                           */
/*                                                                       */
/* Check for more changes needed to add a new rng in DOCS                */
/*                                                                       */
/* Author: J.Ren                                                         */
/*      Florida State University                                         */
/* Email: ren@csit.fsu.edu                                               */
/* Based on implentation by                                              */
/* Ashok Srinivasan  (Apr 13, 1998)                                      */
/*                                                                       */
/*                                                                       */
/* Note: Data stored by 'pack_rng' is NOT in a machine independent       */
/*       format. For that, please take a look at some SPRNG examples     */
/*       (lcg/lcg.cpp, lfg/lfg.cpp, etc).                                */
/*************************************************************************/
/*************************************************************************/

#ifndef _myrng_h
#define _myrng_h

extern "C" 
{
  class MyRng : public Sprng
  {
  public:

    MyRng();                            /* default constructor */
    int init_rng(int, int, int, int);   /* init function */
    ~MyRng();                           /* destructor */
    MyRng (const MyRng &);              /* copy constructor */
    MyRng & operator = (const MyRng &); /* asignment operator */

    int get_rn_int ();                  /* returns integer */
    float get_rn_flt ();                /* returns float */    
    double get_rn_dbl ();               /* returns double */
    int spawn_rng (int nspawned, Sprng ***newgens, int checkid);
    int get_seed_rng ();
    int free_rng ();
    int pack_rng (char **buffer);
    int unpack_rng (char *packed);
    int print_rng ();

  private:

    /* generator data members */
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
    double x;             /* please remove this in your implementation */
  };
}

#endif
