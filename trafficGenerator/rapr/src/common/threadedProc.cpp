/* $Id: threadedProc.cpp,v 1.1.1.1 2007-01-10 21:14:17 lthompso Exp $ */
/**
 *		Threaded class for mutlithreaded processor operations.
 *
 *	Simple thread handling divorced from the rest of RAPR.
 *	There is no common rapr data to have conflicts on, so 
 *	we can pretty freely have a Processor object create a
 *	Threader object:  Threader becomes the base thread and
 *	runs 'func' in a single thread.  Mutliple Threaders can
 *	be kicked off simultaneously by the same object (though
 *	the Processor class isn't set up for that so far.
 *
 *	Todo:  Need to a) catch thread death - or it'd be nice.
 *	b) make sure killed threads die nicely and clean up their
 *	mess.
 *
 *	Its a prototype, ok?... This only works with the dirac encoder right 
 *	now or at least that the only testing done.  One note about dirac:
 *	any dirac encoding that's killed with cancel/join will leave
 *	a lot of un-reclaimed memory.  Better thread control will fix
 *	this, but for now, just beware that spitting out a ton of
 *	threads and killing them before they complete their work will
 *	become cumulative and result in some very big leaks - just
 *	killing the final one at STOP is no problem.
 *
 *	NB: Conditionally compiled only if -DPROC_THREADS - see the makefile.
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



#if defined (PROC_THREADS)



Threader::Threader() : thread_id(0)
{
	Log (1,"Threader::Threader() \n");
}


Threader::~Threader()
{
	Log (1,"jm: Threader::~Threader () \n");
}


/******************************************************************************
 *
 * 	Threader::CreateThread (void *(*func)(void *), char **args)
 *
 *	Create a thread to run either to its completion or until 
 *	we call KillThread().
 *
 */

bool Threader::CreateThread (void *(*func)(void *), char **args)
{
	int x;
	int arg =  0;	//  1 for continuous encoding loop - but very leaky that...

	if (args == NULL)
		arg = 0;


	if (thread_id) 
	{
		Log (0, "Threader::StartThread Error, thread already created? 0x%x\n", thread_id);

		if (KillThread () != true)
		{
			return false;
		}
		thread_id = 0;
		// continue on...
	}


	// (void) init_thread_attr ();
	// x = pthread_create (&thread_id, &thread_attr, func, (void *)&arg);

	x = pthread_create (&thread_id, NULL, func, (void *)&arg);

	if (x) 
	{
		Log (0, "Threader::CreateThread: Error: creating encoder thread failed <%s>\n", strerror(errno));
		return false;
	}

	Log (0, "Threader::CreateThread: created encoder thread 0x%x\n", thread_id);
	return true;
}


/******************************************************************************
 * 	bool Threader::KillThread(void)
 *
 *	Kill a thread if its still unaccounted for (ie if(thread_id)).
 *	Note that cancel will fail if the thread has already exited,
 *	but we still need to 'join' with it to collect it.
 *
 */

bool Threader::KillThread(void)
{
	if (! thread_id)
	{
		Log (3,"Threader::KillThread: no thread.\n");
		return true;
	}

	Log (0,"Threader::KillThread - killing worker thread (0x%x)\n", thread_id);


	if (pthread_cancel (thread_id))
	{
		Log (1,"Threader::KillThread: Warning: pthread_cancel failed, but see what join says...<%s>\n", 
			    strerror(errno));
	}


	if (pthread_join (thread_id, NULL))
	{
		Log (0,"Threader::KillThread Error? pthread_join id 0x%x <%s>\n", 
			    thread_id, strerror(errno));
		// continue - join complains about no thread as well
	}


	Log (0,"Threader::KillThread pthread_join OK: id 0x%x \n", thread_id );

	thread_id = 0;

	// if no other thread, base_thread_id to 0 too ...
	return true;
}


//! unused for now - testing DETACHED threads
//	Set up our threading
//
bool Threader::init_thread_attr (void)
{
	/********** 
    int old_cancel;
    assert (pthread_setcanceltype (PTHREAD_CANCEL_DEFERRED, &old_cancel) == 0);
    assert (old_cancel == PTHREAD_CANCEL_DEFERRED);
 	*****************/

    pthread_attr_init (&thread_attr);		// currently: joinable

    /* pthread_attr_setdetachstate (&thread_attr, PTHREAD_CREATE_DETACHED); */
    /* pthread_attr_setstacksize (&thread_attr, 0x40000 ); */

    return 0;
}



#endif // PROC_THREADS

 
