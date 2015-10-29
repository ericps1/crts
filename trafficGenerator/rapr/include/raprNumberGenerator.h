#ifndef _NUMBER_GENERATOR
#define _NUMBER_GENERATOR

#ifndef STLPORT
#include <map>
#include <set>
#else
#include "map"
#include "set"
#endif

#ifndef SPRNG2
#include "sprng_cpp.h"
#else
#include "sprng.h"
#endif

#include "protoAddress.h"
#include <math.h>
#include "raprJournaller.h"

struct ltstr
{
  bool operator()(const char* s1, const char* s2) const
  {
    if (s1 == NULL) return true;
    if (s2 == NULL) return false;
    return strcasecmp(s1, s2) < 0;
  }
};

#ifndef STLPORT
typedef std::set<int> FlowIDAvailable;
typedef std::multimap<char *,int,ltstr> FlowIDAssigned;
typedef std::map<int,char *> FlowIDInUse;
#else
typedef _STL::set<int> FlowIDAvailable;
typedef _STL::multimap<char *,int,ltstr> FlowIDAssigned;
typedef _STL::map<int,char *> FlowIDInUse;
#endif

class RaprNumberGenerator {
   public:
      RaprNumberGenerator();
      ~RaprNumberGenerator();
      //UBI set and get
      void SetUBI(unsigned long inUBI);
      void SetSequence(unsigned long Seq);
      unsigned long GetUBI();
      short GetHostIDFromUBI(unsigned long inUBI);
      void SetHostID(short host);
      short GetHostID();
      void SetInitialSeed(int initial_seed) 
      { 
          initialSeed = initial_seed; 
          InitializeSeeds(initialSeed);
      };
      int  GetInitialSeed() {return initialSeed;};
      char *FormatUBI(unsigned int inUBI);
      //FlowID set and get
      void SetFlowID(int inFlow);
      void SetFlowID(int inFlowStart,int inFlowEnd);
      int GetFlowID(ProtoAddress *srcAddr,ProtoAddress *dstAddr);
      int GetFlowID() { return GetFlowID("NONE"); }
      int GetSequenceNumber(ProtoAddress& srcAddr,ProtoAddress& dstAddr);
      void UnlockFlowID(int inFlow);
      //Journal options
      int GetStreamID() {return streamID++;}
      void SetStreamID(int inID) {streamID=inID;}
      void InitializeSeeds(short hostID);
      void InitializeSeeds(int inSeed);
      void SetSeed (unsigned int inSeed);
      unsigned int GetSeed() {return runningSeed;}
      unsigned int GetNextSeed() {return runningSeed++;}
      void DecrementSeed() {runningSeed--;}
      void SetRtiSeed(unsigned int inRtiSeed);
      unsigned int GetRtiSeed() {return runningRtiSeed;}
      unsigned int GetNextRtiSeed() {return runningRtiSeed++;}
      void DecrementRtiSeed() {runningRtiSeed++;}
      void FileOutput(FILE *inStream);
      void FileInput(FILE *inStream);

   private:
      int GetFlowID(char *);
      void LogFlowID(char *inKey,int inFlow);

      FlowIDAvailable flowAvail;
      FlowIDAssigned  flowAssign;
      FlowIDInUse     flowUsed;
      unsigned long   hostID;
      unsigned long   seqNumber;
      unsigned long   SEQMASK;
      int             streamID;
      int             initialSeed;
      unsigned int    runningSeed;
      unsigned int    runningRtiSeed;
      char *          FlowIDAssign;
      int             FlowIDRecNum;
      static const int RECSIZE = 50;
      static const int OK_FLAG = 0xF0F0;
      timeval         lastupdate;

};

class RaprPRNG {
 public:
  RaprPRNG(unsigned int inSeed);
  RaprPRNG();
  ~RaprPRNG();
  void SetSeed(unsigned int inSeed);
  unsigned int GetSeed() {return seed;}
  unsigned int GetRand();
  unsigned int GetRand(unsigned lower,unsigned int upper);
  double GetRandom();
  float GetRandom(float lower,float upper);
  unsigned int GetDraw() {return draw;}

 private:
  static const int NSTREAMS_DEFAULT = 2;
  static const int SPRNG_TYPE_DEFAULT = 1;
  unsigned int 	seed;
  unsigned int 	draw;
  int 		streamnum;
#ifndef SPRNG2
  Sprng *	stream;
#else
  int*          stream;
#endif
};

#endif // _NUMBER_GENERATOR
