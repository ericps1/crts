#include "rapr.h"
#include "protoTimer.h"
#include "behaviorEvent.h"
#include "processor.h"

// So we can call stop function
#include "raprApp.h"

// Mgen
#include "mgen.h"

#include <stdio.h>
#include <ctype.h>     // for toupper()
#include <sys/types.h> // for stat
#include <sys/stat.h>  // for stat
#ifdef UNIX
#include <unistd.h>    // for state
#endif

// So we can tell the user if we are using a default host id.
static bool hostIDSet = false;

Rapr::Rapr(ProtoTimerMgr& timerMgr,
	   ProtoSocket::Notifier& socketNotifier,
	   ProtoPipe& raprControlPipe,
	   bool& raprControlRemote,
	   Mgen& mgen)
  : timer_mgr(timerMgr), 
    rapr_prng(NULL),
    started(false),stopped(false),
    start_hour(0), start_min(0), start_sec(-1.0),
    start_gmt(false),start_time_lock(false),
    active_stream(NULL),
    mgen(mgen),logicTable(this),
    log_file(NULL), log_binary(false),local_time(false),log_flush(false),
    log_verbose(false),
    load_emulator(false),
    log_file_lock(false),log_open(false),log_empty(true),
    offset(-1.0),offset_lock(false),offset_pending(false),
    save_path(NULL),save_path_lock(false),
    rapr_control_pipe(raprControlPipe),
    rapr_control_remote(raprControlRemote)
{
  SetSocketNotifier(&socketNotifier);

  start_timer.SetListener(this, &Rapr::OnStartTimeout);
  start_timer.SetInterval(0.0);
  start_timer.SetRepeat(0);

  behavior_event_timer.SetListener(this,&Rapr::OnBehaviorStartTimeout);
  behavior_event_timer.SetInterval(0.0);
  behavior_event_timer.SetRepeat(-1);
}

Rapr::~Rapr()
{
  DMSG(1,"Last Seed>%d\n",number_generator.GetSeed());
  DMSG(1,"Last rtiSeed>%d\n",number_generator.GetRtiSeed());
  Stop();
}

bool Rapr::LoadFlowIDs()
{

  int FLOWIDSIZE = 100000; // ljt make this a global
  for (int i=1; i < FLOWIDSIZE; i++) 
    {
      number_generator.SetFlowID(i); 
    }
  
  return true;
} // end Rapr::LoadFlowIDs()

bool Rapr::LoadState()
{
  if (!LoadFlowIDs()) return false;

  return true;
} // end Rapr::LoadState()

bool Rapr::Start()
{
  if (!GetNumberGenerator().GetInitialSeed())
    {
      DMSG(0,"Rapr::Start() Warning: Initial seed has not been set, using HostID %hi as the initial seed.\n",GetNumberGenerator().GetHostID());

      GetNumberGenerator().SetInitialSeed(GetNumberGenerator().GetHostID());
    }
    struct timeval currentTime;
    ProtoSystemTime(currentTime);
    
    if (start_sec < 0.0)
    {
        // start immediately
        if (!mgen.Start())
        {
            DMSG(0,"Rapr::Start() Error: error starting mgen!\n");
        }
        // ljt do logging
        if (log_file)
        {
            // Log START event - no binary support yet
	  struct tm* timePtr;
	  if (local_time)
	    timePtr = localtime((time_t*)&currentTime.tv_sec);
	  else
	    timePtr = gmtime((time_t*)&currentTime.tv_sec);

	  fprintf(log_file, "%02d:%02d:%02d.%06lu app>RAPR type>Application action>ApplicationStartUp\n",
                    timePtr->tm_hour, timePtr->tm_min, 
                    timePtr->tm_sec, (unsigned long)currentTime.tv_usec);
            if (log_empty) log_empty = false;
            fflush(log_file);
            
        }
        
        offset_pending = true;
        
        // activate behaviors according to "offset" time
        StartEvents(offset);
        offset_pending = false;
        
    }
    else // schedule absolute start time
    {
        // Make sure there are pending events
        if (!behavior_event_list.IsEmpty())
        {
            // Calculate start time delta and schedule start_timer
            // (can delay start up to 12 hours into future)
            struct tm now;
            memcpy(&now, localtime((time_t*)&currentTime.tv_sec), sizeof(struct tm));
            double nowSec = now.tm_hour*3600 + now.tm_min*60 + now.tm_sec;
            double startSec = start_hour*3600 + start_min*60 + start_sec;
            double delta = startSec - nowSec;
            if (delta < 0.0) delta += (24 * 3600);
            if (delta > 12*3600)
            {
                DMSG(0,"Rapr::Start() Error: Specified start time has already elapsed\n");
                return false;
            }
            start_timer.SetInterval(delta);
            timer_mgr.ActivateTimer(start_timer);
        }
    }

    started = true;
    return true;
} // end Rapr::Start()

void Rapr::StopMgen()
{
  // Should really only be used by fault command!!

  mgen.Stop();
}
void Rapr::StopEvents()
{
  // Used by fault command.
  if (stopped)
    return;

  if (behavior_event_timer.IsActive()) behavior_event_timer.Deactivate();
  behavior_event_list.StopEvents();

} // Rapr::StopEvents

void Rapr::Stop()
{
  // We must have been stopped via the script
  if (stopped)
    return;

  if (start_timer.IsActive()) start_timer.Deactivate();
  if (behavior_event_timer.IsActive()) behavior_event_timer.Deactivate();
  behavior_event_list.Destroy();
  if (started)
  {
      if (log_file)
      {
          // log STOP event
          struct timeval currentTime;
          ProtoSystemTime(currentTime);
	  struct tm* timePtr;

	  if (local_time)
	    timePtr = localtime((time_t*)&currentTime.tv_sec);
	  else
	    timePtr = gmtime((time_t*)&currentTime.tv_sec);

          fprintf(log_file, "%02d:%02d:%02d.%06lu RAPRSTOP\n",
                  timePtr->tm_hour, timePtr->tm_min, 
                  timePtr->tm_sec, (unsigned long)currentTime.tv_usec);
          
          fflush(log_file);
          if ((log_file != stdout) && (stderr != log_file))
          {
              fclose(log_file);
              log_file = NULL;   
          }
      }
      // Save current offset and pending flow sequence state
      if (save_path)
      {
          FILE* saveFile = fopen(save_path, "w+");
          if (saveFile)
          {
              fprintf(saveFile, "OFFSET %f\n", GetCurrentOffset());
              fclose(saveFile);
          }
          else
          {
              DMSG(0, "Mgen::Stop() Error: opening save file: %s\n", strerror(errno));   
          }
      }
      started = false;
      
  }
  
  rapr_control_pipe.Close();
  
  // In case we are stopped from the script
  mgen.Stop();
  
  stopped = true;
} // end Rapr::Stop()

bool Rapr::OnStartTimeout(ProtoTimer& /*theTimer*/)
{
  start_sec = -1.0;
  Start();
  return true;
} // Rapr::OnStartTimeout()

bool Rapr::OnBehaviorStartTimeout(ProtoTimer& /*theTimer*/)
{

    // this needs to be rewritten but I don't
    // want to change it at this point ljt
    
    // 1) Process next pending behavior event
    ASSERT(next_behavior_event);
    double eventTime;
    
    DMSG(1,"\nstart>%.2f end>%.2f off>%.2f ",next_behavior_event->GetBehaviorStartTime(),next_behavior_event->GetBehaviorEndTime(),offset);
    
    // 2) But make sure it's stop time has not      
    // What is this stream/offset stuff??
    if ((!(next_behavior_event->GetType() == BehaviorEvent::STREAM
          && next_behavior_event->GetBehaviorStartTime() < offset))
      &&
        next_behavior_event->InUse())

    {
        // If the stop time is after our offset, start 
        // er up for any remaining time
        if ((next_behavior_event->GetBehaviorEndTime() > offset)
            
            // Or no end time specified & start time after offset
            || ((!next_behavior_event->GetBehaviorEndTime() 
                 || next_behavior_event->GetBehaviorEndTime() == -1)
                &&
                next_behavior_event->GetBehaviorStartTime() >= offset)
            
            // If it's a duration object, see if the
            // duration would last past the offset time
            || (next_behavior_event->GetDuration() 
                
                && ((next_behavior_event->GetBehaviorStartTime()
                     + next_behavior_event->GetBehaviorEndTime())
                    > offset))
            
            // Finally, start all raprevents as we see them
            || (next_behavior_event->GetType() == BehaviorEvent::RAPREVENT
                && ((RaprEvent*)next_behavior_event)->GetOverrideStartTime()))
        {
            // Reset duration end times considering offset
            if ((next_behavior_event->GetBehaviorStartTime() < offset
                 && next_behavior_event->GetDuration())
                
                && ((next_behavior_event->GetBehaviorStartTime()
                     + next_behavior_event->GetBehaviorEndTime())
                    > offset))
            {
                
                next_behavior_event->SetBehaviorEndTime(((next_behavior_event->GetBehaviorStartTime() + next_behavior_event->GetBehaviorEndTime()) - offset));
                
            }
            StartBehaviorEvent(*next_behavior_event);
            // To calculate next interval
            eventTime = next_behavior_event->GetBehaviorStartTime(offset);
            next_behavior_event = (BehaviorEvent*)next_behavior_event->Next();
            
        }
        else
        {
            // Scheduled behavior is over, remove it
            BehaviorEvent* nextEvent = next_behavior_event;
            eventTime = next_behavior_event->GetBehaviorStartTime(offset); 
            next_behavior_event = nextEvent->GetNext();            
            behavior_event_list.Remove(nextEvent);
            delete nextEvent;
            
        }
    }
    else
    {
        // Scheduled behavior is over, remove it
        BehaviorEvent* nextEvent = next_behavior_event;
        eventTime = next_behavior_event->GetBehaviorStartTime(offset);        
        next_behavior_event = nextEvent->GetNext();        
        behavior_event_list.Remove(nextEvent);
        delete nextEvent;
        
        
    }
    // 3) Install next Behavior Event timeout (or kill timer)
    if (next_behavior_event)
    {
        // eventTime from previous event
        double nextInterval = next_behavior_event->GetBehaviorStartTime(offset) - eventTime;
        nextInterval = nextInterval > 0.0 ? nextInterval : 0.0;
        behavior_event_timer.SetInterval(nextInterval);
                
        DMSG(1,"Rapr::StartBehaviorEvent() UBI> %lu eventTime> %.2f nextInterval> %.2f eventStartTime> %.2f eventEndTime>%.2f startTimeOffset> %.2f type>%s\n",next_behavior_event->GetUBI(),eventTime,nextInterval,next_behavior_event->GetBehaviorStartTime(),next_behavior_event->GetBehaviorEndTime(),next_behavior_event->GetBehaviorStartTime(offset),next_behavior_event->GetStringFromEventType(next_behavior_event->GetType()));
        
        return true;
    }
    else 
    {
        if (behavior_event_timer.IsActive()) behavior_event_timer.Deactivate();
        return false;
    }
} // end Rapr::OnBehaviorStartTimeout()

void Rapr::StartBehaviorEvent(BehaviorEvent& theEvent)
{
  theEvent.OnStartUp();

} // end Rapr::StartBehaviorEvent

bool Rapr::SendMgenCommand(const char* cmd, const char* val)
{  

    if (!mgen.OnCommand(Mgen::GetCommandFromString(cmd),val,false))
	{
        if (mgen.IsStarted()) // For fault processing
          
          DMSG(0,"Rapr::SendMgenCommand() Error: processing mgen command: %s\n",val);
        return false;
	}
    return true;

} // end Rapr::SendMgenCommand

// This tells if the command is valid and whether args are expected
Rapr::CmdType Rapr::GetCmdType(const char* cmd)
{
    if (!cmd) return CMD_INVALID;
    char upperCmd[32];  // all commands < 32 characters
    unsigned int len = strlen(cmd);
    len = len < 31 ? len : 31;
    unsigned int i;
    for (i = 0; i < 31; i++)
        upperCmd[i] = toupper(cmd[i]);
    
    bool matched = false;
    const StringMapper* m = COMMAND_LIST;
    CmdType type = CMD_INVALID;
    while (INVALID_COMMAND != (*m).key)
    {
        if (!strncmp(upperCmd, (*m).string+1, len))
        {
            if (matched)
            {
                // ambiguous command (command should match only once)
                return CMD_INVALID;
            }
            else
            {
                matched = true;   
                if ('+' == (*m).string[0])
                    type = CMD_ARG;
                else
                    type = CMD_NOARG;
            }
        }
        m++;
    }
    return type; 

} // end Rapr::GetCmdType()
// Global command processing
const StringMapper Rapr::COMMAND_LIST[] =
{
  // Rapr Global commands:
  {"+RAPRPIPE",               RAPRPIPE},
  {"+LOAD_DICTIONARY",        LOAD_DICTIONARY},
  // RaprApp Global commands:
  {"+OFFEVENT",               OFFEVENT},
  {"+MGENEVENT",              MGENEVENT},
  // Mgen global commands:
  {"+START",                  START},
  {"+STOP",                   STOP},
  {"+EVENT",                  EVENT},
  {"+INPUT",                  INPUT},
  {"+RAPRLOG",                RAPRLOG},
  {"+OVERWRITE_RAPRLOG",      OVERWRITE_RAPRLOG},
  {"+MGENLOG",                MGENLOG},
  {"+OVERWRITE_MGENLOG",      OVERWRITE_MGENLOG},
  {"+SAVE",                   SAVE},
  {"+DEBUG",                  DEBUG},
  {"-VERBOSE",                VERBOSE},
  {"-ENABLE_LOAD_EMULATOR",   ENABLE_LOAD_EMULATOR},
  {"+INITIAL_SEED",           INITIAL_SEED},
  {"+OFFSET",                 OFFSET},
  {"-TXLOG",	              TXLOG},
  {"-LOCALTIME",              LOCALTIME},
  {"-NOLOG",                  NOLOG},
  {"-BINARY",                 BINARY},
  {"-FLUSH",                  FLUSH},
  {"+LABEL",      LABEL},
  {"+RXBUFFER",   RXBUFFER},
  {"+TXBUFFER",   TXBUFFER},
  {"+TOS",        TOS},
  {"+TTL",        TTL},
  {"-INTER",      INVALID_COMMAND}, // to deconflict with INTERROGATIVE
  {"+INTERFACE",  INTERFACE},
  {"-CHECKSUM",   CHECKSUM},
  {"-TXCHECKSUM", TXCHECKSUM},
  {"-RXCHECKSUM", RXCHECKSUM},  
  {"+HOSTID",     HOSTID},
  {"+OFF",        INVALID_COMMAND},  // to deconflict "offset" from "off" event
  {"+JOURNAL",    JOURNAL},
  {"+JINTERVAL",    JINTERVAL},
  {NULL,                      INVALID_COMMAND}   
};


Rapr::Command Rapr::GetCommandFromString(const char* string)
{
    // Make comparison case-insensitive
    char upperString[16];
    unsigned int len = strlen(string);

    len = len < 16 ? len : 16;
    
    for (unsigned int i = 0 ; i < len; i++)
        upperString[i] = toupper(string[i]);
    const StringMapper* m = COMMAND_LIST;
    Rapr::Command theCommand = INVALID_COMMAND;
    while (NULL != (*m).string)
    {
		 if (!strncmp(upperString, (*m).string+1, len)) {
            theCommand = ((Command)((*m).key));
		 }
        m++;
    }
    return theCommand;
}  // end Rapr::GetCommandFromString()

bool Rapr::OnCommand(Rapr::Command cmd, const char* arg,BehaviorEvent::EventSource eventSource,bool override)
{

    switch (cmd)
    {
    case STOPEVENT:
      {
          unsigned short flowId;
          const char * ptr = arg;
          ptr = strstr(arg,"flow>");
          
          if (ptr)
          {
              
              if (1 == sscanf(ptr,"flow> %hu",&flowId))
              {
                  // ljt do we really need to find the flow
                  // if we are clearing all object state upon
                  // timeout?  Leave for now until streams
                  // are implemented.  Just a little extra
                  // processing.

		DMSG(2,"Rapr::OnCommand() STOPEVENT flowId %d\n",flowId);

		BehaviorEvent* theEvent = GetBehaviorEventList().FindBEByFlowID(flowId); 

		// This is all terribly messy and resulted from 
		// changes to mgen that don't play well with what
		// rapr was expecting (e.g. flows are stopped by
		// mgen differently than before)

		// If we still have an event that didn't stop 
		// "naturally", e.g.the event has been generated
		// by mgen due to count being exceeded or a TCP
		// socket being disconnected or something.  Fake
		// an off event so that mgen will accept subsequent
		// flow id reuse.  Is the real solution here is to
		// get rid of event checks in mgen?
		if (theEvent) 
		{      
		  // Mgen stopped the event for some reason,
		  // count or socket failure, so fake an off
		  // event.  
		  if (!theEvent->Stopped())
		    {
		      DMSG(1,"Rapr::OnCommand() Sending mgen forced off event %d\n",flowId);
		      // Don't unlock the flow ID until we get
		      // notification from mgen that the flow OFF
		      // event was processed
		      char buffer[8192];
		      sprintf(buffer,"0.001 OFF %d", flowId);
		      SendMgenCommand("event",buffer);  
		      theEvent->Stopped(true);
		    }
		  else
		    {
		      // The event has already shutdown, we can delete it now
		      // that the flow has been turned off.  (We needed to keep
		      // it around so we could turn it off)
		      if (!theEvent->InUse())
			{
			  GetBehaviorEventList().Remove(theEvent);
			}
		    }
		}
		else
		  {

		    // Otherwise we are getting notification
		    // from an event that has shutdown
		    // gracefully - and it is safe to reuse
		    // the flow id.
		    DMSG(0,"Rapr::OnCommand() Unlocking flow id>%d UNKNOWN EVENT!\n",flowId);
		    //GetNumberGenerator().UnlockFlowID(flowId);                 
		     
		  }
              }
          }	
          break;

      }
    case OFFEVENT:
      {
          unsigned short flowId;
          const char * ptr = arg;
          ptr = strstr(arg,"flow>");
          
          if (ptr)
          {
              
              if (1 == sscanf(ptr,"flow> %hu",&flowId))
              {
		// we only unlock the flow when mgen tells us it's off
		DMSG(1,"Rapr::OnCommand() OFFEVENT Received Unlocking flow id>%d\n",flowId);
		GetNumberGenerator().UnlockFlowID(flowId);

	      } 
          }	
          break;
      }
    case START:
      {
          if (!arg)
          {
              DMSG(0,"Rapr::OnCommand() Error: missing <startTime> \n");
              return false;
              
          }
          if (override || !start_time_lock)
          {
              // convert to upper case for case-insensitivity
              // search for "GMT" or "NOW" keywords
              
              char temp[32];
              unsigned int len = strlen(arg);
              len = len < 31 ? len : 31;
              unsigned int i;
              for (i = 0 ; i < len; i++)
                temp[i] = toupper(arg[i]);
              temp[i] = '\0';
              
              unsigned int startHour, startMin;
              double startSec;
              // arg should be "hr:min:sec[GMT]" or "NOW"
              if (3 == sscanf(temp, "%u:%u:%lf", &startHour, &startMin, &startSec))
              {
                  start_hour = startHour;
                  start_min = startMin;
                  start_sec = startSec;
                  if (strstr(temp, "GMT"))
                    start_gmt = true;
                  else
                    start_gmt = false;
              }
              else
              {
                  // Check for "NOW" keywork (case-insensitive)
                  if (strstr(temp, "NOW"))
                  {
                      // negative start_time_sec indicates immediate start
                      start_sec = -1.0; 
                  }
                  else
                  {
                      DMSG(0, "Rapr::OnCommand() Error: invalid START time\n");
                      return false;
                  }
              }
              start_time_lock = override; //record START command precedence
          } // end if (override || !start_time_lock)
          break;
      } // end case START
      
    case STOP:
      {
          if (!arg)
          {
              DMSG(0,"Rapr::OnCommand() Error: missing <stopTime> \n");
              return false;
              
          }
          // convert to upper case for case-insensitivity
          // search for "GMT" or "NOW" keywords
          
          char temp[512];
          unsigned int len = strlen(arg);
          len = len < 511 ? len : 511;
          unsigned int i;
          for (i = 0 ; i < len; i++)
            temp[i] = toupper(arg[i]);
          temp[i] = '\0';
          
          unsigned int stopHour, stopMin;
          double stopSec;
          bool stop_gmt = false;
          
          // arg should be "hr:min:sec[GMT]" or "NOW"
          if (3 == sscanf(temp, "%u:%u:%lf", &stopHour, &stopMin, &stopSec))
          {
              if (strstr(temp, "GMT"))
                stop_gmt = true;
              else
                stop_gmt = false;
          }
          else
          {
              // Check for "NOW" keywork (case-insensitive)
              if (strstr(temp, "NOW"))
              {
                  // negative stop_time_sec indicates immediate stop
                  stopSec = -1.0; 
              }
              else
              {
                  // Check for "DST" keywork, could be a behavior event                  
		if (!strstr(temp, "DST") && !strstr(temp,"RAPRFLOWID"))
                  {                      
                      DMSG(0, "Rapr::OnCommand() Error: No DST!\n");
                  }
                  return false; 

              }
          }
          
          struct timeval currentTime;
          ProtoSystemTime(currentTime);
          
          struct tm now;
          //	if (stop_gmt)
          memcpy(&now,localtime((time_t*)&currentTime.tv_sec),sizeof(struct tm));
          //	else
          //	  memcpy(&now,localtime((time_t*)&currentTime.tv_sec),sizeof(struct tm));
          double nowSec = now.tm_hour*3600 + now.tm_min*60 + now.tm_sec;
          double stop_sec = stopHour*3600 + stopMin* 60 + stopSec;
          double delta = stop_sec - nowSec;
          if (delta < 0.0) delta += (24 * 3600);
          if (delta > 12*3600)
          {
              DMSG(0,"Rapr::OnCommand() Error: specified stop time has already elapsed.\n");
              return false;
          }
          RaprEvent* theEvent;
          theEvent = new RaprEvent(timer_mgr,number_generator.GetUBI(),*this);
          theEvent->SetRaprEventType(RaprEvent::STOP);
          if (stopSec < 0.0)
            theEvent->SetBehaviorStartTime(-1.0);
          else
            theEvent->SetBehaviorStartTime(delta);
          InsertBehaviorEvent(theEvent);
          
          break;
      } // end case STOP
    case EVENT:
      {
          char eTmp[512];
          RaprPRNG *prng = new RaprPRNG(number_generator.GetNextRtiSeed());
          DMSG(3,"Rapr::ParseEvent() Using rti seed>%d\n",prng->GetSeed());
          
          RaprStringVector *vec;
          strcpy(eTmp,arg);
          vec = LogicTable().TranslateString(eTmp,prng);
          if (vec != NULL) {
              for (unsigned int i=0;i<vec->size();i++) {

                  // Since we want to retranslate periodic events
                  // each time we start up, make sure we only have
                  // one periodic event definition to work with.
                  // (Temporary solution)
                  
                  // Comparing to PERIODIC INT to deconflict with
                  // PERIODIC [ pattern ljt
                  if ((vec->size() > 1) 
                      && (strstr((*vec)[i],"PERIODIC INT")))
                  {
                      DMSG(0,"Rapr::ParseScript() Error: PERIODIC behavior events cannot be translated into multiple events.\n");
                      DMSG(0, "Rapr::ParseScript() Error: invalid rapr logic table line: %s\n",eTmp);
                      number_generator.DecrementRtiSeed();
                      return false;
                  }



                  if (!ParseEvent((*vec)[i], 0,NULL,eventSource,prng,eTmp))
                  {
                      DMSG(0,"Rapr::OnCommand() Error: error parsing event\n");
                      // reset running seed upon failure
                      number_generator.DecrementRtiSeed();
                      return false;   
                  }
              }
              delete vec;
          }
      }
      break;
    case INPUT:
      {
          if (!ParseScript(arg))
          {
              DMSG(0,"Rapr::OnCommand() Error: error parsing script\n");
              return false;
          }
          break;
      }
    case RAPRLOG:
      {
          if (override || !log_file_lock)
          {
              if (!OpenLog(arg,true,false))
              {
                  DMSG(0,"Rapr::OnCommand() Error: rapr log file open error: %s\n", strerror(errno));
                  return false;
              }
              log_file_lock = override;
          }
          break;
      }
    case OVERWRITE_RAPRLOG:
      {
          if (override || !log_file_lock)
          {
              if (!OpenLog(arg,false,false))
              {
                  DMSG(0,"Rapr::OnCommand() Error: rapr log file open error: %s\n", strerror(errno));
                  return false;
              }
              log_file_lock = override;
          }
          break;
      }
    case MGENLOG:
      {
          char tmpOutput[512];
          if (1 != sscanf(arg,"%s",tmpOutput))
          {
              DMSG(0,"Rapr::OnCommand() Error: Log file name required\n");
              return false;
          }
          char buffer[8192];
          sprintf(buffer," LOG %s",tmpOutput);
          SendMgenCommand("event",buffer);
          break;
      }
    case OVERWRITE_MGENLOG:
      {
          char tmpOutput[512];
          if (1 != sscanf(arg,"%s",tmpOutput))
          {
              DMSG(0,"Rapr::OnCommand() Error: Log file name required\n");
              return false;
          }
          char buffer[8192];
          sprintf(buffer," OUTPUT %s",tmpOutput);
          SendMgenCommand("event",buffer);
          break;
      }
    case SAVE:
      {
          char tmpOutput[512];
          if (1 != sscanf(arg,"%s",tmpOutput))
          {
              DMSG(0,"Rapr::OnCommand() Error: Log file name required\n");
              return false;
          }
          char buffer[8192];
          sprintf(buffer," SAVE %s",tmpOutput);
          SendMgenCommand("event",buffer);
          break;
      }
    case DEBUG:
      {
          SetDebugLevel(atoi(arg));
          
          break;
      }
    case VERBOSE:
      {
          SetVerbosity(true);
          break;
      }
    case ENABLE_LOAD_EMULATOR:
      {
          SetLoadEmulator(true);
          break;
      }
    case INITIAL_SEED:
      {
          if (GetNumberGenerator().GetInitialSeed())
          {
              DMSG(0,"Rapr::OnCommand() Error: initial seed has already been setto: %d. Value not changed.\n",GetNumberGenerator().GetInitialSeed());
              //return false;
          } else
	    GetNumberGenerator().SetInitialSeed(atoi(arg));
          break;
      }
    case OFFSET:
      {
          if (override || !offset_lock)
          {
              double timeOffset;
              if (1 == sscanf(arg,"%lf",&timeOffset))
              {
                  journal.SetRestart();
                  offset = timeOffset;
                  offset_lock = override;
              }
              else
              {
                  DMSG(0,"Rapr::OnCommand() Error: invalid OFFSET\n");
                  return false;
              }
          }
          break;
      }  
    case TXLOG:
      {
          char buffer[8192];
          sprintf(buffer," TXLOG");
          SendMgenCommand("event",buffer);
          break;
      }
    case LOCALTIME:
      {
          char buffer [8192];
          sprintf(buffer," LOCALTIME");
	  local_time = true;
          SendMgenCommand("LOCALTIME",buffer);
          break;
      }
    case NOLOG:
      {
          char buffer[8192];
          sprintf(buffer," NOLOG");
          SendMgenCommand("event",buffer);
          break;
      }
    case BINARY:
      {
          char buffer[8192];
          sprintf(buffer," BINARY");
          SendMgenCommand("event",buffer);
          break;
      }
    case FLUSH:
      {
          // Set RAPR log file to flush
          log_flush = true;
          
          // Set MGEN log file to flush
          char buffer[8192];
          sprintf(buffer," FLUSH");
          SendMgenCommand("event",buffer);
          break;
      }
    case LABEL:
      {
          DMSG(0,"Rapr::OnCommand() Error: Label command not implemented.\n");
          return false;
          
      }
    case RXBUFFER:
      {
          unsigned int sizeTemp;
          if (1 != sscanf(arg,"%u",&sizeTemp))
          {
              DMSG(0,"Rapr::OnCommand() Error: invalid rx buffer size\n");
              return false;
          }
          char buffer [8192];
          sprintf(buffer," RXBUFFER %d",sizeTemp);
          SendMgenCommand("event",buffer);
          break;
      }
    case TXBUFFER:
      {
          unsigned int sizeTemp;
          if (1 != sscanf(arg,"%u",&sizeTemp))
          {
              DMSG(0,"Rapr::OnCommand() Error: invalid tx buffer size\n");
              return false;
          }
          char buffer [8192];
          sprintf(buffer," TXBUFFER %d",sizeTemp);
          SendMgenCommand("event",buffer);
          break;
      }
    case TOS:
      {
          int tosTemp;
          int result = sscanf(arg,"%i",&tosTemp);
          if ((1 != result) || (tosTemp < 0) || (tosTemp > 255))
          {
              DMSG(0,"Rapr::OnCommand() Error: invalid tos value\n");
              return false;
          }
          char buffer [8192];
          sprintf(buffer," TOS %i",tosTemp);
          SendMgenCommand("event",buffer);
          break;
      }
    case TTL:
      {
          int ttlTemp;
          int result = sscanf(arg,"%i",&ttlTemp);
          if ((1 != result) || (ttlTemp < 0) || (ttlTemp > 255))
          {
              DMSG(0,"Rapr::OnCommand() Error: invalid ttl value\n");
              return false;
          }
          char buffer[8192];
          sprintf(buffer," TTL %i",ttlTemp);
          SendMgenCommand("event",buffer);
          break;
      }
    case INTERFACE:
      {
          char tmpOutput[512];
          if (1 != sscanf(arg,"%s",tmpOutput))
          {
              DMSG(0,"Rapr::OnCommand() Error: multicast interface name required\n");
              return false;
          }
          
          char buffer[8192];
          sprintf(buffer," INTERFACE %s",tmpOutput);
          SendMgenCommand("event",buffer);
          break;
      }
    case CHECKSUM:
      {
          char buffer[8192];
          sprintf(buffer," CHECKSUM");
          SendMgenCommand("event",buffer);
          break;
      }
    case TXCHECKSUM:
      {
          char buffer[8192];
          sprintf(buffer," TXCHECKSUM");
          SendMgenCommand("event",buffer);
          break;
      }
    case RXCHECKSUM:
      {
          char buffer[8192];
          sprintf(buffer," RXCHECKSUM");
          SendMgenCommand("event",buffer);
          break;
      }
    case HOSTID:
      {
          hostIDSet = true;
          short hostID;
          int result = sscanf(arg,"%hi",&hostID);
          if (1 != result) {
              DMSG(0,"Rapr::OnCommand() Error: invalid host ID %hi\n",hostID);
              return false;
          }
          if (0 > hostID && hostID < 255)
          {
              DMSG(0,"Rapr::OnCommand() Error: Invalid HOSTID %hi \n",hostID);
              return false;
          }

          GetNumberGenerator().SetHostID(hostID);
	  // Initially seed rapr with hostID
	  if (!GetNumberGenerator().GetInitialSeed())
	    {
	      DMSG(0,"Rapr::Start() Warning: Initial seed has not been set, using HostID %hi as the initial seed.\n",GetNumberGenerator().GetHostID());
	      GetNumberGenerator().SetInitialSeed(GetNumberGenerator().GetHostID());
	    }
          DMSG(1,"Rapr::OnCommand() HOSTID set %hi \n",GetNumberGenerator().GetHostID());
          break;
      }
    case JINTERVAL:
      {
          DMSG(1,"Rapr::OnCommand() - Journal Interval : %s\n",arg);
          journal.SetTimer(atoi(arg));
          break;
      }
    case JOURNAL:
      {
          DMSG(1,"Rapr::OnCommand() - Journal : %s\n",arg);
          journal.SetProtoTimer(timer_mgr);
          journal.SetRapr(*this);
          journal.SetFile(arg);
          break;
      }
    case RAPRPIPE:
    case INSTANCE:
    case LOAD_DICTIONARY:
    case MGENEVENT:
      {
          DMSG(0,"Rapr::OnCommand() Error: Keyword is a raprApp keyword.  Program error?\n");
          return false;
      }
    case INVALID_COMMAND:
      {
          DMSG(0,"Rapr::OnCommand() Error: invalid command\n");
          return false;
      }
    } // end switch(cmd)
    return true;
}  // end Rapr::OnCommand

bool Rapr::ParseBehaviorEvent(const char* lineBuffer, unsigned int lineCount,RaprDictionaryTransfer* trans,BehaviorEvent::EventSource eventSource,RaprPRNG* prng,const char* origLineBuffer)
{
    
    // EVENT line can begin with the <eventTime> _or_ the <eventType>
    // for implicit, immediate events.
    
    double eventStartTime = 0;
    double eventEndTime   = 0;
    bool   theDuration    = false;
    int    inRaprFlowId   = 0;
    
    // reset ptr so we can get time if needed
    const char * ptr = lineBuffer;
    if (!ptr) return true;
    // Strip leading white space
    while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
    // Check for comment line (leading '#')
    if ('#' == *ptr) return true;
    char fieldBuffer[SCRIPT_LINE_MAX+1];

    if (1 == sscanf(ptr,"%lf", &eventStartTime))
    {
        // Read event start time 
        if (1 != sscanf(ptr, "%s", fieldBuffer))
        {
            DMSG(0,"Rapr::ParseBehaviorEvent() Error: missing start time: %lu\n", lineCount);
            return false;
        }
        // Set ptr to next field in line, stripping white space
        ptr += strlen(fieldBuffer);
    }
    else
    {
        // It's an immediate event
        eventStartTime = -1.0;
    }          
    while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
    
    // Read behavior end time        
    if (1 == sscanf(ptr, "%s", fieldBuffer))
    {
        if (!strncmp("STOP",fieldBuffer,strlen(fieldBuffer)) ||
            !strncmp("DURATION",fieldBuffer,strlen(fieldBuffer))) {
            
            if (!strncmp("DURATION",fieldBuffer,strlen(fieldBuffer)))
              theDuration = true;
            
            ptr += strlen(fieldBuffer);
            while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
            
            if (1 == sscanf(ptr,"%lf",&eventEndTime))
            {
                if (1 != sscanf(ptr,"%s",fieldBuffer))
                {
                    DMSG(0,"Rapr::ParseBehaviorEvent() Error: missing end time\n");
                    return false;
                }
                ptr += strlen(fieldBuffer);
                while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
                
                if (!theDuration && (eventEndTime < eventStartTime))
                {
                    DMSG(0,"Rapr::ParseBehaviorEvent() Error: end time must be greater than start time.\n");
                    return false;
                }
            }
            else
            {
                // If it's a stop flowid event, the next field should
                // be a RAPRFLOWID, if not fail.
                if (1 == sscanf(ptr,"%s",fieldBuffer))
                {
                    if (strncmp("RAPRFLOWID",fieldBuffer,strlen(fieldBuffer)))
                    {
                        DMSG(0,"Rapr::ParseBehaviorEvent Error: Stop/duration time required.\n");
                        return false;
                    }
                }
            }
        }
        else {
            eventEndTime = 0.0; // no behavior end time
        }
    }
    
    
    // Read in <eventType>
    BehaviorEvent::Type eventType;
    if (1 == sscanf(ptr,"%s",fieldBuffer)) 
    {
        eventType = BehaviorEvent::GetTypeFromString(fieldBuffer);
        // Get the flow id if available
        if (!strncmp("RAPRFLOWID",fieldBuffer,strlen(fieldBuffer)))
        {
            const char* tmpPtr = ptr;
            ptr += strlen(fieldBuffer);
            while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
            
            if (1 == sscanf(ptr,"%d",&inRaprFlowId))
            {
                if (1 != sscanf(ptr,"%s",fieldBuffer))
                {
                    DMSG(0,"Rapr::ParseBehaviorEvent() Error: missing raprFlow id\n");
                    return false;
                }
                ptr += strlen(fieldBuffer);
                while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
                
                // Now see if we're a behavior object
                if (1 == sscanf(ptr,"%s",fieldBuffer))
                {
                    eventType = BehaviorEvent::GetTypeFromString(fieldBuffer);
                }
                else 
                {
                    // Reset pointer if we're a stop raprflowid type
                    ptr = tmpPtr;
                }
            }
        }
    }
    
    // What kind of behaviorEvent is it?
    if (eventType != BehaviorEvent::INVALID_TYPE)
    {
        
        // It's a RAPR Behavior Event
        // Get the attributes      
        
        BehaviorEvent* theEvent;
        switch (eventType) 
        {
        case BehaviorEvent::DECLARATIVE:
          {
              theEvent = new Declarative(timer_mgr,number_generator.GetUBI(),*this);
              if (!theEvent)
              {
                  DMSG(0,"Rapr::ParseBehaviorEvent() Error: rapr event allocation error: %s\n",strerror(errno));
                  return false;
              }
              
              break;
          } // end case DECLARATIVE
          
        case BehaviorEvent::INTERROGATIVE:
          {
              theEvent = new Interrogative(timer_mgr,number_generator.GetUBI(),*this);
              if (!theEvent)
              {
                  DMSG(0,"Rapr::ParseBehaviorEvent() Error: rapr event allocation error: %s\n",strerror(errno));
                  return false;
              }
              
              break;
          } // end case INTERROGATIVE

        case BehaviorEvent::PINGPONG:
          {
              theEvent = new PingPong(timer_mgr,number_generator.GetUBI(),*this);
              if (!theEvent)
              {
                  DMSG(0,"Rapr::ParseBehaviorEvent() Error: rapr evenet allocation error: %s\n",strerror(errno));
                  return false;
              }
              break;
          }
        case BehaviorEvent::STREAM:
          {
              theEvent = new Stream(timer_mgr,number_generator.GetUBI(),*this);
              
              if (!theEvent)
              {
                  DMSG(0,"Rapr::ParseBehaviorEvent() Error: rapr event allocation error: %s\n",strerror(errno));
                  return false;
              }
              // Voip conversations need to be initiated
              // by scripted behavior (for now).  Use the
              // initiating ubi as the stream ubi (for all
              // objects participating in the conversation).
              
              if (eventSource == BehaviorEvent::SCRIPT_EVENT ||
                  eventSource == BehaviorEvent::RTI_EVENT) 
              {
                  ((Stream*)theEvent)->SetStreamUbi(theEvent->GetUBI());
                  if (!eventEndTime) // default to stream stop
                    eventEndTime = STREAM_STOP_TIME;
                  
              }
              else 
                if (eventSource == BehaviorEvent::NET_EVENT) 
                {
                    // A null payload stream id indicates that
                    // triggering network event was not 
                    // generated by a stream object.  For now
                    // this is an error condition.
                    if (trans && (!trans->GetPayload()->GetStreamID()))
                    {
                        DMSG(0,"Rapr::ParseBehaviorEvent() Error: Invalid parameters for stream object creation.  Was the triggering object a stream object?\n");
                        return false;
                    }
                    
                    ((Stream*)theEvent)->SetStreamUbi(trans->GetPayload()->GetStreamID());
                    ((Stream*)theEvent)->SetBurstDuration(trans->GetPayload()->GetStreamDuration());
                    // Start next burst after this one times out ljt
                    eventStartTime = trans->GetPayload()->GetStreamDuration();
                    if (!eventEndTime) // default to stream stop
                      eventEndTime = STREAM_STOP_TIME;
                    ((Stream*)theEvent)->SetBurstCount(trans->GetPayload()->GetBurstCount());
                    ((Stream*)theEvent)->SetBurstPriority(trans->GetPayload()->GetBurstPriority());
                    ((Stream*)theEvent)->SetBurstPayloadID(trans->GetPayload()->GetBurstPayloadID());
                    ((Stream*)theEvent)->SetStreamSeqNum(trans->GetPayload()->GetStreamSeq());
                }
              
                else
                {
                    DMSG(0,"Rapr::ParseBehaviorEvent() Error: invalid event source!\n");
                    return false;
                    
                }	    
              
              break;
          } // end case STREAM
          
          
        case BehaviorEvent::PERIODIC:
          {
              theEvent = new Periodic(timer_mgr,number_generator.GetUBI(),*this);

              if (!theEvent)
              {
                  DMSG(0,"Rapr::ParseBehaviorEvent() Error: rapr event allocation error: %s\n",strerror(errno));
                  return false;
              }
              // we retranslate the command when the event starts
              // up again so we need to save the original command
              // and the contents from the initiating message (so we
              // can translate the PACKET namespace)
              ((Periodic*)theEvent)->SetOriginalCommand(origLineBuffer);
              if (trans)
                ((Periodic*)theEvent)->SetTrans(trans);
              else
              {
                  RaprDictionaryTransfer* trans = new RaprDictionaryTransfer();
                  RaprPayload* raprPayload = new RaprPayload();
                  trans->SetPayload(raprPayload);
                  RaprPRNG *prng = new RaprPRNG(number_generator.GetNextSeed());                  
                  trans->SetPRNG(prng);
                  ((Periodic*)theEvent)->SetTrans(trans);
              }

              break;
          } // end case PERIODIC
          
          
        case BehaviorEvent::PROCESSOR:
          
          theEvent = new Processor(timer_mgr, number_generator.GetUBI(), *this);	// jm: need flowid ?
          
          if (!theEvent)
          {
              DMSG(0,"Rapr::ParseBehaviorEvent() Error: Processor allocation error: %s\n",strerror(errno));
              return false;
          }
          
          break;
          
          
        case BehaviorEvent::RAPREVENT:
          {
              theEvent = new RaprEvent(timer_mgr,number_generator.GetUBI(),*this);
              if (!theEvent)
              {
                  DMSG(0,"Rapr::ParseBehaviorEvent() Error: rapr event allocation error: %s\n",strerror(errno));
                  return false;
              }
              
              break;
          } // end case RAPREVENT	  
        case BehaviorEvent::RECEPTIONEVENT:
          {
              theEvent = new ReceptionEvent(timer_mgr,number_generator.GetUBI(),*this);
              if (!theEvent)
              {
                  DMSG(0,"Rapr::ParseBehaviorEvent() Error: rapr event allocation error: %s\n",strerror(errno));
                  return false;
              }
              
              break;
          } // end case RECEPTIONEVENT
        default:
          {
              DMSG(0,"Rapr::ParseBehaviorEvent() Error: invalid behavior event type!\n");
              return false;
          }
        }
        
        // Set any previously loaded state
        theEvent->SetBehaviorStartTime(eventStartTime);
        theEvent->SetBehaviorEndTime(eventEndTime);
        theEvent->SetDuration(theDuration);
        theEvent->SetRaprFlowID(inRaprFlowId);
        theEvent->SetEventSource(eventSource);	  
        DMSG(3,"Rapr::ParseBehaviorEvent() Setting PRNG: seed> %d\n",prng->GetSeed());
        theEvent->SetPrng(prng);

        // Get foreign ubi from the triggering
        // incoming mgen message ubi. Only gets set by
        // interrogative messages.
        if ((eventSource == BehaviorEvent::NET_EVENT) 
            && trans
            && trans->GetPayload())
        {
            // ljt put this & stream payload stuff in common block
            
            // Only store it if it's not ours.
            if (GetNumberGenerator().GetHostIDFromUBI(trans->GetPayload()->GetUBI())
                != GetNumberGenerator().GetHostID())
              theEvent->SetForeignUBI(trans->GetPayload()->GetUBI());
        }
        // We have this separate drec parsing routine for 
        // historical reasons - could be rewritten...
        if (theEvent->GetType() == BehaviorEvent::RECEPTIONEVENT && 
            !((ReceptionEvent*)theEvent)->ParseDrecOptions(ptr))
        {
            DMSG(0,"Rapr::ParseBehaviorEvent() Error: ParseDrecOptions error\n");
            char msgBuffer[512];
	    sprintf(msgBuffer,"type>Error action>parse info>\"%s\"",ptr);
	    LogEvent("RAPR",msgBuffer);
            delete theEvent;
            return false;
        }
        if (theEvent->GetType() != BehaviorEvent::RECEPTIONEVENT && 
            !theEvent->ParseOptions(ptr))
        {
            DMSG(0,"Rapr::ParseBehaviorEvent() Error: ParseOptions error\n");
            delete theEvent;
            return false;
        }
        InsertBehaviorEvent(theEvent);
    }
    
    else
    {      
        DMSG(0,"Rapr::ParseBehaviorEvent() Error: Invalid command: %s at line: %lu.\n",lineBuffer,lineCount);
        return false;
    }
    
    return true;
} // end Rapr::ParseBehaviorEvent

bool Rapr::ParseEvent(const char* lineBuffer, unsigned int lineCount,RaprDictionaryTransfer* trans,BehaviorEvent::EventSource eventSource,RaprPRNG* prng,const char* origLineBuffer)
{
    const char * ptr = lineBuffer;
    if (!ptr) return true;
    
    lineBuffer = ptr;
    
    // Strip leading white space
    while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
    // Check for comment line (leading '#')
    if ('#' == *ptr) return true;
    char fieldBuffer[SCRIPT_LINE_MAX+1];
    // Script lines are in form {<globalCmd>|<eventTime>} ...
    if (1 != sscanf(ptr, "%s", fieldBuffer))
    {
        // Blank line?
        return true;   
    }
    // Set ptr to next field in line, stripping white space
    ptr += strlen(fieldBuffer);
    while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
    
    Command cmd = GetCommandFromString(fieldBuffer);
    if (EVENT == cmd)
    {
        // read in <eventTime> or <eventType>
        if (1 != sscanf(ptr, "%s", fieldBuffer))
        {
            DMSG(0, "Rapr::ParseEvent() Error: empty EVENT command at line: %lu\n", lineCount);
            return false;    
        }
        // Set ptr to next field in line, stripping white space
        ptr += strlen(fieldBuffer);
        while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;          
    }

    // See if it's an MGEN stop command, otherwise it's a 
    // (possibly) a stop command for an event
    if (cmd == STOP)
    {
        if (!OnCommand(cmd,ptr,eventSource))
          cmd = EVENT;
    }
    
    // If it's not a "global command", assume it's an event.
    if (INVALID_COMMAND == cmd)
    {
        cmd = EVENT;
    }
    
    switch (cmd)
    {
    case RAPRPIPE:
      {
          // Were we already started?
          if (rapr_control_pipe.IsOpen())
          {
              DMSG(0,"Rapr::ParseEvent() Error: Rapr pipe already opened as: %s.\n",rapr_control_pipe.GetName());
              return false;
          }
          
          
          char rapr_name[SCRIPT_LINE_MAX];
          if (1 != sscanf(ptr, "%s" , rapr_name))
          {
              DMSG(0,"Rapr::ParseEvent() Error: Rapr instance name required.\n");
              return false;
          }
          
          // Clean up any left over pipes...
          if (rapr_control_pipe.IsOpen())
            rapr_control_pipe.Close();
          else 
          {
              if ('\0' != rapr_name)
              {
                  char pipeName[PATH_MAX];
                  strcpy(pipeName,"/tmp/");
                  strncat(pipeName,rapr_name,PATH_MAX - strlen(pipeName));
                  unlink(pipeName);
              }
          }
          
          if (rapr_control_pipe.Connect(rapr_name)) 
          {
              rapr_control_remote = true;
          }
          else
            if (!rapr_control_pipe.Listen(rapr_name))
            {
                DMSG(0,"Rapr::ParseEvent(RAPRPIPE) Error: error opening rapr pipe.\n");
                return false;
            }
          break;
      }
      
    case LOAD_DICTIONARY:
      {
          // Get pipe name
          char dictionaryName[SCRIPT_LINE_MAX];
          if (1 != sscanf(ptr, "%s" , dictionaryName))
          {
              DMSG(0,"Rapr::ParseEvent() Error: Dictionary name required.\n");
              return false;
          }
          
          char msgBuffer [512];
          sprintf(msgBuffer,"type>RaprEvent action>loading_dictionary name>%s",dictionaryName);
          LogEvent("RAPR",msgBuffer);
          
          if (!LogicTable().LoadDictionary(dictionaryName))
          {
              sprintf(msgBuffer,"type>Error action>loading_dictionary name>%s",dictionaryName);
              LogEvent("RAPR",msgBuffer);
          }
          // Load HOSTID into the dictionary
          char *name = new char[7];
          sprintf(name,"%s","HOSTID");
          char *val = new char[20];
          sprintf(val,"%d",GetNumberGenerator().GetHostID());
          DMSG(2,"Rapr::ParseEvent() Adding/Replacing dictionary value %s with value %s\n",name,val);
          LogicTable().ResetDictionaryValue(name,val);
          delete [] name;
          delete [] val;
          
          break;
          
      }
    case EVENT:
      {
          return ParseBehaviorEvent(lineBuffer,lineCount,trans,eventSource,prng,origLineBuffer);
          
          break;
      } // end case EVENT
    default:
      // Is it a global command
      if (INVALID_COMMAND != cmd)
      {
          if (!OnCommand(cmd,ptr,eventSource))
          {
              DMSG(0,"Rapr::ParseEvent() Error: Bad global command: %s at line: %lu\n",lineBuffer,lineCount);
              return false;
          }
      } 
      else
      {
          DMSG(0,"Rapr::ParseEvent() Error: invalid command: %s at line: %lu\n",lineBuffer,lineCount);
          return false;
      }
      break;
    }
    
    return true;
} // end Rapr::ParseEvent()

// Parse a Rapr script
bool Rapr::ParseScript(const char* path)
{
    // Open script file
    FILE* scriptFile = fopen(path,"r");
    if (!scriptFile)
    {
        DMSG(0,"Rapr::ParseScript() fopen() Error: %s\n",strerror(errno));
        return false;
    }
    
    // Read script file line by line using FastReader
    FastReader reader;
    unsigned int lineCount = 0;
    unsigned int lines = 0;
    while (1)
    {
        lineCount += lines;  // for grouped (continued) lines
        char lineBuffer[SCRIPT_LINE_MAX+1];
        unsigned int len = SCRIPT_LINE_MAX;
        switch (reader.ReadlineContinue(scriptFile, lineBuffer, &len, &lines))
        {
        case FastReader::OK:
          lineCount++;
          lines--;
          break;
        case FastReader::DONE:
          fclose(scriptFile);
          return true; // done with script file
        case FastReader::ERROR_:
          DMSG(0, "Rapr::ParseScript() Error: script file read error\n");
          fclose(scriptFile);
          return false;
        }
        
        // check for comment line (leading '#')
        if ('#' == *lineBuffer) continue;
        
        RaprStringVector *vec;
        RaprPRNG *prng = new RaprPRNG(number_generator.GetNextSeed());
        DMSG(3,"Rapr::ParseScript() Using next seed>%d for>%s\n",prng->GetSeed(),lineBuffer);
        
        vec = LogicTable().TranslateString(lineBuffer,prng);
        if (vec != NULL) {
            for (unsigned int i=0;i<vec->size();i++) {
                // Since we want to retranslate periodic events
                // each time we start up, make sure we only have
                // one periodic event definition to work with.
                // (Temporary solution)

                // Comparing to PERIODIC INT to deconflict with
                // PERIODIC [ pattern ljt
                if ((vec->size() > 1) && (strstr((*vec)[i],"PERIODIC INT")))
                {
                    DMSG(0,"Rapr::ParseScript() Error: PERIODIC behavior events cannot be translated into multiple events.\n");
                    DMSG(0, "Rapr::ParseScript() Error: invalid rapr script line: %lu\n", lineCount);
                    fclose(scriptFile);
                    return false;
                }

                // Now get a new PRNG for the event
                RaprPRNG *newPrng = new RaprPRNG(number_generator.GetNextSeed());
                if (!ParseEvent((*vec)[i], lineCount,NULL,BehaviorEvent::SCRIPT_EVENT,newPrng,lineBuffer))
                {
                    DMSG(0, "Rapr::ParseScript() Error: invalid rapr script line: %lu\n", 
                         lineCount);
                    fclose(scriptFile);
                    // reset running seed upon failure
                    number_generator.DecrementSeed(); 
                    return false;   
                }
            }
            delete vec;
        }
        delete prng;
    }  // end while (1)
    return true;
    
} // end Rapr::ParseScript

// Query behavior_event_list for an idea of the current
// (or greatest) estimate of current relative script 
// time offset.
double Rapr::GetCurrentOffset() const
{
    if (!started)
      return -1.0;

    if (next_behavior_event)
      return (next_behavior_event->GetBehaviorStartTime() - behavior_event_timer.GetTimeRemaining());
    
    const BehaviorEvent* lastEvent = (const BehaviorEvent*)behavior_event_list.Tail();
    double behaviorEventOffset = lastEvent ? lastEvent->GetBehaviorStartTime() : -1.0;
    return behaviorEventOffset;
} // end Rapr::GetCurrentOffset()

void Rapr::InsertBehaviorEvent(BehaviorEvent* theEvent)
{
    double eventStartTime;
    
    // If we were triggered by a logic table event,
    // Set any start time relative to current offset
    if (theEvent->GetEventSource() == BehaviorEvent::NET_EVENT ||
        theEvent->GetEventSource() == BehaviorEvent::RTI_EVENT)
    {
        // If start time is 0.0 Override offset and start
        // immediately
        if (!theEvent->GetBehaviorStartTime())
        {
            behavior_event_list.Precede(next_behavior_event,theEvent);
            StartBehaviorEvent(*theEvent);
            return;
        }

        theEvent->SetBehaviorStartTime(theEvent->GetBehaviorStartTime() + GetCurrentOffset());
    }
    
    eventStartTime = theEvent->GetBehaviorStartTime();
    
    DMSG(3,"\n\n\n New Behavior start time: %.2f \n",eventStartTime);
    
    if (started)
    {
        double currentTime = GetCurrentOffset();
        if (currentTime < 0.0) currentTime = 0.0;
        if (eventStartTime < currentTime)
        {
            theEvent->SetBehaviorStartTime(currentTime);
            behavior_event_list.Precede(next_behavior_event,theEvent);
            StartBehaviorEvent(*theEvent);
        }
        else
        {
            theEvent->SetBehaviorStartTime(eventStartTime);
            behavior_event_list.Insert(theEvent);
            // Reschedule next behavior event timeout if needed
            if (behavior_event_timer.IsActive())
            {
                double nextTime = next_behavior_event->GetBehaviorStartTime();
                if (eventStartTime < nextTime)
                {
                    next_behavior_event = theEvent;
                    behavior_event_timer.SetInterval(eventStartTime - currentTime);
                    behavior_event_timer.Reschedule();
                }
            }
            else
            {
                next_behavior_event = theEvent;
                behavior_event_timer.SetInterval(eventStartTime - currentTime);
                timer_mgr.ActivateTimer(behavior_event_timer);
            }
        }
    }
    else 
    {
        eventStartTime = eventStartTime > 0.0 ? eventStartTime : 0.0;
        theEvent->SetBehaviorStartTime(eventStartTime);
        behavior_event_list.Insert(theEvent);
    }
    
} // end Rapr::InsertBehaviorEvent()


////////////////////////////////////////////////////////////////
// Rapr::FastReader implementation

Rapr::FastReader::FastReader()
  : savecount(0)
{
    
}

Rapr::FastReader::Result Rapr::FastReader::Read(FILE*           filePtr, 
                                                char*           buffer, 
                                                unsigned int*   len)
{
    unsigned int want = *len;   
    if (savecount)
    {
        unsigned int ncopy = want < savecount ? want : savecount;
        memcpy(buffer, saveptr, ncopy);
        savecount -= ncopy;
        saveptr += ncopy;
        buffer += ncopy;
        want -= ncopy;
    }
    while (want)
    {
        unsigned int result = fread(savebuf, sizeof(char), BUFSIZE, filePtr);
        if (result)
        {
            unsigned int ncopy= want < result ? want : result;
            memcpy(buffer, savebuf, ncopy);
            savecount = result - ncopy;
            saveptr = savebuf + ncopy;
            buffer += ncopy;
            want -= ncopy;
        }
        else  // end-of-file
        {
            if (ferror(filePtr))
            {
                if (EINTR == errno) continue;   
            }
            *len -= want;
            if (*len)
              return OK;  // we read at least something
            else
              return DONE; // we read nothing
        }
    }  // end while(want)
    return OK;
}  // end Rapr::FastReader::Read()

// An OK text readline() routine (reads what will fit into buffer incl. NULL termination)
// if *len is unchanged on return, it means the line is bigger than the buffer and 
// requires multiple reads


Rapr::FastReader::Result Rapr::FastReader::Readline(FILE*         filePtr, 
                                                    char*         buffer, 
                                                    unsigned int* len)
{   
    unsigned int count = 0;
    unsigned int length = *len;
    char* ptr = buffer;
    while (count < length)
    {
        unsigned int one = 1;
        switch (Read(filePtr, ptr, &one))
        {
        case OK:
          if (('\n' == *ptr) || ('\r' == *ptr))
          {
              *ptr = '\0';
              *len = count;
              return OK;
          }
          count++;
          ptr++;
          break;
          
        case ERROR_:
          return ERROR_;
          
        case DONE:
          return DONE;
        }
    }
    // We've filled up the buffer provided with no end-of-line 
    return ERROR_;
}  // end Rapr::FastReader::Readline()

// This reads a line with possible ending '\' continuation character.
// Such "continued" lines are returned as one line with this function
//  and the "lineCount" argument indicates the actual number of lines
//  which comprise the long "continued" line.
//
// (Note: Lines ending with an even number '\\' are considered ending 
//        with one less '\' instead of continuing.  So, to actually 
//        end a line with an even number of '\\', continue it with 
//        an extra '\' and follow it with a blank line.)

Rapr::FastReader::Result Rapr::FastReader::ReadlineContinue(FILE*         filePtr, 
                                                            char*         buffer, 
                                                            unsigned int* len,
                                                            unsigned int* lineCount)
{   
    unsigned int lines = 0;
    unsigned int count = 0;
    unsigned int length = *len;
    char* ptr = buffer;
    while (count < length)
    {
        unsigned int space = length - count;
        switch (Readline(filePtr, ptr, &space))
        {
        case OK:
          {
              lines++;
              // 1) Doesn't the line continue?
              char* cptr = space ? ptr + space - 1 : ptr;
              // a) skip trailing white space
              while (((' ' == *cptr) ||  ('\t' == *cptr)) && (cptr > ptr)) 
              {
                  space--;
                  cptr--;
              }
              
              // If line "continues" to a blank line, skip it
              if ((cptr == ptr) && ((*cptr == '\0') || isspace(*cptr))) 
                continue;
              
              if ('\\' == *cptr)
              {
                  // Make sure line ends with odd number of '\' to continue
                  bool lineContinues = false;
                  while ((cptr >= ptr) && ('\\' == *cptr))
                  {
                      cptr--;
                      lineContinues = lineContinues ? false : true;    
                  }  
                  // lose trailing '\' (continuation or extra '\' char)
                  *(ptr+space-1) = '\0';
                  space -= 1;
                  if (lineContinues)
                  {
                      // get next line to continue
                      count += space;
                      ptr += space;
                      continue;
                  }
              }
              *len = count + space;
              if (lineCount) *lineCount = lines;
              return OK;  
              break;
          }
          
        case ERROR_:
          return ERROR_;
          
        case DONE:
          return DONE;
        }
    }
    // We've filled up the buffer provided with no end-of-line 
    return ERROR_;
}  // end Rapr::FastReader::Readline()

/* End Rapr::FastReader Implementation */

bool Rapr::OpenLog(const char* path, bool append, bool binary)
{
    CloseLog();
    if (append)
    {
        struct stat buf;
        if (stat(path, &buf))   // zero return value == success
          log_empty = true;  // error -- assume file is empty
        else if (buf.st_size == 0)
          log_empty = true;
        else
          log_empty = false;
    }
    else
    {
        log_empty = true;   
    }
    const char* mode;
    if (binary)
    {
        mode = append ? "ab" : "wb+";
        log_binary = true;
    }
    else
    {
        mode = append ? "a" : "w+";
        log_binary = false;
    }
    if (!(log_file = fopen(path, mode)))
    {
        DMSG(0, "Rapr::OpenLog() fopen() Error: %s\n", strerror(errno));   
        return false;    
    }   
    log_open = true;
    return true;
}  // end Rapr::OpenLog()

void Rapr::SetLogFile(FILE* filePtr)
{
    CloseLog();
    log_file = filePtr;

}  // end Rapr::SetLogFile()

void Rapr::CloseLog()
{
    if (log_file)
    {
        if ((stdout != log_file) && (stderr != log_file))
            fclose(log_file);
        log_file = NULL;
    }
}  // end Rapr::CloseLog()

void Rapr::LogEvent(const char* cmd,const char* val)
{
  // Don't log pre-processed events
  // ljt return me if (offset_pending) return;
  if (NULL == log_file) return;

  if (log_file)
    {
      struct timeval currentTime;
      ProtoSystemTime(currentTime);
      struct tm* timePtr;
      if (local_time)
	timePtr = localtime((time_t*)&currentTime.tv_sec);
      else
	timePtr = gmtime((time_t*)&currentTime.tv_sec);
      
      fprintf(log_file,"%02d:%02d:%02d.%06lu app>%s %s\n",
	      timePtr->tm_hour,timePtr->tm_min,
	      timePtr->tm_sec,(unsigned long) currentTime.tv_usec,
	      cmd,val);
      if (log_flush) fflush(log_file);
    }
}

bool Rapr::StartEvents(double offsetTime)
{

  // Defer actual start until "offsetTime", but process events
  // so flow state is appropriate for given offset.
    BehaviorEvent* nextEvent = (BehaviorEvent*)behavior_event_list.Head();
    if (!nextEvent)
    {
        DMSG(0,"Rapr::StartEvents BehaviorEventList::Start() empty event list!\n");
        return false;
        
    }
    
    while (nextEvent)
    {
        if (nextEvent->GetBehaviorStartTime() <= offsetTime)
        {
            // If stop time is after our offset, start er up
            // for any remaining time (except stream objects)
            if ((nextEvent->GetType() != BehaviorEvent::STREAM)
                &&
                ((nextEvent->GetBehaviorEndTime() > offsetTime)
                 
                 // Or, if it's a duration object, see if the
                 // duration wuld last past the offset time
                 || (nextEvent->GetDuration() 
                     
                     && ((nextEvent->GetBehaviorStartTime() 
                          + nextEvent->GetBehaviorEndTime())
                         > offsetTime)))
                // Finally, start all raprevents or reception events
                // as we see them
                || ((nextEvent->GetType() ==  BehaviorEvent::RAPREVENT 
                     || nextEvent->GetType() == BehaviorEvent::RECEPTIONEVENT)
                    && ((RaprEvent*)nextEvent)->GetOverrideStartTime()))
            {
                
                //double currentTime = offset > 0.0 ? offset: 0.0;
                double currentTime = 0.0;
                behavior_event_timer.SetInterval(currentTime);
                timer_mgr.ActivateTimer(behavior_event_timer);
                nextEvent->SetBehaviorStartTime(currentTime);
                break;
                
            }
            else
            {
                // Do nothing with completed events
                behavior_event_list.Remove(nextEvent);
                delete nextEvent;
            }
        }
        else
        {
            double currentTime = offset > 0.0 ? offset: 0.0;
            behavior_event_timer.SetInterval(nextEvent->GetBehaviorStartTime() - currentTime);
            timer_mgr.ActivateTimer(behavior_event_timer);
            break;
            
        }
        nextEvent = (BehaviorEvent*)nextEvent->Next();
    }
    offset_pending = false;
    if (nextEvent) 
    {
        next_behavior_event = nextEvent;
    }
    else 
    {
        DMSG(0,"RAPR:StartEvents empty event list!\n");
        return false;
    }
    started = true;
    return true;
    
}
