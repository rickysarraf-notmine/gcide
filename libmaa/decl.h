/* decl.h -- Declarations for non-ANSI hosts
 * Created: Sun Nov 19 14:04:27 1995 by faith@cs.unc.edu
 * Revised: Thu Sep 25 08:26:34 1997 by faith@acm.org
 * Copyright 1995, 1996, 1997 Rickard E. Faith (faith@acm.org)
 * 
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 * $Id: decl.h,v 1.13 1998/01/16 03:51:48 faith Exp $
 * 
 */

#ifndef _MAA_DECL_H_
#define _MAA_DECL_H_

/* System dependent declarations: Many brain damaged systems don't provide
declarations for standard library calls.  We provide them here for
situations that we know about. */

#if defined(__sparc__) && !defined(__linux__)
#include <sys/resource.h>
				/* Both SunOS and Solaris */
extern int    getrusage( int who, struct rusage * );
extern void   bcopy( const void *, void *, int );
extern void   bzero( char *b, int );
extern long   random( void );
extern int    srandom( unsigned );
extern char   *index( const char *, int c );
extern int    gethostname( char *, int );

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
extern int    select( int, fd_set *, fd_set *, fd_set *, struct timeval * );
extern void   openlog( const char *ident, int, int );
extern int    vsyslog( int, const char *, va_list );
extern void   closelog( void );
extern int    fseek( FILE *, long, int );
extern int    toupper( int );
#endif
#endif				/* __sparc__ */

#if defined(__ultrix__) && defined(__MIPSEL__)
extern long random( void );
extern void srandom( int );
#endif

#ifdef __hpux__
extern int  strncmp(const char *s1, const char *s2, size_t n);
extern void bcopy(const void *s1, void *s2, size_t n);
#endif

#endif
