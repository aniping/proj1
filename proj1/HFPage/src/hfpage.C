#include <iostream>
#include <stdlib.h>
#include <memory.h>

#include "hfpage.h"
#include "buf.h"
#include "db.h"


// **********************************************************
// page class constructor

void HFPage::init(PageId pageNo)
{
  curPage = pageNo;
  nextPage = INVALID_PAGE;
  prevPage = INVALID_PAGE;
  
  cout << sizeof(slot_t) << endl; 

  usedPtr = MAX_SPACE - DPFIXED ;
  freeSpace = MAX_SPACE - DPFIXED ;
  
  slotCnt = 0;
  slot[0].length = EMPTY_SLOT;
  
}

// **********************************************************
// dump page utlity
void HFPage::dumpPage()
{
    int i;

    cout << "dumpPage, this: " << this << endl;
    cout << "curPage= " << curPage << ", nextPage=" << nextPage << endl;
    cout << "usedPtr=" << usedPtr << ",  freeSpace=" << freeSpace
         << ", slotCnt=" << slotCnt << endl;
   
    for (i=0; i < slotCnt; i++) {
        cout << "slot["<< i <<"].offset=" << slot[i].offset
             << ", slot["<< i << "].length=" << slot[i].length << endl;
    }
}

// **********************************************************
PageId HFPage::getPrevPage()
{
	
    return prevPage;
}

// **********************************************************
void HFPage::setPrevPage(PageId pageNo)
{

    prevPage = pageNo;
}

// **********************************************************
PageId HFPage::getNextPage()
{

    return nextPage;
}

// **********************************************************
void HFPage::setNextPage(PageId pageNo)
{

	nextPage = pageNo;
}

// **********************************************************
// Add a new record to the page. Returns OK if everything went OK
// otherwise, returns DONE if sufficient space does not exist
// RID of the new record is returned via rid parameter.
Status HFPage::insertRecord(char* recPtr, int recLen, RID& rid)
{
	if (recLen > freeSpace)
		return DONE;
	
	usedPtr -= recLen;
	
	for (int i = 0; i < recLen; i++) {
		data[usedPtr + i] = *(recPtr + i);
	}
	
	slot[slotCnt].offset = usedPtr;
	slot[slotCnt].length = recLen;

	rid.pageNo = curPage;
	rid.slotNo = slotCnt;
	
	slotCnt++;
	
	freeSpace -= (recLen + sizeof(slot_t));
	
	
    return OK;
}

// **********************************************************
// Delete a record from a page. Returns OK if everything went okay.
// Compacts remaining records but leaves a hole in the slot array.
// Use memmove() rather than memcpy() as space may overlap.
Status HFPage::deleteRecord(const RID& rid)
{
	if (slotCnt <= rid.slotNo || rid.slotNo < 0)
		return FAIL;
	
	int length = slot[rid.slotNo].length;

	void *new_ptr = (void *) ((long) usedPtr + (long) &slot + length);

	memmove(new_ptr, (void *) ((long) usedPtr + (long) &slot), length);
	
	usedPtr += length;
	slot[rid.slotNo].length = EMPTY_SLOT;

	for(int i=rid.slotNo+1; i<slotCnt; i++){
		slot[i].offset+= length;
	}

	for(int i=0; i<slotCnt; i++){
		cout<<"slot no"<<i<< "  " <<(double) data[slot[i].offset] << endl;
	}
	
	while(slot[slotCnt-1].length == EMPTY_SLOT){
		slotCnt--;
		freeSpace+=sizeof(slot_t);
	}
	
	freeSpace += length; 
	
    return OK;
}

// **********************************************************
// returns RID of first record on page
Status HFPage::firstRecord(RID& firstRid)
{

	if (slotCnt < 1) 
		return DONE;

	firstRid.pageNo = curPage;
	firstRid.slotNo = 0;

	while (slot[firstRid.slotNo].length == EMPTY_SLOT) {
		firstRid.slotNo++;
	}

	
    return OK;
}

// **********************************************************
// returns RID of next record on the page
// returns DONE if no more records exist on the page; otherwise OK
Status HFPage::nextRecord (RID curRid, RID& nextRid)
{

	if(curRid.pageNo != curPage || slotCnt <= curRid.slotNo || curRid.slotNo < 0 || slot[curRid.slotNo].length == EMPTY_SLOT)
		return FAIL;
	
	if (slotCnt == curRid.slotNo + 1)
		return DONE;
	
	nextRid.pageNo = curPage;
	nextRid.slotNo = curRid.slotNo + 1;
	
	// makes sure the slot being returned is not empty
	while (slot[nextRid.slotNo].length == EMPTY_SLOT) {
		nextRid.slotNo++;
	}
	
    return OK;
}

// **********************************************************
// returns length and copies out record with RID rid
Status HFPage::getRecord(RID rid, char* recPtr, int& recLen)
{
	if (rid.pageNo != curPage || slotCnt <= rid.slotNo || rid.slotNo < 0 || slot[rid.slotNo].length == EMPTY_SLOT)
		return FAIL;
	
	slot_t s = slot[rid.slotNo];
	
	memcpy(recPtr, &data[s.offset], s.length);
	recLen = s.length;
	
	
    return OK;
}

// **********************************************************
// returns length and pointer to record with RID rid.  The difference
// between this and getRecord is that getRecord copies out the record
// into recPtr, while this function returns a pointer to the record
// in recPtr.
Status HFPage::returnRecord(RID rid, char*& recPtr, int& recLen)
{
   	if (rid.pageNo != curPage || slotCnt <= rid.slotNo || rid.slotNo < 0 || slot[rid.slotNo].length == EMPTY_SLOT)
		return FAIL;
	
	slot_t s = slot[rid.slotNo];
	
	recPtr = &data[s.offset];
	recLen = s.length;
	
    return OK;
}

// **********************************************************
// Returns the amount of available space on the heap file page
int HFPage::available_space(void)
{

    return freeSpace;
}

// **********************************************************
// Returns 1 if the HFPage is empty, and 0 otherwise.
// It scans the slot directory looking for a non-empty slot.
bool HFPage::empty(void)
{
	int i = 0;
	while (i < slotCnt) {
		if (slot[i].length != EMPTY_SLOT)
			return false;
		
		i++;
	}
	
    return true;
}



