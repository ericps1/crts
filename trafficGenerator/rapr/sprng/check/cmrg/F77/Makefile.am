if USE_MPI
  MPIDIR = $(MPICOMPILE)
  MPILIB = $(MPILINK)
  CXX=@MPICXX@
  F77=@MPIF77@
  MPILIBS=$(MPILINK)
endif

SUBDIRS = .

EXTRA_DIST=cmrg.data .insertlib
INCLUDEDIR = $(top_srcdir)/SRC

LIBDIR=../../../lib
CHKDIR=../../../check

bin_PROGRAMS = check.fcmrg check_sim.fcmrg time.fcmrg
check_fcmrg_SOURCES = check_genf.F 
check_fcmrg_DEPENDENCIES = $(LIBDIR)/libsprng.a
check_fcmrg_FFLAGS = $(FFPSIZE) -I$(INCLUDEDIR) $(MPIDIR)
check_fcmrg_LDADD = -L$(LIBDIR) -lsprng -lstdc++
check_fcmrg_LDFLAGS = $(MPILIB)
check_sim_fcmrg_SOURCES = check_genf_simple.F 
check_sim_fcmrg_DEPENDENCIES = $(LIBDIR)/libsprng.a
check_sim_fcmrg_LDADD = -L$(LIBDIR) -lsprng -lstdc++
check_sim_fcmrg_FFLAGS =$(FFPSIZE) -I$(INCLUDEDIR) $(MPIDIR)
check_sim_fcmrg_LDFLAGS = $(MPILIB)
time_fcmrg_SOURCES = timingf.F
time_fcmrg_DEPENDENCIES = $(LIBDIR)/libsprng.a
time_fcmrg_LDADD = -L$(LIBDIR) -lsprng -lstdc++
time_fcmrg_FFLAGS = $(FFPSIZE) -I$(INCLUDEDIR) $(MPIDIR)
time_fcmrg_LDFLAGS = $(MPILIB)

bin_SCRIPTS = checkcmrg timecmrg
CLEANFILES = $(bin_SCRIPTS)

checkcmrg: $(CHKDIR)/insertlib
	rm -f ./checkcmrg
	@$(CHKDIR)/insertlib cmrg ./checkcmrg "Checking Default FORTRAN interface:" "check.fcmrg < ./cmrg.data 2> /dev/null" ./check.fcmrg
	@$(CHKDIR)/insertlib cmrg ./checkcmrg "Checking Simple FORTRAN interface:" "check_sim.fcmrg < ./cmrg.data 2> /dev/null" ./check_sim.fcmrg
	chmod u+x ./checkcmrg

timecmrg: $(CHKDIR)/insertlib
	rm -f ./timecmrg
	@$(CHKDIR)/insertlib cmrg ./timecmrg "Timing FORTRAN interface" ./time.fcmrg ./time.fcmrg
	chmod u+x ./timecmrg
