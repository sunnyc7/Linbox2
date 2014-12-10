/* Copyright (C) 2010 LinBox
 * Adapted by B Boyer <brice.boyer@imag.fr>
 * (from other modular-balanced* files)
 *
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * ========LICENCE========
 */


/*! @file field/Modular/modular-balanced-int64.h
 * @ingroup field
 * @brief Balanced representation of <code>Z/mZ</code> over \c int64_t .
 */

#ifndef __LINBOX_modular_balanced_int64_H
#define __LINBOX_modular_balanced_int64_H

#include "linbox/linbox-config.h"
#include "linbox/integer.h"
#include "linbox/vector/vector-domain.h"
#include "linbox/field/field-interface.h"
#include "linbox/field/field-traits.h"
#include "linbox/util/field-axpy.h"
#include "linbox/util/debug.h"
#include "linbox/field/field-traits.h"

#include <fflas-ffpack/field/modular-balanced-int64.h>

#ifndef LINBOX_MAX_INT64
#ifdef __x86_64__
#define LINBOX_MAX_INT64 INT64_MAX
#else
#define LINBOX_MAX_INT64 INT64_MAX
#endif
#endif


// Namespace in which all LinBox code resides
namespace LinBox
{

	template< class Element >
	class ModularBalanced;
	template< class Element >
	class ModularBalancedRandIter;

	template <class Ring>
	struct ClassifyRing;

	template<class Element>
	struct ClassifyRing<ModularBalanced<Element> >;

	template<>
	struct ClassifyRing<ModularBalanced<int64_t> > {
		typedef RingCategories::ModularTag categoryTag;
	};

	/// \ingroup field
	template <>
	class ModularBalanced<int64_t> : public FieldInterface
	      public FFPACK::ModularBalanced<int64_t>	{

	public:

		friend class FieldAXPY<ModularBalanced<int64_t> >;
		friend class DotProductDomain<ModularBalanced<int64_t> >;

		typedef FFPACK::ModularBalanced<int64_t> Father_t ;

		typedef int64_t Element;
		typedef ModularBalancedRandIter<int64_t> RandIter;

		using Father_t:: cardinality;
		integer &cardinality (integer &c) const
		{
			return c = modulus;
		}

		using Father_t:: characteristic;
		integer &characteristic (integer &c) const
		{
		       	return c = modulus;
		}

		using Father_t:: convert;
		// this function converts an int to a natural number ?
		integer &convert (integer &x, const Element &y) const
		{
			if(y >= 0)
				return x = y;
			else
				return x = y + modulus;
		}

		using Father_t:: init;
		Element &init (Element &x, const integer &y) const
		{
			x = y % (long) (modulus);
			if (x < mhalf_mod) x += modulus;
			else if (x > half_mod) x -= modulus;
			return x;
		}

		Element &init(Element &x) const
		{
			return x = 0 ;
		}

	private:

	};

	template <>
	class FieldAXPY<ModularBalanced<int64_t> > {
	public:

		typedef int64_t Element;
		typedef ModularBalanced<int64_t> Field;

		FieldAXPY (const Field &F) :
			_field (F),_y(0),_times(0)
		{
		}


		FieldAXPY (const FieldAXPY &faxpy) :
			_field (faxpy._field), _y (0),_times(0)
		{}

		FieldAXPY<ModularBalanced<int64_t> > &operator = (const FieldAXPY &faxpy)
		{
			_field = faxpy._field;
			_y = faxpy._y;
			_times = faxpy._times;
			return *this;
		}

		inline int64_t& mulacc (const Element &a, const Element &x)
		{
			int64_t t = (int64_t) a * (int64_t)   x;
			if (_times < blocksize)
			{
				++_times;
				return _y += t;
			}

			else {
				_times = 1;
				normalize();
				return _y += t;
			}
		}

		inline int64_t& accumulate (const Element &t)
		{
			if (_times < blocksize)
			{
				++_times;
				return _y += t;
			}

			else {
				_times = 1;
				normalize();
				return _y += t;
			}
		}

		inline Element& get (Element &y)
		{

			normalize();

			y = _y;

			if (y > _field.half_mod)
				y -= _field.modulus;
			else if (y < _field.mhalf_mod)
				y += _field.modulus;

			return y;
		}

		inline FieldAXPY &assign (const Element y)
		{
			_y = y;
			return *this;
		}

		inline void reset()
		{
			_y = 0;
		}

	private:

		Field _field;
		int64_t _y;
		int64_t _times;
		//!@todo tune me ?
		static const int64_t blocksize = 32;

		inline void normalize()
		{
			_y = (int64_t)_y -(int64_t)(int64_t)((double) _y * _field.modulusinv) * (int64_t)_field.modulus;
		}

	};


	template <>
	class DotProductDomain<ModularBalanced<int64_t> > : private virtual VectorDomainBase<ModularBalanced<int64_t> > {

	private:
		const int64_t blocksize;

	public:
		typedef int64_t Element;
		DotProductDomain (const ModularBalanced<int64_t> &F) :
			VectorDomainBase<ModularBalanced<int64_t> > (F) ,blocksize(32)
		{ }

	protected:
		template <class Vector1, class Vector2>
		inline Element &dotSpecializedDD (Element &res, const Vector1 &v1, const Vector2 &v2) const
		{

			typename Vector1::const_iterator pv1,pv1e;
			typename Vector2::const_iterator pv2;

			int64_t y = 0;
			int64_t t;
			int64_t times = 0;

			pv1 = pv1e = v1.begin();
			pv2 = v2.begin();

			for(int i = 0; i < v1.size() / blocksize ;++i)
			{
				pv1e = pv1e + blocksize;
				for(;pv1 != pv1e;++pv1,++pv2)
				{
					t = (((int64_t) *pv1 ) * ((int64_t) *pv2 ));
					y += t;
				}
				normalize(y);
			}

			for(;pv1 != v1.end(); ++pv1, ++pv2)
			{
				t = (((int64_t) *pv1 ) * ((int64_t) *pv2 ));
				y += t;
			}

			normalize(y);
			res = y;

			if (res > _field.half_mod) res -= _field.modulus;
			else if(res < _field.mhalf_mod) res += _field.modulus;

			return res;

		}

		template <class Vector1, class Vector2>
		inline Element &dotSpecializedDSP (Element &res, const Vector1 &v1, const Vector2 &v2) const
		{

			typename Vector1::first_type::const_iterator i_idx, i_idxe;
			typename Vector1::second_type::const_iterator i_elt;

			int64_t y = 0;
			int64_t t;

			i_idx = i_idxe = v1.first.begin();
			i_elt = v1.second.begin();

			for(int i = 0; i < v1.first.size() / blocksize ; ++i)
			{
				i_idxe = i_idxe + blocksize;
				for(;i_idx!= i_idxe;++i_idx, ++i_elt)
				{
					t = ( (int64_t) *i_elt ) * ( (int64_t) v2[*i_idx] );
					y += t;
				}
				normalize(y);
			}


			for(;i_idx!= v1.first.end();++i_idx, ++i_elt)
			{
				t = ( (int64_t) *i_elt ) * ( (int64_t) v2[*i_idx] );
				y += t;
			}

			normalize(y);

			res = y;
			if (res > _field.half_mod) res -= _field.modulus;
			else if(res < _field.mhalf_mod) res += _field.modulus;

			return res;
		}

		inline void normalize(int64_t& _y) const
		{
			_y = (int64_t)_y -(int64_t)(int64_t)((double) _y * _field.modulusinv) * (int64_t)_field.modulus;
		}

	};
}

#undef LINBOX_MAX_INT64

#include "linbox/randiter/modular.h" // do not unse _LB_MAX inside this one !


#endif //__LINBOX_modular_balanced_int64_H


// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,:0,t0,+0,=s
// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:

