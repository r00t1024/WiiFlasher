#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <gctypes.h>
#include <fat.h>

#ifndef FLASH_H_
#define FLASH_H_

void dumpBlock(const char *filename, int block);
void flashBlock(const char *filename, int block);

#endif
