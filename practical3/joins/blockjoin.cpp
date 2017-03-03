#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../include/minirel.h"
#include "../include/heapfile.h"
#include "../include/scan.h"
#include "../include/join.h"
#include "../include/relation.h"
#include "../include/bufmgr.h"

//---------------------------------------------------------------
// Each join method takes in at least two parameters :
// - specOfS
// - specOfR
//
// They specify which relations we are going to join, which 
// attributes we are going to join on, the offsets of the 
// attributes etc.  specOfS specifies the inner relation while
// specOfR specifies the outer one.
//
//You can use MakeNewRecord() to create the new result record.
//
// Remember to clean up before exiting by "delete"ing any pointers
// that you "new"ed.  This includes any Scan/BTreeFileScan that 
// you have opened.
//---------------------------------------------------------------

void BlockNestedLoopJoin(JoinSpec specOfR, JoinSpec specOfS, int B, long& pinRequests, long& pinMisses, double& duration)
{
	MINIBASE_BM->ResetStat();
	clock_t start = clock();
	Status status = OK;

	Scan* scanR = specOfR.file->OpenScan(status);
	if (status != OK) exit(1);

	HeapFile* result = new HeapFile(NULL, status);
	if (status != OK) exit(1);

	RecordID ridR, ridS, ridRes;
	char* ptrR = new char[specOfR.recLen];
	char* ptrS = new char[specOfS.recLen];

	int recLenRes = specOfS.recLen + specOfR.recLen;
	char* ptrRes = new char[recLenRes];

	char* ptrBlock = new char[B];
	int lenR = specOfR.recLen;
	int recsPerBlock = B / lenR;

	bool done = false;
	while (!done) {
		int read;
		for (read = 0; read < recsPerBlock; read++) {
			if (scanR->GetNext(ridR, ptrBlock + read*lenR, lenR) != OK) {
				done = true;
				break;
			}
		}

		Scan* scanS = specOfS.file->OpenScan(status);
		if (status != OK) exit(1);

		while (scanS->GetNext(ridS, ptrS, specOfS.recLen) == OK) {
			for (int i = 0; i < read; i++) {
				if (*((int*)(ptrS + specOfS.offset)) == *((int*)(ptrBlock+i*lenR+specOfR.offset))) {
					MakeNewRecord(ptrRes, ptrBlock+i*lenR, ptrS, specOfR.recLen, specOfS.recLen);
					result->InsertRecord(ptrRes, recLenRes, ridRes);
				}
			}
		}
		delete scanS;
	}

	delete scanR;
	delete[] ptrR, ptrS, ptrRes, ptrBlock;
	delete result;

	MINIBASE_BM->GetStat(pinRequests, pinMisses);
	duration = (clock() - start) / (double) CLOCKS_PER_SEC;
}
