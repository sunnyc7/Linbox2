/* tests/test-charpoly.C
 * Copyright (C) LinBox
 * Written by bds (starting from test-charpoly.C)
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

/*! @file  tests/test-charpoly.C
 * @ingroup tests
 * @brief tests the characteristic polynomial of sparse and special matrices
 * @warning gcc-4.2 produces bad optimized code there
 * @bug occasionnnaly there is a "SIGFPE, Arithmetic exception." in CRA
 * @bug testRandomCharpoly is not always tested !!
 * @test characteristic polynomial of some matrices (sparse, special)
 */


#include "linbox/linbox-config.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <cstdio>

#include "linbox/blackbox/sparse.h"
#include "linbox/blackbox/scalar-matrix.h"
#include "linbox/solutions/charpoly.h"
#include "linbox/util/commentator.h"
//#include "linbox/ring/givaro-polynomial.h"
#include "linbox/vector/stream.h"

#include "test-common.h"

using namespace LinBox;


/* Test 1: charpoly of the identity matrix
 *
 * Construct the identity matrix and compute its characteristic polynomial. Confirm
 * that the result is (x-1)^n
 *
 * Z - Integers representation over which to perform computations
 * n - Dimension to which to make matrix
 *
 * Return true on success and false on failure
 */
template <class Dom, class Polynomial>
typename Dom::Element eval (const Dom& D,
			    typename Dom::Element& value,
			    const Polynomial& P,
			    const typename Dom::Element x)
{
	typename Dom::Element tmp = P[P.size()-1];
	for (int i = (int)P.size()-2; i >= 0; --i){
		D.mulin (tmp, x);
		D.addin (tmp, P[i]);
	}
	return value = tmp;
}

template <class Dom>
static bool testIdentityCharpoly (Dom &Z, size_t n, bool symmetrizing=false)
{
	typedef typename Dom::Element                Element;
	typedef vector<Element>                       Vector;
	typedef ScalarMatrix<Dom>                   Blackbox;
	typedef GivPolynomialRing<Dom, Givaro::Dense> PolDom;
	typedef typename PolDom::Element          Polynomial;
	//typedef Vector Polynomial;

	LinBox::commentator().start ("Testing identity Charpoly", "testIdentityCharpoly");

	bool ret = true;
	Element one; Z.init(one, 1);
	Element negone; Z.init(negone, -1);

	//PolDom IPD(Z);

	Blackbox A (Z, n, one);

	Polynomial phi;

	charpoly (phi, A);

	ostream &report = LinBox::commentator().report (Commentator::LEVEL_IMPORTANT, INTERNAL_DESCRIPTION);
	report << "Characteristic polynomial is: ";
	printPolynomial<Dom, Polynomial> (Z, report, phi);

	// partial check - just that charpoly has right values at 0, 1, -1.
	Element val, val2, neg2, pow2;
	// value at 1 should be zero
	eval(Z, val, phi, one);
	if (! Z.isZero(val) ) ret = false;
	// value at zero should be (-1)^n
	val = (n % 2 == 0) ? one : negone;
	if (! Z.areEqual(val, phi[0])) ret = false;
	// value at -1 should be (-2)^n
	eval(Z, val2, phi, negone);
	Z.init(neg2, -2); Z.init(pow2, 1);
	for (size_t i = 0; i < n; ++i) Z.mulin(pow2, neg2);
	if (! Z.areEqual(val2, pow2)) ret = false;

	if (! ret){
		LinBox::commentator().report (Commentator::LEVEL_IMPORTANT, INTERNAL_ERROR)
			<< "ERROR: Characteristic polynomial is incorrect" << endl;
	}

	LinBox::commentator().stop (MSG_STATUS (ret), (const char *) 0, "testIdentityCharpoly");

	return ret;
}

/* Test 2: characteristic polynomial of a nilpotent matrix
 *
 * Construct an index-n nilpotent matrix and compute its characteristic
 * polynomial. Confirm that the result is x^n
 *
 * F - Field over which to perform computations
 * n - Dimension to which to make matrix
 *
 * Return true on success and false on failure
 */
template <class Field>
static bool testNilpotentCharpoly (Field &F, size_t n)
{
	typedef vector <typename Field::Element> Vector;
// 	typedef GivPolynomialRing<Field, Givaro::Dense> PolDom;
// 	typedef typename PolDom::Element Polynomial;
	typedef Vector Polynomial;
	typedef pair <vector <size_t>, vector <typename Field::Element> > Row;
	typedef SparseMatrix <Field> Blackbox;

	LinBox::commentator().start ("Testing nilpotent charpoly", "testNilpotentCharpoly");

	bool ret = true;
	bool lowerTermsCorrect = true;

	size_t i;

	StandardBasisStream<Field, Row> stream (F, n);
	Row v;
	stream.next (v);
	Blackbox A (F, stream);

	ostream &who = LinBox::commentator().report (Commentator::LEVEL_UNIMPORTANT, INTERNAL_DESCRIPTION);
	who << "Matrix:" << endl;
	A.write (who, FORMAT_PRETTY);



	Polynomial phi;

	charpoly (phi, A);

	ostream &report = LinBox::commentator().report (Commentator::LEVEL_IMPORTANT, INTERNAL_DESCRIPTION);
	report << "characteristic polynomial is: ";
	printPolynomial (F, report, phi);

	linbox_check(n);
	for (i = 0; i < n - 1; i++)
		if (!F.isZero (phi[i]))
			lowerTermsCorrect = false;

	if (phi.size () != n + 1 || !F.isOne (phi[n]) || !lowerTermsCorrect) {
		ret = false;
		LinBox::commentator().report (Commentator::LEVEL_IMPORTANT, INTERNAL_ERROR)
			<< "ERROR: characteristic polynomial is incorrect (should be x^" << n << ')' << endl;
	}

	LinBox::commentator().stop (MSG_STATUS (ret), (const char *) 0, "testNilpotentCharpoly");

	return ret;
}

#if 1
/* Test 3: Random charpoly of sparse matrix
 *
 * Generates a random sparse matrix with K nonzero elements per row and computes
 * its characteristic polynomial. Then computes random vectors and applies the
 * polynomial to them in Horner style, checking whether the result is 0.
 *
 * F - Field over which to perform computations
 * n - Dimension to which to make matrix
 * K - Number of nonzero elements per row
 * numVectors - Number of random vectors to which to apply the characteristic polynomial
 *
 * Return true on success and false on failure
 */

template <class Field, class Row, class Vector>
bool testRandomCharpoly (Field                 &F,
			VectorStream<Row>    &A_stream,
			VectorStream<Vector> &v_stream)
{
	//typedef GivPolynomialRing<Field, Givaro::Dense> PolDom;
	//typedef typename PolDom::Element Polynomial;
	typedef std::vector<typename Field::Element> Polynomial;
	typedef SparseMatrix <Field> Blackbox;

	LinBox::commentator().start ("Testing sparse random charpoly", "testRandomCharpoly", 1);

	bool ret = true;

	VectorDomain<Field> VD (F);
	Vector w, v;
	VectorWrapper::ensureDim (v, v_stream.n ());
	VectorWrapper::ensureDim (w, v_stream.n ());

	A_stream.reset ();
	Blackbox A (F, A_stream);

	ostream &report = LinBox::commentator().report (Commentator::LEVEL_UNIMPORTANT, INTERNAL_DESCRIPTION);
	report << "Matrix:" << endl;
	A.write (report, FORMAT_PRETTY);

	Polynomial phi;

	charpoly (phi, A);

	report << "characteristic polynomial is: ";
	printPolynomial (F, report, phi);

	LinBox::commentator().report (Commentator::LEVEL_IMPORTANT, INTERNAL_DESCRIPTION)
		<< "deg charpoly (A) = " << phi.size () - 1 << endl;

	v_stream.reset ();

	while (v_stream) {
		v_stream.next (v);
		VD.write (report << "Input vector  " << v_stream.j () << ": ", v) << endl;

		applyPoly (F, w, A, phi, v);
		VD.write (report << "Output vector " << v_stream.j () << ": ", w) << endl;

		//bds: VD.isZero fails to work right when -O2 using gcc 4.2
		if (!VD.isZero (w)) { ret = false; break; }
	}

	if (!ret)
		LinBox::commentator().report (Commentator::LEVEL_IMPORTANT, INTERNAL_ERROR)
			<< "ERROR: Output vector was incorrect" << endl;

	LinBox::commentator().stop (MSG_STATUS (ret), (const char *) 0, "testRandomCharpoly");
	return ret;
}
#endif

int main (int argc, char **argv)
{
	bool pass = true;

	std::cout<<setprecision(8);
	std::cerr<<setprecision(8);
	static size_t n = 50;
	static integer q = 33554467U;
	//static integer q = 103U; // 33554467U;
	static int numVectors = 10;
	static int k = 3;

	static Argument args[] = {
		{ 'n', "-n N", "Set dimension of test matrices to NxN.",                 TYPE_INT,     &n },
		{ 'q', "-q Q", "Operate over the \"field\" GF(Q) [1].",          TYPE_INTEGER, &q },
		{ 'v', "-v V", "Use V test vectors for the random charpoly tests.",      TYPE_INT,     &numVectors },
		{ 'k', "-k K", "K nonzero Elements per row in sparse random apply test.", TYPE_INT,     &k },
		END_OF_ARGUMENTS
	};


	parseArguments (argc, argv, args);

	// Temporarily, only Modular<double> is enabled for the givaro/ntl factorization based charpoly
	typedef Modular<double> Field;
	typedef vector<Field::Element> DenseVector;
	typedef SparseMatrix<Field>::Row SparseVector;
	//typedef pair<vector<size_t>, vector<Field::Element> > SparseVector;
	Field F (q);
	srand ((unsigned)time (NULL));

	LinBox::commentator().getMessageClass (TIMING_MEASURE).setMaxDepth (10);
	LinBox::commentator().getMessageClass (INTERNAL_DESCRIPTION).setMaxDepth (10);
	LinBox::commentator().getMessageClass (INTERNAL_DESCRIPTION).setMaxDetailLevel (Commentator::LEVEL_UNIMPORTANT);

	ostream &report = LinBox::commentator().report (Commentator::LEVEL_IMPORTANT, INTERNAL_DESCRIPTION);
	report << endl << "Black box characteristic polynomial of a matrix over a prime field test suite" << endl;

	RandomDenseStream<Field, DenseVector, NonzeroRandIter<Field> >
		v_stream (F, NonzeroRandIter<Field> (F, Field::RandIter (F)), n, numVectors);
	RandomSparseStream<Field, SparseVector, NonzeroRandIter<Field> >
		A_stream (F, NonzeroRandIter<Field> (F, Field::RandIter (F)), (double) k / (double) n, n, n);

	if (!testNilpotentCharpoly (F, n)) pass = false;
	if (!testRandomCharpoly    (F, A_stream, v_stream)) pass = false;

	// symmetrizing
	if (!testIdentityCharpoly  (F, n, true)) pass = false;
	//need other tests...

	typedef vector<PID_integer::Element> ZDenseVector;
	typedef SparseMatrix<PID_integer>::Row ZSparseVector;
	typedef pair<vector<size_t>, vector<Field::Element> > SparseVector;
	PID_integer  Z;
	srand ((unsigned)time (NULL));

	LinBox::commentator().getMessageClass (TIMING_MEASURE).setMaxDepth (10);
	LinBox::commentator().getMessageClass (INTERNAL_DESCRIPTION).setMaxDepth (10);
	LinBox::commentator().getMessageClass (INTERNAL_DESCRIPTION).setMaxDetailLevel (Commentator::LEVEL_UNIMPORTANT);

	report << endl << "Black box characteristic polynomial of an integer matrix test suite" << endl;

	RandomDenseStream<PID_integer, ZDenseVector, NonzeroRandIter<PID_integer> >
		zv_stream (Z, NonzeroRandIter<PID_integer> (Z, PID_integer::RandIter (Z)), n, numVectors);
	RandomSparseStream<PID_integer, SparseVector, NonzeroRandIter<PID_integer> >
		zA_stream (Z, NonzeroRandIter<PID_integer> (Z, PID_integer::RandIter (Z)), (double) k / (double) n, n, n);

	//no symmetrizing
	if (!testIdentityCharpoly  (Z, n)) pass = false;
	if (!testNilpotentCharpoly (Z, n)) pass = false;

	//Comment by Z. Wan. Stream doesn't work here
	//if (!testRandomCharpoly    (Z, zA_stream, zv_stream)) pass = false;

	// symmetrizing
	if (!testIdentityCharpoly  (Z, n, true)) pass = false;
	//need other tests...

	return pass ? 0 : -1;
}

// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,:0,t0,+0,=s
// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:

