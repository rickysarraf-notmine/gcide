/* prime.c -- Find prime numbers
 * Created: Thu Jul 20 22:04:37 1995 by r.faith@ieee.org
 * Revised: Mon Sep 23 16:23:48 1996 by faith@cs.unc.edu
 * Copyright 1995, 1996 Rickard E. Faith (r.faith@ieee.org)
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
 * 
 * $Id: prime.c,v 1.4 1996/09/23 23:20:41 faith Exp $
 *
 * \section{Prime Number Routines}
 *
 * \intro These routines are used to find the next larger prime number for
 * expansion of the hash tables used by the hash and set routines.  These
 * routines are only useful for 32-bit unsigned values.
 *
 */

#include "maaP.h"

/* \doc |prm_is_prime| returns 1 if |value| is prime; 0 otherwise.
   Primality is determined by testings all odd divisors less than the
   square root of |value|.  For 32-bit integers, this may mean that we will
   test about $\frac{\sqrt{2^{32}-1}}{2} = 32768$ odd values instead of the
   6542 primes that would actually need to be tested.  (A table of
   pre-computed primes using less than 26kB of memory could be used to
   recover this factor of 5 performance loss.) */

int prm_is_prime( unsigned int value )
{
   unsigned int divisor = 3;
   unsigned int square  = divisor * divisor;

   if (value == 2)   return 1;
   if (value == 3)   return 1;
   if (!(value & 1)) return 0;	/* even */

   while (square < value && value % divisor) {
      ++divisor;
      square += 4 * divisor;
      ++divisor;
   }

   return value % divisor != 0;
}

/* \doc |prm_next_prime| returns the smallest odd prime greater than or
   equal to |start|. */

unsigned long prm_next_prime( unsigned int start )
{
   unsigned int next;
   
   for (next = start | 1; !prm_is_prime( next ); next += 2);
   return next;
}
