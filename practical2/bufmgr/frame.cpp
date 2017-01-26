#include "../include/frame.h"
#include "../include/db.h"

Frame::Frame() {
	EmptyIt();
}
Frame::~Frame(){
	delete data;
}
void Frame::Pin() {
	pinCount++;
	clock_gettime(CLOCK_REALTIME, &timestamp);
}
void Frame::Unpin() {
	pinCount--;
}
void Frame::EmptyIt() {
	data = new Page();
	pid = INVALID_PAGE;
	pinCount = 0;
	dirty = false;
	clock_gettime(CLOCK_REALTIME, &timestamp);
}
void Frame::DirtyIt() {
	dirty = true;
}
void Frame::SetPageID(PageID pid) {
	this->pid = pid;
}
Bool Frame::IsDirty() {
	return dirty;
}
Status Frame::Write() {
	return MINIBASE_DB->WritePage(pid, data);
}
Status Frame::Read(PageID pid) {
	Status status = MINIBASE_DB->ReadPage(pid, data);
	if (status == OK) this->pid = pid;
	return status;
}
PageID Frame::GetPageID() {
	return pid;
}
Page* Frame::GetPage() {
	return data;
}
int Frame::GetPinCount() {
	return pinCount;
}
long Frame::GetTimeStamp() {
	timestamp.tv_nsec;
}
