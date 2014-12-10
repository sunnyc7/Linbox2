/* linbox/algorithms/rational-solver.h
 * Copyright (C) 2004 Zhendong Wan, Pascal Giorgi
 *
 * Written by Zhendong Wan  <wan@mail.eecis.udel.edu>
 *         and Pascal Giorgi <pascal.giorgi@ens-lyon.fr>
 * Modified by David Pritchard  <daveagp@mit.edu>
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

/*! @file algorithms/rational-solver.h
 * @ingroup algorithms
 * @brief Rational solving (Dixon, Wiedemann,...)
 */

#ifndef __LINBOX_rational_solver_H
#define __LINBOX_rational_solver_H

#include <iostream>

#include "linbox/linbox-config.h"
#include "linbox/util/debug.h"

// #include "linbox/field/multimod-field.h"
#include "linbox/solutions/methods.h"
#include "linbox/blackbox/archetype.h"
#include "linbox/blackbox/lambda-sparse.h"
#include "linbox/blackbox/compose.h"
#include "linbox/matrix/blas-matrix.h"
#include "linbox/algorithms/vector-fraction.h"
#include "linbox/util/timer.h"

//#define RSTIMING
#define DEFAULT_PRIMESIZE 20

namespace LinBox
{// LinBox


#define SINGULARITY_THRESHOLD 5
#define BAD_PRECONTITIONER_THRESHOLD 5
#define DEFAULT_MAXPRIMES 5
#define SL_DEFAULT SL_LASVEGAS



	/** @defgroup padic p-adic lifting for linear system solutions.
	 *  @brief interface for solving linear system by p-adic lifting technique over the quotient field of a ring.
	 *  i.e. solution over the rational for an integer linear system.
	 *
	 * \par Headers
	 *  <code>\#include "linbox/algorithms/rational-solver.h></code>
	 *
	 *  See the following reference for details on this algorithm:
	 * \bib
	 *    - Robert T. Moenck and John H. Carter <i>Approximate algorithms to
	 *  derive exact solutions to system of linear equations.</i> In Proc.
	 *  EUROSAM'79, volume 72 of Lectures Note in Computer Science, pages
	 *  65-72, Berlin-Heidelberger-New York, 1979. Springer-Verlag.
	 *    - John D. Dixon <i>Exact Solution of linear equations using p-adic
	 *  expansions.</i> Numerische Mathematik, volume 40, pages 137-141,
	 *  1982.
	 *  .
	 * \ingroup algorithms
	 *
	 */


	/** \brief define the different return status of the p-adic based solver's computation.
	 *
	 * \ingroup padic
	 */
	enum SolverReturnStatus {
		SS_OK,
		SS_FAILED,
		SS_SINGULAR,
		SS_INCONSISTENT,
		SS_BAD_PRECONDITIONER
	};

	/** Define the different strategy which can be used in the p-adic based solver.
	 *
	 * Used to determine what level of solving should be done:
	 * - Monte Carlo: Try to solve if possible, but result is not guaranteed.
	 *   In any case a 0 denominator should not be returned.
	 * - Las Vegas  : Result should be guaranteed correct.
	 * - Certified  : Additionally, provide certificates that the result returned is correct.
	 *              - if the return value is \p SS_INCONSISTENT, this means
	 *                   \p lastCertificate satisfies \f$lC \cdot A = 0\f$ and \f$lC \cdot b \neq 0 \f$
	 *              - if diophantine solving was called and the return value is \p SS_OK, this means
	 *                   \p lastCertificate satisfies \f$ \mathrm{den}(lC \cdot A) = 1, \mathrm{den}(lC \cdot b) = \mathrm{den}(answer) \f$
	 *              .
	 * .
	 * \ingroup padic
	 */
	enum SolverLevel {
		SL_MONTECARLO,
		SL_LASVEGAS,
		SL_CERTIFIED
	};    // note: code may assume that each level is 'stronger' than the previous one

	/*****************/
	/*** Interface ***/
	/*****************/

	/** \brief Interface for the different specialization of p-adic lifting based solvers.
	 *
	 * The following type are abstract in the implementation and can be
	 * change during the instanciation of the class:
	 * -  Ring: ring over which entries are defined
	 * -  Field: finite field for p-adic lifting
	 * -  RandomPrime: generator of random primes
	 * -  MethodTraits: type of subalgorithm to use in p-adic lifting (default is DixonTraits)
	 * .
	 *
	 * \ingroup padic
	 */
	template<class Ring, class Field, class RandomPrime, class MethodTraits = DixonTraits>
	class RationalSolver {

	public:
		/** Solve a linear system \c Ax=b over quotient field of a ring
		 *         giving a random solution if the system is singular and consistent,
		 *         giving the unique solution if the system is non-singular.
		 *
		 * @param num  Vector of numerators of the solution
		 * @param den  The common denominator. 1/den * num is the rational solution of \c Ax=b.
		 * @param A    Matrix of linear system
		 * @param b    Right-hand side of system
		 * @param maxPrimes maximum number of moduli to try
		 * @param side
		 *
		 * @return status of solution
		 */
		template<class IMatrix, class Vector1, class Vector2>
		SolverReturnStatus solve(Vector1& num, Integer& den,
					 const IMatrix& A, const Vector2& b,
					 const bool side, int maxPrimes = DEFAULT_MAXPRIMES) const;


		/** Solve a nonsingular linear system \c Ax=b over quotient field of a ring,
		 *          giving the unique solution of the system.
		 *
		 * @param num  Vector of numerators of the solution
		 * @param den  The common denominator. 1/den * num is the rational solution of \c Ax=b.
		 * @param A   Matrix of linear system
		 * @param b   Right-hand side of system
		 * @param maxPrimes maximum number of moduli to try
		 *
		 * @return status of solution
		 */
		template<class IMatrix, class Vector1, class Vector2>
		SolverReturnStatus solveNonsingular(Vector1& num, Integer& den,
						    const IMatrix& A, const Vector2& b,
						    // bool toto,
						    int maxPrimes = DEFAULT_MAXPRIMES) const;

		/** brief Solve a singular linear system \c Ax=b over quotient field of a ring,
		 *         giving a random solution if the system is singular and consistent.
		 *
		 * @param num  Vector of numerators of the solution
		 * @param den  The common denominator. 1/den * num is the rational solution of \c Ax=b.
		 * @param A   Matrix of linear system
		 * @param b   Right-hand side of system
		 * @param maxPrimes maximum number of moduli to try
		 *
		 * @return status of solution
		 */
		template<class IMatrix, class Vector1, class Vector2>
		SolverReturnStatus solveSingular(Vector1& num, Integer& den,
						 const IMatrix& A, const Vector2& b,
						 int maxPrimes = DEFAULT_MAXPRIMES) const;


	};

	/***********************/
	/*** Specialisations ***/
	/***********************/

	/*-----------*/
	/* WIEDEMANN */
	/*-----------*/

#ifdef RSTIMING
	class WiedemannTimer {
	public:
		mutable Timer ttSetup, ttRecon, ttGetDigit, ttGetDigitConvert, ttRingApply, ttRingOther;
		void clear() const {
			ttSetup.clear();
			ttRecon.clear();
			ttGetDigit.clear();
			ttGetDigitConvert.clear();
			ttRingOther.clear();
			ttRingApply.clear();
		}

		template<class RR, class LC>
		void update(RR& rr, LC& lc) const {
			ttSetup += lc.ttSetup;
			ttRecon += rr.ttRecon;
			ttGetDigit += lc.ttGetDigit;
			ttGetDigitConvert += lc.ttGetDigitConvert;
			ttRingOther += lc.ttRingOther;
			ttRingApply += lc.ttRingApply;
		}
	};
#endif

	/** Partial specialization of p-adic based solver with Wiedemann algorithm.
	 *
	 *   See the following reference for details on this algorithm:
	 * @bib
	 *   - Douglas H. Wiedemann <i>Solving sparse linear equations over
	 *   finite fields</i>.  IEEE Transaction on Information Theory, 32(1),
	 *   pages 54-62, 1986.
	 *   - Erich Kaltofen and B. David Saunders <i>On Wiedemann's method of
	 *   solving sparse linear systems</i>.  In Applied Algebra, Algebraic
	 *   Algorithms and Error Correcting Codes - AAECC'91, volume 539 of
	 *   Lecture Notes in Computer Sciences, pages 29-38, 1991.
	 *
	 */
	template<class Ring, class Field,class RandomPrime>
	class RationalSolver<Ring, Field, RandomPrime, WiedemannTraits> {

	public:
		typedef Ring                                 RingType;
		typedef typename Ring::Element                Integer;
		typedef typename Field::Element               Element;
		typedef typename RandomPrime::Prime_Type        Prime;
		typedef std::vector<Element>              FPolynomial;

	protected:
		Ring                       _ring;
		mutable RandomPrime _genprime;
		mutable Prime          _prime;
		WiedemannTraits       _traits;

#ifdef RSTIMING
		mutable Timer  tNonsingularSetup,   ttNonsingularSetup,
			tNonsingularMinPoly, ttNonsingularMinPoly,
			totalTimer;

		mutable WiedemannTimer   ttNonsingularSolve;
#endif
	public:

		/** Constructor
		 * @param r   a Ring, set by default
		 * @param rp  a RandomPrime generator, set by default
		 * @param traits
		 */
		RationalSolver (const Ring& r = Ring(),
				const RandomPrime& rp = RandomPrime(DEFAULT_PRIMESIZE),
				const WiedemannTraits& traits=WiedemannTraits()) :
			_ring(r), _genprime(rp), _traits(traits)
		{

			++_genprime; _prime=*_genprime;
#ifdef RSTIMING
			clearTimers();
#endif
		}

		/**  Constructor with a prime.
		 * @param p   a Prime
		 * @param r   a Ring, set by default
		 * @param rp  a RandomPrime generator, set by default
		 * @param traits
		 */
		RationalSolver (const Prime& p, const Ring& r = Ring(),
				const RandomPrime& rp = RandomPrime(DEFAULT_PRIMESIZE),
				const WiedemannTraits& traits=WiedemannTraits()) :
			_ring(r), _genprime(rp), _prime(p), _traits(traits)
		{

#ifdef RSTIMING
			clearTimers();
#endif
		}


		template<class IMatrix, class Vector1, class Vector2>
		SolverReturnStatus solve(Vector1& num, Integer& den,
					 const IMatrix& A, const Vector2& b,
					 const bool s=false, int maxPrimes = DEFAULT_MAXPRIMES) const;


		template<class IMatrix, class Vector1, class Vector2>
		SolverReturnStatus solveNonsingular(Vector1& num, Integer& den,
						    const IMatrix& A, const Vector2& b,
						    int maxPrimes = DEFAULT_MAXPRIMES) const;


		template<class IMatrix, class Vector1, class Vector2>
		SolverReturnStatus solveSingular(Vector1& num, Integer& den,
						 const IMatrix& A, const Vector2& b,
						 int maxPrimes = DEFAULT_MAXPRIMES) const;


		template <class IMatrix, class FMatrix, class IVector>
		void sparseprecondition (const Field&, const IMatrix* ,
					 Compose< LambdaSparseMatrix<Ring>,Compose<IMatrix, LambdaSparseMatrix<Ring> > > *&,
					 const FMatrix*,
					 Compose<LambdaSparseMatrix<Field>,Compose<FMatrix,LambdaSparseMatrix<Field> > > *&,
					 const IVector&, IVector&, LambdaSparseMatrix<Ring> *&,
					 LambdaSparseMatrix<Ring> *&,
					 LambdaSparseMatrix<Field> *&,
					 LambdaSparseMatrix<Field> *&) const;

#if 0
		   template <class IMatrix, class FMatrix, class IVector, class FVector>
		   void precondition (const Field&,
				      const IMatrix&,
				      BlackboxArchetype<IVector>*&,
				      const FMatrix*,
				      BlackboxArchetype<FVector>*&,
				      const IVector&,
				      IVector&,
				      BlackboxArchetype<IVector>*&,
				      BlackboxArchetype<IVector>*&) const;
#endif

#ifdef RSTIMING
		void clearTimers() const
		{
			ttNonsingularSetup.clear();
			ttNonsingularMinPoly.clear();

			ttNonsingularSolve.clear();
		}

	public:

		inline std::ostream& printTime(const Timer& timer, const char* title, std::ostream& os, const char* pref = "") const
		{
			if (&timer != &totalTimer)
				totalTimer += timer;
			if (timer.count() > 0) {
				os << pref << title;
				for (int i=strlen(title)+strlen(pref); i<28; i++)
					os << ' ';
				return os << timer << std::endl;
			}
			else
				return os;
		}

		inline std::ostream& printWiedemannTime(const WiedemannTimer& timer, const char* title, std::ostream& os) const
		{
			if (timer.ttSetup.count() > 0) {
				printTime(timer.ttSetup, "Setup", os, title);
				printTime(timer.ttGetDigit, "Field Apply", os, title);
				printTime(timer.ttGetDigitConvert, "Ring-Field-Ring Convert", os, title);
				printTime(timer.ttRingApply, "Ring Apply", os, title);
				printTime(timer.ttRingOther, "Ring Other", os, title);
				printTime(timer.ttRecon, "Reconstruction", os, title);
			}
			return os;
		}

		std::ostream& reportTimes(std::ostream& os) const
		{
			totalTimer.clear();
			printTime(ttNonsingularSetup, "NonsingularSetup", os);
			printTime(ttNonsingularMinPoly, "NonsingularMinPoly", os);
			printWiedemannTime(ttNonsingularSolve, "NS ", os);
			printTime(totalTimer , "TOTAL", os);
			return os;
		}
#endif

		void chooseNewPrime() const { ++_genprime; _prime = *_genprime; }

	}; // end of specialization for the class RationalSover with Wiedemann traits

	/*-----------------*/
	/* BLOCK WIEDEMANN */
	/*-----------------*/

#ifdef RSTIMING
	class BlockWiedemannTimer {
	public:
		mutable Timer ttSetup, ttRecon, ttGetDigit, ttGetDigitConvert, ttRingApply, ttRingOther, ttMinPoly;
		void clear() const {
			ttSetup.clear();
			ttRecon.clear();
			ttGetDigit.clear();
			ttGetDigitConvert.clear();
			ttRingOther.clear();
			ttRingApply.clear();
			ttMinPoly.clear();
		}

		template<class RR, class LC>
		void update(RR& rr, LC& lc) const {
			ttSetup += lc.ttSetup;
			ttRecon += rr.ttRecon;
			ttGetDigit += lc.ttGetDigit;
			ttGetDigitConvert += lc.ttGetDigitConvert;
			ttRingOther += lc.ttRingOther;
			ttRingApply += lc.ttRingApply;
			ttMinPoly += lc.ttMinPoly;
		}
	};
#endif

	/** \brief partial specialization of p-adic based solver with block Wiedemann algorithm.
	 *
	 *   See the following reference for details on this algorithm:
	 *   @bib
	 *   - Douglas H. Wiedemann <i>Solving sparse linear equations over
	 *   finite fields</i>.  IEEE Transaction on Information Theory, 32(1),
	 *   pages 54-62, 1986.
	 *   - Don Coppersmith  <i>Solving homogeneous linear equations over
	 *   GF(2) via block Wiedemann algorithm.</i> Mathematic of
	 *   computation, 62(205), pages 335-350, 1994.
	 *
	 *   - Erich Kaltofen and B. David Saunders <i>On Wiedemann's method of
	 *   solving sparse linear systems</i>.  In Applied Algebra, Algebraic
	 *   Algorithms and Error Correcting Codes, AAECC'91, volume 539 of
	 *   Lecture Notes in Computer Sciences, pages 29-38, 1991.
	 *
	 */
	template<class Ring, class Field,class RandomPrime>
	class RationalSolver<Ring, Field, RandomPrime, BlockWiedemannTraits> {

	public:
		typedef Ring                                 RingType;
		typedef typename Ring::Element                Integer;
		typedef typename Field::Element               Element;
		typedef typename RandomPrime::Prime_Type        Prime;
		typedef BlasMatrix<Field>               Coefficient;
		typedef std::vector<Element>              FPolynomial;
		typedef std::vector<Coefficient>     FBlockPolynomial;

	protected:
		Ring                         _ring;
		RandomPrime           _genprime;
		mutable Prime            _prime;
		BlockWiedemannTraits    _traits;

#ifdef RSTIMING
		mutable Timer  tNonsingularSetup,   ttNonsingularSetup,
			tNonsingularBlockMinPoly, ttNonsingularBlockMinPoly,
			totalTimer;

		mutable BlockWiedemannTimer   ttNonsingularSolve;
#endif
	public:

		/*! Constructor.
		 * @param r   a Ring, set by default
		 * @param rp  a RandomPrime generator, set by default
		 * @param traits
		 */
		RationalSolver (const Ring& r = Ring(),
				const RandomPrime& rp = RandomPrime(DEFAULT_PRIMESIZE),
				const BlockWiedemannTraits& traits=BlockWiedemannTraits()) :
			_ring(r), _genprime(rp), _traits(traits)
		{

			++_genprime; _prime=*_genprime;
#ifdef RSTIMING
			clearTimers();
#endif
		}

		/*! Constructor with a prime.
		 * @param p   a Prime
		 * @param r   a Ring, set by default
		 * @param rp  a RandomPrime generator, set by default
		 * @param traits
		 */
		RationalSolver (const Prime& p, const Ring& r = Ring(),
				const RandomPrime& rp = RandomPrime(DEFAULT_PRIMESIZE),
				const BlockWiedemannTraits& traits=BlockWiedemannTraits()) :
			_ring(r), _genprime(rp), _prime(p), _traits(traits)
		{

#ifdef RSTIMING
			clearTimers();
#endif
		}

		template<class IMatrix, class Vector1, class Vector2>
		SolverReturnStatus solve(Vector1& num, Integer& den,
					 const IMatrix& A, const Vector2& b,
					 const bool s=false, int maxPrimes = DEFAULT_MAXPRIMES) const;

		template<class IMatrix, class Vector1, class Vector2>
		SolverReturnStatus solveNonsingular(Vector1& num, Integer& den,
						    const IMatrix& A, const Vector2& b,
						    int maxPrimes = DEFAULT_MAXPRIMES) const;


		template<class IMatrix, class Vector1, class Vector2>
		SolverReturnStatus solveSingular(Vector1& num, Integer& den,
						 const IMatrix& A, const Vector2& b,
						 int maxPrimes = DEFAULT_MAXPRIMES) const;



#ifdef RSTIMING
		void clearTimers() const
		{
			ttNonsingularSetup.clear();
			ttNonsingularBlockMinPoly.clear();

			ttNonsingularSolve.clear();
		}

	public:

		inline std::ostream& printTime(const Timer& timer, const char* title, std::ostream& os, const char* pref = "") const
		{
			if (&timer != &totalTimer)
				totalTimer += timer;
			if (timer.count() > 0) {
				os << pref << title;
				for (int i=strlen(title)+strlen(pref); i<28; i++)
					os << ' ';
				return os << timer << std::endl;
			}
			else
				return os;
		}

		inline std::ostream& printBlockWiedemannTime(const BlockWiedemannTimer& timer, const char* title, std::ostream& os) const
		{
			if (timer.ttSetup.count() > 0) {
				printTime(timer.ttSetup, "Setup", os, title);
				printTime(timer.ttGetDigit, "Field Apply", os, title);
				printTime(timer.ttGetDigitConvert, "Ring-Field-Ring Convert", os, title);
				printTime(timer.ttRingApply, "Ring Apply", os, title);
				printTime(timer.ttRingOther, "Ring Other", os, title);
				printTime(timer.ttRecon, "Reconstruction", os, title);
			}
			return os;
		}

		std::ostream& reportTimes(std::ostream& os) const
		{
			totalTimer.clear();
			printTime(ttNonsingularSetup, "NonsingularSetup", os);
			printTime(ttNonsingularBlockMinPoly, "NonsingularMinPoly", os);
			printBlockWiedemannTime(ttNonsingularSolve, "NS ", os);
			printTime(totalTimer , "TOTAL", os);
			std::cout<<"MinPoly computation        :"<<ttNonsingularSolve.ttMinPoly<<std::endl;
			return os;
		}
#endif
	}; // end of specialization for the class RationalSover with BlockWiedemann traits

	/*-------*/
	/* DIXON */
	/*-------*/

#ifdef RSTIMING
	class DixonTimer {
	public:
		mutable Timer ttSetup, ttRecon, ttGetDigit, ttGetDigitConvert, ttRingApply, ttRingOther;
		mutable int rec_elt;
		void clear() const
		{
			ttSetup.clear();
			ttRecon.clear();
			ttGetDigit.clear();
			ttGetDigitConvert.clear();
			ttRingOther.clear();
			ttRingApply.clear();
			rec_elt=0;
		}

		template<class RR, class LC>
		void update(RR& rr, LC& lc) const
		{
			ttSetup += lc.ttSetup;
			ttRecon += rr.ttRecon;
			rec_elt += rr._num_rec;
			ttGetDigit += lc.ttGetDigit;
			ttGetDigitConvert += lc.ttGetDigitConvert;
			ttRingOther += lc.ttRingOther;
			ttRingApply += lc.ttRingApply;
		}
	};
#endif

	/** \brief partial specialization of p-adic based solver with Dixon algorithm.
	 *
	 *   See the following reference for details on this algorithm:
	 *   @bib
	 *  - John D. Dixon <i>Exact Solution of linear equations using p-adic
	 *  expansions</i>. Numerische Mathematik, volume 40, pages 137-141,
	 *  1982.
	 *
	 */

	template<class Ring, class Field,class RandomPrime>
	class RationalSolver<Ring, Field, RandomPrime, DixonTraits> {

	public:

		typedef Ring                                 RingType;
		typedef typename Ring::Element               Integer;
		typedef typename Field::Element              Element;
		typedef typename RandomPrime::Prime_Type     Prime;

		// polymorphic 'certificate' generated when level >= SL_CERTIFIED
		// certificate of inconsistency when any solving routine returns SS_INCONSISTENT
		// certificate of minimal denominator when findRandomSolutionAndCertificate is called & return is SS_OK
		mutable VectorFraction<Ring>                 lastCertificate;

		//next 2 fields generated only by findRandomSolutionAndCertificate, when return is SS_OK
		mutable Integer                              lastZBNumer;               //filled in if level >= SL_CERTIFIED
		mutable Integer                              lastCertifiedDenFactor;    //filled in if level >= SL_LASVEGAS
		//note: lastCertificate * b = lastZBNumer / lastCertifiedDenFactor, in lowest form

	protected:

		mutable RandomPrime             _genprime;
		mutable Prime                   _prime;
		Ring                            _ring;
#ifdef RSTIMING
		mutable Timer
		tSetup,           ttSetup,
		tLQUP,            ttLQUP,
		tFastInvert,      ttFastInvert,        //only done in deterministic or inconsistent
		tCheckConsistency,ttCheckConsistency,        //includes lifting the certificate
		tMakeConditioner, ttMakeConditioner,
		tInvertBP,        ttInvertBP,              //only done in random
		tCheckAnswer,     ttCheckAnswer,
		tCertSetup,       ttCertSetup,        //remaining 3 only done when makeMinDenomCert = true
		tCertMaking,      ttCertMaking,

		tNonsingularSetup,ttNonsingularSetup,
		tNonsingularInv,  ttNonsingularInv,

		totalTimer;

		mutable DixonTimer
		ttConsistencySolve, ttSystemSolve, ttCertSolve, ttNonsingularSolve;
#endif

	public:

		/** Constructor
		 * @param r   a Ring, set by default
		 * @param rp  a RandomPrime generator, set by default
		 */
		RationalSolver (const Ring& r = Ring(),
				const RandomPrime& rp = RandomPrime(DEFAULT_PRIMESIZE)) :
			lastCertificate(r, 0), _genprime(rp), _ring(r)
		{
			++_genprime; _prime=*_genprime;
#ifdef RSTIMING
			clearTimers();
#endif
		}


		/** Constructor, trying the prime p first
		 * @param p a Prime
		 * @param r a Ring, set by default
		 * @param rp a RandomPrime generator, set by default
		 */
		RationalSolver (const Prime& p, const Ring& r = Ring(),
				const RandomPrime& rp = RandomPrime(DEFAULT_PRIMESIZE)) :
			lastCertificate(r, 0), _genprime(rp), _prime(p), _ring(r)
		{
#ifdef RSTIMING
			clearTimers();
#endif
		}


		/** Solve a linear system \c Ax=b over quotient field of a ring.
		 *
		 * @param num Vector of numerators of the solution
		 * @param den  The common denominator. 1/den * num is the rational solution of \c Ax=b.
		 * @param A        Matrix of linear system
		 * @param b        Right-hand side of system
		 * @param s
		 * @param maxPrimes maximum number of moduli to try
		 * @param level    level of certification to be used
		 *
		 * @return status of solution. if \c (return != SS_FAILED), and \c (level >= SL_LASVEGAS), solution is guaranteed correct.
		 *  \c  SS_FAILED - all primes used were bad
		 *  \c SS_OK - solution found.
		 *  \c  SS_INCONSISTENT - system appreared inconsistent. certificate is in \p  lastCertificate if \c (level >= SL_CERTIFIED)
		 */
		template<class IMatrix, class Vector1, class Vector2>
		SolverReturnStatus solve(Vector1& num, Integer& den, const IMatrix& A,
					 const Vector2& b, const bool s = false,
					 const int maxPrimes = DEFAULT_MAXPRIMES,
					 const SolverLevel level = SL_DEFAULT) const;

		/** overload so that the bool 'oldMatrix' argument is not accidentally set to true */
		template <class IMatrix, class Vector1, class Vector2>
		SolverReturnStatus solve(Vector1& num, Integer& den,
					 const IMatrix& A, const Vector2& b, const int maxPrimes,
					 const SolverLevel level = SL_DEFAULT) const
		{
			return solve (num, den, A, b, false, maxPrimes, level);
		}

		/** Solve a nonsingular, square linear system \c Ax=b over quotient field of a ring.
		 *
		 * @param num       Vector of numerators of the solution
		 * @param den       The common denominator. <code>1/den * num</code> is the rational solution of <code>Ax = b</code>
		 * @param A         Matrix of linear system (it must be square)
		 * @param b         Right-hand side of system
		 * @param s         unused
		 * @param maxPrimes maximum number of moduli to try
		 *
		 * @return status of solution :
		 *   - \c SS_FAILED   all primes used were bad;
		 *   - \c SS_OK       solution found, guaranteed correct;
		 *   - \c SS_SINGULAR system appreared singular mod all primes.
		 *   .
		 */
		template<class IMatrix, class Vector1, class Vector2>
		SolverReturnStatus solveNonsingular(Vector1& num, Integer& den, const IMatrix& A,
						    const Vector2& b, bool s = false,
						    int maxPrimes = DEFAULT_MAXPRIMES) const;

		/** Solve a general rectangular linear system \c Ax=b over quotient field of a ring.
		 *  If A is known to be square and nonsingular, calling solveNonsingular is more efficient.
		 *
		 * @param num       Vector of numerators of the solution
		 * @param den       The common denominator. <code>1/den * num</code> is the rational solution of <code>Ax = b</code>
		 * @param A         Matrix of linear system
		 * @param b         Right-hand side of system
		 * @param maxPrimes maximum number of moduli to try
		 * @param level     level of certification to be used
		 *
		 * @return status of solution. if <code>(return != SS_FAILED)</code>, and <code>(level >= SL_LASVEGAS)</code>, solution is guaranteed correct.
		 *   - \c SS_FAILED        all primes used were bad
		 *   - \c SS_OK            solution found.
		 *   - \c SS_INCONSISTENT  system appreared inconsistent. certificate is in \p lastCertificate if <code>(level >= SL_CERTIFIED)</code>
		 */
		template<class IMatrix, class Vector1, class Vector2>
		SolverReturnStatus solveSingular(Vector1& num, Integer& den, const IMatrix& A,
						 const Vector2& b,
						 int maxPrimes = DEFAULT_MAXPRIMES, const SolverLevel level = SL_DEFAULT) const;

		/** Find a random solution of the general linear system  \c Ax=b over quotient field of a ring.
		 *
		 * @param num   Vector of numerators of the solution
		 * @param den   The common denominator. <code>1/den * num</code> is the rational solution of <code>Ax = b</code>.
		 * @param A         Matrix of linear system
		 * @param b         Right-hand side of system
		 * @param maxPrimes maximum number of moduli to try
		 * @param level     level of certification to be used
		 *
		 * @return status of solution. if <code>(return != SS_FAILED)</code>, and <code>(level >= SL_LASVEGAS)</code>, solution is guaranteed correct.
		 *  - \c SS_FAILED  all primes used were bad
		 *  - \c SS_OK  solution found.
		 *  - \c SS_INCONSISTENT  system appreared inconsistent. certificate is in lastCertificate if <code>(level >= SL_CERTIFIED)</code>
		 */
		template<class IMatrix, class Vector1, class Vector2>
		SolverReturnStatus findRandomSolution(Vector1& num, Integer& den, const IMatrix& A,
						      const Vector2& b,
						      int maxPrimes = DEFAULT_MAXPRIMES,
						      const SolverLevel level = SL_DEFAULT) const;

		/** Big solving routine to perform random solving and certificate generation.
		 * Same arguments and return as findRandomSolution, except
		 *
		 * @param num  Vector of numerators of the solution
		 * @param den  The common denominator. <code>1/den * num</code> is the rational solution of <code>Ax = b</code>
		 * @param A
		 * @param b
		 * @param randomSolution  parameter to determine whether to randomize or not (since solveSingular calls this function as well)
		 * @param makeMinDenomCert  determines whether a partial certificate for the minimal denominator of a rational solution is made
		 * @param maxPrimes
		 * @param level
		 *
		 * When <code>(randomSolution == true && makeMinDenomCert == true)</code>,
		 *  - If <code>(level == SL_MONTECARLO)</code> this function has the same effect as calling findRandomSolution.
		 *  - If <code>(level >= SL_LASVEGAS && return == SS_OK)</code>, \c lastCertifiedDenFactor contains a certified factor of the min-solution's denominator.
		 *  - If <code>(level >= SL_CERTIFIED && return == SS_OK)</code>, \c lastZBNumer and \c lastCertificate are updated as well.
		 *
		 */
		template <class IMatrix, class Vector1, class Vector2>
		SolverReturnStatus monolithicSolve (Vector1& num, Integer& den, const IMatrix& A,
						    const Vector2& b,
						    bool makeMinDenomCert, bool randomSolution,
						    int maxPrimes = DEFAULT_MAXPRIMES,
						    const SolverLevel level = SL_DEFAULT) const;

		Ring getRing() const
		{
			return _ring;
		}

		void chooseNewPrime() const
		{
			++_genprime;
			_prime = *_genprime;
		}

#ifdef RSTIMING
		void clearTimers() const
		{
			ttSetup.clear();
			ttLQUP.clear();
			ttFastInvert.clear();
			ttCheckConsistency.clear();
			ttMakeConditioner.clear();
			ttInvertBP.clear();
			ttCheckAnswer.clear();
			ttCertSetup.clear();
			ttCertMaking.clear();
			ttNonsingularSetup.clear();
			ttNonsingularInv.clear();

			ttConsistencySolve.clear();
			ttSystemSolve.clear();
			ttCertSolve.clear();
			ttNonsingularSolve.clear();
		}

	public:

		inline std::ostream& printTime(const Timer& timer, const char* title,
					       std::ostream& os, const char* pref = "") const
		{
			if (&timer != &totalTimer)
				totalTimer += timer;
			if (timer.count() > 0) {
				os << pref << title;
				for (int i=strlen(title)+strlen(pref); i<28; i++)
					os << ' ';
				return os << timer << std::endl;
			}
			else
				return os;
		}

		inline std::ostream& printDixonTime(const DixonTimer& timer, const char* title,
						    std::ostream& os) const
		{
			if (timer.ttSetup.count() > 0) {
				printTime(timer.ttSetup, "Setup", os, title);
				printTime(timer.ttGetDigit, "Field Apply", os, title);
				printTime(timer.ttGetDigitConvert, "Ring-Field-Ring Convert", os, title);
				printTime(timer.ttRingApply, "Ring Apply", os, title);
				printTime(timer.ttRingOther, "Ring Other", os, title);
				printTime(timer.ttRecon, "Reconstruction", os, title);
				os<<" number of elt recontructed: "<<timer.rec_elt<<std::endl;
			}
			return os;
		}

		std::ostream& reportTimes(std::ostream& os) const
		{
			totalTimer.clear();
			printTime(ttNonsingularSetup, "NonsingularSetup", os);
			printTime(ttNonsingularInv, "NonsingularInv", os);
			printDixonTime(ttNonsingularSolve, "NS ", os);
			printTime(ttSetup , "Setup", os);
			printTime(ttLQUP , "LQUP", os);
			printTime(ttFastInvert , "FastInvert", os);
			printTime(ttCheckConsistency , "CheckConsistency", os);
			printDixonTime(ttConsistencySolve, "INC ", os);
			printTime(ttMakeConditioner , "MakeConditioner", os);
			printTime(ttInvertBP , "InvertBP", os);
			printDixonTime(ttSystemSolve, "SYS ", os);
			printTime(ttCheckAnswer , "CheckAnswer", os);
			printTime(ttCertSetup , "CertSetup", os);
			printDixonTime(ttCertSolve, "CER ", os);
			printTime(ttCertMaking , "CertMaking", os);
			printTime(totalTimer , "TOTAL", os);
			return os;
		}
#endif

	}; // end of specialization for the class RationalSolver with Dixon traits


	/*----------------*/
	/* HYBRID Num/Sym */
	/*----------------*/

	/** \brief solver using a hybrid Numeric/Symbolic computation.
		template argument Field and RandomPrime are not used.
		Keep it just for interface consistency.
	 */
	template <class Ring, class Field, class RandomPrime>
	class RationalSolver<Ring, Field, RandomPrime, NumericalTraits>;

	/** \brief solver using a hybrid Numeric/Symbolic computation.
	 *
	 *   This is the original numerix/symbolic solver, now replaced by an enhanced version.
	 *
	 *   See the following reference for details on this implementation:
	 *   @bib
	 *   - Zhendong Wan <i>Exactly solve integer linear systems using numerical methods.</i>
	 *   Submitted to Journal of Symbolic Computation, 2004.
	 *   @warning entries in Matrix must be smaller than \f$2^50\f$.
	 *
	 */
	//template argument Field and RandomPrime are not used.
	//Keep it just for interface consistency.
	template <class Ring, class Field, class RandomPrime>
	class RationalSolver<Ring, Field, RandomPrime, WanTraits> ;

	/*--------------*/
	/* BLOCK HANKEL */
	/*--------------*/

	/*! Block Hankel.
	 * NO DOC
	 */
	template<class Ring, class Field,class RandomPrime>
	class RationalSolver<Ring, Field, RandomPrime, BlockHankelTraits> {
	public:
		typedef Ring                                 RingType;
		typedef typename Ring::Element               Integer;
		typedef typename Field::Element              Element;
		typedef typename RandomPrime::Prime_Type     Prime;

	protected:
		RandomPrime                     _genprime;
		mutable Prime                   _prime;
		Ring                            _ring;

	public:


		/** Constructor.
		 * @param r   a Ring, set by default
		 * @param rp  a RandomPrime generator, set by default
		 */
		RationalSolver (const Ring& r = Ring(),
				const RandomPrime& rp = RandomPrime(DEFAULT_PRIMESIZE)) :
			_genprime(rp), _ring(r)
		{
			_prime=_genprime.randomPrime();
		}


		/** Constructor, trying the prime p first.
		 * @param p   a Prime
		 * @param r   a Ring, set by default
		 * @param rp  a RandomPrime generator, set by default
		 */
		RationalSolver (const Prime& p, const Ring& r = Ring(),
				const RandomPrime& rp = RandomPrime(DEFAULT_PRIMESIZE)) :
			_genprime(rp), _prime(p), _ring(r)
		{}


		// solve non singular system
		template<class IMatrix, class Vector1, class Vector2>
		SolverReturnStatus solveNonsingular(Vector1& num, Integer& den,
						    const IMatrix& A, const Vector2& b,
						    size_t blocksize, int maxPrimes = DEFAULT_MAXPRIMES) const;
	};



	/*-----------*/
	/* SPARSE LU */
	/*-----------*/

	/*! Sparse LU.
	 * NO DOC
	 */
	template<class Ring, class Field,class RandomPrime>
	class RationalSolver<Ring, Field, RandomPrime, SparseEliminationTraits> {
	public:
		typedef Ring                                 RingType;
		typedef typename Ring::Element               Integer;
		typedef typename Field::Element              Element;
		typedef typename RandomPrime::Prime_Type     Prime;

	protected:
		RandomPrime                     _genprime;
		mutable Prime                   _prime;
		Ring                            _ring;

	public:


		/** Constructor.
		 * @param r   a Ring, set by default
		 * @param rp  a RandomPrime generator, set by default
		 */
		RationalSolver (const Ring& r = Ring(),
				const RandomPrime& rp = RandomPrime(DEFAULT_PRIMESIZE)) :
			_genprime(rp), _ring(r)
		{
			_prime=_genprime.randomPrime();
		}


		/** Constructor, trying the prime p first.
		 * @param p   a Prime
		 * @param r   a Ring, set by default
		 * @param rp  a RandomPrime generator, set by default
		 */
		RationalSolver (const Prime& p, const Ring& r = Ring(),
				const RandomPrime& rp = RandomPrime(DEFAULT_PRIMESIZE)) :
			_genprime(rp), _prime(p), _ring(r)
		{}


		// solve non singular system
		template<class IMatrix, class Vector1, class Vector2>
		SolverReturnStatus solveNonsingular(Vector1& num, Integer& den,
						    const IMatrix& A, const Vector2& b,
						    int maxPrimes = DEFAULT_MAXPRIMES) const;
	};

}


#include "linbox/algorithms/rational-solver.inl"

#endif //__LINBOX_rational_solver_H



// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,:0,t0,+0,=s
// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:

