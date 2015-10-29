#ifndef _fwrap_h
#define _fwrap_h

/************************************************************************/
/************************************************************************/
/*      Inter-language Naming Convention Problem Solution               */
/*                                                                      */
/*      Note that with different compilers you may find that            */
/*      the linker fails to find certain modules due to the naming      */
/*      conventions implicit in particular compilers.  Here the         */
/*      solution was to look at the object code produced by the FORTRAN */
/*      compiler and modify this wrapper code so that the C routines    */
/*      compiled with the same routine names as produced in the FORTRAN */
/*      program.                                                        */
/*                                                                      */
/************************************************************************/
/************************************************************************/

/*
Turn funcName (which must be all lower-case) into something callable from
FORTRAN, typically by appending one or more underscores.
*/

#ifdef UpCase
#define FNAMEOF_finit_rng FINIT_RNG
#define FNAMEOF_fspawn_rng FSPAWN_RNG
#define FNAMEOF_fget_rn_int FGET_RN_INT
#define FNAMEOF_fget_rn_flt FGET_RN_FLT
#define FNAMEOF_fget_rn_dbl FGET_RN_DBL
#define FNAMEOF_fmake_new_seed FMAKE_NEW_SEED
#define FNAMEOF_fseed_mpi FSEED_MPI
#define FNAMEOF_ffree_rng FFREE_RNG
#define FNAMEOF_fget_seed_rng FGET_SEED_RNG
#define FNAMEOF_fpack_rng FPACK_RNG
#define FNAMEOF_funpack_rng FUNPACK_RNG
#define FNAMEOF_fprint_rng FPRINT_RNG

#define FNAMEOF_finit_rng_sim FINIT_RNG_SIM
#define FNAMEOF_fget_rn_int_sim FGET_RN_INT_SIM
#define FNAMEOF_fget_rn_flt_sim FGET_RN_FLT_SIM
#define FNAMEOF_fget_rn_dbl_sim FGET_RN_DBL_SIM
#define FNAMEOF_finit_rng_simmpi FINIT_RNG_SIMMPI
#define FNAMEOF_fget_rn_int_simmpi FGET_RN_INT_SIMMPI
#define FNAMEOF_fget_rn_flt_simmpi FGET_RN_FLT_SIMMPI
#define FNAMEOF_fget_rn_dbl_simmpi FGET_RN_DBL_SIMMPI
#define FNAMEOF_fpack_rng_simple FPACK_RNG_SIMPLE
#define FNAMEOF_funpack_rng_simple FUNPACK_RNG_SIMPLE
#define FNAMEOF_fprint_rng_simple FPRINT_RNG_SIMPLE

#define FNAMEOF_fcpu_t FCPU_T

#else

#define ffree_rng ffree_rng__ 
#define fmake_new_seed fmake_new_seed__
#define fseed_mpi fseed_mpi__
#define finit_rng finit_rng__
#define fspawn_rng fspawn_rng__
#define fget_rn_int fget_rn_int__
#define fget_rn_flt fget_rn_flt__
#define fget_rn_dbl fget_rn_dbl__
#define fget_seed_rng fget_seed_rng__
#define fpack_rng fpack_rng__
#define funpack_rng funpack_rng__
#define fprint_rng fprint_rng__

#define finit_rng_sim finit_rng_sim__
#define fget_rn_int_sim fget_rn_int_sim__
#define fget_rn_flt_sim fget_rn_flt_sim__
#define fget_rn_dbl_sim fget_rn_dbl_sim__
#define finit_rng_simmpi finit_rng_simmpi__
#define fget_rn_int_simmpi fget_rn_int_simmpi__
#define fget_rn_flt_simmpi fget_rn_flt_simmpi__
#define fget_rn_dbl_simmpi fget_rn_dbl_simmpi__
#define fpack_rng_simple fpack_rng_simple__
#define funpack_rng_simple funpack_rng_simple__
#define fprint_rng_simple fprint_rng_simple__

#define fcpu_t fcpu_t__

#endif

#endif
