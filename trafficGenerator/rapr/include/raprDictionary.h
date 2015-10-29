#ifndef STLPORT
#include <map>
#include <vector>
#else
#include "map"
#include "vector"
#endif

#include "raprNumberGenerator.h"
#ifndef _RAPR_DICTIONARY
#define _RAPR_DICTIONARY

#include "raprPayload.h"

#ifndef STLPORT
typedef std::vector<const char *> RaprStringVector;
typedef std::map<const char *,RaprStringVector *,ltstr> RaprDictionaryMap;
typedef std::multimap<int,  const char *> RaprDictionaryNameSpaceMap;
typedef std::map<const char *,RaprDictionaryMap *,ltstr> RaprDictionaryNameSpace;
#else
typedef _STL::vector<const char *> RaprStringVector;
typedef _STL::map<const char *,RaprStringVector *,ltstr> RaprDictionaryMap;
typedef _STL::multimap<int,  const char *> RaprDictionaryNameSpaceMap;
typedef _STL::map<const char *,RaprDictionaryMap *,ltstr> RaprDictionaryNameSpace;
#endif

class RaprDictionaryTransfer {
	public:
	  RaprDictionaryTransfer();
	  ~RaprDictionaryTransfer();
	  void SetPRNG(RaprPRNG *inPrng) {prng = inPrng;}
	  RaprPRNG *GetPRNG() {return prng;}
	  unsigned int GetRandom();
	  float GetRandomF();
	  char *GetSrcIP() { return srcip; }
	  char *GetSrcPort() { return srcport; }
	  char *GetDstIP() { return dstip; }
	  char *GetDstPort() { return dstport; }
	  RaprPayload *GetPayload() { return payload; }
	  void SetSrcIP(char *inVal) { srcip = inVal; }
	  void SetSrcPort(char *inVal) { srcport = inVal; }
	  void SetDstIP(char *inVal) { dstip = inVal; }
	  void SetDstPort(char *inVal) { dstport = inVal; }
	  void SetPayload(RaprPayload *inPayload) { payload = inPayload; }
      void operator=(RaprDictionaryTransfer* trans);
  private:
	  RaprPRNG *prng;
	  char *srcip;
	  char *srcport;
	  char *dstip;
	  char *dstport;
	  RaprPayload *payload;
};

class RaprDictionary {
   public:
	   RaprDictionary();
	   RaprDictionary(char *inFile);
	   ~RaprDictionary();
	   RaprStringVector *translate(const char *baseString,RaprDictionaryTransfer *trans);
	   RaprStringVector *translate(const char *baseString);
	   void SetPersonality(char *inFileName) {;}
	   void SetDefinition(char *inFileName) {;}
	   void SetValue(char *inNS,char *inField,char *inVal);
	   void ResetValue(char *inNS,char *inField,char *inVal);
	   void SetValue(char *inField,char *inVal) { SetValue("DEFAULT",inField,inVal); }
	   void ResetValue(char *inField,char *inVal) { ResetValue("DEFAULT",inField,inVal); }
	   bool LoadFile(char *inFile);
 private:
	   int count(const char *inString,char inSep);
	   int location(const char *inString,char inSep);
	   char *newString(int inSize);
	   RaprStringVector *lookup(char *inIndex,RaprDictionaryTransfer *trans);
	   RaprStringVector *namevalue(const char *inNS,const char *inVal,RaprDictionaryTransfer *trans,char *arg);
	   RaprStringVector *PacketNameSpace(const char *inVal,RaprDictionaryTransfer *trans);
	   RaprStringVector *SystemNameSpace(const char *inVal,RaprDictionaryTransfer *trans,char *arg);
	   RaprStringVector *ParseNestedField(const char *buf,int z,RaprDictionaryTransfer *trans);
	   RaprStringVector *ParseNestedField(const char *buf,char *name,RaprDictionaryTransfer *trans);

	   RaprDictionaryNameSpaceMap nsm;
	   RaprDictionaryNameSpace dmap;
	   int mapcount;
};

#endif //_RAPR_DICTIONARY

