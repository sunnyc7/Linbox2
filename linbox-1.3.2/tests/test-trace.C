
/* tests/test-trace.C
 * Copyright (C) -bds
 *
 * Earlier version by Bradford Hovinen <hovinen@cis.udel.edu>
 *
 * --------------------------------------------------------
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
 *
 */

/*! @file  tests/test-trace.C
 * @ingroup tests
 *
 * @brief no doc
 *
 * @test no doc.
 */


#include "linbox/linbox-config.h"

#include <iostream>
#include <vector>

#include "test-common.h"

#include "linbox/field/modular.h"
#include "linbox/solutions/trace.h"

using namespace LinBox;

int main (int argc, char **argv)
{
	bool pass = true;

	static size_t n = 20;
	static integer q = 101;

	static Argument args[] = {
		{ 'n', "-n N", "Set dimension of test matrices to NxN.", TYPE_INT,     &n },
		{ 'q', "-q Q", "Operate over the \"field\" GF(Q) [1].", TYPE_INTEGER, &q },
		END_OF_ARGUMENTS
	};

	parseArguments (argc, argv, args);

	commentator().start("Trace test suite", "Trace");
	commentator().getMessageClass (INTERNAL_DESCRIPTION).setMaxDepth (3);

	typedef Modular<int> Field;
	typedef Field::Element Element;
	typedef vector<Element> Vector;
	Field F (q);
	Element t, t1, t2, t3;
	F.init(t, 0); F.init(t1, 0); F.init(t2, 0); F.init(t3, 0);

	Element s; F.init(s, 3);
	ScalarMatrix<Field> A(F, n, s);
	F.init(t, n*3);
	trace(t1, A);
	if (! F.areEqual(t1, t) ) pass = false;
	trace(t2, A, TraceTags::Local() );
	if (! F.areEqual(t1, t) ) pass = false;
	trace(t3, A, TraceTags::Generic() );
	if (! F.areEqual(t1, t) ) pass = false;

/*
	Vector v(2*n-1, s);
	for (int i = 0; i < 2*n-1; ++i) if (i != n-1) F.init(v[i], i);
	Toeplitz<Field> B(F, n, v);
	trace(t1, B);
	if (! F.areEqual(t1, t) pass = false;
	trace(t2, B, TraceTags::Local() );
	if (! F.areEqual(t1, t) pass = false;
	trace(t3, B, TraceTags::Generic() );
	if (! F.areEqual(t1, t) pass = false;
*/

	commentator().stop("Trace solution test suite");
	return pass ? 0 : -1;
}

// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,:0,t0,+0,=s
// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:

