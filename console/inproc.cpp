


#include "inproc.h"


#include <chrono>
#include <thread>
#include <iostream>
#ifdef WIN32
	#include <conio.h>
#else
	#include <termios.h>
#endif
#include "xmroundbuf.h"


#ifdef WIN32
int getche(){
	if (_kbhit()) {
		return _getch();
	} else {
		return -1;
	}
}
#else
static struct termios old, current;
volatile bool inpExitRequest = false;

/* Initialize new terminal i/o settings */
void initTermios(int echo) 
{
  tcgetattr(0, &old); /* grab old terminal i/o settings */
  current = old; /* make new settings same as old settings */
  current.c_lflag &= ~ICANON; /* disable buffered i/o */
  if (echo) {
      current.c_lflag |= ECHO; /* set echo mode */
  } else {
      current.c_lflag &= ~ECHO; /* set no echo mode */
  }
  tcsetattr(0, TCSANOW, &current); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios(void) 
{
  tcsetattr(0, TCSANOW, &old);
}

/* Read 1 character - echo defines echo mode */
char getch_(int echo) 
{
  char ch;
  initTermios(echo);
  ch = getchar();
  resetTermios();
  return ch;
}

/* Read 1 character without echo */
char getch(void) 
{
  return getch_(0);
}

/* Read 1 character with echo */
char getche(void) 
{
  return getch_(1);
}

#endif

void inputProc(void(cb)(char*)) {
	int ch;
	using namespace std::chrono_literals;
	const int cmdSize = 256;
	char cmd[cmdSize];
	XMRoundBuf<std::string, 10> cmdList;
	int cmdListIndex = 0;
	int cmdIndex = 0;

    printf("starting inp thread \n");

	while (!inpExitRequest) {
        //printf(".");
		ch = getche();
		if (ch == -1) {
			if (inpExitRequest) {
				break;
			}
			std::this_thread::sleep_for(2ms);
			continue;
		}

		if (cmdIndex > cmdSize - 3) {
			cmdIndex = 0;
		}
		if ((ch == 0) || (ch == 224)) {
			ch = getche();
			bool ok = false;
			switch (ch) {
			case 67:  //  F9
				break;
			case 72: //  (up)
				if (cmdListIndex > 0) {
					cmdListIndex--;
					ok = true;
				}
				break;
			case 80: // 80 (down)
				if (cmdListIndex < (cmdList.num - 1)) {
					cmdListIndex--;
					ok = true;
				}
				break;

			case 75:  //  left
				break;
			case 77: // right
				break;
			case 115: //  ctrl+left
				break;

			case 116: //  ctrl + right
				break;
			case 134:  //  F12
				//system("cls");
				break;

			case 59: //  F1 - F8
			case 60:
			case 61:
			case 62:
			case 63:
			case 64:
			case 65:
			case 66: 
					 printf("F %d\n", ch - 58);
					 break;
			case 68: //  F10
				break;
			case 133:  // F11
				break;

			default:
				xm_printf("got scan code %d\n", ch);
			};

			if (ok) {
				printf("\r                                                           \r");
				strcpy(cmd, cmdList[cmdListIndex].c_str());
				cmdIndex = strlen(cmd);
				cmd[cmdIndex] = 0;
				printf("%s", cmd);
			}
			continue;
		}
		
		cmd[cmdIndex] = ch;
		cmdIndex++;

		//printf("%c", char(ch & 0x00FF));
		ch = toupper(ch);
		if ((ch == 'Q') || (inpExitRequest)) {
			inpExitRequest = true;
			break;
		}


		if ((ch == '\n') || (ch == '\r')) {
            if (cmdIndex < 1) {
                continue;
            }
			cmd[cmdIndex-1] = 0;
            //printf("inp: got {%s} \n", cmd);
			
			//cmd[cmdIndex - 1] = 0;
			cmdListIndex = 0;
			std::cout << std::endl;
			if (strncmp(cmd, "cls", 3) == 0) {
				//clrscr();
				//system("cls");
			} else {
				cb(cmd);
				cmdList.put(std::string(cmd));
				cmdListIndex = cmdList.num - 1;
			}

			//std::cout << " sending " << cmd << std::endl;
			//std::cout << "ret = " << ret << std::endl;
			cmdIndex = 0;
		}

	}
	int abc = 0;
    inpExitRequest = true;
    printf("exiting inp thread \n");

}

