/**  a log file
*/
#pragma once

extern volatile bool lfWriting; ///< true if writing the log file right now
enum LFState {
	lfSInit,
	lfSGood,
	lfSError
};
extern volatile LFState lfState; ///< lfSGood if SD state is good

///  init log file
void lfInit();
/**   start log to file 
 */ 
void lfStart(const char* fileName = nullptr);
/// stop log
void lfStop();

/**  add data to the file.
 *  can be called from the interrupt.
 * */
void lfFeed(void* data, int size);

int lfSendMessage(const unsigned char* data, unsigned char type, unsigned short int size);
template<class T> int lfSendMessage(const T* info) {
	int ret;
	ret = lfSendMessage((unsigned char*)(info), T::type, sizeof(T));
	return ret;
}

/** save info to the log file.
 *  call this not from the interrupt.
 * */
void lfProcess();

void lfPrint();



