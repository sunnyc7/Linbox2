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


/*! @file  tests/test-modular-balanced-int.C
 * @ingroup tests
 * @brief  tests only runFieldTests on modular-balanced-int32_t
 * @test   tests only runFieldTests on modular-balanced-int32_t
 */



#include "linbox/linbox-config.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>

#include "linbox/field/modular-balanced.h"

#include "test-common.h"
#include "test-generic.h"

using namespace LinBox;

/*! @bug the arguments are meaningless
 */
int main (int argc, char **argv)
{
	static integer q1("18446744073709551557");
	static integer q2 = 2147483647;
	static integer q3 = 65521U;
	static int q4 = 101;
	static size_t n = 1000;
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

	commentator().start("ModularBalanced<int32_t> field test suite", "ModularBalanced<int32_t>");
	bool pass = true;

	ModularBalanced<int32_t> F_int (1073741789);//(2147483629);//(2147483647);
	ModularBalanced<int32_t> G_int (2147483647);
	ModularBalanced<int32_t> H_int (3);
	integer k = FieldTraits<ModularBalanced<int32_t> >::maxModulus() ;
	prevprime(k,k);
	ModularBalanced<int32_t> I_int(k);

	// Make sure some more detailed messages get printed
	commentator().getMessageClass (INTERNAL_DESCRIPTION).setMaxDepth (4);
	commentator().getMessageClass (INTERNAL_DESCRIPTION).setMaxDetailLevel (Commentator::LEVEL_UNIMPORTANT);

	if (!runFieldTests (F_int,  "ModularBalanced<int32_t>",  iterations, n, false)) pass = false;
	if (!testRandomIterator (F_int,  "ModularBalanced<int32_t>", trials, categories, hist_level)) pass = false;

	if (!runFieldTests (G_int,  "ModularBalanced<int32_t>",  iterations, n, false)) pass = false;
	if (!testRandomIterator (G_int,  "ModularBalanced<int32_t>", trials, categories, hist_level)) pass = false;

	if (!runFieldTests (H_int,  "ModularBalanced<int32_t>",  iterations, n, false)) pass = false;
	if (!testRandomIterator (H_int,  "ModularBalanced<int32_t>", trials, categories, hist_level)) pass = false;


	if (!runFieldTests (I_int,  "ModularBalanced<int32_t>",  iterations, n, false)) pass = false;
	// if (!testRandomIterator (I_int,  "ModularBalanced<int32_t>", trials, categories, hist_level)) pass = false;


	commentator().stop("ModularBalanced<int32_t> field test suite");
	return pass ? 0 : -1;
}

// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,:0,t0,+0,=s
// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:

