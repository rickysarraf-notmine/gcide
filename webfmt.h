/* webfmt.h -- 
 * Created: Sun Mar 16 09:33:32 1997 by faith@cs.unc.edu
 * Revised: Tue May 20 21:23:33 1997 by faith@acm.org
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
 * $Id: webfmt.h,v 1.3 1997/05/21 23:52:07 faith Exp $
 * 
 */

#ifndef _WEBFMT_H_
#define _WEBFMT_H_

#include "maa.h"
#include "dump.h"
#include "fmt.h"

#define DBG_VERBOSE     (0<<30|1<< 0) /* Verbose                           */
#define DBG_SCAN        (0<<30|1<< 1) /* Config file scan                  */
#define DBG_PARSE       (0<<30|1<< 2) /* Config file parse                 */
#define DBG_NEWLINE     (0<<30|1<< 3) /* Newline folding                   */

typedef struct pgwToken {
   const char   *string;
   int          integer;
   src_Type     src;
} pgwToken;

				/* dmalloc must be last */
#ifdef DMALLOC_FUNC_CHECK
# include "dmalloc.h"
#endif

#endif
