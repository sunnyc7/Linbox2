
/* tests/test-modular.C
 * Copyright (C) 2001, 2002 Bradford Hovinen,
 * Copyright (C) 2002 Dave Saunders
 *
 * Written by Bradford Hovinen <hovinen@cis.udel.edu>,
 *            Dave Saunders <saunders@cis.udel.edu>
 *
 * ------------------------------------
 * 2002-04-10 Bradford Hovinen <hovinen@cis.udel.edu>
 *
 * Rename from test-large-modular.C to test-modular.C; made other updates in
 * accordance with changes to Modular interace.
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

/*! @file   tests/test-modular-int.C
 * @ingroup tests
 * @brief run runFieldTests testRandomIterator tests on modular-int32_t
 * @test run runFieldTests testRandomIterator tests on modular-int32_t
 */


#include "linbox/linbox-config.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>

#include "linbox/field/modular.h"

#include "test-common.h"
#include "test-generic.h"

using namespace LinBox;

int main (int argc, char **argv)
{
	static integer q1("18446744073709551557");
	static integer q2 = 2147483647;
	static integer q3 = 65521U;
	static int q4 = 101;
	static size_t n = 10000;
	static int iterations = 1;
	static int trials = 10000;
	static int categories = 1000;
	static int hist_level = 10;

	static Argument args[] = {
		{ 'K', "-K Q", "Operate over the \"field\" GF(Q) [1] for integer modulus.", TYPE_INTEGER, &q1 },
		{ 'Q', "-Q Q", "Operate over the \"field\" GF(Q) [1] for uint32_t modulus.", TYPE_INTEGER, &q2 },
		{ 'q', "-q Q", "Operate over the \"field\" GF(Q) [1] for uint16_t modulus.", TYPE_INTEGER, &q3 },
		{ 'p', "-p P", "Operate over the \"field\" GF(Q) [1] for uint8_t modulus.", TYPE_INT, &q4 },
		{ 'n', "-n N", "Set dimension of test vectors to NxN.", TYPE_INT,     &n },
		{ 'i', "-i I", "Perform each test for I iterations.", TYPE_INT,     &iterations },
		{ 't', "-t T", "Number of trials for the random iterator test.", TYPE_INT, &trials },
		{ 'c', "-c C", "Number of categories for the random iterator test.", TYPE_INT, &categories },
		{ 'H', "-H H", "History level for random iterator test.", TYPE_INT, &hist_level },
		END_OF_ARGUMENTS
	};

	parseArguments (argc, argv, args);

	commentator().start("Modular<int32_t> field test suite", "Modular<int32_t>");
	bool pass = true;

	Modular<int32_t> F_int (1073741789);//(2147483629);//(2147483647);
	integer k = FieldTraits<Modular<int32_t> >::maxModulus() ;
	prevprime(k,k);
	Modular<int32_t> I_int(k);


	// Make sure some more detailed messages get printed
	commentator().getMessageClass (INTERNAL_DESCRIPTION).setMaxDepth (4);
	commentator().getMessageClass (INTERNAL_DESCRIPTION).setMaxDetailLevel (Commentator::LEVEL_UNIMPORTANT);

	if (!runFieldTests (F_int,  "Modular<int32_t>",  iterations, n, false)) pass = false;
	if (!testRandomIterator (F_int,  "Modular<int32_t>", trials, categories, hist_level)) pass = false;

	if (!runFieldTests (I_int,  "Modular<int32_t>",  iterations, n, false)) pass = false;
	if (!testRandomIterator (I_int,  "Modular<int32_t>", trials, categories, hist_level)) pass = false;


	commentator().stop("Modular<int32_t> field test suite");
	return pass ? 0 : -1;
}

// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,:0,t0,+0,=s
// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:

