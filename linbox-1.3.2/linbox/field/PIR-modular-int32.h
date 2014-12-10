/* Copyright (C) 2010 LinBox
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


#ifndef __LINBOX_pir_modular_int32_H
#define __LINBOX_pir_modular_int32_H

#include "linbox/field/modular.h"
#ifndef LINBOX_MAX_INT
#define LINBOX_MAX_INT 2147483647
#endif

#ifndef LINBOX_MAX_MODULUS
#define LINBOX_MAX_MODULUS 1073741824
#endif
#include "linbox/field/field-traits.h"

// Namespace in which all LinBox code resides
namespace LinBox
{

	template< class Element>
	class PIRModular;

	template< class Element >
	class ModularRandIter;

	template<class Field>
	class DotProductDomain;

	template<class Field>
	class FieldAXPY;

	template<class Field>
	class MVProductDomain;

	template <class Ring>
	struct ClassifyRIng;

	template <class Element>
	struct ClassifyRIng<PIRModular<Element> >;

	template <>
	struct ClassifyRIng<PIRModular<int32_t> >  {
		typedef RingCategories::ModularTag categoryTag;
	};

	/// \ingroup ring
	template <>
	class PIRModular<int32_t> : public Modular<int32_t> {

	public:

		friend class FieldAXPY<PIRModular<int32_t> >;

		friend class DotProductDomain<PIRModular<int32_t> >;

		friend class MVProductDomain<PIRModular<int32_t> >;

		typedef int32_t Element;

		typedef ModularRandIter<int32_t> RandIter;

		//default modular field,taking 65521 as default modulus
		PIRModular () :
			Modular<int32_t>(65521)
		{
		}

		PIRModular (int32_t value, int32_t exp = 1) :
			Modular<int32_t>(value,exp)
		{ }


		/** PIR functions, gcd, xgcd, dxgcd */

		Element& gcd (Element& g, const Element& a, const Element& b) const
		{

			GCD (g, a, b);

			return g;

		}

		Element& xgcd (Element& g, Element& s, Element& t, const Element& a, const Element& b) const
		{

			XGCD (g, s, t, a, b);

			if (s < 0)
				s += modulus;

			if (t < 0)
				t += modulus;


			return g;
		}

		Element& dxgcd (Element& g, Element& s, Element& t, Element& a1, Element& b1,
				const Element& a, const Element& b) const
		{


			xgcd (g, s, t, a, b);

			if (g != 0) {

				a1 = a / g;

				b1 = b / g;
			}

			else {

				a1 = s;

				b1 = t;
			}


			return g;

		}

		bool isDivisor (const Element& a, const Element& b) const
		{

			Element g;

			if (a == 0) return false;

			else if (b == 0) return true;

			else {

				gcd (g, a, modulus);

				return (b % g) == 0;
			}

		}

		Element& div (Element& d, const Element& a, const Element& b) const
		{

			Element g, s;

			HXGCD (g, s, b, modulus);

			Element r;

			r = a % g;

			if (r != 0) throw PreconditionFailed(__func__,__FILE__,__LINE__,"Div: not dividable");

			else {

				d = a / g;

				mulin (d, s);
			}

			return d;

		}

		Element& normal (Element& a, const Element& b) const
		{

			if (b == 0) return a = 0;
			else {
				GCD (a, b, modulus);

				return a;
			}
		}


		Element& gcdin (Element& a, const Element& b) const
		{

			GCD (a, a, b);


			return a;
		}

		Element& normalIn (Element& a) const
		{
			if (a == 0) return a;
			else {
				GCD (a, a, modulus);

				return a;
			}

		}


		Element& divin (Element& a, const Element& b) const
		{

			div (a, a, b);

			return a;
		}


		bool isUnit(const Element& a) const
		{

			Element g;

			GCD (g, a, modulus);


			//	std::cout << a << " is a unit or not " << g;

			//		std::cout << "modulus = " << modulus <<"\n";

			return g == 1;

		}

	private:
		static void GCD (int32_t& g, int32_t a, int32_t b) {

			int32_t  u, v, /*  q,*/ r;

			if (a < 0) {
				if (a < -LINBOX_MAX_INT) throw PreconditionFailed(__func__,__FILE__,__LINE__,"XGCD: integer overflow");
				a = -a;

			}

			if (b < 0) {
				if (b < -LINBOX_MAX_INT) throw PreconditionFailed(__func__,__FILE__,__LINE__,"XGCD: integer overflow");
				b = -b;
			}

			u = a; v = b;

			while (v != 0) {
				// q = u / v;
				r = u % v;
				u = v;
				v = r;
			}

			g = u;

		}

		static void XGCD(int32_t& d, int32_t& s, int32_t& t, int32_t a, int32_t b) {
			int32_t  u, v, u0, v0, u1, v1, u2, v2, q, r;

			int32_t aneg = 0, bneg = 0;

			if (a < 0) {
				if (a < -LINBOX_MAX_INT) throw PreconditionFailed(__func__,__FILE__,__LINE__,"XGCD: integer overflow");
				a = -a;
				aneg = 1;
			}

			if (b < 0) {
				if (b < -LINBOX_MAX_INT) throw PreconditionFailed(__func__,__FILE__,__LINE__,"XGCD: integer overflow");
				b = -b;
				bneg = 1;
			}

			u1 = 1; v1 = 0;
			u2 = 0; v2 = 1;
			u = a; v = b;

			while (v != 0) {
				q = u / v;
				r = u % v;
				u = v;
				v = r;
				u0 = u2;
				v0 = v2;
				u2 =  u1 - q*u2;
				v2 = v1- q*v2;
				u1 = u0;
				v1 = v0;
			}

			if (aneg)
				u1 = -u1;

			if (bneg)
				v1 = -v1;

			d = u;
			s = u1;
			t = v1;
		}


		static void HXGCD (int32_t& d, int32_t& s, int32_t a, int32_t b) {

			int32_t  u, v, u0, u1, u2, q, r;

			int32_t aneg = 0;

			if (a < 0) {
				if (a < -LINBOX_MAX_INT) throw PreconditionFailed(__func__,__FILE__,__LINE__,"XGCD: integer overflow");
				a = -a;
				aneg = 1;
			}

			if (b < 0) {
				if (b < -LINBOX_MAX_INT) throw PreconditionFailed(__func__,__FILE__,__LINE__,"XGCD: integer overflow");
				b = -b;
			}

			u1 = 1;
			u2 = 0;
			u = a; v = b;

			while (v != 0) {
				q = u / v;
				r = u % v;
				u = v;
				v = r;
				u0 = u2;

				u2 =  u1 - q*u2;

				u1 = u0;

			}

			if (aneg)
				u1 = -u1;


			d = u;
			s = u1;

		}

	};

	template <>
	class FieldAXPY<PIRModular<int32_t> > {
	public:

		typedef int32_t Element;
		typedef PIRModular<int32_t> Field;

		FieldAXPY (const Field &F) :
			_field (F),_y(0)
		{}


		FieldAXPY (const FieldAXPY &faxpy) :
			_field (faxpy._field), _y (0)
		{}

		FieldAXPY<PIRModular<int32_t> > &operator = (const FieldAXPY &faxpy) {
			_field = faxpy._field;
			_y = faxpy._y;
			return *this;
		}

		inline uint64_t& mulacc (const Element &a, const Element &x) {
			uint64_t t = (uint64_t) a * (uint64_t) x;
			_y += t;
			if (_y < t)
				return _y += _field._two64;
			else
				return _y;
		}

		inline uint64_t& accumulate (const Element &t) {
			_y += t;
			if (_y < (uint64_t)t)
				return _y += _field._two64;
			else
				return _y;
		}

		inline Element& get (Element &y) {
			y = Element(_y % (uint64_t) _field.modulus);
			return y;
		}

		inline FieldAXPY &assign (const Element y) {
			_y = y;
			return *this;
		}

		inline void reset() {
			_y = 0;
		}


	protected:
		Field _field;
		uint64_t _y;
	};


	template <>
	class DotProductDomain<PIRModular<int32_t> > : public virtual VectorDomainBase<PIRModular<int32_t> > {

	public:
		typedef int32_t Element;
		DotProductDomain (const PIRModular<int32_t> &F) :
			VectorDomainBase<PIRModular<int32_t> > (F)
		{}


	protected:
		template <class Vector1, class Vector2>
		inline Element &dotSpecializedDD (Element &res, const Vector1 &v1, const Vector2 &v2) const
		{

			typename Vector1::const_iterator i;
			typename Vector2::const_iterator j;

			uint64_t y = 0;
			uint64_t t;

			for (i = v1.begin (), j = v2.begin (); i < v1.end (); ++i, ++j) {
				t = ( (uint64_t) *i ) * ( (uint64_t) *j );
				y += t;

				if (y < t)
					y += _field._two64;
			}

			y %= (uint64_t) _field.modulus;
			return res = Element(y);

		}

		template <class Vector1, class Vector2>
		inline Element &dotSpecializedDSP (Element &res, const Vector1 &v1, const Vector2 &v2) const
		{
			typename Vector1::first_type::const_iterator i_idx;
			typename Vector1::second_type::const_iterator i_elt;

			uint64_t y = 0;
			uint64_t t;

			for (i_idx = v1.first.begin (), i_elt = v1.second.begin (); i_idx != v1.first.end (); ++i_idx, ++i_elt) {
				t = ( (uint64_t) *i_elt ) * ( (uint64_t) v2[*i_idx] );
				y += t;

				if (y < t)
					y += _field._two64;
			}


			y %= (uint64_t) _field.modulus;

			return res = (Element)y;
		}
	};


	// Specialization of MVProductDomain for int32_t modular field
	template <>
	class MVProductDomain<PIRModular<int32_t> > {
	public:

		typedef int32_t Element;

	protected:
		template <class Vector1, class Matrix, class Vector2>
		inline Vector1 &mulColDense
		(const VectorDomain<PIRModular<int32_t> > &VD, Vector1 &w, const Matrix &A, const Vector2 &v) const
		{
			return mulColDenseSpecialized
			(VD, w, A, v, typename VectorTraits<typename Matrix::Column>::VectorCategory ());
		}

	private:
		template <class Vector1, class Matrix, class Vector2>
		Vector1 &mulColDenseSpecialized
		(const VectorDomain<PIRModular<int32_t> > &VD, Vector1 &w, const Matrix &A, const Vector2 &v,
		 VectorCategories::DenseVectorTag) const;
		template <class Vector1, class Matrix, class Vector2>
		Vector1 &mulColDenseSpecialized
		(const VectorDomain<PIRModular<int32_t> > &VD, Vector1 &w, const Matrix &A, const Vector2 &v,
		 VectorCategories::SparseSequenceVectorTag) const;
		template <class Vector1, class Matrix, class Vector2>
		Vector1 &mulColDenseSpecialized
		(const VectorDomain<PIRModular<int32_t> > &VD, Vector1 &w, const Matrix &A, const Vector2 &v,
		 VectorCategories::SparseAssociativeVectorTag) const;
		template <class Vector1, class Matrix, class Vector2>
		Vector1 &mulColDenseSpecialized
		(const VectorDomain<PIRModular<int32_t> > &VD, Vector1 &w, const Matrix &A, const Vector2 &v,
		 VectorCategories::SparseParallelVectorTag) const;

		mutable std::vector<uint64_t> _tmp;
	};

	template <class Vector1, class Matrix, class Vector2>
	Vector1 &MVProductDomain<PIRModular<int32_t> >::mulColDenseSpecialized
	(const VectorDomain<PIRModular<int32_t> > &VD, Vector1 &w, const Matrix &A, const Vector2 &v,
	 VectorCategories::DenseVectorTag) const
	{

		linbox_check (A.coldim () == v.size ());
		linbox_check (A.rowdim () == w.size ());

		typename Matrix::ConstColIterator i = A.colBegin ();
		typename Vector2::const_iterator j;
		typename Matrix::Column::const_iterator k;
		std::vector<uint64_t>::iterator l;

		uint64_t t;

		if (_tmp.size () < w.size ())
			_tmp.resize (w.size ());

		std::fill (_tmp.begin (), _tmp.begin () + w.size (), 0);

		for (j = v.begin (); j != v.end (); ++j, ++i) {
			for (k = i->begin (), l = _tmp.begin (); k != i->end (); ++k, ++l) {
				t = ((uint64_t) *k) * ((uint64_t) *j);

				*l += t;

				if (*l < t)
					*l += VD.field ()._two64;
			}
		}

		typename Vector1::iterator w_j;

		for (w_j = w.begin (), l = _tmp.begin (); w_j != w.end (); ++w_j, ++l)
			*w_j = *l % VD.field ().modulus;

		return w;
	}

	template <class Vector1, class Matrix, class Vector2>
	Vector1 &MVProductDomain<PIRModular<int32_t> >::mulColDenseSpecialized
	(const VectorDomain<PIRModular<int32_t> > &VD, Vector1 &w, const Matrix &A, const Vector2 &v,
	 VectorCategories::SparseSequenceVectorTag) const
	{
		linbox_check (A.coldim () == v.size ());
		linbox_check (A.rowdim () == w.size ());

		typename Matrix::ConstColIterator i = A.colBegin ();
		typename Vector2::const_iterator j;
		typename Matrix::Column::const_iterator k;
		std::vector<uint64_t>::iterator l;

		uint64_t t;

		if (_tmp.size () < w.size ())
			_tmp.resize (w.size ());

		std::fill (_tmp.begin (), _tmp.begin () + w.size (), 0);

		for (j = v.begin (); j != v.end (); ++j, ++i) {
			for (k = i->begin (), l = _tmp.begin (); k != i->end (); ++k, ++l) {
				t = ((uint64_t) k->second) * ((uint64_t) *j);

				_tmp[k->first] += t;

				if (_tmp[k->first] < t)
					_tmp[k->first] += VD.field ()._two64;
			}
		}

		typename Vector1::iterator w_j;

		for (w_j = w.begin (), l = _tmp.begin (); w_j != w.end (); ++w_j, ++l)
			*w_j = *l % VD.field ().modulus;

		return w;
	}

	template <class Vector1, class Matrix, class Vector2>
	Vector1 &MVProductDomain<PIRModular<int32_t> >::mulColDenseSpecialized
	(const VectorDomain<PIRModular<int32_t> > &VD, Vector1 &w, const Matrix &A, const Vector2 &v,
	 VectorCategories::SparseAssociativeVectorTag) const
	{

		linbox_check (A.coldim () == v.size ());
		linbox_check (A.rowdim () == w.size ());

		typename Matrix::ConstColIterator i = A.colBegin ();
		typename Vector2::const_iterator j;
		typename Matrix::Column::const_iterator k;
		std::vector<uint64_t>::iterator l;

		uint64_t t;

		if (_tmp.size () < w.size ())
			_tmp.resize (w.size ());

		std::fill (_tmp.begin (), _tmp.begin () + w.size (), 0);

		for (j = v.begin (); j != v.end (); ++j, ++i) {
			for (k = i->begin (), l = _tmp.begin (); k != i->end (); ++k, ++l) {
				t = ((uint64_t) k->second) * ((uint64_t) *j);

				_tmp[k->first] += t;

				if (_tmp[k->first] < t)
					_tmp[k->first] += VD.field ()._two64;
			}
		}

		typename Vector1::iterator w_j;

		for (w_j = w.begin (), l = _tmp.begin (); w_j != w.end (); ++w_j, ++l)
			*w_j = *l % VD.field ().modulus;

		return w;
	}

	template <class Vector1, class Matrix, class Vector2>
	Vector1 &MVProductDomain<PIRModular<int32_t> >::mulColDenseSpecialized
	(const VectorDomain<PIRModular<int32_t> > &VD, Vector1 &w, const Matrix &A, const Vector2 &v,
	 VectorCategories::SparseParallelVectorTag) const
	{

		linbox_check (A.coldim () == v.size ());
		linbox_check (A.rowdim () == w.size ());

		typename Matrix::ConstColIterator i = A.colBegin ();
		typename Vector2::const_iterator j;
		typename Matrix::Column::first_type::const_iterator k_idx;
		typename Matrix::Column::second_type::const_iterator k_elt;
		std::vector<uint64_t>::iterator l;

		uint64_t t;

		if (_tmp.size () < w.size ())
			_tmp.resize (w.size ());

		std::fill (_tmp.begin (), _tmp.begin () + w.size (), 0);

		for (j = v.begin (); j != v.end (); ++j, ++i) {
			for (k_idx = i->first.begin (), k_elt = i->second.begin (), l = _tmp.begin ();
			     k_idx != i->first.end ();
			     ++k_idx, ++k_elt, ++l)
			{
				t = ((uint64_t) *k_elt) * ((uint64_t) *j);

				_tmp[*k_idx] += t;

				if (_tmp[*k_idx] < t)
					_tmp[*k_idx] += VD.field ()._two64;
			}
		}

		typename Vector1::iterator w_j;

		for (w_j = w.begin (), l = _tmp.begin (); w_j != w.end (); ++w_j, ++l)
			*w_j = *l % VD.field ().modulus;

		return w;
	}


}

#include "linbox/randiter/modular.h"
#endif //__LINBOX_pir_modular_int32_H


// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,:0,t0,+0,=s
// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:

