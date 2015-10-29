#include "raprUBIState.h"
#include <stddef.h>

RaprUBIStateStore::RaprUBIStateStore() {
  ubi = 0;
  state = 0;
}

RaprUBIStateStore::~RaprUBIStateStore() {
}

RaprUBIState::UBIStateStorage::iterator RaprUBIState::find(unsigned int inUBI) {
	if (storage.size() == 0) return storage.end();
	RaprUBIState::UBIStateStorage::iterator it;
	for (it = storage.begin();it != storage.end();it++) {
		if (*it != NULL) {
			if ((*it)->GetUBI() == inUBI) {
				return it;
			}
		}
	}
	return storage.end();
}

RaprUBIState::RaprUBIState() {
}

RaprUBIState::~RaprUBIState() {
	RaprUBIState::UBIStateStorage::iterator it;
	RaprUBIStateStore *tmp;
	while (storage.size() > 0) {
		it = storage.begin();
		tmp = *it;
		delete tmp;
		storage.erase(it);
	}
}

int RaprUBIState::GetState(unsigned int inUBI) {
	RaprUBIState::UBIStateStorage::iterator it;
	it = find(inUBI);
	if (it == storage.end()) {
		return 0;
	}
	return (*it)->GetState();
}

void RaprUBIState::SetState(unsigned int inUBI,int inState) {
	RaprUBIState::UBIStateStorage::iterator it;
	RaprUBIStateStore *tmp;
	it=find(inUBI);
	if (it == storage.end()) {
		if ((int)(storage.size()) == MAXSIZE) {
			it = storage.begin();
			delete *it;
			storage.erase(it);
		}
		tmp = new RaprUBIStateStore();
		tmp->SetUBI(inUBI);
		tmp->SetState(inState);
		storage.push_back(tmp);
	}
	else {
		tmp = (*it);
		tmp->SetState(inState);
		storage.erase(it);
		storage.push_back(tmp);
	}
}
