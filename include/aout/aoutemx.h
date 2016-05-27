/* traditional EMX a.out format differences for i386 running OS/2 or DOS */

#define SEGMENT_SIZE 0x10000

#define N_HEADER_IN_TEXT(x) 0

#define	N_TXTSIZE(x) ((x).a_text)
#define N_TXTOFF(x)	\
    (N_MAGIC(x) != ZMAGIC ? EXEC_BYTES_SIZE : /* object file or NMAGIC */\
     0x400 + (x).a_hdrofs \
    )

#define N_DATOFF(x) (N_TXTOFF(x) + N_TXTSIZE(x))

/* Address of text segment in memory after it is loaded.  */
#define N_TXTADDR(x) \
    (N_MAGIC(x) != ZMAGIC ? 0 : SEGMENT_SIZE)
#define N_DATADDR(x) \
    (N_MAGIC(x) == OMAGIC ? \
     (N_TXTADDR (x) + (x).a_text) : \
     ((N_TXTADDR(x) + (x).a_text + SEGMENT_SIZE - 1) & ~(SEGMENT_SIZE - 1)))

/* EMX-specific symbols.  */
#define N_IMP1 0x68		/* Import reference (emx specific) */
#define N_IMP2 0x6a		/* Import definition (emx specific) */
#define N_EXP  0x6c		/* Export definition (emx specific) */
