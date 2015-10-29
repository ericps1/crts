SUBDIRS = . 

LIBDIR=../../../lib
SRCDIR=../../../SRC
INCLUDEDIR=../../../include
INCLUDES = -I$(INCLUDEDIR)

GEN=sprng
CLEANFILES=*.$(GEN)

CXX=@MPICXX@
F77=@MPIF77@
MPILIBS=$(MPILINK)

bin_PROGRAMS = 2streamsf_mpi.tmp fsprngf_mpi.tmp fsprngf-simple_mpi.tmp messagef_mpi.tmp messagef-simple_mpi.tmp \
	seedf_mpi.tmp seedf-simple_mpi.tmp sprngf_mpi.tmp sprngf-simple_mpi.tmp


2streamsf_mpi_tmp_SOURCES = 2streamsf_mpi.F $(SRCDIR)/sprng_f.h
2streamsf_mpi_tmp_FFLAGS = -I$(INCLUDEDIR) -DSPRNG_MPI
2streamsf_mpi_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm -lstdc++
2streamsf_mpi_tmp_LDFLAGS = $(MPILIBS)

fsprngf_mpi_tmp_SOURCES = fsprngf_mpi.F $(SRCDIR)/sprng_f.h
fsprngf_mpi_tmp_FFLAGS = -I$(INCLUDEDIR) -DSPRNG_MPI
fsprngf_mpi_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm -lstdc++
fsprngf_mpi_tmp_LDFLAGS = $(MPILIBS)

fsprngf_simple_mpi_tmp_SOURCES = fsprngf-simple_mpi.F $(SRCDIR)/sprng_f.h
fsprngf_simple_mpi_tmp_FFLAGS = -I$(INCLUDEDIR) -DSPRNG_MPI
fsprngf_simple_mpi_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm -lstdc++
fsprngf_simple_mpi_tmp_LDFLAGS = $(MPILIBS)

messagef_mpi_tmp_SOURCES = messagef_mpi.F $(SRCDIR)/sprng_f.h
messagef_mpi_tmp_FFLAGS = -I$(INCLUDEDIR) -DSPRNG_MPI
messagef_mpi_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm -lstdc++
messagef_mpi_tmp_LDFLAGS = $(MPILIBS)

messagef_simple_mpi_tmp_SOURCES = messagef-simple_mpi.F $(SRCDIR)/sprng_f.h
messagef_simple_mpi_tmp_FFLAGS = -I$(INCLUDEDIR)  -DSPRNG_MPI
messagef_simple_mpi_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm -lstdc++
messagef_simple_mpi_tmp_LDFLAGS = $(MPILIBS)

seedf_mpi_tmp_SOURCES = seedf_mpi.F $(SRCDIR)/sprng_f.h
seedf_mpi_tmp_FFLAGS = -I$(INCLUDEDIR) -DSPRNG_MPI
seedf_mpi_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm -lstdc++
seedf_mpi_tmp_LDFLAGS = $(MPILIBS)

seedf_simple_mpi_tmp_SOURCES = seedf-simple_mpi.F $(SRCDIR)/sprng_f.h
seedf_simple_mpi_tmp_FFLAGS = -I$(INCLUDEDIR) -DSPRNG_MPI
seedf_simple_mpi_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm -lstdc++
seedf_simple_mpi_tmp_LDFLAGS = $(MPILIBS)

sprngf_mpi_tmp_SOURCES = sprngf_mpi.F $(SRCDIR)/sprng_f.h
sprngf_mpi_tmp_FFLAGS = -I$(INCLUDEDIR) -DSPRNG_MPI
sprngf_mpi_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm -lstdc++
sprngf_mpi_tmp_LDFLAGS = $(MPILIBS)

sprngf_simple_mpi_tmp_SOURCES = sprngf-simple_mpi.F $(SRCDIR)/sprng_f.h
sprngf_simple_mpi_tmp_FFLAGS = -I$(INCLUDEDIR) -DSPRNG_MPI
sprngf_simple_mpi_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lm -lstdc++
sprngf_simple_mpi_tmp_LDFLAGS = $(MPILIBS)

bin_SCRIPTS = sprng_mpi_test
sprng_mpi_test:
	@rm -f *.o
	@mv 2streamsf_mpi.tmp 2streamsf_mpi.$(GEN)
	@mv fsprngf_mpi.tmp fsprngf_mpi.$(GEN)
	@mv fsprngf-simple_mpi.tmp fsprngf-simple_mpi.$(GEN)
	@mv messagef_mpi.tmp messagef_mpi.$(GEN)
	@mv messagef-simple_mpi.tmp messagef-simple_mpi.$(GEN)
	@mv seedf_mpi.tmp seedf_mpi.$(GEN)
	@mv seedf-simple_mpi.tmp seedf-simple_mpi.$(GEN)
	@mv sprngf_mpi.tmp sprngf_mpi.$(GEN)
	@mv sprngf-simple_mpi.tmp sprngf-simple_mpi.$(GEN)

