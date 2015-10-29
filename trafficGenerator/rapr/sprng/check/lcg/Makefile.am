if USE_MPI
  MPIDIR = $(MPICOMPILE)
  MPILIB = $(MPILINK)
  CXX=@MPICXX@
  F77=@MPIF77@
  MPILIBS=$(MPILINK)
endif

SUBDIRS = $(FORTRANDIR) .

EXTRA_DIST=lcg.data .insertlib
INCLUDEDIR = $(top_srcdir)/SRC

LIBDIR=../../lib
CHKDIR=../../check

bin_PROGRAMS = check.clcg check_sim.clcg time.clcg
check_clcg_SOURCES = check_gen.cpp 
check_clcg_DEPENDENCIES = $(LIBDIR)/libsprng.a
check_clcg_CXXFLAGS = -I$(INCLUDEDIR) $(MPIDIR)
check_clcg_LDADD = -L$(LIBDIR) -lsprng
check_clcg_LDFLAGS = $(MPILIB)
check_sim_clcg_SOURCES = check_gen_simple.cpp
check_sim_clcg_DEPENDENCIES = $(LIBDIR)/libsprng.a
check_sim_clcg_CXXFLAGS = -I$(INCLUDEDIR) $(MPIDIR)
check_sim_clcg_LDADD = -L$(LIBDIR) -lsprng 
check_sim_clcg_LDFLAGS = $(MPILIB)
time_clcg_SOURCES = timing.cpp
time_clcg_DEPENDENCIES = $(LIBDIR)/libsprng.a
time_clcg_LDADD = -L$(LIBDIR) -lsprng
time_clcg_CXXFLAGS = -I$(INCLUDEDIR) $(MPIDIR) 
time_clcg_LDFLAGS = $(MPILIB)


bin_SCRIPTS = checklcg timelcg
CLEANFILES = $(bin_SCRIPTS)

checklcg: $(CHKDIR)/insertlib
	rm -f ./checklcg
	@$(CHKDIR)/insertlib lcg ./checklcg "Checking Default C++ interface:" "check.clcg < ./lcg.data 2> /dev/null" ./check.clcg
	@$(CHKDIR)/insertlib lcg ./checklcg "Checking Simple C++ interface:" "check_sim.clcg < ./lcg.data 2> /dev/null" ./check_sim.clcg
	chmod u+x ./checklcg

timelcg: $(CHKDIR)/insertlib
	rm -f ./timelcg
	@$(CHKDIR)/insertlib lcg ./timelcg "Timing C++ interface" ./time.clcg ./time.clcg
	chmod u+x ./timelcg
