#include <stdio.h>
#include <stdlib.h>
#include "makerom.h"
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include "mkrom_vars.h"
#include "mkrom_macros.h"

static unsigned char* func_0040B780(unsigned char* fullpath, unsigned char* postRootSuffix, unsigned char* fname);

int main(int argc, char** argv) {
    int c;
    FILE* f;
    Wave* wave;
    char* cppCmd;
    long cppCmdCount;
    int headerFd;
    int pif2bootFd;
    char* bootFileName = NULL;
    char* headerFileName = NULL;
    char* pif2bootFileName = NULL;
    char* fontFileName = NULL;
    int createRom = 1;
    char CheckerBuf[0x100];
    char NameBuf[0x100];
    char* rootName;
    int quietMode = 0;

    B_1000BA40 = argv[0]; //Program name

    if ((cppCmdCount = sysconf(1)) == -1) {
        fprintf(stderr, "makerom: sysconf(_SC_ARG_MAX): %s\n", sys_errlist[errno]);
        exit(1);
    }

    if ((cppCmd = malloc(cppCmdCount)) == NULL) {
        fprintf(stderr, "malloc failed\n");
        return -1;
    }

    sprintf(cppCmd, "/usr/lib/cpp -D_LANGUAGE_MAKEROM");
    cppCmdCount -= strlen(cppCmd) + 1;

    // Parse options
    while ((c = getopt(argc, argv, "D:I:U:cdeimnor:gb:h:p:s:f:O:C:QqVv")) != -1) {
        switch (c) {
            case 'D':
            case 'I':
            case 'U':
                cppCmdCount -= strlen(optarg) + 3;
                if (cppCmdCount <= 0) {
                    fprintf(stderr, "makerom: too many -[DIU] flags\n");
                    exit(1);
                }
                sprintf(cppCmd, "%s -%c%s", cppCmd, c, optarg);
                continue;

            case 'c':
                cosim = 1;
                continue;

            case 'd':
                debug = 1;
                continue;

            case 'g':
                gcord = 1;
                continue;

            case 'k':
                keep_going = 1;
                continue;

            case 'e':
                emulator = 1;
                continue;

            case 'm':
                loadMap = 1;
                continue;

            case 'n':
                nofont = 1;
                continue;

            case 'o':
                D_10014204 = 0;
                continue;

            case 'r':
                D_10014200 = optarg;
                continue;

            case 'b':
                bootFileName = optarg;
                continue;

            case 'h':
                headerFileName = optarg;
                continue;

            case 'p':
                pif2bootFileName = optarg;
                continue;

            case 's':
                finalromSize = strtol(optarg, 0, 0);
                continue;

            case 'f':
                fillData = strtol(optarg, 0, 0);
                continue;

            case 'O':
                offset = strtol(optarg, 0, 0);
                continue;

            case 'C':
                changeclock = 1;
                clockrate = strtol(optarg, 0, 0);
                if (clockrate == 0) {
                    clockrate = 60850000;
                }
                continue;

            case '?':
                func_0040A700();
                exit(1);

            case 'Q':
            case 'q':
                quietMode = 1;
                continue;

            case 'V':
            case 'v':
                func_0040ABA0();
                exit(1);
        }
    }

    // Check options
    if ((argc - optind) != 1) {
        func_0040A700();
        exit(1);
    }

    if ((cosim + emulator) > 1) {
        // "-i"?
        fprintf(stderr, "makerom: only specify one of -c, -e, or -i\n");
        exit(1);
    }

    func_0040AB44();
    if (!quietMode) {
        func_0040ABA0();
    }

    func_0040AC0C(bootFileName);
    func_0040AE34(pif2bootFileName);
    func_0040B05C(headerFileName);
    getFontDataFile(fontFileName);

    if ((unlink(D_10014200) == -1) && (errno != 2)) {
        fprintf(stderr, "makerom: %s: %s\n", D_10014200, sys_errlist[errno]);
        exit(1);
    }

    fileName = argv[optind];
    if ((yyin = fopen(fileName, "r")) == 0) {
        fprintf(stderr, "makerom: %s: %s\n", fileName, sys_errlist[errno]);
        exit(1);
    }
    fclose(yyin);

    cppCmdCount -= strlen(fileName);
    if (cppCmdCount <= 0) {
        fprintf(stderr, "makerom: cpp command line too long\n");
        exit(1);
    }

    sprintf(cppCmd, "%s %s", cppCmd, fileName);
    if ((yyin = popen(cppCmd, "r")) == 0) {
        fprintf(stderr, "makerom: could not run cpp on %s: %s\n", fileName, sys_errlist[errno]);
        exit(1);
    }

    /*
    if (yyparse() != 0) {
        exit(1);
    }
*/
    if (pclose(yyin) != 0) {
        exit(1);
    }

    if (scanSegments() == -1) {
        exit(1);
    }

    if (checkSizes() != 0) {
        createRom = 0;
    }

    if (D_10014204 && checkOverlaps()) {
        createRom = 0;
    }

    func_0040BA54();
    sigaction(1, &D_100141E0, NULL);
    sigaction(2, &D_100141E0, NULL);
    sigaction(15, &D_100141E0, NULL);

    if (debug) {
        printf("Creating segment symbol source file in %s\n", B_10016520);
    }

    if (createSegmentSymbols(B_10016520, B_10016620) == -1) {
        func_0040BC54();
        exit(1);
    }

    for (wave = waveList; wave != NULL; wave = wave->next) {
        func_0040B93C(wave);
    }

    if ((rootName = getenv("ROOT")) == NULL) {
        rootName = "/";
    }

    if (irixVersion > IRIX_VERSION_53) {
        if (func_0040A9AC(rootName) < 2) {
            fprintf(stderr, "makerom: This IDO version is not compatible with the\n");
            fprintf(stderr, "         Nintendo64 development environment on this\n");
            fprintf(stderr, "         version of IRIX.\n");
            exit(1);
        }
        sprintf(CheckerBuf, "%s/usr/sbin/u64check -fmulmul:check:noforce:norepair", rootName);
    } else {
        sprintf(CheckerBuf, "%s/usr/sbin/r4300_check", rootName);
    }

    if (debug) {
        printf("Checking fmulmul status\n");
    }

    for (wave = waveList; wave != NULL; wave = wave->next) {
        sprintf(NameBuf, "%s %s", CheckerBuf, wave->name);
        if (debug) {
            printf("  %s\n", NameBuf);
        }
        if ((execCommand(NameBuf) == -1) && !keep_going) {
            func_0040BC54();
            exit(1);
        }
    }

    if (gcord) {
        sprintf(CheckerBuf, "%s/usr/lib/PR/gcord ", rootName);
        for (wave = waveList; wave != NULL; wave = wave->next) {
            char gcordFileBuf[0x100];

            sprintf(NameBuf, "%s %s", CheckerBuf, wave->name);
            if (debug) {
                printf("makerom: %s\n", NameBuf);
            }
            if ((execCommand(NameBuf) == -1) && !keep_going) {
                func_0040BC54();
                exit(1);
            }
            strcat(strcpy(gcordFileBuf, wave->name), ".cord");
            strcpy(wave->name, gcordFileBuf);
        }
    }

    if (debug) {
        printf("Creating entry source file in %s\n", B_10016720);
    }
    if (createEntryFile(B_10016720, B_10016820) == -1) {
        func_0040BC54();
        exit(1);
    }

    if (createRom) {
        if (debug) {
            printf("Extracting sections from ELF wave files");
            printf(" to create ROM image in %s\n", D_10014200);
        }
        if (createRomImage(D_10014200, B_10016820) == -1) {
            func_0040BC54();
            exit(1);
        }
    }
    func_0040BC54();
    if (bootBuf != NULL) {
        free(bootBuf);
    }
    if (headerBuf != NULL) {
        free(headerBuf);
    }
    if (fontBuf != NULL) {
        free(fontBuf);
    }

    exit(createRom ? 0 : 1);
    return 0;
}

//Usage
void func_0040A700(void) {
    fprintf(stderr, "usage: makerom [-D<define>] [-I<dir>] -[U<define>]\n");
    fprintf(stderr, "                [-c] [-n] [-d] [-g] [-e] [-m] [-q] [-v]\n");
    fprintf(stderr, "                [-o] [-b bootfile] [-h headerfile]\n");
    fprintf(stderr, "                [-p pif2bootfile]\n");
    fprintf(stderr, "                [-s romsize (Mbits)]\n");
    fprintf(stderr, "                [-f filldata (0x00 - 0xff)]\n");
    fprintf(stderr, "                [-t type]\n");
    fprintf(stderr, "                [-C clockrate] \n");
    fprintf(stderr, "                [-r romfile] specfile\n");
}

//func_0040AB44
void func_0040AB44(void) {
    u8* cmd;
    u8 buf[4096];
    FILE* procPtr;

    cmd = "/sbin/uname -r";

    if ((procPtr = popen(cmd, "r")) != NULL) {
        fgets(&buf, 0x1000, procPtr);
        pclose(procPtr);
        if (strcmp(&buf, "5.3\n") == 0) {
            irixVersion = 0;
        } else if (strcmp(&buf, "6.2\n") == 0) { //strncmp is better
            irixVersion = 1;
        } else if (strcmp(&buf, "6.3\n") == 0) {
            irixVersion = 2;
        } else if (strcmp(&buf, "6.4\n") == 0) {
            irixVersion = 3;
        } else {
            irixVersion = 4;
            fprintf(stderr, "makedisk: Operating system not recognized.  Trying 6.x ...\n");
        }
    } else {
        fprintf(stderr, "makedisk: Unable to find uname command!\n");
        exit(1);
    }
}

//checkIdoVersion
s32 func_0040A9AC(const char* arg0) {
    s32 u64CheckFound;
    s32 v70Found;
    char cmd[0x100];
    char buffer[0x100];
    struct stat statBuffer;
    FILE* procPtr;

    sprintf(buffer, "%s/usr/sbin/u64check", arg0);
    u64CheckFound = (stat(buffer, &statBuffer) != 0) ? 0 : 1;

    //ah
    sprintf(cmd, "/usr/sbin/showprods -D 1 dev"); //IDO installation packages saves a regiter

    if ((procPtr = popen(&cmd, "r")) != 0) {
        fgets(buffer, 0xFF, procPtr);
        fgets(buffer, 0xFF, procPtr);
        fgets(buffer, 0xFF, procPtr);
        fgets(buffer, 0xFF, procPtr);

        pclose(procPtr);

        if (strstr(buffer, "7.0") != NULL) {
            v70Found = 1;
        } else {
            v70Found = 0;
        }
    }
    if (v70Found != 0) {
        fprintf(stderr, "makerom: IDO v7.0 does not work with the Nintendo64\n");
        fprintf(stderr, "         development environment.  Please upgrade to\n");
        fprintf(stderr, "         IDO v7.1.\n");
        exit(1);
    }
    if (u64CheckFound != 0) {
        return 2;
    } else {
        return 0;
    }
}

//PrintVersion
void func_0040ABA0(void) {
    if (irixVersion == IRIX_VERSION_53) {
        printf("Nintendo64 Makerom v2.2 for IRIX.\n");
    } else {
        printf("Nintendo64 Makerom v2.2 -BETA- for IRIX.\n");
    }
}

//getBootFile
void func_0040AC0C(u8* bootFileName) {
    s32 bootFd;
    s8 scratchFileName[255];
    struct stat buf;
    s8 errMessage[255];

    if ((bootFileName == NULL) && (func_0040B780(&scratchFileName, "/usr/lib/PR", "Boot") != 0)) {
        bootFileName = &scratchFileName;
    }
    if (bootFileName != NULL) {

        if ((bootFd = open(bootFileName, 0x800)) < 0) {
            sprintf(&errMessage, "%s: unable to open %s", B_10016A20, bootFileName);
            perror(&errMessage);
            exit(1);
        }
        if (fstat(bootFd, &buf) < 0) {
            sprintf(&errMessage, "%s unable to stat %s", B_10016A20, bootFileName);
            perror(&errMessage);
            close(bootFd);
            exit(1);
        }
        bootBuf = malloc((u32) buf.st_size);
        if (bootBuf == NULL) {
            fprintf(stderr, "%s: unable to malloc buffer to hold %d bytes\n", B_10016A20, buf.st_size);
            close(bootFd);
            exit(1);
        }
        close(bootFd);
        bootWordAlignedByteSize = readCoff((u8* ) bootFileName, bootBuf);
    } else {
        bootBuf = NULL;
    }
}


//getPif2BootFile
void func_0040AE34(char* pif2bootFileName) {
    int pif2bootFd; //Segment
    char scratchFileName[0x100];
    struct stat buf;
    char errMessage[0x100];
                            //Why separate the strings?
    if ((pif2bootFileName == NULL) && (func_0040B780(scratchFileName, "/usr/lib/PR", "pif2Boot") != 0)) {
        pif2bootFileName = scratchFileName;
    }
    if (pif2bootFileName != NULL) {
        if ((pif2bootFd = open(pif2bootFileName, 0x800)) < 0) {
            sprintf(errMessage, "%s: unable to open %s", B_10016A20, pif2bootFileName);
            perror(errMessage);
            exit(1);
        }
        if (fstat(pif2bootFd, &buf) < 0) {
            sprintf(errMessage, "%s unable to stat %s", /*bss*/B_10016A20, pif2bootFileName);
            perror(errMessage);
            close(pif2bootFd);
            exit(1);
        }

        pif2bootBuf = malloc(buf.st_size);
        if (pif2bootBuf == 0) {
            fprintf(stderr, "%s: unable to malloc buffer to hold %d bytes\n", B_10016A20, buf.st_size);
            close(pif2bootFd);
            exit(1);
        }
        close(pif2bootFd);
        pif2bootWordAlignedByteSize = readCoff(pif2bootFileName, pif2bootBuf);
    } else {
        pif2bootBuf = 0;
    }
}

//getRomheaderFile
void func_0040B05C(unsigned char *headerFileName) {
    int headerFd;
    unsigned char scratchFileName[0x100];
    struct stat buf;
    unsigned char errMessage[0x100];
    unsigned char nibbleString[4];
    int nibbleVal;
    int i;
    int readPtr;
    int retval;
    // s32 temp_t8;
    // u8 *temp_t6_2;

    if ((headerFileName == NULL) && (func_0040B780(scratchFileName, "/usr/lib/PR", "romheader") != 0)) {
        headerFileName = scratchFileName;
    }
    if (headerFileName != NULL) {
        if ((headerFd = open(headerFileName, 0x800)) < 0) {
            sprintf(errMessage, "%s unable to open %s", B_10016A20, headerFileName);
            perror(errMessage);
            exit(1);
        }
        if (fstat(headerFd, &buf) < 0) {
            sprintf(errMessage, "%s unable to stat %s", B_10016A20, headerFileName);
            perror(errMessage);
            close(headerFd);
            exit(1);
        }

        headerWordAlignedByteSize = buf.st_size;

        headerBuf = malloc(headerWordAlignedByteSize);

        if (headerBuf == 0) {
            fprintf(stderr, "%s: unable to malloc buffer to hold %d bytes\n", B_10016A20, headerWordAlignedByteSize);
            close(headerFd);
            exit(1);
        }

        nibbleString[1] = '\0';
        for (i = 0, readPtr = 0; readPtr < headerWordAlignedByteSize; i++, readPtr++) {
                retval = read(headerFd, &nibbleString, 1);
                if (retval != 1) {
                    fprintf(stderr, "%s: short read from %s.\n", B_10016A20, headerFileName);
                    free(headerBuf);
                    close(headerFd);
                    exit(1);
                }
                if (nibbleString[0] == 0xA) {
                    if (++readPtr < headerWordAlignedByteSize) {
                        retval = read(headerFd, &nibbleString, 1);
                        if (retval != 1) {
                            fprintf(stderr, "%s: short read from %s.\n", B_10016A20, headerFileName);
                            free(headerBuf);
                            close(headerFd);
                            exit(1);
                        }
                    }
                }
                nibbleVal = strtol(nibbleString, 0, 16);
                if (i % 2) {
                    headerBuf[i >> 1] |= nibbleVal;
                } else {
                    headerBuf[i >> 1] = nibbleVal << 4;
                }
        }

        headerWordAlignedByteSize = (i - 1) >> 1;
        if (headerWordAlignedByteSize & 3) {
            headerWordAlignedByteSize += 4;
            headerWordAlignedByteSize &= ~3;
        }
        close(headerFd);
    } else {
        headerBuf = 0;
    }
}

void getFontDataFile(s8* fontFileName) {
    int fontFd;
    unsigned char scratchFileName[255];
    struct stat buf;
    unsigned char errMessage[255];

    if (func_0040B780(&scratchFileName, "/usr/lib/PR", "font") != 0) {
        fontFileName = &scratchFileName;
    }
    if (fontFileName != NULL) {

        if ((fontFd = open(fontFileName, 0x800)) < 0) {
            sprintf(&errMessage, "%s: unable to open %s", B_10016A20, fontFileName);
            perror(&errMessage);
            exit(1);
        }

        if (fstat(fontFd, &buf) < 0) {
            sprintf(&errMessage, "%s unable to stat %s", B_10016A20, fontFileName);
            perror(&errMessage);
            close(fontFd);
            exit(1);
        }
        fontBuf = malloc((size_t) buf.st_size);
        if (fontBuf == NULL) {
            fprintf(stderr, "%s: unable to malloc buffer to hold %d bytes\n", B_10016A20, buf.st_size);
            close(fontFd);
            exit(1);
        }
        fontdataWordAlignedByteSize = (size_t) buf.st_size;
        if (read(fontFd, fontBuf, fontdataWordAlignedByteSize) != fontdataWordAlignedByteSize) {
            sprintf(&errMessage, "%s unable to read %s", B_10016A20, fontFileName);
            perror(&errMessage);
            close(fontFd);
            exit(1);
        }
        close(fontFd);
    } else {
        fontBuf = NULL;
    }
}

//gloadFindFile
static unsigned char* func_0040B780(unsigned char* fullpath, unsigned char* postRootSuffix, unsigned char* fname) {
    unsigned char* rootname;
    unsigned char* rootpath;
    s32 fd; //UNUSED
    s32 try;

    for (try = 0; try < 3; try++) {
        *fullpath = 0;
        switch (try) {
        case 0:
            rootname = "ROOT";
            break;
        case 1:
            rootname = "WORKAREA";
            break;
        case 2:
            rootname = NULL;
            break;
        }

        if (rootname != NULL) {
            if ((rootpath = getenv(rootname)) == NULL) {
                continue;
            }
            strcat(fullpath, rootpath);
        }
        if (postRootSuffix != NULL) {
            strcat(fullpath, postRootSuffix);
            strcat(fullpath, "/");
        }
        strcat(fullpath, fname);
        if (access(fullpath, 4) == 0) {
            return fullpath;
        }
    }
    fprintf(stderr, "gloadFindFile: can't find file %s\n", fullpath);
    *fullpath = '\0';
    return NULL;
}

//doWave
void func_0040B93C(Wave* wave) {
    if (debug != 0) {
        printf("Translating ROM spec file into");
        printf(" linker spec file in %s\n", wave->elspecFile);
    }
    if (createElspec(wave) == -1) {
        func_0040BC54();
        exit(1);
    }
    if ((runLinker(wave, &B_10016620, &B_10016920) == -1) && (keep_going == 0)) {
        func_0040BC54();
        exit(1);
    }
}

//nameTempFiles
void func_0040BA54(void) {
    Wave* wave;
    unsigned char* tmpdir;

    if ((tmpdir = getenv("TMPDIR")) == NULL) {
        tmpdir = "/tmp";
    }

    for (wave = waveList ; wave != NULL; wave = wave->next) {
        sprintf(wave->elspecFile, "%s/%sElspecXXXXXX", tmpdir, wave->name);
        mktemp(wave->elspecFile);
    }

    sprintf(&B_10016520, "%s/segmentXXXXXX", tmpdir);
    mktemp((u8* ) &B_10016520);
    strcpy(&B_10016620, &B_10016520);
    strcat(&B_10016520, ".s");
    strcat(&B_10016620, ".o");
    sprintf(&B_10016720, "%s/entryXXXXXX", tmpdir);
    mktemp((u8* ) &B_10016720);
    strcpy(&B_10016820, &B_10016720);
    strcat(&B_10016720, ".s");
    strcat(&B_10016820, ".o");
    sprintf(&B_10016920, "%s/objListXXXXXX", tmpdir);
    mktemp((u8* ) &B_10016920);
}

//unlinkTempFiles
void func_0040BC54(void) {
    Wave* wave;

    //Same name temp files for loop
    if (debug == 0) {
        for (wave = waveList  ; wave != NULL; wave=wave->next) {
            unlink(wave->elspecFile);
        }

        unlink(&B_10016520);
        unlink(&B_10016620);
        unlink(&B_10016720);
        unlink(&B_10016820);
        unlink(&B_10016920);
    }
}

//cleanup
void func_0040BD2C(int sig) {
    func_0040BC54();
    exit(1);
}

int execCommand(const char* cmd) {
    int console = system(cmd);

    if (console == -1) {
        fprintf(stderr, "makerom: cannot execute command: %s\n", sys_errlist[errno]);
        return -1;
    } else if (WIFEXITED(console) && WEXITSTATUS(console) == 0) {
        return 0;
    } else if (keep_going && (WIFEXITED(console) && WEXITSTATUS(console) == 1)) {
            return 1;
    } else {
        if (debug) {
            fprintf(stderr, "makerom: [%s] returned %d (%08x), errno=%d\n", cmd, WEXITSTATUS(console), console, errno);
        }
        return -1;
    }
}
