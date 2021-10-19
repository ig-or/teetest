/**  a log file
*/
#pragma once

///  init log file
void lfInit();
/**   start log to file 
 */ 
void lfStart(const char* fileName);
/// stop log
void lfStop();

/**  add data to the file.
 *  can be called from the interrupt.
 * */
void lfFeed(void* data, int size);

/** save info to the log file.
 *  call this not from the interrupt.
 * */
void lfProcess();

void lfPrint();



