#include "flash.h"
#include "tools.h"

#define IOS_SLOT           15

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

int main(int argc, char **argv){
	IOS_ReloadIOS(IOS_SLOT); // Need IOS15, IOS16 or maybe IOS255

	VIDEO_Init();
	WPAD_Init();
	PAD_Init();
	rmode = VIDEO_GetPreferredMode(NULL);
	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	console_init(xfb,20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);

	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(xfb);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();
	
	if(!fatInitDefault()){
		printf("Error: cannot init FAT\n");
		WaitExit();
	}

	printf("\x1b[2;0H");
	printf("Wii flasher!\n");

	// EXAMPLE
	//dumpBlock("/blk2dmp1.bin", 2);
	//flashBlock("/block2.bin", 2);
	//dumpBlock("/blk2dmp2.bin", 2);

	printf("Done\n");

	WaitExit();
	return 0;
}
