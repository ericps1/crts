#ifndef _RAPR
#define _RAPR
// Mgen
#include "mgen.h"
// Protolib
#include "protokit.h"
// Rapr
#include "behaviorEvent.h"
#include "raprPattern.h"
#include "raprNumberGenerator.h"
#include "raprGlobals.h"
#include "raprLogicTable.h"
#include "raprJournaller.h"

#include <stdio.h>

/*!
Rapr is the top level state and controller of a RAPR instance.
 
 */
class Rapr
{
 public:

  Rapr(ProtoTimerMgr& timerMgr,
       ProtoSocket::Notifier& socketNotifier,
       ProtoPipe& raprControlPipe,
       bool& raprControlRemote,
       Mgen& mgen);
  virtual ~Rapr();
  Mgen& GetMgen() {return mgen;}
  bool Start();
  void Stop();
  void StopEvents();

  bool DelayedStart() {return start_timer.IsActive();}
  void GetStartTime(char* buffer)
    {
      sprintf(buffer, "%02d:%02d:%02.0f%s", 
	      start_hour, start_min, start_sec,
	      (start_gmt ? "GMT" : ""));
    }        
  void SetSocketNotifier(ProtoSocket::Notifier* socketNotifier)
    {
      socket_notifier = socketNotifier;
    }
  // RAPR "global command" set
  enum Command
    {
      // Mgen Global commands
      INVALID_COMMAND,
      EVENT,
      START,     // specify absolute start time
      STOP,      // specify absolute stop time
      INPUT,     // input and parse an RAPR script
      OVERWRITE_RAPRLOG,// open rapr output (log) file 
      RAPRLOG,   // open rapr log file for appending
      OVERWRITE_MGENLOG,// open mgen output (log) file 
      MGENLOG,   // open mgen log file for appending
      SAVE,      // save info on exit.
      DEBUG,     // specify debug level
      VERBOSE,   // turn verbose logging on
      ENABLE_LOAD_EMULATOR, // enable load emulator
      INITIAL_SEED, // Initial seed to associate with generated behavior events
      OFFSET,    // time offset into script
      TXLOG,     // turn transmit logging on
      LOCALTIME, // turn on localtime logging in mgen rather than gmtime (the default)
      NOLOG,     // no output
      BINARY,    // mgen binary log 
      FLUSH,     //
      LABEL,
      RXBUFFER,
      TXBUFFER,
      TOS,
      TTL,
      INTERFACE,
      CHECKSUM,
      TXCHECKSUM,
      RXCHECKSUM,
      // Rapr Global Commands
      HOSTID,    // Unigue host identifier
      RAPRPIPE,  // rapr instance
      LOAD_DICTIONARY, // loads dictionary file
      INSTANCE,  // rapr instance
      // RaprApp Global Commands
      // ljt should these go elsewhere?
      OFFEVENT,  // mgen flow OFF events
      STOPEVENT, // mgen stop flow events
      MGENEVENT,  // mgen recv events
      JOURNAL,
      JINTERVAL
    };

  enum CmdType {CMD_INVALID, CMD_ARG, CMD_NOARG};
  static CmdType GetCmdType(const char* cmd);
  static Command GetCommandFromString(const char* string);
  bool OnCommand(Rapr::Command cmd, const char* arg,BehaviorEvent::EventSource eventSource,bool override = false);
  bool ParseBehaviorEvent(const char* lineBuffer, unsigned int lineCount,RaprDictionaryTransfer* trans,BehaviorEvent::EventSource eventSource,RaprPRNG* prng,const char* origLineBuffer = NULL);
  bool ParseEvent(const char* lineBuffer, unsigned int lineCount,RaprDictionaryTransfer* trans,BehaviorEvent::EventSource eventSource,RaprPRNG* prng,const char* origLineBuffer = NULL);
  bool ParseScript(const char* path);

  bool OpenLog(const char* path, bool append, bool binary);
  void CloseLog();
  void SetLogFile(FILE* filePtr);
  void SetVerbosity(bool logVerbose) {log_verbose = logVerbose;};
  bool GetLocalTime() {return local_time;}
  bool GetVerbosity() {return log_verbose;};
  void SetLoadEmulator(bool loadEmulator) {load_emulator = loadEmulator;};
  bool GetLoadEmulator() {return load_emulator;};
  bool LoadState();
  RaprNumberGenerator& GetNumberGenerator() {return number_generator;};
  RaprPRNG* GetRaprPRNG() {return rapr_prng;};
  
  double GetCurrentOffset() const;
  const double GetOffset() {return offset;};
 
  void InsertBehaviorEvent(BehaviorEvent* event);
  BehaviorEventList& GetBehaviorEventList() {return behavior_event_list;};
  bool StartEvents(double offsetTime);
  ProtoTimerMgr& GetTimerMgr() {return timer_mgr;};
  ProtoTimer& GetBehaviorEventTimer() {return behavior_event_timer;};

  // A little utility class for reading script files line by line
  class FastReader
    {
    public:
      enum Result {OK, ERROR_, DONE};  // trailing '_' for WIN32
      FastReader();
      FastReader::Result Read(FILE* filePtr, char* buffer, unsigned int* len);
      FastReader::Result Readline(FILE* filePtr, char* buffer, 
				  unsigned int* len);
      FastReader::Result ReadlineContinue(FILE* filePtr, char* buffer, 
					  unsigned int* len,
					  unsigned int* lineCount = NULL);
      
    private:
      enum {BUFSIZE = 1024};
      char         savebuf[BUFSIZE];
      char*        saveptr;
      unsigned int savecount;

    };  // end class FastReader        

  bool SendMgenCommand(const char* cmd, const char* val);
  void StopMgen();
  bool ProcessMgenEvent();
  void LogEvent(const char* cmd,const char* val);
  RaprLogicTable& LogicTable() {return logicTable;};
  void SetActiveStream(Stream* theActiveStream) {active_stream = theActiveStream;};
  Stream* GetActiveStream() {return active_stream;};
  
 private:
  // for mapping protocol types from script line fields
  static const StringMapper COMMAND_LIST[]; 

  bool OnStartTimeout(ProtoTimer& theTimer);
  bool OnBehaviorStartTimeout(ProtoTimer& theTimer);
  void StartBehaviorEvent(BehaviorEvent& event); // ljt was (const BehaviorEvent??)
  bool LoadFlowIDs();
  // common state
  ProtoTimerMgr&           timer_mgr;
  RaprNumberGenerator      number_generator;  
  RaprPRNG*                rapr_prng;
  bool                     started;
  bool                     stopped;

  ProtoTimer               start_timer;
  unsigned int             start_hour;  //absolute start time
  unsigned int             start_min;
  double                   start_sec;
  bool                     start_gmt;
  bool                     start_time_lock;
  Stream*                  active_stream;

  Mgen&                    mgen;        // Mgen engine
  RaprLogicTable           logicTable;  // logicTable

  FILE*        log_file;
  bool         log_binary; // not yet implemented but needed for common mgen functions
  bool         local_time;
  bool         log_flush;
  bool         log_verbose;
  bool         load_emulator;
  bool         log_file_lock;
  bool         log_open;
  bool         log_empty;

  double       offset;
  bool         offset_lock;
  bool         offset_pending;


  char*        save_path;
  bool         save_path_lock;

  ProtoPipe&   rapr_control_pipe; // so we can pass our name to mgen
  bool&        rapr_control_remote; 

  //journalling controller
  RaprJournaller journal;
  
  // Behavior Event state
  ProtoTimer               behavior_event_timer;
  BehaviorEventList        behavior_event_list;
  BehaviorEvent*           next_behavior_event;

  ProtoSocket::Notifier*   socket_notifier;
}; //end class Rapr

#endif // _RAPR
