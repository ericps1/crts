#include "raprPayload.h"

RaprPayload::RaprPayload() {
	ubi=0;
	foreignUBI=0;
	logicid=0;
	seed=0;
	streamid=0;
	streamseq=0;
	duration=0;
	burstCount=0;
	burstPriority=0;
	streamid=0;
	streamseq=0;
	burstPayloadID=0;
	duration=0;
	validUBI=0;
	validForeignUBI=0;
	validLogicID=0;
	validSeed=0;
	validStreamID=0;
	validStreamSeq=0;
	validStreamDuration=0;
	validBurstCount=0;
	validBurstPriority=0;
	validStreamID=0;
	validStreamSeq=0;
	validStreamDuration=0;
	validBurstPayloadID=0;
	origin = 0;
	validOrigin = 0;
}

RaprPayload::RaprPayload(unsigned int inUBI) {
	ubi=1;
	foreignUBI=0;
	logicid=0;
	seed=0;
	streamid=0;
	streamseq=0;
	duration=0;
	burstCount=0;
	burstPriority=0;
	burstPayloadID=0;
	streamid=0;
	streamseq=0;
	duration=0;
	validUBI=inUBI;
	validLogicID=0;
	validSeed=0;
	validStreamID=0;
	validStreamSeq=0;
	validStreamDuration=0;
	validBurstCount=0;
	validBurstPriority=0;
	validBurstPayloadID=0;
	validStreamID=0;
	validStreamSeq=0;
	validStreamDuration=0;
	origin = 0;
	validOrigin = 0;
}

RaprPayload::RaprPayload(unsigned int inUBI,unsigned int inLogic) {
	ubi=1;
	foreignUBI=0;
	logicid=1;
	seed=0;
	streamid=0;
	streamseq=0;
	duration=0;
	burstCount=0;
	burstPriority=0;
	burstPayloadID=0;
	streamid=0;
	streamseq=0;
	duration=0;
	validUBI=inUBI;
	validForeignUBI=0;
	validLogicID=inLogic;
	validSeed=0;
	validStreamID=0;
	validStreamSeq=0;
	validStreamDuration=0;
	validBurstCount=0;
	validBurstPriority=0;
	validBurstPayloadID=0;
	validStreamID=0;
	validStreamSeq=0;
	validStreamDuration=0;
	origin = 0;
	validOrigin = 0;
}


RaprPayload::RaprPayload(MgenPayload *inPayload) {
	ubi=0;
	foreignUBI=0;
	logicid=0;
	seed=0;
	streamid=0;
	streamseq=0;
	duration=0;
	burstCount=0;
	burstPriority=0;
	burstPayloadID=0;
	streamid=0;
	streamseq=0;
	duration=0;
	validUBI=0;
	validForeignUBI=0;
	validLogicID=0;
	validSeed=0;
	validStreamID=0;
	validStreamSeq=0;
	validStreamDuration=0;
	validBurstCount=0;
	validBurstPriority=0;
	validBurstPayloadID=0;
	validStreamID=0;
	validStreamSeq=0;
	validStreamDuration=0;
	SetHex(inPayload->GetPayload());
}

RaprPayload::~RaprPayload() {
}

void RaprPayload::operator=(RaprPayload* aRaprPayload)
{
    ubi = aRaprPayload->GetUBI();
    foreignUBI = aRaprPayload->GetForeignUBI();
    logicid = aRaprPayload->GetLogicID();
    origin = aRaprPayload->GetOrigin();
    seed = aRaprPayload->GetSeed();
    streamid = aRaprPayload->GetStreamID();
    streamseq = aRaprPayload->GetStreamSeq();
    duration = aRaprPayload->GetStreamDuration();
    burstCount = aRaprPayload->GetBurstCount();
    burstPriority = aRaprPayload->GetBurstPriority();
    burstPayloadID = aRaprPayload->GetBurstPayloadID();
    validUBI = aRaprPayload->validUBI;
    validForeignUBI = aRaprPayload->validForeignUBI;
    validLogicID = aRaprPayload->validLogicID;
    validOrigin = aRaprPayload->validOrigin;
    validSeed = aRaprPayload->validSeed;
    validStreamID = aRaprPayload->validStreamID;
    validStreamSeq = aRaprPayload->validStreamSeq;
    validStreamDuration = aRaprPayload->validStreamDuration;
    validBurstCount = aRaprPayload->validBurstCount;
    validBurstPriority = aRaprPayload->validBurstPriority;
    validBurstPayloadID = aRaprPayload->validBurstPayloadID;
    
}

void RaprPayload::SetUBI(unsigned int inUBI) {
  if (inUBI) {
	ubi=inUBI;
	validUBI=1;
  }
}
unsigned int RaprPayload::GetUBI() {
  if (validUBI == 0) return 0;
  return ubi;
}

void RaprPayload::SetForeignUBI(unsigned int inForeignUBI) {
  if (inForeignUBI) {
	foreignUBI=inForeignUBI;
	validForeignUBI=1;
  }
}

unsigned int RaprPayload::GetForeignUBI() {
  if (validForeignUBI == 0) return 0;
  return foreignUBI;
}

void RaprPayload::SetLogicID(unsigned int inLogic) {
  if (inLogic) {
	validLogicID=1;
	logicid=inLogic;
  }
}

unsigned int RaprPayload::GetLogicID() {
	if (validLogicID == 0) return 0;
	return logicid;
}

void RaprPayload::SetSeed(unsigned int inSeed) {
  if (inSeed) 
    {
	validSeed=1;
	seed=inSeed;
    }
}

unsigned int RaprPayload::GetSeed() {
	if (validSeed == 0) return 0;
	return seed;
}

void RaprPayload::SetStreamID(unsigned int inID) {
  if (inID) 
    {
		 validStreamID=1;
		 streamid=inID;
    }
}

unsigned int RaprPayload::GetStreamID() {
	if (validStreamID == 0) return 0;
	return streamid;
}

void RaprPayload::SetStreamSeq(unsigned int inSeq) {
  if (inSeq) 
    {
		 validStreamSeq=1;
		 streamseq=inSeq;
    }
}

unsigned int RaprPayload::GetStreamSeq() {
	if (validStreamSeq == 0) return 0;
	return streamseq;
}
void RaprPayload::SetStreamDuration(double inDuration) {
  if (inDuration) 
    {
		 validStreamDuration=1;
		 duration=inDuration;
    }
}

double RaprPayload::GetStreamDuration() {
	if (validStreamDuration == 0) return 0;
	return duration;
}

unsigned int RaprPayload::GetBurstCount() {
	if (validBurstCount == 0) return 0;
	return burstCount;
}
void RaprPayload::SetBurstCount(unsigned int inBurstCount) {
  if (inBurstCount) 
    {
		 validBurstCount=1;
		 burstCount=inBurstCount;
    }
}
void RaprPayload::SetBurstPriority(unsigned int inBurstPriority) {
  if (inBurstPriority) 
    {
		 validBurstPriority=1;
		 burstPriority=inBurstPriority;
    }
}

unsigned int RaprPayload::GetBurstPriority() {
	if (validBurstPriority == 0) return 0;
	return burstPriority;
}

void RaprPayload::SetBurstPayloadID(unsigned int inBurstPayloadID) {
  if (inBurstPayloadID) 
    {
		 validBurstPayloadID=1;
		 burstPayloadID=inBurstPayloadID;
    }
}

unsigned int RaprPayload::GetBurstPayloadID() {
	if (validBurstPayloadID == 0) return 0;
	return burstPayloadID;
}

void RaprPayload::SetOrigin(unsigned int inOrigin) {
  if (inOrigin) 
    {
		 validOrigin=1;
		 origin=inOrigin;
    }
}

unsigned int RaprPayload::GetOrigin() {
	if (validOrigin == 0) return 0;
	return origin;
}

//translate hex code into RaprPayload variables
void RaprPayload::SetHex(const char *inHex) {
	if (inHex == NULL) return;
	MgenPayload payload;
	payload.SetPayload(inHex);
	int len=0;
	short type = 0;
	short size = 0;
	const char *raw = payload.GetRaw();
	while (len < payload.GetPayloadLen()) {
		//read the type field
		memcpy(&type,raw+len,1);
		//read the size field
		memcpy(&size,raw+len+1,1);
		len += 2;
		//copy the ubi
		if (type == 1) {
			memcpy(&ubi,raw+len,size);
			validUBI = 1;
		}

		//copy the logic id
		else if (type == 2) {
			memcpy(&logicid,raw+len,size); 
			validLogicID = 1;
		}
		//copy the random seed
		else if (type == 3) {
			memcpy(&seed,raw+len,size); 
			validSeed = 1;
		}
		//copy the foreignUBI
		else if (type == 4) {
			memcpy(&foreignUBI,raw+len,size);
			validForeignUBI = 1;
		}
		//copy the Stream ID
		else if (type == 5) {
			memcpy(&streamid,raw+len,size);
			validStreamID = 1;
		}
		//copy the Stream Seq #
		else if (type == 6) {
			memcpy(&streamseq,raw+len,size);
			validStreamSeq = 1;
		}
		// copy the Stream duration
		else if (type == 7) {
			memcpy(&duration,raw+len,size);
			validStreamDuration = 1;
		}
		// copy the Stream burst count
		else if (type == 8) {
			memcpy(&burstCount,raw+len,size);
			validBurstCount = 1;
		}
		// copy the Stream burst payload id
		else if (type == 9) {
			memcpy(&burstPayloadID,raw+len,size);
			validBurstPayloadID = 1;
		}
		// copy the Stream burst priority
		else if (type == 10) {
			memcpy(&burstPriority,raw+len,size);
			validBurstPriority = 1;
		}
		// copy the Stream burst priority  
        // Isn't this supposed to be origin, whatever ~that~ is?
		else if (type == 11) {
			memcpy(&origin,raw+len,size);
			validBurstPriority = 1;
		}
		len += size;
	}
}

//translate the binary into hex code
char *RaprPayload::GetHex() 
{
	char *raw;
	raw=new char[255];
	int len = 0;
	if (validUBI == 1) {
		//set the type field
		memset(raw+len,1,1);
		//set the lenth
		memset(raw+len+1,4,1);
		//copy the variable
		memcpy(raw+len+2,&ubi,4);
		len += 6;
	}
	if (validLogicID == 1) {
		//set the type field
		memset(raw+len,2,1);
		//set the lenth
		memset(raw+len+1,4,1);
		//copy the variable
		memcpy(raw+len+2,&logicid,4);
		len += 6;
	}
	if (validSeed == 1) {
		//set the type field
		memset(raw+len,3,1);
		//set the lenth
		memset(raw+len+1,4,1);
		//copy the variable
		memcpy(raw+len+2,&seed,4);
		len += 6;
	}
	if (validForeignUBI == 1) {
		//set the type field
		memset(raw+len,4,1);
		//set the lenth
		memset(raw+len+1,4,1);
		//copy the variable
		memcpy(raw+len+2,&foreignUBI,4);
		len += 6;
	}
	if (validStreamID == 1) {
		//set the type field
		memset(raw+len,5,1);
		//set the lenth
		memset(raw+len+1,4,1);
		//copy the variable
		memcpy(raw+len+2,&streamid,4);
		len += 6;
	}
	if (validStreamSeq == 1) {
		//set the type field
		memset(raw+len,6,1);
		//set the lenth
		memset(raw+len+1,4,1);
		//copy the variable
		memcpy(raw+len+2,&streamseq,4);
		len += 6;
	}
	if (validStreamDuration == 1) {
		//set the type field
		memset(raw+len,7,1);
		//set the lenth
		memset(raw+len+1,8,1);
		//copy the variable
		memcpy(raw+len+2,&duration,8);
		len += 10;
	}
	if (validBurstCount == 1) {
		//set the type field
		memset(raw+len,8,1);
		//set the lenth
		memset(raw+len+1,4,1);
		//copy the variable
		memcpy(raw+len+2,&burstCount,4);
		len += 6;
	}
	if (validBurstPriority == 1) {
		//set the type field
		memset(raw+len,10,1);
		//set the lenth
		memset(raw+len+1,4,1);
		//copy the variable
		memcpy(raw+len+2,&burstPriority,4);
		len += 6;
	}
	if (validBurstPayloadID == 1) {
		//set the type field
		memset(raw+len,9,1);
		//set the lenth
		memset(raw+len+1,4,1);
		//copy the variable
		memcpy(raw+len+2,&burstPayloadID,4);
		len += 6;
	}
	if (validOrigin == 1) {
		//set the type field
		memset(raw+len,11,1);
		//set the lenth
		memset(raw+len+1,4,1);
		//copy the variable
		memcpy(raw+len+2,&origin,4);
		len += 6;
	}

	//use mgen payload to encode the thing
	if (len == 0) 
	{
		delete [] raw;
		return NULL;
	}
	MgenPayload payload;
	payload.SetRaw(raw,len);
	delete [] raw;
	return payload.GetPayload();
}

