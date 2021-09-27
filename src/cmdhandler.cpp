

#include "cmdhandler.h"
#include "teetools.h"

static const int infoSize = 88;
static char info[infoSize];
static int infoIndex = 0;

void onIncomingInfo(char* s, int size) {
	int i;
	for (i = 0; i < size; i++) {
		if (i + infoIndex >= (infoSize-1)) {  //   overflow?
			infoIndex = 0;  //  reset!
		}
		if ((s[i] == 0) || (s[i] == '\n') || (s[i] == '\r')) {
			info[infoIndex] = 0;
			processTheCommand(info, infoIndex);

			infoIndex = 0;
		} else {
			info[infoIndex] = s[i];
			infoIndex++;
		}
	}
	
}

int processTheCommand(char* s, int size) {
    xmprintf(0, "got cmd size=%d (%s)", size, s);
    return 0;
}

