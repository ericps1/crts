/* $Id: threadedProc.h,v 1.1.1.1 2007-01-10 21:14:17 lthompso Exp $ */
/*
 *	Threading Class 
 *
 *
 */

#ifndef _THREADED_PROC_H
#define _THREADED_PROC_H


class Threader 
{
	// friend Processor;

public:
	Threader ();
	~Threader ();
	bool CreateThread (void *(*func)(void *), char **args);
	bool KillThread  (void);
	bool init_thread_attr (void);

private:

	// volitle: base thread can read, only spawned thread can write
	volatile int x;		
	pthread_t		thread_id;		// will class this..
	pthread_attr_t	thread_attr;

};

#endif // _THREADED_PROC_H
