/*
 * examples/sparseelimrank.C
 *
 * Copyright (C) 2006, 2010  J-G Dumas
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
 */

/** \file examples/sparseelimrank.C
 * @example  examples/sparseelimrank.C
 \brief Gaussian elimination Rank of sparse matrix over Z or Zp.
 \ingroup examples
 */
//#include "linbox-config.h"

#include <iostream>
#include <vector>
#include <utility>

#include "linbox/field/modular.h"
#include "linbox/field/gf2.h"
#include "linbox/blackbox/sparse.h"
#include "linbox/blackbox/zero-one.h"
#include "linbox/solutions/rank.h"
#include "linbox/util/matrix-stream.h"
#include "linbox/field/givaro.h"

using namespace LinBox;
using namespace std;

int main (int argc, char **argv)
{
	commentator().setMaxDetailLevel (-1);
	commentator().setMaxDepth (-1);
	commentator().setReportStream (std::cerr);

	if (argc < 2 || argc > 3)
	{	cerr << "Usage: rank <matrix-file-in-supported-format> [<p>]" << endl; return -1; }

	ifstream input (argv[1]);
	if (!input) { cerr << "Error opening matrix file: " << argv[1] << endl; return -1; }

	long unsigned int r;

	if (argc == 2) { // rank over the integers.

		/* We could pick a random prime and work mod that prime, But the point here
		   is that the rank function in solutions/ handles that issue.  Our matrix here
		   is an integer matrix and our concept is that we are getting the rank of that
		   matrix by some blackbox magic inside linbox.
		   */
		LinBox::GivaroRational ZZ;
		MatrixStream<GivaroRational> ms( ZZ, input );
		SparseMatrix<GivaroRational> A ( ms );
		cout << "A is " << A.rowdim() << " by " << A.coldim() << endl;

		LinBox::rank (r, A, Method::SparseElimination() );

		cout << "Rank is " << r << endl;
	}
	if (argc == 3) {
		double q = atof(argv[2]);
		typedef Modular<double> Field;
		Field F(q);
		MatrixStream<Field> ms( F, input );
		SparseMatrix<Field, Vector<Field>::SparseSeq > B (ms);
		cout << "B is " << B.rowdim() << " by " << B.coldim() << endl;
		if (B.rowdim() <= 20 && B.coldim() <= 20) B.write(cout) << endl;


		Method::SparseElimination SE;
		SE.strategy(Specifier::PIVOT_NONE);
		// using Sparse Elimination
		LinBox::rank (r, B, SE);
		if (B.rowdim() <= 20 && B.coldim() <= 20) B.write(cout) << endl;
		cout << "Rank is " << r << endl;

		SE.strategy(Specifier::PIVOT_LINEAR);
		// using Sparse Elimination
		LinBox::rank (r, B, SE);
		if (B.rowdim() <= 20 && B.coldim() <= 20) B.write(cout) << endl;
		cout << "Rank is " << r << endl;


	}

	return 0;
}

// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,:0,t0,+0,=s
// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:

