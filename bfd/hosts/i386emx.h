/* Intel ix86 running OS/2 or DOS */

#ifndef __I386EMX__H__
#define __I386EMX__H__

#ifndef EMX
#define EMX
#endif

#define HOST_PAGE_SIZE          0x1000
#define HOST_SEGMENT_SIZE       0x10000
#define HOST_TEXT_START_ADDR    0x10000
#define HOST_STACK_END_ADDR     0x80000000

#ifndef TEXT_START_ADDR /* bird: does this need to be BFD wide? */
#define TEXT_START_ADDR		0x10000
#endif

#define NO_CORE_COMMAND

#if 0 /* moved to i386aoutemx.c */

#define DEFAULT_ARCH		bfd_arch_i386

#define TARGET_PAGE_SIZE	0x1000
#define TARGET_SEGMENT_SIZE	0x10000
#define TARGET_STACK_END_ADDR	0x80000000

#define ZMAGIC_DISK_BLOCK_SIZE	1024
#define BYTES_IN_WORD		4

#define MY(OP)			CONCAT2(i386aout_emx_,OP)
#define TARGETNAME		"a.out-emx"
#define NO_WRITE_HEADER_KLUDGE	1

#define N_HEADER_IN_TEXT(x) 0

#define N_TXTOFF(x)	\
    (N_MAGIC(x) != ZMAGIC ? EXEC_BYTES_SIZE : /* object file or NMAGIC */\
     N_SHARED_LIB(x) ? 0 : \
     N_HEADER_IN_TEXT(x) ?	\
	    EXEC_BYTES_SIZE :			/* no padding */\
            0x400 + (x).a_hdrofs \
    )

#define N_DATOFF(x) (N_TXTOFF(x) + N_TXTSIZE(x))

#endif /* moved to i386aoutemx.c */

/* a.out executables emx-specific header */
struct exec                     /* bird: should this be BFD wide or only for the emx target? */
{
  unsigned long a_info;		/* Use macros N_MAGIC, etc for access */
  unsigned a_text;		/* length of text, in bytes */
  unsigned a_data;		/* length of data, in bytes */
  unsigned a_bss;		/* length of uninitialized data area for file, in bytes */
  unsigned a_syms;		/* length of symbol table data in file, in bytes */
  unsigned a_entry;		/* start address */
  unsigned a_trsize;		/* length of relocation info for text, in bytes */
  unsigned a_drsize;		/* length of relocation info for data, in bytes */
};

 /* bird: should this be BFD wide or only for the emx target? */
#define IS_STAB(flags) (((flags) & N_STAB) \
	&& (flags) != (N_IMP1|N_EXT) && (flags) != (N_IMP2|N_EXT) \
        && ((flags) & ~N_EXT) != N_EXP)
#endif /* __I386EMX__H__ */
