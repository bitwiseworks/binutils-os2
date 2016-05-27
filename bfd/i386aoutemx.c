/* BFD back-end for emx a.out binaries, derived from i386aout.c and aout-target.h
   Copyright 1990, 1991, 1992 Free Software Foundation, Inc.

This file is part of BFD, the Binary File Descriptor library.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#include "sysdep.h"
#ifdef TRAD_HEADER
#include TRAD_HEADER
#endif
#include "sysdep.h"
#include "bfd.h"

#ifndef EMX
#error "EMX TARGET! EMX isn't defined!!!"
#endif


/* Avoid multiple defininitions from aoutx if supporting standarad a.out
   as well as our own.  */
/* Do not "beautify" the CONCAT* macro args.  Traditional C will not
   remove whitespace added here, and thus will fail to concatenate
   the tokens.  */
#define NAME(x,y)               CONCAT3(i386aout_emx,_32_,y)

/* previously in hosts/i386emx.h */
#define DEFAULT_ARCH		bfd_arch_i386

#define MY(OP)			CONCAT2(i386aout_emx_,OP)
#define TARGETNAME		"a.out-emx"

#define TARGET_PAGE_SIZE	0x1000
#define TARGET_SEGMENT_SIZE	0x10000
#define TEXT_START_ADDR		0x10000
#define TARGET_STACK_END_ADDR	0x80000000

#define HOST_PAGE_SIZE          0x1000
#define HOST_SEGMENT_SIZE       0x10000
#define HOST_TEXT_START_ADDR    0x10000
#define HOST_STACK_END_ADDR     0x80000000


#define ZMAGIC_DISK_BLOCK_SIZE	1024
#define BYTES_IN_WORD		4
#define NO_CORE_COMMAND

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

#define IS_STAB(flags) (((flags) & N_STAB) \
	&& (flags) != (N_IMP1|N_EXT) && (flags) != (N_IMP2|N_EXT) \
        && ((flags) & ~N_EXT) != N_EXP)

/* end of old hosts/i386emx.h */


#include <symcat.h>
#define MY_object_p                 MY(object_p)
#define MY_backend_data            &MY(backend_data)
#define MY_bfd_reloc_type_lookup    i386aout_emx_reloc_type_lookup

#include "libaout.h"
#include "aout/aout64.h"

static bfd_boolean      MY(set_sizes) PARAMS ((bfd *));
const bfd_target *      MY(object_p) PARAMS ((bfd *));
reloc_howto_type *      MY(reloc_type_lookup) PARAMS ((bfd *, bfd_reloc_code_real_type));
bfd_reloc_status_type   MY(generic_reloc) PARAMS ((bfd *, arelent *, asymbol *, PTR, asection *, bfd *, char **));
/*reloc_howto_type *  MY(reloc_howto) PARAMS ((bfd *, struct reloc_std_external *, int *, int *, int *));*/

reloc_howto_type MY(howto_table)[] =
{
  /* type              rs size bsz  pcrel bitpos ovrf                     sf                name      part_inpl readmask  setmask    pcdone.  */
HOWTO ( 0,             0,  0,   8,  FALSE, 0, complain_overflow_bitfield, MY(generic_reloc),"8",        TRUE, 0x000000ff,0x000000ff, FALSE),
HOWTO ( 1,             0,  1,   16, FALSE, 0, complain_overflow_bitfield, MY(generic_reloc),"16",       TRUE, 0x0000ffff,0x0000ffff, FALSE),
HOWTO ( 2,             0,  2,   32, FALSE, 0, complain_overflow_bitfield, MY(generic_reloc),"32",       TRUE, 0xffffffff,0xffffffff, FALSE),
HOWTO ( 3,             0,  4,   64, FALSE, 0, complain_overflow_bitfield, MY(generic_reloc),"64",       TRUE, 0xdeaddead,0xdeaddead, FALSE),
HOWTO ( 4,             0,  0,   8,  TRUE,  0, complain_overflow_signed,   MY(generic_reloc),"DISP8",    TRUE, 0x000000ff,0x000000ff, FALSE),
HOWTO ( 5,             0,  1,   16, TRUE,  0, complain_overflow_signed,   MY(generic_reloc),"DISP16",   TRUE, 0x0000ffff,0x0000ffff, FALSE),
HOWTO ( 6,             0,  2,   32, TRUE,  0, complain_overflow_signed,   MY(generic_reloc),"DISP32",   TRUE, 0xffffffff,0xffffffff, FALSE),
HOWTO ( 7,             0,  4,   64, TRUE,  0, complain_overflow_signed,   MY(generic_reloc),"DISP64",   TRUE, 0xfeedface,0xfeedface, FALSE),
HOWTO ( 8,             0,  2,    0, FALSE, 0, complain_overflow_bitfield, MY(generic_reloc),"GOT_REL",  FALSE,         0,0x00000000, FALSE),
HOWTO ( 9,             0,  1,   16, FALSE, 0, complain_overflow_bitfield, MY(generic_reloc),"BASE16",   FALSE,0xffffffff,0xffffffff, FALSE),
HOWTO (10,             0,  2,   32, FALSE, 0, complain_overflow_bitfield, MY(generic_reloc),"BASE32",   FALSE,0xffffffff,0xffffffff, FALSE),
EMPTY_HOWTO (-1),
EMPTY_HOWTO (-1),
EMPTY_HOWTO (-1),
EMPTY_HOWTO (-1),
EMPTY_HOWTO (-1),
  HOWTO (16,	       0,  2,	 0, FALSE, 0, complain_overflow_bitfield, MY(generic_reloc),"JMP_TABLE", FALSE,         0,0x00000000, FALSE),
EMPTY_HOWTO (-1),
EMPTY_HOWTO (-1),
EMPTY_HOWTO (-1),
EMPTY_HOWTO (-1),
EMPTY_HOWTO (-1),
EMPTY_HOWTO (-1),
EMPTY_HOWTO (-1),
EMPTY_HOWTO (-1),
EMPTY_HOWTO (-1),
EMPTY_HOWTO (-1),
EMPTY_HOWTO (-1),
EMPTY_HOWTO (-1),
EMPTY_HOWTO (-1),
EMPTY_HOWTO (-1),
EMPTY_HOWTO (-1),
  HOWTO (32,	       0,  2,	 0, FALSE, 0, complain_overflow_bitfield, MY(generic_reloc),"RELATIVE",  FALSE,         0,0x00000000, FALSE),
EMPTY_HOWTO (-1),
EMPTY_HOWTO (-1),
EMPTY_HOWTO (-1),
EMPTY_HOWTO (-1),
EMPTY_HOWTO (-1),
EMPTY_HOWTO (-1),
EMPTY_HOWTO (-1),
  HOWTO (40,	       0,  2,	 0, FALSE, 0, complain_overflow_bitfield,0,"BASEREL",   FALSE,         0,0x00000000, FALSE),
};


CONST struct aout_backend_data MY(backend_data) = {
  0,				/* zmagic contiguous */
  0,				/* text incl header */
  0,				/* entry is text address */
  0,				/* exec_hdr_flags */
  0,				/* text vma? */
  MY(set_sizes),
  1,				/* exec header not counted */
  0,				/* add_dynamic_symbols */
  0,				/* add_one_symbol */
  0,				/* link_dynamic_object */
  0,				/* write_dynamic_symbol */
  0,				/* check_dynamic_reloc */
  0				/* finish_dynamic_link */
};

#define MY_get_section_contents _bfd_generic_get_section_contents
#include "aoutx.h"

#include "aout-target.h"

#ifndef __EMX__

/* Cross-compilation support, borrowed from EMX C runtime library */
int _fseek_hdr PARAMS ((FILE *));

int _fseek_hdr (FILE *stream)
{
  struct
    {
      char magic[2];
      char fill1[6];
      unsigned short hdr_size;
    } exe_hdr;
  struct
    {
      char sig[16];
      char bound;
      char fill1;
      unsigned short hdr_loc_lo;      /* cannot use long, alignment! */
      unsigned short hdr_loc_hi;
    } patch;
  long original_pos;
  int saved_errno;

  original_pos = ftell (stream);
  if (fread (&exe_hdr, sizeof (exe_hdr), 1, stream) != 1)
    goto failure;
  if (memcmp (exe_hdr.magic, "MZ", 2) != 0)
    return (fseek (stream, original_pos, SEEK_SET) == -1 ? -1 : 0);
  if (fseek (stream, original_pos + 16 * exe_hdr.hdr_size, SEEK_SET) == -1)
    goto failure;
  if (fread (&patch, sizeof (patch), 1, stream) != 1)
    goto failure;
  if (memcmp (patch.sig, "emx", 3) != 0)
    goto failure;
  if (fseek (stream, original_pos + patch.hdr_loc_lo
             + 65536L * patch.hdr_loc_hi, SEEK_SET) == -1)
    goto failure;
  return 0;

failure:
  saved_errno = errno;
  fseek (stream, original_pos, SEEK_SET);
  errno = saved_errno;
  return -1;
}
#endif


/*
 * Finish up the reading of an a.out file header
 */
const bfd_target *
MY(object_p) (abfd)
  bfd *abfd;
{
#if 0
  struct external_exec exec_bytes;	/* Raw exec header from file */
  struct internal_exec exec;		/* Cleaned-up exec header */
  const bfd_target *target;
  size_t org_pos, add;

  org_pos = bfd_tell (abfd);
  (void)_fseek_hdr(bfd_cache_lookup(abfd));
  add = bfd_tell (abfd) - org_pos;

  if (bfd_bread ((PTR) &exec_bytes, EXEC_BYTES_SIZE, abfd)
      != EXEC_BYTES_SIZE)
  {
    if (bfd_get_error () != bfd_error_system_call)
      bfd_set_error (bfd_error_wrong_format);
    return 0;
  }

  exec.a_info = bfd_h_get_32 (abfd, exec_bytes.e_info);

  if (N_BADMAG (exec))
    return 0;

  NAME(aout,swap_exec_header_in)(abfd, &exec_bytes, &exec);
  exec.a_hdrofs = add;
  target = NAME(aout,some_aout_object_p) (abfd, &exec, MY(callback));
  return target;
#else
  struct external_exec exec_bytes;	/* Raw exec header from file.  */
  struct internal_exec exec;	/* Cleaned-up exec header.  */
  const bfd_target *target;
  bfd_size_type amt = EXEC_BYTES_SIZE;

  if (bfd_bread ((PTR) &exec_bytes, amt, abfd) != amt)
    {
      if (bfd_get_error () != bfd_error_system_call)
	bfd_set_error (bfd_error_wrong_format);
      return 0;
    }

#ifdef SWAP_MAGIC
  exec.a_info = SWAP_MAGIC (exec_bytes.e_info);
#else
  exec.a_info = H_GET_32 (abfd, exec_bytes.e_info);
#endif /* SWAP_MAGIC */

  if (N_BADMAG (exec))
    return 0;
#ifdef MACHTYPE_OK
  if (!(MACHTYPE_OK (N_MACHTYPE (exec))))
    return 0;
#endif

  NAME (aout, swap_exec_header_in) (abfd, &exec_bytes, &exec);

#ifdef SWAP_MAGIC
  /* swap_exec_header_in read in a_info with the wrong byte order */
  exec.a_info = SWAP_MAGIC (exec_bytes.e_info);
#endif /* SWAP_MAGIC */

  target = NAME(aout,some_aout_object_p) (abfd, &exec, MY(callback));

#ifdef ENTRY_CAN_BE_ZERO
  /* The NEWSOS3 entry-point is/was 0, which (amongst other lossage)
     means that it isn't obvious if EXEC_P should be set.
     All of the following must be true for an executable:
     There must be no relocations, the bfd can be neither an
     archive nor an archive element, and the file must be executable.  */

  if (exec.a_trsize + exec.a_drsize == 0
      && bfd_get_format (abfd) == bfd_object && abfd->my_archive == NULL)
    {
      struct stat buf;
#ifndef S_IXUSR
#define S_IXUSR 0100		/* Execute by owner.  */
#endif
      if (stat (abfd->filename, &buf) == 0 && (buf.st_mode & S_IXUSR))
	abfd->flags |= EXEC_P;
    }
#endif /* ENTRY_CAN_BE_ZERO */

  return target;
#endif
}

reloc_howto_type *
MY(reloc_type_lookup) (abfd,code)
     bfd *abfd;
     bfd_reloc_code_real_type code;
{
#undef STD
#define STD(i, j)	case i: return &MY(howto_table)[j]

  if (obj_reloc_entry_size (abfd) == RELOC_EXT_SIZE)
    return NAME(aout,reloc_type_lookup) (abfd, code);


  if (code == BFD_RELOC_CTOR)
    switch (bfd_get_arch_info (abfd)->bits_per_address)
      {
      case 32:
	code = BFD_RELOC_32;
	break;
      case 64:
	code = BFD_RELOC_64;
	break;
      }

  /* std relocs.  */
  switch (code)
    {
      STD (BFD_RELOC_8, 0);
      STD (BFD_RELOC_16, 1);
      STD (BFD_RELOC_32, 2);
      STD (BFD_RELOC_8_PCREL, 4);
      STD (BFD_RELOC_16_PCREL, 5);
      STD (BFD_RELOC_32_PCREL, 6);
      STD (BFD_RELOC_16_BASEREL, 9);
      STD (BFD_RELOC_32_BASEREL, 10);
    default: return (reloc_howto_type *) NULL;
    }
}

/* ELF relocs are against symbols.  If we are producing relocateable
   output, and the reloc is against an external symbol, and nothing
   has given us any additional addend, the resulting reloc will also
   be against the same symbol.  In such a case, we don't want to
   change anything about the way the reloc is handled, since it will
   all be done at final link time.  Rather than put special case code
   into bfd_perform_relocation, all the reloc types use this howto
   function.  It just short circuits the reloc if producing
   relocateable output against an external symbol.  */

bfd_reloc_status_type
MY(generic_reloc) (abfd,
		   reloc_entry,
		   symbol,
		   data,
		   input_section,
		   output_bfd,
		   error_message)
     bfd *abfd ATTRIBUTE_UNUSED;
     arelent *reloc_entry;
     asymbol *symbol;
     PTR data ATTRIBUTE_UNUSED;
     asection *input_section;
     bfd *output_bfd;
     char **error_message ATTRIBUTE_UNUSED;
{
  if (output_bfd != (bfd *) NULL
      && (symbol->flags & BSF_SECTION_SYM) == 0
      && (! reloc_entry->howto->partial_inplace
	  || reloc_entry->addend == 0))
    {
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  return bfd_reloc_continue;
}



