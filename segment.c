#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <libelf.h>
#include <fcntl.h>

#include "makerom.h"
#include "mkrom_macros.h"


extern s32 debug;
extern unsigned char *B_10016A60; //romImage
extern Segment* segmentList;
 char* sys_errlist; //TEMP!!!!

int scanSegments() {
    Segment* s;
    u32 offset;
    u32 rom_size;
    void* temp_t2;

    offset = 0x50;
    if (elf_version(1) == 0) {
        fprintf(stderr, "makerom: out of date\n");
        return -1;
    }

    for (s = segmentList ; s != NULL ; s = s->next) {
        if (s->w == 0) {
            fprintf(stderr, "makerom: segment \"%s\": not found in any wave\n", s->name);
            return -1;
        }
        s->romOffset = offset;
        if (s->flags & OBJECT) {
            if (func_0040CB84(s) == -1) { //sizeObject
                return -1;
            }
        } else if (s->flags & RAW) {
            if (func_0040DA80(s) == -1) { //sizeRaw
                return -1;
            }
        }

        offset = s->romOffset;
        offset += s->textSize + s->dataSize + s->sdataSize;
        offset = ALIGNn(s->romalign, offset);
    }
    rom_size = (offset > 0x50) ? offset : 0x50;
    B_10016A60 = calloc(rom_size, 1U);
    if (B_10016A60 == NULL) {
        fprintf(stderr, "makerom: malloc failed [RomSize= %d kB]\n", (s32) (rom_size + 0x3FF) / 0x400);
        return -1;
    }
    return 0;
}

// sizeObject
int func_0040CBA4(Segment* segment) {
    u32 address1;
    u32 address2;
    int elfid;
    Elf* cur_elf;
    Elf_Scn* scn;
    Elf32_Ehdr* ehdr;
    Elf32_Shdr* shdr;
    Path* cur_file;
    u32 section_index;                                       /* compiler-managed */
    char* section_name;
    s32 currAddress;
    s32 firstSection;

    firstSection = 1;
    segment->textAlign = 0x10;

    if (debug) {
        if ((segment->align != 0x10) && (segment->align != 0)) {
            printf("Segment %s: alignment %x\n", segment->name, segment->align);
        }
        if ((segment->romalign != 0x10) && (segment->romalign != 0)) {
            printf("Segment %s: romalign %x\n", segment->name, segment->romalign);
        }
    }

    for (cur_file = segment->pathList; cur_file != NULL; cur_file = cur_file->next) {
        cur_file->textSize = 0;
        cur_file->dataSize = 0;
        cur_file->sdataSize = 0;
        cur_file->sbssSize = 0;
        cur_file->bssSize = 0;
        cur_file->textAlign = 0;
        cur_file->dataAlign = 0;
        cur_file->sdataAlign = 0;
        cur_file->sbssAlign = 0;
        cur_file->bssAlign = 0;

        if ((elfid = open(cur_file->name, 0)) == -1) {
            fprintf(stderr, "makerom: %s: %s\n", cur_file->name, sys_errlist[errno]);
            return -1;
        }

        if (debug) {
            printf("Scanning %s\n", cur_file->name);
        }
        cur_elf = elf_begin(elfid, ELF_C_READ, NULL);
        if ((elf_kind(cur_elf) != 3) || ((ehdr = elf32_getehdr(cur_elf)) == NULL)) {
            fprintf(stderr, "makerom: %s: not a valid ELF object file\n", cur_file->name);
            return -1;
        }
            /* OLD _elf_getscn NEW elf_getscn*/
        for (section_index = 1; section_index < ehdr->e_shnum; section_index++) {
            if (((scn = elf_getscn(cur_elf, section_index)) == NULL) || ((shdr = elf32_getshdr(scn)) == NULL)) {
                fprintf(stderr, "makerom: %s: can't get section index %d\n", cur_file->name, section_index);
                return -1;
            }

            section_name = elf_strptr(cur_elf, ehdr->e_shstrndx, shdr->sh_name);
            if (strcmp(section_name, ".text") == 0) {
                segment->textSize += shdr->sh_size;
                cur_file->textAlign = shdr->sh_addralign;
                cur_file->textSize = shdr->sh_size;
                cur_file->sectionsExisting |= SECTION_TEXT;
                segment->sectionsExisting |= SECTION_TEXT;
                if (cur_file->textAlign > segment->textAlign) {
                    segment->textAlign = cur_file->textAlign;
                }
                if (debug) {
                    printf("  text size  = %x\n", shdr->sh_size);
                    printf("       align = %x\n", shdr->sh_addralign);
                }
            } else if ((strcmp(section_name, ".data") == 0) || (strcmp(section_name, ".rodata") == 0)) {
                segment->dataSize += shdr->sh_size;
                cur_file->dataAlign = shdr->sh_addralign;
                cur_file->dataSize += shdr->sh_size;
                cur_file->sectionsExisting |= SECTION_DATA_RODATA;
                segment->sectionsExisting |= SECTION_DATA_RODATA;
                if (cur_file->dataAlign > segment->dataAlign) {
                    segment->dataAlign = cur_file->dataAlign;
                }
                if (debug) {
                    printf("  data&rodata size  = %x\n", shdr->sh_size);
                    printf("       align = %x\n", shdr->sh_addralign);
                }
            } else if (strcmp(section_name, ".sdata") == 0) {
                segment->sdataSize += shdr->sh_size;
                cur_file->sdataAlign = shdr->sh_addralign;
                cur_file->sdataSize = shdr->sh_size;
                segment->sectionsExisting |= SECTION_SDATA;
                cur_file->sectionsExisting |= SECTION_SDATA;
                if (cur_file->sdataAlign > segment->sdataAlign) {
                    segment->sdataAlign = cur_file->sdataAlign;
                }
                if (debug) {
                    printf("  sdata size  = %x\n", shdr->sh_size);
                    printf("        align = %x\n", shdr->sh_addralign);
                }
            } else if (strcmp(section_name, ".sbss") == 0) {
                segment->sbssSize += shdr->sh_size;
                cur_file->sbssAlign = shdr->sh_addralign;
                cur_file->sbssSize = shdr->sh_size;
                cur_file->sectionsExisting |= SECTION_SBSS;
                segment->sectionsExisting |= SECTION_SBSS;
                if (cur_file->sbssAlign > segment->sbssAlign) {
                    segment->sbssAlign = cur_file->sbssAlign;
                }
                if (debug) {
                    printf("  sbss size  = %x\n", shdr->sh_size);
                    printf("       align = %x\n", shdr->sh_addralign);
                }
            } else if (strcmp(section_name, ".bss") == 0) {
                segment->bssSize += shdr->sh_size;
                cur_file->bssAlign = shdr->sh_addralign;
                cur_file->bssSize = shdr->sh_size;
                cur_file->sectionsExisting |= SECTION_BSS;
                segment->sectionsExisting |= SECTION_BSS;
                if (cur_file->bssAlign > segment->bssAlign) {
                    segment->bssAlign = cur_file->bssAlign;
                }
                if (debug) {
                    printf("  bss size  = %x\n", shdr->sh_size);
                    printf("      align = %x\n", shdr->sh_addralign);
                }
            }
        }

        close(elfid);
    }

    switch (segment->addrFunc) {
        case 2:
            address1 = segment->afterSeg1->address + segment->afterSeg1->totalSize;
            address2 = segment->afterSeg2->address + segment->afterSeg2->totalSize;
            currAddress = (address1 > address2) ? address1 : address2;
            break;

        case 3:
            address1 = segment->afterSeg1->address + segment->afterSeg1->totalSize;
            address2 = segment->afterSeg2->address + segment->afterSeg2->totalSize;
            currAddress = (address1 < address2) ? address1 : address2;
            break;

        case 1:
            address1 = segment->afterSeg1->address + segment->afterSeg1->totalSize;
            currAddress = address1;
            break;

        case 4:
            currAddress = segment->address;
            break;

        case 5:
            currAddress = segment->address;
            break;

        default:
            break;
    }

    currAddress = ALIGNn(segment->align, currAddress);
    segment->address = currAddress;
    if (segment->flags & 1) {
        currAddress = ALIGNn(segment->textAlign, currAddress);
        segment->romOffset = ALIGNn(segment->textAlign, segment->romOffset);
        segment->romOffset = ALIGNn(segment->align, segment->romOffset);
    }

    if (segment->sectionsExisting & SECTION_TEXT) {
        currAddress = ALIGNn(segment->textAlign, currAddress);
        segment->textStart = currAddress;
        segment->address = currAddress;
        firstSection = 0;

        for (cur_file = segment->pathList; cur_file != NULL; cur_file = cur_file->next) {
            if (cur_file->sectionsExisting & SECTION_TEXT) {
                currAddress = ALIGNn(cur_file->textAlign, currAddress);
                cur_file->textStart = currAddress;
                currAddress += cur_file->textSize;
            }
        }
    } else {
        segment->textStart = currAddress;
    }
    if (segment->sectionsExisting & SECTION_DATA_RODATA) {
        currAddress = ALIGNn(segment->dataAlign, currAddress);
        segment->dataStart = currAddress;
        if (firstSection) {
            segment->address = currAddress;
            firstSection = 0;
        }
        for (cur_file = segment->pathList; cur_file != NULL; cur_file = cur_file->next) {
            if (cur_file->sectionsExisting & SECTION_DATA_RODATA) {
                currAddress = ALIGNn(cur_file->dataAlign, currAddress);
                cur_file->dataStart = currAddress;
                currAddress += cur_file->dataSize;
            }
        }
    } else {
        segment->dataStart = currAddress;
    }
    if (segment->sectionsExisting & SECTION_SDATA) {
        currAddress = ALIGNn(segment->sdataAlign, currAddress);
        segment->sdataStart = currAddress;
        if (firstSection) {
            segment->address = currAddress;
            firstSection = 0;
        }
        for (cur_file = segment->pathList; cur_file != NULL; cur_file = cur_file->next) {
            if (cur_file->sectionsExisting & SECTION_SDATA) {
                currAddress = ALIGNn(cur_file->sdataAlign, currAddress);
                cur_file->sdataStart = currAddress;
                currAddress += cur_file->sdataSize;
            }
        }
    } else {
        segment->sdataStart = currAddress;
    }
    if (segment->sectionsExisting & SECTION_SBSS) {
        currAddress = ALIGNn(segment->sbssAlign, currAddress);
        segment->sbssStart = currAddress;
        if (firstSection) {
            segment->address = currAddress;
            firstSection = 0;
        }
        for (cur_file = segment->pathList; cur_file != NULL; cur_file = cur_file->next) {
            if (cur_file->sectionsExisting & SECTION_SBSS) {
                currAddress = ALIGNn(cur_file->sbssAlign, currAddress);
                cur_file->sbssStart = currAddress;
                currAddress += cur_file->sbssSize;
            }
        }
    } else {
        segment->sbssStart = currAddress;
    }
    if (segment->sectionsExisting & SECTION_BSS) {
        currAddress = ALIGNn(segment->bssAlign, currAddress);
        segment->bssStart = currAddress;
        if (firstSection) {
            segment->address = currAddress;
            firstSection = 0;
        }
        for (cur_file = segment->pathList; cur_file != NULL; cur_file = cur_file->next) {
            if (cur_file->sectionsExisting & SECTION_BSS) {
                currAddress = ALIGNn(cur_file->bssAlign, currAddress);
                cur_file->bssStart = currAddress;
                currAddress += cur_file->bssSize;
            }
        }
    } else {
        segment->bssStart = currAddress;
    }

    segment->textSize = segment->dataStart - segment->address;
    segment->dataSize = segment->sdataStart - segment->dataStart;
    segment->sdataSize = segment->sbssStart - segment->sdataStart;
    segment->sbssSize = segment->bssStart - segment->sbssStart;
    segment->bssSize = currAddress - segment->bssStart;
    segment->totalSize = currAddress - segment->address;
    return 0;
}


int sizeRaw(Segment* segment) {
    u32 address1;
    u32 address2;
    u32 currAddress;
    s32 fd;
    Path* cur_file;
    struct stat statBuffer;


    segment->dataAlign = 0x10;
    segment->sectionsExisting = 2;


    for(cur_file = segment->pathList ; cur_file != NULL ; cur_file = cur_file->next) {
        cur_file->textSize = 0;
        cur_file->dataSize = 0;
        cur_file->sdataSize = 0;
        cur_file->sbssSize = 0;
        cur_file->bssSize = 0;
        cur_file->textAlign = 0;
        cur_file->dataAlign = 0x10;
        cur_file->sdataAlign = 0;
        cur_file->sbssAlign = 0;
        cur_file->bssAlign = 0;
        cur_file->sectionsExisting = 2;

        if ((fd = open(cur_file->name, 0)) == -1) {
            fprintf(stderr, "makerom: %s: %s\n", cur_file->name, sys_errlist[errno]);
            return -1;
        }
        if (fstat(fd, &statBuffer) == -1) {
            fprintf(stderr, "makerom: lstat failed: %s\n", sys_errlist[errno]);
            return -1;
        }
        segment->dataSize = (s32) (segment->dataSize + statBuffer.st_size);
        close(fd);
    }

    segment->totalSize = segment->dataSize =  ((u32) (segment->dataSize + 0xF) >> 4) * 0x10;

    switch (segment->addrFunc) {
    case 2:
        address1 = segment->afterSeg1->address + segment->afterSeg1->totalSize;
        address2 = segment->afterSeg2->address + segment->afterSeg2->totalSize;
        currAddress = (address1 > address2) ? address1 : address2;
        break;
    case 3:
        address1 = segment->afterSeg1->address + segment->afterSeg1->totalSize;
        address2 = segment->afterSeg2->address + segment->afterSeg2->totalSize;
        currAddress = (address1 < address2) ? address1 : address2;
        break;
    case 1:
        address1 = segment->afterSeg1->address + segment->afterSeg1->totalSize;
        currAddress = address1;
        break;
    case 4:
        currAddress = segment->address;
        break;
    case 5:
        currAddress = segment->address;
        break;
    default:
        break;
    }

    currAddress = ALIGNn(segment->align, currAddress);
    currAddress = ALIGNn(segment->dataAlign, currAddress);
    segment->address = currAddress;
    return 0;
}
