#ifndef FLASH_H_
#define FLASH_H_

void dumpBlocks(const char *filename, int firstBlock, int lastBlock);
void flashBlocks(const char *filename, int firstBlock, int lastBlock);

void dumpBlock(const char *filename, int block);
void flashBlock(const char *filename, int block);

#endif
