#ifndef _APERTIUM_UNLOCKED_CSTDIO_
#define _APERTIUM_UNLOCKED_CSTDIO_

#include <cstdio>
#include <apertium_config.h>

#if !HAVE_DECL_FPUTS_UNLOCKED
#define fputws_unlocked fputws
#endif

#if !HAVE_DECL_FGETC_UNLOCKED 
#define fgetwc_unlocked fgetwc
#endif

#if !HAVE_DECL_PUTC_UNLOCKED
#define fputwc_unlocked fputwc
#endif

#endif
