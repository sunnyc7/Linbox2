/* linbox/algorithms/gauss-gf2.h
 * Copyright (C) 2009 The LinBox group
 * Written by JG Dumas
 *
 * Time-stamp: <23 Mar 12 17:28:19 Jean-Guillaume.Dumas@imag.fr>
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
 *
 * SparseSeqMatrix is container< container< size_t > >
 * as e.g. linbox/blackbox/zo-gf2.h
 */

#ifndef __LINBOX_gauss_gf2_H
#define __LINBOX_gauss_gf2_H

#include "linbox/util/debug.h"
#include "linbox/util/commentator.h"
#include "linbox/field/gf2.h"
#include "linbox/vector/vector-domain.h"
#include "linbox/algorithms/gauss.h"
#include "linbox/blackbox/zo-gf2.h"

/** @file algorithms/gauss-gf2.h
 * @brief  Gauss elimination and applications for sparse matrices on \f$F_2\f$.
 * Rank, nullspace, solve...
 */


namespace LinBox
{

	template <>
	class GaussDomain<GF2> {
	public:
		typedef GF2 Field;
		typedef Field::Element Element;

		// Preferred Matrix type
		typedef ZeroOne<GF2> Matrix;

	public:

		/** \brief The field parameter is the domain  over which to perform computations.
		 */
		GaussDomain (const Field &) {}

		//Copy constructor
		///
		GaussDomain (const GaussDomain &)  {}

		/** accessor for the field of computation.
		*/
		const Field &field () const { return *(new GF2()); }

		/** @name rank
		  Callers of the different rank routines
		  @li  The "in" suffix indicates in place computation
		  @li  Without Ni, Nj, the SparseSeqMatrix parameter must be a vector of sparse
		  row vectors, NOT storing any zero.
		  @li  Calls @link rankinLinearPivoting@endlink (by default) or @link rankinNoReordering@endlink
		  */
		//@{
		///
		///
		template <class SparseSeqMatrix> unsigned long& rankin(unsigned long &Rank,
		SparseSeqMatrix        &A,
		unsigned long  Ni,
		unsigned long  Nj,
		SparseEliminationTraits::PivotStrategy   reord = SparseEliminationTraits::PIVOT_LINEAR) const ;

		///
		template <class SparseSeqMatrix> unsigned long& rankin(unsigned long &Rank,
		SparseSeqMatrix        &A,
		SparseEliminationTraits::PivotStrategy   reord = SparseEliminationTraits::PIVOT_LINEAR) const;

		///
		template <class SparseSeqMatrix> unsigned long& rank(unsigned long &rk,
		const SparseSeqMatrix        &A,
		unsigned long  Ni,
		unsigned long  Nj,
		SparseEliminationTraits::PivotStrategy   reord = SparseEliminationTraits::PIVOT_LINEAR) const ;

		///
		template <class SparseSeqMatrix> unsigned long& rank(unsigned long &rk,
		const SparseSeqMatrix        &A,
		SparseEliminationTraits::PivotStrategy   reord = SparseEliminationTraits::PIVOT_LINEAR) const ;

		//@}

		/** @name det
		  Callers of the different determinant routines\\
		  -/ The "in" suffix indicates in place computation\\
		  -/ Without Ni, Nj, the SparseSeqMatrix parameter must be a vector of sparse
		  row vectors, NOT storing any zero.\\
		  -/ Calls @link LinearPivoting@endlink (by default) or @link NoReordering@endlink
		  */
		//@{
		///
		template <class SparseSeqMatrix> Element& detin(Element &determinant,
		SparseSeqMatrix        &A,
		SparseEliminationTraits::PivotStrategy   reord = SparseEliminationTraits::PIVOT_LINEAR) const;
		///
		template <class SparseSeqMatrix> Element& detin(Element &determinant,
		SparseSeqMatrix        &A,
		unsigned long  Ni,
		unsigned long  Nj,
		SparseEliminationTraits::PivotStrategy   reord = SparseEliminationTraits::PIVOT_LINEAR) const;
		///
		template <class SparseSeqMatrix> Element& det(Element &determinant,
		const SparseSeqMatrix        &A,
		SparseEliminationTraits::PivotStrategy   reord = SparseEliminationTraits::PIVOT_LINEAR) const;
		///
		template <class SparseSeqMatrix> Element& det(Element &determinant,
		const SparseSeqMatrix        &A,
		unsigned long  Ni,
		unsigned long  Nj,
		SparseEliminationTraits::PivotStrategy   reord = SparseEliminationTraits::PIVOT_LINEAR) const;
		//@}


		/** \brief Sparse in place Gaussian elimination with reordering to reduce fill-in.
		 * pivots are chosen in sparsest column of sparsest row.
		 * This runs in linear overhead.
		 * It is similar in spirit but different from Markovitz' approach.
		 *
		 * \pre Using : SparseFindPivot(..., density) for sparsest column, and
		 * eliminate (..., density)
		 *
		 * The SparseSeqMatrix parameter must meet the LinBox sparse matrix interface.
		 * [check details].
		 * The computedet indicates whether the algorithm must compute the determionant as it goes
		 *
		 * @bib
		 * - Jean-Guillaume Dumas and Gilles Villard,
		 * <i>Computing the rank of sparse matrices over finite fields.</i>
		 * In Ganzha et~al. CASC'2002, pages 47--62.
		 */
		template <class SparseSeqMatrix, class Perm>
		unsigned long& QLUPin(unsigned long &Rank,
				      Element& determinant,
				      Perm          &Q,
				      SparseSeqMatrix	    &L,
				      SparseSeqMatrix        &U,
				      Perm	    &P,
				      unsigned long Ni,
				      unsigned long Nj) const;

		template <class SparseSeqMatrix, class Perm, class Vector1, class Vector2>
		Vector1& solve(Vector1& x, Vector1& w, unsigned long Rank, const Perm& Q, const SparseSeqMatrix& L, const SparseSeqMatrix& U, const Perm& P, const Vector2& b) const;


		template <class SparseSeqMatrix, class Vector1, class Vector2>
		Vector1& solvein(Vector1& x,
				 SparseSeqMatrix        &A,
				 const Vector2& b) const;

		template <class SparseSeqMatrix, class Vector1, class Vector2, class Random>
		Vector1& solvein(Vector1& x,
				 SparseSeqMatrix        &A,
				 const Vector2& b, Random& generator) const;


		template <class SparseSeqMatrix, class Perm>
		unsigned long& InPlaceLinearPivoting(unsigned long &Rank,
						     Element& determinant,
						     SparseSeqMatrix        &A,
						     Perm                   &P,
						     unsigned long Ni,
						     unsigned long Nj) const;
		template <class SparseSeqMatrix>
		unsigned long& NoReordering (unsigned long & Rank, Element& , SparseSeqMatrix &, unsigned long , unsigned long ) const
		{
			std::cerr << "Sparse elimination over GF2 without reordering not implemented" << std::endl;
			return Rank;
		}


	protected:

		//-----------------------------------------
		// Sparse elimination using a pivot row :
		// lc <-- lc - lc[k]/lp[0] * lp
		// D is the number of elements per column
		//   it is updated and used for reordering
		// Vector is a vector of Pair (lin_pair.h)
		//-----------------------------------------
		template <class Vector, class D>
		void eliminateBinary (Element             & headpivot,
				      Vector              &lignecourante,
				      const Vector        &lignepivot,
				      const unsigned long indcol,
				      const long indpermut,
				      const unsigned long npiv,
				      D                   &columns) const;

		template <class Vector>
		void permuteBinary (Vector              &lignecourante,
				    const unsigned long &indcol,
				    const long &indpermut) const;

		//------------------------------------------
		// Looking for a non-zero pivot in a row
		// Using the column density for reordering
		// Pivot is chosen as to :
		// 1. Row density is minimum
		// 2. Column density is minimum for this row
		//------------------------------------------
		template <class Vector, class D>
		void SparseFindPivotBinary (Vector &lignepivot, unsigned long &indcol, long &indpermut, D &columns, Element& determinant) const;

		//------------------------------------------
		// Looking for a non-zero pivot in a row
		// No reordering
		//------------------------------------------
		template <class Vector>
		void SparseFindPivotBinary (Vector &lignepivot, unsigned long &indcol, long &indpermut, Element& determinant) const;

	};
} // namespace LinBox

#include "linbox/algorithms/gauss/gauss-gf2.inl"
#include "linbox/algorithms/gauss/gauss-pivot-gf2.inl"
#include "linbox/algorithms/gauss/gauss-elim-gf2.inl"
#include "linbox/algorithms/gauss/gauss-rank-gf2.inl"
#include "linbox/algorithms/gauss/gauss-solve-gf2.inl"

#endif // __LINBOX_gauss_gf2_H


// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,:0,t0,+0,=s
// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:

