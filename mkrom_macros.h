#ifndef MKROM_MACROS_H
#define MKROM_MACROS_H

//Irix Version Macros
#define IRIX_VERSION_53 0
#define IRIX_VERSION_62 1
#define IRIX_VERSION_63 2
#define IRIX_VERSION_64 3
#define IRIX_VERSION_UNKNOWN 4 //LINUX?

#define OBJECT 2
#define RAW 4

#define SECTION_TEXT (1 << 0)
#define SECTION_DATA_RODATA (1 << 1)
#define SECTION_SDATA (1 << 2)
#define SECTION_BSS (1 << 3)
#define SECTION_SBSS (1 << 4)

#define TO_PHYSICAL(addr) ((u32)(addr) & 0x1FFFFFFF)

//Magic constants
#define  MIPSEBMAGIC	0x0160
#define  MIPSELMAGIC	0x0162
#define  SMIPSEBMAGIC	0x6001
#define  SMIPSELMAGIC	0x6201
#define  MIPSEBUMAGIC	0x0180
#define  MIPSELUMAGIC	0x0182

#define  MIPSEBMAGIC_2	0x0163
#define  MIPSELMAGIC_2	0x0166
#define  SMIPSEBMAGIC_2	0x6301
#define  SMIPSELMAGIC_2	0x6601


#define  MIPSEBMAGIC_3	0x0140
#define  MIPSELMAGIC_3	0x0142
#define  SMIPSEBMAGIC_3	0x4001
#define  SMIPSELMAGIC_3	0x4201


/*
 * Byte sex constants
 */
#define BIGENDIAN	0
#define LITTLEENDIAN	1
#define UNKNOWNENDIAN	2


#endif /* MKROM_MACROS_H */
