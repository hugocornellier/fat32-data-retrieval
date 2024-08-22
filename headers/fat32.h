#ifndef FAT32_H
#define FAT32_H

#include <inttypes.h>
#include <stdio.h>

#define BUF_SIZE 8184
#define BUFFER_SIZE 512
#define BS_OEMName_LENGTH 8
#define BS_VolLab_LENGTH 11
#define BS_FilSysType_LENGTH 8
#define ATTR_DIRECTORY 0x10
#define ATTR_VOLUME_ID 0x08
#define ATTR_HIDDEN 0x02

#pragma pack(push)
#pragma pack(1)
struct fat32BS_struct {
    char BS_jmpBoot[3];
    char BS_OEMName[BS_OEMName_LENGTH];
    uint16_t BPB_BytesPerSec;
    uint8_t BPB_SecPerClus;
    uint16_t BPB_RsvdSecCnt;
    uint8_t BPB_NumFATs;
    uint16_t BPB_RootEntCnt;
    uint16_t BPB_TotSec16;
    uint8_t BPB_Media;
    uint16_t BPB_FATSz16;
    uint16_t BPB_SecPerTrk;
    uint16_t BPB_NumHeads;
    uint32_t BPB_HiddSec;
    uint32_t BPB_TotSec32;
    uint32_t BPB_FATSz32;
    uint16_t BPB_ExtFlags;
    uint8_t BPB_FSVerLow;
    uint8_t BPB_FSVerHigh;
    uint32_t BPB_RootClus;
    uint16_t BPB_FSInfo;
    uint16_t BPB_BkBootSec;
    char BPB_reserved[12];
    uint8_t BS_DrvNum;
    uint8_t BS_Reserved1;
    uint8_t BS_BootSig;
    uint32_t BS_VolID;
    char BS_VolLab[BS_VolLab_LENGTH];
    char BS_FilSysType[BS_FilSysType_LENGTH];
    char BS_CodeReserved[420];
    uint8_t BS_SigA;
    uint8_t BS_SigB;
};
#pragma pack(pop)
typedef struct fat32BS_struct fat32BS;

#pragma pack(push)
#pragma pack(1)
struct Fat32Dir {
    char DIR_Name[11];
    uint8_t DIR_Attr;
    uint8_t DIR_NTRes;
    uint8_t DIR_CrtTimeTenth;
    uint16_t DIR_CrtTime;
    uint16_t DIR_CrtDate;
    uint16_t DIR_LstAccDate;
    uint16_t DIR_FstClusHi;
    uint16_t DIR_WrtTime;
    uint16_t DIR_WrtDate;
    uint16_t DIR_FstClusLo;
    uint32_t DIR_FileSize;

};
#pragma pack(pop)
typedef struct Fat32Dir fat32Dir;

#pragma pack(push)
#pragma pack(1)
struct fatEntry_struct {
    unsigned char value[4];
};
#pragma pack(pop)
typedef struct fatEntry_struct fatEntry;

void tokenize(char** dest, char* src, char* needle);
void printInfo();
void doDownload(char* filename);
char* getWriteLoc(char* filename);
void doCd(char* folderPath);
void doDir();
uint64_t getSectorNum(uint64_t clusterNum, fat32BS *b_s);
int readSectorNum(uint64_t sectorNum);
void setCurrentDir(uint64_t newDirClusterNum);
uint32_t getNextClusterNum(uint32_t clusterNumber, fat32BS *boot_sector);
char* getFileName(const char* file);
char* removeWhiteSpace(const char* str);
void doPut(char* filename);
void isFATMirrored();

fat32BS bootSector;
fat32BS* bs;
unsigned char *buf;
unsigned char *bootbuffer;
uint64_t curDirClus;
char *vol_ID;
char *filepath;
int fatmirrored;
FILE* fp;

#endif
