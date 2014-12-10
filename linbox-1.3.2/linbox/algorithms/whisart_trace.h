/* linbox/algorithms/cra-domain.h
 * Copyright (C) 1999-2010 The LinBox group
 *
 * Time-stamp: <19 Apr 10 18:35:14 Jean-Guillaume.Dumas@imag.fr>
 * Computes the trace of D1 B D2 B^T D1 or D1 B^T D2 B D1
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
#ifndef __LINBOX_whisart_trace_H
#define __LINBOX_whisart_trace_H

#include "linbox/field/modular.h"
#include "linbox/blackbox/compose.h"
#include "linbox/blackbox/transpose.h"
#include "linbox/blackbox/sparse.h"
#include "linbox/blackbox/lambda-sparse.h"
// #include "linbox/blackbox/subrowmatrix.h"
#include "linbox/solutions/trace.h"

namespace LinBox
{

	// Trait to show whether or not the BB class has a Indexed iterator
	template<class BB> struct IndexedCategory;

	/// limited doc so far
	namespace IndexedTags
	{
		struct HasIndexed{};
		struct NoIndexed{};
	}

	template<class BB> struct IndexedCategory {
		typedef IndexedTags::NoIndexed Tag;
	};

	template<class Field>
	struct IndexedCategory< BlasMatrix<Field> > 	{
		typedef IndexedTags::HasIndexed Tag; };


	template<class Field, class Row>
	struct IndexedCategory< LambdaSparseMatrix<Field,Row> > 	{
		typedef IndexedTags::HasIndexed Tag; };

	template<class Field, class Row>
	struct IndexedCategory< SparseMatrix<Field,Row> > 	{
		typedef IndexedTags::HasIndexed Tag; };

#if 0
	template<class Matrix, class MatrixCategory>
	struct IndexedCategory< SubRowMatrix<Matrix,MatrixCategory> > 	{
		typedef IndexedTags::HasIndexed Tag;
	};
#endif



	template<class Field, class BB>
	typename Field::Element& WhisartTrace(
					      typename Field::Element& trace,
					      const Field& F,
					      const LinBox::Diagonal<Field>& ExtD,
					      const BB& A,
					      const LinBox::Diagonal<Field>& InD)
	{
		return WhisartTrace(trace, F, ExtD, A, InD, typename IndexedCategory<BB>::Tag() );
	}

	template<class Field, class BB>
	typename Field::Element& WhisartTraceTranspose(
						       typename Field::Element& trace,
						       const Field& F,
						       const LinBox::Diagonal<Field>& ExtD,
						       const BB& A,
						       const LinBox::Diagonal<Field>& InD)
	{
		return WhisartTraceTranspose(trace, F, ExtD, A, InD, typename IndexedCategory<BB>::Tag() );
	}

	template<class Field, class BB>
	typename Field::Element& WhisartTrace(
					      typename Field::Element& tr,
					      const Field& F,
					      const LinBox::Diagonal<Field>& ExtD,
					      const BB& A,
					      const LinBox::Diagonal<Field>& InD, IndexedTags::NoIndexed t)
	{
		// Trace of ExtD B InD B^T ExtD
		typedef Compose<Diagonal<Field>, BB > C_DB;
		C_DB	B1 (&ExtD, 	&A); 	// D1 A
		typedef Compose<C_DB, Diagonal<Field> > C_DBD;
		C_DBD 	B2 (&B1,   	&InD);	// D1 A D2
		Transpose<BB> AT (&A);
		typedef Compose<C_DBD, Transpose<BB> > C_DBDBt;
		C_DBDBt	B3 (&B2, 	&AT);	// D1 A D2 A^T
		typedef Compose<C_DBDBt, Diagonal<Field> > C_DBDBtD;
		C_DBDBtD 	B  (&B3, 	&ExtD);	// D1 A D2 A^T D1
		return trace(tr, B);
	}

	template<class Field, class BB>
	typename Field::Element& WhisartTraceTranspose(
						       typename Field::Element& tr,
						       const Field& F,
						       const LinBox::Diagonal<Field>& ExtD,
						       const BB& A,
						       const LinBox::Diagonal<Field>& InD,
						       IndexedTags::NoIndexed t)
	{
		// Trace of ExtD A^T  InD A ExtD
		Transpose<BB> AT (&A);
		typedef Compose<Diagonal<Field>,Transpose<BB> > C_DBt;
		C_DBt	B1 (&ExtD, 	&AT); 	// D1 A^T
		typedef Compose<C_DBt, Diagonal<Field> > C_DBtD;
		C_DBtD 	B2 (&B1,   	&InD);	// D1 A^T D2
		typedef Compose<C_DBtD, BB> C_DBtDB;
		C_DBtDB	B3 (&B2, 	&A);	// D1 A^T D2 A
		typedef Compose<C_DBtDB, Diagonal<Field> > C_DBtDBD;
		C_DBtDBD 	B  (&B3, 	&ExtD);	// D1 A^T D2 A D1
		return trace(tr, B);
	}



	template<class Field, class BB>
	typename Field::Element& WhisartTrace(
					      typename Field::Element& tr,
					      const Field& F,
					      const LinBox::Diagonal<Field>& ExtD,
					      const BB& A,
					      const LinBox::Diagonal<Field>& InD,
					      IndexedTags::HasIndexed )
	{
		// Trace of ExtD B InD B^T ExtD
		// is sum ExtD_i^2 B_{i,j} InD_j
		F.init(tr, 0);
		for(typename BB::ConstIndexedIterator it = A.IndexedBegin();
		    it != A.IndexedEnd(); ++it) {
			typename Field::Element tmp,e,i;
			F.init(tmp);
			F.init(e);F.init(i);
			F.mul(tmp,it.value(),it.value());
			ExtD.getEntry(e, it.rowIndex(),it.rowIndex());
			InD.getEntry(i, it.colIndex(),it.colIndex());
			F.mulin(tmp,e);
			F.mulin(tmp,e);
			F.mulin(tmp,i);
			F.addin(tr, tmp);
		}
		return tr;
	}

	template<class Field, class BB>
	typename Field::Element& WhisartTraceTranspose(
						       typename Field::Element& tr,
						       const Field& F,
						       const LinBox::Diagonal<Field>& ExtD,
						       const BB& A,
						       const LinBox::Diagonal<Field>& InD,
						       IndexedTags::HasIndexed )
	{
		// Trace of ExtD B^T  InD B ExtD
		// is sum ExtD_j^2 B_{i,j} InD_i
		F.init(tr, 0);
		for(typename BB::ConstIndexedIterator it = A.IndexedBegin();
		    it != A.IndexedEnd(); ++it) {

			typename Field::Element tmp,e,i;
			F.init(tmp,0UL);
			F.init(e,0UL);
			F.init(i,0UL);

			F.mul(tmp, it.value(),it.value());
			ExtD.getEntry(e, it.colIndex(),it.colIndex());
			InD.getEntry(i, it.rowIndex(),it.rowIndex());
			F.mulin(tmp,e);
			F.mulin(tmp,e);
			F.mulin(tmp,i);
			F.addin(tr, tmp);
		}

		return tr;
	}



}
#endif //__LINBOX_whisart_trace_H


// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,:0,t0,+0,=s
// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:

