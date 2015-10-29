#ifndef _BEHAVIOR_EVENT
#define _BEHAVIOR_EVENT

#include "raprDictionary.h"
#include "protokit.h"
#include "mgenPattern.h"
#include "raprGlobals.h"
#include "mgenEvent.h"
#include "raprNumberGenerator.h"
#include "mgenMsg.h"
using namespace std;
#include "string"

class Rapr;
/*! 
Base class for Rapr Behavior events
*/
class BehaviorEvent
{
  friend class BehaviorEventList;
  
public:
  BehaviorEvent(ProtoTimerMgr& timerMgr,
		unsigned long inUbi,
		Rapr& theRapr);
  BehaviorEvent();
  virtual ~BehaviorEvent();

  const BehaviorEvent* Next() const {return next;}
  const BehaviorEvent* Prev() const {return prev;}

  // ljt fix this - problem w/const & deleting in startbehaviortimeout
  BehaviorEvent* GetNext() const {return next;}


  static Protocol GetProtocolFromString(const char* string);
  static const char* GetStringFromProtocol(Protocol protocol);
	
  enum Option 
    {
      INVALID_OPTION = 0x0000,
      PROTOCOL =       0x0001,  // flow protocol was set
      DST =            0x0002,  // flow destination address was set
      SRC =            0x0004,  // flow source port was set
      PATTERN =        0x0008,  // flow pattern was set
      TOS =            0x0010,  // flow TOS was set
      RSVP =           0x0020,  // flow RSVP spec was set
      INTERFACE =      0x0040,  // flow multicast interface was set
      TTL =            0x0080,  // flow ttl was set
      SEQUENCE =       0x0100,  // flow sequence number was set        
      LABEL =          0x0200,  // flow label option for IPV6
      TXBUFFER  =      0x0400,  // Tx socket buffer size
      SEED =           0x0800,  // Seed was set
      RAPRFLOWID =     0x1000,  // RaprFlowID was set
      COUNT =          0x2000   // Count was set
    };

  enum LogicIdType
    {
      INVALID_LOGIC_ID = 0x0000,
      SUCCESS     = 0x0001,
      FAILURE     = 0x0002,
      PAYLOAD     = 0x0004,
      TIMEOUT     = 0x0008,
      ORIGIN	  = 0x0010,
      LOGICID     = 0x0020
    };

  enum StreamOption
    {
      INVALID_STREAM_OPTION = 0x0000,
      BURSTDURATION         = 0x0001,
      BURSTCOUNT            = 0x0002,
      BURSTPRIORITY         = 0x0004,
      BURSTDELAY            = 0x0008,
      BURSTRANGE            = 0x0010,
      RESPPROB              = 0x0020,
      BURSTPAYLOADID        = 0x0040,
      TIMEOUTINTERVAL       = 0x0080
    };

  // RAPR event source types
  enum EventSource
    {
      INVALID_EVENT,
      SCRIPT_EVENT,
      RTI_EVENT,
      NET_EVENT,
      MGEN_EVENT
    };
  // RAPR Fault types
  enum FaultType
    {
      INVALID_FAULT,
      STOP_RAPR,
      STOP_MGEN,
      STOP_EVENTS
    };
  // RAPR stream states
  enum StreamState
    {
      INVALID_STATE,
      IDLE_STREAM,
      ACTIVE_STREAM,
      INACTIVE_STREAM,
      STOPPED_STREAM
    };

  static const StringMapper EVENTSOURCE_LIST[];     // for mapping event sources

  // RAPR script event types
  enum Type 
    {
      INVALID_TYPE,
      RECEPTIONEVENT,
      RAPREVENT,
      DECLARATIVE,
      INTERROGATIVE,
      PERIODIC,
      STREAM,
	  PROCESSOR,
      PINGPONG,
      ALL
    };
  static const StringMapper TYPE_LIST[];      // for mapping event types
  static const StringMapper EVENTTYPE_LIST[]; // for mapping behavior event types
  
  static Type GetTypeFromString(const char* string);  
  Type GetType() {return type;};
  void SetBehaviorEventType(Type eventType) {type = eventType;};
  void SetEventSource(EventSource event_source) {eventSource = event_source;};
  static EventSource GetEventSourceFromString(const char* string);  
  EventSource GetEventSource() {return eventSource;};
  static const char* GetStringFromEventSource(EventSource eventSource);
  static const char* GetStringFromEventType(Type eventType);

  void SetUBI(unsigned long inUbi) {ubi = inUbi;};
  unsigned long GetUBI() const {return ubi;};
  void SetForeignUBI(unsigned long inForeignUBI) {foreignUbi = inForeignUBI;};
  unsigned long GetForeignUBI() const {return foreignUbi;};
  void SetFlowID(int inFlowID) {flowID = inFlowID;};
  int GetFlowID() const {return flowID;};
  void SetSrcAddr(ProtoAddress src_addr, int port);
  ProtoAddress GetSrcAddr() {return srcAddr;};
  int GetSrcPort();
  void SetSrcPort(int port) {srcPort = port;};
  void SetDstAddr(ProtoAddress dst_addr, int port);
  ProtoAddress GetDstAddr() {return dstAddr;};
  int GetDstPort();
  void SetDstPort(int port) {dstPort = port;};
  Protocol GetProtocol() {return protocol;};
  void SetProtocol(Protocol inProtocol) {protocol = inProtocol;};
  long GetSeed() {return prng->GetSeed();};
  void SetSeed(long inSeed) {seed = inSeed;
                             prng->SetSeed(inSeed);};
  void SetBehaviorStartTime(double theTime) {behaviorStartTime = theTime;};
  double GetBehaviorStartTime() const {return behaviorStartTime;};
  double GetBehaviorStartTime(double offset) 
    {
      return behaviorStartTime > offset ? behaviorStartTime : offset;
    };
  void SetBehaviorEndTime(double theTime) {behaviorEndTime = theTime;};
  double GetBehaviorEndTime() const {return behaviorEndTime;};
  void ActivateEndTimer(ProtoTimer& endBehaviorTimer);
  void SetDuration(bool theDuration) {duration = theDuration;};
  bool GetDuration() {return duration;};
  void SetSuccessLogicID(int logicID) {successLogicID = logicID;};
  int GetSuccessLogicID() {return successLogicID;};
  void SetFailureLogicID(int logicID) {failureLogicID = logicID;};
  int GetFailureLogicID() {return failureLogicID;};
  void SetTimeoutLogicID(int logicID) {timeoutLogicID = logicID;};
  int GetTimeoutLogicID() {return timeoutLogicID;};
  void SetPayloadLogicID(int logicID) {payloadLogicID = logicID;};
  int GetPayloadLogicID() {return payloadLogicID;};
  void SetOriginUBI(unsigned int inUBI) { origin = inUBI; };
  unsigned int GetOriginUBI() { return origin; };

  bool deleteMe();

  MgenPattern& GetMgenPattern() {return pattern;};
  int GetTOS() const {return tos;}
  void SetTOS(int inTos) {tos = inTos;};
  // ljt no ipv6 yet  UINT32 GetFlowLabel() const {return flow_label;}
  unsigned char GetTTL() const {return ttl;}
  unsigned int GetTxBuffer() const {return tx_buffer_size;}
  unsigned long GetSequence() const {return sequence;}
  const char* GetInterface()  const
    {return (('\0' != interface_name[0]) ? interface_name : NULL);}
  
  bool OptionIsSet(Option option) const
    {return (0 != (option & option_mask));}
  bool ParseOptions(const char* string);

  bool ParsePingPongOptions(const char* ptr,int& cnt);
  bool ParseInterrogativeOptions(const char* ptr,int& cnt);
  bool ParsePeriodicInterrogativeOptions(const char* ptr,int& cnt);
  bool ParseStreamOptions(const char* ptr,int& cnt);
  bool ParsePeriodicStreamOptions(const char* ptr,int& cnt);
  bool ParsePeriodicOptions(const char* ptr,int& cnt);
  bool ParseRaprEventOptions(const char* ptr,int& cnt);
  const char* ParsePattern(const char* ptr,char* fieldBuffer);
  
  bool LogicIdIsSet(LogicIdType logicId) const
    {return (0 != (logicId & logic_id_mask));}
  // subclasses must reimplement!
  virtual bool OnStartUp() = 0;
  virtual bool Stop() = 0;
  virtual bool ProcessEvent(const MgenMsg& theMsg) = 0;
  virtual bool OnBehaviorTimeout(ProtoTimer &theTimer) = 0;
  virtual void ClearState();

  int GetRaprFlowID() {return raprFlowID;};
  void SetRaprFlowID(int inRaprFlowID) {raprFlowID = inRaprFlowID;};
  const RaprPRNG* GetPrng() {return prng;};
  void SetPrng(RaprPRNG* inPrng) {prng = inPrng;};
  bool InUse() {return inUse;};
  const char* LookupDictionaryEntry(char* buf);
  bool Stopped() { return stopped;};
  void Stopped(bool theVal) {stopped = theVal;}

protected:

  static const StringMapper OPTION_LIST[];   // for mapping event options
  static Option GetOptionFromString(const char* string);
  static const char* GetStringFromOption(Option option);
  static const unsigned int ON_REQUIRED_OPTIONS;

  // for mapping protocol types from script line fields
  static const StringMapper PROTOCOL_LIST[]; 

  // for mapping logic id types from script line fields
  static const StringMapper LOGIC_ID_LIST[];
  static LogicIdType GetLogicIdFromString(const char* string);
  static const char* GetStringFromLogicId(LogicIdType logicId);

  // for mapping stream option types from script line fields
  static const StringMapper STREAM_OPTION_LIST[];
  static StreamOption GetStreamOptionFromString(const char* string);
  static const char* GetStringFromStreamOption(StreamOption streamOption);
  static const unsigned int STREAM_REQUIRED_OPTIONS;

  Rapr&            rapr;
  bool             inUse;
  Type             type;
  EventSource      eventSource;
  BehaviorEvent*   prev;
  BehaviorEvent*   next;
  unsigned long    ubi;
  unsigned long    foreignUbi;
  int              flowID;
  Protocol         protocol;
  ProtoAddress     srcAddr;
  unsigned short   srcPort;
  ProtoAddress     dstAddr;
  unsigned short   dstPort;
  double           behaviorStartTime;
  double           behaviorEndTime;
  int              successLogicID;
  int              failureLogicID;
  int              timeoutLogicID;
  int              payloadLogicID;
  unsigned int		 origin;
  ProtoTimerMgr&   timer_mgr;
  bool             duration;
  long             seed;

  // why do we need pattern & patternString? ljt
  MgenPattern      pattern;
  char             patternString[SCRIPT_LINE_MAX];
  int              tos;
  unsigned int     tx_buffer_size;
  int              mgen_msg_count; // mgen message count
  char             interface_name[16];
  unsigned int     ttl;
  unsigned long    sequence; // ljt probably not going to use
  unsigned int     option_mask;  
  unsigned int     logic_id_mask;
  int              raprFlowID;
  RaprPRNG*         prng;
  bool             stopped;
}; // end class BehaviorEvent

class MgenMessage : public BehaviorEvent
{
 public:
  MgenMessage(ProtoTimerMgr& timerMgr,
	    unsigned long ubi,
	    Rapr& theRapr);
  ~MgenMessage();
	    
}; // end class BehaviorEvent::MgenMessage

class RaprEvent : public BehaviorEvent
{
 public:
  RaprEvent(ProtoTimerMgr& timerMgr,
	    unsigned long ubi,
	    Rapr& theRapr);
  ~RaprEvent();

  // RaprEvent event types
  enum EventType
    {
      STOP,
      RAPRFLOWID,
      DICTIONARY_ENTRY,
      LOGICTABLE_FILE,
      DICTIONARY_FILE,
      CHANGE_STATE,
      FAULT,
      CHANGE_UBI_STATE,
      LOGICID,
      INPUT,
      CLEAR,
      ALL,
      INVALID_TYPE
    };

  static const StringMapper RAPREVENT_TYPE_LIST[]; // for mapping raprEvent event types

  void SetRaprEventType(EventType inType) {raprEventType = inType;};
  static EventType GetTypeFromString(const char* string);
  EventType GetEventType() {return raprEventType;};
  void SetName(char* inName) {strcpy(name,inName);};
  void SetValue(char* inValue) {strcpy(value,inValue);};
  void SetOverrideStartTime(bool inBool) {overrideStartTime = inBool;};
  bool GetOverrideStartTime() {return overrideStartTime;};
  void SetFaultType(FaultType inType) {faultType = inType;};
  FaultType GetFaultType() {return faultType;};
  void SetClearEventType(Type inType) {clearEventType = inType;};
  Type GetClearEventType() {return clearEventType;};
  virtual bool OnStartUp();
  virtual bool Stop();
  virtual bool ProcessEvent(const MgenMsg& theMsg);
  virtual bool OnBehaviorTimeout(ProtoTimer &theTimer);
  virtual void ClearState();
  
 private:
  EventType raprEventType;
  bool overrideStartTime;
  FaultType faultType;
  Type clearEventType;
  char name[100];
  char value[100];

}; // end class RaprEvent::RaprEvent

class Declarative : public BehaviorEvent
{
 public:

  Declarative(ProtoTimerMgr& timerMgr,
	      unsigned long ubi,
	      Rapr& theRapr);
  ~Declarative();
  virtual bool OnStartUp();                         // virtual function
  virtual bool Stop();
  virtual void ClearState();
  virtual bool ProcessEvent(const MgenMsg& theMsg);

 private:
  bool OnBehaviorTimeout(ProtoTimer& theTimer);

  ProtoTimer       endBehaviorTimer;

}; // end class BehaviorEvent::Declarative

class Interrogative : public BehaviorEvent
{
 public:

  Interrogative(ProtoTimerMgr& timerMgr,
		unsigned long ubi,
		Rapr& theRapr);
  ~Interrogative();
  virtual bool OnStartUp();
  virtual bool Stop();
  virtual bool ProcessEvent(const MgenMsg& theMsg);
  virtual void ClearState();
  void SetRetryInterval(double theInterval) {retryInterval = theInterval;};
  double GetRetryInterval() {return retryInterval;};
  void SetRetryCount(int theRetryCount) {retryCount = theRetryCount;};
  int GetRetryCount() {return retryCount;};

  protected:
  int             retryCount;

 private:
  int GetRetriesSent() {return retriesSent;};
  void SetRetriesSent(int retries_sent) {retriesSent = retries_sent;};
  int GetPayloadSeed() {return payloadSeed;};
  void SetPayloadSeed(int payload_seed) {payloadSeed = payload_seed;};
  bool OnBehaviorTimeout(ProtoTimer& theTimer);
  bool OnRetryTimeout(ProtoTimer& theTimer);
  double          retryInterval;
  int             retriesSent;
  ProtoTimer      endBehaviorTimer;
  ProtoTimer      retryTimer;
  int             payloadSeed;


}; // end class Interrogative

// The ping-pong object is only partially implemented
class PingPong : public Interrogative
{
 public:

  PingPong(ProtoTimerMgr& timerMgr,
		unsigned long ubi,
		Rapr& theRapr);
  ~PingPong();
  virtual bool ProcessEvent(const MgenMsg& theMsg);
 private:

}; // end class PingPong

class Stream : public BehaviorEvent
{
 public:

  Stream(ProtoTimerMgr& timerMgr,
		unsigned long ubi,
		Rapr& theRapr);
  ~Stream();
  virtual bool OnStartUp();
  virtual bool Stop();
  virtual bool ProcessEvent(const MgenMsg& theMsg);
  virtual void ClearState();
  void SetBurstDuration(double theDuration) {burstDuration = theDuration;};
  double GetBurstDuration() {return burstDuration;};  
  void SetBurstCount(int theBurstCount) {burstCount = theBurstCount;};
  int GetBurstCount() {return burstCount;};
  void SetBurstPriority(int theBurstPriority) {burstPriority = theBurstPriority;};
  int GetBurstPriority() {return burstPriority;};

  void SetRespProbLowRange(int theRespProbLowRange) {respProbLowRange = theRespProbLowRange;};
  int GetRespProbLowRange() {return respProbLowRange;};
  void SetRespProbHighRange(int theRespProbHighRange) {respProbHighRange = theRespProbHighRange;};
  int GetRespProbHighRange() {return respProbHighRange;};

  void SetBurstDelayLowRange(double theBurstDelayLowRange) {burstDelayLowRange = theBurstDelayLowRange;};
  double GetBurstDelayLowRange() {return burstDelayLowRange;};
  void SetBurstDelayHighRange(double theBurstDelayHighRange) {burstDelayHighRange = theBurstDelayHighRange;};
  double GetBurstDelayHighRange() {return burstDelayHighRange;};

  void SetBurstRangeLowRange(double theBurstRangeLowRange) {burstRangeLowRange = theBurstRangeLowRange;};
  double GetBurstRangeLowRange() {return burstRangeLowRange;};
  void SetBurstRangeHighRange(double theBurstRangeHighRange) {burstRangeHighRange = theBurstRangeHighRange;};
  double GetBurstRangeHighRange() {return burstRangeHighRange;};  

  void SetBurstPayloadID(int burstPayloadId) {burstPayloadID = burstPayloadId;};
  int GetBurstPayloadID() {return burstPayloadID;};
  void SetTimeoutInterval(double timeoutIntervalIn) {timeoutInterval = timeoutIntervalIn;};
  double GetTimeoutInterval() {return timeoutInterval;};

  void SetStarted(bool startedBool) {started = startedBool;};
  bool GetStarted() {return started;};
  void SetStreamQueued(bool stream_queued) {streamQueued = stream_queued;};
  bool GetStreamQueued() {return streamQueued;};
  void SetStreamState(StreamState stream_state) {streamState = stream_state;};
  bool GetStreamState() {return streamState;};
  void SetStreamUbi(unsigned long inStreamUbi) {streamUbi = inStreamUbi;};
  unsigned long GetStreamUbi() const {return streamUbi;};
  void SetStreamSeqNum(unsigned long inStreamSeqNum) {streamSeqNum = inStreamSeqNum;};
  unsigned long GetStreamSeqNum() const {return streamSeqNum;};
  bool OptionIsSet(StreamOption option) const
    {return (0 != (option & stream_option_mask));}

  void ActivateEndTimer(ProtoTimer& endBehaviorTimer);

  // ljt initialize
  vector<char *>  streamPatternVector;
  vector<int>     streamFlowIdList;
  unsigned int    stream_option_mask;  

 private:
  int GetBurstsSent() {return burstsSent;};
  void SetBurstsSent(int bursts_sent) {burstsSent = bursts_sent;};
  bool OnBehaviorTimeout(ProtoTimer& theTimer);
  bool OnBurstTimeout(ProtoTimer& theTimer);
  bool StartBurst(ProtoTimer& theTimer);
  double          burstDuration;
  int             burstCount;
  int             burstPriority;
  int             burstsSent;
  unsigned long   streamUbi;
  unsigned long   streamSeqNum;
  int             respProbLowRange;
  int             respProbHighRange;
  double          burstDelayLowRange;
  double          burstDelayHighRange;
  double          burstRangeLowRange;
  double          burstRangeHighRange;
  int             burstPayloadID;
  double          timeoutInterval;
  bool            started;
  bool            streamQueued;
  StreamState     streamState;

  ProtoTimer      endBehaviorTimer;
  ProtoTimer      endBurstTimer;
  ProtoTimer      startBurstTimer;


}; // end class Stream

class Periodic : public BehaviorEvent
{
 public:

  Periodic(ProtoTimerMgr& timerMgr,
	   unsigned long ubi,
	   Rapr& theRapr);
  ~Periodic();
  virtual bool OnStartUp();                         // virtual function
  virtual bool Stop();
  virtual void ClearState();
  virtual bool ProcessEvent(const MgenMsg& theMsg);
  Type GetBehaviorEventType() {return behaviorEventType;};
  void SetPeriodicityInterval(double inInterval) {periodicityInterval = inInterval;};
  double GetNextPeriodicityInterval();
  void SetBehaviorEndBehaviorTime(double inTime) {behaviorEndBehaviorTime = inTime;};
  void SetBehaviorEventType(Type eventType) {behaviorEventType = eventType;};
  void SetBehaviorPattern(const char* ptr) {strcpy(behaviorPattern,ptr);};
  void SetOriginalCommand(const char* ptr) {strcpy(origCommand,ptr);};
  void SetTrans(RaprDictionaryTransfer* inTrans);
  void SetExponential(bool inExponential) {exponential = inExponential;}
 private:
  bool OnBehaviorTimeout(ProtoTimer& theTimer);
  bool OnPeriodicityTimeout(ProtoTimer& theTimer);
  bool             exponential;
  ProtoTimer       endBehaviorTimer;
  ProtoTimer       periodicityTimer;
  double           periodicityInterval;
  double           behaviorEndBehaviorTime;
  Type             behaviorEventType;
  char             behaviorPattern[SCRIPT_LINE_MAX];
  int              periodicityCount;
  char             origCommand[SCRIPT_LINE_MAX];
  RaprDictionaryTransfer trans;
}; // end class BehaviorEvent::Periodic

class ReceptionEvent : public BehaviorEvent
{
 public:
  ReceptionEvent(ProtoTimerMgr& timerMgr,
	  unsigned long ubi,
	  Rapr& theRapr);
  ~ReceptionEvent();
  virtual bool OnStartUp();
  virtual bool Stop();
  virtual bool ProcessEvent(const MgenMsg& theMsg);
  virtual void ClearState();
  const char* GetStringFromType(DrecEvent::Type theType);
  const char* GetInterface()  const
    {return (('\0' != interface_name[0]) ? interface_name : NULL);}
  bool ParseDrecOptions(const char* string);

 private:
  bool OnBehaviorTimeout(ProtoTimer& theTimer);
  char* BuildDrecMsg(char* buffer,DrecEvent::Type type);
  
  ProtoTimer            endBehaviorTimer;
  DrecEvent::Type       event_type;
  
  // ljt these may need to be in another class?
  // JOIN group event parameters

  ProtoAddress          group_addr;
  char                  interface_name[16];
  unsigned short        port_count;  // (port_count is also used to hold the JOIN event PORT option)
  unsigned short*       port_list;    
  unsigned int          rx_buffer_size;

}; // end class BehaviorEvent::ReceptionEvent

/*!
Time ordered, linked list of BehaviorEvent(s)
*/
class BehaviorEventList
{
public:
  BehaviorEventList();
  ~BehaviorEventList();
  void Destroy();
  void StopEvents();
  void Insert(BehaviorEvent* theEvent);
  void Remove(BehaviorEvent* theEvent);
  /*!
    This places "theEvent" _before_ "nextEvent" in the list.
    If "nextEvent" is NULL, "theEvent" goes to the end of the list
  */
  void Precede(BehaviorEvent* nextEvent, BehaviorEvent* theEvent);

  bool IsEmpty() {return (NULL == head);}
  const BehaviorEvent* Head() const {return head;}
  const BehaviorEvent* Tail() const {return tail;}

  BehaviorEvent* FindBEByUBI(unsigned long inUBI);
  BehaviorEvent* FindBEByStreamUBI(unsigned long inUBI);
  BehaviorEvent* FindBEByFlowID(int inFlowID);
  BehaviorEvent* FindBEByRaprFlowID(int inRaprFlowID);
  BehaviorEvent* FindBEByEventType(BehaviorEvent::Type inType,BehaviorEvent* theTrigger);
  BehaviorEvent* FindUnusedBehaviorEvent(BehaviorEvent::Type objectType);

private:
  BehaviorEvent* head;
  BehaviorEvent* tail;

}; // end class BehaviorEventList

#endif // _BEHAVIOR_EVENT

