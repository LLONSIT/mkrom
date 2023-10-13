#include <stdio.h>
#include <stdlib.h>
#include "mkrom_vars.h"
#include "mkrom_macros.h"
#include "makerom.h"

struct filehdr {
	unsigned short	f_magic;	/* magic number */
	unsigned short	f_nscns;	/* number of sections */
	long		f_timdat;	/* time & date stamp */
	long		f_symptr;	/* file pointer to symbolic header */
	long		f_nsyms;	/* sizeof(symbolic hdr) */
	unsigned short	f_opthdr;	/* sizeof(optional hdr) */
	unsigned short	f_flags;	/* flags */
	};


struct	ldfile {
	int		_fnum_;	/* so each instance of an LDFILE is unique */
	FILE	*	ioptr;	/* system I/O pointer value */
	long		offset;	/* absolute offset to the start of the file */
	struct filehdr		header;	/* the file header of the opened file */
// #ifdef __mips
//	pCHDRR		pchdr;  /* pointer to the symbol table */ # We don't need this '
	long		lastindex; /* index of last symbol accessed */
	unsigned short	type;	/* indicator of the type of the file */
	unsigned	fswap : 1;	/* if set, we must swap */
	unsigned	fBigendian : 1;	/* if set, we must swap aux for the
					 * last retrieved symbol
					 */
// #else
// 	unsigned short	type;		/* indicator of the type of the file */
// #endif /* __mips */
};

struct scnhdr {
	char		s_name[8];	/* section name */
	long		s_paddr;	/* physical address, aliased s_nlib */
	long		s_vaddr;	/* virtual address */
	long		s_size;		/* section size */
	long		s_scnptr;	/* file ptr to raw data for section */
	long		s_relptr;	/* file ptr to relocation */
	long		s_lnnoptr;	/* file ptr to gp histogram */
	unsigned short	s_nreloc;	/* number of relocation entries */
	unsigned short	s_nlnno;	/* number of gp histogram entries */
	long		s_flags;	/* flags */
	};

extern u32 Address;
extern u32 Data0;
extern u32 Data1;
extern struct ldfile* LDPtr;
extern struct scnhdr SHeader;
extern char* SName;
extern s32 Swap;


s32 readCoff(unsigned char *fname, unsigned int *buf) {
     int textSize;
     int dataSize;
     int bssSize;

    OFileName = fname;

    SName = ".text";

  //  textSize = func_0041080C(buf);

    if (textSize < 0) {
        return -1;
    }
    return textSize;
}

// s32 Extract(unsigned int* buf) {
//
//         int bytesRead; //UNUSED
//
//     // LDPtr = ldopen(OFileName, NULL);
//     if ((LDPtr = ldopen(OFileName, NULL)) == NULL) {
//         fprintf(stderr, "Extract(): Cannot open %s.\n", OFileName);
//         return -1;
//     }
//
//     switch (LDPtr->header.f_magic) {                              /* irregular */
//         case MIPSEBMAGIC:
//         case MIPSEBMAGIC_2:
//         case MIPSEBMAGIC_3:
//             Swap = gethostsex() == LITTLEENDIAN;
//             break;
//
//         case MIPSELMAGIC:
//         case MIPSELMAGIC_2:
//         case MIPSELMAGIC_3:
//             Swap = gethostsex() == BIGENDIAN;
//             break;
//     }
//     if (ldnshread(LDPtr, SName, &SHeader) == 0) {
//
//     } else {
//         ldfseek(LDPtr, SHeader.s_scnptr, 0);
//         for (Address = SHeader.s_paddr; (Address - SHeader.s_paddr) < SHeader.s_size; Address += 8) {
//                 ldfread((char*)&Data0, 1, 4, LDPtr); // Could use a union instead but this seems more likely
//                 if (Swap) {
//                     Data0 = swap_word(Data0);
//                 }
//                 ldfread((char*)&Data1, 1, 4, LDPtr);
//                 if (Swap) {
//                     Data1 = swap_word(Data1);
//                 }
//                 if (Swap) {
//                     buf[0] = Data1;
//                     buf[1] = Data0;
//                 } else {
//                     buf[0] = Data0;
//                     buf[1] = Data1;
//                 }
//                 buf += 2;
//
//         }
// //     }
//     ldclose(LDPtr);
//     return SHeader.s_size;
// }
