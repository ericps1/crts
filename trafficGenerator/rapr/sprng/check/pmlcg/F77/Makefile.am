if USE_MPI
  MPIDIR = $(MPICOMPILE)
  MPILIB = $(MPILINK)
  CXX=@MPICXX@
  F77=@MPIF77@
  MPILIBS=$(MPILINK)
endif

SUBDIRS = .

EXTRA_DIST=pmlcg.data .insertlib
INCLUDEDIR = $(top_srcdir)/SRC

LIBDIR=../../../lib
CHKDIR=../../../check

bin_PROGRAMS = check.fpmlcg check_sim.fpmlcg time.fpmlcg
check_fpmlcg_SOURCES = check_genf.F 
check_fpmlcg_DEPENDENCIES = $(LIBDIR)/libsprng.a
check_fpmlcg_FFLAGS = $(FFPSIZE) -I$(INCLUDEDIR) $(MPIDIR)
check_fpmlcg_LDADD = -L$(LIBDIR) -lsprng -lstdc++
check_fpmlcg_LDFLAGS = $(MPILIB)
check_sim_fpmlcg_SOURCES = check_genf_simple.F 
check_sim_fpmlcg_DEPENDENCIES = $(LIBDIR)/libsprng.a
check_sim_fpmlcg_LDADD = -L$(LIBDIR) -lsprng -lstdc++
check_sim_fpmlcg_FFLAGS = $(FFPSIZE) -I$(INCLUDEDIR) $(MPIDIR)
check_sim_fpmlcg_LDFLAGS = $(MPILIB)
time_fpmlcg_SOURCES = timingf.F
time_fpmlcg_DEPENDENCIES = $(LIBDIR)/libsprng.a
time_fpmlcg_LDADD = -L$(LIBDIR) -lsprng -lstdc++
time_fpmlcg_FFLAGS = $(FFPSIZE) -I$(INCLUDEDIR) $(MPIDIR)
time_fpmlcg_LDFLAGS = $(MPILIB)

bin_SCRIPTS = checkpmlcg timepmlcg
CLEANFILES = $(bin_SCRIPTS)

checkpmlcg: $(CHKDIR)/insertlib
	rm -f ./checkpmlcg
	@$(CHKDIR)/insertlib pmlcg ./checkpmlcg "Checking Default FORTRAN interface:" "check.fpmlcg < ./pmlcg.data 2> /dev/null" ./check.fpmlcg
	@$(CHKDIR)/insertlib pmlcg ./checkpmlcg "Checking Simple FORTRAN interface:" "check_sim.fpmlcg < ./pmlcg.data 2> /dev/null" ./check_sim.fpmlcg
	chmod u+x ./checkpmlcg

timepmlcg: $(CHKDIR)/insertlib
	rm -f ./timepmlcg
	@$(CHKDIR)/insertlib pmlcg ./timepmlcg "Timing FORTRAN interface" ./time.fpmlcg ./time.fpmlcg
	chmod u+x ./timepmlcg

