if USE_MPI
  MPIDIR = $(MPICOMPILE)
  MPILIB = $(MPILINK)
  CXX=@MPICXX@
  F77=@MPIF77@
  MPILIBS=$(MPILINK)
endif

SUBDIRS = $(FORTRANDIR) .

EXTRA_DIST=pmlcg.data .insertlib
INCLUDEDIR = $(top_srcdir)/SRC

LIBDIR=../../lib
CHKDIR=../../check

bin_PROGRAMS = check.cpmlcg check_sim.cpmlcg time.cpmlcg
check_cpmlcg_SOURCES = check_gen.cpp
check_cpmlcg_DEPENDENCIES = $(LIBDIR)/libsprng.a
check_cpmlcg_CXXFLAGS = -I$(INCLUDEDIR) $(MPIDIR)
check_cpmlcg_LDADD = -L$(LIBDIR) -lsprng
check_cpmlcg_LDFLAGS = $(MPILIB)
check_sim_cpmlcg_SOURCES = check_gen_simple.cpp
check_sim_cpmlcg_DEPENDENCIES = $(LIBDIR)/libsprng.a
check_sim_cpmlcg_CXXFLAGS = -I$(INCLUDEDIR) $(MPIDIR)
check_sim_cpmlcg_LDADD = -L$(LIBDIR) -lsprng
check_sim_cpmlcg_LDFLAGS = $(MPILIB)
time_cpmlcg_SOURCES = timing.cpp
time_cpmlcg_DEPENDENCIES = $(LIBDIR)/libsprng.a
time_cpmlcg_LDADD = -L$(LIBDIR) -lsprng
time_cpmlcg_CXXFLAGS = -I$(INCLUDEDIR) $(MPIDIR)
time_cpmlcg_LDFLAGS = $(MPILIB)

bin_SCRIPTS = checkpmlcg timepmlcg
CLEANFILES = $(bin_SCRIPTS)

checkpmlcg: $(CHKDIR)/insertlib
	rm -f ./checkpmlcg
	@$(CHKDIR)/insertlib pmlcg ./checkpmlcg "Checking Default C++ interface:" "check.cpmlcg < ./pmlcg.data 2> /dev/null" ./check.cpmlcg
	@$(CHKDIR)/insertlib pmlcg ./checkpmlcg "Checking Simple C++ interface:" "check_sim.cpmlcg < ./pmlcg.data 2> /dev/null" ./check_sim.cpmlcg
	chmod u+x ./checkpmlcg

timepmlcg: $(CHKDIR)/insertlib
	rm -f ./timepmlcg
	@$(CHKDIR)/insertlib pmlcg ./timepmlcg "Timing C++ interface" ./time.cpmlcg ./time.cpmlcg
	chmod u+x ./timepmlcg

