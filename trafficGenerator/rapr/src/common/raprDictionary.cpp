#include <string.h>
#include "raprDictionary.h"
#include "tinyxml.h"
#include "rapr.h"

RaprDictionaryTransfer::RaprDictionaryTransfer() {
	prng = NULL;
	srcip = NULL;
	srcport = NULL;
	dstip = NULL;
	dstport = NULL;
	payload = NULL;
}

RaprDictionaryTransfer::~RaprDictionaryTransfer() {
	if (srcip != NULL) delete [] srcip;
	if (srcport != NULL) delete [] srcport;
	if (dstip != NULL) delete [] dstip;
	if (dstport != NULL) delete [] dstport;
	//if (prng != NULL) delete prng;
    // ljt why aren't we deleting payload & prng? 
    // If you delete payload make sure the raprPayload
    // assignment overload is working.
}

void RaprDictionaryTransfer::operator=(RaprDictionaryTransfer* inTrans)
{
    // rewrite this stupid trans "object"
    char *tmp;
    tmp = new char[20];
    memset(tmp,0,20);
    if (inTrans->GetSrcIP())
      strcpy(tmp,inTrans->GetSrcIP());
    SetSrcIP(tmp);

    tmp = new char[20];
    memset(tmp,0,20);
    if (inTrans->GetSrcPort())
      strcpy(tmp,inTrans->GetSrcPort());
    SetSrcPort(tmp);

    tmp = new char[20];
    memset(tmp,0,20);
    if (inTrans->GetDstIP())
      strcpy(tmp,inTrans->GetDstIP());
    SetDstIP(tmp);

    tmp = new char[20];
    memset(tmp,0,20);
    if (inTrans->GetDstPort())
      strcpy(tmp,inTrans->GetDstPort());
    SetDstPort(tmp);

    payload = inTrans->GetPayload();
    prng = inTrans->GetPRNG();
    
} // end RaprDictionaryTransfer::operator=()

// ben why is this function named differently than 
// numbergenerator's getrand() ??
unsigned int RaprDictionaryTransfer::GetRandom() {
	if (prng == 0) {
	  DMSG(0,"RaprDictionaryTransfer::GetRandom() Error: No prng available!\n");
	}
	return prng->GetRand();
}

float RaprDictionaryTransfer::GetRandomF() {
	if (prng == 0) {
	  DMSG(0,"RaprDictionaryTransfer::GetRandomF() Error: No prng available!\n");
	}
	return prng->GetRandom();
}

RaprDictionary::RaprDictionary() {
#ifndef STLPORT
  	nsm.insert(std::pair<int, char *>(99,"DEFAULT"));
#else
  	nsm.insert(_STL::pair<int, char *>(99,"DEFAULT"));
#endif
  	dmap["DEFAULT"]=new RaprDictionaryMap();
	mapcount = 0;
//	LoadFile("dictionary.xml");
}

RaprDictionary::RaprDictionary(char *inFile) {
#ifndef STLPORT
	nsm.insert(std::pair<int, const char *>(99,"DEFAULT"));
#else
	nsm.insert(_STL::pair<int, const char *>(99,"DEFAULT"));
#endif
	dmap["DEFAULT"]=new RaprDictionaryMap();
	mapcount = 0;
	LoadFile(inFile);
}

RaprDictionary::~RaprDictionary() {
	RaprDictionaryNameSpace::iterator it;
	for (it = dmap.begin();it != dmap.end();it++) {
		if (it->second != NULL) delete it->second;
	}
}

void RaprDictionary::SetValue(char *inNS,char *inField,char *inVal) {
	char *localNS;
	char *localField;
	char *localVal;
	if (inNS == NULL) {
		SetValue("DEFAULT",inField,inVal);
		return;
	}
	if ((inField == NULL) || (inVal == NULL)) return;
	localVal = newString(strlen(inVal) + 1);
	strcpy(localVal,inVal);
	localField = newString(strlen(inField) + 1);
	strcpy(localField,inField);
	localNS = newString(strlen(inNS) + 1);
	strcpy(localNS,inNS);

	if (dmap[localNS] == NULL) {
		dmap[localNS] = new RaprDictionaryMap();
#ifndef STLPORT
		nsm.insert(std::pair<int, char *>(mapcount++,localNS));
#else
		nsm.insert(_STL::pair<int, char *>(mapcount++,localNS));
#endif
	}
	if ((*(dmap[localNS]))[localField] == NULL) {
		(*(dmap[localNS]))[localField] = new RaprStringVector();
	}
	(*(dmap[localNS]))[localField]->push_back(localVal);
}

void RaprDictionary::ResetValue(char *inNS,char *inField,char *inVal) {
	char *localNS;
	char *localField ; 
	char *localVal ;   
	if (inNS == NULL) {
		ResetValue("DEFAULT",inField,inVal);
		return;
	}
	if ((inField == NULL) || (inVal == NULL)) return;
	localVal = newString(strlen(inVal) + 1);
	strcpy(localVal,inVal);
	localField = newString(strlen(inField) + 1);
	strcpy(localField,inField);
	localNS = newString(strlen(inNS) + 1);
	strcpy(localNS,inNS);
	if (dmap[localNS] == NULL) {
		dmap[localNS] = new RaprDictionaryMap();
#ifndef STLPORT
		nsm.insert(std::pair<int, char *>(mapcount++,localNS));
#else
		nsm.insert(_STL::pair<int, char *>(mapcount++,localNS));
#endif
	}
	if ((*(dmap[localNS]))[localField] != NULL) {
		RaprStringVector *tmpVec = (*(dmap[localNS]))[localField];
		delete tmpVec;
	}
	(*(dmap[localNS]))[localField] = new RaprStringVector();
	(*(dmap[localNS]))[localField]->push_back(localVal);
}

RaprStringVector *RaprDictionary::lookup(char *inIndex,RaprDictionaryTransfer *trans) {
	char *arg;
	arg = NULL;
	RaprStringVector *res;
	if ((count(inIndex,'(') == 1) && (count(inIndex,')') == 1) &&
		  (location(inIndex,')') > location(inIndex,'('))) {
		arg = newString(location(inIndex,')') - location(inIndex,'('));
		strncpy(arg,inIndex+location(inIndex,'(')+1,location(inIndex,')') - 
		        location(inIndex,'(')-1);
		inIndex[location(inIndex,'(')] = '\0';
	}
	if (count(inIndex,':') == 1) {
		//a specific namespace is identified, no others will be checked
		char *tmp1;
		char *tmp2;
		int x = location(inIndex,':');
		tmp1 = newString(x+1);
		tmp2 = newString(strlen(inIndex) - x);
		strncpy(tmp1,inIndex,x);
		strcpy(tmp2,inIndex+x+1);
		res = namevalue(tmp1,tmp2,trans,arg);
		if (tmp1 != NULL) delete [] tmp1;
		if (tmp2 != NULL) delete [] tmp2;
		if (arg != NULL) delete [] arg;
		return res;
	}
	else {
		RaprDictionaryNameSpaceMap::iterator it;
		//walk through each namespace and check if the value is in the ns
		for (it=nsm.begin();it != nsm.end();it++) {
			res = namevalue(it->second,inIndex,trans,arg);
			if (res != NULL) {
				if (arg != NULL) delete arg;
				return res;
			}
		}
	}
	if (arg != NULL) delete arg;
	return NULL;
}

RaprStringVector *RaprDictionary::SystemNameSpace(const char *inVal, RaprDictionaryTransfer *trans,char *arg) {
	RaprStringVector *res = NULL;
	char *tmp = NULL;
	if (trans != NULL) {
		if (strcasecmp(inVal,"RANDOMI") == 0) {
			if ((arg == NULL) || (count(arg,',') != 1)) {
				DMSG(0,"RaprDictionary::SystemNameSpace - Error:, incorrect number of arguements to RandomI.\n");
				return NULL;
			}
			tmp = newString(20);
			int start,end;
			strncpy(tmp,arg,location(arg,','));
			if (strlen(tmp) == 0) {
				DMSG(0,"RaprDictionary::SystemNameSpace - Error: blank arguement.\n");
				return NULL;
			}
			start = atoi(tmp);
			strcpy(tmp,arg+location(arg,',')+1);
			if (strlen(tmp) == 0) {
				DMSG(0,"RaprDictionary::SystemNameSpace - Error: blank arguement.\n");
				return NULL;
			}
			end = atoi(tmp);
			if (end < start) {
				DMSG(0,"RaprDictionary::SystemNameSpace - Error: incorrect range.\n");
				return NULL;
			}
			int diff = end - start;
			end = start + int(rint(diff * trans->GetRandomF()));
			sprintf(tmp,"%u",end);
		}
		else if (strcasecmp(inVal,"RANDOMF") == 0) {
			if ((arg == NULL) || (count(arg,',') < 1) || (count(arg,',') > 2)) {
				DMSG(0,"RaprDictionary::SystemNameSpace - Error: incorrect number of arguements to RandomI.\n");
				return NULL;
			}
			tmp = newString(20);
			float start,end;
			strncpy(tmp,arg,location(arg,','));
			if (strlen(tmp) == 0) {
				DMSG(0,"RaprDictionary::SystemNameSpace - Error: blank argument.\n");
				return NULL;
			}
			start = atof(tmp);
			int len = 0;
			if (count(arg,',') == 2) {
				len = location(arg+location(arg,',')+1,',');
			}
			else {
				len = strlen(arg) - location(arg,',');
			}
			strncpy(tmp,arg+location(arg,',')+1,len);
			if (strlen(tmp) == 0) {
				DMSG(0,"RaprDictionary::SystemNameSpace - Error: blank argument.\n");
				return NULL;
			}
			end = atof(tmp);
			if (end < start) {
				DMSG(0,"RaprDictionary::SystemNameSpace - Error: incorrect range.\n");
				return NULL;
			}
			int width = 1;
			if (count(arg,',') == 2) {
				strcpy(tmp,arg+location(arg+location(arg,',')+1,',')+location(arg,',')+2);
				width = atoi(tmp);
			}
			float diff = end - start;
			end = start + diff * trans->GetRandomF();
			sprintf(tmp,"%.*f",width,end);
		}
	}
	if (tmp != NULL) {
		res = new RaprStringVector();
		res->push_back(tmp);
	}
	return res;
}

RaprStringVector *RaprDictionary::PacketNameSpace(const char *inVal, RaprDictionaryTransfer *trans) {
	//PACKET is a reserved Namespace
	RaprStringVector *res = NULL;
	char *tmp = NULL;
	if (trans != NULL) {
	//Return the next random seed
		if (strcasecmp(inVal,"SEED") == 0) {
			tmp = newString(20);
			sprintf(tmp,"%u",trans->GetRandom());
		}
		else if (strcasecmp(inVal,"SRCIP") == 0) {
			if (trans->GetSrcIP() != NULL) {
				tmp = newString(20);
				strcpy(tmp,trans->GetSrcIP());
			}
		}
		else if (strcasecmp(inVal,"SRCPORT") == 0) {
			if (trans->GetSrcIP() != NULL) {
				tmp = newString(20);
				strcpy(tmp,trans->GetSrcPort());
			}
		}
		else if (strcasecmp(inVal,"DSTIP") == 0) {
			if (trans->GetSrcIP() != NULL) {
				tmp = newString(20);
				strcpy(tmp,trans->GetDstIP());
			}
		}
		else if (strcasecmp(inVal,"DSTPORT") == 0) {
			if (trans->GetSrcIP() != NULL) {
				tmp = newString(20);
				strcpy(tmp,trans->GetDstPort());
			}
		}
		else if (strcasecmp(inVal,"UBI") == 0) {
			if (trans->GetPayload() != NULL) {
				RaprPayload *payload = trans->GetPayload();
				tmp = newString(20);
				if (payload->GetUBI() != 0) {
					sprintf(tmp,"%u",payload->GetUBI());
				}
				else if (payload->GetForeignUBI() != 0) {
					sprintf(tmp,"%u",payload->GetForeignUBI());
				}
				else {
					sprintf(tmp,"0");
//					return NULL;
				}
			}
		}
		else if (strcasecmp(inVal,"ORIGIN") == 0) {
			if (trans->GetPayload() != NULL) {
				RaprPayload *payload = trans->GetPayload();
				tmp = newString(20);
				if (payload->GetOrigin() != 0) {
					sprintf(tmp,"%u",payload->GetOrigin());
				}
				else if (payload->GetUBI() != 0) {
					sprintf(tmp,"%u",payload->GetUBI());
				}
				else if (payload->GetForeignUBI() != 0) {
					sprintf(tmp,"%u",payload->GetForeignUBI());
				}
				else {
					return NULL;
				}
			}
		}
	}
	if (tmp != NULL) {
		res = new RaprStringVector();
		res->push_back(tmp);
	}
	return res;
}

//Does the actual lookup in the Namespace table.  This is implemented so that
//reserve space like LOCAL can be done without creating a new namespace everytime
//translate is called.
RaprStringVector *RaprDictionary::namevalue(const char *inNS,const char *inVal,RaprDictionaryTransfer *trans, char *arg) {
	if (inNS == NULL) return NULL;
	if (strcasecmp(inNS,"PACKET") == 0) {
		return PacketNameSpace(inVal,trans);
	}
	else if (strcasecmp(inNS,"SYSTEM") == 0) {
		return SystemNameSpace(inVal,trans,arg);
	}
	else {
        if (dmap.find(inNS) == dmap.end()) return NULL;
//		if (dmap[inNS] == NULL) return NULL;
		if ((*(dmap[inNS]))[inVal] == NULL) return NULL;		// jm: reading free'd memory
		return (*(dmap[inNS]))[inVal];
	}
}

RaprStringVector *RaprDictionary::translate(const char *baseString) {
	return translate(baseString,NULL);
}

RaprStringVector *RaprDictionary::ParseNestedField(const char *buf,char *name,RaprDictionaryTransfer *trans)
{
  char *buf1,*buf2;
  RaprStringVector *tmpVal = NULL;
  RaprStringVector *tmpVal2 = NULL;

  int z,zend;
  
  // this is broken but so is location
  // at the risk of breaking other parsing code jury rig this
  //  char *tmp = strchr(buf+1,'%');
  //  char *tmp = strchr(buf,'%');
  char *tmp = strchr(name+1,'%');
  if (tmp == NULL)
    return NULL; // not nested

  z = 1+location(buf+1,'%');
  if (z > 0)
    zend=z+1+location(buf+z+1,'%');
  if (zend-z != 1) 
    return NULL;

  z=location(buf+1,'%');
  buf1=new char[z+1];
  strncpy(buf1,buf+1,z);
  buf1[z]='\0';
  tmpVal = lookup(buf1,trans);

  if (tmpVal != NULL)
    {
      buf2=new char[strlen(buf)];
      strcpy(buf2,name);
      strcat(buf2,(*tmpVal)[0]);

      if (count(buf2,'%')) 
	{
	  DMSG(0,"RaprDictionary::ParseNestedField Error: multi-level translatinos not allowed for nested fields> %s\n",buf2);
	  delete [] buf1;
	  delete [] buf2;
	  return NULL;
	}
      tmpVal2 = lookup(buf2,trans);
      delete [] buf1;
      delete [] buf2;
      return tmpVal2;
    }
  delete [] buf1;
  return NULL;
}


RaprStringVector *RaprDictionary::ParseNestedField(const char *buf,int y,RaprDictionaryTransfer *trans)
{
  char *buf1,*buf2,*buf3;
  RaprStringVector *tmpVal;
  //    RaprStringVector *tmpVal = new RaprStringVector();
  int x,z;

  z=y+1+location(buf+y+1,'%');
  buf1=new char[(z-y)+1];
  strncpy(buf1,buf+y+1,z-y-1);
  buf1[z-y-1] = '\0';
  tmpVal = lookup(buf1,trans);

  if (tmpVal != NULL) {
    if (tmpVal->size() > 1)
      {
	DMSG(0,"RaprDictionary ::translate Error: nested translation fields cannot have multiple values\n");
	delete[] buf1;
	return NULL;
	
      }
    //char *tmp = new char[SCRIPT_LINE_MAX];

    x = location(buf+z+1,'%');
    buf2=new char[x+1];
    strncpy(buf2,buf+z+1,x);
    buf2[x] = '\0';
    buf3=new char[strlen(buf)];
    strcpy(buf3,(*tmpVal)[0]);
    strcat(buf3,buf2);
    tmpVal = lookup(buf3,trans);
	delete [] buf1;
	delete [] buf2;
	delete [] buf3;
    if (tmpVal != NULL) 
      {
          return tmpVal;
      }
  }
  DMSG(0,"RaprDictionary::ParseNestedField() Unable to translate>%s\n",buf1);
  return NULL;
}


//This will take a string and do replace on all %Keyword% inside the 
//string.  This is done combanatorially.  IE if first key word is replace
//twice and second three times, the output will be six unique strings.
RaprStringVector *RaprDictionary::translate(const char *baseString,RaprDictionaryTransfer *trans) 
{
	//res will be deleted by the calling function
	RaprStringVector *res=new RaprStringVector();
	RaprStringVector *work = NULL;
	const char *buf = NULL;
	char *buf1 = NULL;
	char *buf2 = NULL;
	char *buf3 = NULL;
	char *buf4 = NULL;
	RaprStringVector *tmpVal = NULL;
	int x,y,z,zend = 0;
	int i = 0;
	
	if (count(baseString,'%')%2 != 0) {
        DMSG(0,"RaprDictionary::Translate Error:  Misformed String : %s.\n",baseString);
        return NULL;
	}
	
	if (count(baseString,'%') > 0) 
    {
	    //insert the initial string into the working vector
	    res->push_back(baseString);
	    while (count((*res)[0],'%') > 0) 
        {
            work=new RaprStringVector();
            for (i=0;i<(int)(res->size());i++) 
            {
                tmpVal = NULL;
                //get the working string
                buf=(*res)[i];
                //find the first 2 % in the string
                x=location(buf,'%');
                y=x+1+location(buf+x+1,'%');
                buf1=new char[x+1];
                strncpy(buf1,buf,x);
                buf1[x] = '\0';
                
                //Is there a nested value at the
                //beginning of the field?  
                if (((y-x) == 1) && (count((*res)[0],'%') != 2))
                {
                    tmpVal = ParseNestedField(buf,y,trans);
                    
                    // recalc position of the remainder and build buf3
                    z=y+1+location(buf+y+1,'%');
                    zend=z+1+location(buf+z+1,'%');
                    buf3=new char[((strlen(buf)-(zend+1))+1)];
                    strncpy(buf3,buf+zend+1,((strlen(buf)-(zend+1))+1));
                    buf3[((strlen(buf)-(zend+1)+1)-1)] = '\0';
                    DMSG(4,"buf3>%s\n",buf3);
                }
                else 
                {
                    //split the string into 3
                    buf2=new char[y-x];
                    strncpy(buf2,buf+x+1,y-x-1);
                    buf2[y-x-1] = '\0';
                    // Is there a nested field at the end?
                    tmpVal = ParseNestedField(buf+y,buf2,trans);
                    
                    // It was nested
                    if (tmpVal != NULL)
                    {
                        // recalc position of the remainder and build buf3
                        z=y+1+location(buf+y+1,'%');
                        if (z > 0)
                          zend=z+1+location(buf+z+1,'%');
                        
                        // get the remaining string
                        buf3=new char[(strlen(buf)-(zend+1))+1];
                        strncpy(buf3,buf+zend+1,((strlen(buf) - (zend+1))+1));
                        buf3[((strlen(buf) - (zend+1)+1) -1)]='\0';
                        DMSG(4,"2nd buf3> %s\n",buf3);
                    }
                    else
                    {
                        tmpVal = lookup(buf2,trans);
                        // get the remaining string
                        buf3=new char[(strlen(buf)-y)];
                        strcpy(buf3,buf+y+1);
                        DMSG(4,"3rd buf3>%s\n",buf3);
                    }
                }
                
                
                if (tmpVal != NULL) 
                {
                    for (unsigned int j=0;j<tmpVal->size();j++) 
                    {
                        //create the new string
                        buf4 = new char[((strlen(buf1) + strlen(buf3) + strlen((*tmpVal)[j])) + 1)];
                        strcpy(buf4,buf1);
                        strcat(buf4,(*tmpVal)[j]);
                        strcat(buf4,buf3);
                        //put it into a working vector
                        work->push_back(buf4);
                        // delete [] buf4;  push_back refers to this still
                    }
                }
                
                else 
                {
                    // if string contains "default" it's a member
                    // of the default dictionary - don't display error
                    // because we don't ~have~ to have a default
                    // dictionary
                    if (!strcasestr(baseString,"%SystemDefaults:"))
                    {
                        DMSG(0,"RaprDictionary::Translate Error:  Unable to translate in %s.\n",baseString);
                    }
                    return NULL;
                }
                
                //delete the memory, buf4 is now inside the vector so no cleanup required
            }
            if (!work->empty()) {
                //reset the result vector and continue the loop
                delete res;
                res=work;
                work=NULL;
            }
        }
    }
	else 
    {
	    res->push_back(baseString);
    }
	if (buf1) delete [] buf1;
	// if (buf2) delete [] buf2;	// jm:  somehow buf2's data is still referenced
    // jm:  by something.  delete taken out until we find 
    // jm:  that problem - ie, memory leak is better than SEGV
	if (buf3) delete [] buf3;
    
	return res;
}

//Count the number of % inside the string
int RaprDictionary::count(const char *inString,char inSep) {
	int z=0;
	char* localVal;
	localVal = newString(strlen(inString) + 1);
	strcpy(localVal,inString);
	char *tmp = strchr(localVal,inSep);
	while (tmp != NULL) {
		z++;
		tmp = strchr(tmp+1,inSep);
	}
	return z;
}

//allocate and 0 set a new character buffer
char *RaprDictionary::newString(int inSize) {
	char *tmp = NULL;
	tmp = new char [inSize];
	memset(tmp,0,inSize);
	return tmp;
}

//find the first instance of % in the string
int RaprDictionary::location(const char *inString,char inSep) {

        char* localVal;
	localVal = newString(strlen(inString) + 1);
	strcpy (localVal,inString);
	char *tmp = strchr(localVal,inSep);
	return tmp-localVal;
}

bool RaprDictionary::LoadFile(char *inFile) {
	TiXmlDocument doc(inFile);
	
	bool loadOkay = doc.LoadFile();
	if ( !loadOkay )
	{
		DMSG(0, "RaprDictionary::LoadFile Error: can not load file %s.\n",inFile);
		return false;
	}
	
	TiXmlNode* nodeL1 = 0;
	TiXmlNode* tmpNode = 0;
	TiXmlNode* nodeL2 = 0;
	TiXmlNode* nodeL3 = 0;
	TiXmlNode* topnode = 0;
	char *value  = NULL;
	char *value1 = NULL;
	char *value2 = NULL;
	int bReset;

	if (doc.FirstChild("namespace") != NULL) {
		topnode = doc.FirstChild("namespace");
	}
	else if ((doc.FirstChild("RaprDictionary") != NULL) && (doc.FirstChild("RaprDictionary")->FirstChild("namespace") != NULL)) {
		topnode = doc.FirstChild("RaprDictionary")->FirstChild("namespace");
	}
	else {
		DMSG(0, "RaprDictionary::LoadFile() Error: can not find top level tag\n");		
		return false;
	}
	
	for (nodeL1 = topnode;nodeL1;nodeL1 = nodeL1->NextSibling("namespace")) {
		tmpNode = nodeL1->FirstChild("label");
		if ((tmpNode != NULL) && (tmpNode->FirstChild() != NULL)) {
			value = newString(strlen(tmpNode->FirstChild()->Value()) + 1);
			strcpy(value,tmpNode->FirstChild()->Value());
			for (nodeL2 = nodeL1->FirstChild("item");nodeL2;nodeL2=nodeL2->NextSibling("item")) {
				tmpNode = nodeL2->FirstChild("field");
				if ((tmpNode != NULL) && (tmpNode->FirstChild() != NULL)) {
					value1 =newString(strlen(tmpNode->FirstChild()->Value()) + 1);
					strcpy(value1,tmpNode->FirstChild()->Value());
					bReset = 1;
					for (nodeL3 = nodeL2->FirstChild("value");nodeL3;nodeL3=nodeL3->NextSibling("value")) {
						if (nodeL3 != NULL) {
							if (nodeL3->FirstChild() != NULL) {
								value2 = newString(strlen(nodeL3->FirstChild()->Value()) + 1);
								strcpy(value2,nodeL3->FirstChild()->Value());
							}
							else {
								value2 = newString(1);
								memset(value2,0,1);
							}
							DMSG(3, "RaprDictionary load definition %s,%s,%s. %d\n",value,value1,value2,bReset);
							if (bReset == 1) {
								ResetValue(value,value1,value2);
								bReset = 0;
							}
							else {
								SetValue(value,value1,value2);
							}
							delete [] value2; value2 = NULL;
						}
					}
					delete [] value1; value1 = NULL;
				}
			}
			delete [] value; value = NULL;
		}
	}
	return true;
}
