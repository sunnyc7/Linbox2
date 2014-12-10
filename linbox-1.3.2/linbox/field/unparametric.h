
/* linbox/field/unparametric.h
 * Copyright (C) 1999-2005 William J Turner,
 *               2001 Bradford Hovinen
 *               2005 Clement Pernet
 *
 * Written by W. J. Turner <wjturner@acm.org>,
 *            Bradford Hovinen <hovinen@cis.udel.edu>
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

#ifndef __LINBOX_field_unparametric_H
#define __LINBOX_field_unparametric_H
#include <typeinfo>

#include <string>
#include <algorithm>


#include "linbox/linbox-config.h"
#include "linbox/integer.h"
#include "linbox/field/field-interface.h"
#include "linbox/randiter/unparametric.h"
#include "linbox/field/field-traits.h"
#include <fflas-ffpack/field/unparametric.h>
#include "linbox/randiter/nonzero.h"
//#if __LINBOX_HAVE_NTL
//#include "linbox/field/ntl-RR.h"
//#endif // __LINBOX_HAVE_NTL


namespace LinBox
{

    using Givaro::Caster;

#if 0
#if __LINBOX_HAVE_NTL
	typedef NTL::RR Targ;
	template <>
	Targ& Caster<Targ, int> (Targ& t, const int& s)
	{
		return t = s;
	}
#endif // __LINBOX_HAVE_NTL
#endif

	template <class Ring>
	struct ClassifyRing;

	template <class K>
	class UnparametricField;

	// using FFPACK::UnparametricField ;
	using FFPACK::UnparametricOperations;

	template <class K>
	struct ClassifyRing<UnparametricField<K> > {
		typedef RingCategories::GenericTag categoryTag;
	};

	template <>
	struct ClassifyRing<UnparametricField<integer> > {
		typedef RingCategories::IntegerTag categoryTag;
	};


	/** \brief Unparameterized field adapter.
	 * \ingroup field
	 * \defgroup UnparametricField UnparametricField
	 *
	 * A field having an interface similar to that of floats is adapted to
	 * LinBox.
	 *
	 *  Used to generate efficient field classes for unparameterized fields
	 *  (or hidden parameter fields).
	 *
	 *  Some fields are implemented by definition of the C++ arithmetic
	 *  operators, such as <code>z = x*y</code>, for \c x, \c y, \c z
	 *  instances of a type \c K.  The LinBox field LinBox::Unparametric<K>
	 *  is the adaptation to LinBox.
	 *
	 *  For a typical unparametric field, some of the methods must be
	 *  defined in a specialization.
	 */
	template <class K>
	class UnparametricField : public FieldInterface,
		public  FFPACK::UnparametricField<K> {
	protected:
		integer _p; integer _card;
	public:
		typedef typename FFPACK::UnparametricField<K> Father_t ;

		/** @name Common Object Interface for a LinBox Field.
		 * These methods and member types are required of all LinBox fields.
		 * See \ref FieldArchetype  for detailed specifications.
		 */
		//@{

		/** The field's element type.
		 * Type K must provide a default constructor,
		 * a copy constructor, a destructor, and an assignment operator.
		 */

		typedef K Element;

		/// Type of random field element generators.
		typedef UnparametricRandIter<K> RandIter;

		/** @name Field Object Basics.
		*/
		//@{

		/** Builds this field to have characteristic q and cardinality q<sup>e</sup>.
		 *  This constructor must be defined in a specialization.
		 */
		UnparametricField(integer q = 0, size_t e = 1) :
			Father_t(q, e),
			//Father_t((unsigned long)q,(unsigned long)e)
			_p(q),
			_card(q == 0 ?
			integer(-1) :
			pow(q, e) )
			{}  // assuming q is a prime or zero.

		/// construct this field as copy of F.
		UnparametricField (const UnparametricField &F) :
			Father_t(F),_p(F._p), _card(F._card)
		{}


		// field/ntl-ZZ_p.h me les demande... //

		using Father_t::inv ;
		//using Father_t::read ;
		std::istream &read(std::istream & is) { return Father_t::read(is); }

        std::istream &read(std::istream & s, Element &a) const
		{
            Integer tmp;
            s >> tmp;
            init(a, tmp);
            return s;
		}
		using Father_t::invin;
		using Father_t::write;
		using Father_t::isZero;
		using Father_t::isOne;

		template<typename Src>
		Element&init(Element&x, const Src&s) const
		{
			return Caster (x, s);
		}
		Element&init(Element&x)const
		{
			return Caster (x, 0);
		}
		template<typename T>
		T&convert(T&x, const Element&y) const
		{
			return Caster(x,y);
		}

		// fin des trucs zarbs //

		/// c := cardinality of this field (-1 if infinite).
		using Father_t::cardinality ;
		integer &cardinality (integer &c) const
		{
			return c = _card;
		}

		/// c := characteristic of this field (zero or prime).
		using Father_t::characteristic;
		integer &characteristic (integer &c) const
		{
			return c = _p;
		}

		//@} Data Object Management




		//@} Common Object Interface

		/** @name Implementation-Specific Methods.
		 * These methods are not required of all LinBox fields
		 * and are included only for the implementation of this field
		 * template.
		 */
		//@{

		//- Default constructor
		//UnparametricField (void) {}

		/** Constructor from field object.
		 * @param  A unparameterized field object
		 */
		UnparametricField (const K &A) {}

		/** Constant access operator.
		 * @return constant reference to field object
		 */
		const K &operator () (void) const
		{
			return Element ();
		}

		/** Access operator.
		 * @return reference to field object
		 */
		K &operator () (void)
		{
			return Element ();
		}

		//@} Implementation-Specific Methods

	}; // template <class K> class UnparametricField

	template<class Field>
	class FieldAXPY;

	/*! @ingroup integers
	 * @brief NO DOc
	 */
	template<>
	class FieldAXPY<UnparametricField<integer> >  {
	public:
		typedef UnparametricField<integer> Field;
		typedef integer Element;

		/** Constructor.
		 * A faxpy object if constructed from a Field and a field element.
		 * Copies of this objects are stored in the faxpy object.
		 * @param F field F in which arithmetic is done
		 */
		FieldAXPY (const Field &F) :
			_field (F)
		{ _y = 0; }

		/** Copy constructor.
		 * @param faxpy
		 */
		FieldAXPY (const FieldAXPY<Field> &faxpy) :
			_field (faxpy._field), _y (faxpy._y)
		{}

		/** Assignment operator
		 * @param faxpy
		 */
		FieldAXPY<Field> &operator = (const FieldAXPY &faxpy)
		{ _y = faxpy._y; return *this; }

		/** Add a*x to y
		 * y += a*x.
		 * @param a constant reference to element a
		 * @param x constant reference to element x
		 * allow optimal multiplication, such as integer * int
		 */
		template<class Element1>
		inline Element&  mulacc (const Element &a, const Element1 &x)
		{
			return _y += (a * x);
		}

		template<class Element1>
		inline Element& accumulate (const Element1 &t)
		{
			return _y += t;
		}

		/** Retrieve y
		 *
		 * Performs the delayed modding out if necessary
		 */
		inline Element &get (Element &y) { y = _y; return y; }

		/** Assign method.
		 * Stores new field element for arithmetic.
		 * @return reference to self
		 * @param y constant reference to element a
		 */
		inline FieldAXPY &assign (const Element& y)
		{
			_y = y;
			return *this;
		}

		inline void reset() {
			_y = 0;
		}

	private:

		/// Field in which arithmetic is done
		Field _field;

		/// Field element for arithmetic
		Element _y;

	};

} // namespace LinBox

#include "linbox/randiter/unparametric.h"

#endif // __LINBOX_field_unparametric_H


// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:
// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,\:0,t0,+0,=s

