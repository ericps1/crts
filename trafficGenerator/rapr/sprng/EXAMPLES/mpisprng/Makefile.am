SUBDIRS = . 

LIBDIR=../../lib
SRCDIR=../../SRC
INCLUDEDIR=../../include
INCLUDES = -I$(INCLUDEDIR)

GEN=sprng
CLEANFILES=*.$(GEN)

CXX=@MPICXX@
F77=@MPIF77@
MPILIBS=$(MPILINK)

bin_PROGRAMS = 2streams_mpi.tmp fsprng_mpi.tmp fsprng-simple_mpi.tmp message_mpi.tmp message-simple_mpi.tmp \
	pi-simple_mpi.tmp seed_mpi.tmp seed-simple_mpi.tmp sprng_mpi.tmp sprng-simple_mpi.tmp


2streams_mpi_tmp_SOURCES = 2streams_mpi.cpp $(SRCDIR)/sprng_cpp.h
2streams_mpi_tmp_CXXFLAGS = -I$(INCLUDEDIR) -DSPRNG_MPI
2streams_mpi_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm 
2streams_mpi_tmp_LDFLAGS = $(MPILIBS)

fsprng_mpi_tmp_SOURCES = fsprng_mpi.cpp $(SRCDIR)/sprng_cpp.h
fsprng_mpi_tmp_CXXFLAGS = -I$(INCLUDEDIR) -DSPRNG_MPI
fsprng_mpi_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm
fsprng_mpi_tmp_LDFLAGS = $(MPILIBS)

fsprng_simple_mpi_tmp_SOURCES = fsprng-simple_mpi.cpp $(SRCDIR)/sprng_cpp.h
fsprng_simple_mpi_tmp_CXXFLAGS = -I$(INCLUDEDIR) -DSPRNG_MPI
fsprng_simple_mpi_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm 
fsprng_simple_mpi_tmp_LDFLAGS = $(MPILIBS)

message_mpi_tmp_SOURCES = message_mpi.cpp $(SRCDIR)/sprng_cpp.h
message_mpi_tmp_CXXFLAGS = -I$(INCLUDEDIR) -DSPRNG_MPI
message_mpi_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm 
message_mpi_tmp_LDFLAGS = $(MPILIBS)

message_simple_mpi_tmp_SOURCES = message-simple_mpi.cpp $(SRCDIR)/sprng_cpp.h
message_simple_mpi_tmp_CXXFLAGS = -I$(INCLUDEDIR) -DSPRNG_MPI
message_simple_mpi_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm 
message_simple_mpi_tmp_LDFLAGS = $(MPILIBS)

pi_simple_mpi_tmp_SOURCES = pi-simple_mpi.cpp $(SRCDIR)/sprng_cpp.h
pi_simple_mpi_tmp_CXXFLAGS = -I$(INCLUDEDIR) -DSPRNG_MPI
pi_simple_mpi_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm 
pi_simple_mpi_tmp_LDFLAGS = $(MPILIBS)

seed_mpi_tmp_SOURCES = seed_mpi.cpp $(SRCDIR)/sprng_cpp.h
seed_mpi_tmp_CXXFLAGS = -I$(INCLUDEDIR) -DSPRNG_MPI
seed_mpi_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm
seed_mpi_tmp_LDFLAGS = $(MPILIBS)

seed_simple_mpi_tmp_SOURCES = seed-simple_mpi.cpp $(SRCDIR)/sprng_cpp.h
seed_simple_mpi_tmp_CXXFLAGS = -I$(INCLUDEDIR) -DSPRNG_MPI
seed_simple_mpi_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm 
seed_simple_mpi_tmp_LDFALGS = $(MPILIBS)

sprng_mpi_tmp_SOURCES = sprng_mpi.cpp $(SRCDIR)/sprng_cpp.h
sprng_mpi_tmp_CXXFLAGS = -I$(INCLUDEDIR)  -DSPRNG_MPI
sprng_mpi_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm
sprng_mpi_tmp_LDFLAGS  = $(MPILIBS)

sprng_simple_mpi_tmp_SOURCES = sprng-simple_mpi.cpp $(SRCDIR)/sprng_cpp.h 
sprng_simple_mpi_tmp_CXXFLAGS = -I$(INCLUDEDIR)  -DSPRNG_MPI
sprng_simple_mpi_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm
sprng_simple_mpi_tmp_LDFLAGS = $(MPILIBS)

bin_SCRIPTS = sprng_mpi_test
sprng_mpi_test:
	@rm -f *.o
	@mv sprng-simple_mpi.tmp sprng-simple_mpi.$(GEN)
	@mv 2streams_mpi.tmp 2streams_mpi.$(GEN)
	@mv fsprng_mpi.tmp fsprng_mpi.$(GEN)
	@mv fsprng-simple_mpi.tmp fsprng-simple_mpi.$(GEN)
	@mv message_mpi.tmp message_mpi.$(GEN)
	@mv message-simple_mpi.tmp message-simple_mpi.$(GEN)
	@mv pi-simple_mpi.tmp pi-simple_mpi.$(GEN)
	@mv seed_mpi.tmp seed_mpi.$(GEN)
	@mv seed-simple_mpi.tmp seed-simple_mpi.$(GEN)
	@mv sprng_mpi.tmp sprng_mpi.$(GEN)

