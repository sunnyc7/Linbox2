/* linbox/field/givaro-gfq.h
 * Copyright (C) 2005 JGD
 *
 * Time-stamp: <22 Jun 10 10:02:34 Jean-Guillaume.Dumas@imag.fr>
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

/*! @file field/Givaro/givaro-extension.h
 * @ingroup field
 * @brief NO DOC
 */

#ifndef __LINBOX_field_givaro_extension_H
#define __LINBOX_field_givaro_extension_H


#include "linbox/linbox-config.h"
#include "linbox/integer.h"
#include "linbox/field/field-traits.h"
#include "linbox/field/field-interface.h"
#include "linbox/util/debug.h"
#include "linbox/field/field-traits.h"
#include "linbox/field/Givaro/givaro-gfq.h"

#ifdef __LINBOX_XMLENABLED

#include "linbox/util/xml/linbox-reader.h"
#include "linbox/util/xml/linbox-writer.h"

#include <iostream>
#include <string>
#include <vector>

#endif //__LINBOX_XMLENABLED

//---------------------------------------------
// Files of Givaro library
#include <givaro/givextension.h>
#include <givaro/giv_randiter.h>
//---------------------------------------------
// To convert linbox fields to Givaro interface
#include "linbox/field/Givaro/givaro-field.h"

//---------------------------------------------
// Namespace in which all LinBox code resides
namespace LinBox
{

	template <class Ring>
	struct ClassifyRing;

	template< class BaseField>
	class GivaroExtension;

// #if !defined(__INTEL_COMPILER) && !defined(__CUDACC__) && !defined(__clang__)
	// template<>
// #endif
	template< class BaseField>
	struct ClassifyRing<GivaroExtension<BaseField> > {
		typedef RingCategories::ModularTag categoryTag;
	};

// #if !defined(__INTEL_COMPILER) && !defined(__CUDACC__) && !defined(__clang__)
	// template<>
// #endif
	template< class BaseField>
	struct FieldTraits< GivaroExtension<BaseField> > {
		typedef RingCategories::ModularTag categoryTag;

		static integer& maxModulus( integer& i )
		{
			return  FieldTraits<BaseField>::maxModulus(i);
		}

		static bool goodModulus( const integer& i )
		{
			return  FieldTraits<BaseField>::goodModulus(i);
		}

		// After that degree might not be correct ...
		static integer& maxExponent( integer& i )
		{
			return i = 2147483648UL;
		}
		static bool goodExponent( const integer& i )
		{
			integer max;
			return ( i >= 1 && i <= maxExponent( max ) );
		}
	};


	/** This template class is defined to be in phase with the LinBox
	 *  archetype.
	 *  Most of all methods are inherited from Extension  class
	 *  of Givaro.
	 *  These class allow to construct only extension field with a prime characteristic.
	 */
	template< class BaseField = LinBox::GivaroGfq>
	class GivaroExtension : public Givaro::Extension<GivaroField<BaseField> >, public FieldInterface {

		typedef GivaroExtension<GivaroField<BaseField> > Self_t;
		typedef Givaro::Extension<GivaroField<BaseField> >       Extension_t;
	public:
		/** Element type.
		 *  This type is inherited from the Givaro class Extension
		 */
		typedef typename Givaro::Extension<GivaroField<BaseField> >::Element Element;

		typedef Givaro::             Extension<GivaroField<BaseField> > Father_t;

		using Extension_t::zero;
		using Extension_t::one;
		using Extension_t::mOne;

		/** RandIter type.
		 *  This type is inherited from the Givaro class GFqDom<TAG>
		 */
		typedef Givaro::GIV_ExtensionrandIter< Givaro::Extension<GivaroField<BaseField> >, LinBox::integer >  RandIter;


		GivaroExtension()
		{}


		/** Constructor from an integer.
		*/
		GivaroExtension(const integer& p, const integer& k=1) :
		 Givaro::Extension<GivaroField<BaseField> >(static_cast<typename Givaro::Extension< GivaroField< BaseField > >::Residu_t >(int32_t(p)), static_cast<typename Givaro::Extension< GivaroField< BaseField > >::Residu_t>(int32_t(k)))
		{
		}

		/** Constructor extension of a base field.
		*/
		GivaroExtension(const BaseField& bF, const integer& ext=1) :
		 Givaro::Extension<GivaroField<BaseField> >( GivaroField<BaseField>(bF),
								      static_cast<typename Givaro::Extension< GivaroField< BaseField > >::Residu_t>(int32_t(ext)))
		{
		}


		/** Copy Constructor
		*/
		GivaroExtension(const Self_t& F) :
		 Givaro::Extension<GivaroField<BaseField> >(F) {
			}


		using Givaro::Extension<GivaroField<BaseField> >::               characteristic ;
#if (GIVARO_VERSION<30403)
		long unsigned int & characteristic(long unsigned int & c) const
		{
			return  c = (long unsigned)Givaro::Extension<GivaroField<BaseField> >::characteristic() ;
		}
#else
		long unsigned int & characteristic(long unsigned int & c) const
		{
			return  Givaro::Extension<GivaroField<BaseField> >::characteristic(c) ;
		}
#endif

	}; // class GivaroExtension



	/** This template class is define just to be in phase with the LinBox
	 *  archetype.
	 *  Most of all methods are inherited from Extension  class
	 *  of Givaro.
	 *  these class allow to construct only extension field with a prime characteristic.
	 */
// #ifndef __INTEL_COMPILER
	template<>
// #endif
	class GivaroExtension<LinBox::GivaroGfq> : public Givaro::Extension< Givaro::GFqDom<int32_t> >, public FieldInterface {

		typedef GivaroExtension<GivaroGfq> Self_t;
		typedef Givaro::Extension< Givaro::GFqDom<int32_t>  >       Extension_t;
	public:

		/** Element type.
		 *  This type is inherited from the Givaro class Extension
		 */
		typedef Givaro::Extension< Givaro::GFqDom<int32_t> >::Element Element;
		typedef Givaro::Extension< Givaro::GFqDom<int32_t> > Father_t;

		using Extension_t::zero;
		using Extension_t::one;
		using Extension_t::mOne;


		/** RandIter type.
		 *  This type is inherited from the Givaro class GFqDom<TAG>
		 */
		typedef Givaro::GIV_ExtensionrandIter< Givaro::Extension< Givaro::GFqDom<int32_t> >, LinBox::integer >  RandIter;

		/** Constructor from an integer.
		*/
		GivaroExtension(const integer& p, const integer& k=1) :
		 Givaro::Extension< Givaro::GFqDom<int32_t> >(static_cast< Givaro::Extension< Givaro::GFqDom<int32_t> >::Residu_t>(int32_t(p)),
						  static_cast< Givaro::Extension< Givaro::GFqDom<int32_t> >::Residu_t>(int32_t(k)))
		{
		}

		/** Constructor extension of a base field.
		*/
		GivaroExtension(const GivaroGfq& bF, const integer& ext=1) :
		 Givaro::Extension< Givaro::GFqDom<int32_t> >( static_cast< const Givaro::Extension< Givaro::GFqDom< int32_t > >::BaseField_t &>(bF),
						   static_cast< Givaro::Extension< Givaro::GFqDom<int32_t> >::Residu_t >(int32_t(ext)))
		{
		}


		/** Copy Constructor.
		*/
		GivaroExtension(const Self_t& F) :
		 Givaro::Extension< Givaro::GFqDom<int32_t> >(F)
		{ }


	}; // class GivaroExtension



} // namespace LinBox



// Specialization of homomorphism for basefield
#include "linbox/field/hom.h"
namespace LinBox
{
	///  NO DOC
	template< class BaseField>
	class Hom < BaseField, GivaroExtension<BaseField> > {
		typedef BaseField Source;
		typedef GivaroExtension<BaseField> Target;
	public:
		typedef typename Source::Element SrcElt;
		typedef typename Target::Element Elt;

		//Hom(){}
		/** Constructor.
		 * Construct a homomorphism from a specific source ring S and target
		 * field T with Hom(S, T).  The default behaviour is error.
		 * Specializations define all actual homomorphisms.
		 */
		Hom(const Source& S, const Target& T) :
			_source(S), _target(T)
		{}

		/** Image.
		 * image(t, s) implements the homomorphism, assigning the
		 * t the value of the image of s under the mapping.
		 *
		 * The default behaviour is a no-op.
		 */
		Elt& image(Elt& t, const SrcElt& s) const
		{
			return _target.assign(t, s);
		}

		/** Preimage.
		 * If possible, preimage(s,t) assigns a value to s such that
		 * the image of s is t.  Otherwise behaviour is unspecified.
		 * An error may be thrown, a conventional value may be set, or
		 * an arb value set.
		 *
		 * The default behaviour is a no-op.
		 */
		SrcElt& preimage(SrcElt& s, const Elt& t) const
		{
			//                     return _target.getEntry(s, Degree(0), t);
			return _target.convert(s, t);
		}

		const Source& source() const
		{
			return _source;
		}
		const Target& target() const
		{
			return _target;
		}

	private:
		Source _source;
		Target _target;
	}; // end Hom
}
#endif // __LINBOX_field_givaro_extension_H


// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,:0,t0,+0,=s
// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:

