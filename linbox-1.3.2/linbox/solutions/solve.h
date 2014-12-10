/* linbox/solutions/solve.h
 * Copyright(C) LinBox
 *  Evolved from an earlier one by Bradford Hovinen <hovinen@cis.udel.edu>
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

#ifndef __LINBOX_solve_H
#define __LINBOX_solve_H

/*! @file linbox/solutions/solve.h
 * @ingroup solutions
 * @brief System Solving.
 * @details NO DOC
 */

#include <vector>
#include <algorithm>

// must fix this list...
#include "linbox/algorithms/gauss.h"
#include "linbox/algorithms/gauss-gf2.h"
#include "linbox/algorithms/wiedemann.h"
#include "linbox/algorithms/rational-solver.h"
#include "linbox/algorithms/diophantine-solver.h"
#include "linbox/matrix/factorized-matrix.h"
#include "linbox/util/debug.h"
#include "linbox/util/error.h"
#include "linbox/vector/vector-domain.h"
#include "linbox/solutions/methods.h"
#include "linbox/algorithms/bbsolve.h"

#include "linbox/algorithms/rational-cra2.h"
#include "linbox/algorithms/varprec-cra-early-multip.h"

#ifdef __LINBOX_HAVE_IML
#include "linbox/util/iml_wrapper.h"
#endif

namespace LinBox
{

	// for specialization with respect to the DomainCategory
	template< class Vector, class Blackbox, class SolveMethod, class DomainCategory>
	Vector & solve (Vector & 			x,
			const Blackbox &                A,
			const Vector &			b,
			const DomainCategory &        tag,
			const SolveMethod &            M);

	/** \brief Solve Ax = b, for x.
	 *
	 * Vector x such that Ax = b is returned.  In the case of a singular
	 * matrix A, if the system is consistent, a random solution is returned
	 * by default.  The method parameter may contain an indication that an
	 * arbitrary element of the solution space is acceptable, which can be
	 * faster to compute.  If the system is inconsistent the zero vector is
	 * returned.
	 *
	 * @warning no doc for when using what method
	 *
	 * @param [out] x solution
	 * @param [in]  A matrix
	 * @param [in]  b target
	 * @param [in]  M method to use (\see solutions/method.h)
	 * @return reference to \p x
	 */
	 // * \ingroup solutions
	//and the SolveStatus, if non-null, is set to indicate inconsistency.
	template< class Vector, class Blackbox, class SolveMethod>
	Vector & solve (Vector &        		x,
			const Blackbox &                A,
			const Vector &			b,
			const SolveMethod &             M)
	{
		return solve(x, A, b, typename FieldTraits<typename Blackbox::Field>::categoryTag(), M);
	}

	/*!
	 * the solve with default method.
	 */
	template< class Vector, class Blackbox>
	Vector& solve(Vector& x, const Blackbox& A, const Vector& b)
	{
		return solve(x, A, b, Method::Hybrid());
	}

	// in methods.h FoobarMethod and Method::Foobar are the same class.
	// in methods.h template<BB> bool useBB(const BB& A) is defined.

	//! @internal specialize this on blackboxes which have local methods
	template <class Vector, class BB>
	Vector& solve(Vector& x, const BB& A, const Vector& b,
		      const Method::Hybrid& m)
	{
		if (useBB(A)) return solve(x, A, b, Method::Blackbox(m));
		else return solve(x, A, b, Method::Elimination(m));
	}

	/**  @internal Blackbox method specialisation */
	template <class Vector, class BB>
	Vector& solve(Vector& x, const BB& A, const Vector& b,
		      const Method::Blackbox& m)
	{
		// what is chosen here should be best and/or most reliable currently available choice
		// 		integer c; A.field().cardinality(c);
		// 		if (c < 100) return solve(x, A, b, Method::BlockLanczos(m));
		return solve(x, A, b, Method::Wiedemann(m));
	}

	//x @todo temporary - fix this
	//#define inBlasRange(p) true

	/**  @internal Elimination method specialisation */
	template <class Vector, class BB>
	Vector& solve(Vector& x, const BB& A, const Vector& b,
		      const Method::Elimination& m)
	{
		integer c, p;
		A.field().cardinality(c);
		A.field().characteristic(p);
		//if ( p == 0 || (c == p && inBlasRange(p)) )
		return solve(x, A, b,
			     typename FieldTraits<typename BB::Field>::categoryTag(),
			     Method::BlasElimination(m));
		//else
		//	return solve(x, A, b,
		//			typename FieldTraits<typename BB::Field>::categoryTag(),
		//			Method::NonBlasElimination(m));
	}

	//! @internal inplace Sparse Elimination.
	template <class Vector, class Field>
	Vector& solvein(Vector& x, SparseMatrix<Field, typename LinBox::Vector<Field>::SparseSeq>& A, const Vector& b, const Method::SparseElimination& m)
	{
		commentator().start ("Sparse Elimination Solve In Place", "sesolvein");
		GaussDomain<Field> GD ( A.field() );
		GD.solvein(x, A, b);
		commentator().stop ("done", NULL, "sesolvein");
		return x;
	}

	template <class Vector, class Field, class Random>
	Vector& solvein(Vector& x, SparseMatrix<Field, typename LinBox::Vector<Field>::SparseSeq>& A, const Vector& b, const Method::SparseElimination& m, Random& generator)
	{
		commentator().start ("Sparse Elimination Solve In Place with random solution", "sesolvein");
		GaussDomain<Field> GD ( A.field() );
		GD.solvein(x, A, b, generator);
		commentator().stop ("done", NULL, "sesolvein");
		return x;
	}


	//! @internal  Change of representation to be able to call the sparse elimination
	template <class Vector, class Blackbox>
	Vector& solve(Vector& x, const Blackbox& A, const Vector& b,
		      const Method::SparseElimination& m)
	{
		typedef typename Blackbox::Field Field;
		typedef SparseMatrix<Field, typename LinBox::Vector<Field>::SparseSeq> SparseBB;
		SparseBB SpA(A.field(), A.rowdim(), A.coldim());
		MatrixHom::map(SpA, A, A.field());
		return solvein(x, SpA, b, m);
	}

	template <class Vector, class Blackbox, class Random>
	Vector& solve(Vector& x, const Blackbox& A, const Vector& b,
		      const Method::SparseElimination& m, Random& generator)
	{
		typedef typename Blackbox::Field Field;
		typedef SparseMatrix<Field, typename LinBox::Vector<Field>::SparseSeq> SparseBB;
		SparseBB SpA(A.field(), A.rowdim(), A.coldim());
		MatrixHom::map(SpA, A, A.field());
		return solvein(x, SpA, b, generator);
	}

	//! @internal specialisation for inplace SparseElimination on GF2
	template <class Vector>
	Vector& solvein(Vector& x,
			GaussDomain<GF2>::Matrix    &A,
			const Vector& b,
			const Method::SparseElimination& m)
	{
		commentator().start ("Sparse Elimination Solve In Place over GF2", "GF2sesolvein");
		GaussDomain<GF2> GD ( A.field() );
		GD.solvein(x, A, b);
		commentator().stop ("done", NULL, "GF2sesolvein");
		return x;
	}
	template <class Vector, class Random>
	Vector& solvein(Vector& x,
			GaussDomain<GF2>::Matrix    &A,
			const Vector& b,
			const Method::SparseElimination& m, 
            Random& generator)
	{
		commentator().start ("Sparse Elimination Solve In Place over GF2", "GF2sesolvein");
		GaussDomain<GF2> GD ( A.field() );
		GD.solvein(x, A, b, generator);
		commentator().stop ("done", NULL, "GF2sesolvein");
		return x;
	}

	//! @internal specialisation for SparseElimination on GF2
	template <class Vector>
	Vector& solve(Vector& x,
		      GaussDomain<GF2>::Matrix    &A,
		      const Vector& b,
		      const Method::SparseElimination& m)
	{
		// We make a copy
		GaussDomain<GF2>::Matrix SpA(A.field(), A.rowdim(), A.coldim());
		MatrixHom::map(SpA, A );
		return solvein(x, SpA, b, m);
	}
	template <class Vector, class Random>
	Vector& solve(Vector& x,
		      GaussDomain<GF2>::Matrix    &A,
		      const Vector& b,
		      const Method::SparseElimination& m,
              Random& generator)
	{
		// We make a copy
		GaussDomain<GF2>::Matrix SpA(A.field(), A.rowdim(), A.coldim());
		MatrixHom::map(SpA, A );
		return solvein(x, SpA, b, m, generator);
	}

	//! @internal Generic Elimination for SparseMatrix
	template <class Vector, class Field>
	Vector& solve(Vector& x, const SparseMatrix<Field>& A, const Vector& b,
		      const Method::Elimination& m)
	{
		//             bool consistent = false;
		// sparse elimination based solver can be called here ?
		// For now we call the dense one

		return solve(x, A, b,
			     typename FieldTraits<typename SparseMatrix<Field>::Field>::categoryTag(),
			     Method::BlasElimination(m));

#if 0
		if ( ! consistent ) {  // we will return the zero vector
			typename Field::Element zero; A.field().init(zero, 0);
			for (typename Vector::iterator i = x.begin(); i != x.end(); ++i) *i = zero;
		}
		return x;
#endif
	}
	// BlasElimination section ///////////////////

	//! @internal Generic Elimination on Z/pZ (convert A to DenseMatrix)
	template <class Vector, class BB>
	Vector& solve(Vector& x, const BB& A, const Vector& b,
		      const RingCategories::ModularTag & tag,
		      const Method::BlasElimination& m)
	{
		BlasMatrix<typename BB::Field> B(A); // copy A into a BlasMatrix
		return solve(x, B, b, tag, m);
	}

	//! @internal Generic Elimination for DenseMatrix on Z/pZ
	template <class Vector, class Field>
	Vector& solve(Vector& x, const BlasMatrix<Field>& A, const Vector& b,
		      const RingCategories::ModularTag & tag,
		      const Method::BlasElimination& m)
	{
		if ((A.coldim() != x.size()) || (A.rowdim() != b.size()))
			throw LinboxError("LinBox ERROR: dimension of data are not compatible in system solving (solving impossible)");

		commentator().start ("Solving linear system (FFLAS LQUP)", "LQUP::left_solve");
		//bool consistent = false;
		LQUPMatrix<Field> LQUP(A);
		//FactorizedMatrix<Field> LQUP(A);

		LQUP.left_solve(x, b);

#if 0
		// this should be implemented directly in left_solve
		if ( ! consistent ) {  // we will return the zero vector
			typename Field::Element zero;
			A.field().init(zero, 0);
			for (typename Vector::iterator i = x.begin(); i != x.end(); ++i)
				*i = zero;
		}
#endif
		commentator().stop ("done", NULL, "LQUP::left_solve");
		return x;
	}

	template <class Vector, class Field>
	Vector& solve(Vector& x, const BlasMatrix<Field>& A, const Vector& b,
		      const RingCategories::ModularTag & tag,
		      const Method::Dixon & m)
	{
		throw LinBoxFailure("You cannot do this");
	}

	/* Integer tag Specialization for Dixon method:
	 * 2 interfaces:
	 *   - the output is a common denominator and a vector of numerator (no need of rational number)
	 *   - the output is a vector of rational
	 */


	// error handler for bad use of the integer solver API
	//! @internal Generic Elimination for  Integer matrices
	template <class Vector, class BB>
	Vector& solve(Vector& x, const BB& A, const Vector& b,
		      const RingCategories::IntegerTag & tag,
		      const Method::BlasElimination& m)
	{
		std::cout<<"try to solve system over the integer\n"
		<<"the API need either \n"
		<<" - a vector of rational as the solution \n"
		<<" - or an integer for the common denominator and a vector of integer for the numerators\n\n";
		throw LinboxError("bad use of integer API solver\n");

	}

#if 0
	template <class RatVector, class Vector, class BB, class MethodTraits>
	Vector& solve(RatVector& x, const BB& A, const Vector& b,
		      const RingCategories::RationalTag & tag,
		      const MethodTraits& m)
	{
		if ((A.coldim() != x.size()) || (A.rowdim() != b.size()))
			throw LinboxError("LinBox ERROR: dimension of data are not compatible in system solving (solving impossible)");

		commentator().start ("Rational CRA Solve", "Rsolve");
		size_t bits = 26 -(int)ceil(log((double)A.rowdim())*0.7213475205);
		RandomPrimeIterator genprime( bits);

		RationalRemainder2< VarPrecEarlyMultipCRA< Modular<double> > > rra(3UL);//using default RR method
		IntegerModularSolve<BB,Vector,MethodTraits > iteration(A, b, m);
		integer den;
		std::vector< integer > num(A.coldim());

		rra(num, den, iteration, genprime);

		typename RatVector::iterator it_x= x.begin();
		typename std::vector<integer>::const_iterator it_num= num.begin();

		for (; it_x != x.end(); ++it_x, ++it_num){
			integer g = gcd( *it_num, den);
			*it_x = typename RatVector::value_type(*it_num/g, den/g);
		}

		commentator().stop ("done", NULL, "Rsolve");
		return x;
	}
#endif
	// error handler for non defined solver over rational domain
#if 0
	template <class Vector, class BB, class MethodTraits>
	Vector& solve(Vector& x, const BB& A, const Vector& b,
		      const RingCategories::RationalTag & tag,
		      const MethodTraits& m)
	{
		throw LinboxError("LinBox ERROR: solver not yet defined over rational domain");
	}
#endif

	/*
	 * 1st integer solver API :
	 * solution is a vector of rational numbers
	 * RatVector is assumed to be the type of a vector of rational number
	 */

	// default API (method is BlasElimination)
	template<class RatVector, class Vector, class BB>
	RatVector& solve(RatVector& x, const BB &A, const Vector &b)
	{
		return solve(x, A, b, Method::BlasElimination());
	}

	// API with Hybrid method
	template<class RatVector, class Vector, class BB>
	RatVector& solve(RatVector& x, const BB &A, const Vector &b,
			 const Method::Hybrid &m)
	{
		if (useBB(A))
			return solve(x, A, b, Method::Blackbox(m));
		else
			return solve(x, A, b, Method::Elimination(m));
	}

	// API with Blackbox method
	template<class RatVector, class Vector, class BB>
	RatVector& solve(RatVector& x, const BB &A, const Vector &b, const Method::Blackbox &m)
	{
		return solve(x, A, b, Method::Wiedemann(m));
	}

	// API with Elimination method
	template<class RatVector, class Vector, class BB>
	RatVector& solve(RatVector& x, const BB &A, const Vector &b, const Method::Elimination &m)
	{
		return solve(x, A, b,  Method::BlasElimination(m));
	}


	// launcher of specialized solver depending on the MethodTrait
	template<class RatVector, class Vector, class BB, class MethodTraits>
	RatVector& solve(RatVector& x, const BB &A, const Vector &b, const MethodTraits &m)
	{
		return solve(x, A, b, typename FieldTraits<typename BB::Field>::categoryTag(),  m);
	}


	/* Specializations for BlasElimination over the integers
	*/

	// input matrix is generic (copying it into a BlasMatrix)
	template <class RatVector, class Vector, class BB>
	RatVector& solve(RatVector& x, const BB& A, const Vector& b,
			 const RingCategories::IntegerTag & tag,
			 const Method::BlasElimination& m)
	{
		BlasMatrix<typename BB::Field> B(A); // copy A into a BlasMatrix
		return solve(x, B, b, tag, m);
	}

	// input matrix is a BlasMatrix (no copy)
	template <class RatVector, class Vector, class Ring>
	RatVector& solve(RatVector& x, const BlasMatrix<Ring>& A, const Vector& b,
			 const RingCategories::IntegerTag & tag,
			 const Method::BlasElimination& m)
	{

		Method::Dixon mDixon(m);
		typename Ring::Element d;
		std::vector< typename Ring::Element> num(A.coldim());
		solve (num, d, A, b, tag, mDixon);

		typename RatVector::iterator it_x= x.begin();
		typename std::vector< typename Ring::Element>::const_iterator it_num= num.begin();
		integer n,den;
		A.field().convert(den,d);
		for (; it_x != x.end(); ++it_x, ++it_num){
			A.field().convert(n, *it_num);
			*it_x = typename RatVector::value_type(n, den);
		}

		return x;
	}

	/*!
	 * 2nd integer solver API :
	 * solution is a formed by a common denominator and a vector of integer numerator
	 * solution is num/d
	 * BB: why not a struct RatVector2 { IntVector _n ; Int _d } ; ?
	 */

	//@{

	// default API (method is BlasElimination)
	template< class Vector, class BB>
	Vector& solve(Vector &x, typename BB::Field::Element &d, const BB &A, const Vector &b)
	{
		return solve(x, d, A, b, typename FieldTraits<typename BB::Field>::categoryTag(),  Method::BlasElimination());
	}

	// launcher of specialized solver depending on the MethodTraits
	template< class Vector, class BB, class MethodTraits>
	Vector& solve(Vector &x, typename BB::Field::Element &d, const BB &A, const Vector &b, const MethodTraits &m)
	{
		return solve(x, d, A, b, typename FieldTraits<typename BB::Field>::categoryTag(), m);
	}

	/* Specialization for BlasElimination over the integers
	*/

	// input matrix is generic (copying it into a BlasMatrix)
	template <class Vector, class BB>
	Vector& solve(Vector& x, typename BB::Field::Element &d, const BB& A, const Vector& b,
		      const RingCategories::IntegerTag & tag,
		      const Method::BlasElimination& m)
	{
		BlasMatrix<typename BB::Field> B(A); // copy A into a BlasMatrix
		return solve(x, d, B, b, tag, m);
	}

	// input matrix is a BlasMatrix (no copy)
	template <class Vector, class Ring>
	Vector& solve(Vector& x, typename Ring::Element &d,
		      const BlasMatrix<Ring>& A, const Vector& b,
		      const RingCategories::IntegerTag & tag,
		      const Method::BlasElimination& m)
	{
		//!@bug check we don't copy
		Method::Dixon mDixon(m);
		return solve(x, d, A, b, tag, mDixon);
	}

	// input matrix is a SparseMatrix (no copy)
	template <class Vect, class Ring>
	Vect& solve(Vect& x, typename Ring::Element &d,
		    const SparseMatrix<Ring, typename Vector<Ring>::SparseSeq>& A,
		    const Vect& b,
		    const RingCategories::IntegerTag & tag,
		    const Method::SparseElimination& m)
	{
		Method::Dixon mDixon(m);
		return solve(x, d, A, b, tag, mDixon);
	}



	/** \brief solver specialization with the 2nd API and DixonTraits over integer (no copying)
	*/
	template <class Vector, class Ring>
	Vector& solve(Vector& x, typename Ring::Element &d,
		      const BlasMatrix<Ring>& A,
		      const Vector& b,
		      const RingCategories::IntegerTag tag, Method::Dixon& m)
	{
		if ((A.coldim() != x.size()) || (A.rowdim() != b.size()))
			throw LinboxError("LinBox ERROR: dimension of data are not compatible in system solving (solving impossible)");

		commentator().start ("Padic Integer Blas-based Solving", "solving");

		typedef Modular<double> Field;
		// 0.7213475205 is an upper approximation of 1/(2log(2))
		RandomPrimeIterator genprime( 26-(int)ceil(log((double)A.rowdim())*0.7213475205));
		RationalSolver<Ring, Field, RandomPrimeIterator, DixonTraits> rsolve(A.field(), genprime);
		SolverReturnStatus status = SS_OK;

		// if singularity unknown and matrix is square, we try nonsingular solver
		switch ( m.singular() ) {
		case Specifier::SINGULARITY_UNKNOWN:
			switch (A.rowdim() == A.coldim() ?
				status=rsolve.solveNonsingular(x, d, A, b, false ,(int)m.maxTries()) : SS_SINGULAR) {
			case SS_OK:
				m.singular(Specifier::NONSINGULAR);
				break;
			case SS_SINGULAR:
				switch (m.solution()){
				case DixonTraits::DETERMINIST:
					status= rsolve.monolithicSolve(x, d, A, b, false, false, (int)m.maxTries(),
								       (m.certificate()? SL_LASVEGAS: SL_MONTECARLO));
					break;
				case DixonTraits::RANDOM:
					status= rsolve.monolithicSolve(x, d, A, b, false, true, (int)m.maxTries(),
								       (m.certificate()? SL_LASVEGAS: SL_MONTECARLO));
					break;
				case DixonTraits::DIOPHANTINE:
					{
						DiophantineSolver<RationalSolver<Ring,Field,RandomPrimeIterator, DixonTraits> > dsolve(rsolve);
						status= dsolve.diophantineSolve(x, d, A, b, (int)m.maxTries(),
										(m.certificate()? SL_LASVEGAS: SL_MONTECARLO));
					}
					break;
				default:
					break;
				}
				break;
			default:
				break;
			}
			break;

		case Specifier::NONSINGULAR:
			rsolve.solveNonsingular(x, d, A, b, false ,(int)m.maxTries());
			break;

		case Specifier::SINGULAR:
			switch (m.solution()){
			case DixonTraits::DETERMINIST:
				status= rsolve.monolithicSolve(x, d, A, b,
							       false, false, (int)m.maxTries(),
							       (m.certificate()? SL_LASVEGAS: SL_MONTECARLO));
				break;

			case DixonTraits::RANDOM:
				status= rsolve.monolithicSolve(x, d, A, b,
							       false, true, (int)m.maxTries(),
							       (m.certificate()? SL_LASVEGAS: SL_MONTECARLO));
				break;

			case DixonTraits::DIOPHANTINE:
				{
					DiophantineSolver<RationalSolver<Ring,Field,RandomPrimeIterator, DixonTraits> > dsolve(rsolve);
					status= dsolve.diophantineSolve(x, d, A, b, (int)m.maxTries(),
									(m.certificate()? SL_LASVEGAS: SL_MONTECARLO));
				}
				break;

				//default:
				//	break;
			}
		default:
			break;
		}

		commentator().stop("done", NULL, "solving");

		if ( status == SS_INCONSISTENT ) {
			throw LinboxMathInconsistentSystem("Linear system is inconsistent");
			//			typename Ring::Element zero; A.field().init(zero, 0);
			// 			for (typename Vector::iterator i = x.begin(); i != x.end(); ++i) *i = zero;
		}
		return x;
	}

	/** \brief solver specialization with the 2nd API and DixonTraits over integer (no copying)
	*/
	template <class Vect, class Ring>
	Vect& solve(Vect& x, typename Ring::Element &d,
		    const SparseMatrix<Ring, typename Vector<Ring>::SparseSeq> & A,
		    const Vect& b,
		    const RingCategories::IntegerTag tag,
		    Method::Dixon& m)
	{
		if ((A.coldim() != x.size()) || (A.rowdim() != b.size()))
			throw LinboxError("LinBox ERROR: dimension of data are not compatible in system solving (solving impossible)");

		commentator().start ("Padic Integer Sparse Elimination Solving", "solving");

		typedef Modular<double> Field;
		// 0.7213475205 is an upper approximation of 1/(2log(2))
		RandomPrimeIterator genprime( 26-(int)ceil(log((double)A.rowdim())*0.7213475205));
		RationalSolver<Ring, Field, RandomPrimeIterator, SparseEliminationTraits> rsolve(A.field(), genprime);
		SolverReturnStatus status = SS_OK;

		// if singularity unknown and matrix is square, we try nonsingular solver
		switch ( m.singular() ) {
		case Specifier::SINGULARITY_UNKNOWN:
			switch (A.rowdim() == A.coldim() ?
				status=rsolve.solveNonsingular(x, d, A, b,(int)m.maxTries()) : SS_SINGULAR) {
			case SS_OK:
				m.singular(Specifier::NONSINGULAR);
				break;
#if 0
			case SS_SINGULAR:
				switch (m.solution()){
				case DixonTraits::DETERMINIST:
					status= rsolve.monolithicSolve(x, d, A, b, false, false, (int)m.maxTries(),
								       (m.certificate()? SL_LASVEGAS: SL_MONTECARLO));
					break;
				case DixonTraits::RANDOM:
					status= rsolve.monolithicSolve(x, d, A, b, false, true, (int)m.maxTries(),
								       (m.certificate()? SL_LASVEGAS: SL_MONTECARLO));
					break;
				case DixonTraits::DIOPHANTINE:
					{
						DiophantineSolver<RationalSolver<Ring,Field,RandomPrimeIterator, DixonTraits> > dsolve(rsolve);
						status= dsolve.diophantineSolve(x, d, A, b, (int)m.maxTries(),
										(m.certificate()? SL_LASVEGAS: SL_MONTECARLO));
					}
					break;
				default:
					break;
				}
				break;
#endif
			default:
				break;
			}
			break;

		case Specifier::NONSINGULAR:
			rsolve.solveNonsingular(x, d, A, b, (int)m.maxTries());
			break;

		case Specifier::SINGULAR:
#if 0
			switch (m.solution()){
			case DixonTraits::DETERMINIST:
				status= rsolve.monolithicSolve(x, d, A, b,
							       false, false, (int)m.maxTries(),
							       (m.certificate()? SL_LASVEGAS: SL_MONTECARLO));
				break;

			case DixonTraits::RANDOM:
				status= rsolve.monolithicSolve(x, d, A, b,
							       false, true, (int)m.maxTries(),
							       (m.certificate()? SL_LASVEGAS: SL_MONTECARLO));
				break;

			case DixonTraits::DIOPHANTINE:
				{
					DiophantineSolver<RationalSolver<Ring,Field,RandomPrimeIterator, DixonTraits> > dsolve(rsolve);
					status= dsolve.diophantineSolve(x, d, A, b, (int)m.maxTries(),
									(m.certificate()? SL_LASVEGAS: SL_MONTECARLO));
				}
				break;

				//default:
				//	break;
			}
#endif
		default:
			break;
		}

		commentator().stop("done", NULL, "solving");

		if ( status == SS_INCONSISTENT ) {
			throw LinboxMathInconsistentSystem("Linear system is inconsistent");
			//			typename Ring::Element zero; A.field().init(zero, 0);
			// 			for (typename Vect::iterator i = x.begin(); i != x.end(); ++i) *i = zero;
		}
		return x;
	}

	//@}

	// NonBlasElimination section ////////////////

	template <class Vector, class BB>
	Vector& solve(Vector& x, const BB& A, const Vector& b,
		      const RingCategories::ModularTag & tag,
		      const Method::NonBlasElimination& m)
	{
		BlasMatrix<typename BB::Field> B(A); // copy
		return solve(x, B, b, tag, m);
	}

	// note: no need for NonBlasElimination when RingCategory is integer

	// Lanczos ////////////////
	// may throw SolverFailed or InconsistentSystem

	// Wiedemann section ////////////////

	// may throw SolverFailed or InconsistentSystem
	template <class Vector, class BB>
	Vector& solve(Vector& x, const BB& A, const Vector& b,
		      const RingCategories::ModularTag & tag,
		      const Method::Wiedemann& m)
	{
		if ((A.coldim() != x.size()) || (A.rowdim() != b.size()))
			throw LinboxError("LinBox ERROR: dimension of data are not compatible in system solving (solving impossible)");

		// adapt to earlier signature of wiedemann solver
		solve(A, x, b, A.field(), m);
		return x;
	}


	/* remark 1.  I used copy constructors when switching method types.
	   But if the method types are (empty) child classes of a common  parent class containing
	   all the information, then casts can be used in place of copies.
	   */

} // LinBox



#include "linbox/field/modular.h"
#include "linbox/algorithms/rational-cra.h"
#include "linbox/algorithms/rational-cra-early-multip.h"
#include "linbox/randiter/random-prime.h"
#include "linbox/algorithms/matrix-hom.h"
#include "linbox/vector/vector-traits.h"

namespace LinBox
{ /*  Integer */


	template <class Blackbox, class Vector, class MyMethod>
	struct IntegerModularSolve {
		const Blackbox &A;
		const Vector &B;
		const MyMethod &M;

		IntegerModularSolve(const Blackbox& b, const Vector& v, const MyMethod& n) :
			A(b), B(v), M(n)
		{}


		template<typename Field>
		typename Rebind<Vector, Field>::other& operator()(typename Rebind<Vector, Field>::other& x, const Field& F) const
		{
			typedef typename Blackbox::template rebind<Field>::other FBlackbox;
			FBlackbox Ap(F, A.rowdim(), A.coldim());
			MatrixHom::map(Ap, A, F);

			typedef typename Rebind<Vector, Field>::other FVector;
			Hom<typename Blackbox::Field, Field> hom(A.field(), F);
			FVector Bp(B.size());
			typename Vector::const_iterator Bit = B.begin();
			typename FVector::iterator      Bpit = Bp.begin();
			for( ; Bit != B.end(); ++Bit, ++Bpit)
				hom.image (*Bpit, *Bit);

			VectorWrapper::ensureDim (x, A.coldim());
			return solve( x, Ap, Bp, M);
		}
	};


	//BB: How come I have to change the name so it works when directly called ?
	template <class Vector, class BB, class MyMethod>
	Vector& solveCRA(Vector& x, typename BB::Field::Element& d, const BB& A, const Vector& b,
		      const RingCategories::IntegerTag & tag,
		      const MyMethod& M)
	{
		if ((A.coldim() != x.size()) || (A.rowdim() != b.size()))
			throw LinboxError("LinBox ERROR: dimension of data are not compatible in system solving (solving impossible)");

		commentator().start ("Integer CRA Solve", "Isolve");

		RandomPrimeIterator genprime( 26 -(int)ceil(log((double)A.rowdim())*0.7213475205));
		//         RationalRemainder< Modular<double> > rra((double)
		//                                                  ( A.coldim()/2.0*log((double) A.coldim()) ) );

		RationalRemainder< EarlyMultipRatCRA< Modular<double> > > rra(3UL);
		IntegerModularSolve<BB,Vector,MyMethod> iteration(A, b, M);

		// use of integer due to non genericity of rra (PG 2005-09-01)
		Integer den;
		std::vector< Integer > num(A.coldim());
		rra(num, den, iteration, genprime);
		//rra(x, d, iteration, genprime);

		typename Vector::iterator it_x= x.begin();
		typename std::vector<Integer>::const_iterator it_num= num.begin();

		// convert the result
		for (; it_x != x.end(); ++it_x, ++it_num)
			A.field().init(*it_x, *it_num);
		A.field().init(d, den);

		commentator().stop ("done", NULL, "Isolve");
		return x;
	}

	//BB: How come SparseElimination needs this ?
	// may throw SolverFailed or InconsistentSystem
	template <class Vector, class BB, class MyMethod>
	Vector& solve(Vector& x, typename BB::Field::Element& d, const BB& A, const Vector& b,
		      const RingCategories::IntegerTag & tag,
		      const MyMethod& M)
	{
		return solveCRA(x,d,A,b,tag,M);
	}


#if 0 /*  not working */
	template <class Vector, class Field>
	Vector& solve(Vector& x, typename Field::Element& d, const BlasMatrix<Field>& A, const Vector& b,
		      const RingCategories::IntegerTag & tag,
		      const Method::CRA & M)
	{
		return solve(x,d,A,b,tag,M.iterationMethod());
	}

	template <class Vector, class BB>
	Vector& solve(Vector& x, typename BB::Field::Element& d, const BB& A, const Vector& b,
		      const RingCategories::ModularTag & tag,
		      const Method::CRA& m)
	{
		return solve(x, d, A, b, tag, m.iterationMethod());
	}
#endif

#ifdef __LINBOX_HAVE_IML
	//! IML wrapper.
	//! @bug not recognised as template spec...
	std::vector<PID_integer::Element>&
	solveIML(std::vector<PID_integer::Element>& x, PID_integer::Element & d,
			 const BlasMatrix<PID_integer>& B, const std::vector<PID_integer::Element>& b,
			 const Method::IML& m)
	{
		THIS_CODE_COMPILES_BUT_IS_NOT_TESTED; // NOT MUCH
		switch (m.routine()) {
		case(1) : { /*  non singular */
					  linbox_check(B.rowdim()==B.coldim());
					  mpz_t * mp_A = REINTERP_IML_CONST(B.getPointer()) ;
					  // reinterpret_cast<mpz_t*>(const_cast<PID_integer::Element*>((B.getPointer())));
					  //B.getConstPointer() ?
					  mpz_t * mp_B = REINTERP_IML_CONST(&b[0]);
					  mpz_t * mp_N = REINTERP_IML(&x[0]);
					  mpz_t mp_D ;
					  mpz_init(mp_D);
				  if (!m.computeRNS()) {
					  IML::nonsingSolvLlhsMM(IML::RightSolu,B.rowdim(),1,
								 mp_A, mp_B, mp_N, mp_D);
				  }
				  else {
					  long n = B.coldim();
					  long basislen = 1;
					  mpz_t mp_alpha, mp_maxInter;
					  IML::FiniteField qh;
					  IML::FiniteField *basis, **basiscmb;
					  IML::Double ** ARNS ;
					  mpz_init(mp_alpha);
					  IML::maxMagnMP(mp_A, n, n, n, mp_alpha);
					  mpz_init_set_ui(mp_maxInter, 1);
					  mpz_addmul_ui(mp_maxInter, mp_alpha, 2);
					  qh            = IML::RNSbound(n);
					  basiscmb      = IML::findRNS(qh, mp_maxInter, &basislen);
					  basis = basiscmb[0];
					  mpz_clear(mp_maxInter);
					  mpz_clear(mp_alpha);
					  /*  CRNS[i] = [A_11, A_12] mod basis[i] */
					  ARNS = IML_XMALLOC(IML::Double *, basislen);
					  for (long i = 0; i < basislen; ++i)
					  {
						  ARNS[i] = IML_XMALLOC(IML::Double, n*n);
						  for (long j = 0; j < n; ++j)
							  for (long l = 0; l < n; l++)
								  ARNS[i][j*n+l] = (IML::Double) mpz_fdiv_ui(mp_A[j * n + l], basis[i]);
					  }
					  IML::nonsingSolvRNSMM(IML::RightSolu, n, 1, basislen,
								basis, ARNS, mp_B, mp_N, mp_D);

				  }
				  mpz_set(d.get_mpz(),mp_D);
				  mpz_clear(mp_D);

			  }
			  //!@todo wrap nonsingSolvRNSMM too
			  break;
		case (2) : { /*  certified */
				   mpz_t * mp_A = REINTERP_IML_CONST(B.getPointer()) ;
				   // reinterpret_cast<mpz_t*>(const_cast<PID_integer::Element*>((B.getPointer())));
				   //B.getConstPointer() ?
				   mpz_t * mp_b = REINTERP_IML_CONST(&b[0]);
				   mpz_t * mp_N = REINTERP_IML(&x[0]);
				   mpz_t mp_D ;
				   mpz_init(mp_D);
				   mpz_t * mp_NZ = NULL;
				   mpz_t mp_DZ ;
				   if (m.certificate()) {
					   mp_NZ = IML_XMALLOC(mpz_t,x.size());
					   for (size_t i = 0; i < x.size(); ++i) { mpz_init(mp_NZ[i]); }
					   mpz_init(mp_DZ);
				   }
				   if ( !m.reduced() ) {
					   IML::certSolveMP(m.certificate(),
							    B.rowdim(),B.coldim(),
							    mp_A, mp_b, mp_N, mp_D,
							    mp_NZ, mp_DZ);
				   }
				   else {
					   IML::certSolveRedMP(m.certificate(),
							       m.nullcol() //NULLSPACE_COLUMN
							       ,B.rowdim(),B.coldim(),
							       mp_A, mp_b, mp_N, mp_D,
							       mp_NZ, mp_DZ);

				   }
				   mpz_set(d.get_mpz(),mp_D);
				   mpz_clear(mp_D);
			   }
			   break;
		default :
			   throw LinBoxError("unknownn routine from IML (choice 1/2). Got XXX");
		}
		return x;
	}
#endif

	template <class RatVector, class Vector, class BB, class MyMethod>
	RatVector& solve(RatVector& x, const BB& A, const Vector& b,
			 const RingCategories::IntegerTag & tag,
			 const MyMethod& M)
	{
		if ((A.coldim() != x.size()) || (A.rowdim() != b.size()))
			throw LinboxError("LinBox ERROR: dimension of data are not compatible in system solving (solving impossible)");

		commentator().start ("Rational CRA Solve", "Rsolve");
		typename BB::Field::Element den;
		std::vector<typename BB::Field::Element > num(A.coldim());
		solve (num, den, A, b, tag, M);
		typename RatVector::iterator it_x= x.begin();
		typename std::vector<typename BB::Field::Element>::const_iterator it_num= num.begin();
		integer n,d;
		A.field().convert(d,den);
		for (; it_x != x.end(); ++it_x, ++it_num){
			A.field().convert(n, *it_num);
			*it_x = typename RatVector::value_type(n, d);
		}
		commentator().stop ("done", NULL, "Rsolve");
		return x;
	}

	template <class RatVector, class Vector, class BB, class MethodTraits>
	RatVector& solve(RatVector& x, const BB& A, const Vector& b,
			 const RingCategories::RationalTag & tag,
			 const MethodTraits& m)
	{
		if ((A.coldim() != x.size()) || (A.rowdim() != b.size()))
			throw LinboxError("LinBox ERROR: dimension of data are not compatible in system solving (solving impossible)");
		commentator().start ("Rational CRA Solve", "Rsolve");
		size_t bits = 26 -(int)ceil(log((double)A.rowdim())*0.7213475205);
		RandomPrimeIterator genprime( (unsigned) bits);
		RationalRemainder2< VarPrecEarlyMultipCRA< Modular<double> > > rra(3UL);//using default RR method
		IntegerModularSolve<BB,Vector,MethodTraits > iteration(A, b, m);
		integer den;
		std::vector< integer > num(A.coldim());
		rra(num, den, iteration, genprime);
		typename RatVector::iterator it_x= x.begin();
		typename std::vector<integer>::const_iterator it_num= num.begin();
		for (; it_x != x.end(); ++it_x, ++it_num){
			integer g = gcd( *it_num, den);
			*it_x = typename RatVector::value_type(*it_num/g, den/g);
		}
		commentator().stop ("done", NULL, "Rsolve");
		return x;
	}

	template <class RatVector, class BB, class MethodTraits>
	RatVector& solve(RatVector& x, const BB& A, const RatVector& b,
			 const RingCategories::RationalTag & tag,
			 const MethodTraits& m)
	{
		if ((A.coldim() != x.size()) || (A.rowdim() != b.size()))
			throw LinboxError("LinBox ERROR: dimension of data are not compatible in system solving (solving impossible)");
		commentator().start ("Rational CRA Solve", "Rsolve");
		size_t bits = 26 -(int)ceil(log((double)A.rowdim())*0.7213475205);
		RandomPrimeIterator genprime((unsigned) bits);
		RationalRemainder2< VarPrecEarlyMultipCRA< Modular<double> > > rra(3UL);//using default RR method
		IntegerModularSolve<BB,RatVector,MethodTraits > iteration(A, b, m);
		integer den;
		std::vector< integer > num(A.coldim());
		rra(num, den, iteration, genprime);
		typename RatVector::iterator it_x= x.begin();
		typename std::vector<integer>::const_iterator it_num= num.begin();
		for (; it_x != x.end(); ++it_x, ++it_num){
			integer g = gcd( *it_num, den);
			*it_x = typename RatVector::value_type(*it_num/g, den/g);
		}
		commentator().stop ("done", NULL, "Rsolve");
		return x;
	}

} // LinBox

#include "linbox/config-blas.h"
#ifdef __LINBOX_HAVE_LAPACK
#include "linbox/algorithms/numeric-solver-lapack.h"
#include "linbox/algorithms/rational-solver-sn.h"
namespace LinBox {
	std::vector<PID_integer::Element>&
	solveNum(std::vector<PID_integer::Element>& x, PID_integer::Element & d,
		 const BlasMatrix<PID_integer>& B, const std::vector<PID_integer::Element>& b,
		 const Method::Numerical & m)
	{
		THIS_CODE_COMPILES_BUT_IS_NOT_TESTED; // NOT MUCH

		typedef ParamFuzzy Field ;
		typedef BlasMatrix<Field> FMatrix;

		typedef LPS<FMatrix > NumSolver;
		NumSolver numSolver;
		bool e = false ;
		RationalSolverSN<PID_integer, NumSolver > rsolver(PID_integer(), numSolver, e);

		int status = rsolver.solve(x, d, B, b);
		if (status)
			throw LinBoxError("fail") ;
		return x;
	}
}
#endif

namespace LinBox {
	std::vector<PID_integer::Element>&
	solveNum(std::vector<PID_integer::Element>& x, PID_integer::Element & d,
		 const BlasMatrix<PID_integer>& B, const std::vector<PID_integer::Element>& b,
		 const Method::NumericalWan & m)
	{
		THIS_CODE_COMPILES_BUT_IS_NOT_TESTED; // NOT MUCH

		typedef Modular<int32_t> ZField;
		// typedef Modular<double> ZField;
		PID_integer ZZ ;
		RationalSolver<PID_integer, ZField, RandomPrimeIterator, WanTraits> rsolver(ZZ);

		int status = rsolver.solve(x, d, B, b);
		if (status)
			throw "fail" ;
		return x;
	}
}

#endif // __LINBOX_solve_H



// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,:0,t0,+0,=s
// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:

