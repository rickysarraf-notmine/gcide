/* dump.h -- 
 * Created: Sat Mar  8 15:48:21 1997 by faith@cs.unc.edu
 * Revised: Sun Mar 16 11:54:40 1997 by faith@cs.unc.edu
 * Copyright 1997 Rickard E. Faith (faith@cs.unc.edu)
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 1, or (at your option) any
 * later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 * $Id: dump.h,v 1.1 1997/03/18 01:38:21 faith Exp $
 * 
 */

#ifndef _DUMP_H_
#define _DUMP_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef __GNUC__
#define __FUNCTION__ __FILE__
#endif

/* AIX requires this to be the first thing in the file.  */
#ifdef __GNUC__
# define alloca __builtin_alloca
#else
# if HAVE_ALLOCA_H
#  include <alloca.h>
# else
#  ifdef _AIX
 #pragma alloca
#  else
#   ifndef alloca /* predefined by HP cc +Olibcalls */
char *alloca ();
#   endif
#  endif
# endif
#endif

/* Get string functions */
#if STDC_HEADERS
# include <string.h>
#else
# if HAVE_STRINGS_H
#  include <strings.h>
# endif
# if !HAVE_STRCHR
#  define strchr index
#  define strrchr rindex
# endif
# if !HAVE_MEMCPY
#  define memcpy(d, s, n) bcopy ((s), (d), (n))
#  define memmove(d, s, n) bcopy ((s), (d), (n))
# endif
#endif

#if !HAVE_STRDUP
extern char *strdup( const char * );
#endif

#if !HAVE_STRTOL
extern long strtol( const char *, char **, int );
#endif

#if !HAVE_STRTOUL
extern unsigned long int strtoul( const char *, char **, int );
#endif

/* Get time functions */
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

/* Include some standard header files. */
#include <stdio.h>
#if HAVE_UNISTD_H
# include <sys/types.h>
# include <unistd.h>
# include <stdlib.h>
#endif

#include <getopt.h>

/* We actually need a few non-ANSI C things... */
#if defined(__STRICT_ANSI__)
extern char     *strdup( const char * );
extern int      fileno( FILE *stream );
extern FILE     *fdopen( int fildes, const char *mode );
extern void     bcopy( const void *src, void *dest, int n );
extern long int random( void );
extern void     srandom( unsigned int );
#endif

#if HAVE_SYS_RESOURCE_H
# include <sys/resource.h>
#endif

/* Provide assert() */
#include <assert.h>

/* Provide stdarg support */
#include <stdarg.h>

#if HAVE_LIMITS_H
#include <limits.h>
#endif

				/* Local stuff */
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif

#include "maa.h"
#include "fmt.h"

#define BUFFERSIZE 32 * 1024


/* System dependent declarations: Many brain damaged systems don't provide
declarations for standard library calls.  We provide them here for
situations that we know about. */

#if defined(__sparc__)
				/* Both SunOS and Solaris */
extern int    getrusage( int who, struct rusage * );
extern void   bcopy( const void *, void *, int );
extern long   random( void );
extern int    srandom( unsigned );
extern char   *index( const char *, int c );

#if !defined(__svr4__)
				/* Old braindamage for SunOS only */
extern char   *memset( void *, int, int );
extern char   *strchr( const char *, int );
extern char   *strdup( const char * );
extern char   *strrchr( const char *, int );
extern char   *strtok( char *, const char * );
extern int    _filbuf( FILE * );
extern int    _flsbuf( unsigned char, FILE * );
extern int    fflush( FILE * );
extern int    fprintf( FILE *, const char *, ... );
extern int    fputc( char, FILE * );
extern int    fputs( const char *, FILE * );
extern int    fread( char *, int, int, FILE * );
extern int    fscanf( FILE *, const char *, ... );
extern int    fwrite( char *, int, int, FILE * );
extern int    gettimeofday( struct timeval *, struct timezone * );
extern int    on_exit( void (*)(), caddr_t );
extern int    pclose( FILE * );
extern int    printf( const char *, ... );
extern int    scanf( const char *, ... );
extern int    sscanf( const char *, const char *, ... );
extern int    unlink( const char * );
extern int    vfprintf( FILE *, const char *, ... );
extern int    vsprintf( char *, const char *, ... );
extern long   strtol( const char *, char **, int );
extern time_t time( time_t * );
extern void   fclose( FILE * );
extern void   perror( const char * );
#endif
#endif				/* __sparc__ */

#if defined(__ultrix__) && defined(__MIPSEL__)
extern long random( void );
extern void srandom( int );
#endif

				/* dmalloc must be last */
#ifdef DMALLOC_FUNC_CHECK
# include "dmalloc.h"
#endif

#endif
