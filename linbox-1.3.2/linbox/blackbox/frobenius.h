/* linobx/blackbox/frobenuis.h
 * Copyright(c) 2010 LinBox
 * Written by Austin Lobo <alobo@cis.udel.edu> and
 *            B.D. Saunders <saunders@cis.udel.edu>
 *  ========LICENCE========
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
 */

/*! @file blackbox/frobenius.h
 * @ingroup blackbox
 * @brief NO DESC
 */

#ifndef __LINBOX_frobenius_H
#define __LINBOX_frobenius_H

#include "linbox/blackbox/blackbox-interface.h"
#include "linbox/blackbox/companion.h"
#include "linbox/blackbox/direct-sum.h"
#include <vector>

namespace LinBox
{
	/// \ingroup blackbox
	template <class _Field>
	class Frobenius: public BlackboxInterface, public DirectSum<Companion<_Field> > {
	public:
		Frobenius() { }   // default constructor

		/** Constructor.
		 *    Build a matrix in Frobenius form whose block sizes are
		 *    specified by vlist, generated from random polynomials
		 *    @param F
		 *    @param vlist diagonal-block sizes, positive ints in non-increasing order
		 */
		template <class VDegList>
		Frobenius( const _Field &F, const VDegList &vlist)
		{ }

		/** Constructor.
		 *    Build a square, block-diagonal matrix as a direct sum of the companion
		 *    matrices of the polynomials. The dimension is the sum of the degrees.
		 *    @param pbegin iterator pointing to the start of a list of polynomials
		 *    @param F
		 *    @param pend   iterator pointing after end   of a list of polynomials
		 */
		template <class PolyIterator>
		Frobenius( const _Field &F, PolyIterator pbegin, PolyIterator pend)
		{
			this->_VB.resize(pend - pbegin);
			PolyIterator pp = pbegin;
			typename std::vector<const Companion<_Field>* >::iterator vp;
			this->m = 0;
			this->n = 0;
			for(vp = this->_VB.begin(); vp != this->_VB.end(); ++vp,++pp)  {
				*vp = new  Companion<_Field>(F,*pp);
				this->m += (*vp) -> rowdim();
				this->n += (*vp) -> coldim();
			}
		}


		/// destructor
		~Frobenius()
		{
			typename std::vector< const Companion<_Field>* >::iterator vp;
			for(vp = this->_VB.begin(); vp != this->_VB.end(); ++vp)
				delete (*vp);
		}


		template<typename _Tp1>
		struct rebind {
			typedef Frobenius<_Tp1> other;
		};



	}; // class Frobenius

}// Namespace LinBox

#endif //__LINBOX_frobenius_H


// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,:0,t0,+0,=s
// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:

