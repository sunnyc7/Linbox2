/* linbox/algorithms/
 * Copyright (C) 2005  Pascal Giorgi
 *
 * Written by Pascal Giorgi <pgiorgi@uwaterloo.ca>
 *
 * ========LICENCE========
 * This file is part of the library LinBox.
 *
  * LinBox is free software: you can redistribute it and/or modify
 * it under the terms of the  GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * ========LICENCE========
 */


#ifndef __LINBOX_random_fftprime_H
#define __LINBOX_random_fftprime_H

#include "linbox/integer.h"
#include "linbox/util/timer.h"

namespace LinBox
{

	class RandomFFTPrime {
	public:

		int         _bits;

		RandomFFTPrime(int bits = 20, unsigned long seed = 0) :
			_bits(bits)
		{
			if (! seed)
				RandomFFTPrime::setSeed( (unsigned long)BaseTimer::seed() );
			else
				RandomFFTPrime::setSeed( seed );
		}

		// define the prime type
		typedef integer Prime_Type;

		/** @brief randomPrime()
		 *  return a random prime
		 */
		inline Prime_Type randomPrime() const
		{
			integer tmp;
			size_t cbits=5;
			size_t tresh;
			do {
				tresh = 1<<(cbits);
				size_t p = 1<<((size_t)_bits-cbits);
				do {
					integer::random(tmp,cbits);
					tmp = tmp*p+1;
					tresh--;
				} while (( Givaro::probab_prime(tmp)<2) && (tresh));
				cbits++;
			}
			while(tresh==0);
			return tmp;
		}

		/** @brief randomPrime(Prime_Type& p)
		 *  return a random prime
		 */
		inline Prime_Type randomPrime (Prime_Type& t) const
		{
			size_t cbits=5;
			size_t tresh;
			do {
				tresh = 1<<(cbits);
				size_t p = 1<<((size_t)_bits-cbits);
				do {
					integer::random(t,cbits);
					t = t*p+1;
					tresh--;
				} while (!Givaro::probab_prime(t) && (tresh));
				cbits++;
			}
			while(tresh==0);

			return t;
		}

		/** @brief setSeed (unsigned long ul)
		 *  Set the random seed to be ul.
		 */
		void static setSeed(unsigned long ul)
		{
			integer::seeding(ul);
		}


	};
}

#endif //__LINBOX_random_fftprime_H


// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,:0,t0,+0,=s
// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:

