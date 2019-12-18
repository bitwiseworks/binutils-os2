/* Demangler for GNU C++
   Copyright (C) 1989-2019 Free Software Foundation, Inc.
   Written by James Clark (jjc@jclark.uucp)
   Rewritten by Fred Fish (fnf@cygnus.com) for ARM and Lucid demangling
   Modified by Satish Pai (pai@apollo.hp.com) for HP demangling

This file is part of the libiberty library.
Libiberty is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

In addition to the permissions in the GNU Library General Public
License, the Free Software Foundation gives you unlimited permission
to link the compiled version of this file into combinations with other
programs, and to distribute those combinations without any restriction
coming from the use of this file.  (The Library Public License
restrictions do apply in other respects; for example, they cover
modification of the file, and distribution when not linked into a
combined executable.)

Libiberty is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with libiberty; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,
Boston, MA 02110-1301, USA.  */

/* This file lives in both GCC and libiberty.  When making changes, please
   try not to break either.  */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "safe-ctype.h"

#include <string.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#else
void * malloc ();
void * realloc ();
#endif

#include <demangle.h>
#undef CURRENT_DEMANGLING_STYLE
#define CURRENT_DEMANGLING_STYLE options

#include "libiberty.h"
#include "rust-demangle.h"

enum demangling_styles current_demangling_style = auto_demangling;

const struct demangler_engine libiberty_demanglers[] =
{
  {
    NO_DEMANGLING_STYLE_STRING,
    no_demangling,
    "Demangling disabled"
  }
  ,
  {
    AUTO_DEMANGLING_STYLE_STRING,
      auto_demangling,
      "Automatic selection based on executable"
  }
  ,
  {
    GNU_V3_DEMANGLING_STYLE_STRING,
    gnu_v3_demangling,
    "GNU (g++) V3 (Itanium C++ ABI) style demangling"
  }
  ,
  {
    JAVA_DEMANGLING_STYLE_STRING,
    java_demangling,
    "Java style demangling"
  }
  ,
  {
    GNAT_DEMANGLING_STYLE_STRING,
    gnat_demangling,
    "GNAT style demangling"
  }
  ,
  {
    DLANG_DEMANGLING_STYLE_STRING,
    dlang_demangling,
    "DLANG style demangling"
  }
  ,
  {
    RUST_DEMANGLING_STYLE_STRING,
    rust_demangling,
    "Rust style demangling"
  }
  ,
  {
    NULL, unknown_demangling, NULL
  }
};

/* Add a routine to set the demangling style to be sure it is valid and
   allow for any demangler initialization that maybe necessary. */

enum demangling_styles
cplus_demangle_set_style (enum demangling_styles style)
{
  const struct demangler_engine *demangler = libiberty_demanglers; 

  for (; demangler->demangling_style != unknown_demangling; ++demangler)
    if (style == demangler->demangling_style)
      {
	current_demangling_style = style;
	return current_demangling_style;
      }

  return unknown_demangling;
}

/* Do string name to style translation */

enum demangling_styles
cplus_demangle_name_to_style (const char *name)
{
  const struct demangler_engine *demangler = libiberty_demanglers;

  for (; demangler->demangling_style != unknown_demangling; ++demangler)
    if (strcmp (name, demangler->demangling_style_name) == 0)
      return demangler->demangling_style;

  return unknown_demangling;
}

/* char *cplus_demangle (const char *mangled, int options)

   If MANGLED is a mangled function name produced by GNU C++, then
   a pointer to a @code{malloc}ed string giving a C++ representation
   of the name will be returned; otherwise NULL will be returned.
   It is the caller's responsibility to free the string which
   is returned.

   Note that any leading underscores, or other such characters prepended by
   the compilation system, are presumed to have already been stripped from
   MANGLED.  */

char *
cplus_demangle (const char *mangled, int options)
{
  char *ret;

  if (current_demangling_style == no_demangling)
    return xstrdup (mangled);

  if ((options & DMGL_STYLE_MASK) == 0)
    options |= (int) current_demangling_style & DMGL_STYLE_MASK;

  /* The V3 ABI demangling is implemented elsewhere.  */
  if (GNU_V3_DEMANGLING || RUST_DEMANGLING || AUTO_DEMANGLING)
    {
      ret = cplus_demangle_v3 (mangled, options);
      if (GNU_V3_DEMANGLING)
	return ret;

      if (ret)
	{
	  /* Rust symbols are GNU_V3 mangled plus some extra subtitutions.
	     The subtitutions are always smaller, so do in place changes.  */
	  if (rust_is_mangled (ret))
	    rust_demangle_sym (ret);
	  else if (RUST_DEMANGLING)
	    {
	      free (ret);
	      ret = NULL;
	    }
	}

      if (ret || RUST_DEMANGLING)
	return ret;
    }

  if (JAVA_DEMANGLING)
    {
      ret = java_demangle_v3 (mangled);
      if (ret)
        return ret;
    }

  if (GNAT_DEMANGLING)
    return ada_demangle (mangled, options);

  if (DLANG_DEMANGLING)
    {
      ret = dlang_demangle (mangled, options);
      if (ret)
	return ret;
    }

  return (ret);
}

char *
rust_demangle (const char *mangled, int options)
{
  /* Rust symbols are GNU_V3 mangled plus some extra subtitutions.  */
  char *ret = cplus_demangle_v3 (mangled, options);

  /* The Rust subtitutions are always smaller, so do in place changes.  */
  if (ret != NULL)
    {
      if (rust_is_mangled (ret))
	rust_demangle_sym (ret);
      else
	{
	  free (ret);
	  ret = NULL;
	}
    }

  return ret;
}

/* Demangle ada names.  The encoding is documented in gcc/ada/exp_dbug.ads.  */

char *
ada_demangle (const char *mangled, int option ATTRIBUTE_UNUSED)
{
  int len0;
  const char* p;
  char *d;
  char *demangled = NULL;
  
  /* Discard leading _ada_, which is used for library level subprograms.  */
  if (strncmp (mangled, "_ada_", 5) == 0)
    mangled += 5;

  /* All ada unit names are lower-case.  */
  if (!ISLOWER (mangled[0]))
    goto unknown;

  /* Most of the demangling will trivially remove chars.  Operator names
     may add one char but because they are always preceeded by '__' which is
     replaced by '.', they eventually never expand the size.
     A few special names such as '___elabs' add a few chars (at most 7), but
     they occur only once.  */
  len0 = strlen (mangled) + 7 + 1;
  demangled = XNEWVEC (char, len0);
  
  d = demangled;
  p = mangled;
  while (1)
    {
      /* An entity names is expected.  */
      if (ISLOWER (*p))
        {
          /* An identifier, which is always lower case.  */
          do
            *d++ = *p++;
          while (ISLOWER(*p) || ISDIGIT (*p)
                 || (p[0] == '_' && (ISLOWER (p[1]) || ISDIGIT (p[1]))));
        }
      else if (p[0] == 'O')
        {
          /* An operator name.  */
          static const char * const operators[][2] =
            {{"Oabs", "abs"},  {"Oand", "and"},    {"Omod", "mod"},
             {"Onot", "not"},  {"Oor", "or"},      {"Orem", "rem"},
             {"Oxor", "xor"},  {"Oeq", "="},       {"One", "/="},
             {"Olt", "<"},     {"Ole", "<="},      {"Ogt", ">"},
             {"Oge", ">="},    {"Oadd", "+"},      {"Osubtract", "-"},
             {"Oconcat", "&"}, {"Omultiply", "*"}, {"Odivide", "/"},
             {"Oexpon", "**"}, {NULL, NULL}};
          int k;

          for (k = 0; operators[k][0] != NULL; k++)
            {
              size_t slen = strlen (operators[k][0]);
              if (strncmp (p, operators[k][0], slen) == 0)
                {
                  p += slen;
                  slen = strlen (operators[k][1]);
                  *d++ = '"';
                  memcpy (d, operators[k][1], slen);
                  d += slen;
                  *d++ = '"';
                  break;
                }
            }
          /* Operator not found.  */
          if (operators[k][0] == NULL)
            goto unknown;
        }
      else
        {
          /* Not a GNAT encoding.  */
          goto unknown;
        }

      /* The name can be directly followed by some uppercase letters.  */
      if (p[0] == 'T' && p[1] == 'K')
        {
          /* Task stuff.  */
          if (p[2] == 'B' && p[3] == 0)
            {
              /* Subprogram for task body.  */
              break;
            }
          else if (p[2] == '_' && p[3] == '_')
            {
              /* Inner declarations in a task.  */
              p += 4;
              *d++ = '.';
              continue;
            }
          else
            goto unknown;
        }
      if (p[0] == 'E' && p[1] == 0)
        {
          /* Exception name.  */
          goto unknown;
        }
      if ((p[0] == 'P' || p[0] == 'N') && p[1] == 0)
        {
          /* Protected type subprogram.  */
          break;
        }
      if ((*p == 'N' || *p == 'S') && p[1] == 0)
        {
          /* Enumerated type name table.  */
          goto unknown;
        }
      if (p[0] == 'X')
        {
          /* Body nested.  */
          p++;
          while (p[0] == 'n' || p[0] == 'b')
            p++;
        }
      if (p[0] == 'S' && p[1] != 0 && (p[2] == '_' || p[2] == 0))
        {
          /* Stream operations.  */
          const char *name;
          switch (p[1])
            {
            case 'R':
              name = "'Read";
              break;
            case 'W':
              name = "'Write";
              break;
            case 'I':
              name = "'Input";
              break;
            case 'O':
              name = "'Output";
              break;
            default:
              goto unknown;
            }
          p += 2;
          strcpy (d, name);
          d += strlen (name);
        }
      else if (p[0] == 'D')
        {
          /* Controlled type operation.  */
          const char *name;
          switch (p[1])
            {
            case 'F':
              name = ".Finalize";
              break;
            case 'A':
              name = ".Adjust";
              break;
            default:
              goto unknown;
            }
          strcpy (d, name);
          d += strlen (name);
          break;
        }

      if (p[0] == '_')
        {
          /* Separator.  */
          if (p[1] == '_')
            {
              /* Standard separator.  Handled first.  */
              p += 2;

              if (ISDIGIT (*p))
                {
                  /* Overloading number.  */
                  do
                    p++;
                  while (ISDIGIT (*p) || (p[0] == '_' && ISDIGIT (p[1])));
                  if (*p == 'X')
                    {
                      p++;
                      while (p[0] == 'n' || p[0] == 'b')
                        p++;
                    }
                }
              else if (p[0] == '_' && p[1] != '_')
                {
                  /* Special names.  */
                  static const char * const special[][2] = {
                    { "_elabb", "'Elab_Body" },
                    { "_elabs", "'Elab_Spec" },
                    { "_size", "'Size" },
                    { "_alignment", "'Alignment" },
                    { "_assign", ".\":=\"" },
                    { NULL, NULL }
                  };
                  int k;

                  for (k = 0; special[k][0] != NULL; k++)
                    {
                      size_t slen = strlen (special[k][0]);
                      if (strncmp (p, special[k][0], slen) == 0)
                        {
                          p += slen;
                          slen = strlen (special[k][1]);
                          memcpy (d, special[k][1], slen);
                          d += slen;
                          break;
                        }
                    }
                  if (special[k][0] != NULL)
                    break;
                  else
                    goto unknown;
                }
              else
                {
                  *d++ = '.';
                  continue;
                }
            }
          else if (p[1] == 'B' || p[1] == 'E')
            {
              /* Entry Body or barrier Evaluation.  */
              p += 2;
              while (ISDIGIT (*p))
                p++;
              if (p[0] == 's' && p[1] == 0)
                break;
              else
                goto unknown;
            }
          else
            goto unknown;
        }

      if (p[0] == '.' && ISDIGIT (p[1]))
        {
          /* Nested subprogram.  */
          p += 2;
          while (ISDIGIT (*p))
            p++;
        }
      if (*p == 0)
        {
          /* End of mangled name.  */
          break;
        }
      else
        goto unknown;
    }
  *d = 0;
  return demangled;

 unknown:
  XDELETEVEC (demangled);
  len0 = strlen (mangled);
  demangled = XNEWVEC (char, len0 + 3);

  if (mangled[0] == '<')
     strcpy (demangled, mangled);
  else
    sprintf (demangled, "<%s>", mangled);

  return demangled;
}

/* bird: This was put back since we're building some older gcc, remove
         when going for gcc which doesn't expect MAIN and main().
 */

/* To generate a standalone demangler program for testing purposes,
   just compile and link this file with -DMAIN and libiberty.a.  When
   run, it demangles each command line arg, or each stdin string, and
   prints the result on stdout.  */

#ifdef MAIN

#include "getopt.h"

static const char *program_name;
static const char *program_version = VERSION;
static int flags = DMGL_PARAMS | DMGL_ANSI | DMGL_VERBOSE;

static void demangle_it PARAMS ((char *));
static void usage PARAMS ((FILE *, int)) ATTRIBUTE_NORETURN;
static void fatal PARAMS ((const char *)) ATTRIBUTE_NORETURN;
static void print_demangler_list PARAMS ((FILE *));

static void
demangle_it (mangled_name)
     char *mangled_name;
{
  char *result;

  /* For command line args, also try to demangle type encodings.  */
  result = cplus_demangle (mangled_name, flags | DMGL_TYPES);
  if (result == NULL)
    {
      printf ("%s\n", mangled_name);
    }
  else
    {
      printf ("%s\n", result);
      free (result);
    }
}

static void
print_demangler_list (stream)
     FILE *stream;
{
  const struct demangler_engine *demangler;

  fprintf (stream, "{%s", libiberty_demanglers->demangling_style_name);

  for (demangler = libiberty_demanglers + 1;
       demangler->demangling_style != unknown_demangling;
       ++demangler)
    fprintf (stream, ",%s", demangler->demangling_style_name);

  fprintf (stream, "}");
}

static void
usage (stream, status)
     FILE *stream;
     int status;
{
  fprintf (stream, "\
Usage: %s [-_] [-n] [--strip-underscores] [--no-strip-underscores] \n",
	   program_name);

  fprintf (stream, "\
       [-s ");
  print_demangler_list (stream);
  fprintf (stream, "]\n");

  fprintf (stream, "\
       [--format ");
  print_demangler_list (stream);
  fprintf (stream, "]\n");

  fprintf (stream, "\
       [--help] [--version] [arg...]\n");
  exit (status);
}

#define MBUF_SIZE 32767
char mbuffer[MBUF_SIZE];

/* Defined in the automatically-generated underscore.c.  */
extern int prepends_underscore;

int strip_underscore = 0;

static const struct option long_options[] = {
  {"strip-underscores", no_argument, 0, '_'},
  {"format", required_argument, 0, 's'},
  {"help", no_argument, 0, 'h'},
  {"no-strip-underscores", no_argument, 0, 'n'},
  {"version", no_argument, 0, 'v'},
  {0, no_argument, 0, 0}
};

/* More 'friendly' abort that prints the line and file.
   config.h can #define abort fancy_abort if you like that sort of thing.  */

void
fancy_abort ()
{
  fatal ("Internal gcc abort.");
}


static const char *
standard_symbol_characters PARAMS ((void));

static const char *
hp_symbol_characters PARAMS ((void));

static const char *
gnu_v3_symbol_characters PARAMS ((void));

/* Return the string of non-alnum characters that may occur
   as a valid symbol component, in the standard assembler symbol
   syntax.  */

static const char *
standard_symbol_characters ()
{
  return "_$.";
}


/* Return the string of non-alnum characters that may occur
   as a valid symbol name component in an HP object file.

   Note that, since HP's compiler generates object code straight from
   C++ source, without going through an assembler, its mangled
   identifiers can use all sorts of characters that no assembler would
   tolerate, so the alphabet this function creates is a little odd.
   Here are some sample mangled identifiers offered by HP:

	typeid*__XT24AddressIndExpClassMember_
	[Vftptr]key:__dt__32OrdinaryCompareIndExpClassMemberFv
	__ct__Q2_9Elf64_Dyn18{unnamed.union.#1}Fv

   This still seems really weird to me, since nowhere else in this
   file is there anything to recognize curly brackets, parens, etc.
   I've talked with Srikanth <srikanth@cup.hp.com>, and he assures me
   this is right, but I still strongly suspect that there's a
   misunderstanding here.

   If we decide it's better for c++filt to use HP's assembler syntax
   to scrape identifiers out of its input, here's the definition of
   the symbol name syntax from the HP assembler manual:

       Symbols are composed of uppercase and lowercase letters, decimal
       digits, dollar symbol, period (.), ampersand (&), pound sign(#) and
       underscore (_). A symbol can begin with a letter, digit underscore or
       dollar sign. If a symbol begins with a digit, it must contain a
       non-digit character.

   So have fun.  */
static const char *
hp_symbol_characters ()
{
  return "_$.<>#,*&[]:(){}";
}


/* Return the string of non-alnum characters that may occur
   as a valid symbol component in the GNU C++ V3 ABI mangling
   scheme.  */

static const char *
gnu_v3_symbol_characters ()
{
  return "_$.";
}


extern int main PARAMS ((int, char **));

int
main (argc, argv)
     int argc;
     char **argv;
{
  char *result;
  int c;
  const char *valid_symbols;
  enum demangling_styles style = auto_demangling;

  program_name = argv[0];

  strip_underscore = prepends_underscore;

  while ((c = getopt_long (argc, argv, "_ns:", long_options, (int *) 0)) != EOF)
    {
      switch (c)
	{
	case '?':
	  usage (stderr, 1);
	  break;
	case 'h':
	  usage (stdout, 0);
	case 'n':
	  strip_underscore = 0;
	  break;
	case 'v':
	  printf ("GNU %s (C++ demangler), version %s\n", program_name, program_version);
	  return (0);
	case '_':
	  strip_underscore = 1;
	  break;
	case 's':
	  {
	    style = cplus_demangle_name_to_style (optarg);
	    if (style == unknown_demangling)
	      {
		fprintf (stderr, "%s: unknown demangling style `%s'\n",
			 program_name, optarg);
		return (1);
	      }
	    else
	      cplus_demangle_set_style (style);
	  }
	  break;
	}
    }

  if (optind < argc)
    {
      for ( ; optind < argc; optind++)
	{
	  demangle_it (argv[optind]);
	}
    }
  else
    {
      switch (current_demangling_style)
	{
	case gnu_demangling:
	case lucid_demangling:
	case arm_demangling:
	case java_demangling:
	case edg_demangling:
	case gnat_demangling:
	case auto_demangling:
	  valid_symbols = standard_symbol_characters ();
	  break;
	case hp_demangling:
	  valid_symbols = hp_symbol_characters ();
	  break;
	case gnu_v3_demangling:
	  valid_symbols = gnu_v3_symbol_characters ();
	  break;
	default:
	  /* Folks should explicitly indicate the appropriate alphabet for
	     each demangling.  Providing a default would allow the
	     question to go unconsidered.  */
	  abort ();
	}

      for (;;)
	{
	  int i = 0;
	  c = getchar ();
	  /* Try to read a label.  */
	  while (c != EOF && (ISALNUM (c) || strchr (valid_symbols, c)))
	    {
	      if (i >= MBUF_SIZE-1)
		break;
	      mbuffer[i++] = c;
	      c = getchar ();
	    }
	  if (i > 0)
	    {
	      int skip_first = 0;

	      if (mbuffer[0] == '.' || mbuffer[0] == '$')
		++skip_first;
	      if (strip_underscore && mbuffer[skip_first] == '_')
		++skip_first;

	      if (skip_first > i)
		skip_first = i;

	      mbuffer[i] = 0;
	      flags |= (int) style;
	      result = cplus_demangle (mbuffer + skip_first, flags);
	      if (result)
		{
		  if (mbuffer[0] == '.')
		    putc ('.', stdout);
		  fputs (result, stdout);
		  free (result);
		}
	      else
		fputs (mbuffer, stdout);

	      fflush (stdout);
	    }
	  if (c == EOF)
	    break;
	  putchar (c);
	  fflush (stdout);
	}
    }

  return (0);
}

static void
fatal (str)
     const char *str;
{
  fprintf (stderr, "%s: %s\n", program_name, str);
  exit (1);
}

PTR
xmalloc (size)
  size_t size;
{
  register PTR value = (PTR) malloc (size);
  if (value == 0)
    fatal ("virtual memory exhausted");
  return value;
}

PTR
xrealloc (ptr, size)
  PTR ptr;
  size_t size;
{
  register PTR value = (PTR) realloc (ptr, size);
  if (value == 0)
    fatal ("virtual memory exhausted");
  return value;
}
#endif	/* main */
