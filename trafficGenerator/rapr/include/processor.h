/* $Id: processor.h,v 1.1.1.1 2007-01-10 21:14:17 lthompso Exp $ */
/*
 *	Processor Class definitions
 *
 *
 */

#ifndef _PROCESSOR_H
#define _PROCESSOR_H

#include <pthread.h>

#include "behaviorEvent.h"
#include "threadedProc.h"



/**
 *	class Processor : public BehaviorEvent
 *
 *	class to handle system call and system load emulations
 *
 */

class Processor : public BehaviorEvent
{

public:

    Processor(ProtoTimerMgr& timerMgr,
	      	  unsigned long  ubi,
	      	  Rapr&          theRapr);

    ~Processor();

    virtual bool OnStartUp();           	// virtual function
    virtual bool Stop();
    virtual void ClearState();
  	virtual bool ProcessEvent(const MgenMsg& theMsg);

	bool ParseProcessorOptions (const char* lineBuffer);

    bool GrabMemory (void);
    bool FreeMemory (void);
	bool ReadWriteDisk (void);
	bool RunCPU (void);
	bool VideoEnCodec (void);


private:

	int             abs_mem;		// testing - what's with logic id?
	bool            alt_mem;		// alternate alloc/free  (bool)
	bool            usr_cpu;		// run 2stream > /dev/pty/5 until STOP
	int             rw_disk;		// kbs > 0 if on
	char            rw_disk_dir [1024];		
	double          timeslice;		
	bool			do_thread;

	Threader	*	CodecThread;

    bool OnBehaviorTimeout(ProtoTimer& theTimer);
    bool OnInterrupt (ProtoTimer& theTimer); 


    double   	interruptPeriod;
    ProtoTimer	interruptTimer;		// emulate fake process flow (and exec)

    ProtoTimer	endBehaviorTimer;

    void	  * absmem_ptr;			
	pid_t		strm_pid;			// if usr_cpu (bool), fork/exec ...

	//  'processor' variables
  	enum ProcOption
    {
		ABS_MEM,		// baseline MBs of memory used
		ALT_MEM,	
		USR_CPU,		// % load cpu usage, user space
		SYS_CPU,		// % load cpu usage, in kernel
		RW_DISK,
		DIR_RW_DISK,
		ENCODER,		// vid codex encoding
		TIMESLICE,

		PROC_TOKEN,		// parse options sends this too
		BAD_PROC_OPTION
	};

	static const StringMapper PROC_OPT_LIST[];

}; // end class BehaviorEvent::Processor



/**************
some debugging stuff - for later...
extern  int   _lineno;
extern  char *_filenm;
extern unsigned long base_thread_id ;	// set if threading in operation

#if ! defined (__GNUC__)
void __datedlog (const char *format, ...);
#else  // __GNUC__
void _tmp_logger (const char *format, ...)    __attribute__ ((format (printf, 1, 2))) ;
#endif

#define DBUG 	_filenm=__FILE__,_lineno=__LINE__,DMSG
******************/

extern  int   _lineno;
extern  char *_filenm;
void tmp_logger (int level, const char *format, ...) ;  
#define Loggit 	_filenm=__FILE__,_lineno=__LINE__,tmp_logger


#endif // _PROCESSOR_H

