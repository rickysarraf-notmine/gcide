/* pr.c -- Process creation and tracking support
 * Created: Sun Jan  7 13:34:08 1996 by r.faith@ieee.org
 * Revised: Wed Mar 19 13:04:19 1997 by faith@cs.unc.edu
 * Copyright 1996 Rickard E. Faith (r.faith@ieee.org)
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
 * $Id: pr.c,v 1.9 1997/03/19 21:10:23 faith Exp $
 *
 * \section{Process Management Routines}
 *
 * \intro The process management routines are designed to facilitate the
 * creation and management of child processes, coprocesses, and associated
 * pipelines and I/O.  Some support for daemons and socket connections is
 * also provided. 
 *
 */

#include "maaP.h"
#include <errno.h>
#include <signal.h>
#include <fcntl.h>

#if HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif

#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif
#ifndef WEXITSTATUS
# define WEXITSTATUS(stat_val) ((unsigned)(stat_val) >> 8)
#endif
#ifndef WIFEXITED
# define WIFEXITED(stat_val) (((stat_val) & 255) == 0)
#endif

typedef struct _pr_Obj {
   int pid;
} *_pr_Obj;

static _pr_Obj _pr_objects = NULL;

#if 0
static void _pr_check( pr_Object object, const char *function )
{
   Obj o = (Obj)object;

   if (!o) err_internal( function, "object is null\n" );
#if MAA_MAGIC
   if (o->magic != PR_MAGIC)
      err_internal( function,
		    "Magic match failed: 0x%08x (should be 0x%08x)\n",
		    o->magic,
		    PR_MAGIC );
#endif
}
#endif

/* The idea for the max_fd call is from W. Richard Stevens; Advanced
   Programming in the UNIX Environment (Addison-Wesley Publishing Co.,
   1992); page 43.  The implementation here, however, is different from
   that provided by Stevens for his open_max routine. */

static int max_fd( void )
{
   static int maxFd = 0;

   if (maxFd) return maxFd;

#if HAVE_SYSCONF
   if ((maxFd = sysconf( _SC_OPEN_MAX )) > 0) return maxFd;
#endif

#ifdef NOFILE
   return maxFd = NOFILE;	/* Usually in sys/params.h */
#else
# ifdef _NFILE
   return maxFd = _NFILE;
# else
   return maxFd = 256;		/* best guess */
# endif
#endif
}

static void _pr_init( void )
{
   if (!_pr_objects)
      _pr_objects = xcalloc( max_fd(), sizeof( struct _pr_Obj ) );
}

void _pr_shutdown( void )
{
   int i;

   if (_pr_objects) {
      for (i = 0; i < max_fd(); i++) {
				/* FIXME: blah */;
	 if (_pr_objects[i].pid) {
	    kill( _pr_objects[i].pid, SIGKILL ); /* FIXME! be gentler. */
	    pr_wait( _pr_objects[i].pid );
	    _pr_objects[i].pid = 0;
	 }
      }
      xfree( _pr_objects );
      _pr_objects = NULL;
   }
}

int pr_open( const char *command, int flags, int *infd, int *outfd, int *errfd)
{
   int        pid;
   int        fdin[2];
   int        fdout[2];
   int        fderr[2];
   arg_List   list;
   int        argc;
   char       **argv;
   int        null;
   
   _pr_init();

   if (flags & ~(PR_USE_STDIN | PR_USE_STDOUT | PR_USE_STDERR
		 | PR_CREATE_STDIN | PR_CREATE_STDOUT | PR_CREATE_STDERR
		 | PR_STDERR_TO_STDOUT))
      err_internal( __FUNCTION__, "Illegal flags: 0x%08x\n", flags );
   if ((flags & PR_USE_STDIN) && (flags & PR_CREATE_STDIN))
      err_internal( __FUNCTION__, "Cannot both use and create stdin\n" );
   if ((flags & PR_USE_STDOUT) && (flags & PR_CREATE_STDOUT))
      err_internal( __FUNCTION__, "Cannot both use and create stdout\n" );
   if ((flags & PR_USE_STDERR) && (flags & PR_CREATE_STDERR))
      err_internal( __FUNCTION__, "Cannot both use and create stderr\n" );
   if ((flags & PR_STDERR_TO_STDOUT)
       && ((flags & PR_USE_STDERR) || (flags & PR_CREATE_STDERR)))
      err_internal( __FUNCTION__,
		    "Cannot use/create stderr when duping to stdout\n" );
   
   list = arg_argify( command, 0 );
   arg_get_vector( list, &argc, &argv );
   PRINTF(MAA_PR,("Execing %s with \"%s\"\n", argv[0], command ));

   if ((flags & PR_CREATE_STDIN) && pipe( fdin ) < 0)
      err_fatal_errno( __FUNCTION__, "Cannot create pipe for stdin\n" );
   if ((flags & PR_CREATE_STDOUT) && pipe( fdout ) < 0)
      err_fatal_errno( __FUNCTION__, "Cannot create pipe for stdout\n" );
   if ((flags & PR_CREATE_STDERR) && pipe( fderr ) < 0)
      err_fatal_errno( __FUNCTION__, "Cannot create pipe for stderr\n" );
   
   if ((pid = fork()) < 0)
      err_fatal_errno( __FUNCTION__, "Cannot fork\n" );

   if (pid == 0) {		/* child */
      int        i;

#define CHILD(CREATE,USE,fds,writefd,readfd,fd,FILENO,flag)    \
      if (flags & CREATE) {                                    \
	 close( fds[writefd] );                                \
	 dup2( fds[readfd], FILENO );                          \
	 close( fds[readfd] );                                 \
      } else if (flags & USE) {                                \
	 if (fd && *fd) {                                      \
	    dup2( *fd, FILENO );                               \
	    close( *fd );                                      \
	 } else {                                              \
	    if ((null = open( "/dev/null", flag )) >= 0) {     \
	       dup2( null, FILENO );                           \
	       close( null );                                  \
	    }                                                  \
	 }                                                     \
      }
      
      CHILD( PR_CREATE_STDIN, PR_USE_STDIN, fdin, 1, 0, infd,
	     STDIN_FILENO, O_RDONLY );
      CHILD( PR_CREATE_STDOUT, PR_USE_STDOUT, fdout, 0, 1, outfd,
	     STDOUT_FILENO, O_WRONLY );
      CHILD( PR_CREATE_STDERR, PR_USE_STDERR, fderr, 0, 1, errfd,
	     STDERR_FILENO, O_WRONLY );
      
#undef CHILD

      if (flags & PR_STDERR_TO_STDOUT)
	 dup2( STDOUT_FILENO, STDERR_FILENO );
      
      for (i = 0; i < max_fd(); i++)
	 if (_pr_objects[i].pid > 0) close( i );

      execvp( argv[0], argv );
      _exit(127);
   }
				/* parent */
#define PARENT(CREATE,USE,fds,readfd,writefd,fd,flag,name)  \
   if (flags & CREATE) {                                    \
      close( fds[ readfd ] );                               \
      *fd = fds[ writefd ];                                 \
      _pr_objects[ *fd ].pid = pid;                         \
      PRINTF(MAA_PR,(name " = %d; ",*fd));                  \
   } else if (flags & USE) {                                \
      if (fd && *fd) {                                      \
	 PRINTF(MAA_PR,(name " = %d*; ",*fd));              \
	 _pr_objects[ *fd ].pid =0;                         \
	 close( *fd );                                      \
      }                                                     \
   }

   PARENT( PR_CREATE_STDIN,  PR_USE_STDIN, fdin,  0, 1,
	   infd,  "w", "stdin" );
   PARENT( PR_CREATE_STDOUT, PR_USE_STDOUT, fdout, 1, 0,
	   outfd, "r", "stdout" );
   PARENT( PR_CREATE_STDERR, PR_USE_STDERR, fderr, 1, 0,
	   errfd, "r", "stderr" );
   
#undef PARENT
   
   PRINTF(MAA_PR,("child pid = %d\n",pid));
   arg_destroy( list );
   
   return pid;
}

int pr_wait( int pid )
{   
   int exitStatus = 0;
   int status;
   
   PRINTF(MAA_PR,("waiting on pid %d\n",pid));
   
   while (waitpid( pid, &status, 0 ) < 0) {
      if (errno != EINTR) {
	 if (errno == ECHILD) return 0;	/* We've already waited */
				/* This is really bad... */
	 PRINTF(MAA_PR,("waitpid() < 0, errno = %d\n", errno ));
	 perror( __FUNCTION__ );
	 return -1;
      }
   }
   
   if (WIFEXITED( status ))
      exitStatus |= WEXITSTATUS( status );
      
                                /* SIGPIPE is ok here, since tar may
                                   shutdown early.  Anything else is a
                                   problem.  */
   if (WIFSIGNALED( status ) && WTERMSIG( status ) != SIGPIPE)
      exitStatus |= 128 + WTERMSIG( status ); /* like bash :-) */

   PRINTF(MAA_PR,("Child %d exited with status 0x%04x\n",pid,exitStatus));
   
   return exitStatus;
}

int pr_close_nowait( int fd )
{
   int pid;

   if (!_pr_objects)
      err_internal( __FUNCTION__, "No previous call to pr_open()\n" );
   if (!(pid = _pr_objects[ fd ].pid))
      err_internal( __FUNCTION__, "File (%d) not created by pr_open()\n", fd );

   _pr_objects[ fd ].pid = 0;

   close( fd );
   return pid;
}

int pr_close( int fd )
{
   int pid = pr_close_nowait( fd );
   
   return pr_wait( pid );
}

int pr_readwrite( int in, int out,
		  const char *inBuffer, int inLen,
		  char *outBuffer, int outMaxLen )
{
   long           flags;
   const char     *inPt  = inBuffer;
   char           *outPt = outBuffer;
   int            outLen = 0;
   fd_set         rfds, wfds, efds;
   struct timeval tv;
   int            n;
   int            count;
   int            retval;
   int            status;
   
   if ((flags = fcntl( in, F_GETFL )) < 0)
      err_fatal_errno( __FUNCTION__, "Can't get flags for output stream\n" );
#ifdef O_NONBLOCK
   flags |= O_NONBLOCK;
#else
   flags |= FNDELAY;
#endif
   fcntl( in, F_SETFL, flags );

   if ((flags = fcntl( out, F_GETFL )) < 0)
      err_fatal_errno( __FUNCTION__, "Can't get flags for input stream\n" );
#ifdef O_NONBLOCK
   flags |= O_NONBLOCK;
#else
   flags |= FNDELAY;
#endif
   fcntl( out, F_SETFL, flags );

   n = max( in, out ) + 1;

   for (;;) {
      tv.tv_sec  = 5;
      tv.tv_usec = 0;
      FD_ZERO( &rfds );
      FD_ZERO( &wfds );
      FD_ZERO( &efds );
      FD_SET( out, &rfds );
      FD_SET( out, &efds );
      if (inLen) {
	 FD_SET( in, &wfds );
	 FD_SET( in, &efds );
      }
   
      switch ((retval = select( n, &rfds, &wfds, &efds, &tv )))
      {
      case -1: err_fatal_errno( __FUNCTION__, "Filter failed\n" ); break;
/*       case 0:  err_fatal( __FUNCTION__, "Filter hung\n" );         break; */
      default:
	 if (dbg_test(MAA_PR)) {
	    printf( "select(2) returns %d,"
		    " inLen = %d, outLen = %d, outMaxLen = %d\n",
		    retval, inLen, outLen, outMaxLen );
	    if (FD_ISSET( in, &rfds ))  printf( "  in/read\n" );
	    if (FD_ISSET( out, &rfds )) printf( "  out/read\n" );
	    if (FD_ISSET( in, &wfds ))  printf( "  in/write\n" );
	    if (FD_ISSET( out, &wfds )) printf( "  out/write\n" );
	    if (FD_ISSET( in, &efds ))  printf( "  in/error\n" );
	    if (FD_ISSET( out, &efds )) printf( "  out/error\n" );
	 }
	 if (inLen) {
	    if ((count = write( in, inPt, inLen )) <= 0) {
	       if (errno != EAGAIN)
		  err_fatal_errno( __FUNCTION__, "Error writing to filter\n" );
	    } else {
	       PRINTF(MAA_PR,("  wrote %d\n",count));
	       inLen -= count;
	       inPt  += count;
	       if (!inLen) {
		  pr_close_nowait( in );
		  n = out + 1;
	       }
	    }
	 }
	 if ((count = read( out, outPt, outMaxLen )) <= 0) {
	    if (!count) {
	       if (inLen)
		  err_fatal( __FUNCTION__,
			     "End of output, but input not flushed\n" );
	       if ((status = pr_close( out )))
		  err_warning( __FUNCTION__,
			       "Filter had non-zero exit status: 0x%x\n",
			       status );
	       return outLen;
	    } else if (errno != EAGAIN)
	       err_fatal_errno( __FUNCTION__, "Error reading from filter\n" );
	 } else {
	    PRINTF(MAA_PR,("  read %d\n",count));
	    outLen    += count;
	    outPt     += count;
	    if ((outMaxLen -= count) < 0)
	       err_fatal( __FUNCTION__, "Output buffer overflow\n" );
	 }
	 break;
      }
   }
}

int pr_filter( const char *command,
	       const char *inBuffer, int inLen,
	       char *outBuffer, int outMaxLen )
{
   int  pid;
   int  in, out;
   
   pid = pr_open( command, PR_CREATE_STDIN | PR_CREATE_STDOUT,
		  &in, &out, NULL );
   return pr_readwrite( in, out, inBuffer, inLen, outBuffer, outMaxLen );
}

int pr_spawn( const char *command )
{
   arg_List list;
   int      argc;
   char     **argv;
   int      pid;
   int      status;
   int      exitStatus = 0;
   
   _pr_init();

   list = arg_argify( command, 0 );
   arg_get_vector( list, &argc, &argv );
   PRINTF(MAA_PR,("Execing %s with \"%s\"\n", argv[0], command ));
   
   if ((pid = fork()) < 0)
      err_fatal_errno( __FUNCTION__, "Cannot fork\n" );

   if (pid == 0) {		/* child */
      execvp( argv[0], argv );
      _exit(127);
   }
   
				/* parent */
   PRINTF(MAA_PR,("child pid = %d\n",pid));
   arg_destroy( list );

   PRINTF(MAA_PR,("waiting on pid %d\n",pid));
   while (waitpid( pid, &status, 0 ) < 0) {
      if (errno != EINTR) {
	 if (errno == ECHILD) return 0;	/* We've already waited */
				/* This is really bad... */
	 PRINTF(MAA_PR,("waitpid() < 0, errno = %d\n", errno ));
	 perror( __FUNCTION__ );
	 return -1;
      }
   }
   
   if (WIFEXITED( status ))
      exitStatus |= WEXITSTATUS( status );
      
                                /* SIGPIPE is ok here, since tar may
                                   shutdown early.  Anything else is a
                                   problem.  */
   if (WIFSIGNALED( status ) && WTERMSIG( status ) != SIGPIPE)
      exitStatus |= 128 + WTERMSIG( status ); /* like bash :-) */

   PRINTF(MAA_PR,("Child %d exited with status 0x%04x\n",pid,exitStatus));
   
   return exitStatus;
}
