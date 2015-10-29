C
C           Demonstrates sprng use with one stream per process          
C A distinct stream is created on each process then prints a few      
C random numbers.  
C

       program sprngf_mpi
       implicit none

#define SIMPLE_SPRNG		! simple interface          
#define USE_MPI			! use MPI to find number of processes 
#include <mpif.h>
#include "sprng_f.h"

       SPRNG_POINTER junkPtr
       real*8 rn
       integer seed, i, myid,  ierror,junk
C---
       integer gtype
C---                   
      call MPI_INIT(ierror)

      call MPI_COMM_RANK(MPI_COMM_WORLD, myid, ierror)

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
       junkPtr = init_sprng(seed,SPRNG_DEFAULT,gtype)
       write(*,"('Process', i2, ', print information about stream:')")
     &       myid
       junk = print_sprng()

       do 100 i = 1, 3
          rn = sprng()
          write(*, "('Process',i2,', random number ',i1,': ',f16.14)")
     &    myid,i,rn
 100   continue

       call MPI_FINALIZE(ierror)

       end
