#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#include "include/minirel.h"
#include "include/bufmgr.h"
#include "include/heapfile.h"
#include "include/join.h"
#include "include/relation.h"

int MINIBASE_RESTART_FLAG = 0;// used in minibase part

#define NUM_OF_DB_PAGES  5000 // define # of DB pages
#define NUM_OF_BUF_PAGES 50 // define Buf manager size.You will need to change this for the analysis
#define REPS 10

void printStats(int sizeBuf, int sizeR, int sizeS) {
	Status s;

	long pinRequests = 0;
	long pinMisses = 0;
	double duration = 0;

	long pinRequests0 = 0;
	long pinMisses0 = 0;
	double duration0 = 0;

	long pinRequests1 = 0;
	long pinMisses1 = 0;
	double duration1 = 0;

	int B;

	JoinSpec specOfS, specOfR;

	srand(1);

	for (int i = 0; i < REPS; i++) {
		minibase_globals = new SystemDefs(
			s,
			"MINIBASE.DB",
			"MINIBASE.LOG",
			NUM_OF_DB_PAGES,   // Number of pages allocated for database
			500,
			sizeBuf,  // Number of frames in buffer pool
			NULL
		);

		CreateR(sizeR, sizeS);
		CreateS(sizeS);

		CreateSpecForR(specOfR);
		CreateSpecForS(specOfS);

		B = (MINIBASE_BM->GetNumOfBuffers()-3*3)*MINIBASE_PAGESIZE;
		// B = (MINIBASE_BM->GetNumOfUnpinnedFrames()-3*3)*MINIBASE_PAGESIZE;

		pinRequests = 0;
		pinMisses = 0;
		duration = 0;
		TupleNestedLoopJoin(specOfR, specOfS, pinRequests, pinMisses, duration);
		pinRequests0 += pinRequests;
		pinMisses0 += pinMisses;
		duration0 += duration;

		pinRequests = 0;
		pinMisses = 0;
		duration = 0;
		BlockNestedLoopJoin(specOfR, specOfS, B, pinRequests, pinMisses, duration);
		pinRequests1 += pinRequests;
		pinMisses1 += pinMisses;
		duration1 += duration;

		remove("MINIBASE.DB");
	}

	cout << "TupleNestedLoopJoin:" << endl;
	cout << "pinRequests: " << pinRequests0 / REPS << endl;
	cout << "pinMisses: " << pinMisses0 / REPS << endl;
	cout << "duration: " << duration0 / REPS << "s" << endl;

	cout << endl;
	cout << "BlockNestedLoopJoin (B=" << B << "):" << endl;
	cout << "pinRequests: " << pinRequests1 / REPS << endl;
	cout << "pinMisses: " << pinMisses1 / REPS << endl;
	cout << "duration: " << duration1 / REPS << "s" << endl;
}

int main() {
	printStats(NUM_OF_BUF_PAGES, NUM_OF_REC_IN_R, NUM_OF_REC_IN_S);

	return 0;
}
