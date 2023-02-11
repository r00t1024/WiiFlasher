#include "tools.h"

void WaitExit( void ) {
	u8 i = 0;
	u32 wpadButtons = 0;
	u32 padButtons = 0;
	printf("\n\nPress any controller button to exit.");
	while(1) {
		WPAD_ScanPads();
		PAD_ScanPads();
		for(i = 0; i < 4; i++) {
			wpadButtons += WPAD_ButtonsDown(i);
			padButtons += PAD_ButtonsDown(i);
		}
		if (wpadButtons || padButtons) exit(0);

		VIDEO_WaitVSync();
	}
}
