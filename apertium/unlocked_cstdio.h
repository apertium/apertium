#ifndef _APERTIUM_UNLOCKED_CSTDIO_
#define _APERTIUM_UNLOCKED_CSTDIO_

#include <cstdio>

#if !HAVE_DECL_FPUTS_UNLOCKED
#define fputws_unlocked fputws
#endif

#if !HAVE_DECL_FGETC_UNLOCKED 
#define fgetwc_unlocked fgetwc
#endif

#if !HAVE_DECL_PUTC_UNLOCKED
#define fputwc_unlocked fputwc
#endif

#if !HAVE_DECL_FREAD_UNLOCKED
#define fread_unlocked fread
#endif

#endif
