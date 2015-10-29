#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT(SRC/lfg/lfg.cpp)
AC_CANONICAL_SYSTEM

AM_INIT_AUTOMAKE(sprng,4)
AM_CONFIG_HEADER(config.h:config.in)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_F77

AC_F77_LIBRARY_LDFLAGS 

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lm':
AC_CHECK_LIB([m], [main])

AC_PROG_RANLIB
AC_PATH_PROGS(BASH, bash)

AC_SUBST(sprng_cxxflags)
AC_SUBST(sprng_fflags)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([limits.h stddef.h stdlib.h string.h sys/time.h])

AC_CHECK_SIZEOF([long int])

CXXFLAGS="$CXXFLAGS -Wno-deprecated"

if test "$ac_cv_sizeof_long_int" = "8"; then
  CXXFLAGS="$CXXFLAGS -DLONG64=long"
fi

AC_PATH_PROG(MPICXX, mpicxx)
AC_PATH_PROG(MPIF77, mpif77)
#AC_CHECK_PROGS(MPICXX, mpic++ mpicxx mpiCC hcp mpxlC_r mpxlC mpCC cmpic++, $CXX)
#AC_CHECK_PROGS(MPIF77, mpif77 hf77 mpxlf_r mpxlf mpf77 cmpifc, $F77)


if test $CXX != $MPICXX; then  
  can_use_mpi=y
else
  can_use_mpi=n 
fi

AM_CONDITIONAL([USE_MPI], [test $can_use_mpi = y])

if test $can_use_mpi = y; then
  MPI_DEF="-DSPRNG_MPI"
else
  MPI_DEF=""
fi

AC_SUBST(MPI_DEF)

AM_CONDITIONAL(compile_FORTRAN, test -z "$FFLAGS")

AC_CHECK_HEADER(fft.h,fft="yes",fft="no")
if test $fft="yes"; then
  AC_CHECK_FUNC(dzfft2dui, [fft="yes"], [fft="no"])
fi
AM_CONDITIONAL(FFT, test $fft = "yes")

AC_C_BIGENDIAN

dnl deal with Fortran pointer size
AC_CHECK_SIZEOF(int *)
if test "$ac_cv_sizeof_int_p" = "8"; then
  FFPSIZE="-DPOINTER_SIZE=8 -DLONG64=long"
else
  FFPSIZE=""
fi
AC_SUBST(FFPSIZE)

AC_CHECK_SIZEOF(double)
AC_CHECK_SIZEOF(float)

dnl AC_LANG(C++)

if test -z "$F77"; then
  F77="echo"
  FORTRANDIR=
else
dnl AC_LANG(Fortran 77)
  FORTRANDIR=F77

AC_F77_WRAPPERS

dnl deal with fortran wrapper name mangling
AC_F77_NAME_MANGLING()
if test "$ac_case" = "upper"; then
  FSUFFIX="F"
else
  FSUFFIX="f"
fi

if test -n "$ac_underscore"; then
  FFXN="-DAdd_"
  if test -n "$ac_extra"; then
    FFXN="-DAdd__"
  fi
else
  FFXN="-DNoChange"
fi

AC_SUBST(FSUFFIX)
AC_SUBST(FFXN)
fi

AC_SUBST(FORTRANDIR)
#AC_CONFIG_FILES([Makefile src/Makefile opt/Makefile])


if test "$with_parallel" = "enable"; then
  MPIBIN=`dirname $MPICXX`
  MPIRUN=${MPIBIN}/mpirun
  MPIROOT=`dirname $MPIBIN`

  if test -z "$MPIINC"; then
    MPIINC=${MPIROOT}/include
  fi

  find_mpilib=`find ${MPIROOT}/lib* -name libmpi* | head -n 1`

  if test -n ${find_mpilib}; then
    MPILIB=`dirname ${find_mpilib}`
  else
    AC_MSG_ERROR([Unable to find mpi libraries in MPI lib directory])
  fi

  if test -e ${MPILIB}/libmpi.a || test -e ${MPILIB}/libmpi.so || test -e ${MPILIB}/libmpi.dylib; then
#    MPILINK="-L${MPILIB} -lmpi"
    $MPICXX --showme:compile > tempmpicompile
    $MPICXX --showme:link > tempmpilink
    MPICOMPILE=`cat tempmpicompile`
    MPILINK=`cat tempmpilink`
    rm -f tempmpicompile tempmpilink
#    CXXFLAGS="${MPICOMPILE} ${CXXFLAGS}"
#    LDFLAGS="${MPILINK} ${LDFLAGS}"
    LAM=true
  elif test -e ${MPILIB}/libmpich.a || test -e ${MPILIB}/libmpich.so || test -e ${MPILIB}/libmpich.dylib; then
    if test -e ${MPIINC}/mpi.h; then
      echo "mpi.h is found"
    else
      AC_MSG_ERROR([Please specify the INCLUDE directory for MPI with MPIINC])
    fi

    MPILINK="-L${MPILIB} -lmpich"
    MPICOMPILE="-I${MPIINC}"
#    CXXFLAGS="${CXXFLAGS} -I${MPIINC}"
#    LDFLAGS="${MPILINK} ${LDFLAGS}"
    MPICH=true
  else
    AC_MSG_ERROR([Unable to find libmpi.a, libmpi.so, libmpich.a, or libmpich.so in MPI lib directory])
  fi

  CXX=${MPICXX}
fi

AC_SUBST(MPICOMPILE)
AC_SUBST(MPILINK)

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_CHECK_FUNCS([memset pow sqrt])

AC_PROG_INSTALL
AC_OUTPUT([Makefile 
	     include/Makefile
	     lib/Makefile
	     SRC/Makefile
	     SRC/cmrg/Makefile
	     SRC/lcg/Makefile
	     SRC/lcg64/Makefile
	     SRC/lfg/Makefile
	     SRC/mlfg/Makefile
	     SRC/pmlcg/Makefile
		 check/Makefile 
		 check/lcg/Makefile 
		 check/lcg/F77/Makefile 
		 check/lfg/Makefile
		 check/lfg/F77/Makefile
		 check/mlfg/Makefile
		 check/mlfg/F77/Makefile
		 check/cmrg/Makefile
		 check/cmrg/F77/Makefile
		 check/lcg64/Makefile
		 check/lcg64/F77/Makefile
		 check/pmlcg/Makefile
		 check/pmlcg/F77/Makefile
		 TESTS/Makefile
		 TESTS/mpitests/Makefile
         EXAMPLES/Makefile
		 EXAMPLES/mpisprng/Makefile])

