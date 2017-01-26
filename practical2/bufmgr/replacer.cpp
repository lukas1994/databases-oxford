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

