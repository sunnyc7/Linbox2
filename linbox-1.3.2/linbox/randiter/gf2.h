/* linbox/randiter/gf2.h
 * Copyright (C) 2003 Bradford Hovinen
 *
 * Written by Bradford Hovinen <bghovinen@math.uwaterloo.ca>
 *
 * ------------------------------------
 *
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * ========LICENCE========
 *.
 */

#ifndef __LINBOX_randiter_gf2_H
#define __LINBOX_randiter_gf2_H

#include <iostream>
#include <vector>

#include <time.h>
#include "linbox/linbox-config.h"
#include "linbox/integer.h"
// #include "linbox/field/modular.h"
#include "linbox/element/abstract.h"
#include "linbox/element/envelope.h"
#include "linbox/util/commentator.h"
#include "linbox/randiter/mersenne-twister.h"
#include "linbox/vector/bit-vector.h"

namespace LinBox
{

	class GF2RandIter {
	public:
		typedef bool Element;

		/** Constructor from field, sampling size, and seed.
		 * The random field element iterator works in the field F, is seeded
		 * by seed, and it returns any one element with probability no more
		 * than 1/min (size, F.cardinality (c)).
		 * A sampling size of zero means to sample from the entire field.
		 * A seed of zero means to use some arbitrary seed for the generator.
		 * Purely virtual.
		 * @param F    LinBox field archetype object in which to do arithmetic
		 * @param size constant integer reference of sample size from which to
		 *             sample (default = modulus of field)
		 * @param seed constant integer reference from which to seed random number
		 *             generator (default = 0)
		 */
		GF2RandIter (const GF2 & F,
			     const integer &size  = 0 ,
			     const integer &seed = 0)
		{
			long _seed = seed;

			if (_seed == 0) _seed = time (NULL);
			MT.setSeed ((uint32_t)_seed);
		}

		/// constructor
		GF2RandIter (const GF2RandIter &) {}

		/** Destructor.
		 * This destructs the random field element generator object.
		 */
		~GF2RandIter () {}

		/** Assignment operator.
		 * Assigns ModularRandIter object R to generator.
		 * @param  R ModularRandIter object.
		 */
		GF2RandIter &operator = (const GF2RandIter & R)
		{ return *this; }

		/** Random field element creator.
		 * This returns a random field element from the information supplied
		 * at the creation of the generator.
		 * Required by abstract base class.
		 * @return reference to random field element
		 */
		bool &random (bool &a)  const
		{
			return a = MT.randomIntRange (0, 2);
		}

		/** Random field element creator.
		 * This returns a random field element from the information supplied
		 * at the creation of the generator.
		 * Required by abstract base class.
		 * @return reference to random field element
		 */
		BitVector::reference random (BitVector::reference a)  const
		{ return a = MT.randomIntRange (0, 2); }


		/** Random field element creator.
		 * This returns a random field element from the information supplied
		 * at the creation of the generator.
		 * Required by abstract base class.
		 * @return reference to random field element
		 */
		stdBitReference random (stdBitReference a)  const
		{ return a = MT.randomIntRange (0, 2); }

		/** Random field element creator.
		 * This returns a random field element from the information supplied
		 * at the creation of the generator.
		 * Required by abstract base class.
		 * @return reference to random field element
		 */
		ElementAbstract &random (ElementAbstract &a)  const
		{
			bool tmp;

			random (tmp);
			return (a = ElementEnvelope <GF2> (tmp));
		}


		uint32_t& random (uint32_t& a)  const
		{ return a = MT.randomInt(); }

		MersenneTwister& getMT() { return MT; }
		const MersenneTwister& getMT() const { return MT; }

	private:

		MersenneTwister MT;

	}; // class GF2RandIter

} // namespace LinBox

namespace LinBox
{

	template<size_t bitsize>
	struct MTrandomInt {
		template<typename M32Twister>
		uint32_t operator() (M32Twister& MT) const
		{
			return MT.randomInt();
		}
	};

	template<>
	struct MTrandomInt<64> {
		template<typename M32Twister>
		uint64_t operator() (M32Twister& MT) const
		{
			uint64_t tmp = MT.randomInt();
			tmp <<=32;
			return tmp += MT.randomInt();
		}
	};

} // namespace LinBox

#endif // __LINBOX_randiter_gf2_H


// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,:0,t0,+0,=s
// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:

