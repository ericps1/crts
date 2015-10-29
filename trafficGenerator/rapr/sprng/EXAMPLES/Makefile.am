if USE_MPI
  SUBDIRS = mpisprng
  MPIDIR = $(MPICOMPILE)
  MPILIB = $(MPILINK)
  MPILIBS=$(MPILINK)
else
  SUBDIRS = .
endif


LIBDIR=../lib
SRCDIR=../SRC
INCLUDEDIR=../include
INCLUDE = -I$(INCLUDEDIR)

GEN=sprng
CLEANFILES=*.$(GEN)

bin_PROGRAMS = convert.tmp pi-simple.tmp seed.tmp seed-simple.tmp simple-simple.tmp spawn.tmp sprng.tmp sprng-simple.tmp


convert_tmp_SOURCES = convert.cpp gen_types_menu.h
convert_tmp_CXXFLAGS = -I$(INCLUDEDIR) $(MPIDIR)
convert_tmp_LDADD = -L$(LIBDIR) -l$(GEN) 
convert_tmp_LDFLAGS = $(MPILIB)

pi_simple_tmp_SOURCES = pi-simple.cpp gen_types_menu.h
pi_simple_tmp_CXXFLAGS = -I$(INCLUDEDIR) $(MPIDIR)
pi_simple_tmp_LDADD = -L$(LIBDIR) -l$(GEN) 
pi_simple_tmp_LDFLAGS = $(MPILIB)

seed_tmp_SOURCES = seed.cpp gen_types_menu.h
seed_tmp_CXXFLAGS = -I$(INCLUDEDIR) $(MPIDIR)
seed_tmp_LDADD = -L$(LIBDIR) -l$(GEN) 
seed_tmp_LDFLAGS = $(MPILIB)

seed_simple_tmp_SOURCES = seed-simple.cpp gen_types_menu.h
seed_simple_tmp_CXXFLAGS = -I$(INCLUDEDIR) $(MPIDIR)
seed_simple_tmp_LDADD = -L$(LIBDIR) -l$(GEN) 
seed_simple_tmp_LDFLAGS = $(MPILIB)

simple_simple_tmp_SOURCES = simple-simple.cpp gen_types_menu.h
simple_simple_tmp_CXXFLAGS = -I$(INCLUDEDIR) $(MPIDIR)
simple_simple_tmp_LDADD = -L$(LIBDIR) -l$(GEN) 
simple_simple_tmp_LDFLAGS = $(MPILIB)

spawn_tmp_SOURCES = spawn.cpp gen_types_menu.h
spawn_tmp_CXXFLAGS = -I$(INCLUDEDIR) $(MPIDIR)
spawn_tmp_LDADD = -L$(LIBDIR) -l$(GEN)
spawn_tmp_LDFLAGS = $(MPILIB)

sprng_tmp_SOURCES = sprng.cpp gen_types_menu.h
sprng_tmp_CXXFLAGS = -I$(INCLUDEDIR) $(MPIDIR)
sprng_tmp_LDADD = -L$(LIBDIR) -l$(GEN)
sprng_tmp_LDFLAGS = $(MPILIB)

sprng_simple_tmp_SOURCES = sprng-simple.cpp gen_types_menu.h
sprng_simple_tmp_CXXFLAGS = -I$(INCLUDEDIR) $(MPIDIR)
sprng_simple_tmp_LDADD = -L$(LIBDIR) -l$(GEN)
sprng_simple_tmp_LDFLAGS = $(MPILIB)


bin_SCRIPTS = sprngtest
sprngtest:
	@rm -f *.o
	@mv convert.tmp convert.$(GEN)
	@mv pi-simple.tmp pi-simple.$(GEN)
	@mv seed.tmp seed.$(GEN)
	@mv seed-simple.tmp seed-simple.$(GEN)
	@mv simple-simple.tmp simple-simple.$(GEN)
	@mv spawn.tmp spawn.$(GEN)
	@mv sprng.tmp sprng.$(GEN)
	@mv sprng-simple.tmp sprng-simple.$(GEN)

