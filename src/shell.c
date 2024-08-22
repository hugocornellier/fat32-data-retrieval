#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "../headers/shell.h"
#include "../headers/fat32.h"
#include <stdbool.h>

int running = true;
char buffer[BUF_SIZE];
char bufferRaw[BUF_SIZE];

void shellLoop(int fd, char* filename) {
    init(filename, fd);

    // Start loop
	while (running) {
		printf(">");
		if (fgets(bufferRaw, BUF_SIZE, stdin) == NULL) {
			running = false;
			continue;
		}
		bufferRaw[strlen(bufferRaw)-1] = '\0';
		for (int i = 0; i < strlen(bufferRaw)+1; i++)
			buffer[i] = (char)toupper(bufferRaw[i]);
        char** commands = malloc(sizeof(char**) * 256);
        tokenize(commands, buffer, " ");
        if (strncmp(buffer, CMD_INFO, strlen(CMD_INFO)) == 0)
            printInfo();
        else if (strncmp(buffer, CMD_DIR, strlen(CMD_DIR)) == 0)
            doDir();
        else if (strncmp(buffer, CMD_CD, strlen(CMD_CD)) == 0)
            doCd(*(commands + 1));
        else if (strncmp(buffer, CMD_GET, strlen(CMD_GET)) == 0)
            doDownload(*(commands + 1));
        else if (strncmp(buffer, CMD_PUT, strlen(CMD_PUT)) == 0)
            doPut(*(commands + 1));
        else printf("Command not found\n");
	}
	printf("\n");
}

// Read disk image to bootbuffer
// If successful, copy data to bootSector
void init(char* filename, int fd) {
    filepath = filename;
    bootbuffer = malloc(BUFFER_SIZE);
    fp = fopen(filepath, "rb+");
    fread(&bootSector, sizeof(fat32BS), 1, fp);
    bs = &bootSector;
    storeVolID();
}

// Fetch volume ID from root dir & store it in vol_ID
void storeVolID() {
    curDirClus = bs->BPB_RootClus;
    readSectorNum(getSectorNum(bs->BPB_RootClus, bs));
    fat32Dir* dir = (fat32Dir*)&buf[0];
    vol_ID = dir->DIR_Name;
}