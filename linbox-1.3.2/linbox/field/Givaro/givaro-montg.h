/* linbox/field/givaro-gfq.h
 * Copyright (C) 2004 Jean-Guillaume Dumas
 *
 * Written by Jean-Guillaume Dumas <Jean-Guillaume.Dumas@imag.fr>
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

/* WARNING this wrapper works only with an improved version of Givaro.
 * This version of givaro won't be available for public yet.
 * But it is available on my web page.
 * You can send me a mail to get it or for others details.
 */

#ifndef __LINBOX_field_givaro_montgomery_H
#define __LINBOX_field_givaro_montgomery_H


#include "linbox/integer.h"
#include "linbox/field/field-interface.h"
#include "linbox/util/debug.h"
#include "linbox/linbox-config.h"
#include "linbox/field/field-traits.h"

//------------------------------------
// Files of Givaro library


#include <givaro/givmontg32.h>
#include <givaro/giv_randiter.h>
//------------------------------------

// Namespace in which all LinBox code resides
namespace LinBox
{


	template <class Ring>
	struct ClassifyRing;

	class GivaroMontg;

	template<>
	struct ClassifyRing<GivaroMontg> {
		typedef RingCategories::ModularTag categoryTag;
	};
	/**
	 \brief wrapper of Givaro's Givaro::Montgomery< Givaro::Std32>.
	 \ingroup field

	 *  This class is a modular representation with a Givaro::Montgomery reduction
	 */
	class GivaroMontg : public Givaro::Montgomery< Givaro::Std32>, public FieldInterface {

	public:

		/** Element type.
		 *  This type is inherited from the Givaro class Givaro::Montgomery< Givaro::Std32>
		 */
		typedef Givaro::Montgomery< Givaro::Std32> Father_t ;
		typedef  Father_t::Rep Element;
		using Father_t::one;
		using Father_t::mOne;
		using Father_t::zero;

		/** RandIter type
		 *  This type is inherited from the Givaro class Givaro::Montgomery< Givaro::Std32>
		 */
		typedef Givaro::GIV_randIter< Givaro::Montgomery< Givaro::Std32>, LinBox::integer >  RandIter;

		/** Constructor from an integer
		 *  this constructor use the ZpzDom<TAG> constructor
		 */
		GivaroMontg(const integer& p) :
		 Givaro::Montgomery< Givaro::Std32>(static_cast<uint32_t>(long(p)))
		{ }

		/** Constructor from an integer (takes degree of extension as 2nd parameter, must be 1)
		 *  this constructor use the ZpzDom<TAG> constructor
		 */
	  	GivaroMontg(const integer& p, const integer& k) :
		 Givaro::Montgomery< Givaro::Std32>(static_cast<uint32_t>(long(p)))
		{

			if (k!=1)
				throw PreconditionFailed(__func__,__FILE__,__LINE__,"exponent must be 1");
		}

		/** Characteristic.
		 * Return integer representing characteristic of the domain.
		 * Returns a positive integer to all domains with finite characteristic,
		 * and returns 0 to signify a domain of infinite characteristic.
		 * @return integer representing characteristic of the domain.
		 */
		integer& characteristic(integer& c) const
		{
			return c=integer(static_cast<long>( Givaro::Montgomery< Givaro::Std32>::characteristic()));
		}

		long characteristic() const
		{
			return static_cast<long>( Givaro::Montgomery< Givaro::Std32>::characteristic());
		}


		/** Cardinality.
		 * Return integer representing cardinality of the domain.
		 * Returns a non-negative integer for all domains with finite
		 * cardinality, and returns -1 to signify a domain of infinite
		 * cardinality.
		 * @return integer representing cardinality of the domain
		 */
		integer& cardinality(integer& c) const
		{
		       	return c=integer(static_cast<long>( Givaro::Montgomery< Givaro::Std32>::size()));
		}

		long cardinality() const
		{
			return static_cast<long>( Givaro::Montgomery< Givaro::Std32>::size());
		}

		/** Initialization of field base Element from an integer.
		 * Behaves like C++ allocator construct.
		 * This function assumes the output field base Element x has already been
		 * constructed, but that it is not already initialized.
		 * We assume that the type of Element is short int.
		 * this methos is just a simple cast.
		 * @return reference to field base Element.
		 * @param x field base Element to contain output (reference returned).
		 * @param y integer.
		 */
		Element& init(Element& x , const integer& y=0) const
		{
			return Givaro::Montgomery< Givaro::Std32>::init( x,long(y % (integer)_p));
		}

		Element& init(Element& x , const double y) const
		{
		       	return Givaro::Montgomery< Givaro::Std32>::init( x, y);
		}

		/** Conversion of field base element to an integer.
		 * This function assumes the output field base element x has already been
		 * constructed, but that it is not already initialized.
		 * @return reference to an integer.
		 * @param x integer to contain output (reference returned).
		 * @param y constant field base element.
		 */
		integer& convert(integer& x, const Element& y) const
		{
			long tmp;
			//	return x = *(new integer(Montgomery< Givaro::Std32>::convert(tmp,y)));
			return x = integer( Givaro::Montgomery< Givaro::Std32>::convert(tmp,y));
		}

		double& convert(double& x, const Element& y) const
		{
			return Givaro::Montgomery< Givaro::Std32>::convert( x, y);
		}

#if 0 /*  isZero */
		bool isZero(const Element& x) const
		{
		       	return Givaro::Montgomery< Givaro::Std32>::isZero(x);
		}
#endif

		static inline int getMaxModulus() { return 40504; }

	}; // class GivaroMontg



} // namespace LinBox

#endif // __LINBOX_field_givaro_montgomery_H


// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,:0,t0,+0,=s
// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:

