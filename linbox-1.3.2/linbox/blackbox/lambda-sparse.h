/* linbox/blackbox/lambda-sparse.h
 * Copyright (C) 2004 Pascal Giorgi
 *
 * Written by Pascal Giorgi <pascal.giorgi@ens-lyon.fr>
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


#ifndef __LINBOX_lambda_sparse_H
#define __LINBOX_lambda_sparse_H

#include "linbox/blackbox/sparse.h"
#include "linbox/blackbox/archetype.h"
#include "linbox/vector/stream.h"
#include "linbox/vector/vector-traits.h"
#include "linbox/integer.h"

namespace LinBox
{


	/// \ingroup blackbox
	template< class _Field,
	class _Row = typename Vector<_Field>::SparseSeq >
	class LambdaSparseMatrix : public SparseMatrix<_Field,_Row> {

	public:

		typedef _Row Row;
		typedef _Field Field;

		template<typename _Tp1, typename _Rw1 = _Row>
		struct rebind
		{ typedef LambdaSparseMatrix<_Tp1, _Rw1> other; };

		// Contructor of a lambda-sparse matrix as defined in Mulder 2003
		// with non-zero elements choosen from entire Field
		LambdaSparseMatrix(const Field& F,size_t m, size_t n, double LAMBDA = 3.) :
			SparseMatrix<Field,Row> (F,m,n)
		{

			integer card;
			F.cardinality (card);
			typename Field::RandIter _randiter(F);
			double                   init_p = 1.0 - 1.0 / (double) card;
			double                   log_m = LAMBDA * log ((double) m) / M_LN2;
			double                   new_p;

			RandomSparseStream<Field,typename Vector<Field>::SparseSeq> stream (F, _randiter, init_p, n, m );

			for (unsigned int i = 0; i < m; ++i) {
				new_p = log_m / double(m - i);

				if (init_p < new_p)
					stream.setP (init_p);
				else
					stream.setP (new_p);

				stream >> this->getRow (i);
			}
		}

		// Contructor of a lambda-sparse matrix as defined in Mulder 2003
		// with non-zero elements choosen from a subset of the Field
		LambdaSparseMatrix(const Field& F,size_t m, size_t n,const integer size, double LAMBDA = 3.) :
			SparseMatrix<Field,Row> (F,m,n)
		{

			typename Field::RandIter _randiter(F,size,0);
			double init_p = 1.0 - 1.0 / (double) size;
			double log_m = LAMBDA * log ((double) m) / M_LN2;
			double new_p;

			RandomSparseStream<Field,typename Vector<Field>::SparseSeq> stream (F, _randiter, init_p, n, m );

			for (unsigned int i = 0; i < m; ++i) {
				new_p = log_m / double(m - i);

				if (init_p < new_p)
					stream.setP (init_p);
				else
					stream.setP (new_p);

				stream >> this->getRow (i);
			}
		}


		// Copy constructor
		LambdaSparseMatrix (const LambdaSparseMatrix<Field,Row>& L) :
			SparseMatrix<Field,Row>(L)
		{}


		// Copy constructor from a LambdaSparseMatrix over a Ring
		// allow the mod p reduction for all entries.
		template<class _Ring, class _IRow>
		LambdaSparseMatrix (const Field& F, const LambdaSparseMatrix<_Ring,_IRow>& L) :
			SparseMatrix<Field,Row> (F,L.rowdim(),L.coldim())
		{

			//typename LambdaSparseMatrix<_Ring,_IRow>::ConstIterator Liter = L.Begin();
			typename LambdaSparseMatrix<_Ring,_IRow>::ConstIndexedIterator Literindex = L.IndexedBegin();

			integer tmp;
			_Ring r= L.field();
			for (;  Literindex!=L.IndexedEnd();  ++Literindex) {
				r.convert(tmp,*Literindex);
				F.init(refEntry(Literindex.rowIndex(),Literindex.colIndex()),tmp);
			}


		}

		// return the norm of the matrix (= the maximum value)
		integer& Norm(integer& norm)
		{
			typename Field::Element max;
			// Dan Roche 7-20-04 added typename here to stop compiler warning
			typename LambdaSparseMatrix<_Field,_Row>::ConstIterator iter= this->Begin();
			max = *iter;
			for (; iter != this->End(); ++iter)
				if (*iter > max) max=*iter;

			this->_field.convert(norm,max);
			return norm;
		}




	}; //end of class LambdaSparseMatrix

} //end of namespace LinBox

#endif //__LINBOX_lambda_sparse_H



// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,:0,t0,+0,=s
// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:

