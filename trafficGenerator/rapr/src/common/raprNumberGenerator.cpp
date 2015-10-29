#include "raprNumberGenerator.h"
#include "protoDebug.h"
#include "raprGlobals.h"
#include <stdlib.h>

#ifndef SPRNG2
#include "sprng_cpp.h"
#else
#include "sprng.h"
#endif

RaprNumberGenerator::RaprNumberGenerator() 
{
  //mask the last 24 bits of 32 bits long
  SEQMASK = 16777215;
  streamID = 1;
  initialSeed = 0;
  runningSeed = 0;
  runningRtiSeed = 0;
  FlowIDAssign = NULL;
  FlowIDRecNum = 0;
  lastupdate.tv_sec = 0;
  lastupdate.tv_usec = 0;

  // Default host id to last octet of ip address
  short hostid = 0;
  ProtoAddress nodeAddress;
  if (nodeAddress.ResolveLocalAddress())
  {
      const char *ptr = strrchr(nodeAddress.GetHostString(),'.');
      hostid = atoi(++ptr);
      SetHostID(hostid);
  } else
  {
      DMSG(0,"RaprNumberGenerator::RaprNumberGenerator() ERROR: Unable to default HOST ID!\n");
  }
  
}

RaprNumberGenerator::~RaprNumberGenerator() {
}

void RaprNumberGenerator::InitializeSeeds(short hostID)
{
  runningSeed =  ((4294967295u / 1000) * hostID);
  runningRtiSeed = (runningSeed + ((4294967295u/1000)/2));

  DMSG(1,"RaprNumberGenerator::InitializeSeeds() initializing starting seed from HOSTID>%d seed>%d initialRtiSeed> %d\n",hostID,runningSeed,runningRtiSeed);
}
// InitialSeed if specified, should be specified after
// host id in the input scripts
void RaprNumberGenerator::InitializeSeeds(int initialSeed)
{

  runningSeed =  ((4294967295u / 1000) * initialSeed);
  runningRtiSeed = (runningSeed + ((4294967295u/1000)/2));

  DMSG(1,"RaprNumberGenerator::InitializeSeeds() initializing starting seed from INITIAL_SEED>%d seed>%d initialRtiSeed> %d\n",initialSeed,runningSeed,runningRtiSeed);
}

short RaprNumberGenerator::GetHostIDFromUBI(unsigned long inUBI) {
	short tmp;
	tmp = inUBI >> 24;
	return tmp;
}

short RaprNumberGenerator::GetHostID() {
	short tmp;
	tmp = hostID >> 24;
	return tmp;
}
void RaprNumberGenerator::SetHostID(short host) {

	unsigned long tmp;
	//convert it to a long
	tmp = host;
	//only take the right most byte
	hostID = tmp << 24;
}

void RaprNumberGenerator::SetUBI(unsigned long inUBI) {
	//get the host id portion
	hostID = inUBI & ~SEQMASK;
	//get the sequence number portion
	seqNumber = inUBI & SEQMASK;
}
	
void RaprNumberGenerator::SetSequence(unsigned long Seq) {
	seqNumber = Seq & SEQMASK;
}

unsigned long RaprNumberGenerator::GetUBI() {
	return hostID | (seqNumber++ & SEQMASK);
}

char *RaprNumberGenerator::FormatUBI(unsigned int inUBI) {
	char *res = new char[20];
	unsigned int tmp = inUBI & SEQMASK;
	if (GetHostID() == GetHostIDFromUBI(inUBI)) {
		sprintf(res,"%u",tmp);
	}
	else {
		sprintf(res,"%u (foreign)",tmp);
	}
	return res;
}

//add a flow id to be available to use
void RaprNumberGenerator::SetFlowID(int inFlow) {
	if (inFlow < 0) return;
	flowAvail.insert(inFlow);
}

//add a range of flow id to be available to use
void RaprNumberGenerator::SetFlowID(int inFlowStart, int inFlowEnd) {
	if ((inFlowStart < 0) || (inFlowEnd < 0)) return;
	if (inFlowEnd < inFlowStart) return;
	for (int i=inFlowStart;i <=inFlowEnd;i++) {
	   flowAvail.insert(i);
	}
}

int RaprNumberGenerator::GetFlowID(char *inKey) {
	int ret = -1;
	//check if a flow id has already been assigned and is open
	if (!flowAssign.empty()) {
		FlowIDAssigned::iterator it = flowAssign.find(inKey);
  	   if (it != flowAssign.end()) {
			ret = it->second;
			flowAssign.erase(it);
		}
	}	
	//now check for any open flow id
	if ((ret == -1) && (!flowAvail.empty())) {
		FlowIDAvailable::iterator it2 = flowAvail.begin();
		ret = *it2;
		flowAvail.erase(it2);
        LogFlowID(inKey,ret);
	}
	
	if (ret >= 0) {
		flowUsed[ret] = inKey;
		return ret;
	}
	DMSG(1,"RaprPRNG::GetFlowID() Error: flowID>%d\n",ret);

	//error condition
	return ret;
}

int RaprNumberGenerator::GetFlowID(ProtoAddress *srcAddr,ProtoAddress *dstAddr) {
	char *tmp;
	char tmp2[11];
	int ret = 0;
	
	memset(tmp2,0,11);
	tmp = new char[45];
	memset(tmp,0,45);
	strcpy(tmp,srcAddr->GetHostString());
	strcat(tmp,"-");
	strcat(tmp,dstAddr->GetHostString());
	sprintf(tmp2,"%d",dstAddr->GetPort());
	strcat(tmp,"/");
	strcat(tmp,tmp2);
	ret = GetFlowID(tmp);
	DMSG(1,"RaprNumberGenerator::GetFlowID() Getting flow id>%d\n",ret);

	return ret;
}

void RaprNumberGenerator::UnlockFlowID(int inFlow) {

	DMSG(1,"RaprPRNG::UnlockFlowID() flowID>%d\n",inFlow);

	if (!flowUsed.empty()) {
		FlowIDInUse::iterator it = flowUsed.find(inFlow);
		//found the flow id
		if (it != flowUsed.end()) {
			char *tmpVal = flowUsed[inFlow];
			flowUsed.erase(it);
			//insert into the ready to use group
#ifndef STLPORT
			flowAssign.insert(std::pair<char *,int>(tmpVal,inFlow));
#else
			flowAssign.insert(_STL::pair<char *,int>(tmpVal,inFlow));
#endif
		}
	}
}

void RaprNumberGenerator::FileOutput(FILE *inStream) {
    if (inStream == NULL) return;
    timeval time1;
    int flag;
    gettimeofday(&time1,NULL);
    flag=MgenSequencer::OK_FLAG;
    //timestamp
    fwrite((void *)&time1,sizeof(timeval),1,inStream);
    //size
    fwrite((void *)&FlowIDRecNum,sizeof(int),1,inStream);
    //data
    fwrite((void *)FlowIDAssign,RECSIZE,FlowIDRecNum,inStream);
    //validation flag
    fwrite((void *)&flag,sizeof(int),1,inStream);
}

void RaprNumberGenerator::FileInput(FILE *inStream) {
    if (inStream == NULL) return;
    char *tmp;
    timeval time1;
    int size = 0;
    int flag = 0;
    FlowIDAvailable::iterator it;
    //read in timestamp
    fread((void *)&time1,sizeof(timeval),1,inStream);
    //read in data size
    fread((void *)&size,sizeof(int),1,inStream);
    tmp = new char[size*RECSIZE];
    //read data
    fread((void *)tmp,RECSIZE,size,inStream);
    //read integrity flag
    fread((void *)&flag,sizeof(int),1,inStream);
    if (flag == MgenSequencer::OK_FLAG) {
        if (MgenSequencer::Compare(time1,lastupdate)) {
            memcpy(&lastupdate,&time1,sizeof(timeval));
            //clear any old assignment
            if (FlowIDAssign != NULL) {
                delete FlowIDAssign;
            }
            flowAssign.clear();
            char *tmpStr;
            int tmpFlow;
            //parse each line separately
            for (int i=0;i < size;i++) {
                tmpStr = new char[strlen(tmp+(i*RECSIZE))+1];
                memset(tmpStr,0,strlen(tmp+(i*RECSIZE))+1);
                memcpy(tmpStr,tmp+(i*RECSIZE),strlen(tmp+(i*RECSIZE)));
                memcpy(&tmpFlow,tmp+(i*RECSIZE)+45,sizeof(int));
                //store the parsed line
#ifndef STLPORT
                flowAssign.insert(std::pair<char *,int>(tmpStr,tmpFlow));
#else
                flowAssign.insert(_STL::pair<char *,int>(tmpStr,tmpFlow));
#endif
                //remove it from available id's
                it=flowAvail.find(tmpFlow);
                if (it != flowAvail.end()) {
                    flowAvail.erase(it);
                }
            }
            FlowIDAssign = tmp;
            FlowIDRecNum = size;
        }
    }
}

void RaprNumberGenerator::LogFlowID(char *inKey,int inFlow) {
    char *tmp;
    if (inKey == NULL) return;
    tmp = new char[(FlowIDRecNum+1)*RECSIZE];
    memset(tmp,0,(FlowIDRecNum+1)*RECSIZE);
    if (FlowIDRecNum > 0) {
        memcpy(tmp,FlowIDAssign,FlowIDRecNum*RECSIZE);
        delete [] FlowIDAssign;
    }
    memcpy(tmp+(FlowIDRecNum*RECSIZE),inKey,strlen(inKey));
    memcpy(tmp+(FlowIDRecNum*RECSIZE)+45,&inFlow,sizeof(int));
    FlowIDAssign = tmp;
    FlowIDRecNum++;
}

void RaprPRNG::SetSeed(unsigned int inSeed) {
	//This is the front end of the prng from NIST
  DMSG(5,"RaprPRNG::SetSeed seed>%d\n",inSeed);
  seed = inSeed;

#ifndef SPRNG2  
  stream = SelectType(SPRNG_TYPE_DEFAULT);
  stream->init_sprng(streamnum,NSTREAMS_DEFAULT,inSeed,SPRNG_DEFAULT);
#else
  stream = init_sprng(SPRNG_LCG,SPRNG_DEFAULT,NSTREAMS_DEFAULT,inSeed,SPRNG_DEFAULT);
  // ljt stream = init_sprng(SPRNG_DEFAULT,NSTREAMS_DEFAULT,inSeed,SPRNG_DEFAULT);
  //print_sprng(stream);
#endif
  draw = 0;
}

double RaprPRNG::GetRandom() {
  if (!seed || stream == NULL)
    {
      DMSG(0,"RaprPRNG::GetRandom() Error: uninitialized stream - using 0 value\n");
      return 0;
    }

  draw++;
  DMSG(3,"RaprPRNG::GetRandom() Draw: %d Seed: %d\n",draw,seed);

#ifndef SPRNG2
  return stream->sprng();
#else
  return sprng(stream);
  //	return (float)random()/(float)RAND_MAX;
#endif
}

float RaprPRNG::GetRandom(float lower,float upper) {
  if (!seed || stream == NULL)
    {
      DMSG(0,"RaprPRNG::GetRandom(lower,upper) Error: uninitialized stream - using 0 value\n");
      return 0;
    }
  
  if (lower > upper) return 0;
  //draw++; 
  //scw there is actually no draw here, its in GetRandom()
  float range = upper - lower;
  DMSG(5,"RaprPRNG::GetRandom(n,n) Draw: %d Seed: %d\n",draw,seed);

  return lower + (range*GetRandom());
}

unsigned int RaprPRNG::GetRand() {
  if (!seed || stream == NULL)
    {
      DMSG(0,"RaprPRNG::GetRand() Error: uninitialized stream - using 0 value\n");
      return 0;
    }

  draw++;
  DMSG(5,"RaprPRNG::GetRand() Draw: %d Seed: %d\n",draw,seed);

#ifndef SPRNG2
  return stream->isprng();
#else
  return isprng(stream);
  //return random();
#endif
}

unsigned int RaprPRNG::GetRand(unsigned int lower,unsigned int upper) {
  if (!seed || stream == NULL)
    {
      DMSG(0,"RaprPRNG::GetRand(lower,upper) Error: uninitialized stream - using 0 value\n");
      return 0;
    }

  if (lower > upper) return 0;
  //draw++;
  //scw there is actually no draw here, its in GetRandom()
  unsigned int range = upper - lower;
  DMSG(5,"RaprPRNG::GetRand(n,n) Draw: %d Seed: %d\n",draw,seed);

  return lower + (unsigned int)(rint(range*GetRandom()));
}


RaprPRNG::RaprPRNG() {
        DMSG(0,"RaprPRNG::RaprPRNG Warning: No stream initialized!\n");
	seed = 0;  
	draw = 0;
        streamnum = 0;
	stream = NULL;
}

RaprPRNG::RaprPRNG(unsigned int inSeed) {
  DMSG(5,"RaprPRNG::RaprPRNG seed>%d\n",inSeed);

	seed = inSeed;
        streamnum = 0;
#ifndef SPRNG2  
	stream = SelectType(SPRNG_TYPE_DEFAULT);
	stream->init_sprng(streamnum,NSTREAMS_DEFAULT,inSeed,SPRNG_DEFAULT);
#else
	stream = init_sprng(SPRNG_LCG,SPRNG_DEFAULT,NSTREAMS_DEFAULT,inSeed,SPRNG_DEFAULT);
	// ljt stream = init_sprng(SPRNG_DEFAULT,NSTREAMS_DEFAULT,inSeed,SPRNG_DEFAULT);
	//print_sprng(stream);
#endif
	draw = 0;
}

RaprPRNG::~RaprPRNG() 
{
	if (stream)
#ifndef SPRNG2
		stream->free_sprng();
#else
		free_sprng (stream);
#endif
}
