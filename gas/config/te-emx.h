#define TE_EMX

#undef abort
#include <process.h>
#include <io.h>
#define abort()		as_abort (__FILE__, __LINE__, __PRETTY_FUNCTION__)

#if 0
#define LEX_AT 1 /* can have @'s inside labels */
#else
#define LEX_AT (LEX_BEGIN_NAME | LEX_NAME) /* Can have @'s inside labels.  */
#endif
#ifndef EMX
#define EMX
#endif
#define LOCAL_LABELS_DOLLAR     1

#include "obj-format.h"
