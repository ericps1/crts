/* $Id: processor.cpp,v 1.1.1.1 2007-01-10 21:14:17 lthompso Exp $ */
/*
 *		Processor Class for making RAPR generate noise like real 
 *		applications.
 *
 *	stuff...
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>


#include "behaviorEvent.h"
#include "processor.h"

#include "rapr.h"




// some globals for debugging

int   _lineno;
char *_filenm;
unsigned long base_thread_id = 0;	// set if threading in operation

static int	disgooble = 1;			// disable PROCESSOR from scripted global


Processor::Processor(ProtoTimerMgr& timerMgr,
			 		 unsigned long  ubi,
			 		 Rapr&          theRapr)
		 : BehaviorEvent(timerMgr,ubi,theRapr), 
		 abs_mem(0), 
		 usr_cpu(0),
		 rw_disk(0), 
		 timeslice(0)
{

	type = PROCESSOR;

	CodecThread 	= NULL;
	base_thread_id 	= 0;
	strm_pid 	   	= 0;
	do_thread      	= false;

	Loggit (1,"jm: Processor::Processor() \n");

	if ( ! theRapr.GetLoadEmulator()) 
	{
		DMSG(0,"Processor::Processor() Error: Processor globally disabled\n");
		disgooble = 0;
	}

	absmem_ptr = NULL;


}



Processor::~Processor()
{
	Loggit (1,"jm: Processor:: Destructor () - call stop...\n");

	(void) Stop();
}



bool Processor::OnStartUp()
{
	char tmpBuffer[512];

	Loggit (1,"jm: Processor::Onstartup() \n");

	if ( ! disgooble) {
		Loggit (1, "Processor::OnStartUp: Error:  PROCESSOR directive globally disabled\n");
		return false;
	}
	// log event
	char msgBuffer[8192];

	sprintf(msgBuffer,"type>Processor action>start ubi>%lu eventSource>%s",
			ubi, GetStringFromEventSource(eventSource));

	if (rapr.GetVerbosity()) 
	{
	    if (foreignUbi)
		{
	  		sprintf(tmpBuffer," foreignUBI>%lu",foreignUbi);
	  		strcat(msgBuffer,tmpBuffer);
		}
	    if (successLogicID)
		{
	  		sprintf(tmpBuffer," success>%d",successLogicID);
	  		strcat(msgBuffer,tmpBuffer);
		}
	    if (failureLogicID)
		{
	  		sprintf(tmpBuffer," failure>%d",failureLogicID);
	  		strcat(msgBuffer,tmpBuffer);
		}
	    if (timeoutLogicID)
		{
	  		sprintf(tmpBuffer," timeout>%d",timeoutLogicID);
	  		strcat(msgBuffer,tmpBuffer);
		}
	    if (payloadLogicID)
		{
	  		sprintf(tmpBuffer," payload>%d",payloadLogicID);
	  		strcat(msgBuffer,tmpBuffer);
		}
	    if (raprFlowID)
		{
	  		sprintf(tmpBuffer," raprFlowID>%d",raprFlowID);
	  		strcat(msgBuffer,tmpBuffer);
		}
	    if (behaviorStartTime)
		{
	  		sprintf(tmpBuffer," startTime>%.2f",behaviorStartTime);
	  		strcat(msgBuffer,tmpBuffer);
		}
	    if (behaviorEndTime)
		{
	  		sprintf(tmpBuffer," endTime>%.2f",behaviorEndTime);
	  		strcat(msgBuffer,tmpBuffer);
		}

	    if (GetSeed())
		{
	  		sprintf(tmpBuffer," seed>%ld",GetSeed());
	  		strcat(msgBuffer,tmpBuffer);
		}
	}
	strcat(msgBuffer,tmpBuffer);

	rapr.LogEvent("RAPR",msgBuffer);

	// ljt if No end behavior timeout exists, end now.

	if (behaviorEndTime != 0.0)
	{
	    endBehaviorTimer.SetListener(this,&Processor::OnBehaviorTimeout);
	    ActivateEndTimer(endBehaviorTimer);
	}
	else 
	{
	    OnBehaviorTimeout(endBehaviorTimer);
	}

    if (abs_mem)
		(void) GrabMemory ();

    if (rw_disk)
		(void) ReadWriteDisk ();

	if (usr_cpu)
		(void) RunCPU ();

	if (do_thread)
		(void) VideoEnCodec ();		// will take params....


	if (timeslice)
	{
		Loggit (0,"Processor::OnStarup - start retry timeout() for all\n");

		interruptPeriod = timeslice;
		interruptTimer.SetInterval (interruptPeriod);  
		interruptTimer.SetListener (this, &Processor::OnInterrupt);  
  		interruptTimer.SetRepeat(500);
		BehaviorEvent::timer_mgr.ActivateTimer(interruptTimer); 
	}

	Loggit (1,"jm: leaving Processor::Onstartup() \n");
	return true;       

} // end Processor::OnStartup





bool Processor::Stop()
{
	Loggit (1,"jm: Processor:: Stop! () \n");

	if (absmem_ptr)
	{
		Loggit (0,"Processor::Stop - freeing abs mem() \n");
		free (absmem_ptr);
		absmem_ptr = NULL;
	}

	if (strm_pid) 
	{ 
		Loggit (0,"Processor::Stop - killing pid (%d)\n", strm_pid);
		kill (strm_pid, SIGTERM);
		strm_pid = 0;
	} 

#if defined (PROC_THREADS)
	if (do_thread && CodecThread)
	{
		Loggit (1,"Processor::Stop - killing thread \n");
		if (CodecThread->KillThread() == false)
		{
			Loggit (0,"Processor::Stop Error - couldn't kill thread \n");
		}
		// clean up anyway
		delete CodecThread;
		CodecThread = NULL;
	}
#endif // PROC_THREADS

	// Turn off flow  ljt - check to see that flow is still going!
	if (GetFlowID())
	{
	    // log event
	    char msgBuffer[512];
	    sprintf(msgBuffer,"type>Processor action>stopped ubi>%lu raprFlowId>%d ",
						   ubi, raprFlowID);
	    rapr.LogEvent("RAPR",msgBuffer);
	    

	    if (interruptTimer.IsActive()) 	
		{
			Loggit (1,"jm: Processor:: Stop - clearing interrupt timer () \n");
			interruptTimer.Deactivate();
		}

	    ClearState();

	    if (timeoutLogicID)
			Loggit (1,"jm: Processor::Stop - leftover TO logic id %d() \n", 
					    timeoutLogicID);

	    //rapr.LogicTable().DoLogicID(timeoutLogicID);

	}
	return true;       

} // end Processor::Stop






/******************************************************************************
 *
 *
 */

const StringMapper Processor::PROC_OPT_LIST [] =  
{
    {"ABS_MEM", 	ABS_MEM},
    {"ALT_MEM", 	ALT_MEM},		// bool
    {"USR_CPU", 	USR_CPU},		// bool
    {"RW_DISK", 	RW_DISK},
    {"DIR_RW_DISK", DIR_RW_DISK},
    {"TIMESLICE", 	TIMESLICE},
	{"ENCODER",		ENCODER},	
    {"PROCESSOR", 	PROC_TOKEN},	// bool; ParseOptions sends this too...
    { NULL,			BAD_PROC_OPTION}   
}; 



const char * NextToken (const char * ptr) // , int * len)
{
	if (!ptr || *ptr == '\0')
		return NULL;
	
	while (*ptr && ! isspace (*ptr)) ptr++; // ,++len;
	while (*ptr &&   isspace (*ptr)) ptr++; // ,++len;

	if (ptr) 
		return ptr;

	return NULL;
}

bool Processor::ParseProcessorOptions (const char* lineBuffer)
{
	const char * ptr = lineBuffer;
	bool	got_proc = false;
	int		tmp_int;

  	if (!ptr) 
  		return false;

  	char fieldBuffer[SCRIPT_LINE_MAX+1];

    ProcOption result; 	


	while (ptr)
	{
  		if (1 != sscanf (ptr,"%s",fieldBuffer))
		{
			if (!got_proc)
				Loggit (0,"Parse Processor Options Invalid string - no tokens\n");
			return true;
		}

    	const StringMapper * m 	= PROC_OPT_LIST;
    	result				= BAD_PROC_OPTION;

    	while ((*m).string && BAD_PROC_OPTION != (*m).key)
    	{
        	if (!strncasecmp (fieldBuffer, (*m).string, strlen ((*m).string)) )
        	{        
            	result = (ProcOption) ((*m).key);
				break;
        	}
        	m++;
    	}

		ptr += strlen (fieldBuffer);
		ptr = NextToken (ptr); 

		if ( ! ptr)
		{
			Loggit (0,"Parse Processor Options Invalid - token <%s> w/o parameter\n",
					(*m).string);
			return true;
		}

		switch (result)
		{
	    case PROC_TOKEN:
			got_proc	= true;
			Loggit (0,"Got PROCESSOR in parse proc - field %s\n", fieldBuffer);
			continue;

	    case BAD_PROC_OPTION:
	    default:
		  	Loggit (0,"Parse Processor Options() Error: Invalid token <%s>\n", fieldBuffer);
			continue;

	    case ENCODER:		// video codex encoder
	      
			if (1 != sscanf(ptr,"%s",fieldBuffer))  
			   	/* (1 != sscanf(fieldBuffer,"%s",tmp_str)) ) */
		  	{
				Loggit (0,"Parse Processor Options() Error: invalid Encoder arg\n");
		    	return false;
		  	}
		
			Loggit (0,"jm: Got ENCODER %s \n", fieldBuffer);

			do_thread = true;
			
			break;

	    case ABS_MEM:
	      
			if (1 != sscanf(ptr,"%s",fieldBuffer))
			{
		  		Loggit (0,"Parse Processor Options() Error: missing abs_mem \n");
		  		return false;
			}
		
			if (1 != sscanf(fieldBuffer,"%i",&abs_mem))
		  	{
				Loggit (0,"Parse Processor Options() Error: invalid \n");
		    	return false;
		  	}

		
			Loggit (0,"jm: Got ABS_MEM %d KBs\n", abs_mem);
			
			break;


	    case ALT_MEM:
			if (1 != sscanf(ptr,"%s",fieldBuffer))
			{
		  		Loggit (0,"Parse ProcessorOptions() Error: missing alt_mem \n");
		  		return false;
			}
		
			if (1 != sscanf(fieldBuffer, "%i", &tmp_int))
		  	{
		    	Loggit (0,"Parse ProcessorOptions() Error: invalid \n");
		    	return false;
		  	}

			alt_mem = (tmp_int)? true : false;

			Loggit (0,"jm: Got ALT_MEM %d (bool)\n", alt_mem);
		
			break;


	    case USR_CPU:
			if (1 != sscanf(ptr,"%s",fieldBuffer))
			{
		  		Loggit (0,"Parse ProcessorOptions() Error: missing usr_cpu \n");
		  		return false;
			}
		
			if (1 != sscanf(fieldBuffer, "%i", &tmp_int))
		  	{
		    	Loggit (0,"Parse ProcessorOptions() Error: invalid usr_cpu \n");
		    	return false;
		  	}
			usr_cpu = (tmp_int)? true: false;

			usr_cpu = (usr_cpu)? true : false;

			Loggit (0,"jm: Got USR_CPU %d (bool)\n", usr_cpu);
			break;


	    case DIR_RW_DISK:
			rw_disk_dir[0] = '\0';

			if (1 != sscanf(ptr,"%s",fieldBuffer))
			{
		  		Loggit (0,"Parse ProcessorOptions() Error: missing rw_disk dir \n");
		  		return false;
			}
		
			if (1 != sscanf(fieldBuffer,"%s", rw_disk_dir))
		  	{
		   		Loggit (0,"Parse ProcessorOptions() Error: invalid rw_disk_dir \n");
		   		return false;
		  	}

			Loggit (0,"jm: Got DIR_RW_DISK %s \n", rw_disk_dir);
			break;

	    case RW_DISK:
			if (1 != sscanf(ptr,"%s",fieldBuffer))
			{
		  		Loggit (0,"Parse ProcessorOptions() Error: missing rw_disk \n");
		  		return false;
			}
		
			if (1 != sscanf(fieldBuffer,"%i",&rw_disk))
		  	{
		    	Loggit (0,"Parse ProcessorOptions() Error: invalid rw_disk \n");
		    	return false;
		  	}

			Loggit (0,"jm: Got RW_DISK %d KBs\n", rw_disk);
			break;

	    case TIMESLICE:
	      
			if (1 != sscanf(ptr,"%s",fieldBuffer))
			{
		  		Loggit (0,"Parse ProcessorOptions() Error: missing timeslice \n");
		  		return false;
			}
		
			if (1 != sscanf(fieldBuffer,"%lf",&timeslice))
		  	{
		    	Loggit (0,"Parse ProcessorOptions() Error: invalid timeslice param \n");
		    	return false;
		  	}

			Loggit (0,"jm: Got TIMESLICE %lf seconds\n", timeslice);
			break;

		} // end switch 
	  
		ptr = NextToken (ptr);

		/* if (ptr) Loggit (0,"jm:  bottom{%s}\n", ptr); */
	}

	return true;

}




bool Processor::ProcessEvent (const MgenMsg& theMsg)
{
	// Processor shouldn't get any response!
	Loggit (1,"jm: Processor::ProcessEvent() \n");
	return false;

} // end Processor::ProcessEvent




void Processor::ClearState()
{
	Loggit (1,"jm: Processor::ClearState() \n");

	if (endBehaviorTimer.IsActive()) 
		endBehaviorTimer.Deactivate();

	BehaviorEvent::ClearState();


} // end Processor::ClearState






bool Processor::OnInterrupt (ProtoTimer& theTimer) 
{
	Loggit (0,"jm: Processor::OnInterrupt() GOT TIMER, reset to %f - repeat at %d\n",
			Processor::interruptPeriod, theTimer.GetRepeat() );

    if (abs_mem)
		(void) GrabMemory ();

    if (rw_disk)
		(void) ReadWriteDisk ();

	if (do_thread)
		(void) VideoEnCodec ();		// will take params....

	/* theTimer.SetInterval (Processor::interruptPeriod);    */
	/* theTimer.SetListener (this, &Processor::OnInterrupt);   */
	/* BehaviorEvent::timer_mgr.ActivateTimer (theTimer);  */

	return true; 		// true: repeat works - if false and repeat, goes double time!!
}





bool Processor::OnBehaviorTimeout(ProtoTimer& /*theTimer*/)
{
	// log event
	char msgBuffer[512];
	sprintf(msgBuffer,"type>Processor action>timeout ubi>%lu ",ubi);
	rapr.LogEvent("RAPR",msgBuffer);


	if (interruptTimer.IsActive()) 
	{
		Loggit (3,"jm: Timeout() had active timer\n");
		interruptTimer.Deactivate();
	}

	if (absmem_ptr)
	{
		Loggit (0,"jm: Processor::behaviour timeout - freeing abs mem() \n");
		free (absmem_ptr);
		absmem_ptr = NULL;
	}

#if defined (PROC_THREADS)
	if (do_thread && CodecThread)
	{
		Loggit (0,"Processor::OnBehaviorTimeout - killing thread\n");
		if (CodecThread->KillThread() == false)
		{
			Loggit (0,"Processor::OnBehaviorTimeout - couldn't kill thread \n");
			// try again later or at stop
		}
		else
		{
			delete CodecThread;
			CodecThread = NULL;
		}
	}
#endif // PROC_THREADS



	if (rw_disk)
	{
		// nothing to do?
	}

	if (strm_pid) 
	{ 
		Loggit (0,"Processor::behaviour timeout - killing pid (%d)\n", strm_pid);
		kill (strm_pid, SIGTERM);
		strm_pid = 0;
	} 


	ClearState();

	return false; 	// ProtoTimerMgr will what? not reschedule timer if true?

} // end Processor::OnBehaviorTimeout


bool Processor::VideoEnCodec (void)
{
#if ! defined (PROC_THREADS)

	Loggit (0, "Processor::CreateThread Error: threading of thirdparty libs disabled\n");
	return false;

#else  // PROC_THREADS enabled:

	unsigned long get_thrd_id (void);
	void * dirac_encode_thrd (void *);
	
	if (CodecThread) 
	{
		Loggit (0, "Processor::VidEnCodec: thread already created, kill/restart\n");

		if (CodecThread->KillThread() == false)
		{
			Loggit (0, "Processor::VidEnCodec: Error can't kill/restart\n");
			return false;
		}
		// else continue on...
	}


	CodecThread = new Threader();


	if (CodecThread->CreateThread (dirac_encode_thrd, NULL) != true)
	{
		Loggit (0, "Processor::VideoEnCodec Error: creating encoder thread failed, cleanup\n");

		delete CodecThread;
		base_thread_id = 0;
		return false;
	}

	base_thread_id = get_thrd_id ();

	Loggit (0, "Processor::VideoEnCodec created encoder thread\n" );
	return true;


#endif // PROC_THREADS

}

//
//	Fork/exec a process until we kill it at OnBehaveTimout
//
bool Processor::RunCPU (void)
{
	const char * run_str = "./2streams";

	strm_pid = fork ();

	if (strm_pid < 0)
	{
	   	Loggit (0,"Error: Proc::RunCPU - bad fork <%s>, failing\n", strerror(errno));
		return false;
	}

	if (strm_pid)
	{
    	Loggit  (0,"jm: RunCPU spawned pid %d\n", strm_pid);
		return true;
	}

	// in new process here.

	close (1);
	open ("/dev/pts/5", O_APPEND, 0); 	
	execl (run_str, run_str, ">/dev/pts/5", NULL);		// hmmm.  I've broken this somehow? (jm:)

	// not reached unless failed

    Loggit  (0,"Processor::RunCPU exec <%s> failed!! <%s>\n", run_str, strerror(errno));
	exit (1);
}


//
//	read x kbs from rw_disk_dir/rapr_readfile.yy to rw_disk_dir/rapr_writefile.yy
//

bool Processor::ReadWriteDisk (void)
{
	static int roundrobin = 0;

    Loggit  (2,"jm: ReadWriteDisk %d KB\n", rw_disk);

	if ( ! rw_disk_dir[0])
	{
    	Loggit  (0,"Processor::ReadWriteDisk Error: no directory name\n");
		return false;
	}

	char 	readfile [1024];
	char 	writefile [1024];
	int		rd_fd = 0, wr_fd = 0;
	uint	xferbytes = rw_disk * 1024;
	struct stat 	statbuf;
	
	if (roundrobin > 9) roundrobin = 0;

	sprintf (readfile, "%s/%s.0%d", rw_disk_dir, "rapr_readfile", roundrobin);
	sprintf (writefile, "%s/%s.0%d", rw_disk_dir, "rapr_writefile", roundrobin);

	++roundrobin;

    Loggit  (0,"Processor::ReadWriteDisk - coping %d KBs from <%s> to <%s>\n", rw_disk, readfile, writefile);

	if (stat (readfile, &statbuf))
	{
	   	Loggit (0,"Proc::RWDisk Error - bad readfile <%s>, <%s>\n", readfile, strerror(errno));
		return false;
	}

	rd_fd  = open (readfile, O_RDONLY, 0);
	if ( ! rd_fd )
	{
	   	Loggit (0,"Proc::RWDisk: Error - bad readfile open <%s>, <%s>\n", readfile, strerror(errno));
		return false;
	}


	wr_fd  = open (writefile, O_WRONLY | O_CREAT | O_TRUNC,  /* O_EXCL ?? */
							  S_IRWXU | S_IRWXG | S_IRWXO );
	if ( ! wr_fd )
	{
	   	Loggit (0,"Proc::RWDisk: Error - bad writefile open <%s>, <%s>\n", readfile, strerror(errno));
		close (rd_fd);
		return false;
	}

#define OURBLOCK	10 * 1024 
	char buffer [OURBLOCK + 1];
	int  i;

	for (i = xferbytes; i > 0 ; i -= OURBLOCK)
	{
		size_t  get;
		ssize_t got, put;

		get = (i < OURBLOCK)? i : OURBLOCK;

		got = read (rd_fd, buffer, get);
		
		if (got < 0)		// actual error
	   		Loggit (0,"Proc::RWDisk Error - bad read <%s>, failing RW\n", strerror(errno));

		if (got <= 0)		// error or eof reached
			break;

		// else, got it, write it.
	
		put = write (wr_fd, buffer, got);
		if (put < 0)
		{
	   		Loggit (0,"Proc::RWDisk Error - bad write <%s>, failing RW\n", strerror(errno));
			break;
		}
	}

	close (wr_fd);
	close (rd_fd);

	return true;
}




//
//	Grab or realloc memory 
//	if none, alloc.  if alloc'd, realloc 
//
//	nb:  jiggling tried to keep the operating system
//	from becoming complacent:  as in, don't always alloc the
//	same memory
//

bool Processor::GrabMemory (void)
{
	bool 	skip_this_round = false;
	bool 	had_mem = false;
    int		grab    = abs_mem * 1024;
    int 	partial = grab / 10;
    void  *	jiggle  = NULL;

    Loggit  (2,"jm: GrabMemory %d KB\n", abs_mem);

    if (absmem_ptr)
    {
		had_mem = true;
		skip_this_round = (alt_mem)? 1 : 0;		// don't re-alloc

    	Loggit  (3,"jm: GrabMemory, Freeing previous abs_memory first\n");
    	free (absmem_ptr);
		absmem_ptr = NULL;
	}

	if (skip_this_round)
	{
    	Loggit  (1,"jm: GrabMemory skipping this round\n\n");
		return true;
	}

	if (had_mem)
	{
		jiggle = calloc (partial * 2, 1); 
		if ( ! jiggle )
		{
	    	// warn, but no remedy 
	    	Loggit (0,"Error: Processor::GrabMemory - partial memory alloc %d KB failed, <%s>\n", 
			   	partial * 2,
		   		strerror(errno));
    	}
	}

    for (absmem_ptr = calloc (grab, 1);  !absmem_ptr && grab > 0; grab -= partial) 
    {
    	Loggit  (0,"Processor::GrabMemory newvalue %d bytes - was %d, syserr <%s>\n", 
		         grab, (abs_mem * 1024), strerror(errno));
    }


    if (!absmem_ptr)
    {
		Loggit (0,"Error: Processor::GrabMemory - memory alloc of %d bytes failed, <%s>\n", 
				grab,
				strerror(errno));
    	if (jiggle) free (jiggle);
		return false;
    }

    Loggit  (1,"GrabMemory got %d kbs (of %d) at 0x%p (jiggle 0x%p) \n", 
			 grab/1024, abs_mem, absmem_ptr, jiggle );

    if (jiggle) free (jiggle);

	// do some accessing to force it out of virtual

	partial /= 100;		// smaller yet

	int 	x;
	char *  virtmem = (char *) absmem_ptr;

	for (x = grab - 1; x > 0; x -= partial)
		virtmem [x] = x & 0xff;		// stab low order into mem

	return true;
}





//
//	Utility
//
unsigned long get_thrd_id (void)
{
	return (unsigned long) pthread_self ();
}




/******************************************************************************
 *
 *		Miscellany
 *
 */



/******************************************************************************
 *
 *		My own logger, for now?
 *
 *	used with DBUG macro to get time, thrdid, __file__ and __lineno__
 *
 *	yes... the whole thing's very very! C
 *
 */

void tmp_logger (int level, const char *format, ...)
{
	if ((unsigned int) level > GetDebugLevel())
		return;

	char newfmt [300];
	va_list arg;
	unsigned long id = get_thrd_id ();

	sprintf (newfmt, "%-23.23s[%4.4d] %-5.5lx %s", 
					 _filenm, 
					 _lineno, 
					 (base_thread_id && 
					  base_thread_id != id) ? id : 0L,  
					  format);

	format = newfmt;

	va_start (arg, format);
	
	vfprintf (stdout, format, arg);
	fflush   (stdout);

	va_end (arg);
}




//
//	end of processor.cpp

