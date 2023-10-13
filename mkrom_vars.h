#ifndef MKROM_VARS_H
#define MKROM_VARS_H
#include <stdio.h>
#include <libelf.h>
#include <unistd.h>
#include <signal.h>
#include "makerom.h"
#include "mkrom_types.h"

static char* OFileName;
static char* SName;
static s8 B_10016920;
static char* B_10016A20;
static s32 pif2bootBuf;
static s32 pif2bootWordAlignedByteSize;
static int D_10014204;
static const char *fileName;
static const struct sigaction *D_100141E0;
static unsigned char B_10016720[255]; // entrySource
static unsigned char B_10016820[255]; // entryObject
static unsigned char B_10016520[255]; // segmentSymbolSource
static unsigned char B_10016620[255]; // segmentSymbolObject
static unsigned char* D_10014200;
static int D_10014204;
static int loadMap;
static int emulator;
static char *B_1000BA40;
static int irixVersion;
static s8* bootEntryName;
static s8* bootStackName;
static s32 bootStackOffset;
static s32 cosim;
static s32 finalromSize;
static int gcord;
static void* bootAddress;
static s32 changeclock;
static s32 clockrate;
static u8 fillData;
static s32 finalromSize;
static s32* fontBuf; //maybe?
static s32 nofont;
static s32 offset;
static Segment* segmentList;
static s32 debug;
static s32 offset;
static unsigned char *B_10016A60; //romImage
static Segment* segmentList;
static Wave* waveList;
static const char* sys_errlist; //TEMP!!!!
static char* headerBuf;
static int headerWordAlignedByteSize;
static size_t fontdataWordAlignedByteSize;
static int bootWordAlignedByteSize;
static size_t* bootBuf;
static int keep_going;
static FILE* yyin;

#endif /* MKROM_VARS_H */
