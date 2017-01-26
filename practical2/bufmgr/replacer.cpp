#include "../include/replacer.h"


Replacer::Replacer() {}
Replacer::~Replacer() {}

Clock::Clock( int bufSize, Frame **frames) {
	this->frames = frames;
	this->numOfBuf = bufSize;
	this->current = 0;
}

Clock::~Clock() {}

int Clock::PickVictim() {
	for (int i = 0; i < numOfBuf; i++) {
		if (frames[(current+i)%numOfBuf]->GetPinCount() == 0) {
			current = (current+i)%numOfBuf;
			return current;
		}
	}
	return INVALID_FRAME;
}

LRU::LRU( int bufSize, Frame **frames) {
	this->frames = frames;
	this->numOfBuf = bufSize;
}

LRU::~LRU() {}

int LRU::PickVictim() {
	timespec t;
	clock_gettime(CLOCK_REALTIME, &t);
	long lr = t.tv_nsec;
	int victim = INVALID_FRAME;
	for (int i = 0; i < numOfBuf; i++) {
		if (frames[i]->GetPageID() == INVALID_PAGE) return i;
		if (frames[i]->GetPinCount() == 0 && frames[i]->GetTimeStamp() < lr) {
			lr = frames[i]->GetTimeStamp();
			victim = i;
		}
	}
	return victim;
}
