#ifndef _RAPR_JOURNALER
#define _RAPR_JOURNALER

#ifndef STLPORT
#include <vector>
#else
#include "vector"
#endif

#include "protokit.h"
#include "mgenSequencer.h"

class Rapr;

class RaprJournaller
{
   public:
      static const int TIMERINTERVAL = 10;
      RaprJournaller();
      RaprJournaller(ProtoTimerMgr& timerMgr,Rapr& rapr);
      ~RaprJournaller();
      void SetProtoTimer(ProtoTimerMgr &timerMgr);
      void SetRapr(Rapr& rapr);
      void SetTimer(int inTime);
      bool OnAction(ProtoTimer &theTimer);
      void SetFile(const char *arg);
      void SetRestart();
   private:
      ProtoTimer journalTimer;
      ProtoTimerMgr *mgr;
      Rapr *raprPtr;
      bool restart;
      bool locktimer;
#ifndef STLPORT
      std::vector<char *> files;
#else
      _STL::vector<char *> files;
#endif
};

#endif //_RAPR_JOURNALE
