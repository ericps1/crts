C
C           Demonstrates the use of make_seed with MPI          
C'make_sprng_seed' is called to produce a new seed each time the program
C is run. The same seed is produced on each process. 
C
C Uncomment the following line to get the interface with pointer checking
C #define CHECK_POINTERS

       program seedf_mpi
       implicit none

#define USE_MPI 1
#include <mpif.h>
#include "sprng_f.h"

       integer streamnum, nstreams, seed, i
       SPRNG_POINTER stream
       real*8 rn
       integer myid, nprocs, ierror, junk
C---
       integer gtype
C---     
      call MPI_INIT(ierror)

      call MPI_COMM_SIZE(MPI_COMM_WORLD, nprocs, ierror)

      call MPI_COMM_RANK(MPI_COMM_WORLD, myid, ierror)

       
       streamnum = myid
       nstreams = nprocs
       seed = make_sprng_seed() !produce a new seed each time program is run
C--- node 0 is reading in a generator type
       if (myid .eq. 0) then
#include "genf_types_menu.h"
         print *,'Type in a generator type (integers: 0,1,2,3,4,5):  '
         read *, gtype
       endif
       call MPI_BCAST(gtype,1, MPI_INTEGER,0,MPI_COMM_WORLD,ierror)
C---          
C Seed should be the same on all processes
       write(6, 911) myid, seed
 911   format("Process", i2, ": seed = ", i16)

       stream = init_sprng(gtype,streamnum,nstreams,seed,SPRNG_DEFAULT)
       write(6, 922) myid
 922   format("Process", i2, 
     &        ": Print information about stream:")
       junk = print_sprng(stream)

       do 100 i = 1, 3
          rn = sprng(stream)
          write(6, 933) myid, i, rn
 100   continue
 933   format("process", i2, ", random number", i2, ": ", f8.6)
       junk = free_sprng(stream)
       call MPI_FINALIZE(ierror)

       end
