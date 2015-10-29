C            ____Demonstrates converting code to SPRNG____ 
C    
C The original random number call is to 'myrandom'. We change it to 
C call SPRNG by defining a macro.     
C                     
C The lines between the '#ifdef CONVERT' and the '#else' are the
C newly added lines. Those lines between the '#else' and the '#endif'
C are theoriginal lines that need to be deleted.



       program sprngf_simple
       implicit none

#define CONVERT

#ifdef CONVERT
#define SIMPLE_SPRNG	
#include "sprng_f.h"
#define myrandom sprng	

       SPRNG_POINTER junkPtr
#else
       external myrandom
       real*8   myrandom
#endif
       real*8 rn
       integer seed, irn, i, junk

C--- reading in a generator type
       integer gtype
#include "genf_types_menu.h"
       print *,'Type in a generator type (integers: 0,1,2,3,4,5):  '
       read *, gtype
C---

       seed = 985456376

#ifdef CONVERT
C   initialization
       junkPtr = init_sprng(seed,SPRNG_DEFAULT,gtype)
       print *, 'Printing information about new stream'
       junk = print_sprng()
#else
C Old initialization lines
#endif

       print *, 'Printing 3 double precision numbers in [0,1): '
       do 100 i = 1, 3
          rn = myrandom()
          print *,i, rn
 100   continue

 
       end
