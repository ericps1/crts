#include "raprLogicTable.h"
#include "rapr.h"

#include <math.h>
#include "tinyxml.h"
#include <time.h>

RaprLogicState::RaprLogicState() {
}

//output to a "1.2.3.4" formatted string
char *RaprLogicState::ToString(unsigned int inUBI) {
	char *res = 0;
	char *cur = 0;
	char *tmp;
	int size;
	for (int i=0;i<(int)(key.size());i++) {
		if (i != (int)(key.size()-1)) {
			//alloc the space for "1."
		  // size=(int)trunc(log10(key[i] == 0 ? 1 : key[i]))+3;
		  size=(int)trunc(log10((double)(key[i] == 0 ? 1 : key[i])))+3;
			cur=new char[size];
			memset(cur,0,size);
			sprintf(cur,"%d",key[i]);
			strcat(cur,".");
		}
		else {
			//last pair does not have a trailing "."
		  //size=(int)trunc(log10(key[i] == 0 ? 1 : key[i]))+2;
		  size=(int)trunc(log10((double)(key[i] == 0 ? 1 : key[i])))+2;
			cur=new char[size];
			memset(cur,0,size);
			sprintf(cur,"%d",key[i]);
		}
		if (res != NULL) {
			//append the current pair to the old string
			tmp = new char[strlen(res) + strlen(cur) + 1];
			memset(tmp,0,strlen(res) + strlen(cur) + 1);
			strcpy(tmp,res);
			strcat(tmp,cur);
			delete[] cur;
		}
		else {
			//create a new string
			tmp = new char[strlen(cur) + 1];
			memset(tmp,0,strlen(cur) + 1);
			strcpy(tmp,cur);
			delete[] cur;
		}
		delete res;  // jm: wha?
		res = tmp;
	}
	int ubi = ubiState.GetState(inUBI);
	//size=(int)trunc(log10(ubi == 0 ? 1 : ubi))+3;
	size=(int)trunc(log10((double)(ubi == 0 ? 1 : ubi)))+3;
	cur=new char[size];			// new cur alloc'd
	memset(cur,0,size);
	sprintf(cur,",%d",ubi);
	tmp = new char[strlen(res) + strlen(cur) + 1];
	memset(tmp,0,strlen(res) + strlen(cur) + 1);
	strcpy(tmp,res);
	strcat(tmp,cur);
	delete[] res;
	res = tmp;

	return res;
}

//hash function
int RaprLogicState::ToHash(unsigned int inUBI) {
	char *tmp = ToString(inUBI);
#ifndef STLPORT
	std::tr1::hash<std::string> H;
        std::string Str=std::string(tmp);	
	int res = H(Str);
#else
	_STL::hash<char *> H;
	int res = H(tmp);
#endif

        //DMSG(0, "RaprLogicState::ToHash() : UBI Source=%s Hash=%d.\n",tmp,res);
	delete[] tmp;	// from new[] in ToString
	return res;
}

int RaprLogicState::ToHash(char *inState) {
#ifndef STLPORT
	std::tr1::hash<std::string> H;
        std::string Str=std::string(inState);
	int res = H(Str);
#else
	_STL::hash<char *> H;
	int res = H(inState);
#endif
        //DMSG(0, "RaprLogicState::ToHash() : Source=%s Hash=%d.\n",inState,res);
	return res;
}

//convert "1.2.3.4" to internal state holder
void RaprLogicState::FromString(char *inState) {
	char *brk;
	char *word;
	char *locale;
	char *sep = ".";
	//copy the string prep it for tokenization
	locale=new char[strlen(inState) + 1];
	memset(locale,0,strlen(inState) + 1);
	strcpy(locale,inState);
	key.resize(0);
	//tokenize the string
	word = strtok_r(locale,sep,&brk);
	while (word != NULL) {
		key.push_back(atoi(word));
		word=strtok_r(NULL,sep,&brk);
	}

	delete[] locale;
}

void RaprLogicState::SetName(char *inName) {
	SetName(inName,ToString());
}

void RaprLogicState::SetName(char *inName,char *inState) {
	char *locale;
	char *tmpState;
	//both buffer will be in the map and delete when map is deleted
	if ((inName == NULL) || (inState == NULL)) return;
	locale=new char[strlen(inName) + 1];
	memset(locale,0,strlen(inName) + 1);
	strcpy(locale,inName);
	tmpState=new char[strlen(inState) + 1];
	memset(tmpState,0,strlen(inState) + 1);
	strcpy(tmpState,inState);
	if (keymap[locale] != NULL) {
		delete keymap[locale];
	}
	keymap[locale]=tmpState;
}

//change the state at the level specified.  0 is the first level
void RaprLogicState::SetState(int inLevel,unsigned int inState) {
	if (inLevel + 1 <= (int)(key.size())) {
		key.resize(inLevel+1);
		key[inLevel] = inState;
	}
	else {
		int size = inLevel-key.size();
		key.resize(inLevel+1);
		for (int i=0;i<size;i++) key.push_back(0);
		key.push_back(inState);
	}
}

//change the state to another named state
void RaprLogicState::SetState(char *inName) {
	if (inName == NULL) return;
	if (keymap[inName] == NULL) return;
	FromString(keymap[inName]);
}

RaprLogicTableEntry::RaprLogicTableEntry() {
	success=timeout=failure=0;
	percent=1;
}

RaprLogicTableEntry::~RaprLogicTableEntry() {
	for (int i=0;i<(int)(main.size());i++) {
		if (main[i] != NULL) {
			delete main[i];
		}
	}
}

void RaprLogicTableEntry::AddEntry(char *inEntry) {
	if (inEntry == NULL) return;
	char *locale=new char[strlen(inEntry) + 1];
	memset(locale,0,strlen(inEntry) + 1);
	strcpy(locale,inEntry);
	main.push_back(locale);
}

RaprStringVector *RaprLogicTableEntry::GetEntry() {
	RaprStringVector *res;
	if (main.empty()) return NULL;
	res = new RaprStringVector();
	for (int i=0;i<(int)(main.size());i++) {
		res->push_back(main[i]);
	}
	return res;
}

RaprLogicTable::RaprLogicTable(Rapr *theRapr)
  : rapr(theRapr)
{
	state.FromString("0");
	RaprLogicIDTable *current=new RaprLogicIDTable();
	table[state.ToHash()] = current;

}

RaprLogicTable::RaprLogicTable(Rapr *theRapr, char *inFile)
  : rapr(theRapr)
{
	state.FromString("0");
	RaprLogicIDTable *current=new RaprLogicIDTable();
	table[state.ToHash()] = current;
	if (!LoadFile(inFile))
	  DMSG(0, "RaprLogicTable::LoadFile() Error: can not load file logicTable file %s. \n",inFile);

}

RaprLogicTable::RaprLogicTable()
  : rapr(NULL)
{
	state.FromString("0");
	RaprLogicIDTable *current=new RaprLogicIDTable();
	table[state.ToHash()] = current;
}

RaprLogicTable::RaprLogicTable(char *inFile)
  : rapr(NULL)
{
	state.FromString("0");
	RaprLogicIDTable *current=new RaprLogicIDTable();
	table[state.ToHash()] = current;
	LoadFile(inFile);	
}

RaprLogicTable::~RaprLogicTable() {

}

//pass thru functions to state.  Then set the current pointer to the 
//new state
bool RaprLogicTable::SetStateFromString(char *inState) {
	if (inState != NULL) {
		state.FromString(inState);
	}
	return true;
}

bool RaprLogicTable::SetState(char *inName) {
	if (inName != NULL) {
		state.SetState(inName);
	}
	return true;
}

bool RaprLogicTable::SetState(int inLevel,int inState) {
	state.SetState(inLevel,inState);
	return true;
}

bool RaprLogicTable::SetUBIState(char *inUBI,char *inState) {
	unsigned int ubi = 0;
	int iState = 0;
	if ((inUBI == NULL) || (inState == NULL)) return false;
	ubi = strtoul(inUBI,NULL,10);
	if (ubi == 0) return false;
	iState = atol(inState);
	state.SetUBIState(ubi,iState);
	return true;
}

void RaprLogicTable::SetEntry(char *inState,int inID,RaprLogicTableEntry *inEntry) {
	RaprLogicIDTable *current;
	if (table[state.ToHash(inState)] == NULL) {
		current=new RaprLogicIDTable();
		table[state.ToHash(inState)] = current;
	}
	else {
		current = table[state.ToHash(inState)];
	}
	if ((*current)[inID] != NULL) {
		delete (*current)[inID];
	}
	(*current)[inID] = inEntry;
}
// Called when processing a mgen message
bool RaprLogicTable::DoLogicID(int inID,
			       const MgenMsg& inMsg,
			       const ProtoAddress& srcAddr,
			       BehaviorEvent* theBehaviorObject) 
{
  // BehaviorEvent handling not fully implemented yet 
  // Right now for stream object support
  // that isn't really being used yet.

  if (&inMsg == NULL) 
    {
      DMSG(0,"RaprLogicTable::DoLogicID() Error: No payload available in message.\n");
      return false;      
    }

  unsigned short payloadlen;
  payloadlen = inMsg.GetPayloadLen();
  if (payloadlen < 1) 
    {
      // We should always at least have a seed
      DMSG(0,"RaprLogicTable::DoLogicID() Error: No Rapr payload available.\n");
      return false;
    }

  RaprPayload *raprPayload = new RaprPayload();
  raprPayload->SetHex(inMsg.GetPayload());
  RaprDictionaryTransfer *trans = new RaprDictionaryTransfer();
  trans->SetPayload(raprPayload);
  RaprPRNG *prng;

  if (raprPayload->GetSeed() != 0) 
    {
      prng = new RaprPRNG(raprPayload->GetSeed());
      DMSG(5,"RaprLogicTable::DoLogicID Using payload seed>%d\n",raprPayload->GetSeed());
      trans->SetPRNG(prng);
    }
  else
    {
      if (theBehaviorObject != NULL && theBehaviorObject->GetSeed())
	{
	  prng = new RaprPRNG(theBehaviorObject->GetSeed());
	  DMSG(5,"RaprLogicTable::DoLogicID Using behavior event seed>%d",theBehaviorObject->GetSeed());

	  trans->SetPRNG(prng);
	    
	}
      else 
	{
	  DMSG(0,"RaprLogicTable::DoLogicID() Error: No incoming seed in payload! LogicID> %d failed.\n",inID);
	  return false;
	}
    }
  char *tmp;
  tmp = new char[20];
  memset(tmp,0,20);

  // ljt what should srcAddr be if we have
  // a behavior object (and therefore no srcAddr
  // from the socket?

  strcpy(tmp,(srcAddr.GetHostString()));
  trans->SetSrcIP(tmp);
  DMSG(4, "RaprLogicTable.DoLogic(MgenMsg) : SRCIP :%s\n",tmp);
  tmp = new char[20];
  memset(tmp,0,20);
  sprintf(tmp,"%d",(srcAddr.GetPort()));
  trans->SetSrcPort(tmp);
  DMSG(4, "RaprLogicTable.DoLogic(MgenMsg) : SRCPORT :%s\n",tmp);
  tmp = new char[20];
  memset(tmp,0,20);
  strcpy(tmp,(inMsg.GetDstAddr()).GetHostString());
  trans->SetDstIP(tmp);
  DMSG(4, "RaprLogicTable.DoLogic(MgenMsg) : DSTIP :%s\n",tmp);
  tmp = new char[20];
  memset(tmp,0,20);
  sprintf(tmp,"%d",(inMsg.GetDstAddr()).GetPort());
  trans->SetDstPort(tmp);
  DMSG(4, "RaprLogicTable.DoLogic(MgenMsg) : DSTPORT :%s\n",tmp);
  
  DoLogicIDInternal(inID,trans,theBehaviorObject);
  delete trans;  

  return true;
} // RaprLogicTable::DoLogicID

bool RaprLogicTable::DoLogicID(int inID,BehaviorEvent* theBehaviorEvent) 
{
  bool ret;
  RaprDictionaryTransfer *trans = new RaprDictionaryTransfer();
  RaprPRNG *prng = new RaprPRNG(theBehaviorEvent->GetSeed());
  DMSG(5,"RaprLogicTable::DoLogicID using behaviorevent seed>%d\n",prng->GetSeed());
  trans->SetPRNG(prng);
  ret = DoLogicIDInternal(inID,trans,theBehaviorEvent);
  delete trans;
  return ret;
}


RaprStringVector *RaprLogicTable::TranslateString(char* string,RaprPRNG* prng)
{
  RaprDictionaryTransfer *trans = new RaprDictionaryTransfer();
  trans->SetPRNG(prng);
  RaprStringVector *ret=dictionary.translate(string,trans);
  delete trans;
  return ret;
}

bool RaprLogicTable::DoLogicIDInternal(int inID,RaprDictionaryTransfer *trans,BehaviorEvent* theBehaviorEvent) 
{
    int i,j;
    RaprLogicIDTable *current;
    unsigned int ubi = 0;
    char msgBuffer[512];
    if (((trans != NULL) && (trans->GetPayload() != NULL))
        // if we have a behavior event don't set ubi state
        && (!theBehaviorEvent)) 
    {
        if (trans->GetPayload()->GetUBI() != 0) {
            ubi=trans->GetPayload()->GetUBI();
        }
        else if (trans->GetPayload()->GetForeignUBI() != 0) {
            ubi=trans->GetPayload()->GetForeignUBI();
        }
    }

     
    //DMSG(0, "RaprLogicTable::DoLogicIDInternal() : UBI %d.\n",ubi);
    current = table[state.ToHash(ubi)];
    if (current == NULL) 
      {

	if (theBehaviorEvent != NULL)
	  sprintf(msgBuffer,"type>error action>doLogicID logicID>%d eventSource>%s info>\"Logic state table is not found\"",inID,BehaviorEvent::GetStringFromEventSource(theBehaviorEvent->GetEventSource()));
	else
	  sprintf(msgBuffer,"type>error action>doLogicID logicID>%d info>\"Logic state table is not found\"",inID);

	rapr->LogEvent("RAPR",msgBuffer);
        DMSG(0, "RaprLogicTable::DoLogicIDInternal() Error: Table not found.\n");
        return false;
    }
    RaprLogicTableEntry *tableentry = (*current)[inID];
    if (tableentry == NULL)
    {
      if (theBehaviorEvent != NULL)
	sprintf(msgBuffer,"type>Error action>doLogicID logicID>%d eventSource>%s info>\"LogicID is not found in current state table\"",inID,BehaviorEvent::GetStringFromEventSource(theBehaviorEvent->GetEventSource()));
      else
	sprintf(msgBuffer,"type>Error action>doLogicID logicID>%d info>\"LogicID is not found in current state table\"",inID);

      rapr->LogEvent("RAPR",msgBuffer);
      DMSG(0,"RaprLogicTable::DoLogicIDInternal() Error: Table entry %d not found.  LogicID is not found in current state table\n",inID);
      return false;
    }
    
    tableentry = RandomizeEntry(tableentry,trans->GetPRNG());
    if (tableentry == NULL) 
    {
      if (theBehaviorEvent != NULL)
	sprintf(msgBuffer,"type>logicTable action>doLogicID logicID>%d eventSource>%s result>randTestFailure",inID,BehaviorEvent::GetStringFromEventSource(theBehaviorEvent->GetEventSource()));
      else
	sprintf(msgBuffer,"type>logicTable action>doLogicID logicID>%d result>randTestFailure",inID);
	rapr->LogEvent("RAPR",msgBuffer);

        DMSG(1,"RaprLogicTable::DoLogicIDInternal() Warning: Unable to randomize tableentry for ID>%d or event set to NULL due to randomness.\n",inID);
        return false;
    }  
    
    //this has the strings to be processed
    RaprStringVector *entry = tableentry->GetEntry();
    RaprStringVector *tmpList;
    if (entry->empty()) 
    {
      if (theBehaviorEvent != NULL)	
	sprintf(msgBuffer,"type>Error action>doLogicID logicID>%d eventSource>%s info>\"empty table entry for LogicID\"",inID,BehaviorEvent::GetStringFromEventSource(theBehaviorEvent->GetEventSource()));
      else
	sprintf(msgBuffer,"type>Error action>doLogicID logicID>%d info>\"empty table entry for LogicID\"",inID);

	rapr->LogEvent("RAPR",msgBuffer);

        DMSG(1,"RaprLogicTable::DoLogicIDInternal() Error: empty table entry for logicID>%d\n",inID);
        return false;
    }
    
    for (i=0;i < (int)(entry->size());i++) {
        tmpList = dictionary.translate((*entry)[i],trans);
        if (tmpList == NULL) 
	  {
            DMSG(0,"RaprLogicTable::DoLogicIDInternal() Error: unable to translate logic table entry for logic id>%d\n",inID);
	    if (theBehaviorEvent != NULL)
	      sprintf(msgBuffer,"type>Error action>doLogicID logicID>%d eventSource>%s info>\"unable to translate logic table entry\"",inID,BehaviorEvent::GetStringFromEventSource(theBehaviorEvent->GetEventSource()));
	    else
	      sprintf(msgBuffer,"type>Error action>doLogicID logicID>%d info>\"unable to translate logic table entry\"",inID);

	    rapr->LogEvent("RAPR",msgBuffer);
	    
            return false;
        }
        for (j=0;j < (int)(tmpList->size());j++) {
            DMSG(2, "RaprLogicTable.DoLogicInternal : Parsing :%s\n",(*tmpList)[j]);

            // Since we want to retranslate periodic events
            // each time we start up, make sure we only have
            // one periodic event definition to work with.
            // (Temporary solution)

            // Comparing to PERIODIC INT to deconflict with
            // PERIODIC [ pattern ljt
            if ((tmpList->size() > 1) 
                && (strstr((*tmpList)[j],"PERIODIC INT")))
                {

		  // ljt?? 01/14 what is this? parsing test? seems broken
                    DMSG(0,"Rapr::ParseScript() Error: PERIODIC behavior events cannot be translated into multiple events.\n");
                    DMSG(0, "Rapr::ParseScript() Error: invalid rapr logic table line: %s\n", (*entry)[i]);

		    sprintf(msgBuffer,"type>Error action>doLogicID logicID>%d info>\"Periodic events cannot be translated into multiple events\"",inID);
		    rapr->LogEvent("RAPR",msgBuffer);

                    return false;
                }


            // Now get a draw on the incoming seed
            // to use for any new behavior objects
            RaprPRNG *prng;
            prng = new RaprPRNG(trans->GetPRNG()->GetRand());
            if (theBehaviorEvent != NULL && theBehaviorEvent->GetType() == BehaviorEvent::STREAM)
            {
                if (theBehaviorEvent->GetType() == BehaviorEvent::STREAM)
                {
                    int tmp = 0;
                    theBehaviorEvent->ParseStreamOptions((*tmpList)[j],tmp);
                }
                else
                {  
                    DMSG(0,"RaprLogicTable.DoLogicInternal Error: Currently only Stream events are supported in the logic table.\n");
                }
            }
            else
            {
                if (rapr != NULL) 
                {
		  // If we don't have a behavior event, it's a net event
		  BehaviorEvent::EventSource event_source = BehaviorEvent::NET_EVENT;
		  if (theBehaviorEvent != NULL)
		    BehaviorEvent::EventSource event_source = theBehaviorEvent->GetEventSource();

		  sprintf(msgBuffer,"type>logicTable action>doLogicID logicID>%d eventSource>%s result>parsingEvent",inID,BehaviorEvent::GetStringFromEventSource(event_source));
		  rapr->LogEvent("RAPR",msgBuffer);

		  rapr->ParseEvent((*tmpList)[j],0,trans,event_source,prng,(*entry)[i]);
                }
                else
                {
                    DMSG(0,"RaprLogicTable::DoLogicInternal Error: no rapr object available!\n");
                }
            }
        }
    }
    
    return true;
}

//Handle Random on an entry by entry basis to begin with
//Evantually this will create a completely new entry to pass back
//Random at that point will be individual strings
RaprLogicTableEntry *RaprLogicTable::RandomizeEntry(RaprLogicTableEntry *inEntry,RaprPRNG *prng) {
	if (inEntry->GetPercent() >= prng->GetRandom()) {
		return inEntry;
	}
	else {
	  for (int i=0;i<(int)(inEntry->GetEntry()->size());i++)
	    {
	      
		DMSG(1, "RaprLogicTable.RandomizeEntry : Event set to NULL due to Randomness %s\n",(*inEntry->GetEntry())[i]);
	    }
		return NULL;
	}
}

bool RaprLogicTable::LoadFile(char *inFile) {
	TiXmlDocument doc(inFile);
	
	DMSG(4, "RaprLogicTable Load file %s.\n",inFile);
	bool loadOkay = doc.LoadFile();
	if ( !loadOkay )
	{
		DMSG(0, "RaprLogicTable::LoadFile() Error: can not load file %s.  Error=%s \n",inFile,doc.ErrorDesc());
		return false;
	}
	
	TiXmlNode* nodeL1 = 0;
	TiXmlNode* tmpNode = 0;
	TiXmlNode* nodeL2 = 0;
	TiXmlNode* nodeL3 = 0;
	TiXmlNode* topnode = 0;
	char *value;
	char *value2;
	int id;
	float percent;
	RaprLogicTableEntry *tmpEntry;
	
	if (doc.FirstChild("state") != NULL) {
		topnode = doc.FirstChild("state");
	}
	else if ((doc.FirstChild("RaprLogicTable") != NULL) && (doc.FirstChild("RaprLogicTable")->FirstChild("state") != NULL)) {
		topnode = doc.FirstChild("RaprLogicTable")->FirstChild("state");
	}
	else {
		DMSG(0, "RaprLogicTable::LoadFile() Error: can not find top level tag\n");		
		return false;
	}
	for (nodeL1 = topnode;nodeL1;nodeL1 = nodeL1->NextSibling("state")) {
		tmpNode = nodeL1->FirstChild("value");
		if ((tmpNode != NULL) && (tmpNode->FirstChild() != NULL)) {
			if (strchr(tmpNode->FirstChild()->Value(),',') == NULL) {
				value = newString(strlen(tmpNode->FirstChild()->Value()) + 3);
				strcpy(value,tmpNode->FirstChild()->Value());
				strcat(value,",0");
			}
			else {
				value = newString(strlen(tmpNode->FirstChild()->Value()) + 1);
				strcpy(value,tmpNode->FirstChild()->Value());
			}
//			SetStateFromString(value);
			for (nodeL2 = nodeL1->FirstChild("logicid");nodeL2;nodeL2=nodeL2->NextSibling("logicid")) {
				tmpNode = nodeL2->FirstChild("id");
				if ((tmpNode != NULL) && (tmpNode->FirstChild() != NULL)) {
					id=atoi(tmpNode->FirstChild()->Value());
					percent=1;
					tmpEntry = new RaprLogicTableEntry();
					if ((nodeL2->FirstChild("percent") != NULL) && (nodeL2->FirstChild("percent")->FirstChild() != NULL)) {
						percent=atof(nodeL2->FirstChild("percent")->FirstChild()->Value());
						tmpEntry->SetPercent(percent);
					}
					for (nodeL3 = nodeL2->FirstChild("entry");nodeL3;nodeL3=nodeL3->NextSibling("entry")) {
						if (nodeL3 != NULL) {
						   if (nodeL3->FirstChild() != NULL) {
								value2 = newString(strlen(nodeL3->FirstChild()->Value()) + 1);
								strcpy(value2,nodeL3->FirstChild()->Value());
							}
							else {
								value2 = newString(1);
							}
							DMSG(4, "RaprLogicTable load entry %s,%d,%f,%s.\n",value,id,percent,value2);
							tmpEntry->AddEntry(value2);
							delete [] value2;
						}
					}
					SetEntry(value,id,tmpEntry);
				}
			}
			delete [] value;
		}
	}
	return true;
}

//allocate and 0 set a new character buffer
char *RaprLogicTable::newString(int inSize) {
	char *tmp;
	tmp = new char [inSize];
	memset(tmp,0,inSize);
	return tmp;
}

