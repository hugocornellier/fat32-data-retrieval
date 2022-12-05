#include "fat32.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

// Returns the size of a file in bytes
long getSizeFseek(const char *path) {
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        printf("Open error %s\n", path);
        exit(EXIT_FAILURE);
    }
    if (fseek(fp, 0, SEEK_END) == -1) {
        printf("Fseek error %s\n", path);
        exit(EXIT_FAILURE);
    }
    long filesize = ftell(fp);
    if (filesize == -1) {
        printf("Ftell error %s\n", path);
        exit(EXIT_FAILURE);
    }
    fclose(fp);
    return filesize;
}

void printInfo() {
    printf("---- Device Info ----\n");
    printf("OEM Name: %s\n", bootSector.BS_OEMName);
    char label[8];
    strncpy(label, bootSector.BS_VolLab, (sizeof label) - 1);
    printf("Label: %s\n", label);

    char filesystype[6];
    strncpy(filesystype, bootSector.BS_FilSysType, (sizeof filesystype) - 1);
    printf("File System Type: %s\n", filesystype);
    printf("Media Type: %X\n", bootSector.BPB_Media);

    // Print file size in bytes, MB and GB.
    long fsz = getSizeFseek(filepath);
    printf("Size: %lu bytes (%gMB, %gGB)\n", fsz,
           (double)fsz/1000000, (double)fsz/1000000000);

    printf("Driver Number: %d\n", bootSector.BS_DrvNum);
    printf("\n---- Geometry ----\n");
    printf("Bytes per Sector: %hu\n", bootSector.BPB_BytesPerSec);
    printf("Sector per Cluster: %hhu\n", bootSector.BPB_SecPerClus);
    printf("Total Sectors: %d\n", bootSector.BPB_TotSec32);
    printf("Geom: Sectors per Track: %d\n", bootSector.BPB_SecPerTrk);
    printf("Geom: Heads: %d\n", bootSector.BPB_NumHeads);
    printf("Hidden Sectors: %d\n", bootSector.BPB_HiddSec);
    printf("\n---- FS Info ----\n");
    printf("Volume ID: %s\n", vol_ID);
    printf("Version: %d:%d\n", bootSector.BPB_FSVerLow, bootSector.BPB_FSVerHigh);
    printf("Reserved Sectors: %d\n", bootSector.BPB_RsvdSecCnt);
    printf("Number of FATs: %d\n", bootSector.BPB_NumFATs);
    printf("FAT Size: %d\n", bootSector.BPB_FATSz32);
    isFATMirrored();
    if (fatmirrored == 0)
        printf("Mirrored FAT: %u (yes)\n", fatmirrored);
    else
        printf("Mirrored FAT: %u (no)\n", fatmirrored);
    printf("Boot Sector Backup Sector No: %d\n", bootSector.BPB_BkBootSec);
}

// Print the current directory
// Implemented as per white paper
void doDir() {
    printf("\nDIRECTORY LISTING");
    printf("\nVOL_ID: %s\n\n", vol_ID);
    setCurrentDir(curDirClus);
    int i = 0;
    while (i <= bs->BPB_BytesPerSec * 8) {
        fat32Dir *dir = (fat32Dir*)&buf[i];
        if (dir->DIR_Name[0] == 0x00)
            break;
        if ((dir->DIR_Attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == ATTR_DIRECTORY)
            // Found a directory
            printf("<%s>\t%d\n", removeWhiteSpace(dir->DIR_Name), dir->DIR_FileSize);
        else if ((dir->DIR_Attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == 0x00)
            // Found a file
            printf("%s\t\t%d\n", getFileName(dir->DIR_Name), dir->DIR_FileSize);
        i += sizeof(fat32Dir);
    }
    printf("---DONE\n");
}

// Change directory to folder path if folder exists
void doCd(char* folderPath) {
    uint64_t newDirClusterNumNumber = -1;
    setCurrentDir(curDirClus);
    int i = 0;
    while (i < bs->BPB_BytesPerSec * 8) {
        fat32Dir *dir = (fat32Dir*)&buf[i];
        if (dir->DIR_Name[0] == 0x00)
            break;
        if ((dir->DIR_Attr & ATTR_DIRECTORY) == ATTR_DIRECTORY &&
                strcmp(folderPath, removeWhiteSpace(dir->DIR_Name)) == 0) {
            newDirClusterNumNumber = dir->DIR_FstClusLo;
            break;
        }
        i += sizeof(fat32Dir);
    }
    if (newDirClusterNumNumber != -1) {
        setCurrentDir(newDirClusterNumNumber);
        doDir();
    } else printf("Error: Folder doesn't exist.\n");
}

// Download file
// Reads file from disk image and writes to current working dir
void doDownload(char* filename) {
    uint64_t originalDirClus = curDirClus;
    uint64_t fileClusterNum = -1;
    setCurrentDir(curDirClus);
    int i = 0;
    while (i < (bs->BPB_BytesPerSec * 8)) {
        fat32Dir *pDir = (fat32Dir*)&buf[i];
        if (pDir->DIR_Name[0] == 0x00)
            break;
        if ((pDir->DIR_Attr & ATTR_DIRECTORY) != ATTR_DIRECTORY
                && strcmp(filename, getFileName(pDir->DIR_Name)) == 0) {
            fileClusterNum = (pDir->DIR_FstClusHi) << 16 | pDir->DIR_FstClusLo;
            break;
        }
        i += sizeof(fat32Dir);
    }
    if (fileClusterNum != -1) {
        setCurrentDir(fileClusterNum);
        FILE* output = fopen(getWriteLoc(filename), "w");
        readSectorNum(getSectorNum(curDirClus, bs));
        fwrite(buf, 1, bs->BPB_BytesPerSec * 8, output);
        uint32_t nextDirClus = getNextClusterNum(curDirClus, bs);
        while (nextDirClus != 0) {
            curDirClus = nextDirClus;
            readSectorNum(getSectorNum(curDirClus, bs));
            fwrite(buf, 1, bs->BPB_BytesPerSec * 8, output);
            nextDirClus = getNextClusterNum(curDirClus, bs);
        }
        curDirClus = originalDirClus;
        setCurrentDir(originalDirClus);
        printf("\nDone\n");
    }
    else printf("Error: File doesn't exist.\n");
}

// Returns a filename in a standard/expected format
// Returned format: "FILENAME.FILETYPE"
// EXAMPLE: A file named 1 with the file type JPG will be returned as 1.JPG
char* getFileName(const char* file) {
    char *filename = malloc(sizeof(char *) * 256);
    int i, spaceAdded = 0, pos = 0;
    for (i = 0; i < strlen(file) - 1; i++) {
        if (!isspace(file[i]))
            filename[pos++] = file[i];
        else if (!spaceAdded) {
            filename[pos++] = '.';
            spaceAdded = 1;
        }
    }
    return filename;
}

// Returns an identical string with the spaces removed
char* removeWhiteSpace(const char* str) {
    char *str_no_spaces = malloc(sizeof(char *) * 256);
    int i, pos = 0;
    for (i = 0; i < strlen(str) - 1; i++)
        if (!isspace(str[i]))
            str_no_spaces[pos++] = str[i];
    return str_no_spaces;
}

// Separate a string into tokens by needle
// Store tokens in dest
void tokenize(char** dest, char* src, char* needle) {
    char buffer[BUF_SIZE];
    int len = (int)strlen(buffer);
    if (src[len - 1] == '\n')
        src[len - 1] = '\0';
    int i = 0;
    char *token = strtok(src, needle);
    while (token) {
        dest[i++] = token;
        token = strtok(NULL, needle);
    }
    dest[i] = NULL;
}

int readSectorNum(uint64_t sectorNum) {
    size_t n;
    int ret = 0;
    buf = malloc(BUF_SIZE);
    FILE *fp = fopen(filepath, "r");
    fseek(fp, (long)sectorNum * bs->BPB_BytesPerSec, SEEK_SET);
    n = fread(buf, 1, BUF_SIZE, fp);
    if (n != BUFFER_SIZE)
        ret = 1;
    fclose(fp);
    return ret;
}

uint64_t getSectorNum(uint64_t clusterNum, fat32BS *b_s) {
    uint64_t FistDataSector = b_s->BPB_RsvdSecCnt + (b_s->BPB_NumFATs * b_s->BPB_FATSz32);
    return ((clusterNum - 2) * b_s->BPB_SecPerClus) + FistDataSector;
}

void setCurrentDir(uint64_t newDirClusterNum) {
    if (newDirClusterNum == 0 || newDirClusterNum == 1)
        newDirClusterNum = 2;
    curDirClus = newDirClusterNum;
    readSectorNum(getSectorNum(curDirClus, bs));
}

uint32_t getNextClusterNum(uint32_t clusterNumber, fat32BS *boot_sector) {
    uint32_t result = 0;
    readSectorNum(boot_sector->BPB_RsvdSecCnt + (clusterNumber * 4 / boot_sector->BPB_BytesPerSec));
    fatEntry* temp = (fatEntry*)&(buf[clusterNumber * 4 % boot_sector->BPB_BytesPerSec]);
    // If we haven't reached the end of cluster...
    if (!(temp->value[0] == 0xFF && temp->value[1] == 0xFF
            && temp->value[2] == 0xFF && temp->value[3] == 0x0F)) {
        memcpy(&result, &temp->value, 4);
    }
    return result;
}

void doPut(char* filename) {
    // To do
    // Bonus stuff
}

char* getWriteLoc(char* filename) {
    char* path = malloc(sizeof(char*) * 8);
    path[0] = '/';
    strcat(path, filename);
    char* writeLoc = getcwd(0, 0);
    strcat(writeLoc, path);
    return writeLoc;
}

void isFATMirrored() {
    uint16_t result = bootSector.BPB_ExtFlags & 64;
    fatmirrored = (result & 64) == result ? 0 : result;
}