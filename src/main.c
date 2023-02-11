#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gccore.h>
#include <string.h>
#include <wiiuse/wpad.h>
#include <gctypes.h>
#include <fat.h>
#include <malloc.h>

#define NAND_PAGE_SIZE     0x840
#define NAND_BLOCK_SIZE    NAND_PAGE_SIZE*64

#define IOS_SLOT           15

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

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

void dumpBlock(const char *filename, int block){
    static unsigned char buffer[NAND_BLOCK_SIZE] __attribute__ ((aligned(32)));
    s32 fd = -1;
    int rv;

    printf("Dumping block %d...\n", block);

    FILE *fout = fopen(filename, "wb");
    if(fout == NULL){
        printf("Error: cannot open file %s\n", filename);
        exit(0);
    }

    fd = IOS_Open("/dev/flash", IPC_OPEN_READ);
	if (fd < 0) {
		printf("Failed to open /dev/flash (fd = %d)\n", fd);
		exit(0);
	}

    rv = IOS_Seek(fd, block * 64, 0);
    if (rv < 0){
        printf("Failed to seek to block %d (rv = %d)\n", block, rv);
		exit(0);
    }

    for (int page = 0; page < 64; page++) {
        rv = IOS_Seek(fd, block * 64 + page, 0);
        if (rv < 0){
            printf("Failed to seek to page %d (rv = %d)\n", block * 64 + page, rv);
		    exit(0);
        }

		rv = IOS_Read(fd, buffer, (u32) NAND_PAGE_SIZE);

        fwrite(buffer, NAND_PAGE_SIZE, 1, fout);

        if(page % 16 == 0){
            printf("Page %d: ", page);
            for(int j=0; j<16; j++) // Print the first 16 bytes of the dumped page (debugging)
                printf("%02X ", buffer[j]);
            printf("\n");
        }
	}

    IOS_Close(fd);
    fclose(fout);
}

void flashBlock(const char *filename, int block){
    static unsigned char buffer[NAND_BLOCK_SIZE] __attribute__ ((aligned(32)));
    s32 fd = -1;
    int rv;

    printf("Flashing block %d...\n", block);

    FILE *fin = fopen(filename, "rb");
    if(fin == NULL){
        printf("Error: cannot open file %s\n", filename);
        WaitExit();
    }

    fd = IOS_Open("/dev/flash", IPC_OPEN_WRITE);
	if (fd < 0) {
		printf("Failed to open /dev/flash (fd = %d)\n", fd);
		WaitExit();
	}

    rv = IOS_Seek(fd, block * 64, 0);
    if (rv < 0){
        printf("Failed to seek to block %d (rv = %d)\n", block, rv);
		WaitExit();
    }

    rv = IOS_Ioctl(fd, 3, NULL, 0, NULL, 0); // This at least works
    if(rv < 0){
        printf("Failed to erase block %d (rv = %d)\n", block, rv);
		WaitExit();
    }

    for (int page = 0; page < 64; page++) {
        rv = IOS_Seek(fd, block * 64 + page, 0);
        if (rv < 0){
            printf("Failed to seek to page %d (rv = %d)\n", block * 64 + page, rv);
		    exit(0);
        }

        fread(buffer, NAND_PAGE_SIZE, 1, fin);

		rv = IOS_Write(fd, buffer, (u32) NAND_PAGE_SIZE);

        if(rv != 2112)
            printf("Write error: page %d block %d (rv = %d)\n", page, block, rv);
	}

    IOS_Close(fd);
    fclose(fin);
}

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
    printf("Wii bricker!\n");

    dumpBlock("/blk2dmp1.bin", 2);
    flashBlock("/block2.bin", 2);
    dumpBlock("/blk2dmp2.bin", 2);

    printf("Done.\n");
    
    WaitExit();
	return 0;
}
