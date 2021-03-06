/* Copyright (C) LinBox
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


/*! @file tests/test-optimization.C
 * @ingroup tests
 * @brief no doc.
 * @test no doc
 */

#include "linbox/linbox-config.h"


#include <iostream>
#include <fstream>
#include "linbox/field/modular.h" /* WHY ?? */
/*   otherwise the compiler says :
 *   /usr/bin/ld: test-optimization.o: undefined reference to symbol 'dtrmm_'
 *   /usr/bin/ld: note: 'dtrmm_' is defined in DSO  /usr/lib64/libblas.so.3gf so try adding it to the linker command line
 */
#include "linbox/config-blas.h"
#include <fflas-ffpack/fflas/fflas.h>
#include "linbox/util/timer.h"
#include "linbox/util/commentator.h"

#include "test-common.h"

using namespace LinBox;

int main (int argc, char ** argv)
{
	size_t n=300, nmax=1000, prec=256;

	static Argument args[] = {
		{ 'n', "-n n", "Operate over the \"field\" GF(Q) [1] for integer modulus.", TYPE_INT, &n },
		{ 'm', "-m m", "Operate over the \"field\" GF(Q) [1] for uint32_t modulus.", TYPE_INT, &nmax },
		{ 'p', "-p p", "Operate over the \"field\" GF(Q) [1] for uint16_t modulus.", TYPE_INT, &prec },
		END_OF_ARGUMENTS
	};

	parseArguments (argc, argv, args);

	commentator().start("Optimization suite", "Optim");
	std::ostream& report = commentator().report();

	Modular<double> F(17);
	Timer chrono;

	double *A, *C;
	A = new double[nmax*nmax];
	C = new double[nmax*nmax];
	for (size_t i=0; i<nmax*nmax;++i){
		A[i]=rand() % 17;
	}

	do {
		chrono.start();
		FFLAS::fgemm(F, FFLAS::FflasNoTrans, FFLAS::FflasNoTrans,
			     n, n, n, 1., A, n, A, n, 0., C, n);
		chrono.stop();
		report << std::endl
		<< "fgemm " << FFLAS::WinoSteps(n) << "Wino: " << n << "x" << n << ": "
		<< chrono.usertime() << " s, "
		<< (2.0/chrono.usertime()*(double)n/100.0*(double)n/100.0*(double)n/100.0) << " Mffops"
		<< std::endl;

		n+=prec;
	} while ((n < nmax));

	delete[] A;
	delete[] C;

	return 0;
}

// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,:0,t0,+0,=s
// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:

