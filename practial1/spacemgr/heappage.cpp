#include <iostream>
#include <stdlib.h>
#include <memory.h>

#include "../include/heappage.h"
#include "../include/heapfile.h"
#include "../include/bufmgr.h"
#include "../include/db.h"

using namespace std;

//------------------------------------------------------------------
// Constructor of HeapPage
//
// Input     : Page ID
// Output    : None
//------------------------------------------------------------------

void HeapPage::Init(PageID pageNo)
{
	pid = pageNo;
	nextPage = INVALID_PAGE;
	prevPage = INVALID_PAGE;
	numOfSlots = 0;
	fillPtr = HEAPPAGE_DATA_SIZE;
	freeSpace = HEAPPAGE_DATA_SIZE;
}

void HeapPage::SetNextPage(PageID pageNo)
{
	nextPage = pageNo;
}

void HeapPage::SetPrevPage(PageID pageNo)
{
	prevPage = pageNo;
}

PageID HeapPage::GetNextPage()
{
	return nextPage;
}

PageID HeapPage::GetPrevPage()
{
	return prevPage;
}


//------------------------------------------------------------------
// HeapPage::InsertRecord
//
// Input     : Pointer to the record and the record's length 
// Output    : Record ID of the record inserted.
// Purpose   : Insert a record into the page
// Return    : OK if everything went OK, DONE if sufficient space 
//             does not exist
//------------------------------------------------------------------

Status HeapPage::InsertRecord(char *recPtr, int length, RecordID& rid)
{
	if (length > AvailableSpace()) return DONE;
	freeSpace -= length + sizeof(Slot);
	fillPtr -= length;
	slots[numOfSlots].offset = fillPtr;
	slots[numOfSlots].length = length;
	memcpy(data + fillPtr, recPtr, length);
	rid.pageNo = pid;
	rid.slotNo = numOfSlots;
	numOfSlots += 1;
	return OK;
}

// Validates Record ID.
bool HeapPage::validate(const RecordID& rid) {
	return rid.pageNo == pid && 0 <= rid.slotNo && rid.slotNo < numOfSlots && !SLOT_IS_EMPTY(slots[rid.slotNo]);
}

//------------------------------------------------------------------
// HeapPage::DeleteRecord 
//
// Input    : Record ID
// Output   : None
// Purpose  : Delete a record from the page
// Return   : OK if successful, FAIL otherwise  
//------------------------------------------------------------------ 

Status HeapPage::DeleteRecord(const RecordID& rid)
{
	if (!validate(rid))
		return FAIL;
	short length = slots[rid.slotNo].length;
	short offset = slots[rid.slotNo].offset;
	memmove(data + fillPtr + length, data + fillPtr, offset - fillPtr);
	for (int i = rid.slotNo + 1; i < numOfSlots; i++) {
		slots[i].offset += length;
	}
	SLOT_SET_EMPTY(slots[rid.slotNo]);
	return OK;
}


//------------------------------------------------------------------
// HeapPage::FirstRecord
//
// Input    : None
// Output   : record id of the first record on a page
// Purpose  : To find the first record on a page
// Return   : OK if successful, DONE otherwise
//------------------------------------------------------------------

Status HeapPage::FirstRecord(RecordID& rid)
{
	for (int i = 0; i < numOfSlots; i++) {
		if (!SLOT_IS_EMPTY(slots[i])) {
			rid.pageNo = pid;
			rid.slotNo = i;
			return OK;
		}
	}
	return DONE;
}


//------------------------------------------------------------------
// HeapPage::NextRecord
//
// Input    : ID of the current record
// Output   : ID of the next record
// Return   : Return DONE if no more records exist on the page; 
//            otherwise OK
//------------------------------------------------------------------

Status HeapPage::NextRecord (RecordID curRid, RecordID& nextRid)
{
	if (!validate(curRid))
		return FAIL;
	for (int i = curRid.slotNo + 1; i < numOfSlots; i++) {
		if (!SLOT_IS_EMPTY(slots[i])) {
			nextRid.pageNo= pid;
			nextRid.slotNo = i;
			return OK;
		}
	}
	return DONE;
}


//------------------------------------------------------------------
// HeapPage::GetRecord
//
// Input    : Record ID
// Output   : Records length and a copy of the record itself
// Purpose  : To retrieve a _copy_ of a record with ID rid from a page
// Return   : OK if successful, FAIL otherwise
//------------------------------------------------------------------

Status HeapPage::GetRecord(RecordID rid, char *recPtr, int& length)
{
	if (!validate(rid))
		return FAIL;
	length = slots[rid.slotNo].length;
	memcpy(recPtr, data + slots[rid.slotNo].offset, length);
	return OK;
}


//------------------------------------------------------------------
// HeapPage::ReturnRecord
//
// Input    : Record ID
// Output   : pointer to the record, record's length
// Purpose  : To output a _pointer_ to the record
// Return   : OK if successful, FAIL otherwise
//------------------------------------------------------------------

Status HeapPage::ReturnRecord(RecordID rid, char*& recPtr, int& length)
{
	if (!validate(rid))
		return FAIL;
	length = slots[rid.slotNo].length;
	recPtr = data + slots[rid.slotNo].offset;
	return OK;
}


//------------------------------------------------------------------
// HeapPage::AvailableSpace
//
// Input    : None
// Output   : None
// Purpose  : To return the amount of available space
// Return   : The amount of available space on the heap file page.
//------------------------------------------------------------------

int HeapPage::AvailableSpace(void)
{
	return freeSpace - sizeof(Slot);
}


//------------------------------------------------------------------
// HeapPage::IsEmpty
// 
// Input    : None
// Output   : None
// Purpose  : Check if there is any record in the page.
// Return   : true if the HeapPage is empty, and false otherwise.
//------------------------------------------------------------------

bool HeapPage::IsEmpty(void)
{
	return numOfSlots == 0;
}


void HeapPage::CompactSlotDir()
{
	short newNumOfSlots = 0;
	short newFillPtr = HEAPPAGE_DATA_SIZE;
	short newFreeSpace = HEAPPAGE_DATA_SIZE;
	short lastSlot = 0;
	for (short i = 0; i < numOfSlots; i++) {
		if (SLOT_IS_EMPTY(slots[i])) continue;
		short length = slots[i].length;
		newFillPtr -= length;
		slots[lastSlot].length = length;
		slots[lastSlot].offset = newFillPtr;
		newFreeSpace -= length + sizeof(Slot);
		memmove(data + newFillPtr, data + slots[i].offset, length);
		lastSlot++;
	}
	fillPtr = newFillPtr;
	freeSpace = newFreeSpace;
	numOfSlots = lastSlot;
}

int HeapPage::GetNumOfRecords()
{
	return numOfSlots;
}
