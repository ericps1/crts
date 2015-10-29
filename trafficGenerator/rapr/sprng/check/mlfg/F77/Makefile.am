if USE_MPI
  MPIDIR = $(MPICOMPILE)
  MPILIB = $(MPILINK)
  CXX=@MPICXX@
  F77=@MPIF77@
  MPILIBS=$(MPILINK)
endif

SUBDIRS = .

EXTRA_DIST=mlfg.data .insertlib
INCLUDEDIR = $(top_srcdir)/SRC

LIBDIR=../../../lib
CHKDIR=../../../check

bin_PROGRAMS = check.fmlfg check_sim.fmlfg time.fmlfg
check_fmlfg_SOURCES = check_genf.F 
check_fmlfg_DEPENDENCIES = $(LIBDIR)/libsprng.a
check_fmlfg_FFLAGS = $(FFPSIZE) -I$(INCLUDEDIR) $(MPIDIR)
check_fmlfg_LDADD = -L$(LIBDIR) -lsprng -lstdc++
check_fmlfg_LDFLAGS = $(MPILIB)
check_sim_fmlfg_SOURCES = check_genf_simple.F 
check_sim_fmlfg_DEPENDENCIES = $(LIBDIR)/libsprng.a
check_sim_fmlfg_LDADD = -L$(LIBDIR) -lsprng -lstdc++
check_sim_fmlfg_FFLAGS =$(FFPSIZE) -I$(INCLUDEDIR) $(MPIDIR)
check_sim_fmlfg_LDFLAGS = $(MPILIB)
time_fmlfg_SOURCES = timingf.F
time_fmlfg_DEPENDENCIES = $(LIBDIR)/libsprng.a
time_fmlfg_LDADD = -L$(LIBDIR) -lsprng  -lstdc++
time_fmlfg_FFLAGS = $(FFPSIZE) -I$(INCLUDEDIR) $(MPIDIR)
time_fmlfg_LDFLAGS = $(MPILIB)

bin_SCRIPTS = checkmlfg timemlfg
CLEANFILES = $(bin_SCRIPTS)

checkmlfg: $(CHKDIR)/insertlib
	rm -f ./checkmlfg
	@$(CHKDIR)/insertlib mlfg ./checkmlfg "Checking Default FORTRAN interface:" "check.fmlfg < ./mlfg.data 2> /dev/null" ./check.fmlfg
	@$(CHKDIR)/insertlib mlfg ./checkmlfg "Checking Simple FORTRAN interface:" "check_sim.fmlfg < ./mlfg.data 2> /dev/null" ./check_sim.fmlfg
	chmod u+x ./checkmlfg

timemlfg: $(CHKDIR)/insertlib
	rm -f ./timemlfg
	@$(CHKDIR)/insertlib mlfg ./timemlfg "Timing FORTRAN interface" ./time.fmlfg ./time.fmlfg
	chmod u+x ./timemlfg
