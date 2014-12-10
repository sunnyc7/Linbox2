
/* Copyright (C) 2010 LinBox
 * Written by <brice.boyer@imag.fr>
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

/** @internal
 * @file permutation-permutation.inl
 * @brief Implementation of permutation-matrix.h
 */

#ifndef __LINBOX_matrix_permutation_INL
#define __LINBOX_matrix_permutation_INL

#define _LB_DBG

#include <algorithm>
#include "linbox/util/debug.h"

// BlasPermutation
namespace LinBox
{
	template<class _Uint>
	BlasPermutation<_Uint>::BlasPermutation() :
	       	r_(0),n_((_Uint)-1),P_(0),Q_(0),inv_(false)
	{
#ifndef NDEBUG
		std::cout << "NULL permutation created. Beware !" << std::endl;
#endif
	}

	template<class _Uint>
	BlasPermutation<_Uint>::BlasPermutation(size_t n) :
	       	r_(n),n_((_Uint)-1),P_(n),Q_(0),inv_(false)
	{ }


	template<class _Uint>
	BlasPermutation<_Uint>::~BlasPermutation() {}

	// n_ is not computed here.
	template<class _Uint>
	BlasPermutation<_Uint>::BlasPermutation(const _Uint *P, const _Uint & r ) :
	       	r_(r), n_((_Uint)-1),P_(0),Q_(0),inv_(false)
	{
		// std::cout << "CTOR 2" << std::endl;
		// std::cout << "got : perm of " << r << std::endl;
		// for (size_t i = 0 ; i< r ; ++i) std::cout << P[i] << ' ' ;
		// std::cout<<std::endl;
		if (!r) {
			// n_ = 0 ;
			return ;
		}
		P_.resize(r_);
		for (_Uint  i = 0 ; i < r_ ; ++i) {
			P_[i] = P[i] ;
		}

		// std::cout << "return : perm  (" << cleaned_ << ") of " << r_ << std::endl;
		// for (size_t i = 0 ; i< r_ ; ++i) std::cout << P_[i] << ' ' ;
		// std::cout<<std::endl;

		return ;

	}

	// n_ is not computed here.
	template<class _Uint>
	BlasPermutation<_Uint>::BlasPermutation(const std::vector<_Uint> & P) :
	       	r_(P.size()), n_((_Uint)-1),P_(0),Q_(0),inv_(false)
	{
		if ( !r_ )  {
			// n_ = 0 ;
			return ;
		}
		P_ = P;
		return ;

	}

	// n_ is not computed here.
	template<class _Uint>
	BlasPermutation<_Uint>::BlasPermutation(const MatrixPermutation<_Uint> & P) :
	       	r_(P.getSize()), n_((_Uint)/*P.getSize()*/-1),P_(P.getSize()),Q_(P),inv_(false)
	{
		if ( !r_ ) {
			return ;
		}
		std::vector<_Uint> Qinv(n_);
		InvertQ_(Qinv);
		BuildP_(Qinv,Q_);
	}

	// n_ is computed here
	template<class _Uint>
	_Uint
	BlasPermutation<_Uint>::getSize() const
	{
		// std::cout << "getting size (" << r_ << ") :";
		// this->write(std::cout) << std::endl ;

		// std::cout << " was " << n_ << std::endl;
		if ( n_ == (_Uint) -1 ) { //! @warning potentially catastrophic
			if (!r_)
				n_ = 0 ;
			else
				n_ = (*(std::max_element(P_.begin(),P_.end())))+1 ;
		}
		// std::cout << " is " << n_ << std::endl;
		return n_ ;
	}

	template<class _Uint>
	_Uint
	BlasPermutation<_Uint>::getOrder() const
	{
		return r_ ;
	}

	template<class _Uint>
	void BlasPermutation<_Uint>::setOrder( size_t r)
	{
		r_ = r  ;
		n_ = (_Uint) -1 ;
	}


	template<class _Uint>
	MatrixPermutation<_Uint> &
	BlasPermutation<_Uint>::Convert (MatrixPermutation<_Uint> & P)
	{
		getSize() ;   // si c'était pas déjà fait...
		P.resize(n_); // sets P to identity
		for (_Uint i = 0 ; i < n_ ; ++i)
			std::swap(P[i],P[P_[i]]);
		return P ;
	}

	/// compresses BlasPermutation to a smaller \c r_.
	template<class _Uint>
	void BlasPermutation<_Uint>::Compress()
	{
		// std::cout << r_ << std::endl;
		if (!r_) {
			linbox_check(!n_);
			P_.resize(0) ;
			return ;
		}
		_Uint rr = r_-1 ;
		while ( rr && (P_[rr] == 0  )) --rr ;    // removing trailing zeros
		while ( rr && (P_[rr] == rr )) --rr ;    // useless information
		if ((rr == 0) && (P_[0] == 0)) {
			r_ = 0 ;
			n_ = 0  ;
			P_.resize(0) ;
			return ;
		}
		r_ = rr+1 ;
		P_.resize(r_,0);   // done cleaning.
		n_ = -1 ;
		// recomputing n_ if lost.
		// if (n_ != (_Uint) -1) {
			// n_ = getSize();
		// }
		return ;
	}

	template<class _Uint>
	void BlasPermutation<_Uint>::InitQ_() const
	{
		getSize();
		Q_.resize(n_);
		for (_Uint i = 0 ; i < n_ ; ++i) Q_[i] = i ;
	}

	template<class _Uint>
	void BlasPermutation<_Uint>::Transpose()
	{
		Invert();
	}

	template<class _Uint>
	void BlasPermutation<_Uint>::Invert()
	{
		if (!r_) {
			return ;
		}
		if (inv_) {
			inv_ = false ;
			return ;
		}
		inv_ = false ;
		getSize();
		/* if not already computed, build standard permuation Q_ */
		BuildQ_();
		std::vector<_Uint> Qinv(n_) ;
		/* invert standard matrix Q_*/
		InvertQ_(Qinv);
		/*  recover P_ from Qinv) */
		BuildP_(Q_,Qinv);
		/*  free Q_ (no longer representing P_ */
		Q_.resize(0); // Q_ = Qinv ?
		return ;
	}

	// P = convert(Q), using Qinv
	template<class _Uint>
	void BlasPermutation<_Uint>::BuildP_( std::vector<_Uint> &Q, std::vector<_Uint> &Qinv)
	{
		linbox_check( r_ );
		P_.resize(getSize());
		/*  building */
		// std::cout << "Buiding P (" << n_ << ")" << std::endl;
		_Uint pi,qi,qpi ;
		for (_Uint i = 0 ;i < n_ ; ++i) {
			pi  = P_[i]  = Qinv[i];
			if(i == pi) continue ;
			qi  = Q[i];
			qpi = Q[pi] ;
			std::swap(Q[i],Q[pi]);
			std::swap(Qinv[qi],Qinv[qpi]);
		}
		/*  cleaning */
		linbox_check(n_ && (n_ != (_Uint)-1) );
		r_ = n_-1 ;
		Compress();
	}

	// apply P_ to identity to get Q_
	template<class _Uint>
	void BlasPermutation<_Uint>::BuildQ_() const
	{
		if ((_Uint)Q_.size() == n_) return ; // si Q_ est déjà initialisée, alors P_ == Q_
		// set Q_ to identity
		InitQ_();
		// then permute it
		// faster if P_ is ::Compress()ed
		for (_Uint i = 0 ; i < r_ ; ++i) {
			if (P_[i]>i) {
				std::swap(Q_[i],Q_[P_[i]]);
			}
		}
		return ;
	}

	template<class _Uint>
	bool BlasPermutation<_Uint>::CheckP_()
	{
		for (_Uint i = 0 ; i < r_ ; ++i)
			if (P_[i] && P_[i] < i)
				return false ;
		return true ;
	}

	// invert a standard permutation
	template<class _Uint>
	std::vector<_Uint> & BlasPermutation<_Uint>::InvertQ_(std::vector<_Uint> & Qinv)
	{
		linbox_check(n_ != (_Uint) -1);
		for (_Uint i = 0 ; i < n_ ; ++i)
			Qinv[Q_[i]] = i ;
		return Qinv ;
	}

#if 0
	template<class _Uint>
	inline _Uint
	BlasPermutation<_Uint>::operator[](const _Uint & i)
	{
		BuildQ_() ;
		linbox_check( i<Q_.size() ) ;
		return Q_[i] ;
	}
#endif

	template<class _Uint>
	inline  _Uint
	BlasPermutation<_Uint>::operator[](const _Uint  i) const
	{
		if (!r_) return i ;
		getSize() ;
		BuildQ_() ;
		linbox_check(n_ == Q_.size() );
		if (i >= n_)
			return i ;
		return Q_[i] ;
	}


	/* ****** */
	/* output */
	/* ****** */
	template<class _Uint>
	std::ostream & BlasPermutation<_Uint>::write (std::ostream & o, bool Lapack) const
	{
		if (Lapack) {
			o << '['  ;
			_Uint i = 0 ;
			if (r_) {
				if (r_ > 1) {
					for ( ; i < r_-1 ; ++i)
						o << P_[i] << ',';
				}
				o << P_[i] ;
			}
			o  << ']' ;
			if (inv_) o << "^{-1}" ;
			o << '(' << (long int) (n_+1)-(long int)1 << ')' ;
		}
		else {
			// std::cout << "order : " << r_ << std::endl;
			// std::cout << "P_ := " << (std::vector<_Uint>)P_ << std::endl;
			// std::cout << "Q_ := " << (std::vector<_Uint>)Q_ << std::endl;
			// std::cout << Q_.size() << std::endl;
			BuildQ_() ;
			// std::cout << "Q_ := " << (std::vector<_Uint>)Q_ << std::endl;
			// std::cout << Q_.size() << std::endl;
			o << '['  ;
			_Uint i = 0 ;
			if (n_) {
				if (n_ > 1) {
					for ( ; i < n_-1 ; ++i)
						o << Q_[i] << ',';
				}
				o << Q_[i] ;
			}
			o  << ']' ;
			if (inv_) o << "^{-1}" ;
			o << '(' << (long int) (n_+1)-(long int)1 << ')' ;
		}
		return o;
	}

	template<class _Uint>
	std::ostream & operator<<(std::ostream &o, BlasPermutation<_Uint> & P)
	{
		return P.write(o) ;
	}

#if 0

	/* ******* */
	/*  Apply  */
	/* ******* */

	template<class _Uint>
	template<class OutVector, class InVector>
	OutVector &BlasPermutation<_Uint>::apply (OutVector &y, const InVector &x)
	{
		linbox_check((_Uint)x.size() == getSize());
		linbox_check((_Uint)y.size() == getSize());
		y = x ; // no need for Field operations...
		for (_Uint i = 0 ; i < r_ ; ++i)
			std::swap(y[i],y[P_[i]]) ;

		return y ;
	}

	template<class _Uint>
	template<class OutVector, class InVector>
	OutVector &BlasPermutation<_Uint>::applyTranspose (OutVector &y, const InVector &x)
	{
		linbox_check((_Uint)x.size() == getSize());
		linbox_check((_Uint)y.size() == getSize());
		y = x ; // no need for Field operations...
		_Uint i = r_ ;
		for ( ; i-- ; )
			std::swap(y[i],y[P_[i]]) ;

		return y ;
	}

#endif

#if 0
	/* *************** */
	/*  Transposition  */
	/* *************** */

	template<class _Uint>
	void BlasPermutation<_Uint>::TransposeRows(_Uint i, _Uint j)
	{
		if (i == j) return ;
		linbox_check(i<getSize());
		linbox_check(j<getSize());
		BuildQ_() ;
		std::vector<_Uint> Qinv(n_) ;
		InvertQ_(Qinv);
		std::swap(Q_[Qinv[i]],Q_[Qinv[j]]);
		std::swap(Qinv[i],Qinv[j]);
		BuildP_(Qinv,Q_);
		Q_.resize(0);

	}

	template<class _Uint>
	void BlasPermutation<_Uint>::TransposeCols(_Uint i, _Uint j)
	{
		if (i == j) return ;
		linbox_check(i<getSize());
		linbox_check(j<getSize());
		BuildQ_() ;
		std::swap(Q_[i],Q_[j]);
		std::vector<_Uint> Qinv(n_) ;
		InvertQ_(Qinv);
		BuildP_(Qinv,Q_);
		Q_.resize(0);

	}
#endif

}


namespace LinBox
{
	template<class _UnsignedInt>
	MatrixPermutation<_UnsignedInt>::MatrixPermutation() :
	       	n_(0), P_(0)
       	{}

	template<class _UnsignedInt>
	MatrixPermutation<_UnsignedInt>::MatrixPermutation(const _UnsignedInt *P, const _UnsignedInt &n) :
		n_(n), P_(n)
	{
		for (_UnsignedInt i = 0 ; i < n ; ++i)
			P_[i] = P[i] ;
	}

	template<class _UnsignedInt>
	MatrixPermutation<_UnsignedInt>::MatrixPermutation(const std::vector<_UnsignedInt> & P) :
		n_(P.size()), P_(P)
       	{}

	template<class _UnsignedInt>
	inline  _UnsignedInt
	MatrixPermutation<_UnsignedInt>::operator[](const _UnsignedInt  i) const
	{
		return P_[i] ;
	}


	template<class _UnsignedInt>
	_UnsignedInt
	MatrixPermutation<_UnsignedInt>::getSize() const
	{ return n_ ; }

	template<class _UnsignedInt>
	void
	MatrixPermutation<_UnsignedInt>::resize( _UnsignedInt  n)
	{
		if (n < n_) {
#ifdef DEBUG
			/* checking that we do only remove terms
			 * that don't alter the fact P_ is a permuation of [[1,n]].
			 */
			bool lost = false ;
			for (_UnsignedInt i = n ; !lost && i < n_ ; ++i)
				if (P_[i]<n-1) lost = true ;
			if (lost)
				std::cerr << "Warning ! (in " << __FILE__ << " at " << __func__ << " (" << __LINE__ << ") your permutation is no longer consistent" << std::endl;
#endif
		}
		/* resizing to identity */
		P_.resize(n);
		for (_UnsignedInt i = n_ ; i< n ; ++i)
			P_[i] =  i ;
		n_ = n ;

		return ;
	}


	/* ****** */
	/* output */
	/* ****** */
	template<class _UnsignedInt>
	std::ostream & MatrixPermutation<_UnsignedInt>::write (std::ostream & o) const
	{
		o << '['  ;
		for (_UnsignedInt i = 0 ; i < n_ ; ++i)
		{ o << P_[i]  ; if (i< n_-1) o << ','; }
		o << ']' ;
		return o;
	}


	template<class _UnsignedInt>
	std::ostream & operator<<(std::ostream &o, MatrixPermutation<_UnsignedInt> & P)
	{
		return P.write(o) ;
	}

	template<class _UnsignedInt>
	void MatrixPermutation<_UnsignedInt>::Transpose()
	{
		/* not in place */
		std::vector<_UnsignedInt> Q(n_) ;
		for (_UnsignedInt i = 0 ; i < (_UnsignedInt) n_ ; ++i)
			Q[P_[i]] = i ;
		P_ = Q ;
		/* in place */
		//! @todo in place ! (revient à parcourir des cycles)

	}

	template<class _UnsignedInt>
	void MatrixPermutation<_UnsignedInt>::Invert()
	{
		Transpose() ;
	}

	template<class _UnsignedInt>
	MatrixPermutation<_UnsignedInt> & MatrixPermutation<_UnsignedInt>::Transpose(Self_t &Mt)
	{
		//Mt(*this);
		Mt.P_ = P_;
		Mt.n_ = n_;
		Mt.Transpose();
		return Mt ;
	}

	template<class _UnsignedInt>
	MatrixPermutation<_UnsignedInt> & MatrixPermutation<_UnsignedInt>::Invert(Self_t &Mt)
	{
		return Transpose(Mt) ;
	}


	//        Self_t & TransposeCols(_UnsignedInt i, _UnsignedInt j);

	template<class _UnsignedInt>
	template<class OutVector, class InVector>
	OutVector &MatrixPermutation<_UnsignedInt>::apply (OutVector &y, const InVector &x) const
	{
		linbox_check((_UnsignedInt)x.size() == n_);
		linbox_check((_UnsignedInt)y.size() == n_);
		_UnsignedInt i = n_;
		for (;i--;)
			y[i] = x[P_[i]] ; // no need for Field operations...

		return y ;
	}

	template<class _UnsignedInt>
	template<class OutVector, class InVector>
	OutVector &MatrixPermutation<_UnsignedInt>::applyTranspose (OutVector &y, const InVector &x) const
	{
		linbox_check((_UnsignedInt)x.size() == n_);
		linbox_check((_UnsignedInt)y.size() == n_);
		_UnsignedInt i = n_;
		for (;i--;)
			y[P_[i]] = x[i] ; // no need for Field operations...

		return y ;
	}

	template<class _UnsignedInt>
	void MatrixPermutation<_UnsignedInt>::TransposeCols(_UnsignedInt i, _UnsignedInt j)
	{
		linbox_check(i<n_);
		linbox_check(j<n_);
		if (i == j) return ;
		std::swap(P_[i],P_[j]);
	}

	template<class _UnsignedInt>
	void MatrixPermutation<_UnsignedInt>::TransposeRows(_UnsignedInt i, _UnsignedInt j)
	{
		linbox_check(i<n_);
		linbox_check(j<n_);
		if (i == j) return ;
		_UnsignedInt iloc = 0 ;
		_UnsignedInt jloc = 0 ;
		_UnsignedInt l = 0 ;
		for ( ; l < n_ && !(iloc && jloc) ; ++l)
			if (P_[l] == i)
				iloc = l+1;
			else if (P_[l] == j)
				jloc = l+1;
		linbox_check(iloc);
		linbox_check(jloc);
		--iloc ;
		--jloc ;
		std::swap(P_[iloc],P_[jloc]);

	}

}

#endif //__LINBOX_matrix_permutation_INL


// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,:0,t0,+0,=s
// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:

