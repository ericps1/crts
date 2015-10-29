#include "behaviorEvent.h"
#include "raprPattern.h" // for StreamMapper
#include "rapr.h"
#include "processor.h"
#include "raprNumberGenerator.h"
#include "raprGlobals.h"

#include "mgenEvent.h"
#include "mgenPattern.h"
#include "raprPayload.h"

#include <ctype.h>       // for toupper()

//#include <string>

BehaviorEvent::BehaviorEvent(ProtoTimerMgr& timerMgr,
                             unsigned long inUbi,
                             Rapr& theRapr)
  : rapr(theRapr),inUse(true),type(INVALID_TYPE),
    eventSource(INVALID_EVENT),
    prev(NULL),next(NULL),
    ubi(inUbi),foreignUbi(0),
    flowID(0),
    protocol(INVALID_PROTOCOL),
    srcAddr(),srcPort(0),dstAddr(),dstPort(0),
    behaviorStartTime(0),behaviorEndTime(0),
    successLogicID(0),failureLogicID(0),timeoutLogicID(0),
    payloadLogicID(0),origin(0),
    timer_mgr(timerMgr),duration(false),
    seed(0),
    pattern(),patternString(),
    tos(0),tx_buffer_size(0),
    mgen_msg_count(0),interface_name(),ttl(0),sequence(0),
    option_mask(0),logic_id_mask(0),raprFlowID(0),stopped(false)
{
}

BehaviorEvent::~BehaviorEvent()
{
    // Should be a reference!!
    delete prng;

} // end BehaviorEvent::~BehaviorEvent

void BehaviorEvent::ClearState()
{
    inUse = false;
    
    SetEventSource(INVALID_EVENT);
    SetUBI(0);
    SetForeignUBI(0);
    // We invalidate the flow id when we get confirmation
    // that mgen processed the off event... what a mess!
    //SetFlowID(0);
    SetProtocol(INVALID_PROTOCOL);
    GetSrcAddr().Invalidate();
    SetSrcPort(0);
    GetDstAddr().Invalidate();
    SetDstPort(0);
    // breaks scheduling!   SetBehaviorStartTime(0); 
    // breaks scheduling!   SetBehaviorEndTime(0);
    SetSuccessLogicID(0);
    SetFailureLogicID(0);
    SetTimeoutLogicID(0);
    SetPayloadLogicID(0);
    SetOriginUBI(0);
    SetDuration(false);
    SetSeed(0);
    GetMgenPattern().InvalidatePattern();
    patternString[0] = '\0';
    SetTOS(0);
    
    tx_buffer_size = 0;
    mgen_msg_count = 0;
    interface_name[0] = '\0';
    ttl = 0;
    sequence = 0;
    option_mask = 0;
    logic_id_mask = 0;
    raprFlowID = 0;

    // We probably don't need to reset all this state if we're
    // just going to delete it...

    // And we should either delete(this) or do cleanup
    // somewhere!

    // we need this until we fix object cleanup
    // (so start next event will work in Rapr::StartBehaviorEvent)
    //    prev=NULL;
    //    next=NULL;
    
    // ljt 02/15/14 !!  BLOAT
    // This hack is because of the mgen/rapr off event disconnect
    // and we don't have time to properly fix it.  We need
    // to make sure that mgen has processed the off event
    // before we remove the object.

    //rapr.GetBehaviorEventList().Remove(this);
    
} // end BehaviorEvent::ClearState()

const StringMapper BehaviorEvent::TYPE_LIST[] = 
  {
    {"+LISTEN",            RECEPTIONEVENT},
    {"+JOIN",              RECEPTIONEVENT},
    {"+RAPREVENT",         RAPREVENT}, 
    {"+RAPRFLOWID",        RAPREVENT},
    {"+DICTIONARY_ENTRY",  RAPREVENT},
    {"+STOP",              RAPREVENT}, 
    {"+FAULT",             RAPREVENT},
    {"+CLEAR",             RAPREVENT},
    {"+DECLARATIVE",       DECLARATIVE},
    {"+INTERROGATIVE",     INTERROGATIVE},
    {"+PINGPONG",          PINGPONG},
    {"+PERIODIC",          PERIODIC},
    {"+STREAM",            STREAM},
    {"+PROCESSOR",         PROCESSOR},
    {"+LOGICTABLE_FILE",	  RAPREVENT},
    {"+DICTIONARY_FILE",	  RAPREVENT},
    {"+CHANGE_STATE",	  RAPREVENT},
    {"+CHANGE_UBI_STATE",  RAPREVENT},
    {"+LOGICID",	          RAPREVENT},
    {"+INPUT",             RAPREVENT},
    {"+ALL",               ALL}, 
    {"+XXXX",              INVALID_TYPE}   
  };

BehaviorEvent::Type BehaviorEvent::GetTypeFromString(const char* string)
{
    // Make comparison case-insensitive
    char upperString[16];
    unsigned int len = strlen(string);
    len = len < 15 ? len : 15;
    unsigned int i;
    for (i =0 ; i < len; i++)
      upperString[i] = toupper(string[i]);
    upperString[i] = '\0';
    const StringMapper* m = TYPE_LIST;
    unsigned matchCount = 0;
    Type eventType = INVALID_TYPE;
    while (INVALID_TYPE != (*m).key)
    {
        if (!strncmp(upperString, (*m).string+1, len))
        {        
            matchCount++;
            eventType = ((Type)((*m).key));
        }
        m++;
    }
    if (matchCount > 1)
    {
        DMSG(0, "BehaviorEvent::GetTypeFromString() Error: ambiguous event type\n");
        return INVALID_TYPE;   
    }
    else
    {
        return eventType;
    }    
}  // end BehaviorEvent::GetTypeFromString()

const StringMapper BehaviorEvent::EVENTTYPE_LIST[] = 
  {
    {"RECEPTIONEVENT",   RECEPTIONEVENT},
    {"RAPREVENT",        RAPREVENT},
    {"DECLARATIVE",      DECLARATIVE},
    {"INTERROGATIVE",    INTERROGATIVE},
    {"PINGPONG",         PINGPONG},
    {"PERIODIC",         PERIODIC},
    {"STREAM",           STREAM},
    {"INVALID_TYPE",     INVALID_TYPE}
  };


const char* BehaviorEvent::GetStringFromEventType(Type eventType)
{
    const StringMapper* m = EVENTTYPE_LIST;
    while (INVALID_TYPE != (*m).key)
    {
        if (eventType == (*m).key) {
            return (*m).string;  
        }
        m++;
    }
    return "UNKNOWN";
}  // end BehaviorEvent::GetStringFromEventType()

const StringMapper BehaviorEvent::EVENTSOURCE_LIST[] = 
  {
    {"script_event",   SCRIPT_EVENT},
    {"rti_event",      RTI_EVENT}, // need me?
    {"net_event",      NET_EVENT},
    {"invalid_event",  INVALID_EVENT}
  };

const char* BehaviorEvent::GetStringFromEventSource(EventSource eventSource)
{
    const StringMapper* m = EVENTSOURCE_LIST;
    while (INVALID_EVENT != (*m).key)
    {
        if (eventSource == (*m).key) {
            return (*m).string;  
        }
        m++;
    }
    return "UNKNOWN";
}  // end BehaviorEvent::GetStringFromEventSource()

BehaviorEvent::EventSource BehaviorEvent::GetEventSourceFromString(const char* string)
{
    // Make comparison case-insensitive
    char upperString[16];
    unsigned int len = strlen(string);
    len = len < 15 ? len : 15;
    unsigned int i;
    for (i =0 ; i < len; i++)
      upperString[i] = toupper(string[i]);
    upperString[i] = '\0';
    const StringMapper* m = TYPE_LIST;
    unsigned matchCount = 0;
    EventSource eventSource = INVALID_EVENT;
    while (INVALID_EVENT != (*m).key)
    {
        if (!strncmp(upperString, (*m).string, len))
        {        
            matchCount++;
            eventSource = ((EventSource)((*m).key));
        }
        m++;
    }
    if (matchCount > 1)
    {
        DMSG(0, "BehaviorEvent::GetTypeFromString() Error: ambiguous event type\n");
        return INVALID_EVENT;   
    }
    else
    {
        return eventSource;
    }    
}  // end BehaviorEvent::GetEventSourceFromString()

const StringMapper BehaviorEvent::PROTOCOL_LIST[] = 
  {
    {"UDP",     UDP}, 
    {"TCP",     TCP},
    {"SINK",    SINK},
    {"XXX",     INVALID_PROTOCOL}   
  };

Protocol BehaviorEvent::GetProtocolFromString(const char* string)
{
    // Make comparison case-insensitive
    char upperString[16];
    unsigned int len = strlen(string);
    len = len < 15 ? len : 15;
    unsigned int i;
    for (i =0 ; i < len; i++)
      upperString[i] = toupper(string[i]);
    upperString[i] = '\0';
    unsigned int matchCount = 0;
    Protocol protocolType = INVALID_PROTOCOL;
    const StringMapper* m = PROTOCOL_LIST;
    while (INVALID_PROTOCOL != (*m).key)
    {
        if (!strncmp(upperString, (*m).string, len))
        {
            protocolType = ((Protocol)((*m).key));
            matchCount++;
        }
        m++;  
    }
    if (matchCount > 1)
    {
        DMSG(0, "BehaviorEvent::GetProtocolFromString() Error: ambiguous protocol\n");
        return INVALID_PROTOCOL;
    }
    else
    {
        return protocolType;
    }
}  // end BehaviorEvent::GetProtocolFromString()

const char* BehaviorEvent::GetStringFromProtocol(Protocol protocol)
{
    const StringMapper* m = PROTOCOL_LIST;
    while (INVALID_PROTOCOL != (*m).key)
    {
        if (protocol == (*m).key)
          return (*m).string;  
        m++;
    }
    return "UNKNOWN";
}  // end BehaviorEvent::GetStringFromProtocol()

const StringMapper BehaviorEvent::OPTION_LIST[] =  
  {
    {"PROTOCOL", PROTOCOL},
    {"DST", DST},
    {"SRC", SRC},
    {"PATTERN", PATTERN},
    {"TOS", TOS},
    {"RSVP", RSVP},
    {"INTERFACE", INTERFACE},
    {"TTL", TTL},
    {"SEQUENCE", SEQUENCE},
    {"LABEL",LABEL},
    {"TXBUFFER",TXBUFFER},
    {"SEED",SEED},
    {"RAPRFLOWID",RAPRFLOWID},
    {"COUNT",COUNT},
    {"XXXX", INVALID_OPTION}   
  }; // end BehaviorEvent::OPTION_LIST


const StringMapper BehaviorEvent::LOGIC_ID_LIST[] =  
  {
    {"SUCCESS", SUCCESS},
    {"FAILURE", FAILURE},
    {"PAYLOAD", PAYLOAD},
    {"TIMEOUT", TIMEOUT},
    {"ORIGIN",  ORIGIN},
    {"LOGICID", LOGICID},
    {"XXXX", INVALID_OPTION}   
  }; // end BehaviorEvent::LOGIC_ID_LIST

const unsigned int BehaviorEvent::ON_REQUIRED_OPTIONS = 
  (BehaviorEvent::PROTOCOL | BehaviorEvent::DST | BehaviorEvent::PATTERN);

BehaviorEvent::Option BehaviorEvent::GetOptionFromString(const char* string)
{
    // Make comparison case-insensitive
    char upperString[16];
    unsigned int len = strlen(string);
    len = len < 15 ? len : 15;
    unsigned int i;
    for (i =0 ; i < len; i++)
      upperString[i] = toupper(string[i]);
    upperString[i] = '\0';
    unsigned int matchCount = 0;
    Option optionType = INVALID_OPTION;
    const StringMapper* m = OPTION_LIST;
    while (INVALID_OPTION != (*m).key)
    {
        if (!strncmp(upperString, (*m).string, len))
        {
            optionType = ((Option)((*m).key));
            matchCount++;
        }
        m++;
    }
    if (matchCount > 1)
    {
        DMSG(0, "BehaviorEvent::GetOptionFromString() Error: ambiguous option\n");        
        return INVALID_OPTION;
    }
    else
    {
        return optionType;
    }
}  // end BehaviorEvent::GetOptionFromString()

const char* BehaviorEvent::GetStringFromOption(Option option)
{
    const StringMapper* m = OPTION_LIST;
    while (INVALID_OPTION != m->key)
    {
        if (option == (Option)(m->key))
          return m->string;
        else
          m++;  
    }
    return "INVALID";
} // end BehaviorEvent::GetStringFromOption()

BehaviorEvent::StreamOption BehaviorEvent::GetStreamOptionFromString(const char* string)
{
    // Make comparison case-insensitive
    char upperString[16];
    unsigned int len = strlen(string);
    len = len < 15 ? len : 15;
    unsigned int i;
    for (i =0 ; i < len; i++)
      upperString[i] = toupper(string[i]);
    upperString[i] = '\0';
    unsigned int matchCount = 0;
    StreamOption streamOption = INVALID_STREAM_OPTION;
    const StringMapper* m = STREAM_OPTION_LIST;
    while (INVALID_STREAM_OPTION != (*m).key)
    {
        if (!strncmp(upperString, (*m).string, len))
        {
            streamOption = ((StreamOption)((*m).key));
            matchCount++;
        }
        m++;
    }
    if (matchCount > 1)
    {
        DMSG(0, "BehaviorEvent::GetStreamOptionFromString() Error: ambiguous stream option");        
        return INVALID_STREAM_OPTION;
    }
    else
    {
        return streamOption;
    }
}  // end BehaviorEvent::GetStreamOptionFromString()

const char* BehaviorEvent::GetStringFromStreamOption(StreamOption streamOption)
{
    const StringMapper* m = STREAM_OPTION_LIST;
    while (INVALID_STREAM_OPTION != m->key)
    {
        if (streamOption == (StreamOption)(m->key))
          return m->string;
        else
          m++;  
    }
    return "INVALID";
} // end BehaviorEvent::GetStringFromStreamOption()

BehaviorEvent::LogicIdType BehaviorEvent::GetLogicIdFromString(const char* string)
{
    // Make comparison case-insensitive
    char upperString[16];
    unsigned int len = strlen(string);
    len = len < 15 ? len : 15;
    unsigned int i;
    for (i =0 ; i < len; i++)
      upperString[i] = toupper(string[i]);
    upperString[i] = '\0';
    unsigned int matchCount = 0;
    LogicIdType logicIdType = INVALID_LOGIC_ID;
    const StringMapper* m = LOGIC_ID_LIST;
    while (INVALID_LOGIC_ID != (*m).key)
    {
        if (!strncmp(upperString, (*m).string, len))
        {
            logicIdType = ((LogicIdType)((*m).key));
            matchCount++;
        }
        m++;
    }
    if (matchCount > 1)
    {
        DMSG(0, "BehaviorEvent::GetLogicIdFromString() Error: ambiguous logic ID");        
        return INVALID_LOGIC_ID;
    }
    else
    {
        return logicIdType;
    }
}  // end BehaviorEvent::GetLogicIdFromString()

const char* BehaviorEvent::GetStringFromLogicId(LogicIdType logicId)
{
    const StringMapper* m = LOGIC_ID_LIST;
    while (INVALID_LOGIC_ID != m->key)
    {
        if (logicId == (LogicIdType)(m->key))
          return m->string;
        else
          m++;  
    }
    return "INVALID";
} // end BehaviorEvent::GetStringFromLogicId()

bool BehaviorEvent::ParseRaprEventOptions(const char* lineBuffer, int& cnt)
{
    const char * ptr = lineBuffer;
    if (!ptr) return true;
    
    char fieldBuffer[SCRIPT_LINE_MAX+1];
    if (1 != sscanf(ptr,"%s",fieldBuffer))
    {
        DMSG(0,"BehaviorEvent::ParseRaprEventOptions Error: Invalid RaprEvent\n");
    }
    
    ptr += strlen(fieldBuffer);
    while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
    
    RaprEvent::EventType raprEventType = RaprEvent::GetTypeFromString(fieldBuffer);
    char inName[100];
    char inValue[100];
    inName[0] = '\0';
    inValue[0] = '\0';
    
    // Default behavior is if no time is specified
    // ALWAYS process the rapr event.  
    if ((static_cast<RaprEvent*>(this))->GetBehaviorStartTime() < 0)
    {
        ((static_cast<RaprEvent*>(this)))->SetOverrideStartTime(true);
    }
    else 
    {  
        ((static_cast<RaprEvent*>(this)))->SetOverrideStartTime(false);
    }
    
    switch (raprEventType)
    {
    case RaprEvent::STOP:
      {
          // No implementation?
          DMSG(0,"BehaviorEvent::ParseRaprEvent Error: STOP raprevent not implemented.\n");
          break;
      }
    case RaprEvent::RAPRFLOWID:
      {
          
          ((RaprEvent*)this)->SetRaprEventType(RaprEvent::RAPRFLOWID);
          break;
      }
    case RaprEvent::DICTIONARY_ENTRY:
      {
          if (1 == sscanf(ptr,"%s",inName))
          {
              if (1 != sscanf(ptr,"%s",fieldBuffer))
              {
                  DMSG(0,"Rapr::ParseRaprEvent() Error: missing dictionary name\n");
                  return false;
              }
              
              ((RaprEvent*)this)->SetName(inName);
              ptr += strlen(fieldBuffer);
              while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
          }
          else
          {
              DMSG(0,"Rapr::ParseBehaviorEvent() Error: missing dictionary name\n");
              return false;
          }
          // Point to beginning of value parameters
          const char* pptr = ptr;
          // Find end of pattern parameter set
          unsigned int nested = 1;
          while (nested)
          {
              ptr++;
              if ('[' == *ptr)
              {
                  nested++;
              }
              else if (']' == *ptr)
              {
                  nested--;
              }
              if ('\0' == *ptr)
              {
                  DMSG(0,"Rapr::ParseBehaviorEvent() Error: non-terminated <dictionaryValueParams>\n");
                  return false;
              }
          }
          if ((*pptr != '[') || !ptr)
          {
              DMSG(0,"Rapr::ParseBehaviorEvenet() Error: missing <dictionaryValueParams>\n");
              return false;
          }
          strncpy(fieldBuffer,pptr+1,ptr - pptr - 1);
          
          // rebuild "our" patternString
          char patternStringTmp[SCRIPT_LINE_MAX] = "";
          strncpy(patternStringTmp,fieldBuffer,ptr - pptr - 1);
          strncat(inValue,patternStringTmp,sizeof(patternStringTmp));
          // end rebuild
          
          // Set ptr to next field, skipping any white space
          ptr++;
          while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
          
          ((RaprEvent*)this)->SetValue(inValue);
          ((RaprEvent*)this)->SetRaprEventType(RaprEvent::DICTIONARY_ENTRY);
          
          break;
      }
    case RaprEvent::LOGICTABLE_FILE:
      {
          if (1 == sscanf(ptr,"%s",inName))
          {
              if (1 != sscanf(ptr,"%s",fieldBuffer))
              {
                  DMSG(0,"Rapr::ParseBehaviorEvent() Error: missing logic table name\n");
                  return false;
              }
              ptr += strlen(fieldBuffer);
              while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
          }
          else 
          {
              DMSG(0,"Rapr::ParseBehaviorEvent() Error: missing logic table name\n");
              return false;
          }
          // just reusing the method - clean up!
          ((RaprEvent*)this)->SetName(inName);
          ((RaprEvent*)this)->SetRaprEventType(RaprEvent::LOGICTABLE_FILE);
          
          break;
      }
    case RaprEvent::DICTIONARY_FILE:
      {
          if (1 == sscanf(ptr,"%s",inName))
          {
              if (1 != sscanf(ptr,"%s",fieldBuffer))
              {
                  DMSG(0,"Rapr::ParseBehaviorEvent() Error: missing dictionary file name\n");
                  return false;
              }
              ptr += strlen(fieldBuffer);
              while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
          }
          else 
          {
              DMSG(0,"Rapr::ParseBehaviorEvent() Error: missing dictionary file name\n");	      
              return false;
          }
          
          ((RaprEvent*)this)->SetRaprEventType(RaprEvent::DICTIONARY_FILE);
          ((RaprEvent*)this)->SetName(inName);
          
          break;
      }
    case RaprEvent::CHANGE_STATE:
      {
          if (1 == sscanf(ptr,"%s",inName))
          {
              if (1 != sscanf(ptr,"%s",fieldBuffer))
              {
                  DMSG(0,"Rapr::ParseBehaviorEvent() Error: missing state name\n");
                  return false;
              }
              ptr += strlen(fieldBuffer);
              while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
          }
          else 
          {
              DMSG(0,"Rapr::ParseBehaviorEvent() Error: missing state name\n");
              return false;
          }
          // reusing - fix this!
          ((RaprEvent*)this)->SetName(inName);
          ((RaprEvent*)this)->SetRaprEventType(RaprEvent::CHANGE_STATE);
          
          break;
      }
    case RaprEvent::FAULT:
      {
          int inFault=0;
          
          if (1 == sscanf(ptr,"%i",&inFault))
          {
              if (1 != sscanf(ptr,"%s",fieldBuffer))
              {
                  DMSG(0,"Rapr::ParseBehaviorEvent() Error: missing fault type\n");
                  return false;
              }
              if (inFault < 1 || inFault > 3)
              {
                  DMSG(0,"Rapr::ParseBehaviorEvent() Error: Invalid fault type.\n");
                  return false;
              }
              
              ptr += strlen(fieldBuffer);
              while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
          }
          else 
          {
              DMSG(0,"Rapr::ParseBehaviorEvent() Error: missing fault name\n");
              return false;
          }
          
          ((RaprEvent*)this)->SetRaprEventType(RaprEvent::FAULT);
          switch (inFault)
          {
          case 0:
            ((RaprEvent*)this)->SetFaultType(BehaviorEvent::INVALID_FAULT);
            break;
          case 1:
            ((RaprEvent*)this)->SetFaultType(BehaviorEvent::STOP_RAPR);
            break;
          case 2:
            ((RaprEvent*)this)->SetFaultType(BehaviorEvent::STOP_MGEN);
            break;
          case 3:
            ((RaprEvent*)this)->SetFaultType(BehaviorEvent::STOP_EVENTS);
            break;
          default:
            ((RaprEvent*)this)->SetFaultType(BehaviorEvent::INVALID_FAULT);
            DMSG(0,"Rapr::ParseBehaviorEvent Error: Invalid Fault type assigned!\n");
          }
          
          break;
      }
    case RaprEvent::CLEAR:
      {
          char fieldBuffer[SCRIPT_LINE_MAX+1];
          BehaviorEvent::Type eventType;
          
          if (1 == sscanf(ptr,"%s",fieldBuffer))
          {
              eventType = BehaviorEvent::GetTypeFromString(fieldBuffer);
              if (eventType != BehaviorEvent::INVALID_TYPE) 
              {
                  ((RaprEvent*)this)->SetClearEventType(eventType);
                  ((RaprEvent*)this)->SetRaprEventType(RaprEvent::CLEAR);
              }
              else 
              {
                  DMSG(0,"RaprEvent::Clear Error: invalid clear event type.\n");
                  return false;
              }
          }
          else 
          {
              DMSG(0,"RaprEvent::ParseOptions Error: invalid clear statment.\n");
          }
          
          break;
      }
      
    case RaprEvent::CHANGE_UBI_STATE:
      {
          if (1 == sscanf(ptr,"%s",inName))
          {
              if (1 != sscanf(ptr,"%s",fieldBuffer))
              {
                  DMSG(0,"Rapr::ParseBehaviorEvent() Error: missing state UBI\n");
                  return false;
              }
              ptr += strlen(fieldBuffer);
              while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
          }
          else 
          {
              DMSG(0,"Rapr::ParseBehaviorEvent() Error: missing state UBI\n");
              return false;
          }
          
          while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
          
          if (1 == sscanf(ptr,"%s",inValue))
          {
              if (1 != sscanf(ptr,"%s",fieldBuffer))
              {
                  DMSG(0,"Rapr::ParseBehaviorEvent() Error: missing state value\n");
                  return false;
              }
              ptr += strlen(fieldBuffer);
              while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
          }
          else 
          {
              DMSG(0,"Rapr::ParseBehaviorEvent() Error: missing state value\n");
              return false;
          }
          
          ((RaprEvent*)this)->SetRaprEventType(RaprEvent::CHANGE_UBI_STATE);
          // reuse - Better names!
          ((RaprEvent*)this)->SetName(inName);
          ((RaprEvent*)this)->SetValue(inValue);
          
          break;
      }
    case RaprEvent::LOGICID:
      {
          if (1 == sscanf(ptr,"%s",inName))
          {
              if (1 != sscanf(ptr,"%s",fieldBuffer))
              {
                  DMSG(0,"Rapr::ParseBehaviorEvent() Error: missing logic id number\n");
                  return false;
              }
              ptr += strlen(fieldBuffer);
              while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
          }
          else 
          {
              DMSG(0,"Rapr::ParseBehaviorEvent() Error: missing logic id number\n");	      return false;
          }
          ((RaprEvent*)this)->SetRaprEventType(RaprEvent::LOGICID);
          ((RaprEvent*)this)->SetName(inName);
          
          break;
      }
    case RaprEvent::INPUT:
      {
          if (1 == sscanf(ptr,"%s",inName))
          {
              if (1 != sscanf(ptr,"%s",fieldBuffer))
              {
                  DMSG(0,"Rapr::ParseBehaviorEvent() Error: missing input script file name\n");
                  return false;
              }
              ptr += strlen(fieldBuffer);
              while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
          }
          else 
          {
              DMSG(0,"Rapr::ParseBehaviorEvent() Error: missing input script file name\n");	      
              return false;
          }
          ((RaprEvent*)this)->SetRaprEventType(RaprEvent::INPUT);
          ((RaprEvent*)this)->SetName(inName);
          
          break;
          
      }
    default:
      {
          DMSG(0,"BehaviorEvent::ParseRaprEventOptions Error: invalid rapr event type\n");
          return true;
      }
    }
    
    return true;
    
} // end BehaviorEvent::ParseRaprEventOptions

bool BehaviorEvent::ParsePeriodicOptions(const char* ptr, int& cnt)
{
    // Get periodicity interval
    char fieldBuffer[SCRIPT_LINE_MAX];
    if (1 == sscanf(ptr,"%s",fieldBuffer))
    {
        if (!strncmp("INTERVAL",fieldBuffer,strlen(fieldBuffer)))
        {
            ptr += strlen(fieldBuffer);
            cnt += strlen(fieldBuffer);
            while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
            double periodicityInterval = 0;
            if (1 == sscanf(ptr,"%lf",&periodicityInterval))
            {
                ((Periodic*)this)->SetPeriodicityInterval(periodicityInterval);
                if (1 != sscanf(ptr,"%s",fieldBuffer))
                {
                    DMSG(0,"BehaviorEvent::ParsePeriodicOptins() Error: periodicity interval must be specified.\n");
                    return false;
                }
                ptr += strlen(fieldBuffer);
                cnt += strlen(fieldBuffer);
                while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
                if (1 == sscanf(ptr,"%s",fieldBuffer))
                {
                    if (!strncmp("EXP",fieldBuffer,strlen(fieldBuffer)))
                    {
                        (static_cast<Periodic*>(this))->SetExponential(true);
                        ptr += strlen(fieldBuffer);
                        cnt += strlen(fieldBuffer);
                        while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
                    }
                }
            }
            else 
            {
                DMSG(0,"BehaviorEvent::ParsePeriodicOptions() Error: periodicity interval must be specified.\n");
                return false;
            }
        }
        else 
        {
            DMSG(0,"BehaviorEvent::ParsePeriodicOptions() Error: periodicity interval must be specified.\n");
            return false;
        }
    }
    else 
    {
        DMSG(0,"BehaviorEvent::ParsePeriodicOptions() Error: periodicity interval must be specified.\n");
        return false;
    }
    
    // Get behavior duration.  (No STOP option for periodic objects)
    double behaviorEndBehaviorTime = 0;
    if (1 == sscanf(ptr,"%s",fieldBuffer))
    {
        if (!strncmp("DURATION",fieldBuffer,strlen(fieldBuffer)))
        {
            ptr += strlen(fieldBuffer);
            cnt += strlen(fieldBuffer);
            while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
            if (1 == sscanf(ptr,"%lf",&behaviorEndBehaviorTime))
            {
                ((Periodic*)this)->SetBehaviorEndBehaviorTime(behaviorEndBehaviorTime);
                if (1 != sscanf(ptr,"%s",fieldBuffer))
                {
                    DMSG(0,"BehaviorEvent::ParsePeriodicOptins() Error: periodicity stop/duration must be specified.\n");
                    return false;
                }
                ptr += strlen(fieldBuffer);
                cnt += strlen(fieldBuffer);
                while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
            }
            else 
            {
                DMSG(0,"BehaviorEvent::ParsePeriodicOptions() Error: periodicity duration must be specified.\n");
                return false;
            }
        }
    }

    // if !behaviorEndBehaviorTime will be set by periodic object type
    // Get the behavior type
    if (1 != sscanf(ptr,"%s",fieldBuffer))
    {
        DMSG(0,"BehaviorEvent::ParsePeriodicOptions() Error: No behavior type.\n");
        return false;
    }
    
    BehaviorEvent::Type eventType = BehaviorEvent::GetTypeFromString(fieldBuffer);
    // If its a valid event, store it for later.
    if (eventType != BehaviorEvent::INVALID_TYPE)
    {
        ((Periodic*)this)->SetBehaviorEventType(eventType);
    }
    
    return true;
} // end BehaviorEvent::ParsePeriodicOptions

const char* BehaviorEvent::LookupDictionaryEntry(char* buf)
{
    RaprStringVector *vec;
    RaprPRNG *prng = NULL;
    vec = rapr.LogicTable().TranslateString(buf,prng);
    if (vec != NULL)
    {
        for (unsigned int i = 0; i < vec->size(); i++)
        {
            return ((*vec)[i]);
        }
    }
    return NULL;
} // end BehaviorEvent::LookupDictionaryEntry

bool BehaviorEvent::ParseInterrogativeOptions(const char* ptr,int& cnt)
{
    
    char fieldBuffer[SCRIPT_LINE_MAX];
    // Read Retry Time period
    if (1 == sscanf(ptr,"%s",fieldBuffer))
    {
        // convert to upper case for case-insensitivity          
        unsigned int len = strlen(fieldBuffer);
        len = len < 31 ? len : 31;
        unsigned int i;
        for (i = 0 ; i < len; i++)
          fieldBuffer[i] = toupper(fieldBuffer[i]);

        if (!strncmp("RETRYINTERVAL",fieldBuffer,strlen(fieldBuffer)))
        {
            ptr += strlen(fieldBuffer);
            cnt += strlen(fieldBuffer);
            while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
            
            double retryInterval = 0;
            if (1 == sscanf(ptr,"%lf",&retryInterval))
            {
                if (1 != sscanf(ptr,"%s",fieldBuffer))
                {
                    DMSG(0,"BehaviorEvent::ParseInterrogativeOptions() Error: retry interval required\n");
                    return false;
                }
                ((Interrogative*)this)->SetRetryInterval(retryInterval);
                
                ptr += strlen(fieldBuffer);
                cnt += strlen(fieldBuffer);
                while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
            }
            else // no retry interval specified 
            {
                DMSG(0,"BehaviorEvent::ParseInterrogativeOptions() Error: Warning - no retry interval specified\n");
                return false;
                
            }	  
        }
        else  // No retry interval keyword, look in dictionary for default
        {
            const char *dictionaryValue;
            if ((dictionaryValue = LookupDictionaryEntry("%SystemDefaults:retryinterval%")))
              ((Interrogative*)this)->SetRetryInterval(atoi(dictionaryValue));
        }
    }
    
    if (1 == sscanf(ptr,"%s",fieldBuffer))
    {
        // convert to upper case for case-insensitivity          
        unsigned int len = strlen(fieldBuffer);
        len = len < 31 ? len : 31;
        unsigned int i;
        for (i = 0 ; i < len; i++)
          fieldBuffer[i] = toupper(fieldBuffer[i]);
        
        if (!strncmp("NUMRETRIES",fieldBuffer,strlen(fieldBuffer)))
        {
            ptr += strlen(fieldBuffer);
            cnt += strlen(fieldBuffer);
            while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
            
            // Read retry count
            int retryCount=0;
            if (1 == sscanf (ptr,"%d",&retryCount))
            {
                if (1 != sscanf(ptr,"%s", fieldBuffer))
                {
                    
                    DMSG(0,"BehaviorEvent::ParseInterrogativeOptions() Error: retry count requried\n");
                    return false;
                }
                ((Interrogative*)this)->SetRetryCount(retryCount);
                ptr += strlen(fieldBuffer);
                cnt += strlen(fieldBuffer);
                while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++; 
            }  
        }
        else
        {
            const char *dictionaryValue;
            if ((dictionaryValue = LookupDictionaryEntry("%SystemDefaults:numretries%")))
              ((Interrogative*)this)->SetRetryCount(atoi(dictionaryValue));
        }
    }

    if (!behaviorEndTime)
      behaviorEndTime = ((Interrogative*)this)->GetRetryInterval() * (((Interrogative*)this)->GetRetryCount() + 1) + behaviorStartTime;  
    
    return true;
} // end BehaviorEvent::ParseInterrogativeOptions

// This utter hack is made in the essence of time.  How did
// the old code ever work?
bool BehaviorEvent::ParsePeriodicInterrogativeOptions(const char* ptr,int& cnt)
{
    
    char fieldBuffer[SCRIPT_LINE_MAX];
    // Read Retry Time period
    if (1 == sscanf(ptr,"%s",fieldBuffer))
    {
        // convert to upper case for case-insensitivity          
        unsigned int len = strlen(fieldBuffer);
        len = len < 31 ? len : 31;
        unsigned int i;
        for (i = 0 ; i < len; i++)
          fieldBuffer[i] = toupper(fieldBuffer[i]);

        if (!strncmp("RETRYINTERVAL",fieldBuffer,strlen(fieldBuffer)))
        {
            ptr += strlen(fieldBuffer);
            cnt += strlen(fieldBuffer);
            while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
            
            double retryInterval = 0;
            if (1 == sscanf(ptr,"%lf",&retryInterval))
            {
                if (1 != sscanf(ptr,"%s",fieldBuffer))
                {
                    DMSG(0,"BehaviorEvent::ParseInterrogativeOptions() Error: retry interval required\n");
                    return false;
                }
                //((Interrogative*)this)->SetRetryInterval(retryInterval);
                
                ptr += strlen(fieldBuffer);
                cnt += strlen(fieldBuffer);
                while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
            }
            else // no retry interval specified 
            {
                DMSG(0,"BehaviorEvent::ParseInterrogativeOptions() Error: Warning - no retry interval specified\n");
                return false;
                
            }	  
        }
        else  // No retry interval keyword, look in dictionary for default
        {
            const char *dictionaryValue;
	    //    if ((dictionaryValue = LookupDictionaryEntry("%SystemDefaults:retryinterval%")))
              //((Interrogative*)this)->SetRetryInterval(atoi(dictionaryValue));
        }
    }
    
    if (1 == sscanf(ptr,"%s",fieldBuffer))
    {
        // convert to upper case for case-insensitivity          
        unsigned int len = strlen(fieldBuffer);
        len = len < 31 ? len : 31;
        unsigned int i;
        for (i = 0 ; i < len; i++)
          fieldBuffer[i] = toupper(fieldBuffer[i]);
        
        if (!strncmp("NUMRETRIES",fieldBuffer,strlen(fieldBuffer)))
        {
            ptr += strlen(fieldBuffer);
            cnt += strlen(fieldBuffer);
            while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
            
            // Read retry count
            int retryCount=0;
            if (1 == sscanf (ptr,"%d",&retryCount))
            {
                if (1 != sscanf(ptr,"%s", fieldBuffer))
                {
                    
                    DMSG(0,"BehaviorEvent::ParseInterrogativeOptions() Error: retry count requried\n");
                    return false;
                }
                //((Interrogative*)this)->SetRetryCount(retryCount);
                ptr += strlen(fieldBuffer);
                cnt += strlen(fieldBuffer);
                while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++; 
            }  
        }
        else
        {
            const char *dictionaryValue;
            //if ((dictionaryValue = LookupDictionaryEntry("%SystemDefaults:numretries%")))
              //((Interrogative*)this)->SetRetryCount(atoi(dictionaryValue));
        }
    }

    //    if (!behaviorEndTime)
    // behaviorEndTime = ((Interrogative*)this)->GetRetryInterval() * (((Interrogative*)this)->GetRetryCount() + 1) + behaviorStartTime;  
    
    return true;
} // end BehaviorEvent::ParsePeriodicInterrogativeOptions

const StringMapper BehaviorEvent::STREAM_OPTION_LIST[] =  // ljt get rid of unused ones
  {
    {"BURSTDURATION", BURSTDURATION},
    {"BURSTCOUNT",BURSTCOUNT},
    {"BURSTPRIORITY",BURSTPRIORITY},
    {"BURSTDELAY",BURSTDELAY},
    {"BURSTRANGE",BURSTRANGE},
    {"RESPPROB",RESPPROB},
    {"BURSTPAYLOADID",BURSTPAYLOADID},
    {"TIMEOUTINTERVAL",TIMEOUTINTERVAL},
    {"XXXX", INVALID_OPTION}   
  }; // end BehaviorEvent::STREAM_OPTION_LIST

bool BehaviorEvent::ParseStreamOptions(const char* ptr,int& cnt)
{
    char fieldBuffer[SCRIPT_LINE_MAX];
    
    while(( ' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
    
    if (1 != sscanf(ptr,"%s",fieldBuffer))
    {
        DMSG(0,"BehaviorEvent::ParseOptions() Error: empty stream option type.\n");
        return false;
    }
    
    
    // Interate through any options
    ((Stream*)this)->stream_option_mask = 0;
    while ('\0' != *ptr)
    {
        // Read option label
        if (1 != sscanf(ptr,"%s",fieldBuffer))
        {
            DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: stream option parse error \n");
            return false;
        }
        // Is it a stream option?
        StreamOption streamOption = GetStreamOptionFromString(fieldBuffer);
        
        if (streamOption != INVALID_STREAM_OPTION)
        {
            if (1 != sscanf(ptr,"%s",fieldBuffer))
            {
                DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: stream option parse error.\n");
                return false;
            }
            ptr += strlen(fieldBuffer);
            cnt += strlen(fieldBuffer);
            while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
            
            switch (streamOption)
            {
            case BURSTDURATION:
              {
                  if (1 != sscanf(ptr,"%s",fieldBuffer))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing burst duration.\n");
                      return false;
                  }
                  double burstDuration = 0;
                  if (1 != sscanf(fieldBuffer,"%lf",&burstDuration))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing burst duration.\n");
                      return false;
                  }
                  ((Stream*)this)->SetBurstDuration(burstDuration);
                  ptr += strlen(fieldBuffer);
                  cnt += strlen(fieldBuffer);
                  while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
                  
                  break;
              }
            case BURSTCOUNT:
              {
                  if (1 != sscanf(ptr,"%s",fieldBuffer))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing burst count.\n");
                      return false;
                  }
                  int burstCount = 0;
                  if (1 != sscanf(fieldBuffer,"%d",&burstCount))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing burst count.\n");
                      return false;
                  }
                  ((Stream*)this)->SetBurstCount(burstCount);
                  
                  ptr += strlen(fieldBuffer);
                  cnt += strlen(fieldBuffer);
                  while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
                  
                  break;
              }
            case BURSTPRIORITY:
              {
                  if (1 != sscanf(ptr,"%s",fieldBuffer))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing burst priority.\n");
                      return false;
                  }
                  int burstPriority = 0;
                  if (1 != sscanf(fieldBuffer,"%d",&burstPriority))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing burst priority.\n");
                      return false;
                  }
                  ((Stream*)this)->SetBurstPriority(burstPriority);
                  
                  ptr += strlen(fieldBuffer);
                  cnt += strlen(fieldBuffer);
                  while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
                  
                  break;
              }
            case BURSTDELAY:
              {
                  if (1 != sscanf(ptr,"%s",fieldBuffer))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing burst delay lower range value.\n");
                      return false;
                  }
                  double burstDelayLowRange=0;
                  double burstDelayHighRange=0;
                  if (1 != sscanf(fieldBuffer,"%lf",&burstDelayLowRange))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing burst delay lower range value.\n");
                      return false;
                  }
                  ((Stream*)this)->SetBurstDelayLowRange(burstDelayLowRange);
                  ptr += strlen(fieldBuffer);
                  cnt += strlen(fieldBuffer);
                  while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
                  if (1 != sscanf(ptr,"%s",fieldBuffer))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing burst delay higher range value.\n");
                      return false;
                  }
                  if (1 != sscanf(fieldBuffer,"%lf",&burstDelayHighRange))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing burst delay higher range value.\n");
                      return false;
                  }
                  ((Stream*)this)->SetBurstDelayHighRange(burstDelayHighRange);
                  
                  ptr += strlen(fieldBuffer);
                  cnt += strlen(fieldBuffer);
                  while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
                  
                  break;
              }
              
            case BURSTRANGE:
              {
                  if (1 != sscanf(ptr,"%s",fieldBuffer))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing burst delay lower range value.\n");
                      return false;
                  }
                  double burstRangeLowRange=0;
                  double burstRangeHighRange=0;
                  if (1 != sscanf(fieldBuffer,"%lf",&burstRangeLowRange))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing burst delay lower range value.\n");
                      return false;
                  }
                  ((Stream*)this)->SetBurstRangeLowRange(burstRangeLowRange);
                  ptr += strlen(fieldBuffer);
                  cnt += strlen(fieldBuffer);
                  while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
                  if (1 != sscanf(ptr,"%s",fieldBuffer))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing burst delay higher range value.\n");
                      return false;
                  }
                  if (1 != sscanf(fieldBuffer,"%lf",&burstRangeHighRange))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing burst delay higher range value.\n");
                      return false;
                  }
                  ((Stream*)this)->SetBurstRangeHighRange(burstRangeHighRange);
                  
                  ptr += strlen(fieldBuffer);
                  cnt += strlen(fieldBuffer);
                  while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
                  
                  break;
              }
            case RESPPROB:
              {
                  if (1 != sscanf(ptr,"%s",fieldBuffer))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing burst delay lower range value.\n");
                      return false;
                  }
                  int respProbLowRange=0;
                  int respProbHighRange=0;
                  if (1 != sscanf(fieldBuffer,"%i",&respProbLowRange))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing burst delay lower range value.\n");
                      return false;
                  }
                  ((Stream*)this)->SetRespProbLowRange(respProbLowRange);
                  ptr += strlen(fieldBuffer);
                  cnt += strlen(fieldBuffer);
                  while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
                  if (1 != sscanf(ptr,"%s",fieldBuffer))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing burst delay higher range value.\n");
                      return false;
                  }
                  if (1 != sscanf(fieldBuffer,"%i",&respProbHighRange))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing burst delay higher range value.\n");
                      return false;
                  }
                  ((Stream*)this)->SetRespProbHighRange(respProbHighRange);
                  
                  ptr += strlen(fieldBuffer);
                  cnt += strlen(fieldBuffer);
                  while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
                  
                  break;
              }
            case BURSTPAYLOADID:
              {
                  if (1 != sscanf(ptr,"%s",fieldBuffer))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing payload logic id.\n");
                      return false;
                  }
                  int burstPayloadID = 0;
                  if (1 != sscanf(fieldBuffer,"%d",&burstPayloadID))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing burstPayloadID.\n");
                      return false;
                  }
                  ((Stream*)this)->SetBurstPayloadID(burstPayloadID);
                  
                  ptr += strlen(fieldBuffer);
                  cnt += strlen(fieldBuffer);
                  while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
                  
                  break;
              }
              
            case TIMEOUTINTERVAL:
              {
                  if (1 != sscanf(ptr,"%s",fieldBuffer))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing timeout interval.\n");
                      return false;
                  }
                  double timeoutInterval = 0;
                  if (1 != sscanf(fieldBuffer,"%lf",&timeoutInterval))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing timeout interval.\n");
                      return false;
                  }
                  ((Stream*)this)->SetTimeoutInterval(timeoutInterval);
                  
                  ptr += strlen(fieldBuffer);
                  cnt += strlen(fieldBuffer);
                  while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
                  
                  break;
              }
              
            case INVALID_OPTION:
              DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: invalid stream option: %s\n",fieldBuffer);
              return false;
              
            } // end switch (streamOption)
            ((Stream*)this)->stream_option_mask |= streamOption;
            continue;
        }
        ptr += strlen(fieldBuffer);
        while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
        
        
        
    }  // end while ('\0' != *ptr)
    
    return true;
} // end BehaviorEvent::ParseStreamOptions

// Another egregious hack in the essense of time
bool BehaviorEvent::ParsePeriodicStreamOptions(const char* ptr,int& cnt)
{
    char fieldBuffer[SCRIPT_LINE_MAX];
    
    while(( ' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
    
    if (1 != sscanf(ptr,"%s",fieldBuffer))
    {
        DMSG(0,"BehaviorEvent::ParseOptions() Error: empty stream option type.\n");
        return false;
    }
    
    
    // Interate through any options
    ((Stream*)this)->stream_option_mask = 0;
    while ('\0' != *ptr)
    {
        // Read option label
        if (1 != sscanf(ptr,"%s",fieldBuffer))
        {
            DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: stream option parse error \n");
            return false;
        }
        // Is it a stream option?
        StreamOption streamOption = GetStreamOptionFromString(fieldBuffer);
        
        if (streamOption != INVALID_STREAM_OPTION)
        {
            if (1 != sscanf(ptr,"%s",fieldBuffer))
            {
                DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: stream option parse error.\n");
                return false;
            }
            ptr += strlen(fieldBuffer);
            cnt += strlen(fieldBuffer);
            while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
            
            switch (streamOption)
            {
            case BURSTDURATION:
              {
                  if (1 != sscanf(ptr,"%s",fieldBuffer))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing burst duration.\n");
                      return false;
                  }
                  double burstDuration = 0;
                  if (1 != sscanf(fieldBuffer,"%lf",&burstDuration))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing burst duration.\n");
                      return false;
                  }
                  //((Stream*)this)->SetBurstDuration(burstDuration);
                  ptr += strlen(fieldBuffer);
                  cnt += strlen(fieldBuffer);
                  while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
                  
                  break;
              }
            case BURSTCOUNT:
              {
                  if (1 != sscanf(ptr,"%s",fieldBuffer))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing burst count.\n");
                      return false;
                  }
                  int burstCount = 0;
                  if (1 != sscanf(fieldBuffer,"%d",&burstCount))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing burst count.\n");
                      return false;
                  }
                  //((Stream*)this)->SetBurstCount(burstCount);
                  
                  ptr += strlen(fieldBuffer);
                  cnt += strlen(fieldBuffer);
                  while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
                  
                  break;
              }
            case BURSTPRIORITY:
              {
                  if (1 != sscanf(ptr,"%s",fieldBuffer))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing burst priority.\n");
                      return false;
                  }
                  int burstPriority = 0;
                  if (1 != sscanf(fieldBuffer,"%d",&burstPriority))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing burst priority.\n");
                      return false;
                  }
                  //((Stream*)this)->SetBurstPriority(burstPriority);
                  
                  ptr += strlen(fieldBuffer);
                  cnt += strlen(fieldBuffer);
                  while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
                  
                  break;
              }
            case BURSTDELAY:
              {
                  if (1 != sscanf(ptr,"%s",fieldBuffer))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing burst delay lower range value.\n");
                      return false;
                  }
                  double burstDelayLowRange=0;
                  double burstDelayHighRange=0;
                  if (1 != sscanf(fieldBuffer,"%lf",&burstDelayLowRange))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing burst delay lower range value.\n");
                      return false;
                  }
                  //((Stream*)this)->SetBurstDelayLowRange(burstDelayLowRange);
                  ptr += strlen(fieldBuffer);
                  cnt += strlen(fieldBuffer);
                  while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
                  if (1 != sscanf(ptr,"%s",fieldBuffer))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing burst delay higher range value.\n");
                      return false;
                  }
                  if (1 != sscanf(fieldBuffer,"%lf",&burstDelayHighRange))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing burst delay higher range value.\n");
                      return false;
                  }
                  //((Stream*)this)->SetBurstDelayHighRange(burstDelayHighRange);
                  
                  ptr += strlen(fieldBuffer);
                  cnt += strlen(fieldBuffer);
                  while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
                  
                  break;
              }
              
            case BURSTRANGE:
              {
                  if (1 != sscanf(ptr,"%s",fieldBuffer))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing burst delay lower range value.\n");
                      return false;
                  }
                  double burstRangeLowRange=0;
                  double burstRangeHighRange=0;
                  if (1 != sscanf(fieldBuffer,"%lf",&burstRangeLowRange))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing burst delay lower range value.\n");
                      return false;
                  }
                  //((Stream*)this)->SetBurstRangeLowRange(burstRangeLowRange);
                  ptr += strlen(fieldBuffer);
                  cnt += strlen(fieldBuffer);
                  while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
                  if (1 != sscanf(ptr,"%s",fieldBuffer))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing burst delay higher range value.\n");
                      return false;
                  }
                  if (1 != sscanf(fieldBuffer,"%lf",&burstRangeHighRange))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing burst delay higher range value.\n");
                      return false;
                  }
                  //((Stream*)this)->SetBurstRangeHighRange(burstRangeHighRange);
                  
                  ptr += strlen(fieldBuffer);
                  cnt += strlen(fieldBuffer);
                  while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
                  
                  break;
              }
            case RESPPROB:
              {
                  if (1 != sscanf(ptr,"%s",fieldBuffer))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing burst delay lower range value.\n");
                      return false;
                  }
                  int respProbLowRange=0;
                  int respProbHighRange=0;
                  if (1 != sscanf(fieldBuffer,"%i",&respProbLowRange))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing burst delay lower range value.\n");
                      return false;
                  }
                  //((Stream*)this)->SetRespProbLowRange(respProbLowRange);
                  ptr += strlen(fieldBuffer);
                  cnt += strlen(fieldBuffer);
                  while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
                  if (1 != sscanf(ptr,"%s",fieldBuffer))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing burst delay higher range value.\n");
                      return false;
                  }
                  if (1 != sscanf(fieldBuffer,"%i",&respProbHighRange))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing burst delay higher range value.\n");
                      return false;
                  }
                  //((Stream*)this)->SetRespProbHighRange(respProbHighRange);
                  
                  ptr += strlen(fieldBuffer);
                  cnt += strlen(fieldBuffer);
                  while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
                  
                  break;
              }
            case BURSTPAYLOADID:
              {
                  if (1 != sscanf(ptr,"%s",fieldBuffer))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing payload logic id.\n");
                      return false;
                  }
                  int burstPayloadID = 0;
                  if (1 != sscanf(fieldBuffer,"%d",&burstPayloadID))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing burstPayloadID.\n");
                      return false;
                  }
                  //((Stream*)this)->SetBurstPayloadID(burstPayloadID);
                  
                  ptr += strlen(fieldBuffer);
                  cnt += strlen(fieldBuffer);
                  while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
                  
                  break;
              }
              
            case TIMEOUTINTERVAL:
              {
                  if (1 != sscanf(ptr,"%s",fieldBuffer))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing timeout interval.\n");
                      return false;
                  }
                  double timeoutInterval = 0;
                  if (1 != sscanf(fieldBuffer,"%lf",&timeoutInterval))
                  {
                      DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: missing timeout interval.\n");
                      return false;
                  }
                  //((Stream*)this)->SetTimeoutInterval(timeoutInterval);
                  
                  ptr += strlen(fieldBuffer);
                  cnt += strlen(fieldBuffer);
                  while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
                  
                  break;
              }
              
            case INVALID_OPTION:
              DMSG(0,"BehaviorEvent::ParseStreamOptions() Error: invalid stream option: %s\n",fieldBuffer);
              return false;
              
            } // end switch (streamOption)
            //((Stream*)this)->stream_option_mask |= streamOption;
            continue;
        }
        ptr += strlen(fieldBuffer);
        while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
        
        
        
    }  // end while ('\0' != *ptr)
    
    return true;
} // end BehaviorEvent::ParsePeriodicStreamOptions

const char* BehaviorEvent::ParsePattern(const char* ptr,char* fieldBuffer)
{
    //ljt why are we doing all this pattern string stuff?
    MgenPattern::Type patternType 
      = MgenPattern::GetTypeFromString(fieldBuffer); 
    
    // rebuild "our" patternString
    sprintf(patternString,"%s",fieldBuffer);
    strcat(patternString," [");
              
    // Point to beginning of pattern parameters
    const char* pptr = ptr;
    // Find end of pattern parameter set
    unsigned int nested = 1;
    while (nested)
    {
        ptr++;
        if ('[' == *ptr) 
        {
            nested++;   
        }
        else if (']' == *ptr)
        {
            nested--;
        }
        if ('\0' == *ptr)
        {
            DMSG(0, "BehaviorEvent::ParseOptions() Error: non-terminated <patternParams>\n");
            return NULL;
        }
    }
    if ((*pptr != '[') || !ptr)
    {
        DMSG(0, "BehaviorEvent::ParseOptions() Error: missing <patternParams>\n");
        return NULL;
    }
    strncpy(fieldBuffer, pptr+1, ptr - pptr - 1);
    
    // rebuild "our" patternString
    char patternStringTmp[SCRIPT_LINE_MAX] ="";
    strncpy(patternStringTmp,fieldBuffer,ptr - pptr - 1);
    strcat(patternStringTmp," ]\0");
    strncat(patternString,patternStringTmp,sizeof(patternStringTmp));
    // end rebuild
    
    fieldBuffer[ptr - pptr - 1] = '\0';	    
    
    if (!pattern.InitFromString(patternType, fieldBuffer,protocol))
    {
        DMSG(0, "BehaviorEvent::ParseOptions() Error: invalid <patternParams>\n");
        return NULL;
    }
    ptr++;
    
    // Load our pattern vector (only voip has multiple
    // flows so far.  Probably rewrite so everyone
    // uses a pattern vector ljt
    if (type == STREAM)
    {
        char *tmp = new char[SCRIPT_LINE_MAX];
        strncpy(tmp,patternString,sizeof(patternString));
        ((Stream*)this)->streamPatternVector.push_back(tmp);
    }
    
    
    // Get any multiple patterns
    while ((';' == *ptr) || ('\t' == *ptr)) ptr++;
    return ptr;
}

bool BehaviorEvent::ParseOptions(const char* string)
{
    const char* ptr = string;
    char fieldBuffer[SCRIPT_LINE_MAX];
    
    while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
    if (1 != sscanf(ptr,"%s",fieldBuffer))
    {
        DMSG(0,"BehaviorEvent::ParseOptions() Error: empty Event Type.\n");
        return false;
    }
    
    ptr += strlen(fieldBuffer);
    while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
    
    // skip any leading white space
    while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
    
    int cnt = 0;
    if (GetType() == PERIODIC)
    {
        if (!ParsePeriodicOptions(ptr,cnt))
        {
            return false;
        }
        ptr += cnt;
        (static_cast<Periodic*>(this))->SetBehaviorPattern(ptr);
        
        //Now move past the behavior type
        if (1 != sscanf(ptr,"%s",fieldBuffer))
        {
            DMSG(0,"BehaviorEvent::ParsePeriodicOptions() Error: No behavior type.\n");
            return false;
        }
        ptr += strlen(fieldBuffer);
        cnt += strlen(fieldBuffer);
        while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
    }
    
    
    cnt = 0;
    
    // each object type should have a "parse options" method
    // versus this evolutionary messiness
    
    if (GetType() == (INTERROGATIVE))
    {
        if (!ParseInterrogativeOptions(ptr,cnt))
          return false;
    }
    if ((GetType() == PERIODIC) &&
	((Periodic*)this)->GetBehaviorEventType() == INTERROGATIVE)
    {
      if (!ParsePeriodicInterrogativeOptions(ptr,cnt))
	return false;
    }
    
    if (GetType() == STREAM)
    {
        if (!ParseStreamOptions(ptr,cnt))
          return false;
        
        if (!((Stream*)this)->OptionIsSet(RESPPROB))
        {
            DMSG(0,"BehaviorEvent::ParseOptions() Error: Response probability option required!\n");
            return false;
        }
    }
    
    if ((GetType() == PERIODIC) && 
	((Periodic*)this)->GetBehaviorEventType() == STREAM)
    {
        if (!ParsePeriodicStreamOptions(ptr,cnt))
          return false;        
    }
    
    
    if (GetType() == PROCESSOR)
    {
        if (! ((Processor *)this)->ParseProcessorOptions (string))
          return false;
        else
          return true;  // no more options for Processor
    }
    
    
    if (GetType() == RAPREVENT)
    {
        if (!ParseRaprEventOptions(string,cnt))
          return false;
        else
          return true;  // no more options for raprEvents
    }
    ptr += cnt;
    while ((' ' == *ptr) || ('\t' == *ptr)) ptr++,cnt++;
    
    // Iterate through any options
    while ('\0' != *ptr)
    {
        Option option = INVALID_OPTION;
        // Read option label
        if (1 != sscanf(ptr, "%s", fieldBuffer))
        {
            DMSG(0, "BehaviorEvent::ParseOptions() Error: event option parse error\n");
            return false;   
        }
        // 0) Is it a logic ID?  
        LogicIdType logicId = GetLogicIdFromString(fieldBuffer);
        
        if (GetLogicIdFromString(fieldBuffer) != INVALID_LOGIC_ID)
        {
            if (1 != sscanf(ptr, "%s", fieldBuffer))
            {
                DMSG(0, "BehaviorEvent::ParseOptions() Error: event option parse error\n");
                return false;   
            }
            ptr += strlen(fieldBuffer);
            while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
            
            switch (logicId)
            {
            case SUCCESS:
              {
                  if (1 != sscanf(ptr,"%s",fieldBuffer))
                  {
                      DMSG(0,"BehaviorEvent::ParseOptions() Error: missing success <logicId>\n");
                      return false;
                  }
                  
                  if (1 != sscanf(fieldBuffer,"%i",&successLogicID))
                  {
                      DMSG(0,"BehaviorEvent::ParseOptions() Error: invalid success logicId\n");
                      return false;
                  }
                  logic_id_mask |= logicId;
                  
                  ptr += strlen(fieldBuffer);
                  while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;	    
                  
                  break;
              }
            case FAILURE:
              {
                  if (1 != sscanf(ptr,"%s",fieldBuffer))
                  {
                      DMSG(0,"BehaviorEvent::ParseOptions() Error: missing failure <logicId>\n");
                      return false;
                  }
                  
                  if (1 != sscanf(fieldBuffer,"%i",&failureLogicID))
                  {
                      DMSG(0,"BehaviorEvent::ParseOptions() Error: invalid failure logicId\n");
                      return false;
                  }
                  logic_id_mask |= logicId;
                  
                  ptr += strlen(fieldBuffer);
                  while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;	    
                  
                  
                  break;
              }
              
            case TIMEOUT:
              { 
                  if (1 != sscanf(ptr,"%s",fieldBuffer))
                  {
                      DMSG(0,"BehaviorEvent::ParseOptions() Error: missing timeout <logicId>\n");
                      return false;
                  }
                  
                  if (1 != sscanf(fieldBuffer,"%i",&timeoutLogicID))
                  {
                      DMSG(0,"BehaviorEvent::ParseOptions() Error: invalid timeout logicId\n");
                      return false;
                  }
                  logic_id_mask |= logicId;
                  
                  ptr += strlen(fieldBuffer);
                  while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;	    
                  
                  
                  break;
              }
              
            case PAYLOAD:
            case LOGICID:
              {
                  if (1 != sscanf(ptr,"%s",fieldBuffer))
                  {
                      DMSG(0,"BehaviorEvent::ParseOptions() Error: missing payload <logicId>\n");
                      return false;
                  }
                  
                  if (1 != sscanf(fieldBuffer,"%i",&payloadLogicID))
                  {
                      DMSG(0,"BehaviorEvent::ParseOptions() Error: invalid payload logicId\n");
                      return false;
                  }
                  logic_id_mask |= logicId;
                  
                  ptr += strlen(fieldBuffer);
                  while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;	    
                  
                  
                  break;
              }
              
            case ORIGIN:
              {
                  if (1 != sscanf(ptr,"%s",fieldBuffer))
                  {
                      DMSG(0,"BehaviorEvent::ParseOptions() Error: missing payload <logicId>\n");
                      return false;
                  }
                  
                  if (1 != sscanf(fieldBuffer,"%u",&origin))
                  {
                      DMSG(0,"BehaviorEvent::ParseOptions() Error: invalid payload logicId\n");
                      return false;
                  }
                  ptr += strlen(fieldBuffer);
                  while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
                  break;
              }
            default:
              break;
            } // end switch (logicId)
            
            //break;
            continue;
        }
        
        // Point to next field, skipping any white space
        ptr += strlen(fieldBuffer);
        while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
        
        // 1) Is it an implicitly labeled "PROTOCOL" type?
        if (INVALID_PROTOCOL != GetProtocolFromString(fieldBuffer)) 
          option = PROTOCOL;
        // 2) Is in an implicitly labeled "PATTERN" spec?
        else if (MgenPattern::INVALID_TYPE != MgenPattern::GetTypeFromString(fieldBuffer))
          option = PATTERN;
        else
          option = GetOptionFromString(fieldBuffer);
        
        
        switch (option)
        {
        case PROTOCOL:  // protocol type (UDP or TCP)
          {
              protocol = GetProtocolFromString(fieldBuffer);
              if (INVALID_PROTOCOL == protocol)
              {
                  DMSG(0, "BehaviorEvent::ParseOptions() Error: invalid protocol type\n");
                  return false;    
              }
              break;
          }
        case DST: // destination address/port
          {
              if (1 != sscanf(ptr, "%s", fieldBuffer))
              {
                  DMSG(0, "BehaviorEvent::ParseOptions() Error: missing <dstAddr>\n");
                  return false;   
              }
              char* portPtr= strchr(fieldBuffer, '/');
              if (portPtr)
              {
                  *portPtr++ = '\0';
              }
              else
              {
                  DMSG(0, "BehaviorEvent::ParseOptions() Error: missing <dstPort>\n");
                  return false;
              } 
              if (!dstAddr.ResolveFromString(fieldBuffer))
              {
                  DMSG(0, "BehaviorEvent::ParseOptions() Error: invalid <dstAddr>\n");
                  return false;   
              }
              int dstPort;
              if (1 != sscanf(portPtr, "%d", &dstPort))
              {
                  DMSG(0, "BehaviorEvent::ParseOptions() Error: invalid <dstPort>\n");
                  return false; 
              }
              if ((dstPort < 1) || (dstPort > 65535))
              {
                  DMSG(0, "BehaviorEvent::ParseOptions() Error: invalid <dstPort>\n");
                  return false; 
              }
              dstAddr.SetPort(dstPort);
              // Set ptr to next field, skipping any white space
              ptr += strlen(fieldBuffer) + strlen(portPtr) + 1;
              while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
              break;
              
          }
          
        case SRC:       // source port
          {
              if (1 != sscanf(ptr, "%s", fieldBuffer))
              {
                  DMSG(0, "BehaviorEvent::ParseOptions() Error: missing <srcPort>\n");
                  return false;   
              }
              int inSrcPort;
              if (1 != sscanf(fieldBuffer, "%d", &inSrcPort))
              {
                  DMSG(0, "BehaviorEvent::ParseOptions() Error: invalid <srcPort>\n");
                  return false;
              }
              
              if (inSrcPort < 0 || inSrcPort > 65535)
              {
                  DMSG(0, "BehaviorEvent::ParseOptions() Error: invalid <srcPort>\n");
                  return false; 
              }
              srcPort = inSrcPort;
              srcAddr.SetPort(srcPort);
              // Set ptr to next field, skipping any white space
              ptr += strlen(fieldBuffer);
              while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
              break;
          }                
          
        case PATTERN:
          {
              const char *tmpPtr = const_cast<char*>(ptr);
              ptr = ParsePattern(tmpPtr,fieldBuffer);
              if (ptr == NULL) 
                return false;
              // Set ptr to next field, skipping any white space
              while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
              
              break;
          }  
        case TOS:       // tos spec
          {
              if (1 != sscanf(ptr, "%s", fieldBuffer))
              {
                  DMSG(0, "BehaviorEvent::ParseOptions() Error: missing <tosValue>\n");
                  return false;   
              }
              int tosValue;
              if (1 != sscanf(fieldBuffer, "%i", &tosValue))
              {
                  DMSG(0, "BehaviorEvent::ParseOptions() Error: invalid <tosValue>\n");
                  return false;
              }
              if ((tosValue < 0) || (tosValue > 255))
              {
                  DMSG(0, "BehaviorEvent::ParseOptions() Error: invalid <tosValue>\n");
                  return false;
              }
              tos = tosValue;
              // Set ptr to next field, skipping any white space
              ptr += strlen(fieldBuffer);
              while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
              break;
          }
        case RSVP:
          // (TBD)
          DMSG(0, "BehaviorEvent::ParseOptions() Error: RSVP option not yet supported\n");
          return false;
          break;
          
        case INTERFACE:  // multicast interface name
          if (1 != sscanf(ptr, "%s", fieldBuffer))
          {
              DMSG(0, "BehaviorEvent::ParseOptions() Error: missing <interfaceName>\n");
              return false;   
          }
          strncpy(interface_name, fieldBuffer, 15);
          interface_name[15] = '\0';
          // Set ptr to next field, skipping any white space
          ptr += strlen(fieldBuffer);
          while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
          break;
          
        case TTL:     // multicast ttl value
          {
              if (1 != sscanf(ptr, "%s", fieldBuffer))
              {
                  DMSG(0, "BehaviorEvent::ParseOptions() Error: finding <ttlValue>\n");
                  return false;   
              }
              unsigned int ttlValue;
              if (1 != sscanf(fieldBuffer, "%u", &ttlValue))
              {
                  DMSG(0, "BehaviorEvent::ParseOptions() Error: invalid <ttlValue>\n");
                  return false;
              }
              if (ttlValue > 255)
              {
                  DMSG(0, "BehaviorEvent::ParseOptions() Error: invalid <ttlValue>\n");
                  return false;
              }
              ttl = ttlValue;
              // Set ptr to next field, skipping any white space
              ptr += strlen(fieldBuffer);
              while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
              break;
          }
        case SEQUENCE:  // sequence number initialization
          {
              // ljt do we want to allow this for rapr?
              
              if (1 != sscanf(ptr, "%s", fieldBuffer))
              {
                  DMSG(0, "BehaviorEvent::ParseOptions() Error: finding <seqNum>\n");
                  return false;   
              }
              unsigned long seqTemp;
              if (1 != sscanf(fieldBuffer, "%lu", &seqTemp))
              {
                  DMSG(0, "BehaviorEvent::ParseOptions() Error: invalid <seqNum>\n");
                  return false;
              }
              sequence = seqTemp;
              // Set ptr to next field, skipping any white space
              ptr += strlen(fieldBuffer);
              while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
              break;
          }	  	  
        case LABEL:     // IPv6 traffic class/ flow label
          {
              DMSG(0,"BehaviorEvent::ParseOptions() Error: IPv6 not suppored.\n");
              return false;
              
              // Will need this if we decide to support IPv6
              if (1 != sscanf(ptr, "%s", fieldBuffer))
              {
                  DMSG(0, "BehaviorEvent::ParseOptions() Error: missing <flowLabel>\n");
                  return false;   
              }
              int flowLabel; 
              if (1 != sscanf(fieldBuffer, "%i", &flowLabel))
              {
                  DMSG(0, "BehaviorEvent::ParseOptions() Error: invalid <flowLabel>\n");
                  return false;
              }
              // (TBD) check validity
              // ljt uncomment me for ipv6 flow_label = flowLabel; 
              // Set ptr to next field, skipping any white space
              ptr += strlen(fieldBuffer);
              while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
              break;
          }
          
        case TXBUFFER:
          {
              if (1 != sscanf(ptr, "%s", fieldBuffer))
              {
                  DMSG(0, "BehaviorEvent::ParseOptions() Error: missing <socket Tx  buffer size>\n");
                  return false;   
              }
              unsigned int txBuffer; 
              
              if (1 != sscanf(fieldBuffer, "%u", &txBuffer))
              {
                  DMSG(0, "BehaviorEvent::ParseOptions() Error: invalid <socket Tx buffer size>\n");
                  return false;
              }
              
              // (TBD) check validity
              tx_buffer_size = txBuffer; 
              // Set ptr to next field, skipping any white space
              ptr += strlen(fieldBuffer);
              while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
              
              break;
              
          }
        case SEED:
          {
              if (1 != sscanf(ptr,"%s",fieldBuffer))
              {
                  DMSG(0,"BehaviorEvent::ParseOptions() Error: finding <seedValue>\n");
                  return false;
              }
              unsigned int seedValue = 0;
              if (1 != sscanf(fieldBuffer,"%u",&seedValue))
              {
                  DMSG(0,"BehaviorEvent::ParseOptions() Error: invalid <seedValue>\n");
                  return false;
              }
              SetSeed(seedValue);
              if (GetEventSource() == BehaviorEvent::RTI_EVENT)
              {
                  // We overrode the rti seed, reset 		
                  rapr.GetNumberGenerator().DecrementRtiSeed();
              }
              // Set ptr to next field, skipping any white space
              ptr += strlen(fieldBuffer);
              while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
              break;
          }	  
        case RAPRFLOWID:
          {
              if (1 != sscanf(ptr,"%s",fieldBuffer))
              {
                  DMSG(0,"BehaviorEvent::ParseOptions() Error: finding <raprFlowID>\n");
                  return false;
              }
              if (1 != sscanf(fieldBuffer,"%d",&raprFlowID))
              {
                  DMSG(0,"BehaviorEvent::ParseOptions() Error: invalid <raprFlowID>\n");
                  return false;
              }
              ptr += strlen(fieldBuffer);
              while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
              break;
          }
        case COUNT:
          {
              if (1 != sscanf(ptr, "%s", fieldBuffer))
              {
                  DMSG(0, "BehaviorEvent::ParseOptions() Error: missing <count>\n");
                  return false;   
              }
              unsigned int count; 
              
              if (1 != sscanf(fieldBuffer, "%u", &count))
              {
                  DMSG(0, "BehaviorEvent::ParseOptions() Error: invalid <count>\n");
                  return false;
              }
              
              mgen_msg_count = count; 

              // Set ptr to next field, skipping any white space
              ptr += strlen(fieldBuffer);
              while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
              
              
              break;
          }
        case INVALID_OPTION:
          DMSG(0, "BehaviorEvent::ParseOptions() Error: invalid event option: \"%s\"\n",
               fieldBuffer);
          return false;
        }  // end switch (option)
        option_mask |= option;
    }  // end while ('\0' != *ptr)

    if (!OptionIsSet(PROTOCOL))
    {
        const char *dictionaryValue;
        if ((dictionaryValue = LookupDictionaryEntry("%SystemDefaults:protocol%")))
        {
            if (INVALID_PROTOCOL != GetProtocolFromString(dictionaryValue))
            {
                option_mask |= PROTOCOL;
                protocol = GetProtocolFromString(dictionaryValue);
            }
        }
    }
    // add validation for logic id's etc??

    // Validate option_mask 
    unsigned int mask = option_mask & PATTERN;

    if (mask != PATTERN)
    {
        // See if we have a default dictionary entry
        // othersize, use our internal default
        const char *defaultPattern = NULL;
        MgenPattern::Type patternType;
        char defaultPatternType[SCRIPT_LINE_MAX+1] = "\0";

        if ((defaultPattern = LookupDictionaryEntry("%SystemDefaults:pattern%")))
        {
            // read pattern label
            if (1 != sscanf(defaultPattern,"%s",defaultPatternType))
            {
                DMSG(0,"BehaviorEvent::ParseOptions() Error: invalid PATTERN in the default dictionary\n");    
                return false;
            }
            patternType = MgenPattern::GetTypeFromString(defaultPatternType);
            defaultPattern += strlen(defaultPatternType);
            while ((' ' == *defaultPattern) || ('\t' == *defaultPattern)) defaultPattern++;
        }
        else
        {
            patternType = MgenPattern::GetTypeFromString("PERIODIC");
            sprintf(defaultPatternType,"PERIODIC");            
            defaultPattern = "[1 1024]";
        }
        ParsePattern(defaultPattern,defaultPatternType);
        option_mask |= PATTERN;
    }
    // Validate option_mask 
    mask = option_mask & ON_REQUIRED_OPTIONS;
    if (mask != ON_REQUIRED_OPTIONS)
    {
        DMSG(0, "BehaviorEvent::ParseOptions() Error: incomplete option list:\n"
             "          (missing");
        // Incomplete "ON" event.  What are we missing?
        // mask = ON_REQUIRED_OPTIONS - option_mask
        mask = ON_REQUIRED_OPTIONS & ~option_mask;
        unsigned int flag = 0x01;
        while (mask)
        {
            if (0 != (flag & mask))
            {
                const char* optionString = GetStringFromOption((Option)flag);
                mask &= ~flag;   
                DMSG(0, ": %s ", optionString);
            }   
            flag <<= 1;
        }
        DMSG(0, ")\n");
        return false;   
    }
    
    // Now that we finally have the pattern, see if we
    // need to ROUGHLY estimate an end time for COUNT events
    if (!behaviorEndTime && OptionIsSet(COUNT))
    {
        behaviorEndTime = (pattern.GetIntervalAve()) * mgen_msg_count;
        if (behaviorStartTime > 0)
          behaviorEndTime = mgen_msg_count + behaviorStartTime;
    }
    
    return true;
    
} // end BehaviorEvent::ParseOptions


void BehaviorEvent::ActivateEndTimer(ProtoTimer& endBehaviorTimer)
{
    double offsetTime = rapr.GetOffset() > 0.0 ? rapr.GetOffset() : 0.0;
    if (duration)
    {
        endBehaviorTimer.SetInterval(behaviorEndTime);
    }
    else
    {
        if (offsetTime < behaviorStartTime)
        {
            endBehaviorTimer.SetInterval(behaviorEndTime - behaviorStartTime);
        }
        else
        {
            endBehaviorTimer.SetInterval(behaviorEndTime - offsetTime);
        }
    }
    endBehaviorTimer.SetRepeat(0);
    
    timer_mgr.ActivateTimer(endBehaviorTimer);
    
} // end BehaviorEvent::ActivateEndTimer

const StringMapper RaprEvent::RAPREVENT_TYPE_LIST[] = 
  {
    {"STOP",             STOP}, 
    {"RAPRFLOWID",       RAPRFLOWID},
    {"DICTIONARY_ENTRY", DICTIONARY_ENTRY},
    {"LOGICTABLE_FILE",  LOGICTABLE_FILE},
    {"DICTIONARY_FILE",  DICTIONARY_FILE},
    {"CHANGE_STATE",     CHANGE_STATE},
    {"FAULT",            FAULT},
    {"CLEAR",            CLEAR},
    {"CHANGE_UBI_STATE", CHANGE_UBI_STATE},
    {"LOGICID",	       LOGICID},
    {"INPUT",            INPUT},
    {"XXXX",             INVALID_TYPE}   
  };

RaprEvent::EventType RaprEvent::GetTypeFromString(const char* string)
{
    // Make comparison case-insensitive
    char upperString[16];
    unsigned int len = strlen(string);
    len = len < 15 ? len : 15;
    unsigned int i;
    for (i =0 ; i < len; i++)
      upperString[i] = toupper(string[i]);
    upperString[i] = '\0';
    const StringMapper* m = RAPREVENT_TYPE_LIST;
    unsigned matchCount = 0;
    EventType eventType = INVALID_TYPE;
    while (INVALID_TYPE != (*m).key)
    {
        if (!strncmp(upperString, (*m).string, len))
        {        
            matchCount++;
            eventType = ((EventType)((*m).key));
        }
        m++;
    }
    if (matchCount > 1)
    {
        DMSG(0, "BehaviorEvent::GetTypeFromString() Error: ambiguous event type\n");
        return INVALID_TYPE;   
    }
    else
    {
        return eventType;
    }    
}  // end RaprEvent::GetTypeFromString()


RaprEvent::RaprEvent(ProtoTimerMgr& timerMgr,
                     unsigned long ubi,
                     Rapr& theRapr)
  : BehaviorEvent(timerMgr,ubi,theRapr)
{
    type = RAPREVENT;
    raprEventType = INVALID_TYPE;
    overrideStartTime = true;
    faultType = INVALID_FAULT;
    name[0] = '\0';
    value[0] = '\0';
} // end RaprEvent::RaprEvent()

RaprEvent::~RaprEvent()
{
    
} // end RaprEvent::RaprEvent()
bool RaprEvent::OnStartUp()
{
    char msgBuffer [512];
    msgBuffer[0] = '\0';
    switch (raprEventType)
    {
    case STOP:
      {
          sprintf(msgBuffer,"type>application action>stop.");
          rapr.LogEvent("RAPR",msgBuffer);
          
          rapr.Stop();
      }
      break;
      
    case FAULT:
      {
          switch (faultType) 
          {
          case STOP_RAPR:
            sprintf(msgBuffer,"type>application action>fault eventSource>%s type>STOP_RAPR.",GetStringFromEventSource(eventSource));
            rapr.Stop();
            break;
          case STOP_MGEN:
            sprintf(msgBuffer,"type>application action>fault eventSource>%s type>STOP_MGEN.",GetStringFromEventSource(eventSource));	
            rapr.StopMgen();
            break;
          case STOP_EVENTS:
            sprintf(msgBuffer,"type>application action>fault eventSource>%s type>STOP_EVENTS.",GetStringFromEventSource(eventSource));
            rapr.StopEvents();
            break;
          case INVALID_FAULT:
            sprintf(msgBuffer,"type>application action>fault eventSource>%s type>INVALID_FAULT.",GetStringFromEventSource(eventSource));
            DMSG(0,"RaprEvent::OnStartUp Error: Invalid fault type defined.\n");
            break;
          default:
            sprintf(msgBuffer,"type>application action>fault eventSource>%s type>INVALID_FAULT.",GetStringFromEventSource(eventSource));
            DMSG(0,"RaprEvent::OnStartUp Error: Invalid fault type defined.\n");
          }
          rapr.LogEvent("RAPR",msgBuffer);
          
      }
      break;
    case CLEAR:
      {
          
	sprintf(msgBuffer,"type>RaprEvent action>clearing eventSource>%s objectType>%s ubi>%lu",GetStringFromEventSource(eventSource),GetStringFromEventType(clearEventType),ubi);
          rapr.LogEvent("RAPR",msgBuffer);
          
          // Make sure the be is not me
          while (BehaviorEvent* theEvent = rapr.GetBehaviorEventList().FindBEByEventType(clearEventType,this))
          {
              DMSG(2,"I found it >%s ubi>%d\n",GetStringFromEventType(theEvent->GetType()),theEvent->GetUBI());
              // don't delete ourself!
              if (theEvent != this) {
                  theEvent->Stop();
                  rapr.GetBehaviorEventList().Remove(theEvent);
                  delete theEvent;
              }
          }	
          
          // If we don't have any events left,
          // kill the next behavior timer.
          if (rapr.GetBehaviorEventList().IsEmpty() || // shouldn't be
              ((rapr.GetBehaviorEventList().Head() == this) &&
               (rapr.GetBehaviorEventList().Tail() == this)))
          {
              if (rapr.GetBehaviorEventTimer().IsActive()) rapr.GetBehaviorEventTimer().Deactivate();
          }
          
          
          break;
      }
    case RAPRFLOWID:
      {
          BehaviorEvent* theEvent = rapr.GetBehaviorEventList().FindBEByRaprFlowID(GetRaprFlowID());
          if (theEvent)
          {
	    sprintf(msgBuffer,"type>RaprEvent action>stopping ubi>%lu eventSource>%s raprFlowID>%d",ubi,GetStringFromEventSource(eventSource),GetRaprFlowID());
              rapr.LogEvent("RAPR",msgBuffer);
              
              theEvent->Stop(); 
          }
          else 
          {

	    sprintf(msgBuffer,"type>Error action>stopping eventSource>%s raprFlowID>%d info>\"No behavior event found for rapr flow id.\"",GetStringFromEventSource(eventSource),GetRaprFlowID());
              rapr.LogEvent("RAPR",msgBuffer);

              DMSG(0,"RaprEvent::OnStartUp() Error: invalid rapr flow id!\n");
              return false;
          }
      }
      break;
    case DICTIONARY_ENTRY:
      {
	sprintf(msgBuffer,"type>RaprEvent action>resetting_dictionary_entry eventSource>%s name>%s value>%s",GetStringFromEventSource(eventSource),name,value);
          rapr.LogEvent("RAPR",msgBuffer);
          
          rapr.LogicTable().ResetDictionaryValue(name,value);
          break;
      }
    case LOGICTABLE_FILE:
      {
	sprintf(msgBuffer,"type>RaprEvent action>loading_logictable eventSource>%s name>%s",GetStringFromEventSource(eventSource),name);
          rapr.LogEvent("RAPR",msgBuffer);
          
          if (!rapr.LogicTable().LoadFile(name))
          {
	    sprintf(msgBuffer,"type>Error action>loading_logictable eventSource>%s name>%s",GetStringFromEventSource(eventSource),name);
              rapr.LogEvent("RAPR",msgBuffer);
              return false;
          }
          break;
      }
    case DICTIONARY_FILE:
      {
	sprintf(msgBuffer,"type>RaprEvent action>loading_dictionary eventSource>%s name>%s",GetStringFromEventSource(eventSource),name);
          rapr.LogEvent("RAPR",msgBuffer);
          
          if (!rapr.LogicTable().LoadDictionary(name))
          {
	    sprintf(msgBuffer,"type>Error action>loading_dictionary eventSource>%s name>%s",GetStringFromEventSource(eventSource),name);
              rapr.LogEvent("RAPR",msgBuffer);
              return false;
          }
          // Load HOSTID into the dictionary
          char *name = new char[6];
          strcpy(name,"HOSTID");
          name[6] = '\0';
          char *val = new char[20];
          sprintf(val,"%d",rapr.GetNumberGenerator().GetHostID());
          rapr.LogicTable().ResetDictionaryValue(name,val);
          delete [] name;
          delete [] val;
          break;
      }
    case CHANGE_STATE:
      {
	sprintf(msgBuffer,"type>RaprEvent action>change_state eventSource>%s stateValue>%s",GetStringFromEventSource(eventSource),name);
          rapr.LogEvent("RAPR",msgBuffer);
          if (!rapr.LogicTable().SetStateFromString(name))
          {
	    sprintf(msgBuffer,"type>Error action>change_state eventSource>%s stateValue>%s",GetStringFromEventSource(eventSource),name);
              rapr.LogEvent("RAPR",msgBuffer);
              return false;
          }
          break;
      }
    case CHANGE_UBI_STATE:
      {
	sprintf(msgBuffer,"type>RaprEvent action>change_ubi_state eventSource>%s ubi>%s stateValue>%s",GetStringFromEventSource(eventSource),name,value);
          rapr.LogEvent("RAPR",msgBuffer);
          if (!rapr.LogicTable().SetUBIState(name,value))
          {
	    sprintf(msgBuffer,"type>Error action>change_ubi_state eventSource>%s ubi>%s stateValue>%s",GetStringFromEventSource(eventSource),name,value);
              rapr.LogEvent("RAPR",msgBuffer);
              return false;
          }
          break;
      }
      
    case LOGICID:
      {
	sprintf(msgBuffer,"type>RaprEvent action>logic_id eventSource>%s logicID>%s",GetStringFromEventSource(eventSource),name);
          rapr.LogEvent("RAPR",msgBuffer);
          int id = 0;
          id = atoi(name);
          if (id == 0) {
	    sprintf(msgBuffer,"type>Error action>logic_id eventSource>%s logicID>%s",GetStringFromEventSource(eventSource),name);
              rapr.LogEvent("RAPR",msgBuffer);
              return false;
              break;
          }
          if (!rapr.LogicTable().DoLogicID(id,(BehaviorEvent*)this))
          {
	    sprintf(msgBuffer,"type>Error action>logic_id eventSource>%s logicID>%d info>\"logicID lookup failure\"",GetStringFromEventSource(eventSource),id);
              rapr.LogEvent("RAPR",msgBuffer);
              return false;
          }
          break;
      }
    case RaprEvent::INPUT:
      {
	sprintf(msgBuffer,"type>RaprEvent action>loading_input_script eventSource>%s value>%s",GetStringFromEventSource(eventSource),name);
          rapr.LogEvent("RAPR",msgBuffer);
          
          if (!rapr.ParseScript(name))
          {
	    sprintf(msgBuffer,"type>Error action>loading_input_script eventSource>%s value>%s",GetStringFromEventSource(eventSource),name);
              rapr.LogEvent("RAPR",msgBuffer);
              return false;
          }
          
          break;
      }
    case INVALID_TYPE:
      DMSG(0,"RaprEvent::OnStartUp() Error: invalid rapr event type!\n");
      return false;
      break;
    default:
      {
          DMSG(0,"RaprEvent::OnStartUp() Error: invalid rapr event type!\n");
          return false;
      }
    }
    
    //ljt    ClearState();

    return true;
} // end RaprEvent::OnStartUp
bool RaprEvent::Stop()
{
    // No implementation
    return true;
}
bool RaprEvent::ProcessEvent(const MgenMsg& theMsg)
{
    // No implementation 
    return true;
} 
bool RaprEvent::OnBehaviorTimeout(ProtoTimer &theTimer)
{
    // No implementation
    return true;
}
void RaprEvent::ClearState()
{
    BehaviorEvent::ClearState();
}

MgenMessage::MgenMessage(ProtoTimerMgr& timerMgr,
                         unsigned long ubi,
                         Rapr& theRapr)
  : BehaviorEvent(timerMgr,ubi,theRapr)
{
} // end MgenMessage::MgenMessage()

MgenMessage::~MgenMessage()
{
} // end MgenMessage::~MgenMessage()

Declarative::Declarative(ProtoTimerMgr& timerMgr,
                         unsigned long ubi,
                         Rapr& theRapr)
  : BehaviorEvent(timerMgr,ubi,theRapr)
{
    type = DECLARATIVE;

}

Declarative::~Declarative()
{
    if (endBehaviorTimer.IsActive()) endBehaviorTimer.Deactivate();
    
}

bool Declarative::OnStartUp()
{
    char buffer[8192];
    char tmpBuffer[512];
    stopped = false;

    // Default end time if not specified
    // (-1 means no behavior end)
    if (behaviorEndTime == 0)
    {
        // No end time and no count option - set it 
        if (!OptionIsSet(COUNT))
        {
            mgen_msg_count = 1;
	    Option option = GetOptionFromString("COUNT");
	    option_mask |= option;
        }
        behaviorEndTime = 0.99;
        
        // See if stream duration is overridden in the dictionary
        const char *dictionaryValue;
        double defaultStreamDuration = 0;
        if ((dictionaryValue = LookupDictionaryEntry("%SystemDefaults:streamduration%")))
        {
            defaultStreamDuration = atof(dictionaryValue);
        }
        if (defaultStreamDuration)
          behaviorEndTime = defaultStreamDuration;            
    }
    if (GetInterface()) 
    {
        ProtoSocket tmpSocket(ProtoSocket::UDP);
        tmpSocket.GetInterfaceAddress(GetInterface(),ProtoAddress::IPv4,srcAddr);
    } 
    else 
    {
        // this should be (local machine's srcAddr) 
        // a global value resolved once at startup
        // (in not a config file setting) since
        // resolveLocalAddress can take a LONG time
        // to timeout
        srcAddr.ResolveLocalAddress();
    }
    
    SetFlowID(rapr.GetNumberGenerator().GetFlowID(&srcAddr,&dstAddr));
    
    sprintf(buffer,"ON %d %s SRC %hu DST %s/%hu %s ",GetFlowID(),GetStringFromProtocol(protocol),srcPort,dstAddr.GetHostString(),dstAddr.GetPort(),patternString);
    RaprPayload raprPayload;
    
    if (GetPayloadLogicID() || GetForeignUBI() || GetSeed())
    {
        raprPayload.SetForeignUBI(GetForeignUBI());
        raprPayload.SetLogicID(GetPayloadLogicID());
        raprPayload.SetOrigin(GetOriginUBI());
        raprPayload.SetSeed(prng->GetRand());
        char *payld = raprPayload.GetHex();
        sprintf(tmpBuffer,"DATA [%s]", payld);
        delete [] payld;
        strcat(buffer,tmpBuffer);
    }
    
    if (tos)
    {
        sprintf(tmpBuffer," TOS %d",tos);
        strcat(buffer,tmpBuffer);
    }
    if (tx_buffer_size)
    {
        sprintf(tmpBuffer," TXBUFFER %d",tx_buffer_size);
        strcat(buffer,tmpBuffer);
    }
    if (mgen_msg_count)
    {
        sprintf(tmpBuffer," COUNT %d",mgen_msg_count);
        strcat(buffer,tmpBuffer);
	// Make sure option mask is set as we are 
	// relying on mgen to stop the flow
	Option option = GetOptionFromString("COUNT");
	option_mask |= option;

    }
    
    if (ttl) 
    {
        sprintf(tmpBuffer," TTL %d",ttl);
        strcat(buffer,tmpBuffer);
    }
    
    if (GetInterface())
    {      
        
        sprintf(tmpBuffer," INTERFACE %s",GetInterface());
        strcat(buffer,tmpBuffer);
    }
    
    rapr.SendMgenCommand("event",buffer);
    
    // ljt sequence?
    
    // log event
    char msgBuffer[8192];
    
    sprintf(msgBuffer,"type>Declarative action>start ubi>%lu eventSource>%s",ubi,GetStringFromEventSource(eventSource));
    
    if (rapr.GetVerbosity()) 
    {
        
        if (foreignUbi)
        {
            sprintf(tmpBuffer," foreignUBI>%lu",foreignUbi);
            strcat(msgBuffer,tmpBuffer);
        }
        if (successLogicID)
        {
            sprintf(tmpBuffer," successLogicID>%d",successLogicID);
            strcat(msgBuffer,tmpBuffer);
        }
        if (failureLogicID)
        {
            sprintf(tmpBuffer," failureLogicID>%d",failureLogicID);
            strcat(msgBuffer,tmpBuffer);
        }
        if (timeoutLogicID)
        {
            sprintf(tmpBuffer," timeoutLogicID>%d",timeoutLogicID);
            strcat(msgBuffer,tmpBuffer);
        }
        if (payloadLogicID)
        {
            sprintf(tmpBuffer," payloadLogicID>%d",payloadLogicID);
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
        
        if (raprPayload.GetSeed())
        {
            sprintf(tmpBuffer," payloadSeed>%u",raprPayload.GetSeed());
            strcat(msgBuffer,tmpBuffer);
        }
        
    }
    sprintf(tmpBuffer," mgenCmd>\"%s\"",buffer);
    strcat(msgBuffer,tmpBuffer);
    
    rapr.LogEvent("RAPR",msgBuffer);
    
    if (behaviorEndTime != -1.0)
    {
        endBehaviorTimer.SetListener(this,&Declarative::OnBehaviorTimeout);
        ActivateEndTimer(endBehaviorTimer);
    }
    //}
    //else 
    //{
    //OnBehaviorTimeout(endBehaviorTimer);
    //}
    
    return true;
} // end Declarative::OnStartup
bool Declarative::Stop()
{
    char msgBuffer[512];

    // Turn off flow  
    if (GetFlowID())
    {
      char buffer[8192];
      // We rely on mgen to stop the flow after COUNT
      // messages have been sent
      if (!stopped && !OptionIsSet(COUNT))
	{
	  sprintf(buffer,"OFF %d",GetFlowID());
	  rapr.SendMgenCommand("event",buffer);  
	  stopped = true;
	}
        // log event
        sprintf(msgBuffer,"type>Declarative action>stop ubi>%lu raprFlowId>%d mgenCmd>\"%s\"",ubi,raprFlowID,buffer);
        strcat (msgBuffer,buffer);
        rapr.LogEvent("RAPR",msgBuffer);
        
        // ljt deal w/ stopping events that haven't started yet...
        // can't just stop the timer cuz they'll still be in the
        // queue?? was that it?  don't remember why I took this out
        // if I did
        
        // if (startBehaviorTimer.IsActive()) startBehaviorTimer.Deactivate();
        if (endBehaviorTimer.IsActive()) endBehaviorTimer.Deactivate();
        
        ClearState();
    } 
    else
      {
        // log event
        sprintf(msgBuffer,"type>Declarative action>stop ubi>%lu raprFlowId>%d info>\"Flow (%d) already stopped\"",ubi,raprFlowID,GetFlowID());
        rapr.LogEvent("RAPR",msgBuffer);

      }


    return true;
    
} // end Declarative::Stop

bool Declarative::ProcessEvent(const MgenMsg& theMsg)
{
    // Declaratives shouldn't get any response!
    return false;
} // end Declarative::ProcessEvent

void Declarative::ClearState()
{
    DMSG(3,"Declarative::ClearState() \n");
    
    if (endBehaviorTimer.IsActive()) endBehaviorTimer.Deactivate();
    BehaviorEvent::ClearState();
    
    
} // end Declarative::ClearState

bool Declarative::OnBehaviorTimeout(ProtoTimer& /*theTimer*/)
{
    
    char buffer[8192];

    DMSG(3,"Declarative::OnBehaviorTimeout() ubi> %lu start> %.2f end> %.2f\n",ubi,behaviorStartTime,behaviorEndTime);

    // ljt -> do we want to do this?     
    // MgenTransport* transport = rapr.GetMgen().GetFlowList().FindFlowById(GetFlowID())->GetFlowTransport();
    // if (GetFlowID() && !transport->TransmittingFlow(GetFlowID())) 
    
    // If flowID is 0 - the flow was already turned off
    // by a mgen offevent (tcp only)
    char msgBuffer[512],tmpBuffer[512];

    if (GetFlowID()) 
    {
      // We rely on mgen to stop the flow id after
      // count messages are sent
      if (!stopped && !OptionIsSet(COUNT))
	{
	  sprintf(buffer,"0.001 OFF %d", GetFlowID());
	  rapr.SendMgenCommand("event",buffer);  
	  stopped = true;
	  DMSG(1,"\nDeclarative::OnBehaviorTImeout() Sending %s\n",buffer);
	  sprintf(msgBuffer,"type>Declarative action>timeout ubi>%lu mgenCmd>\"%s\"",ubi,buffer);

	}
      DMSG(1,"\nDeclarative::OnBehaviorTimeout() flowId>%d \n",GetFlowID());
      sprintf(msgBuffer,"type>Declarative action>timeout ubi>%lu info>\"No Offevent sent\"",ubi);

    }
    else
      {
	DMSG(1,"\nDeclarative::OnBehaviorTimeout() GetFlowID(%d) already off\n",GetFlowID());
	sprintf(msgBuffer,"type>Declarative action>timeout ubi>%lu info>\"flow already turned off\"",ubi);

      }
    
    // log event
    
    // Are these really used?
    if (successLogicID)
      {
	sprintf(tmpBuffer," successLogicID>%d",successLogicID);
	strcat (msgBuffer,tmpBuffer);
	rapr.LogicTable().DoLogicID(successLogicID,(BehaviorEvent*)this);
      }
    if (timeoutLogicID)
      {
	sprintf(tmpBuffer," timeoutLogicID>%d",timeoutLogicID);
	strcat (msgBuffer,tmpBuffer);
	rapr.LogicTable().DoLogicID(timeoutLogicID,(BehaviorEvent*)this);
      }
    rapr.LogEvent("RAPR",msgBuffer);

    
    if (endBehaviorTimer.IsActive()) endBehaviorTimer.Deactivate();
    ClearState();
    
    return false; // ProtoTimerMgr will reschedule timer if true!
    
} // end Declarative::OnBehaviorTimeout

Interrogative::Interrogative(ProtoTimerMgr& timerMgr,
                             unsigned long ubi,
                             Rapr& theRapr)
  : BehaviorEvent(timerMgr,ubi,theRapr),
    retryCount(3),
    retryInterval(10),
    retriesSent(0),
    payloadSeed(0)
{
    type = INTERROGATIVE;
}

Interrogative::~Interrogative()
{
    if (endBehaviorTimer.IsActive()) endBehaviorTimer.Deactivate();
    if (retryTimer.IsActive()) retryTimer.Deactivate();
}

bool Interrogative::OnStartUp()
{
    char buffer[8192];
    char tmpBuffer[512];
    stopped = false;


    // Only set payload seed upon first startup
    if (!payloadSeed)
    {
        payloadSeed = prng->GetRand();
    }
    
    if (GetInterface()) 
    {
        ProtoSocket tmpSocket(ProtoSocket::UDP);
        tmpSocket.GetInterfaceAddress(GetInterface(),ProtoAddress::IPv4,srcAddr);
    } 
    else 
    {
        // this should be (local machine's srcAddr) 
        // a global value resolved once at startup
        // (in not a config file setting) since
        // resolveLocalAddress can take a LONG time
        // to timeout
        
        srcAddr.ResolveLocalAddress();
    }

    // Get a new flow id upon each startup
    // If the old one wasn't stopped yet,
    // it will eventually get recycled 
    // anyway...
    SetFlowID(rapr.GetNumberGenerator().GetFlowID(&srcAddr,&dstAddr));
    
    // We always set the UBI for interrogative objects
    RaprPayload raprPayload;
    raprPayload.SetUBI(GetUBI());
    
    if (GetPayloadLogicID() || GetForeignUBI() || GetSeed() || GetOriginUBI())
    {
        raprPayload.SetLogicID(GetPayloadLogicID());
        raprPayload.SetSeed(GetPayloadSeed()); 
        raprPayload.SetForeignUBI(GetForeignUBI());
        raprPayload.SetOrigin(GetOriginUBI());
    }
    
    char *payld = raprPayload.GetHex();
    sprintf(buffer,"ON %d %s SRC %hu DST %s/%hu %s DATA [%s]",
            GetFlowID(),GetStringFromProtocol(protocol),srcPort,dstAddr.GetHostString(),dstAddr.GetPort(),patternString, payld);
    delete [] payld;
    
    
    if (tos)
    {
        sprintf(tmpBuffer," TOS %d",tos);
        strcat(buffer,tmpBuffer);
    }
    if (tx_buffer_size)
    {
        sprintf(tmpBuffer," TXBUFFER %d",tx_buffer_size);
        strcat(buffer,tmpBuffer);
    }
    if (mgen_msg_count)
    {
      // Make sure count option is set as we are 
      // relying on mgen to stop the flow
      Option option = GetOptionFromString("COUNT");
      option_mask |= option;

      sprintf(tmpBuffer," COUNT %d",mgen_msg_count);
      strcat(buffer,tmpBuffer);
    }
    
    if (ttl) 
    {
        sprintf(tmpBuffer," TTL %d",ttl);
        strcat(buffer,tmpBuffer    );
    }
    
    if (GetInterface())
    {
        sprintf(tmpBuffer," INTERFACE %s",GetInterface());
        strcat(buffer,tmpBuffer);
    }
    
    rapr.SendMgenCommand("event",buffer);
    
    char msgBuffer[8192];
    sprintf(msgBuffer,"type>Interrogative action>start ubi>%lu eventSource>%s retriesSent>%d",ubi,GetStringFromEventSource(eventSource),retriesSent);
    
    if (rapr.GetVerbosity()) 
    {
        
        if (foreignUbi)
        {
            sprintf(tmpBuffer," foreignUBI>%lu",foreignUbi);
            strcat(msgBuffer,tmpBuffer);
        }
        if (successLogicID)
        {
            sprintf(tmpBuffer," successLogicID>%d",successLogicID);
            strcat(msgBuffer,tmpBuffer);
        }
        if (failureLogicID)
        {
            sprintf(tmpBuffer," failureLogicID>%d",failureLogicID);
            strcat(msgBuffer,tmpBuffer);
        }
        if (timeoutLogicID)
        {
            sprintf(tmpBuffer," timeoutLogicID>%d",timeoutLogicID);
            strcat(msgBuffer,tmpBuffer);
        }
        if (payloadLogicID)
        {
            sprintf(tmpBuffer," payloadLogicID>%d",payloadLogicID);
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
        if (retryInterval)
        {
            sprintf(tmpBuffer," retryInterval>%f",retryInterval);
            strcat(msgBuffer,tmpBuffer);
        }
        if (retryCount)
        {
            sprintf(tmpBuffer," retryCount>%d",retryCount);
            strcat(msgBuffer,tmpBuffer);
        }
        if (GetSeed())
        {
            sprintf(tmpBuffer," seed>%ld",GetSeed());
            strcat(msgBuffer,tmpBuffer);
        }
        
        if (raprPayload.GetSeed())
        {
            sprintf(tmpBuffer," payloadSeed>%u",raprPayload.GetSeed());
            strcat(msgBuffer,tmpBuffer);
        }        
    }
    
    sprintf(tmpBuffer," mgenCmd>\"%s\"",buffer);
    strcat(msgBuffer,tmpBuffer);
    rapr.LogEvent("RAPR",msgBuffer);
        
    msgBuffer[0] = '\0';
    
    // Default flow length to one second.  We unlock the flow after
    // mgen processes the off event.

    double off_time = 0.99;

    // See if stream duration is overridden in the dictionary
    const char *dictionaryValue;
    double defaultStreamDuration = 0;
    if ((dictionaryValue = LookupDictionaryEntry("%SystemDefaults:streamduration%")))
    {
        defaultStreamDuration = atof(dictionaryValue);
    }

    if (defaultStreamDuration)
      off_time = defaultStreamDuration;

    if (OptionIsSet(COUNT))
    {
        // we ~could~ default to this?
        //        off_time = (pattern.GetIntervalAve()) * mgen_msg_count;
        off_time = retryInterval;
    }
    
    // If the flow has not already been stopped or count is not
    // set (in which case mgen will turn off the flow)
    // schedule an OFF event.  
    if (!stopped && !OptionIsSet(COUNT))
      {
	sprintf(buffer,"%f OFF %d",off_time,GetFlowID());
	rapr.SendMgenCommand("event",buffer);
	stopped = true;
	sprintf(tmpBuffer,"type>Interrogative action>off_event ubi>%lu eventSource>%s retriesSent>%d mgenCmd>\"%s\"",ubi,GetStringFromEventSource(eventSource),retriesSent,buffer);
	strcat(msgBuffer,tmpBuffer);
	rapr.LogEvent("RAPR",msgBuffer);
      }
    
    // Schedule the end behavior timer first time around only
    if (!endBehaviorTimer.IsActive())
    {
        endBehaviorTimer.SetListener(this,&Interrogative::OnBehaviorTimeout);
        ActivateEndTimer(endBehaviorTimer);
    }
    
    // Start the first retry timeout
    if (!retryTimer.IsActive() && GetType() != PINGPONG) // ljt
    {
        if (retryInterval != 0.0 && retryCount > 0)
        {
            retryTimer.SetListener(this,&Interrogative::OnRetryTimeout);
            //      retryTimer.SetInterval(behaviorStartTime + retryInterval); 
            retryTimer.SetInterval(retryInterval); 
            retryTimer.SetRepeat(-1); // ljt can we use this versus retry count            
            timer_mgr.ActivateTimer(retryTimer);
        }
    }
    // log event
    retriesSent++;
    return true;
    
} // end Interrogative::OnStartup

bool Interrogative::Stop()
{
    // log event
    char msgBuffer[8192];
    char tmpBuffer[512];
    sprintf(msgBuffer,"type>Interrogative action>stopped ubi>%lu",ubi);
    if (rapr.GetVerbosity())
    {
        if (raprFlowID)
        {
            sprintf(tmpBuffer," raprFlowID>%d",raprFlowID);
            strcat(msgBuffer,tmpBuffer);
        }
        if (GetSeed())
        {
            sprintf(tmpBuffer," seed>%ld",GetSeed());
            strcat(msgBuffer,tmpBuffer);
        }
        
    }
    
    rapr.LogEvent("RAPR",msgBuffer);
    
    if (endBehaviorTimer.IsActive()) endBehaviorTimer.Deactivate();
    if (retryTimer.IsActive()) retryTimer.Deactivate();
    ClearState();
    
    // ljt do we want to process logic id's?
    
    return true;
    
} // end Interrogative::Stop

bool Interrogative::ProcessEvent(const MgenMsg& theMsg)
{
    
    // log event
  char msgBuffer[512],tmpBuffer[512];
  sprintf(msgBuffer,"type>Interrogative action>success ubi>%lu ",ubi);
    
  if (endBehaviorTimer.IsActive()) endBehaviorTimer.Deactivate();
  if (retryTimer.IsActive()) retryTimer.Deactivate();
  
  if (successLogicID)
    {
      sprintf(tmpBuffer," successLogicID>%d",successLogicID);
      strcat(msgBuffer,tmpBuffer);
      rapr.LogicTable().DoLogicID(successLogicID,(BehaviorEvent*)this);
    }
  rapr.LogEvent("RAPR",msgBuffer);
  
  ClearState();
    
  return true;
} // end Interrogative::ProcessEvent

void Interrogative::ClearState()
{
    DMSG(3,"Interrogative::ClearState() \n");
    
    if (endBehaviorTimer.IsActive()) endBehaviorTimer.Deactivate();
    if (retryTimer.IsActive()) retryTimer.Deactivate();
    ubi = 0;
    SetRetryInterval(0);
    SetRetryCount(0);
    SetRetriesSent(0);
    SetPayloadSeed(0);

    BehaviorEvent::ClearState();
    
} // end Interrogative::ClearState

bool Interrogative::OnBehaviorTimeout(ProtoTimer& /*theTimer*/)
{    
    // log event
    char msgBuffer[8192],tmpBuffer[512];
    sprintf(msgBuffer,"type>Interrogative action>timeout ubi>%lu ",ubi);
    
    if (endBehaviorTimer.IsActive()) endBehaviorTimer.Deactivate();
    if (retryTimer.IsActive()) retryTimer.Deactivate();
    
    if (timeoutLogicID)
      {
	sprintf(tmpBuffer," timeoutLogicID>%d",timeoutLogicID);
	rapr.LogicTable().DoLogicID(timeoutLogicID,(BehaviorEvent*)this);
      }
    // If we timed out, we failed
    if (failureLogicID)
      {
	sprintf(tmpBuffer," failureLogicID>%d",failureLogicID);
	rapr.LogicTable().DoLogicID(failureLogicID,(BehaviorEvent*)this);
      }
    rapr.LogEvent("RAPR",msgBuffer);

    ClearState();
    
    return false;
} // end Interrogative::OnBehaviorTimeout

bool Interrogative::OnRetryTimeout(ProtoTimer& /*theTimer*/)
{
    // ljt look into using retryCount on timer itself
    if (retryCount > 0 && GetType() != PINGPONG) // ljt
    {
        
        if (retryTimer.IsActive()) {
            retryTimer.SetInterval(retryInterval);
            retryTimer.Reschedule();
            retryCount--;
            
            // log event
            char msgBuffer[8192];
            sprintf(msgBuffer,"type>Interrogative action>retryTimeout ubi>%lu",ubi);
            rapr.LogEvent("RAPR",msgBuffer);
            
            OnStartUp();
            return false; // Otherwise ProtoTimerMgr tries to reinstall the timer
        }
        else 
        {
            DMSG(0,"Interrogative::OnRetryTimeout() Error: No active retry timer!\n");
        }
    }
    else
    {
        retryTimer.Deactivate();
        return false; // needed by protoTimerMgr so it doesn't re-install the timer itself
    }
    return false;
} // end Interrogative::OnRetryTimeout

PingPong::PingPong(ProtoTimerMgr& timerMgr,
               unsigned long ubi,
               Rapr& theRapr)
  : Interrogative(timerMgr,ubi,theRapr)
{
    type = PINGPONG;
}

PingPong::~PingPong()
{
}
bool PingPong::ProcessEvent(const MgenMsg& theMsg)
{    
    // log event
    char msgBuffer[512];
    sprintf(msgBuffer,"type>PingPong action>success ubi>%lu ",ubi);
    rapr.LogEvent("RAPR",msgBuffer);

    if (GetRetryCount() > 0) 
    {
      Interrogative::OnStartUp();
      retryCount--;
    }
    else
      DMSG(0,"Max number of retries reached ljt \n");
    // if (endBehaviorTimer.IsActive()) endBehaviorTimer.Deactivate();

    // ljt take me back out!!
    //    if (retryTimer.IsActive()) retryTimer.Deactivate();
    
    if (successLogicID)
      rapr.LogicTable().DoLogicID(successLogicID,(BehaviorEvent*)this);
    // remember to log event if this is ever implemented
    //   ClearState();
    
    return true;
} // end PingPong::ProcessEvent

Stream::Stream(ProtoTimerMgr& timerMgr,
               unsigned long ubi,
               Rapr& theRapr)
  : BehaviorEvent(timerMgr,ubi,theRapr),
    stream_option_mask(0),
    burstDuration(0),
    burstCount(0),
    burstPriority(0),
    burstsSent(0),
    streamUbi(0),
    streamSeqNum(0),
    respProbLowRange(0),
    respProbHighRange(0),
    burstDelayLowRange(0),
    burstDelayHighRange(0),
    burstRangeLowRange(0),
    burstRangeHighRange(0),
    burstPayloadID(0),
    timeoutInterval(0),
    started(false),
    streamQueued(false),
    streamState(INACTIVE_STREAM)
{
    type = STREAM;
    
}

Stream::~Stream()
{
    if (endBehaviorTimer.IsActive()) endBehaviorTimer.Deactivate();
    if (endBurstTimer.IsActive()) endBurstTimer.Deactivate();
    if (startBurstTimer.IsActive()) startBurstTimer.Deactivate();
}

bool Stream::OnStartUp()
{
    // ljt hack b/c object seems to restart sometimes when
    // rti is sent??? this must be fixed.
    if (started) {
        DMSG(0,"Stream::OnStartup Error: Stream already started! Program error!\n");
        return false;
    }
    char msgBuffer[8192],tmpBuffer[512];
    tmpBuffer[0] = '\0';
    started = true;
    // ljt add default timeout interval?
        
    // Schedule the end behavior timer first time around only
    if (!endBehaviorTimer.IsActive())
    {
        endBehaviorTimer.SetListener(this,&Stream::OnBehaviorTimeout);
        ActivateEndTimer(endBehaviorTimer);
    }
    
    if (rapr.GetActiveStream() && rapr.GetActiveStream() != this)
    {
        DMSG(0,"Stream::OnStartUp Warning: Another stream is active. My streamUbi> %lu Active streamUbi>%lu .\n",streamUbi,rapr.GetActiveStream()->GetStreamUbi());
        
        // ljt work this logic out
        if (rapr.GetActiveStream()->GetBurstPriority() >= burstPriority)
        {
            DMSG(0,"info>\"Warning: Another active stream %lu has an equal or higher priority %d  myPriority %d!  Stream %lu not starting\"",rapr.GetActiveStream()->GetStreamUbi(),rapr.GetActiveStream()->GetBurstPriority(),burstPriority,streamUbi);

	    sprintf(msgBuffer,"type>Stream action>start ubi>%lu streamUbi>%lu eventSource>%s ",ubi,streamUbi,GetStringFromEventSource(eventSource));
	    sprintf(tmpBuffer," info>\"Warning: Another active stream %lu has an equal or higher priority: %d this priority: %d  Stream %lu not starting\"",rapr.GetActiveStream()->GetStreamUbi(),rapr.GetActiveStream()->GetBurstPriority(),burstPriority,streamUbi);
	    strcat(msgBuffer,tmpBuffer);
	    rapr.LogEvent("RAPR",msgBuffer);
            return true; // so timer doesn't restart
        }
        else 
        {
            DMSG(0,"Stream::OnStartUp Warning: Active Stream>%lu has a lower priority>%d stream>%lu priority>%d Stopping active stream and starting.\n",rapr.GetActiveStream()->GetStreamUbi(),rapr.GetActiveStream()->GetBurstPriority(),streamUbi,burstPriority);
	    sprintf(tmpBuffer," info>\"Warning: Active Stream %lu has a lower priority: %d stream %lu priority: %d stopping active stream and starting.\"",rapr.GetActiveStream()->GetStreamUbi(),rapr.GetActiveStream()->GetBurstPriority(),streamUbi,burstPriority);
            rapr.GetActiveStream()->Stop();
        }
    }
    
    sprintf(msgBuffer,"type>Stream action>start ubi>%lu streamUbi>%lu eventSource>%s ",ubi,streamUbi,GetStringFromEventSource(eventSource));
    strcat(msgBuffer,tmpBuffer);
    rapr.LogEvent("RAPR",msgBuffer);
    
    
    // If we were triggered by a net event,
    // go to stream event processing
    if (eventSource == NET_EVENT)
    {
        // dummy reference
        MgenMsg theMsg;
        ProcessEvent(theMsg);
        return true;      
    }
    
    streamSeqNum = 0;
    StartBurst(endBurstTimer);
    
    // Start the timer to stop the first stream burst
    if (!endBurstTimer.IsActive())
    {
        // If burst duration = 0, don't stop the stream
        // this way.  It will timeout, or another stream will
        // interrupt. (?ljt)
        if (burstDuration != 0.0)
        {
            endBurstTimer.SetListener(this,&Stream::OnBurstTimeout);
            endBurstTimer.SetInterval(burstDuration); 
            endBurstTimer.SetRepeat(-1); // ljt can we use this versus retry count?
            
            timer_mgr.ActivateTimer(endBurstTimer);
        }
    }
    // log event
    return true;
    
} // end Stream::OnStartup

void Stream::ActivateEndTimer(ProtoTimer& endBehaviorTImer)
{
    // We override BehaviorEvent::ActivateEndTimer
    // Use timeoutInterval to timeout our objects if
    // it exists.  If we haven't received a response 
    // during this interval, we will timeout.
    
    if (timeoutInterval)
    {
        endBehaviorTimer.SetInterval(timeoutInterval);
    }
    else {
        if (duration)
        {
            endBehaviorTimer.SetInterval(behaviorEndTime);
        }
        else
        {
            double offsetTime = rapr.GetOffset() > 0.0 ? rapr.GetOffset() : 0.0;
            if (offsetTime < behaviorStartTime)
            {
                endBehaviorTimer.SetInterval(behaviorEndTime - behaviorStartTime);
            }
            else
            {
                endBehaviorTimer.SetInterval(behaviorEndTime - offsetTime);
            }
        }
    }
    endBehaviorTimer.SetRepeat(0);
    
    timer_mgr.ActivateTimer(endBehaviorTimer);
    
} // end Stream::ActivateBehaviorTimer

bool Stream::Stop()
{
    // log event
    char msgBuffer[8192];
    char tmpBuffer[512];
    sprintf(msgBuffer,"type>Stream action>stopped ubi>%lu streamUbi>%lu",ubi,streamUbi);
    
    if (rapr.GetVerbosity())
    {
        if (raprFlowID)
        {
            sprintf(tmpBuffer," raprFlowID>%d",raprFlowID);
            strcat(msgBuffer,tmpBuffer);
        }
        
    }
    
    rapr.LogEvent("RAPR",msgBuffer);
    
    // Stop any existing flows
    for (unsigned int i = 0; i < streamFlowIdList.size(); i++)
    {
        char buffer[512];
        msgBuffer[0] = '\0';
	if (!stopped)
	  {
	    sprintf(buffer,"0.01 OFF %d",streamFlowIdList[i]);
	    rapr.SendMgenCommand("event",buffer);
	    stopped = true;
	    sprintf(tmpBuffer,"type>Stream action>off_event ubi>%lu eventSource>%s burstsSent>%d mgenCmd>\"%s\"",ubi,GetStringFromEventSource(eventSource),burstsSent,buffer);
	    strcat(msgBuffer,tmpBuffer);
	    rapr.LogEvent("RAPR",msgBuffer);

	  }
        flowID = 0;
    }
    streamFlowIdList.clear();
    
    if (endBehaviorTimer.IsActive()) endBehaviorTimer.Deactivate();
    if (endBurstTimer.IsActive()) endBurstTimer.Deactivate();
    if (startBurstTimer.IsActive()) startBurstTimer.Deactivate();
    
    streamQueued = false;
    streamState = STOPPED_STREAM;
    if (rapr.GetActiveStream() == this)
    {
        DMSG(0,"Stream::Stop() Relinquishing active stream streamUbi>%lu ubi>%lu.\n",streamUbi,ubi);
        rapr.SetActiveStream(NULL);
    }
    ClearState();
    
    // ljt process logic id's?
    
    return true;
    
} // end Stream::Stop

bool Stream::StartBurst(ProtoTimer& /*theTimer*/)
{
    char buffer[8192];
    char tmpBuffer[512];
    
    
    if (endBurstTimer.IsActive())
    {
        DMSG(0,"Stream::Processevent() Error: Previous burst still active! ubi>%lu streamUbi>%lu \n",ubi,streamUbi);
        // don't want to restart timer
        return true;
    }
    
    // Get a new flow id upon each startup
    // it will eventually get recycled 
    if (GetInterface()) {
        ProtoSocket tmpSocket(ProtoSocket::UDP);
        tmpSocket.GetInterfaceAddress(GetInterface(),ProtoAddress::IPv4,srcAddr);
    } else {
        // this should be (local machine's srcAddr) 
        // a global value resolved once at startup
        // (in not a config file setting) since
        // resolveLocalAddress can take a LONG time
        // to timeout
        
        srcAddr.ResolveLocalAddress();
    }
    SetFlowID(rapr.GetNumberGenerator().GetFlowID(&srcAddr,&dstAddr));
    RaprPayload raprPayload;
    
    
    streamSeqNum++;
    DMSG(3,"Stream::StartBurst() Incrementing sequence number!!!! %d\n",streamSeqNum);
    
    burstsSent++;
    burstCount--;
    streamQueued = false;
    streamState = ACTIVE_STREAM;
    rapr.SetActiveStream(this);
    
    // Use the burst duration if we have that,
    // otherwise get a random burst interval.
    // Always use the 
    
    double newDuration = burstDuration;
    if (burstRangeLowRange || burstRangeHighRange)
    {
        newDuration =  prng->GetRandom(burstRangeLowRange,burstRangeHighRange);
    }
    DMSG(1,"\n\nDuration: %.2f Seed: %d\n\n",newDuration,seed);
    SetBurstDuration(newDuration);
    
    // We set streamUbi in parse event..
    // It's either our ubi if the event is
    // script generated, or that of the incoming
    // triggering packet.
    
    // ljt this is broken, everything gets set anyway by the payload code!
    if (GetPayloadLogicID() || GetSeed() || GetStreamUbi() || GetBurstDuration() || GetBurstCount() || GetStreamSeqNum() || GetBurstPayloadID() || GetBurstPriority() || GetOriginUBI())
    {
        raprPayload.SetLogicID(GetPayloadLogicID());
        raprPayload.SetSeed(prng->GetRand()); 
        raprPayload.SetStreamID(GetStreamUbi());
        raprPayload.SetStreamDuration(GetBurstDuration());
        raprPayload.SetBurstCount(GetBurstCount());
        raprPayload.SetBurstPriority((unsigned int)GetBurstPriority());
        raprPayload.SetBurstPayloadID(GetBurstPayloadID());
        raprPayload.SetStreamSeq(GetStreamSeqNum());
        raprPayload.SetOrigin(GetOriginUBI());
    }
    
    if (GetInterface()) {
        ProtoSocket tmpSocket(ProtoSocket::UDP);
        tmpSocket.GetInterfaceAddress(GetInterface(),ProtoAddress::IPv4,srcAddr);
    } else {
        // this should be (local machine's srcAddr) 
        // a global value resolved once at startup
        // (in not a config file setting) since
        // resolveLocalAddress can take a LONG time
        // to timeout
        
        srcAddr.ResolveLocalAddress();
    }
    for (unsigned int i = 0; i < streamPatternVector.size(); i++)
    {
        // Get a new flow id for each flow upon each startup
        // They will eventually get recycled 
        
        int flowId = rapr.GetNumberGenerator().GetFlowID(&srcAddr,&dstAddr);
        streamFlowIdList.push_back(flowId);
        
        char *payld = raprPayload.GetHex();
        sprintf(buffer,"ON %d %s SRC %hu DST %s/%hu %s DATA [%s]",flowId,GetStringFromProtocol(protocol),srcPort,dstAddr.GetHostString(),dstAddr.GetPort(),streamPatternVector[i], payld);
        delete [] payld;
        
        if (tos)
        {
            sprintf(tmpBuffer," TOS %d",tos);            
            strcat(buffer,tmpBuffer);
        }
        if (tx_buffer_size)
        {
            sprintf(tmpBuffer," TXBUFFER %d",tx_buffer_size);
            strcat(buffer,tmpBuffer);
        }
        if (mgen_msg_count)
        {
	  // Make sure option mask is set as we are 
	  // relying on mgen to stop the flow
	    Option option = GetOptionFromString("COUNT");
	    option_mask |= option;

            sprintf(tmpBuffer," COUNT %d",mgen_msg_count);
            strcat(buffer,tmpBuffer);
        }
        
        if (ttl) 
        {  
            sprintf(tmpBuffer," TTL %d",ttl);
            strcat(buffer,tmpBuffer    );
        }
        
        if (GetInterface())
        {
            sprintf(tmpBuffer," INTERFACE %s",GetInterface());
            strcat(buffer,tmpBuffer);
        }
        
        rapr.SendMgenCommand("event",buffer);
        
        //      buffer[0] = '\0';
    }
    char msgBuffer[8192];
    sprintf(msgBuffer,"type>Stream action>start_burst streamUbi>%lu ubi>%lu eventSource>%s",streamUbi,ubi,GetStringFromEventSource(eventSource));
    
    if (rapr.GetVerbosity()) 
    {
        
        if (foreignUbi)
        {
            sprintf(tmpBuffer," foreignUBI>%lu",foreignUbi);
            strcat(msgBuffer,tmpBuffer);
        }
        if (streamUbi)
        {
            sprintf(tmpBuffer," streamUBI>%lu",streamUbi);
            strcat(msgBuffer,tmpBuffer);
        }
        if (streamSeqNum)
        {
            sprintf(tmpBuffer," streamSeqNum>%lu",streamSeqNum);
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
        if (burstDuration)
        {
            sprintf(tmpBuffer," burstDuration>%.2f",burstDuration);
            strcat(msgBuffer,tmpBuffer);
        }
        if (burstCount)
        {
            sprintf(tmpBuffer," burstCount>%d",burstCount);
            strcat(msgBuffer,tmpBuffer);
        }
        if (burstPriority)
        {
            sprintf(tmpBuffer," burstPriority>%d",burstPriority);
            strcat(msgBuffer,tmpBuffer);
        }
        if (burstsSent)
        {
            sprintf(tmpBuffer," burstsSent>%d",burstsSent);
            strcat(msgBuffer,tmpBuffer);
        }
        if (respProbLowRange || respProbHighRange)
        {
            sprintf(tmpBuffer," respProbLowRange>%d",respProbLowRange);
            strcat(msgBuffer,tmpBuffer);
            
            sprintf(tmpBuffer," respProbHighRange>%d",respProbHighRange);
            strcat(msgBuffer,tmpBuffer);
        }
        if (burstDelayLowRange || burstDelayHighRange)
        {
            sprintf(tmpBuffer," burstDelayLowRange>%.2f",burstDelayLowRange);
            strcat(msgBuffer,tmpBuffer);
            
            sprintf(tmpBuffer," burstDelayHighRange>%.2f",burstDelayHighRange);
            strcat(msgBuffer,tmpBuffer);
        }
        if (burstRangeLowRange || burstRangeHighRange)
        {
            sprintf(tmpBuffer," burstRangeLowRange>%.2f",burstRangeLowRange);
            strcat(msgBuffer,tmpBuffer);
            
            sprintf(tmpBuffer," burstRangeHighRange>%.2f",burstRangeHighRange);
            strcat(msgBuffer,tmpBuffer);
        }
        if (burstPayloadID)
        {
            sprintf(tmpBuffer," burstPayloadID>%d",burstPayloadID);
            strcat(msgBuffer,tmpBuffer);
        }
        if (timeoutInterval)
        {
            sprintf(tmpBuffer," timeoutInterval>%.2f",timeoutInterval);
            strcat(msgBuffer,tmpBuffer);
        }
        if (GetSeed())
        {
            sprintf(tmpBuffer," seed>%ld",GetSeed());
            strcat(msgBuffer,tmpBuffer);
        }
        
        if (raprPayload.GetSeed())
        {
            sprintf(tmpBuffer," payloadSeed>%u",raprPayload.GetSeed());
            strcat(msgBuffer,tmpBuffer);
        }
        
    }
    
    sprintf(tmpBuffer," mgenCmd>\"%s\"",buffer);
    strcat(msgBuffer,tmpBuffer);
    rapr.LogEvent("RAPR",msgBuffer);
    
    
    if (burstDuration != 0.0 && !endBurstTimer.IsActive())
    {
        endBurstTimer.SetListener(this,&Stream::OnBurstTimeout);
        endBurstTimer.SetInterval(burstDuration);
        endBurstTimer.SetRepeat(-1);
        
        timer_mgr.ActivateTimer(endBurstTimer);
    }
    if (startBurstTimer.IsActive())
      startBurstTimer.Deactivate();
    
    // Don't want to reactivate proto-timer!
    return true;
} // end Stream::StartBurst()

bool Stream::ProcessEvent(const MgenMsg& theMsg)
{
  RaprPayload raprPayload;

  const char * payld = theMsg.GetPayload();
  if (payld)
  {
    raprPayload.SetHex(payld);
	delete [] payld;
  }

  // Reschedule timeout interval
  if (streamState != STOPPED_STREAM && endBehaviorTimer.IsActive() && timeoutInterval)
    {
      endBehaviorTimer.SetInterval(timeoutInterval);
      endBehaviorTimer.Reschedule();
    }

  if (rapr.GetActiveStream() && rapr.GetActiveStream() != this)
    {
      DMSG(0,"Stream::ProcessEvent() Warning: Another stream is active. Active stream ubi>%lu My Ubi>%lu\n",rapr.GetActiveStream()->GetStreamUbi(),streamUbi);
      return false;
    }

  if (streamState == STOPPED_STREAM)
    {
      DMSG(0,"Stream::ProcessEvent() Error: Stream was stopped. streamUbi>%lu ubi>%lu\n",streamUbi,ubi);
      return false;
    }
  if (streamQueued)
    {
      DMSG(1,"\nStream::ProcessEvent() Stream already queued, streamstate: %d.\n\n",streamState);
      return false;
    }
  // If payload seq is < our seq num we have an
  // out of order packet, ignore it.
  if ((theMsg.GetPayload()) && (raprPayload.GetStreamSeq() < streamSeqNum))
    {
      // ljt 01/27/14 - should this be an error message?
      DMSG(1,"Stream::ProcessEvent() Error: Out of order burst sequence number received>%d latest>%d seq>%s\n",raprPayload.GetStreamSeq(),streamSeqNum,theMsg.GetSeqNum());
      return false;
    }

  // Get updated burst info from payload
  // (otherwise this was set by rapr::ParseEvent)
  if (theMsg.GetPayload()) 
  {
    DMSG(1,"Stream::ProcessEvent() Setting seq # from payload before>%d ",streamSeqNum);

    streamSeqNum = raprPayload.GetStreamSeq();
    burstDuration = raprPayload.GetStreamDuration();
    DMSG(1,"after>%d duration>%.2f\n",streamSeqNum,burstDuration);

    burstCount = raprPayload.GetBurstCount();
    burstDuration = raprPayload.GetStreamDuration();
    burstPriority = raprPayload.GetBurstPriority();
    prng->SetSeed(raprPayload.GetSeed());
  }

    // Burst payload id's are being used to direct 
    // changes in stream state variables.
  if (theMsg.GetPayload() && raprPayload.GetBurstPayloadID())
    {
      ProtoAddress srcAddr; // ljt tmp
      char msgBuffer [512];
      sprintf(msgBuffer,"type>streamBurst payloadLogicID>%d streamUBI>%lu streamSeqNum>%lu",raprPayload.GetBurstPayloadID(),streamUbi,streamSeqNum);
      rapr.LogEvent("RAPR",msgBuffer);
      if (!rapr.LogicTable().DoLogicID((int)raprPayload.GetBurstPayloadID(),theMsg,srcAddr,(BehaviorEvent*)this))
	{
	  sprintf(msgBuffer,"type>error payloadLogicID>%d streamUBI>%lu streamSeqNum>%lu",raprPayload.GetBurstPayloadID(),streamUbi,streamSeqNum);
	  rapr.LogEvent("RAPR",msgBuffer);

	  DMSG(0,"Stream::ProcessEvent() Error: Burst Logic ID Failure. streamUBI>%lu streamSeqNum>%lu",streamUbi,streamSeqNum);
	  return false;
	}
    }

  if (!started)
    {
      DMSG(1,"Stream::Processevent() Error: Stream object not yet started. ubi>%lu streamUbi>%lu\n",ubi,streamUbi);
      return false;
    }
  
  // Find out if we're the next to respond (even
  // if we started the burst!) if so, queue the
  // next burst.

  // Is it my turn to answer?
  if (respProbLowRange || respProbHighRange)
    {
      int responseProbability = prng->GetRand(0,100); 
      if ((responseProbability >= respProbLowRange) &&
	  (responseProbability <= respProbHighRange))
	{
	  DMSG(1,"\nStream::Processevent() %d > %d < %d My turn to answer\n\n",respProbLowRange,responseProbability,respProbHighRange);	  
	  
	}
      else 
	{
	  DMSG(1,"Stream::Processevent() NOT IN RANGE %d > %d < %d \n",respProbLowRange,responseProbability,respProbHighRange);	  
	  return false;
	}
    }
  else 
    {
      DMSG(1,"Stream::Processevent() NOT MY TURN %d > 0 < %d \n",respProbLowRange,respProbHighRange);	  
      return false;
    }
  if (!burstCount)
    {
      DMSG(1,"Stream::Processevent() Burst count exceeded. ubi>%lu streamUbi>%lu streamSeqNum>%lu burstsSent>%lu burstCount>%lu\n",ubi,streamUbi,streamSeqNum,burstsSent,burstCount);
      return false;
    }

  if (startBurstTimer.IsActive())
    {
      DMSG(1,"Stream::Processevent() Error: Burst already queued! ubi>%lu streamUbi>%lu \n",ubi,streamUbi);
      return false;
    }

  streamQueued = true;

  // log event
  char msgBuffer[512];
  sprintf(msgBuffer,"type>Stream action>burst_queued ubi>%lu streamUbi>%lu\n",ubi,streamUbi);
  rapr.LogEvent("RAPR",msgBuffer);

  if (burstDuration != 0.0) // ljt fix all this
  {
      
      double secs;
      if (burstDelayLowRange || burstDelayHighRange)
      {
          secs = prng->GetRandom(burstDelayLowRange,burstDelayHighRange);	    
      }
      DMSG(1,"\nWaiting for %.2f Seed: %d\n\n",secs,seed);
      
      startBurstTimer.SetListener(this,&Stream::StartBurst);
      startBurstTimer.SetInterval(burstDuration+secs);
      startBurstTimer.SetRepeat(-1);
      timer_mgr.ActivateTimer(startBurstTimer);
  }

  return true;
} // end Stream::ProcessEvent

void Stream::ClearState()
{

  DMSG(1,"Stream::ClearState() \n");

  if (endBehaviorTimer.IsActive()) endBehaviorTimer.Deactivate();
  if (endBurstTimer.IsActive()) endBurstTimer.Deactivate();
  SetBurstDuration(0);
  SetBurstCount(0);
  SetBurstPriority(0);
  SetBurstsSent(0);
  SetStreamUbi(0);
  SetStreamSeqNum(0);
  SetRespProbLowRange(0);
  SetRespProbHighRange(0);
  SetBurstDelayLowRange(0);
  SetBurstDelayHighRange(0);
  SetBurstRangeLowRange(0);
  SetBurstRangeHighRange(0);
  SetStarted(false);
  SetStreamQueued(false);
  SetStreamState(STOPPED_STREAM);
  BehaviorEvent::ClearState();


} // end Stream::ClearState

bool Stream::OnBehaviorTimeout(ProtoTimer& /*theTimer*/)
{

  // why do we need all these buffers ljt?
  char msgBuffer[8192];
  char buffer[8192];
  char tmpBuffer[512];

  // If the flow(s) is still running, stop it
  for (unsigned int i = 0; i < streamFlowIdList.size(); i++)
    {
      msgBuffer[0] = '\0';
      if (!stopped)
	{
	  sprintf(buffer,"OFF %d",streamFlowIdList[i]);
	  rapr.SendMgenCommand("event",buffer);
	  stopped = true;
	  sprintf(tmpBuffer,"type>Stream action>stopped_burst ubi>%lu eventSource>%s streamUbi>%lu burstsSent>%d mgenCmd>\"%s\"",ubi,GetStringFromEventSource(eventSource),streamUbi,burstsSent,buffer);
	  strcat(msgBuffer,tmpBuffer);
	  rapr.LogEvent("RAPR",msgBuffer);
	}
    }
  // Clear the flowId list
  streamFlowIdList.clear();
  
  // log event
  sprintf(msgBuffer,"type>Stream action>timeout ubi>%lu streamUbi>%lu burstsSent>%d",ubi,streamUbi,burstsSent);

  if (endBehaviorTimer.IsActive()) endBehaviorTimer.Deactivate();
  if (endBurstTimer.IsActive()) endBurstTimer.Deactivate();
  if (startBurstTimer.IsActive()) startBurstTimer.Deactivate();

  streamState = STOPPED_STREAM;
  if (rapr.GetActiveStream() == this)
    {
      DMSG(0,"Stream::Stop() Resetting active stream. \n");
      rapr.SetActiveStream(NULL);
    }


  if (timeoutLogicID)
    {
      sprintf(tmpBuffer," timeoutLogicID>%d",timeoutLogicID);
      rapr.LogicTable().DoLogicID(timeoutLogicID,(BehaviorEvent*)this);
    }
  // If we timed out, we failed
  if (failureLogicID)
    {
      sprintf(tmpBuffer," failureLogicID>%d",failureLogicID);
      rapr.LogicTable().DoLogicID(failureLogicID,(BehaviorEvent*)this);
    }
  strcat(msgBuffer,tmpBuffer);
  rapr.LogEvent("RAPR",msgBuffer);


  ClearState();

  return false;
} // end Stream::OnBehaviorTimeout

bool Stream::OnBurstTimeout(ProtoTimer& /*theTimer*/)
{
  // Stop the flow if it's still going
  // ljt come back and add support for multiple flows
  for (unsigned int i = 0; i < streamFlowIdList.size(); i++)
    {
      char buffer[8192];
      if (!stopped)
      {
	sprintf(buffer,"OFF %d",streamFlowIdList[i]);
	rapr.SendMgenCommand("event",buffer);  
	stopped = true;
	// log event
	char msgBuffer[512];
	sprintf(msgBuffer,"type>Stream action>stopped_burst ubi>%lu streamUbi>%lu burstsSent>%d mgenCmd>\"%s\"",ubi,streamUbi,burstsSent,buffer);
	rapr.LogEvent("RAPR",msgBuffer);

      }
      
    }

  streamFlowIdList.clear();
  streamState = INACTIVE_STREAM;
  if (rapr.GetActiveStream() == this)
    {
      DMSG(1,"Stream::OnBurstTimeout() Resetting active stream.\n");
      rapr.SetActiveStream(NULL);
    }

  // deactivate the burst timer
  if (endBurstTimer.IsActive()) endBurstTimer.Deactivate();
  return false;

} // end Stream::OnBurstTimeout


Periodic::Periodic(ProtoTimerMgr& timerMgr,
		   unsigned long ubi,
		   Rapr& theRapr)
  : BehaviorEvent(timerMgr,ubi,theRapr),
    exponential(false),
    periodicityInterval(0),
    behaviorEndBehaviorTime(0),
    behaviorEventType(INVALID_TYPE),
    periodicityCount(0),
    trans()
{
  type = PERIODIC;
  origCommand[0] = '\0';
} // End Periodic::Periodic()

Periodic::~Periodic()
{
  if (endBehaviorTimer.IsActive()) endBehaviorTimer.Deactivate();
  if (periodicityTimer.IsActive()) periodicityTimer.Deactivate();

} // End Periodic::~Periodic()

void Periodic::SetTrans(RaprDictionaryTransfer* inTrans)
{
    trans = inTrans;

} // end Periodic::SetTrans()

bool Periodic::OnStartUp()
{    
    periodicityCount++;

    // Temporary solution to reparse options upon startup
    RaprStringVector *vec;
    vec = rapr.LogicTable().GetDictionary().translate(origCommand,&trans);

    // Now get a new PRNG for the event
    //  RaprPRNG *newPrng = new RaprPRNG(rapr.GetNumberGenerator().GetNextSeed());
    if (!vec)
    {
        DMSG(0,"Periodic::OnStartUp() Error translating command: %s\n",origCommand);
        return false;
    }
    char* retranslatedCommand = strstr((char*)(*vec)[0],"INTERVAL");
    int cnt = 0;
    ParsePeriodicOptions(retranslatedCommand,cnt);

    retranslatedCommand += cnt;
    ((Periodic*)this)->SetBehaviorPattern(retranslatedCommand);

    switch (behaviorEventType)
    {
    case BehaviorEvent::DECLARATIVE:
      {
          Declarative* theEvent;
          theEvent = new Declarative(timer_mgr,rapr.GetNumberGenerator().GetUBI(),rapr);
          
          if (!theEvent)
          {
              DMSG(0,"Periodic::OnStartUp() Error: event allocation error: %s\n",strerror(errno));
              return false;
          }
          // start all events immediately
          theEvent->SetBehaviorStartTime(-1.0);
          theEvent->SetBehaviorEndTime(behaviorEndBehaviorTime);
          theEvent->SetDuration(true);
          theEvent->SetEventSource(eventSource);
          
          // ljt - this is sort of broken for repeatability
          // purposes.  If we start a periodicity object
          // with an offset - the seeding of spawned objects
          // won't be repeatable.  Right now we're not using
          // these but should be fixed.
          
          RaprPRNG *newPrng = new RaprPRNG(prng->GetRand());
          
          theEvent->SetPrng(newPrng);
          
          if (!theEvent->ParseOptions(behaviorPattern))
          {
              DMSG(0,"Periodic::OnStartUp() Error: behavior init error\n");
              delete theEvent;
              return false;
          }
          rapr.InsertBehaviorEvent(theEvent);
          break;
      }
    case BehaviorEvent::INTERROGATIVE:
      {
          // We get new flow id's every time we
          // send a message for interrogative 
          // events.
          Interrogative* theEvent = new Interrogative(timer_mgr,rapr.GetNumberGenerator().GetUBI(),rapr);
          if (!theEvent)
          {
              DMSG(0,"Periodic::OnStartUp() Error: rapr event allocation error: %s\n",strerror(errno));
              return false;
          }
          // Start all events immediately
          theEvent->SetBehaviorStartTime(-1.0);
          theEvent->SetBehaviorEndTime(behaviorEndBehaviorTime);
          theEvent->SetDuration(true);
          theEvent->SetEventSource(eventSource);
          
          // ljt - this is sort of broken for repeatability
          // purposes.  If we start a periodicity object
          // with an offset - the seeding of spawned objects
          // won't be repeatable.  Right now we're not using
          // these but should be fixed.
          
          RaprPRNG *newPrng = new RaprPRNG(prng->GetRand());
          
          theEvent->SetPrng(newPrng);
          
          if (!theEvent->ParseOptions(behaviorPattern))
          {
              DMSG(0,"Periodic::OnStartUp() Error: behavior init error\n");
              delete theEvent;
              return false;
          }
          rapr.InsertBehaviorEvent(theEvent);	
          break;
      }
    default:
      {
          DMSG(0,"Periodic::OnStartUp() Error: invalid behavior event type!\n");
          return false;
      }
    }
    
    
    // Schedule the end behavior timer first time around only
    if (!endBehaviorTimer.IsActive())
    {
        endBehaviorTimer.SetListener(this,&Periodic::OnBehaviorTimeout);
        ActivateEndTimer(endBehaviorTimer);
    }
    
    // Start periodicity timeout
    if (!periodicityTimer.IsActive())
    {
        if (periodicityInterval != 0.0)
        {
            periodicityTimer.SetListener(this,&Periodic::OnPeriodicityTimeout);
            periodicityTimer.SetInterval(GetNextPeriodicityInterval());
            timer_mgr.ActivateTimer(periodicityTimer);
        }
    }
    else 
    {
        periodicityTimer.Deactivate();
        periodicityTimer.SetInterval(GetNextPeriodicityInterval());
        timer_mgr.ActivateTimer(periodicityTimer);        
    }

    char msgBuffer[8192];
    char tmpBuffer[512];
    sprintf(msgBuffer,"type>Periodic action>start ubi>%lu eventSource>%s cnt>%d",ubi,GetStringFromEventSource(eventSource),periodicityCount);
    if (rapr.GetVerbosity())
    {
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
    
        if (periodicityInterval)
        {
            sprintf(tmpBuffer," periodicityInterval>%.2f",periodicityTimer.GetInterval());
            strcat(msgBuffer,tmpBuffer);
        }      
        if (GetSeed())
        {
            sprintf(tmpBuffer," seed>%ld",GetSeed());
            strcat(msgBuffer,tmpBuffer);
        }
        
    }
    rapr.LogEvent("RAPR",msgBuffer);

    return true;
} // end Periodic::OnStartUp()

double Periodic::GetNextPeriodicityInterval()
{
    if (exponential)
      return(-log(((double)prng->GetRand())/((double)RAND_MAX))*periodicityInterval);
    else
      return periodicityInterval;

} // end GetNextPeriodicityInterval()

bool Periodic::Stop()
{
  // log event
  char msgBuffer[8192];
  char tmpBuffer[512];
  sprintf(msgBuffer,"type>Periodic action>stopped ubi>%lu",ubi);

  if (rapr.GetVerbosity())
    {
      if (raprFlowID)
	{
	  sprintf(tmpBuffer," raprFlowID>%d",raprFlowID);
	  strcat(msgBuffer,tmpBuffer);
	}
    }
  
  rapr.LogEvent("RAPR",msgBuffer);

  if (endBehaviorTimer.IsActive()) endBehaviorTimer.Deactivate();
  if (periodicityTimer.IsActive()) periodicityTimer.Deactivate();
  return true;
  
} // End Periodic::Stop()

bool Periodic::ProcessEvent(const MgenMsg& theMsg)
{
  // Periodics shouldn't get any response!
  return false;
} // end Periodic::ProcessEvent

void Periodic::ClearState()
{
  DMSG(3,"Periodic::ClearState() \n");

  if (endBehaviorTimer.IsActive()) endBehaviorTimer.Deactivate();
  if (periodicityTimer.IsActive()) periodicityTimer.Deactivate();
  periodicityInterval = 0;
  behaviorEndBehaviorTime = 0;
  behaviorEventType = INVALID_TYPE;
  behaviorPattern[0] = '\0';
  periodicityCount = 0;
  BehaviorEvent::ClearState();


} // end Periodic::ClearState

bool Periodic::OnBehaviorTimeout(ProtoTimer& /*theTimer*/)
{
  // log event
  char msgBuffer[8192];
  sprintf(msgBuffer,"type>Periodic action>timeout ubi>%lu ",ubi);
  rapr.LogEvent("RAPR",msgBuffer);

  if (endBehaviorTimer.IsActive()) endBehaviorTimer.Deactivate();
  if (periodicityTimer.IsActive()) periodicityTimer.Deactivate();
  // ljt come back to this after first deliverable

  ClearState(); // ljt??
  //flowID = 0;
  //if (timeoutLogicID)
  //rapr.LogicTable().DoLogicID(timeoutLogicID,(BehaviorEvent*)this);

  return false; // ProtoTimerMgr will reschedule timer if true!

} // end Periodic::OnBehaviorTimeout
bool Periodic::OnPeriodicityTimeout(ProtoTimer& theTimer)
{
  if (periodicityTimer.IsActive()) 
    {
      periodicityTimer.SetInterval(periodicityInterval);
      periodicityTimer.Reschedule();
      //log event
      char msgBuffer[8192];
      sprintf(msgBuffer,"type>Periodic action>periodicityTimeout ubi>%lu",ubi);
      rapr.LogEvent("RAPR",msgBuffer);
      // ljt check for success
      OnStartUp();
      return false; // Otherwise ProtoTimerMgr tries to reinstall the timer
    }
  else 
    {
      DMSG(0,"Periodic::OnPeriodictiyTimeout Error: No active periodicity timer.  Report program bug!\n");
      return false;
    }

  return false;

} // end Periodic::OnPeriodicityTimeout

ReceptionEvent::ReceptionEvent(ProtoTimerMgr& timerMgr,
		 unsigned long ubi,
		 Rapr& theRapr)
  : BehaviorEvent(timerMgr,ubi,theRapr),
    event_type(DrecEvent::INVALID_TYPE),group_addr(),
    port_count(0),port_list(NULL),rx_buffer_size(0)
{
  type = RECEPTIONEVENT;
  interface_name[0] = '\0';
}

ReceptionEvent::~ReceptionEvent()
{
  if (endBehaviorTimer.IsActive()) endBehaviorTimer.Deactivate();

}

char* ReceptionEvent::BuildDrecMsg(char* buffer,DrecEvent::Type type)
{
  char tmpBuffer[512];

  sprintf(tmpBuffer,"%s",GetStringFromType(type));
  strcat(buffer,tmpBuffer);

  if (event_type == DrecEvent::JOIN)
    {
      sprintf(tmpBuffer," %s",group_addr.GetHostString());
      strcat(buffer,tmpBuffer);
    }
  const char* iface = GetInterface();

  if (iface)
    {
      sprintf(tmpBuffer," INTERFACE %s",iface);
      strcat(buffer,tmpBuffer);
    }

  if (port_count && (event_type == DrecEvent::JOIN))
    {
      sprintf(tmpBuffer," PORT %hu ",port_count);
      strcat(buffer,tmpBuffer);
    }

  if (protocol)
    {
      sprintf(tmpBuffer," %s ",GetStringFromProtocol(protocol));
      strcat(buffer,tmpBuffer);
    }

  if (event_type == DrecEvent::LISTEN)
    {
      for (unsigned short i = 0; i < port_count; i++)
	{
	  sprintf(tmpBuffer,"%hu,",port_list[i]);
	  strcat(buffer,tmpBuffer);
	}
    }
  if (rx_buffer_size)
    {
      sprintf(tmpBuffer," RXBUFFER %u",rx_buffer_size);
      strcat(buffer,tmpBuffer);
    }

  return buffer;
} // end ReceptionEvent::BuildDrecMsg()

bool ReceptionEvent::OnStartUp()
{
    char buffer[8192] = "\0";
    char msgBuffer[8192];
    char tmpBuffer[512];
    
    BuildDrecMsg(buffer,event_type);
    rapr.SendMgenCommand("event",buffer);
    
    // log event
    sprintf(msgBuffer,"type>Reception action>start ubi>%lu eventSource>%s",ubi,GetStringFromEventSource(eventSource));
    
    if (rapr.GetVerbosity())
    {
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
    }
    
    sprintf(tmpBuffer," mgenCmd>\"%s\"",buffer);
    strcat(msgBuffer,tmpBuffer);
    
    rapr.LogEvent("RAPR",msgBuffer);
    
    if (behaviorEndTime != 0.0) 
    {
        endBehaviorTimer.SetListener(this,&ReceptionEvent::OnBehaviorTimeout);
        ActivateEndTimer(endBehaviorTimer);
    }
    else 
    {
        // ljt what to do?OnBehaviorTimeout(endBehaviorTimer);
    }
    return true;
    
} // end ReceptionEvent::OnStartUp()

bool ReceptionEvent::Stop()
{

  char buffer[8192] = "\0";
  DrecEvent::Type eventType;

  if (event_type == DrecEvent::JOIN)
    eventType = DrecEvent::LEAVE;
  else
    if (event_type == DrecEvent::LISTEN)
      eventType = DrecEvent::IGNORE_;

  BuildDrecMsg(buffer,eventType);
  rapr.SendMgenCommand("event",buffer);

  // log event
  char msgBuffer[8192];
  char tmpBuffer[512];
  sprintf(msgBuffer,"type>Reception action>stop ubi>%lu",ubi);
  if (raprFlowID)
    {
      sprintf(tmpBuffer," raprFlowID>%d",raprFlowID);
      strcat(msgBuffer,tmpBuffer);
    }

  sprintf(tmpBuffer," mgenCmd>\"%s\"",buffer);
  strcat (msgBuffer,tmpBuffer);
  rapr.LogEvent("RAPR",msgBuffer);

  if (endBehaviorTimer.IsActive()) endBehaviorTimer.Deactivate();

  BehaviorEvent::ClearState();

  return true;

} // end ReceptionEvent::Stop()

bool ReceptionEvent::ProcessEvent(const MgenMsg& theMsg)
{
  // ReceptionEvents shouldn't get responses!
  return false;

} // end ReceptionEvent::ProcessEvent

void ReceptionEvent::ClearState()
{
  DMSG(3,"ReceptionEvent::ClearState() \n");
  if (endBehaviorTimer.IsActive()) endBehaviorTimer.Deactivate();

  event_type = DrecEvent::INVALID_TYPE;
  group_addr.Invalidate();
  interface_name[0] = '\0';
  port_count = 0;
  port_list = 0;
  rx_buffer_size = 0;

  BehaviorEvent::ClearState();

} // end ReceptionEvent::ClearState()

bool ReceptionEvent::OnBehaviorTimeout(ProtoTimer& /*theTimer*/)
{

  char buffer[8192] = "\0";
  DrecEvent::Type eventType;

  if (event_type == DrecEvent::JOIN)
    eventType = DrecEvent::LEAVE;
  else
    if (event_type == DrecEvent::LISTEN)
      eventType = DrecEvent::IGNORE_;

  BuildDrecMsg(buffer,eventType);
  rapr.SendMgenCommand("event",buffer);

  // log event
  char msgBuffer[512];
  sprintf(msgBuffer,"type>Reception action>timeout ubi>%lu ",ubi);
  strcat (msgBuffer,buffer);
  rapr.LogEvent("RAPR",msgBuffer);

  if (endBehaviorTimer.IsActive()) endBehaviorTimer.Deactivate();

  ClearState();

  return false;

} // end ReceptionEvent::OnBehaviorTimeout

const char* ReceptionEvent::GetStringFromType(DrecEvent::Type theType)
{
    const StringMapper* m = DrecEvent::TYPE_LIST;
    while (INVALID_OPTION != m->key)
    {
        if (theType == (DrecEvent::Type)(m->key))
            return m->string;
        else
            m++;  
    }
    return "INVALID";
} // end ReceptionEvent::GetStringFromType()


bool ReceptionEvent::ParseDrecOptions(const char* string)
{
  // ljt should probably put this in a common
  // mgen/rapr library

  const char* ptr = string;
  // skip any leading white space
  while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;

  char fieldBuffer[SCRIPT_LINE_MAX];
  if (1 != sscanf(ptr,"%s", fieldBuffer))
    {
      DMSG(0,"ReceptionEvent::ParseDrecOptions() Error: empty string\n");
      return false;
    }

  event_type = DrecEvent::GetTypeFromString(fieldBuffer);
  if (DrecEvent::INVALID_TYPE == event_type)
    {
      DMSG(0,"ReceptionEvent::ParseDrecOptions() Error: invalid <eventType> \n");
      return false;
    }

  // Default behavior is if no time is specified
  // ALWAYS process the rapr event.  
  if (((RaprEvent*)this)->GetBehaviorStartTime() < 0)
    {
      ((RaprEvent*)this)->SetOverrideStartTime(true);
    }
  else 
    {  
      ((RaprEvent*)this)->SetOverrideStartTime(false);
    }

  ptr += strlen(fieldBuffer);
  while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;

  switch (event_type)
    {
    case DrecEvent::JOIN:    // "{JOIN|LEAVE} <groupAddr> [<interfaceName>][PORT <port>]"
      //Leaves handled by the behavior object 
      if (1 != sscanf(ptr, "%s", fieldBuffer))
	{
	  DMSG(0, "ReceptionEvent::ParseDrecOptions() Error: missing <groupAddress>\n");
	  return false;
	}
      if (!group_addr.ResolveFromString(fieldBuffer) ||
	  !group_addr.IsMulticast())
	{
	  DMSG(0, "ReceptionEvent::ParseDrecOptions() Error: invalid <groupAddress>\n"); 
	  return false; 
	}
      // Point to next field, skipping any white space
      ptr += strlen(fieldBuffer);
      while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
      
      // Look for options [INTERFACE <interfaceName>] or [PORT <port>]
      while ('\0' != *ptr)
	{
	  // Read option label
	  if (1 != sscanf(ptr, "%s", fieldBuffer))
	    {
	      DMSG(0, "ReceptionEvent::ParseDrecOptions() Error: invalid JOIN/LEAVE option.\n");
	      return false;   
	    }
	  DrecEvent::Option option = DrecEvent::GetOptionFromString(fieldBuffer);
	  // Point to next field, skipping any white space
	  ptr += strlen(fieldBuffer);
	  while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
	  switch (option)
	    {
	    case DrecEvent::INTERFACE:
	      // Read <interfaceName>
	      if (1 != sscanf(ptr, "%s", fieldBuffer))
		{
		  DMSG(0, "ReceptionEvent::ParseDrecOptions() Error: missing <interfaceName>\n");
		  return false;   
		}
	      strncpy(interface_name, fieldBuffer, 15);
	      interface_name[15] = '\0';  // 16 char max
	      // Point to next field, skipping any white space
	      ptr += strlen(fieldBuffer);
	      while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
	      break;
	      
	    case DrecEvent::PORT:
	      // Read <portNumber>
	      if (1 != sscanf(ptr, "%s", fieldBuffer))
		{
		  DMSG(0, "ReceptionEvent::ParseDrecOptions() Error: missing <portNumber>\n");
		  return false;   
		}
	      if (1 != sscanf(ptr, "%hu", &port_count))
		{
		  DMSG(0, "ReceptionEvent::ParseDrecOptions() Error: invalid <portNumber>\n");
		  return false; 
		}
	      // Point to next field, skipping any white space
	      ptr += strlen(fieldBuffer);
	      while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
	      break;
	      
	    case DrecEvent::RXBUFFER:
	      break; //RXBUFFER is not a leave option, however this case gets rid of a compiler warning.                     
	    case DrecEvent::INVALID_OPTION:
	      DMSG(0, "ReceptionEvent::ParseDrecOptions() Error: invalid option: %s\n",
		   fieldBuffer);
	      return false;
	      break;
	    }
	}  // end while ('\0' != *ptr)
      break;
      
    case DrecEvent::LISTEN:  // "{LISTEN|IGNORE} <protocol> <portList>"
      // Ignore handled by behavior event case DrecEvent::IGNORE_:  

      // Get <protocol>
      if (1 != sscanf(ptr, "%s", fieldBuffer))
	{
	  DMSG(0, "ReceptionEvent::ParseDrecOptions() Error: missing <protocolType>\n");
	  return false;
	}
      protocol = GetProtocolFromString(fieldBuffer);
      if (INVALID_PROTOCOL == protocol)
	{
	  DMSG(0, "ReceptionEvent::ParseDrecOptions() Error: invalid <protocolType>\n");
	  return false;
	}
      // Point to next field, skipping any white space
      ptr += strlen(fieldBuffer);
      while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
      // Get <portList>
      if (1 != sscanf(ptr, "%s", fieldBuffer))
	{
	  DMSG(0, "ReceptionEvent::ParseDrecOptions() Error: missing <portList>\n");
	  return false;
	}
      // Parse port list and build array
      if (!(port_list = DrecEvent::CreatePortArray(fieldBuffer, &port_count)))
	{
	  DMSG(0, "ReceptionEvent::ParseDrecOptions() Error: invalid <portList>\n");
	  return false;
	}
      
      // Point to next field, skipping any white space
      ptr += strlen(fieldBuffer);
      while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
      
      //Is rx socket buffer size specified?
      if (1 != sscanf(ptr, "%s", fieldBuffer))
	break;  // No Rx socket buffer size specified.
      
      if (!strcmp("RXBUFFER",fieldBuffer))
	{
	  if (1 != sscanf(ptr, "%s", fieldBuffer))
	    {
	      DMSG(0, "ReceptionEvent::ParseDrecOptions() Error: missing <socket Rx  buffer size>\n");
	      return false;   
	    }
	  
	  // Point to next field, skipping any white space
	  ptr += strlen(fieldBuffer);
	  while ((' ' == *ptr) || ('\t' == *ptr)) ptr++;
	  
	  unsigned int rxBuffer; 
	  if (1 != sscanf(ptr, "%u", &rxBuffer))
	    { 
	      DMSG(0, "ReceptionEvent::ParseDrecOptions() Error: invalid <socket rx buffer size>\n");
	      return false;
	    }
	  
	  // (TBD) check validity
	  rx_buffer_size = rxBuffer;
	}
      break;
    case DrecEvent::INVALID_TYPE:
      ASSERT(0);  // this should never occur
      return false;
    default:
      DMSG(0,"ReceptionEvent::ParseDrecOptions() Error: Invalid type %s\n",GetStringFromType(event_type));
      return false;
    }  // end switch(event_type)
  return true;

} // end ReceptionEvent::ParseDrecOptions



BehaviorEventList::BehaviorEventList()
  : head(NULL), tail(NULL)
{
}

BehaviorEventList::~BehaviorEventList()
{
  Destroy();
} 

void BehaviorEventList::StopEvents()
{
  BehaviorEvent* next = head;
  while (next)
    {
      BehaviorEvent* current = next;
      next = next->next;
      if (current->GetType() != BehaviorEvent::RECEPTIONEVENT)
	current->Stop();
    }
}

void BehaviorEventList::Destroy()
{
  BehaviorEvent* next = head;
  while (next)
    {
      BehaviorEvent* current = next;
      next = next->next;
      delete current;
    }
  head = tail = NULL;

} // end BehaviorEventList::Destroy()

/*!
Time ordered insertion of event
*/
void BehaviorEventList::Insert(BehaviorEvent* theEvent)
{
  BehaviorEvent* next = head;

  double eventTime = theEvent->GetBehaviorStartTime();
  while (next)
    {

      DMSG(5,"BehaviorEvent.cpp::Insert nextUbi>%d startTime>%.2f\n",next->ubi,next->GetBehaviorStartTime());

      if (eventTime < next->GetBehaviorStartTime())
      {
          theEvent->next = next;
          if ((theEvent->prev = next->prev))
            theEvent->prev->next = theEvent;
          else
            head = theEvent;
          next->prev = theEvent;
          
          next = head;
          while (next)
          {
              DMSG(3,"INSERTED BehaviorEvent.cpp:Insert nextUbi>%d startTime>%.2f\n",next->ubi,next->GetBehaviorStartTime());
              next = next->next;
          }
          
          
          return;	  
      }
      else
      {
          next = next->next;
      }
    }
  // Fell to end of list
  if ((theEvent->prev = tail))
  {
      tail->next = theEvent;
  }
  else 
  {
      head = theEvent;
      theEvent->prev = NULL;
  }
  tail = theEvent;
  theEvent->next = NULL;
  
  next = head;
  while (next)
  {
      DMSG(3,"AFTER BehaviorEvent.cpp:Insert nextUbi>%d startTime>%.2f\n",next->ubi,next->GetBehaviorStartTime());
      next = next->next;
  }
  
} // end BehaviorEventList::Insert()

/*!
This places "theEvent" _before_ "nextEvent" in the list.
If "nextEvent" is NULL, "theEvent" goes to the end of the list.
*/
void BehaviorEventList::Precede(BehaviorEvent* nextEvent,
				BehaviorEvent* theEvent)
{
    
    BehaviorEvent* next = nextEvent;
    
    while (next)
    {
        DMSG(3,"BehaviorEvent.cpp:Preceding nextUbi>%d\n",next->ubi);
        next = next->next;
    }
    
    if ((theEvent->next = nextEvent))
    {
        if ((theEvent->prev = nextEvent->prev))
          theEvent->prev->next = theEvent;
        else
          head = theEvent;
        nextEvent->prev = theEvent;
    }
    else
    {
        if ((theEvent->prev = tail))
          tail->next = theEvent;
        else
          head = theEvent;
        tail = theEvent;   
    }
    
} // end BehaviorEventList::Precede()

void BehaviorEventList::Remove(BehaviorEvent* theEvent)
{
    if (theEvent->prev)
      theEvent->prev->next = theEvent->next;
    else
      head = theEvent->next;
    if (theEvent->next)
      theEvent->next->prev = theEvent->prev;
    else 
      tail = theEvent->prev;
    
} // end BehaviorEventList::Remove()

BehaviorEvent* BehaviorEventList::FindBEByUBI(unsigned long inUBI)
{
    BehaviorEvent* next = head;
    while (next)
    {
        if (next->GetUBI() == inUBI)
        {
            return next;
        }
        next = next->next;
    }
    return (BehaviorEvent*)NULL;
    
} // end BehaviorEventList::FindBEByUBI

BehaviorEvent* BehaviorEventList::FindBEByStreamUBI(unsigned long inUBI)
{
    BehaviorEvent* next = head;
    while (next)
    {
        if ((next->GetType() == BehaviorEvent::STREAM) && (((Stream*)next)->GetStreamUbi() == inUBI))
        {
            return next;
        }
        next = next->next;
    }
    return (BehaviorEvent*)NULL;
    
} // end BehaviorEventList::FindBEByStreamUBI

BehaviorEvent* BehaviorEventList::FindBEByFlowID(int inFlowID)
{
    BehaviorEvent* next = head;
    while (next)
    {
        if (next->GetFlowID() == inFlowID)
        {
            return next;
        }
        next = next->next;
    }
    return (BehaviorEvent*)NULL;
    
} // end BehaviorEventList::FindBEByFlowId()

BehaviorEvent* BehaviorEventList::FindUnusedBehaviorEvent(BehaviorEvent::Type objectType)
{
    BehaviorEvent* next = head;
    while (next)
    {
        if (next->InUse() == false && next->GetType() == objectType)
        {
            return next;	}
        next = next->next;
    }
    return (BehaviorEvent*)NULL;
    
    
} // BehaviorEventList::FindBehaviorEvent

BehaviorEvent* BehaviorEventList::FindBEByRaprFlowID(int inRaprFlowID)
{
    BehaviorEvent* next = head;
    while (next)
    {
        if (next->GetRaprFlowID() == inRaprFlowID)
        {
            return next;
        }
        next = next->next;
    }
    return (BehaviorEvent*)NULL;
    
} // end BehaviorEventList::FindBEByRaprFlowID

BehaviorEvent* BehaviorEventList::FindBEByEventType(BehaviorEvent::Type inType,BehaviorEvent* theTrigger)
{
    BehaviorEvent* next = head;
    while (next)
    {
        if ((next->GetType() == inType) ||
            (inType == BehaviorEvent::ALL) &&
            (next != theTrigger))
        {
            return next;
        }
        next = next->next;
    }
    return (BehaviorEvent*)NULL;
    
} // end BehaviorEventList::FindBEByEventType
