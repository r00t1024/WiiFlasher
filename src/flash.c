#define NAND_PAGE_SIZE     0x840
#define NAND_BLOCK_SIZE    NAND_PAGE_SIZE*64

#include "flash.h"
#include "tools.h"

void dumpBlock(const char *filename, int block){
	static unsigned char buffer[NAND_BLOCK_SIZE] __attribute__ ((aligned(32)));
	s32 fd = -1;
	int rv;

	printf("Dumping block %d...\n", block);

	FILE *fout = fopen(filename, "wb");
	if(fout == NULL){
		printf("Error: cannot open file %s\n", filename);
		WaitExit();
	}

	fd = IOS_Open("/dev/flash", IPC_OPEN_READ);
	if (fd < 0) {
		printf("Failed to open /dev/flash (fd = %d)\n", fd);
		WaitExit();
	}

	rv = IOS_Seek(fd, block * 64, 0);
	if (rv < 0){
		printf("Failed to seek to block %d (rv = %d)\n", block, rv);
		WaitExit();
	}

	for (int page = 0; page < 64; page++) {
		rv = IOS_Seek(fd, block * 64 + page, 0);
		if (rv < 0){
			printf("Failed to seek to page %d (rv = %d)\n", block * 64 + page, rv);
			WaitExit();
		}

		rv = IOS_Read(fd, buffer, (u32) NAND_PAGE_SIZE);

		fwrite(buffer, NAND_PAGE_SIZE, 1, fout);
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

	rv = IOS_Ioctl(fd, 3, NULL, 0, NULL, 0); // Erase block
	if(rv < 0){
		printf("Failed to erase block %d (rv = %d)\n", block, rv);
		WaitExit();
	}

	for (int page = 0; page < 64; page++) {
		rv = IOS_Seek(fd, block * 64 + page, 0);
		if (rv < 0){
			printf("Failed to seek to page %d (rv = %d)\n", block * 64 + page, rv);
			WaitExit();
		}

		fread(buffer, NAND_PAGE_SIZE, 1, fin);

		rv = IOS_Write(fd, buffer, (u32) NAND_PAGE_SIZE);

		if(rv != 2112)
			printf("Write error: page %d block %d (rv = %d)\n", page, block, rv);
	}

	IOS_Close(fd);
	fclose(fin);
}
