if USE_MPI
  SUBDIRS = . mpisprng
  MPIDIR = $(MPICOMPILE)
  MPILIB = $(MPILINK)
  CXX=@MPICXX@
  F77=@MPIF77@
  MPILIBS=$(MPILINK)
else
  SUBDIRS = .
endif


LIBDIR=../../lib
SRCDIR=../../SRC
INCLUDEDIR=../../include
INCLUDE = -I$(INCLUDEDIR)

GEN=sprng
CLEANFILES=*.$(GEN)

bin_PROGRAMS = convertf.tmp pif-simple.tmp seedf.tmp seedf-simple.tmp simplef-simple.tmp spawnf.tmp sprngf.tmp sprngf-simple.tmp subroutinef.tmp


convertf_tmp_SOURCES = convertf.F sprng_f.h genf_types_menu.h
convertf_tmp_FFLAGS = $(FFPSIZE) -I$(INCLUDEDIR) $(MPIDIR)
convertf_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lstdc++
convertf_tmp_LDFLAGS = $(MPILIB)

pif_simple_tmp_SOURCES = pif-simple.F sprng_f.h genf_types_menu.h
pif_simple_tmp_FFLAGS = $(FFPSIZE) -I$(INCLUDEDIR) $(MPIDIR)
pif_simple_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lstdc++
pif_simple_tmp_LDFLAGS = $(MPILIB)

seedf_tmp_SOURCES = seedf.F sprng_f.h genf_types_menu.h
seedf_tmp_FFLAGS = $(FFPSIZE) -I$(INCLUDEDIR) $(MPIDIR)
seedf_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lstdc++
seedf_tmp_LDFLAGS = $(MPILIB)

seedf_simple_tmp_SOURCES = seedf-simple.F sprng_f.h genf_types_menu.h
seedf_simple_tmp_FFLAGS = $(FFPSIZE) -I$(INCLUDEDIR) $(MPIDIR)
seedf_simple_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lstdc++
seedf_simple_tmp_LDFLAGS = $(MPILIB)

simplef_simple_tmp_SOURCES = simplef-simple.F sprng_f.h genf_types_menu.h
simplef_simple_tmp_FFLAGS = $(FFPSIZE) -I$(INCLUDEDIR) $(MPIDIR)
simplef_simple_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lstdc++
simplef_simple_tmp_LDFLAGS = $(MPILIB)

spawnf_tmp_SOURCES = spawnf.F sprng_f.h genf_types_menu.h
spawnf_tmp_FFLAGS = $(FFPSIZE) -I$(INCLUDEDIR) $(MPIDIR)
spawnf_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lstdc++
spawnf_tmp_LDFLAGS = $(MPILIB)

subroutinef_tmp_SOURCES = subroutinef.F sprng_f.h genf_types_menu.h
subroutinef_tmp_FFLAGS = $(FFPSIZE) -I$(INCLUDEDIR) $(MPIDIR)
subroutinef_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lstdc++
subroutinef_tmp_LDFLAGS = $(MPILIB)

sprngf_tmp_SOURCES = sprngf.F sprng_f.h genf_types_menu.h
sprngf_tmp_FFLAGS = $(FFPSIZE) -I$(INCLUDEDIR) $(MPIDIR)
sprngf_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lstdc++
sprngf_tmp_LDFLAGS = $(MPILIB)

sprngf_simple_tmp_SOURCES = sprngf-simple.F sprng_f.h genf_types_menu.h
sprngf_simple_tmp_FFLAGS = $(FFPSIZE) -I$(INCLUDEDIR) $(MPIDIR)
sprngf_simple_tmp_LDADD = -L$(LIBDIR) -l$(GEN) -lstdc++
sprngf_simple_tmp_LDFLAGS = $(MPILIB)


bin_SCRIPTS = sprngtest
sprngtest:
	@rm -f *.o
	@mv convertf.tmp convertf.$(GEN)
	@mv pif-simple.tmp pif-simple.$(GEN)
	@mv seedf.tmp seedf.$(GEN)
	@mv seedf-simple.tmp seedf-simple.$(GEN)
	@mv simplef-simple.tmp simplef-simple.$(GEN)
	@mv spawnf.tmp spawnf.$(GEN)
	@mv sprngf.tmp sprngf.$(GEN)
	@mv sprngf-simple.tmp sprngf-simple.$(GEN)
	@mv subroutinef.tmp subroutinef.$(GEN)
