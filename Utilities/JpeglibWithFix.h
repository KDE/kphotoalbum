#ifndef UTILITIES_JPEGLIBWITHFIX_H
#define UTILITIES_JPEGLIBWITHFIX_H

// FIXME: Hacky work-around for HAVE_STDLIB_H redifiniton warning.

// Some systems (like mine) have HAVE_STDLIB_H defined in jconfig.h
// which is included from jpeglib.h, but then it's also defined in our
// config.h, so GCC complains about HAVE_STDLIB_H redifinition. Here
// is my hacky fix for it. If you know a better way please let me
// know.  -- Tuomas

#ifdef HAVE_STDLIB_H
# undef HAVE_STDLIB_H
# define HAVE_STDLIB_H_WAS_DEFINED
#endif

extern "C" {
#define XMD_H // prevent INT32 clash from jpeglib
#include <stdlib.h>
#include <stdio.h>
#include <jpeglib.h>
}

#ifdef HAVE_STDLIB_H
# undef HAVE_STDLIB_H
#endif
#include "config.h"

#ifdef HAVE_STDLIB_H_WAS_DEFINED
# ifndef HAVE_STDLIB_H
#  define HAVE_STDLIB_H
# endif
#endif

#undef HAVE_STDLIB_H_WAS_DEFINED

#endif /* UTILITIES_JPEGLIBWITHFIX_H */
