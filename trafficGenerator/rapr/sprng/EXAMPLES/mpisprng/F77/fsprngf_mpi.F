C
C          Demonstrates use of the single precision generator      
C One stream is maintained per processor. Each processor prints a few 
C single precision random numbers.
C
C Uncomment the following line to get the interface with pointer checking
C #define CHECK_POINTERS

       program fsprngf_mpi
       implicit none

#define FLOAT_GEN 1	! make 'sprng()' return single precision numbers
#include <mpif.h>
#include "sprng_f.h"

       integer streamnum, nstreams, seed
       SPRNG_POINTER stream
       real*4 rn
       integer i, myid, nprocs, ierror, junk
C---
       integer gtype
C---

      call MPI_INIT(ierror)

      call MPI_COMM_SIZE(MPI_COMM_WORLD, nprocs, ierror)

      call MPI_COMM_RANK(MPI_COMM_WORLD, myid, ierror)

       
       streamnum = myid
       nstreams = nprocs
       seed = 985456376

C--- node 0 is reading in a generator type
       if (myid .eq. 0) then
#include "genf_types_menu.h"
         print *,'Type in a generator type (integers: 0,1,2,3,4,5):  '
         read *, gtype
       endif
       call MPI_BCAST(gtype,1, MPI_INTEGER,0,MPI_COMM_WORLD,ierror)
C---


C 
       stream = init_sprng(gtype,streamnum,nstreams,seed,SPRNG_DEFAULT)
       write(*, "('Process',i2,': Print information about stream:')"
     &      ) myid
       junk = print_sprng(stream)

       do 100 i = 1, 3
          rn = sprng(stream)
          write(*, "('Process',i2,', random number',i2,': ',f8.6)")
     &          myid,i,rn
 100   continue

       junk = free_sprng(stream)
       call MPI_FINALIZE(ierror)


       end
