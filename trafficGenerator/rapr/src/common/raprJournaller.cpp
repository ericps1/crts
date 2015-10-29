#include "raprJournaller.h"
#include "rapr.h"

//DEfault constructor, set the repeat timer to default interval
RaprJournaller::RaprJournaller() {
   journalTimer.SetListener(this,&RaprJournaller::OnAction);
   journalTimer.SetInterval(TIMERINTERVAL);
   journalTimer.SetRepeat(-1);
   restart = false;
   locktimer = false;
}

//Constructor with reference to ProtoTimerMgr and Rapr, both needed to function
RaprJournaller::RaprJournaller(ProtoTimerMgr& timerMgr,Rapr& rapr) {
   journalTimer.SetListener(this,&RaprJournaller::OnAction);
   journalTimer.SetInterval(TIMERINTERVAL);
   journalTimer.SetRepeat(-1);
   restart = false;
   mgr = &timerMgr;
   raprPtr = &rapr;
   locktimer = false;
}

//Set the timer manager if not already
void RaprJournaller::SetProtoTimer(ProtoTimerMgr &timerMgr) {
   mgr = &timerMgr;
}

//Set the reference to Rapr
void RaprJournaller::SetRapr(Rapr& rapr) {
   raprPtr = &rapr;
}

//turn off the timer;
RaprJournaller::~RaprJournaller() {
    if (mgr != NULL) {
        mgr->DeactivateTimer(journalTimer);
    }
}

//Set the timer interval
void RaprJournaller::SetTimer(int inTime) {
    if (locktimer == false) { 
        if (inTime >= 0) {
            journalTimer.SetInterval(inTime);
        }
    }
}

//go into restart mode
void RaprJournaller::SetRestart() {
    restart = true;
}

//Set the files to be journalled, also starts the journalling process
void RaprJournaller::SetFile(const char *arg) {
    char *word;
    char *brk;
    char *sep = " \t\r\n";
    char tmp[512];
    char *word2;
    FILE *jFile;
    strncpy(tmp,arg,512);
    word = strtok_r(tmp,sep,&brk);
    while (word != NULL) {
        word2=new char[strlen(word)+1];
        memset(word2,0,strlen(word)+1);
        strcpy(word2,word);
        files.push_back(word2);
        //open for read if it is a restart
        if (restart == true) {
            jFile = fopen(word2,"rb");
            if (jFile != NULL) {
                (raprPtr->GetNumberGenerator()).FileInput(jFile);
                MgenSequencer::FileInput(jFile);
                fclose(jFile);
            }
        }
        word=strtok_r(NULL,sep,&brk);
    }
        
    if (mgr != NULL) {
        //activate the timer if it has not been done yet
        if (locktimer == false) {
           mgr->ActivateTimer(journalTimer);
           locktimer = true;
        }
    }
}

//On every interval Do journalling
bool RaprJournaller::OnAction(ProtoTimer &theTimer) {
   FILE *jFile;
   for (int i=0;i < (int)files.size();i++) {
       jFile = fopen(files[i],"wb");
       if (jFile != NULL) {
           if (ferror(jFile) == 0) {
               (raprPtr->GetNumberGenerator()).FileOutput(jFile);
               MgenSequencer::FileOutput(jFile);
           } else {
               DMSG(0,"RaprJournaller::OnAction - Can not journal to file %s.",files[i]);
           }
           fclose(jFile);
       }
   }
   return true;
}
