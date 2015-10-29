#ifndef _RAPR_PAYLOAD
#define _RAPR_PAYLOAD

#include "mgenPayload.h"
#include <string.h>

class RaprPayload {
 public:
  RaprPayload();
  RaprPayload(unsigned int inUBI);
  RaprPayload(unsigned int inUBI,unsigned int inLogic);
  RaprPayload(MgenPayload *inPayload);		 
  ~RaprPayload();
  void SetUBI(unsigned int inUBI);
  unsigned int GetUBI();
  unsigned int GetForeignUBI();
  void SetForeignUBI(unsigned int inForeignUBI);
  void SetLogicID(unsigned int inLogic);
  unsigned int GetLogicID();
  void SetSeed(unsigned int inSeed);
  unsigned int GetSeed();
  void SetHex(const char *inHex);
  char *GetHex();
  void SetStreamID(unsigned int inID);
  unsigned int GetStreamID();
  void SetStreamSeq(unsigned int inSeq);
  unsigned int GetStreamSeq();
  void SetStreamDuration(double inDuration);
  double GetStreamDuration();
  void SetBurstCount(unsigned int inCount);
  unsigned int GetBurstCount();
  void SetBurstPriority(unsigned int inPriority);
  unsigned int GetBurstPriority();
  void SetBurstPayloadID(unsigned int inPayloadID);
  unsigned int GetBurstPayloadID();
  void SetOrigin(unsigned int inOrigin);
  unsigned int GetOrigin();
  void operator=(RaprPayload* aRaprPayload);
 private:
  unsigned int ubi;
  unsigned int foreignUBI;
  unsigned int logicid;
  unsigned int origin;
  unsigned int seed;
  unsigned int streamid;
  unsigned int streamseq;
  double duration;
  unsigned int burstCount;
  unsigned int burstPriority;
  unsigned int burstPayloadID;
  bool validUBI;
  bool validForeignUBI;
  bool validLogicID;
  bool validOrigin;
  bool validSeed;
  bool validStreamID;
  bool validStreamSeq;
  bool validStreamDuration;
  bool validBurstCount;
  bool validBurstPriority;
  bool validBurstPayloadID;
};

#endif //_RAPR_PAYLOAD
