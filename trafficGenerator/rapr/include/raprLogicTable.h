#ifndef _RAPR_LOGIC_TABLE
#define _RAPR_LOGIC_TABLE

#ifndef STLPORT
#include <vector>
#include <tr1/functional>
#else
#include "vector"
#endif

#include "raprDictionary.h"
#include "raprNumberGenerator.h"
#include "raprPayload.h"
#include "mgenMsg.h"
#include "raprUBIState.h"
#include "behaviorEvent.h"

class Rapr;

#ifndef STLPORT
typedef std::vector<unsigned int> RaprLogicStateKey;
typedef std::map<char *,char *,ltstr> RaprLogicKeyMap;
#else
typedef _STL::vector<unsigned int> RaprLogicStateKey;
typedef _STL::map<char *,char *,ltstr> RaprLogicKeyMap;
#endif

class RaprLogicState {
    public:
	 	 RaprLogicState();
	 	 ~RaprLogicState() {;};		 
		 //print out the state in x.y.z format	    
	    char *ToString() { return ToString(0); }
	    char *ToString(unsigned int inUBI);
		 //return the hash of the state;
		 int ToHash() { return ToHash((unsigned int)0); }
		 int ToHash(unsigned int inUBI);
		 int ToHash(char *inState);
		 //input the state from x.y.z format
		 void FromString(char *inState);
		 //name the current state to allow for easier access later
		 void SetName(char *inName);
		 void SetName(char *inName,char *inState);
		 //set the state
		 void SetState(int inLevel,unsigned int inState);
		 void SetState(char *inName); 
		 void SetUBIState(unsigned int inUBI,int inState) { ubiState.SetState(inUBI,inState); }
	 private:
	    RaprLogicStateKey key;
		 RaprLogicKeyMap keymap;
		 RaprUBIState ubiState;
};

class RaprLogicTableEntry {
	public:
		RaprLogicTableEntry();
		~RaprLogicTableEntry();
		void AddEntry(char *inEntry);
		RaprStringVector *GetEntry();
		void SetSuccess(int inID) {success = inID;}
		void SetFailure(int inID) {failure = inID;}
		void SetTimeout(int inID) {timeout = inID;}
		void SetPercent(float inPercent) {percent = inPercent;}
		int GetSuccess() {return success;}
		int GetFailure() {return failure;}
		int GetTimeout() {return timeout;}
		float GetPercent() {return percent;}
	private:
		RaprStringVector main;
		int success;
		int failure;
		int timeout;
		float percent;
};

#ifndef STLPORT
typedef std::map<int,RaprLogicTableEntry *> RaprLogicIDTable;
typedef std::map<int,RaprLogicIDTable *> RaprLogicStateTable;
#else
typedef _STL::map<int,RaprLogicTableEntry *> RaprLogicIDTable;
typedef _STL::map<int,RaprLogicIDTable *> RaprLogicStateTable;
#endif

class RaprLogicTable {
	public:
		RaprLogicTable(Rapr *theRapr);
		RaprLogicTable(Rapr *theRapr,char *inFile);
		RaprLogicTable();
		RaprLogicTable(char *inFile);
		~RaprLogicTable();
		//various methods to set the state of the table
		bool SetStateFromString(char *inState);
		bool SetState(char *inName);
		bool SetState(int inLevel,int inState);
		bool SetUBIState(char *inUBI,char *inState);
		
		//init a single logic id
		void SetEntry(char *inState,int inID,RaprLogicTableEntry *inEntry);
		bool DoLogicID(int inID,const MgenMsg& inMsg,const ProtoAddress& srcAddr,BehaviorEvent* theBehaviorObject);
		bool DoLogicID(int inID,BehaviorEvent* theBehaviorObject);
		bool DoSuccess(int inID);
		bool DoFailure(int inID);
		bool DoTimeout(int inID);
		void SetDictionaryValue(char *inNS,char *inField,char *inVal) { dictionary.SetValue(inNS,inField,inVal); }
		void SetDictionaryValue(char *inField,char *inVal) { dictionary.SetValue(inField,inVal); }
		void ResetDictionaryValue(char *inNS,char *inField,char *inVal) { dictionary.ResetValue(inNS,inField,inVal); }
		void ResetDictionaryValue(char *inField,char *inVal) { dictionary.ResetValue(inField,inVal); }
		RaprStringVector *TranslateString(char* string,RaprPRNG* prng);
		bool LoadFile(char *inFile);
		bool LoadDictionary(char *inFile) {return dictionary.LoadFile(inFile);}
        RaprDictionary& GetDictionary() {return dictionary;};
	private:
		RaprLogicState state;
		RaprLogicStateTable table;
		RaprDictionary dictionary;
		Rapr	*rapr;
		RaprLogicTableEntry *RandomizeEntry(RaprLogicTableEntry *inEntry,RaprPRNG *prng);
		bool DoLogicIDInternal(int inID,RaprDictionaryTransfer *trans,BehaviorEvent* theBehaviorObject);
		char *newString(int inSize);
};
#endif // _RAPR_LOGIC_TABLE
