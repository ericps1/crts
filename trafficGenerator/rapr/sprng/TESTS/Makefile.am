if USE_MPI
  SUBDIRS = . mpitests
else
  SUBDIRS = .
endif

LIBDIR=../lib
SRCDIR=../SRC
INCLUDEDIR=../include
EXTRA_DIST = fft.cpp

GEN=sprng
CLEANFILES=*.$(GEN)

#MPI test programs are in directory mpitests
#if MPI
#CC=@MPICC@
#endif

#if FFT
#bin_PROGRAMS = equidist.tmp perm.tmp serial.tmp poker.tmp collisions.tmp gap.tmp maxt.tmp sum.tmp coupon.tmp runs.tmp random_walk.tmp wolff.tmp metropolis.tmp fft.tmp

#equidist_tmp_SOURCES = equidist.cpp chisquare.cpp communicate.cpp init_tests.cpp util.h util.cpp tests.h $(SRCDIR)/sprng_cpp.h
#equidist_tmp_CFLAGS = -I$(INCLUDEDIR)
#equidist_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm $(MPILIBS)

#perm_tmp_SOURCES = perm.c chisquare.c communicate.c init_tests.c util.h util.c tests.h generator.h
#perm_tmp_CFLAGS = -I$(INCLUDEDIR)
#perm_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm $(MPILIBS)

#serial_tmp_SOURCES = serial.c chisquare.c communicate.c init_tests.c util.h util.c tests.h generator.h
#serial_tmp_CFLAGS = -I$(INCLUDEDIR)
#serial_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm $(MPILIBS)

#poker_tmp_SOURCES = poker.c chisquare.c communicate.c init_tests.c util.h util.c tests.h stirling.c generator.h
#poker_tmp_CFLAGS = -I$(INCLUDEDIR)
#poker_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm $(MPILIBS) $(MPILIBS)

#collisions_tmp_SOURCES = collisions.c chisquare.c communicate.c init_tests.c util.h util.c tests.h generator.h
#collisions_tmp_CFLAGS = -I$(INCLUDEDIR)
#collisions_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm $(MPILIBS)

#gap_tmp_SOURCES = gap.c chisquare.c communicate.c init_tests.c util.h util.c tests.h generator.h
#gap_tmp_CFLAGS = -I$(INCLUDEDIR)
#gap_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm $(MPILIBS) 

#maxt_tmp_SOURCES = maxt.c chisquare.c communicate.c init_tests.c util.h util.c tests.h generator.h
#maxt_tmp_CFLAGS = -I$(INCLUDEDIR)
#maxt_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm $(MPILIBS)

#sum_tmp_SOURCES = sum.c chisquare.c communicate.c init_tests.c util.h util.c tests.h generator.h
#sum_tmp_CFLAGS = -I$(INCLUDEDIR)
#sum_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm $(MPILIBS) 

#coupon_tmp_SOURCES = coupon.c chisquare.c communicate.c init_tests.c util.h util.c tests.h stirling.c generator.h
#coupon_tmp_CFLAGS = -I$(INCLUDEDIR)
#coupon_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm $(MPILIBS) 

#runs_tmp_SOURCES = runs.c chisquare.c communicate.c init_tests.c util.h util.c tests.h generator.h
#runs_tmp_CFLAGS = -I$(INCLUDEDIR)
#runs_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm $(MPILIBS)

#random_walk_tmp_SOURCES = random_walk.c chisquare.c communicate.c init_tests.c util.h util.c tests.h generator.h
#random_walk_tmp_CFLAGS = -I$(INCLUDEDIR)
#random_walk_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm $(MPILIBS)

#wolff_tmp_SOURCES = wolff.c chisquare.c communicate.c init_tests.c util.h util.c tests.h generator.h
#wolff_tmp_CFLAGS = -I$(INCLUDEDIR)
#wolff_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm $(MPILIBS)

#metropolis_tmp_SOURCES = metropolis.c chisquare.c communicate.c init_tests.c util.h util.c tests.h generator.h
#metropolis_tmp_CFLAGS = -I$(INCLUDEDIR)
#metropolis_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm $(MPILIBS)

#fft_tmp_SOURCES = fft.c generator.h
#fft_tmp_CFLAGS = -I$(INCLUDEDIR)
#fft_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm $(MPILIBS)

#bin_SCRIPTS = sprngtest
#sprngtest:
#	@rm -f *.o
#	@mv equidist.tmp equidist.$(GEN)
#	@mv perm.tmp perm.$(GEN)
#	@mv serial.tmp serial.$(GEN)
#	@mv poker.tmp poker.$(GEN)
#	@mv collisions.tmp collisions.$(GEN)
#	@mv gap.tmp gap.$(GEN)
#	@mv maxt.tmp maxt.$(GEN)
#	@mv sum.tmp sum.$(GEN)
#	@mv coupon.tmp coupon.$(GEN)
#	@mv runs.tmp runs.$(GEN)
#	@mv random_walk.tmp random_walk.$(GEN)
#	@mv wolff.tmp wolff.$(GEN)
#	@mv metropolis.tmp metropolis.$(GEN)
#	@mv fft.tmp fft.$(GEN)
#else
bin_PROGRAMS = equidist.tmp perm.tmp serial.tmp poker.tmp collisions.tmp gap.tmp maxt.tmp sum.tmp coupon.tmp runs.tmp random_walk.tmp wolff.tmp metropolis.tmp wolfftest.tmp wolffind.tmp

equidist_tmp_SOURCES = equidist.cpp chisquare.cpp communicate.cpp init_tests.cpp util.h util.cpp tests.h $(SRCDIR)/sprng_cpp.h
equidist_tmp_CXXFLAGS = -I$(INCLUDEDIR)
equidist_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm $(MPILIBS)

perm_tmp_SOURCES = perm.cpp chisquare.cpp communicate.cpp init_tests.cpp util.h util.cpp tests.h $(SRCDIR)/sprng_cpp.h
perm_tmp_CXXFLAGS = -I$(INCLUDEDIR)
perm_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm $(MPILIBS)

serial_tmp_SOURCES = serial.cpp chisquare.cpp communicate.cpp init_tests.cpp util.h util.cpp tests.h $(SRCDIR)/sprng_cpp.h
serial_tmp_CXXFLAGS = -I$(INCLUDEDIR)
serial_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm $(MPILIBS)

poker_tmp_SOURCES = poker.cpp chisquare.cpp communicate.cpp init_tests.cpp util.h util.cpp tests.h stirling.cpp $(SRCDIR)/sprng_cpp.h
poker_tmp_CXXFLAGS = -I$(INCLUDEDIR)
poker_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm $(MPILIBS)

collisions_tmp_SOURCES = collisions.cpp chisquare.cpp communicate.cpp init_tests.cpp util.h util.cpp tests.h $(SRCDIR)/sprng_cpp.h
collisions_tmp_CXXFLAGS = -I$(INCLUDEDIR)
collisions_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm $(MPILIBS)

gap_tmp_SOURCES = gap.cpp chisquare.cpp communicate.cpp init_tests.cpp util.h util.cpp tests.h $(SRCDIR)/sprng_cpp.h
gap_tmp_CXXFLAGS = -I$(INCLUDEDIR)
gap_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm $(MPILIBS)

maxt_tmp_SOURCES = maxt.cpp chisquare.cpp communicate.cpp init_tests.cpp util.h util.cpp tests.h $(SRCDIR)/sprng_cpp.h
maxt_tmp_CXXFLAGS = -I$(INCLUDEDIR)
maxt_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm $(MPILIBS)

sum_tmp_SOURCES = sum.cpp chisquare.cpp communicate.cpp init_tests.cpp util.h util.cpp tests.h $(SRCDIR)/sprng_cpp.h
sum_tmp_CXXFLAGS = -I$(INCLUDEDIR)
sum_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm $(MPILIBS)

coupon_tmp_SOURCES = coupon.cpp chisquare.cpp communicate.cpp init_tests.cpp util.h util.cpp tests.h stirling.cpp $(SRCDIR)/sprng_cpp.h
coupon_tmp_CXXFLAGS = -I$(INCLUDEDIR)
coupon_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm $(MPILIBS)

runs_tmp_SOURCES = runs.cpp chisquare.cpp communicate.cpp init_tests.cpp util.h util.cpp tests.h $(SRCDIR)/sprng_cpp.h
runs_tmp_CXXFLAGS = -I$(INCLUDEDIR)
runs_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm $(MPILIBS)

random_walk_tmp_SOURCES = random_walk.cpp chisquare.cpp communicate.cpp init_tests.cpp util.h util.cpp tests.h $(SRCDIR)/sprng_cpp.h
random_walk_tmp_CXXFLAGS = -I$(INCLUDEDIR)
random_walk_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm $(MPILIBS)

wolff_tmp_SOURCES = wolff.cpp chisquare.cpp communicate.cpp init_tests.cpp util.h util.cpp tests.h $(SRCDIR)/sprng_cpp.h
wolff_tmp_CXXFLAGS = -I$(INCLUDEDIR)
wolff_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm $(MPILIBS)

wolffind_tmp_SOURCES = wolffind.cpp chisquare.cpp communicate.cpp init_tests.cpp util.h util.cpp tests.h $(SRCDIR)/sprng_cpp.h
wolffind_tmp_CXXFLAGS = -I$(INCLUDEDIR)
wolffind_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm $(MPILIBS)

wolfftest_tmp_SOURCES = wolfftest.cpp chisquare.cpp communicate.cpp init_tests.cpp util.h util.cpp tests.h $(SRCDIR)/sprng_cpp.h
wolfftest_tmp_CXXFLAGS = -I$(INCLUDEDIR)
wolfftest_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm $(MPILIBS)

metropolis_tmp_SOURCES = metropolis.cpp chisquare.cpp communicate.cpp init_tests.cpp util.h util.cpp tests.h $(SRCDIR)/sprng_cpp.h
metropolis_tmp_CXXFLAGS = -I$(INCLUDEDIR)
metropolis_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm $(MPILIBS)

bin_SCRIPTS = sprngtest
sprngtest:
	@rm -f *.o
	@cp equidist.tmp equidist.$(GEN)
	@cp perm.tmp perm.$(GEN)
	@cp serial.tmp serial.$(GEN)
	@cp poker.tmp poker.$(GEN)
	@cp collisions.tmp collisions.$(GEN)
	@cp gap.tmp gap.$(GEN)
	@cp maxt.tmp maxt.$(GEN)
	@cp sum.tmp sum.$(GEN)
	@cp coupon.tmp coupon.$(GEN)
	@cp runs.tmp runs.$(GEN)
	@cp random_walk.tmp random_walk.$(GEN)
	@cp wolff.tmp wolff.$(GEN)
	@cp wolffind.tmp wolffind.$(GEN)
	@cp wolfftest.tmp wolfftest.$(GEN)
	@cp metropolis.tmp metropolis.$(GEN)
#endif
