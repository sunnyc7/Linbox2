
/* linbox/algorithms/mg-block-lanczos.inl
 * Copyright (C) 2002 Bradford Hovinen
 *
 * Written by Bradford Hovinen <bghovinen@math.waterloo.ca>
 *
 * --------------------------------------------
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
 * Function definitions for block Lanczos iteration
 */

#ifndef __LINBOX_mg_block_lanczos_INL
#define __LINBOX_mg_block_lanczos_INL

#include "linbox/linbox-config.h"

#include <iostream>

#include "linbox/util/debug.h"
#include "linbox/solutions/methods.h"
#include "linbox/blackbox/diagonal.h"
#include "linbox/blackbox/compose.h"
#include "linbox/blackbox/transpose.h"
#include "linbox/randiter/nonzero.h"
#include "linbox/util/commentator.h"
#include "linbox/util/timer.h"
#include "linbox/algorithms/mg-block-lanczos.h"

// I'm putting everything inside the LinBox namespace so that I can drop all of
// this in to LinBox easily at a later date, without any messy porting.

namespace LinBox
{

#ifdef MGBL_DETAILED_TRACE



	template <class Field, class Matrix>
	void MGBLTraceReport (std::ostream &out, MatrixDomain<Field> &MD, const char *text, size_t iter, const Matrix &M)
	{
		out << text << " [" << iter << "]:" << std::endl;
		MD.write (out, M);
	}

	template <class Field, class Vector>
	void MGBLTraceReport (std::ostream &out, VectorDomain<Field> &VD, const char *text, size_t iter, const Vector &v)
	{
		out << text << " [" << iter << "]: ";
		VD.write (out, v) << std::endl;
	}

	void reportS (std::ostream &out, const std::vector<bool> &S, size_t iter)
	{
		out << "S_" << iter << ": [" << S << "]" << std::endl;
	}

	template <class Field, class Matrix>
	void checkAConjugacy (const MatrixDomain<Field> &MD, const Matrix &AV, const Matrix &V, Matrix &T,
			      size_t AV_iter, size_t V_iter)
	{
		std::ostream &report = commentator().report (Commentator::LEVEL_IMPORTANT, INTERNAL_DESCRIPTION);

		report << "Checking whether V_" << V_iter << " is A-conjugate to V_" << AV_iter << "...";

		MD.mul (T, TransposeMatrix<const Matrix> (V), AV);

		if (MD.isZero (T))
			report << "yes" << std::endl;
		else {
			report << "no" << std::endl;

			std::ostream &err_report = commentator().report (Commentator::LEVEL_IMPORTANT, INTERNAL_ERROR);
			err_report << "ERROR: V_" << V_iter << " is not A-conjugate to V_" << AV_iter << std::endl;
			err_report << "Computed V_" << V_iter << "^T AV_" << AV_iter << ":" << std::endl;
			MD.write (report, T);
		}
	}

#else

	template <class Domain, class Object>
	inline void MGBLTraceReport (std::ostream &out, Domain &D, const char *text, size_t iter, const Object &obj)
	{}

	inline void reportS (std::ostream &out, const std::vector<bool> &S, size_t iter)
	{}

	template <class Field, class Matrix>
	inline void checkAConjugacy (const MatrixDomain<Field> &MD, const Matrix &AV, const Matrix &V, Matrix &T,
				     size_t AV_iter, size_t V_iter)
	{}

#endif

#ifdef DETAILED_PROFILE
#  define TIMER_DECLARE(part) UserTimer part##_timer; double part##_time = 0.0;
#  define TIMER_START(part) part##_timer.start ()
#  define TIMER_STOP(part) part##_timer.stop (); part##_time += part##_timer.time ()
#  define TIMER_REPORT(part) \
	commentator().report (Commentator::LEVEL_NORMAL, TIMING_MEASURE) \
	<< "Total " #part " time: " << part##_time << "s" << std::endl;
#else
#  define TIMER_DECLARE(part)
#  define TIMER_START(part)
#  define TIMER_STOP(part)
#  define TIMER_REPORT(part)
#endif

	// N.B. This code was lifted from the Lanczos iteration in LinBox

	template <class Field, class Matrix>
	template <class Blackbox, class Vector>
	inline bool MGBlockLanczosSolver<Field, Matrix>::solve (const Blackbox &A, Vector &x, const Vector &b)
	{
		linbox_check ((x.size () == A.coldim ()) &&
			      (b.size () == A.rowdim ()));

		commentator().start ("Solving linear system (Montgomery's block Lanczos)", "MGBlockLanczosSolver::solve");

		bool success = false;
		Vector d1, d2, b1, b2, bp, y, Ax, ATAx, ATb;

		// Get the temporaries into the right sizes
		_b.resize (b.size (), 1);
		_x.resize (x.size (), 1);

		_tmp.resize (_block, 1);
		_tmp1.resize (_block, 1);

		_matV[0].resize (A.coldim (), _block);
		_matV[1].resize (A.coldim (), _block);
		_matV[2].resize (A.coldim (), _block);
		_AV.resize (A.coldim (), _block);

		NonzeroRandIter<Field> real_ri (_field, _randiter);
		RandomDenseStream<Field, Vector, NonzeroRandIter<Field> > stream (_field, real_ri, A.coldim ());

		for (unsigned int i = 0; !success && i < _traits.maxTries (); ++i) {
			std::ostream &report = commentator().report (Commentator::LEVEL_UNIMPORTANT, INTERNAL_DESCRIPTION);
			report << "In try: " << i << std::endl;

			switch (_traits.preconditioner ()) {
			case BlockLanczosTraits::NO_PRECONDITIONER:
				_VD.copy (*(_b.colBegin ()), b);
				success = iterate (A);
				_VD.copy (x, *(_x.colBegin ()));
				break;

			case BlockLanczosTraits::SYMMETRIZE:
				{
					VectorWrapper::ensureDim (bp, A.coldim ());

					Transpose<Blackbox> AT (&A);
					Compose<Transpose<Blackbox>, Blackbox> B (&AT, &A);

					AT.apply (bp, b);

					_VD.copy (*(_b.colBegin ()), bp);

					success = iterate (B);

					_VD.copy (x, *(_x.colBegin ()));

					break;
				}

			case BlockLanczosTraits::PARTIAL_DIAGONAL:
				{
					VectorWrapper::ensureDim (d1, A.coldim ());
					VectorWrapper::ensureDim (y, A.coldim ());

					stream >> d1;
					Diagonal<Field> D (_field, d1);
					Compose<Blackbox, Diagonal<Field> > B (&A, &D);

					report << "Random D: ";
					_VD.write (report, d1) << std::endl;

					_VD.copy (*(_b.colBegin ()), b);

					success = iterate (B);

					D.apply (x, *(_x.colBegin ()));

					break;
				}

			case BlockLanczosTraits::PARTIAL_DIAGONAL_SYMMETRIZE:
				{
					VectorWrapper::ensureDim (d1, A.rowdim ());
					VectorWrapper::ensureDim (b1, A.rowdim ());
					VectorWrapper::ensureDim (bp, A.coldim ());

					typedef Diagonal<Field> PC1;
					typedef Transpose<Blackbox> PC2;
					typedef Compose<PC1, Blackbox> CO1;
					typedef Compose<PC2, CO1> CO2;

					stream >> d1;
					PC1 D (_field, d1);
					PC2 AT (&A);
					CO1 B1 (&D, &A);
					CO2 B (&AT, &B1);

					report << "Random D: ";
					_VD.write (report, d1) << std::endl;

					D.apply (b1, b);
					AT.apply (bp, b1);

					_VD.copy (*(_b.colBegin ()), bp);
					success = iterate (B);
					_VD.copy (x, *(_x.colBegin ()));

					break;
				}

			case BlockLanczosTraits::FULL_DIAGONAL:
				{
					VectorWrapper::ensureDim (d1, A.coldim ());
					VectorWrapper::ensureDim (d2, A.rowdim ());
					VectorWrapper::ensureDim (b1, A.rowdim ());
					VectorWrapper::ensureDim (b2, A.coldim ());
					VectorWrapper::ensureDim (bp, A.coldim ());
					VectorWrapper::ensureDim (y, A.coldim ());

					typedef Diagonal<Field> PC1;
					typedef Transpose<Blackbox> PC2;
					typedef Compose<Blackbox, PC1> CO1;
					typedef Compose<PC1, CO1> CO2;
					typedef Compose<PC2, CO2> CO3;
					typedef Compose<PC1, CO3> CO4;

					stream >> d1 >> d2;
					PC1 D1 (_field, d1);
					PC1 D2 (_field, d2);
					PC2 AT (&A);
					CO1 B1 (&A, &D1);
					CO2 B2 (&D2, &B1);
					CO3 B3 (&AT, &B2);
					CO4 B (&D1, &B3);

					report << "Random D_1: ";
					_VD.write (report, d1) << std::endl;

					report << "Random D_2: ";
					_VD.write (report, d2) << std::endl;

					D2.apply (b1, b);
					AT.apply (b2, b1);
					D1.apply (bp, b2);

					_VD.copy (*(_b.colBegin ()), bp);
					success = iterate (B);
					D1.apply (x, *(_x.colBegin ()));

					break;
				}

			default:
				throw PreconditionFailed (__func__, __LINE__,
							  "preconditioner is NO_PRECONDITIONER, SYMMETRIZE, PARTIAL_DIAGONAL_SYMMETRIZE, "
							  "PARTIAL_DIAGONAL, or FULL_DIAGONAL");
			}

			if (_traits.checkResult ()) {
				VectorWrapper::ensureDim (Ax, A.rowdim ());

				if (_traits.checkResult () &&
				    ((_traits.preconditioner () == BlockLanczosTraits::SYMMETRIZE) ||
				     (_traits.preconditioner () == BlockLanczosTraits::PARTIAL_DIAGONAL_SYMMETRIZE) ||
				     (_traits.preconditioner () == BlockLanczosTraits::FULL_DIAGONAL)))
				{
					VectorWrapper::ensureDim (ATAx, A.coldim ());
					VectorWrapper::ensureDim (ATb, A.coldim ());

					commentator().start ("Checking whether A^T Ax = A^T b");

					A.apply (Ax, x);
					A.applyTranspose (ATAx, Ax);
					A.applyTranspose (ATb, b);

					if (_VD.areEqual (ATAx, ATb)) {
						commentator().stop ("passed");
						success = true;
					}
					else {
						commentator().stop ("FAILED");
						success = false;
					}
				}
				else if (_traits.checkResult ()) {
					commentator().start ("Checking whether Ax=b");

					A.apply (Ax, x);

					if (_VD.areEqual (Ax, b)) {
						commentator().stop ("passed");
						success = true;
					}
					else {
						commentator().stop ("FAILED");
						success = false;
					}
				}
			}
		}

		commentator().stop ("done", (success ? "Solve successful" : "Solve failed"), "MGBlockLanczosSolver::solve");

		return success;
	}

	template <class Field, class Matrix>
	template <class Blackbox, class Matrix1>
	inline unsigned int MGBlockLanczosSolver<Field, Matrix>::sampleNullspace (const Blackbox &A, Matrix1 &x)
	{
		linbox_check (x.rowdim () == A.coldim ());

		commentator().start ("Sampling from nullspace (Montgomery's block Lanczos)", "MGBlockLanczosSolver::sampleNullspace");

		unsigned int number = 0;

		bool success;

		typename LinBox::Vector<Field>::Dense d1, d2;

		// Get the temporaries into the right sizes
		_b.resize (x.rowdim (), x.coldim ());
		_x.resize (x.rowdim (), x.coldim ());
		_y.resize (x.rowdim (), x.coldim ());

		_tmp.resize (_block, x.coldim ());
		_tmp1.resize (_block, x.coldim ());

		_matV[0].resize (A.coldim (), _block);
		_matV[1].resize (A.coldim (), _block);
		_matV[2].resize (A.coldim (), _block);
		_AV.resize (A.coldim (), _block);

		typename Matrix::ColIterator xi = x.colBegin ();

		NonzeroRandIter<Field> real_ri (_field, _randiter);
		RandomDenseStream<Field, typename LinBox::Vector<Field>::Dense, NonzeroRandIter<Field> > d_stream (_field, real_ri, A.coldim ());

		TransposeMatrix<Matrix> bT (_b);
		TransposeMatrix<Matrix> xT (_x);

		for ( unsigned int i = 0; number < x.coldim () && i < _traits.maxTries (); ++i) {
			std::ostream &report = commentator().report (Commentator::LEVEL_UNIMPORTANT, INTERNAL_DESCRIPTION);
			report << "in try: " << i << std::endl;

			// Fill y with random data
			RandomDenseStream<Field, typename Matrix::Col> stream (_field, _randiter, A.coldim ());
			typename Matrix::ColIterator iter;

			for (iter = _y.colBegin (); iter != _y.colEnd (); ++iter)
				stream >> *iter;

			// Find a right-hand side for the linear system
			_MD.blackboxMulLeft (_b, A, _y);

			switch (_traits.preconditioner ()) {
			case BlockLanczosTraits::NO_PRECONDITIONER:
				success = iterate (A);
				break;

			case BlockLanczosTraits::SYMMETRIZE:
				{
					Transpose<Blackbox> AT (&A);
					Compose<Transpose<Blackbox>, Blackbox> B (&AT, &A);

					_MD.blackboxMulRight (xT, transpose (_b), A);
					_MD.copy (_b, _x);

					success = iterate (B);
					break;
				}

			case BlockLanczosTraits::PARTIAL_DIAGONAL:
				{
					VectorWrapper::ensureDim (d1, A.coldim ());

					d_stream >> d1;
					Diagonal<Field> D (_field, d1);
					Compose<Blackbox, Diagonal<Field> > B (&A, &D);

					report << "Random D: ";
					_VD.write (report, d1) << std::endl;

					success = iterate (B);

					_MD.blackboxMulLeft (_b, D, _x);
					_MD.copy (_x, _b);

					break;
				}

			case BlockLanczosTraits::PARTIAL_DIAGONAL_SYMMETRIZE:
				{
					VectorWrapper::ensureDim (d1, A.rowdim ());

					typedef Diagonal<Field> PC1;
					typedef Transpose<Blackbox> PC2;
					typedef Compose<PC1, Blackbox> CO1;
					typedef Compose<PC2, CO1> CO2;

					d_stream >> d1;
					PC1 D (_field, d1);
					PC2 AT (&A);
					CO1 B1 (&D, &A);
					CO2 B (&AT, &B1);

					report << "Random D: ";
					_VD.write (report, d1) << std::endl;

					_MD.blackboxMulLeft (_x, D, _b);
					_MD.blackboxMulRight (bT, transpose (_x), A);

					success = iterate (B);

					break;
				}

			case BlockLanczosTraits::FULL_DIAGONAL:
				{
					VectorWrapper::ensureDim (d1, A.coldim ());
					VectorWrapper::ensureDim (d2, A.rowdim ());

					typedef Diagonal<Field> PC1;
					typedef Transpose<Blackbox> PC2;
					typedef Compose<Blackbox, PC1> CO1;
					typedef Compose<PC1, CO1> CO2;
					typedef Compose<PC2, CO2> CO3;
					typedef Compose<PC1, CO3> CO4;

					d_stream >> d1 >> d2;
					PC1 D1 (_field, d1);
					PC1 D2 (_field, d2);
					PC2 AT (&A);
					CO1 B1 (&A, &D1);
					CO2 B2 (&D2, &B1);
					CO3 B3 (&AT, &B2);
					CO4 B (&D1, &B3);

					report << "Random D_1: ";
					_VD.write (report, d1) << std::endl;

					report << "Random D_2: ";
					_VD.write (report, d2) << std::endl;

					_MD.blackboxMulLeft (_x, D2, _b);
					_MD.blackboxMulRight (bT, transpose (_x), A);
					_MD.blackboxMulLeft (_x, D1, _b);
					_MD.copy (_b, _x);

					success = iterate (B);

					_MD.blackboxMulLeft (_b, D1, _x);
					_MD.copy (_x, _b);

					break;
				}

			default:
				throw PreconditionFailed (__func__, __LINE__,
							  "preconditioner is NO_PRECONDITIONER, SYMMETRIZE, PARTIAL_DIAGONAL_SYMMETRIZE, "
							  "PARTIAL_DIAGONAL, or FULL_DIAGONAL");
			}

			// Obtain the candidate nullspace vectors
			_MD.subin (_x, _y);

			// Copy vectors of _x that are true nullspace vectors into the solution
			_MD.blackboxMulLeft (_b, A, _x);

			typename Matrix::ColIterator bi, xip;

			for (bi = _b.colBegin (), xip = _x.colBegin (); bi != _b.colEnd (); ++bi, ++xip) {
				if (_VD.isZero (*bi) && !_VD.isZero (*xip)) {
					_VD.copy (*xi, *xip);
					++number; ++xi;
				}
			}
		}

		commentator().stop ("done", NULL, "MGBlockLanczosSolver::sampleNullspace");

		return number;
	}

	template <class Field, class Matrix>
	template <class Blackbox>
	inline bool MGBlockLanczosSolver<Field, Matrix>::iterate (const Blackbox &A)
	{
		linbox_check (_matV[0].rowdim () == A.rowdim ());
		linbox_check (_matV[1].rowdim () == A.rowdim ());
		linbox_check (_matV[2].rowdim () == A.rowdim ());
		linbox_check (_matV[0].coldim () == _matV[1].coldim ());
		linbox_check (_matV[0].coldim () == _matV[2].coldim ());

		commentator().start ("Block Lanczos iteration", "MGBlockLanczosSolver::iterate", A.rowdim ());

		size_t    Ni;
		size_t    total_dim = 0;

		bool      ret = true, done = false;

		// How many iterations between each progress update
		unsigned int progress_interval = (unsigned int) (A.rowdim () / _traits.blockingFactor () / 100);

		// Make sure there are a minimum of ten
		if (progress_interval == 0)
			progress_interval = 1;

		// i is the index for temporaries where we need to go back to i - 1
		// j is the index for temporaries where we need to go back to j - 2
		int i = 0, j = 2, next_j, prev_j = 1, iter = 2;
		typename Matrix::ColIterator k;

		TIMER_DECLARE(AV);
		TIMER_DECLARE(innerProducts);
		TIMER_DECLARE(Winv);
		TIMER_DECLARE(Vnext);
		TIMER_DECLARE(solution);
		TIMER_DECLARE(orthogonalization);
		TIMER_DECLARE(terminationCheck);

		// Get a random fat vector _matV[0]
		RandomDenseStream<Field, typename Matrix::Col> stream (_field, _randiter, A.coldim ());

		for (k = _matV[0].colBegin (); k != _matV[0].colEnd (); ++k)
			stream >> *k;

		TIMER_START(AV);
		_MD.blackboxMulLeft (_AV, A, _matV[0]);
		TIMER_STOP(AV);

		std::ostream &report = commentator().report (Commentator::LEVEL_IMPORTANT, INTERNAL_DESCRIPTION);

		// Initialize S_-1 to IN
		std::fill (_vecS.begin (), _vecS.end (), true);

		// Iteration 1
		TIMER_START(innerProducts);
		_MD.mul (_VTAV, transpose (_matV[0]), _AV);
		TIMER_STOP(innerProducts);

		TIMER_START(Winv);
		Ni = compute_Winv_S (_Winv[0], _vecS, _VTAV);
		TIMER_STOP(Winv);

		// Check for catastrophic breakdown
		if (Ni == 0) {
			commentator().stop ("breakdown", NULL, "MGBlockLanczosSolver::iterate");
			return false;
		}

		total_dim += Ni;

#ifdef MGBL_DETAILED_TRACE
		report << "Iteration " << iter << ": N_i = " << Ni << std::endl;
		report << "Iteration " << iter << ": Total dimension is " << total_dim << std::endl;
#endif

		TransposeMatrix<Matrix> tmpT (_tmp);
		TransposeMatrix<Matrix> tmp1T (_tmp1);

		TIMER_START(solution);
		mul (tmpT, transpose (_b), _matV[0], _vecS);
		_MD.mul (_tmp1, _Winv[0], _tmp);
		mul_SST (tmp1T, tmp1T, _vecS);
		_MD.mul (_x, _matV[0], _tmp1);
		TIMER_STOP(solution);

		TIMER_START(Vnext);
		mul_SST (_matV[1], _AV, _vecS);
		TIMER_STOP(Vnext);

		TIMER_START(innerProducts);
		mul (_AVTAVSST_VTAV, transpose (_AV), _AV, _vecS);
		TIMER_STOP(innerProducts);

		//	MGBLTraceReport (report, _MD, "V", 0, _matV[0]);
		//	MGBLTraceReport (report, _MD, "AV", 0, _AV);
		MGBLTraceReport (report, _MD, "V^T A V", 0, _VTAV);
		MGBLTraceReport (report, _MD, "Winv", 0, _Winv[0]);
		reportS (report, _vecS, 0);
		//	MGBLTraceReport (report, _MD, "x", 0, _x);
		MGBLTraceReport (report, _MD, "AVSS^T", 0, _matV[1]);
		MGBLTraceReport (report, _MD, "V^T A^2 V", 0, _AVTAVSST_VTAV);

		TIMER_START(orthogonalization);
		_MD.addin (_AVTAVSST_VTAV, _VTAV);
		_MD.mul (_DEF, _Winv[0], _AVTAVSST_VTAV);
		addIN (_DEF);

		_MD.axpyin (_matV[1], _matV[0], _DEF);
		TIMER_START(orthogonalization);

		MGBLTraceReport (report, _MD, "D", 1, _DEF);

		//	MGBLTraceReport (report, _MD, "V", 1, _matV[1]);
		checkAConjugacy (_MD, _AV, _matV[1], _DEF, 0, 1);

		TIMER_START(terminationCheck);
		done = _MD.isZero (_matV[1]);
		TIMER_STOP(terminationCheck);

		if (done) {
			commentator().stop ("done", NULL, "MGBlockLanczosSolver::iterate");
			return true;
		}

		// Iteration 2
		TIMER_START(AV);
		_MD.blackboxMulLeft (_AV, A, _matV[1]);
		TIMER_STOP(AV);

#ifdef MGBL_DETAILED_TRACE
		// DEBUG: Save a copy of AV_1 for use later
		Matrix AV1_backup (_AV.rowdim (), _AV.coldim ());
		_MD.copy (AV1_backup, _AV);
#endif

		TIMER_START(innerProducts);
		_MD.mul (_VTAV, transpose (_matV[1]), _AV);
		TIMER_STOP(innerProducts);

		TIMER_START(Winv);
		Ni = compute_Winv_S (_Winv[1], _vecS, _VTAV);
		TIMER_STOP(Winv);

		// Check for catastrophic breakdown
		if (Ni == 0) {
			commentator().stop ("breakdown", NULL, "MGBlockLanczosSolver::iterate");
			return false;
		}

		total_dim += Ni;

#ifdef MGBL_DETAILED_TRACE
		report << "Iteration " << iter << ": N_i = " << Ni << std::endl;
		report << "Iteration " << iter << ": Total dimension is " << total_dim << std::endl;
#endif

		TIMER_START(solution);
		mul (tmpT, transpose (_b), _matV[1], _vecS);
		_MD.mul (_tmp1, _Winv[1], _tmp);
		mul_SST (tmp1T, tmp1T, _vecS);
		_MD.axpyin (_x, _matV[1], _tmp1);
		TIMER_STOP(solution);

		TIMER_START(Vnext);
		mul_SST (_matV[2], _AV, _vecS);
		TIMER_STOP(Vnext);

		TIMER_START(innerProducts);
		mul (_AVTAVSST_VTAV, transpose (_AV), _AV, _vecS);
		TIMER_STOP(innerProducts);

		//	MGBLTraceReport (report, _MD, "AV", 1, _AV);
		MGBLTraceReport (report, _MD, "V^T A V", 1, _VTAV);
		MGBLTraceReport (report, _MD, "Winv", 1, _Winv[1]);
		reportS (report, _vecS, 1);
		//	MGBLTraceReport (report, _MD, "x", 1, _x);
		MGBLTraceReport (report, _MD, "V^T A^2 V", 1, _AVTAVSST_VTAV);

		TIMER_START(orthogonalization);
		_MD.addin (_AVTAVSST_VTAV, _VTAV);
		_MD.mul (_DEF, _Winv[1], _AVTAVSST_VTAV);
		addIN (_DEF);
		_MD.axpyin (_matV[2], _matV[1], _DEF);

		MGBLTraceReport (report, _MD, "D", 2, _DEF);

		mul (_DEF, _Winv[0], _VTAV, _vecS);
		_MD.axpyin (_matV[2], _matV[0], _DEF);
		TIMER_STOP(orthogonalization);

		MGBLTraceReport (report, _MD, "E", 2, _DEF);
		//	MGBLTraceReport (report, _MD, "V", 2, _matV[2]);

		checkAConjugacy (_MD, _AV, _matV[2], _DEF, 1, 2);

		// Now we're ready to begin the real iteration
		while (1) {
			TIMER_START(terminationCheck);
			done = _MD.isZero (_matV[j]);
			TIMER_STOP(terminationCheck);

			if (done) break;

			next_j = j + 1;
			if (next_j > 2) next_j = 0;

			TIMER_START(AV);
			_MD.blackboxMulLeft (_AV, A, _matV[j]);
			TIMER_STOP(AV);

			// First compute F_i+1, where we use Winv_i-2; then Winv_i and
			// Winv_i-2 can share storage, and we don't need the old _VTAV
			// and _AVTAVSST_VTAV any more. After this, F_i+1 is stored in
			// _DEF

			TIMER_START(orthogonalization);
			_MD.mul (_matT, _VTAV, _Winv[1 - i]);
			addIN (_matT);
			_MD.mul (_DEF, _Winv[i], _matT);
			_MD.mulin (_DEF, _AVTAVSST_VTAV);
			TIMER_STOP(orthogonalization);

			// Now get the next VTAV, Winv, and S_i
			TIMER_START(innerProducts);
			_MD.mul (_VTAV, transpose (_matV[j]), _AV);
			TIMER_STOP(innerProducts);

			TIMER_START(Winv);
			Ni = compute_Winv_S (_Winv[i], _vecS, _VTAV);
			TIMER_STOP(Winv);

			// Check for catastrophic breakdown
			if (Ni == 0) {
				ret = false;
				break;
			}

			total_dim += Ni;

#ifdef MGBL_DETAILED_TRACE
			report << "Iteration " << iter << ": N_i = " << Ni << std::endl;
			report << "Iteration " << iter << ": Total dimension is " << total_dim << std::endl;
#endif

			//		MGBLTraceReport (report, _MD, "AV", iter, _AV);
			MGBLTraceReport (report, _MD, "F", iter + 1, _DEF);
			MGBLTraceReport (report, _MD, "V^T AV", iter, _VTAV);
			MGBLTraceReport (report, _MD, "Winv", iter, _Winv[i]);
			reportS (report, _vecS, iter);

			// Now that we have S_i, finish off with F_i+1
			TIMER_START(orthogonalization);
			mulin (_matV[next_j], _DEF, _vecS);
			TIMER_STOP(orthogonalization);

			// Update x
			TIMER_START(solution);
			mul (tmpT, transpose (_b), _matV[j], _vecS);
			_MD.mul (_tmp1, _Winv[i], _tmp);
			mul_SST (tmp1T, tmp1T, _vecS);
			_MD.axpyin (_x, _matV[j], _tmp1);
			TIMER_STOP(solution);

			//		MGBLTraceReport (report, _MD, "x", iter, _x);

			// Compute the next _AVTAVSST_VTAV
			TIMER_START(innerProducts);
			mul (_AVTAVSST_VTAV, transpose (_AV), _AV, _vecS);
			TIMER_STOP(innerProducts);

			MGBLTraceReport (report, _MD, "V^T A^2 V", iter, _AVTAVSST_VTAV);

			TIMER_START(orthogonalization);
			_MD.addin (_AVTAVSST_VTAV, _VTAV);

			// Compute D and update V_i+1
			_MD.mul (_DEF, _Winv[i], _AVTAVSST_VTAV);
			addIN (_DEF);
			_MD.axpyin (_matV[next_j], _matV[j], _DEF);

			MGBLTraceReport (report, _MD, "D", iter + 1, _DEF);

			// Compute E and update V_i+1
			mul (_DEF, _Winv[1 - i], _VTAV, _vecS);
			_MD.axpyin (_matV[next_j], _matV[prev_j], _DEF);
			TIMER_STOP(orthogonalization);

			MGBLTraceReport (report, _MD, "E", iter + 1, _DEF);

			// Add AV_i S_i S_i^T
			TIMER_START(Vnext);
			addin (_matV[next_j], _AV, _vecS);
			TIMER_STOP(Vnext);

			//		MGBLTraceReport (report, _MD, "V", iter + 1, _matV[next_j]);
			checkAConjugacy (_MD, _AV, _matV[next_j], _DEF, iter, iter + 1);

#ifdef MGBL_DETAILED_TRACE
			checkAConjugacy (_MD, AV1_backup, _matV[next_j], _DEF, 1, iter + 1);
#endif

			i = 1 - i;
			prev_j = j;
			j = next_j;
			++iter;

			if (!(iter % progress_interval))
				commentator().progress (total_dim);

			if (total_dim > A.rowdim ()) {
				commentator().report (Commentator::LEVEL_IMPORTANT, INTERNAL_ERROR)
				<< "Maximum number of iterations passed without termination" << std::endl;
				commentator().stop ("ERROR", NULL, "MGBlockLanczosSolver::iterate");
				return false;
			}
		}

		// Because we set Winv to -Winv, we have -x at the end of the
		// iteration. So negate the result and return it
		_MD.negin (_x);

		//	MGBLTraceReport (report, _MD, "x", iter, _x);

		TIMER_REPORT(AV);
		TIMER_REPORT(innerProducts);
		TIMER_REPORT(Winv);
		TIMER_REPORT(Vnext);
		TIMER_REPORT(solution);
		TIMER_REPORT(orthogonalization);
		TIMER_REPORT(terminationCheck);

		commentator().stop (ret ? "done" : "breakdown", NULL, "MGBlockLanczosSolver::iterate");

		return ret;
	}

	template <class Field, class Matrix>
	inline int MGBlockLanczosSolver<Field, Matrix>::compute_Winv_S
	(Matrix        &Winv,
	 std::vector<bool>                               &S,
	 const Matrix  &T)
	{
		linbox_check (S.size () == Winv.rowdim ());
		linbox_check (S.size () == Winv.coldim ());
		linbox_check (S.size () == T.rowdim ());
		linbox_check (S.size () == T.coldim ());
		linbox_check (S.size () == _matM.rowdim ());
		linbox_check (S.size () * 2 == _matM.coldim ());

#ifdef MGBL_DETAILED_TRACE
		commentator().start ("Computing Winv and S", "MGBlockLanczosSolver::compute_Winv_S", S.size ());

		std::ostream &report = commentator().report (Commentator::LEVEL_UNIMPORTANT, INTERNAL_DESCRIPTION);
		report << "Input T:" << std::endl;
		_MD.write (report, T);
#endif

		BlasMatrix<Field> M1 (_matM, 0, 0, T.rowdim (), T.coldim ());
		BlasMatrix<Field> M2 (_matM, 0, T.coldim (), T.rowdim (), T.coldim ());

		_MD.copy (M1, T);
		setIN (M2);

		permute (_indices, S);

		typename Field::Element Mjj_inv;

		size_t row;
		size_t Ni = 0;

		for (row = 0; row < S.size (); ++row) {
#ifdef MGBL_DETAILED_TRACE
			if (!(row & ((1 << 10) - 1)))
				commentator().progress (row);

			std::ostream &report = commentator().report (Commentator::LEVEL_UNIMPORTANT, INTERNAL_DESCRIPTION);
			report << "Iteration " << row << ": Matrix M = " << std::endl;
			_MD.write (report, _matM);
#endif

			if (find_pivot_row (_matM, row, 0, _indices)) {
#ifdef MGBL_DETAILED_TRACE
				commentator().report (Commentator::LEVEL_UNIMPORTANT, INTERNAL_DESCRIPTION)
				<< "Pivot found for column " << _indices[row] << std::endl;
#endif

				// Pivot element was found for (j, j)

				S[_indices[row]] = true;  // Use column j of V_i in W_i

				// Give the (j, j) entry unity
				_field.inv (Mjj_inv, _matM.getEntry (_indices[row], _indices[row]));
				_VD.mulin (*(_matM.rowBegin () + (int)_indices[row]), Mjj_inv);

				// Zero the rest of the column j
				eliminate_col (_matM, row, 0, _indices, Mjj_inv);

				++Ni;
			}
			else {
#ifdef MGBL_DETAILED_TRACE
				commentator().report (Commentator::LEVEL_NORMAL, INTERNAL_DESCRIPTION)
				<< "No pivot found for column " << _indices[row] << std::endl;
#endif

				// No pivot element found

				S[_indices[row]] = false;  // Skip column j

				find_pivot_row (_matM, row, (int)_block, _indices);

				const typename Field::Element &Mjj = _matM.refEntry (_indices[row], _indices[row] + _block);

				linbox_check (!_field.isZero (Mjj));

				// Zero the rest of the column j + N
				eliminate_col (_matM, row, (int)_block, _indices, _field.inv (Mjj_inv, Mjj));

				// Zero row j
				_VD.subin (*(_matM.rowBegin () + (int)_indices[row]), *(_matM.rowBegin () + (int)_indices[row]));
			}
		}

		_MD.neg (Winv, M2);

#ifdef MGBL_DETAILED_TRACE
		report << "Computed Winv:" << std::endl;
		_MD.write (report, Winv);

		commentator().stop ("done", NULL, "MGBlockLanczosSolver::compute_Winv_S");
#endif

		return (int)Ni;
	}

	template <class Field, class Matrix>
	template <class Matrix1, class Matrix2>
	inline Matrix1 &MGBlockLanczosSolver<Field, Matrix>::mul_SST
	(Matrix1                 &BSST,
	 const Matrix2           &B,
	 const std::vector<bool> &S) const
	{
		linbox_check (B.rowdim () == BSST.rowdim ());
		linbox_check (B.coldim () == BSST.coldim ());
		linbox_check (B.coldim () == S.size ());

		typename Matrix2::ConstColIterator i;
		typename Matrix1::ColIterator j;
		std::vector<bool>::const_iterator k;

		for (i = B.colBegin (), j = BSST.colBegin (), k = S.begin ();
		     i != B.colEnd ();
		     ++i, ++j, ++k)
		{
			if (*k)
				_VD.copy (*j, *i);
			else
				_VD.subin (*j, *j);
		}

		return BSST;
	}

	template <class Field, class Matrix>
	template <class Matrix1, class Matrix2, class Matrix3>
	inline Matrix1 &MGBlockLanczosSolver<Field, Matrix>::mul
	(Matrix1                 &C,
	 const Matrix2           &A,
	 const Matrix3           &B,
	 const std::vector<bool> &S) const
	{
		linbox_check (A.coldim () == B.rowdim ());
		linbox_check (A.rowdim () == C.rowdim ());
		linbox_check (B.coldim () == C.coldim ());

		typename Matrix2::ConstRowIterator i;
		typename Matrix3::ConstColIterator j;
		typename Matrix1::ColIterator k1;
		typename Matrix1::Col::iterator k2;
		std::vector<bool>::const_iterator l;

		for (j = B.colBegin (), l = S.begin (), k1 = C.colBegin ();
		     j != B.colEnd ();
		     ++j, ++l, ++k1)
		{
			if (*l) {
				for (i = A.rowBegin (), k2 = k1->begin (); i != A.rowEnd (); ++i, ++k2)
					_VD.dot (*k2, *i, *j);
			}
			else
				_VD.subin (*k1, *k1);
		}

		return C;
	}

	template <class Field, class Matrix>
	template <class Matrix1, class Matrix2>
	inline Matrix1 &MGBlockLanczosSolver<Field, Matrix>::mulin
	(Matrix1                 &A,
	 const Matrix2           &B,
	 const std::vector<bool> &S) const
	{
		linbox_check (A.coldim () == B.rowdim ());
		linbox_check (B.rowdim () == B.coldim ());
		linbox_check (A.coldim () == S.size ());

		typename Matrix1::RowIterator i;
		typename Matrix2::ConstColIterator j;
		typename Vector<Field>::Dense::iterator k;
		std::vector<bool>::const_iterator l;

		typename LinBox::Vector<Field>::Dense tmp (_traits.blockingFactor ());

		for (i = A.rowBegin (); i != A.rowEnd (); ++i) {
			for (j = B.colBegin (), k = tmp.begin (), l = S.begin ();
			     j != B.colEnd ();
			     ++j, ++k, ++l)
			{
				if (*l)
					_VD.dot (*k, *i, *j);
				else
					_field.subin (*k, *k);
			}

			_VD.copy (*i, tmp);
		}

		return A;
	}

	template <class Field, class Matrix>
	template <class Vector1, class Matrix1, class Vector2>
	inline Vector1 &MGBlockLanczosSolver<Field, Matrix>::vectorMul
	(Vector1                 &w,
	 const Matrix1           &A,
	 const Vector2           &v,
	 const std::vector<bool> &S) const
	{
		linbox_check (A.coldim () == v.size ());
		linbox_check (A.rowdim () == w.size ());

		typename Matrix1::ConstColIterator i = A.colBegin ();
		typename Vector2::const_iterator j = v.begin ();
		typename std::vector<bool>::const_iterator k = S.begin ();

		_VD.subin (w, w);

		for (; j != v.end (); ++j, ++i, ++k)
			if (*k)
				_VD.axpyin (w, *j, *i);

		return w;
	}

	template <class Field, class Matrix>
	template <class Vector1, class Matrix1, class Vector2>
	inline Vector1 &MGBlockLanczosSolver<Field, Matrix>::vectorMulTranspose
	(Vector1                 &w,
	 const Matrix1           &A,
	 const Vector2           &v,
	 const std::vector<bool> &S) const
	{
		linbox_check (A.rowdim () == v.size ());
		linbox_check (A.coldim () == w.size ());

		typename Matrix1::ConstColIterator i = A.colBegin ();
		typename Vector1::iterator j = w.begin ();
		typename std::vector<bool>::const_iterator k = S.begin ();

		for (; j != w.end (); ++j, ++i, ++k)
			if (*k)
				_VD.dot (*j, *i, v);

		return w;
	}

	template <class Field, class Matrix>
	template <class Matrix1>
	inline Matrix1 &MGBlockLanczosSolver<Field, Matrix>::addIN (Matrix1 &A) const
	{
		linbox_check (A.coldim () == A.rowdim ());

		typename Matrix1::RowIterator i;
		size_t idx = 0;

		for (i = A.rowBegin (); i != A.rowEnd (); ++i, ++idx)
			_field.addin ((*i)[idx], _one);

		return A;
	}

	template <class Field, class Matrix>
	template <class Matrix1, class Matrix2>
	inline Matrix1 &MGBlockLanczosSolver<Field, Matrix>::addin
	(Matrix1                 &A,
	 const Matrix2           &B,
	 const std::vector<bool> &S) const
	{
		linbox_check (A.rowdim () == B.rowdim ());
		linbox_check (A.coldim () == B.coldim ());

		typename Matrix1::ColIterator i = A.colBegin ();
		typename Matrix2::ConstColIterator j = B.colBegin ();
		std::vector<bool>::const_iterator k = S.begin ();

		for (; i != A.colEnd (); ++i, ++j, ++k)
			if (*k) _VD.addin (*i, *j);

		return A;
	}

	template <class Field, class Matrix>
	inline void MGBlockLanczosSolver<Field, Matrix>::permute (std::vector<size_t>     &indices,
								  const std::vector<bool> &S) const
	{
		size_t idx;

		std::vector<size_t>::iterator i = indices.begin ();
		std::vector<bool>::const_iterator k;

		for (k = S.begin (), idx = 0; k != S.end (); ++k, ++idx) {
			if (!*k) {
				*i = idx;
				++i;
			}
		}

		for (k = S.begin (), idx = 0; k != S.end (); ++k, ++idx) {
			if (*k) {
				*i = idx;
				++i;
			}
		}
	}

	template <class Field, class Matrix>
	template <class Matrix1>
	inline Matrix1 &MGBlockLanczosSolver<Field, Matrix>::setIN (Matrix1 &A) const
	{
		linbox_check (A.coldim () == A.rowdim ());

		typename Matrix1::RowIterator i;
		size_t i_idx;

		for (i = A.rowBegin (), i_idx = 0; i != A.rowEnd (); ++i, ++i_idx) {
			_VD.subin (*i, *i);
			_field.assign ((*i)[i_idx], _one);
		}

		return A;
	}

	/* Find a row suitable for pivoting in column col and exchange that row with row
	 * A - Matrix on which to operate
	 * idx - Index of the row with which to exchange
	 * Returns true if a pivot could be found; false otherwise
	 */

	template <class Field, class Matrix>
	inline bool MGBlockLanczosSolver<Field, Matrix>::find_pivot_row
	(Matrix                    &A,
	 size_t                     row,
	 int                        col_offset,
	 const std::vector<size_t> &indices)
	{
		size_t idx;

		typename Matrix::Col col_vec;
		typename Matrix::Row row_vec;

		col_vec = *(_matM.colBegin () + (int)indices[row] + col_offset);
		row_vec = *(_matM.rowBegin () + (int)indices[row]);

		for (idx = row; idx < A.rowdim (); ++idx) {
			if (!_field.isZero (A.getEntry (indices[idx], indices[row] + col_offset))) {
				if (idx != row) {
					typename Matrix::Row row1 = *(A.rowBegin () + (int)indices[idx]);
					std::swap_ranges (row_vec.begin (), row_vec.end (), row1.begin ());
				}

				return true;
			}
		}

		return false;
	}

	template <class Field, class Matrix>
	inline void MGBlockLanczosSolver<Field, Matrix>::eliminate_col
	(Matrix                        &A,
	 size_t                         pivot,
	 int                            col_offset,
	 const std::vector<size_t>     &indices,
	 const typename Field::Element &Ajj_inv)
	{
		// I'm assuming everything left of the column with the index of the pivot row is 0
		size_t row;

		typename BlasMatrix<Field>::Row pivot_row;
		typename Field::Element p;

		pivot_row = *(A.rowBegin () + (int)indices[pivot]);

		for (row = 0; row < pivot; ++row) {
			const typename Field::Element &Aij = A.getEntry (indices[row], indices[pivot] + col_offset);

			if (!_field.isZero (Aij))
				_VD.axpyin (*(A.rowBegin () +(int) indices[row]), _field.neg (p, Aij), pivot_row);
		}

		for (++row; row < A.rowdim (); ++row) {
			const typename Field::Element &Aij = A.getEntry (indices[row], indices[pivot] + col_offset);

			if (!_field.isZero (Aij))
				_VD.axpyin (*(A.rowBegin () + (int) indices[row]), _field.neg (p, Aij), pivot_row);
		}
	}

	template <class Field, class Matrix>
	inline void MGBlockLanczosSolver<Field, Matrix>::init_temps ()
	{
		_VTAV.resize (_block, _block);
		_Winv[0].resize (_block, _block);
		_Winv[1].resize (_block, _block);
		_AVTAVSST_VTAV.resize (_block, _block);
		_matT.resize (_block, _block);
		_DEF.resize (_block, _block);
		_vecS.resize (_block);
		_matM.resize (_block, 2 * _block);
		_indices.resize (_block);
	}

	// Check whether the given matrix is "almost" the identity, i.e. the identity
	// with some zeros on the diagonal

	template <class Field, class Matrix>
	template <class Matrix1>
	inline bool MGBlockLanczosSolver<Field, Matrix>::isAlmostIdentity (const Matrix1 &M) const
	{
		linbox_check (M.rowdim () == M.coldim ());

		typename Field::Element neg_one;

		_field.init (neg_one, -1);

		size_t i, j;

		for (i = 0; i < M.rowdim (); ++i) {
			for (j = 0; j < M.coldim (); ++j) {
				if (i != j && !_field.isZero (M.getEntry (i, j))) {
					if (!_field.isZero (M.getEntry (i, i))) {
						typename Matrix::ConstRowIterator row = M.rowBegin () + j;
						if (!_VD.isZero (*row))
							return false;
					}
					else if (!_field.isZero (M.getEntry (j, j))) {
						typename Matrix::ConstColIterator col = M.colBegin () + i;
						if (!_VD.isZero (*col))
							return false;
					}
					else
						return false;
				}
				else if (!_field.isZero (M.getEntry (i, j)) && !_field.areEqual (M.getEntry (i, j), neg_one))
					return false;
			}
		}

		return true;
	}

	// Test suite for MGBlockLanczosSolver
	// All tests below return true on success and false on failure. They take a
	// single argument: n for the row and column dimension of the matrices on which
	// to operate

	// Compute a random dense matrix and run compute_Winv_S on it. Check that the
	// resulting S vector is all 'true' and then multiply the original matrix by the
	// output. Add 1 to the result with addIN and check the the result is zero with
	// isZero

	template <class Field, class Matrix>
	inline bool MGBlockLanczosSolver<Field, Matrix>::test_compute_Winv_S_mul (int n) const
	{
		commentator().start ("Testing compute_Winv_S, mul, addIN, and isZero", "test_compute_Winv_S_mul");

		Matrix A (n, n);
		Matrix AT (n, n);
		Matrix ATA (n, n);
		Matrix W (n, n);
		Matrix WA (n, n);
		std::vector<bool> S (n);

		bool ret = true;

		RandomDenseStream<Field, typename Matrix::Row> stream (_field, _randiter, n);
		typename Matrix::RowIterator i = A.rowBegin ();
		typename Matrix::ColIterator j = AT.colBegin ();

		// With very, very, very, very high probability, this will be
		// nonsingular
		for (; i != A.rowEnd (); ++i, ++j) {
			stream >> *i;
			_VD.copy (*j, *i);
		}

		_MD.mul (ATA, AT, A);

		std::ostream &report = commentator().report (Commentator::LEVEL_UNIMPORTANT, INTERNAL_DESCRIPTION);
		report << "Computed A^T A:" << std::endl;
		_MD.write (report, ATA);

		compute_Winv_S (W, S, ATA);

		report << "Computed W:" << std::endl;
		_MD.write (report, W);

		// Now W should be -A^-1
		_MD.mul (WA, W, ATA);

		report << "Computed WA^T A:" << std::endl;
		_MD.write (report, WA);

		if (!isAlmostIdentity (WA)) {
			commentator().report (Commentator::LEVEL_IMPORTANT, INTERNAL_ERROR)
			<< "ERROR: WA^T A != I" << std::endl;
			ret = false;
		}

		// Now, just for kicks, do the same on the other side

		_MD.mul (WA, ATA, W);

		report << "Computed A^T A W:" << std::endl;
		_MD.write (report, WA);

		if (!isAlmostIdentity (WA)) {
			commentator().report (Commentator::LEVEL_IMPORTANT, INTERNAL_ERROR)
			<< "ERROR: A^T AW != I" << std::endl;
			ret = false;
		}

		commentator().stop (MSG_STATUS (ret), NULL, "test_compute_Winv_S_mul");

		return ret;
	}

	// Same as above, but use mulin rather than mul

	template <class Field, class Matrix>
	inline bool MGBlockLanczosSolver<Field, Matrix>::test_compute_Winv_S_mulin (int n) const
	{
		commentator().start ("Testing compute_Winv_S, copy, mulin, addIN, and isZero", "test_compute_Winv_S_mulin");

		Matrix A (n, n);
		Matrix AT (n, n);
		Matrix ATA (n, n);
		Matrix W (n, n);
		Matrix WA (n, n);
		std::vector<bool> S (n);

		bool ret = true;

		RandomDenseStream<Field, typename Matrix::Row> stream (_field, _randiter, n);
		typename Matrix::RowIterator i = A.rowBegin ();
		typename Matrix::ColIterator j = AT.colBegin ();

		// With very, very, very, very high probability, this will be
		// nonsingular
		for (; i != A.rowEnd (); ++i, ++j) {
			stream >> *i;
			_VD.copy (*j, *i);
		}

		_MD.mul (ATA, AT, A);

		std::ostream &report = commentator().report (Commentator::LEVEL_UNIMPORTANT, INTERNAL_DESCRIPTION);
		report << "Computed A^T A:" << std::endl;
		_MD.write (report, ATA);

		compute_Winv_S (W, S, ATA);

		_MD.copy (WA, W);

		report << "Computed W:" << std::endl;
		_MD.write (report, W);

		// Now W should be -A^-1
		_MD.mulin (WA, ATA);

		report << "Computed WA^T A:" << std::endl;
		_MD.write (report, WA);

		if (!isAlmostIdentity (WA)) {
			commentator().report (Commentator::LEVEL_IMPORTANT, INTERNAL_ERROR)
			<< "ERROR: WA^T A != I" << std::endl;
			ret = false;
		}

		// Now, just for kicks, do the same on the other side

		_MD.copy (WA, ATA);

		_MD.mulin (WA, W);

		report << "Computed A^T AW:" << std::endl;
		_MD.write (report, WA);

		if (!isAlmostIdentity (WA)) {
			commentator().report (Commentator::LEVEL_IMPORTANT, INTERNAL_ERROR)
			<< "ERROR: A^T AW != I" << std::endl;
			ret = false;
		}

		commentator().stop (MSG_STATUS (ret), NULL, "test_compute_Winv_S_mulin");

		return ret;
	}

	// Compute a random nonsingular diagonal matrix and set an S vector so that
	// every other entry is true and the rest are false. Call mul_SST and check that
	// the entries on the resulting diagonal are correct.

	template <class Field, class Matrix>
	inline bool MGBlockLanczosSolver<Field, Matrix>::test_mul_SST (int n) const
	{
		commentator().start ("Testing addin", "test_mulTranspose");

		bool ret = true;

		commentator().stop (MSG_STATUS (ret), NULL, "test_mulTranspose");

		return ret;
	}

	// Same as test_compute_Winv_S_mul above, but zero out every other column using
	// the method for test_mul_SST

	template <class Field, class Matrix>
	inline bool MGBlockLanczosSolver<Field, Matrix>::test_mul_ABSST (int n) const
	{
		commentator().start ("Testing addin", "test_mulTranspose");

		bool ret = true;

		commentator().stop (MSG_STATUS (ret), NULL, "test_mulTranspose");

		return ret;
	}

	// Compute a random dense matrix and two random vectors, and check that <A^T x,
	// y> = <x, Ay>

	template <class Field, class Matrix>
	inline bool MGBlockLanczosSolver<Field, Matrix>::test_mulTranspose (int m, int n) const
	{
		commentator().start ("Testing mulTranspose, m-v mul", "test_mulTranspose");

		Matrix A (m, n);
		typename Vector<Field>::Dense x (m), y (n);
		typename Vector<Field>::Dense ATx (n), Ay (m);
		Element ATxy, xAy;

		bool ret = true;

		RandomDenseStream<Field, typename Matrix::Row> stream (_field, _randiter, n);
		typename Matrix::RowIterator i = A.rowBegin ();

		for (; i != A.rowEnd (); ++i)
			stream >> *i;

		std::ostream &report = commentator().report (Commentator::LEVEL_UNIMPORTANT, INTERNAL_DESCRIPTION);
		report << "Computed A:" << std::endl;
		_MD.write (report, A);

		RandomDenseStream<Field, Matrix> stream1 (_field, _randiter, m);
		stream1 >> x;

		RandomDenseStream<Field, Matrix> stream2 (_field, _randiter, n);
		stream1 >> y;

		report << "Computed     x: ";
		_VD.write (report, x) << std::endl;

		report << "Computed     y: ";
		_VD.write (report, y) << std::endl;

		_MD.vectorMul (ATx, transpose (A), x);

		report << "Computed A^T x: ";
		_VD.write (report, ATx) << std::endl;

		_MD.vectorMul (Ay, A, y);

		report << "Computed    Ay: ";
		_VD.write (report, Ay) << std::endl;

		_VD.dot (ATxy, ATx, y);

		report << "Computed  ATxy: ";
		_field.write (report, ATxy) << std::endl;

		_VD.dot (xAy, x, Ay);

		report << "Computed   xAy: ";
		_field.write (report, xAy) << std::endl;

		if (!_field.areEqual (ATxy, xAy)) {
			commentator().report (Commentator::LEVEL_IMPORTANT, INTERNAL_ERROR)
			<< "ERROR: <A^T x, y> != <x, Ay>" << std::endl;
			ret = false;
		}

		commentator().stop (MSG_STATUS (ret), NULL, "test_mulTranspose");

		return ret;
	}

	// Same as test_mul_SST, but using mulTranspose

	template <class Field, class Matrix>
	inline bool MGBlockLanczosSolver<Field, Matrix>::test_mulTranspose_ABSST (int n) const
	{
		commentator().start ("Testing addin_ABSST", "test_mulTranspose_ABSST");

		bool ret = true;

		commentator().stop (MSG_STATUS (ret), NULL, "test_mulTranspose_ABSST");

		return ret;
	}

	// Same as test_mul_ABSST, but using mulin_ABSST

	template <class Field, class Matrix>
	inline bool MGBlockLanczosSolver<Field, Matrix>::test_mulin_ABSST (int n) const
	{
		commentator().start ("Testing addin_ABSST", "test_mulin_ABSST");

		bool ret = true;

		commentator().stop (MSG_STATUS (ret), NULL, "test_mulin_ABSST");

		return ret;
	}

	// Same as test_addin, but using test_addin_ABSST

	template <class Field, class Matrix>
	inline bool MGBlockLanczosSolver<Field, Matrix>::test_addin_ABSST (int n) const
	{
		commentator().start ("Testing addin_ABSST", "test_addin_ABSST");

		bool ret = true;

		commentator().stop (MSG_STATUS (ret), NULL, "test_addin_ABSST");

		return ret;
	}

	template <class Field, class Matrix>
	inline bool MGBlockLanczosSolver<Field, Matrix>::runSelfCheck () const
	{
		bool ret = true;

		commentator().start ("Running self check", "runSelfCheck", 10);

		if (!test_compute_Winv_S_mul (_block)) ret = false;
		if (!test_compute_Winv_S_mulin (_block)) ret = false;
		if (!test_mul_SST (_block)) ret = false;
		if (!test_mul_ABSST (_block)) ret = false;
		if (!test_mulTranspose (_block * 10, _block)) ret = false;
		if (!test_mulTranspose_ABSST (_block)) ret = false;
		if (!test_mulin_ABSST (_block)) ret = false;
		if (!test_addin_ABSST (_block)) ret = false;

		commentator().stop (MSG_STATUS (ret), NULL, "runSelfCheck");

		return ret;
	}

} // namespace LinBox

#endif // __LINBOX_mg_block_lanczos_INL

// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,:0,t0,+0,=s
// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:

