if USE_MPI
  MPIDIR = $(MPICOMPILE)
  MPILIB = $(MPILINK)
#  CXX=@MPICXX@
#  F77=@MPIF77@
  MPILIBS=$(MPILINK)
endif

SUBDIRS = $(FORTRANDIR) .

EXTRA_DIST=lfg.data .insertlib
INCLUDEDIR = $(top_srcdir)/SRC

LIBDIR=../../lib
CHKDIR=../../check

bin_PROGRAMS = check.clfg check_sim.clfg time.clfg
check_clfg_SOURCES = check_gen.cpp
check_clfg_DEPENDENCIES = $(LIBDIR)/libsprng.a
check_clfg_CXXFLAGS = -I$(INCLUDEDIR) $(MPIDIR)
check_clfg_LDADD = -L$(LIBDIR) -lsprng
check_clfg_LDFLAGS = $(MPILIB)
check_sim_clfg_SOURCES = check_gen_simple.cpp
check_sim_clfg_DEPENDENCIES = $(LIBDIR)/libsprng.a
check_sim_clfg_CXXFLAGS = -I$(INCLUDEDIR) $(MPIDIR)
check_sim_clfg_LDADD = -L$(LIBDIR) -lsprng
check_sim_clfg_LDFLAGS = $(MPILIB)
time_clfg_SOURCES = timing.cpp
time_clfg_DEPENDENCIES = $(LIBDIR)/libsprng.a
time_clfg_LDADD = -L$(LIBDIR) -lsprng
time_clfg_CXXFLAGS = -I$(INCLUDEDIR) $(MPIDIR)
time_clfg_LDFLAGS = $(MPILIB)

bin_SCRIPTS = checklfg timelfg
CLEANFILES = $(bin_SCRIPTS)

checklfg: $(CHKDIR)/insertlib
	rm -f ./checklfg
	@$(CHKDIR)/insertlib lfg ./checklfg "Checking Default C++ interface:" "check.clfg < ./lfg.data 2> /dev/null" ./check.clfg
	@$(CHKDIR)/insertlib lfg ./checklfg "Checking Simple C++ interface:" "check_sim.clfg < ./lfg.data 2> /dev/null" ./check_sim.clfg
	chmod u+x ./checklfg

timelfg: $(CHKDIR)/insertlib
	rm -f ./timelfg
	@$(CHKDIR)/insertlib lfg ./timelfg "Timing C++ interface" ./time.clfg ./time.clfg
	chmod u+x ./timelfg
