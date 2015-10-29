if USE_MPI
  MPIDIR = $(MPICOMPILE)
  MPILIB = $(MPILINK)
  CXX=@MPICXX@
  F77=@MPIF77@
  MPILIBS=$(MPILINK)
endif

SUBDIRS = $(FORTRANDIR) .

EXTRA_DIST=lcg64.data .insertlib
INCLUDEDIR = $(top_srcdir)/SRC

LIBDIR=../../lib
CHKDIR=../../check

bin_PROGRAMS = check.clcg64 check_sim.clcg64 time.clcg64
check_clcg64_SOURCES = check_gen.cpp
check_clcg64_DEPENDENCIES = $(LIBDIR)/libsprng.a
check_clcg64_CXXFLAGS = -I$(INCLUDEDIR) $(MPIDIR)
check_clcg64_LDADD = -L$(LIBDIR) -lsprng
check_clcg64_LDFLAGS = $(MPILIB)
check_sim_clcg64_SOURCES = check_gen_simple.cpp
check_sim_clcg64_DEPENDENCIES = $(LIBDIR)/libsprng.a
check_sim_clcg64_CXXFLAGS = -I$(INCLUDEDIR) $(MPIDIR)
check_sim_clcg64_LDADD = -L$(LIBDIR) -lsprng
check_sim_clcg64_LDFLAGS = $(MPILIB)
time_clcg64_SOURCES = timing.cpp
time_clcg64_DEPENDENCIES = $(LIBDIR)/libsprng.a
time_clcg64_LDADD = -L$(LIBDIR) -lsprng
time_clcg64_CXXFLAGS = -I$(INCLUDEDIR) $(MPIDIR)
time_clcg64_LDFLAGS = $(MPILIB)

bin_SCRIPTS = checklcg64 timelcg64
CLEANFILES = $(bin_SCRIPTS)

checklcg64: $(CHKDIR)/insertlib
	rm -f ./checklcg64
	@$(CHKDIR)/insertlib lcg64 ./checklcg64 "Checking Default C++ interface:" "check.clcg64 < ./lcg64.data 2> /dev/null" ./check.clcg64
	@$(CHKDIR)/insertlib lcg64 ./checklcg64 "Checking Simple C++ interface:" "check_sim.clcg64 < ./lcg64.data 2> /dev/null" ./check_sim.clcg64
	chmod u+x ./checklcg64

timelcg64: $(CHKDIR)/insertlib
	rm -f ./timelcg64
	@$(CHKDIR)/insertlib lcg64 ./timelcg64 "Timing C++ interface" ./time.clcg64 ./time.clcg64
	chmod u+x ./timelcg64
