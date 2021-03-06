/* linbox/blackbox/rational-reconstruction-base.h
 * Copyright (C) 2009 Anna Marszalek
 *
 * Written by Anna Marszalek <aniau@astronet.pl>
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
 */

#ifndef __LINBOX_reconstruction_base_H
#define __LINBOX_reconstruction_base_H

#define DEF_RR_THRESH  1

#include <iostream>
#include <deque>
#include <math.h>

#include "linbox/field/PID-integer.h"

namespace LinBox
{

	enum RReconstructionSchedule {
		INCREMENTAL, QUADRATIC, GEOMETRIC, CERTIFIED
	};

	template <class Ring=PID_integer>
	class RReconstructionBase;

	/*
	 * Class to be used for repeated rational reconstruction in schemes such as Rational CRA and p-adic lifting
	 * together with method reconstructRational(a,b,x,m)
	 * implements scheduling INCREMENTAL, QUADRATIC, GEOMETRIC, CERTIFIED
	 * implements vector reconstruction
	 * _intRing is the integer ring used for reconstruction, default PID_integer
	 * _RR is the rational reconstruction method, see fast-ratioinal-reconstruction.h, classic-rational-reconstruction.h
	 * THRESHOLD_ - treshold for INCREMENTAL schedule
	 * rbound_ - min number of iterations for all schedule
	 */
	template <class Ring=PID_integer, class RRBase=RReconstructionBase<PID_integer> >
	struct RReconstruction {
	protected:
		Ring _intRing;
		RRBase _RR;
		mutable size_t RecCounter;
		const RReconstructionSchedule  _meth;
		const size_t THRESHOLD_;
		const size_t rbound_;
	public:
		typedef typename Ring::Element Element;

		RReconstruction(const Ring& Z=Ring(), const RReconstructionSchedule Meth = GEOMETRIC, size_t T=DEF_RR_THRESH, size_t b= 0) :
			_intRing(Z), _RR(Z), _meth(Meth), THRESHOLD_(T), rbound_(b)
		{
			RecCounter =0;
			if (_meth == QUADRATIC) {
				RecCounter = (int)sqrt((double)rbound_);//RecCounter^2 < rbound_ <=(RecCounter+1)^2
			}
			else if (_meth == GEOMETRIC) {
				RecCounter = (size_t) log((double)rbound_) ;//2^RecCounter < rbound_ <=2^(RecCounter+1)
			}
		}

		RReconstruction(const RRBase& RR, const RReconstructionSchedule Meth = GEOMETRIC, size_t T=DEF_RR_THRESH, size_t b = 0) :
			_intRing(RR._intRing), _RR(RR),_meth(Meth), THRESHOLD_(T), rbound_(b)
		{
			RecCounter =0;
			if (_meth == QUADRATIC) {
				RecCounter = (size_t)sqrt((double)rbound_);//RecCounter^2 < rbound_ <=(RecCounter+1)^2
			}
			else if (_meth == GEOMETRIC) {
				RecCounter = (size_t)((double)log((double)rbound_)/log(2.));//2^RecCounter < rbound_ <=2^(RecCounter+1)
			}

		}

		int getCounter()
		{
		       	return RecCounter;
		}

		bool scheduled(const size_t i) const
		{
			//if (RecCounter ==0)  return true;
			if (i < rbound_) return false; //skip first rbound iterations
			if (_meth == INCREMENTAL) {
				if (RecCounter%THRESHOLD_==0 ) return true;
				else return false;
			}
			else if (_meth == QUADRATIC) {
				if (RecCounter*RecCounter < i) return true;
				else return false;
			}
			else if (_meth == GEOMETRIC) {
				if ((1UL << RecCounter) < i) return true;
				else return false;
			}
			else if (_meth == CERTIFIED) {
				if ( i > rbound_) return true;
				else return false;
			}
			return true;
		}

		template <class Vect>
		bool reconstructRational(Vect& a, Element& b, const Vect& x, const Element m, const int inc = 1) const
		{
			++RecCounter;
			b = 1;
			if (a.size() != x.size()) return false;
			typename Vect::iterator it_a,it2_a;
			typename Vect::const_iterator it_x;
			bool res = true;
			Element new_den, old_den;
			old_den = 1;
			if (inc==1) {
				for (it_a = a.begin(), it_x = x.begin(); it_a != a.end(); ++it_a, ++it_x) {
					//use previous modula
					Element x_in(*it_x);
					x_in *=old_den;
					if (x_in <0) {
						if ((-x_in) > m) x_in %= m;
						if (x_in < 0) x_in += m;
					}
					else {
						if (x_in > m) x_in %= m;
					}
					if (x_in > 0) res = res && _RR.reconstructRational(*it_a, new_den,x_in,m);
					else {
						res = true;
						*it_a = 0;
						new_den = 1;
					}
					if (!res) return res;
					else {
						if (new_den > 1) {
							for (it2_a = a.begin(); it2_a != it_a; ++it2_a) {
								*it2_a *=new_den;
							}
							b *= new_den;
							old_den *= new_den;
						}
					}

				}
			}
			else {//if (inc == -1)
				int i = (int)x.size()-1;
				for (; i >=0; --i ) {
					Element x_in(x[i]);
					x_in *=old_den;
					if (x_in <0) {
						if ((-(x_in)) > m) x_in %= m;
						if (x_in < 0) x_in += m;
					}
					else {
						if (x_in > m) x_in %= m;
					}
					if (x_in > 0) res = res && _RR.reconstructRational(a[i], new_den,x_in,m);
					else {
						res = true;
						*it_a = 0;
						new_den = 1;
					}
					if (!res) return res;
					else {
						//std::cout << a[i] << "/" << b*new_den << "\n";
						if (new_den > 1) {
							for (int j = (int)a.size()-1; j > i ; --j) {
								a[j] *=new_den;
							}
							b *= new_den;
							old_den *= new_den;
						}
					}
				}
			}
#if 0
			else if (inc==0) {//no prec
			}
#endif
			return res;

		}

		bool reconstructRational(Element& a, Element& b, const Element& x, const Element& m) const
		{
			++RecCounter;
			Element x_in(x);
			if (x<0) {
				if ((-x)>m)
					x_in %= m;
				if (x<0)
					x_in += m;
			}
			else {
				if (x>m)
					x_in %= m;
			}

			bool res;
			if (x_in >0) res = _RR.reconstructRational(a,b,x_in,m);
			else { a = 0; b =1; res = true;}
			//_RR.write(std::cout);
			return res;
		}

		bool reconstructRational(Element& a, Element& b, const Element& x, const Element& m, const Element& a_bound) const
		{
			++RecCounter;
			Element x_in(x);
			if (x<0) {
				if ((-x)>m)
					x_in %= m;
				if (x<0)
					x_in += m;
			}
			else {
				if (x>m)
					x_in %= m;
			}
			bool res;
			if (x_in >0) res = _RR.reconstructRational(a,b,x_in,m,a_bound);
			else { a = 0; b =1; res = true;}
			//_RR.write(std::cout);
			return res;
		}

		bool reconstructRational(Element& a, Element& b, const Element& x, const Element& m, const Element& a_bound, const Element& b_bound) const
		{
			++RecCounter;
			Element x_in(x);
			if (x<0) {
				if ((-x)>m)
					x_in %= m;
				if (x<0)
					x_in += m;
			}
			else {
				if (x>m)
					x_in %= m;
			}
			Element bound = x_in/b_bound;
			if (x_in > 0) _RR.reconstructRational(a,b,x_in,m,(bound>a_bound?bound:a_bound));
			else  { a = 0; b =1; }
			bool res=  (b > b_bound)? false: true;
			//_RR.write(std::cout);
			return res;
		}

		//fastReconstruction();

		//classicReconstruction();

	};

	class OpCounter {
	public:
		size_t div_counter;
		size_t mul_counter;
		size_t gcd_counter;

		OpCounter() {
			div_counter=0; mul_counter=0; gcd_counter=0;
		}

		void write(std::ostream& is) {
			is << div_counter << " divisions\n";
			is << mul_counter << " multiplications\n";
			is << gcd_counter << " gcds\n";
		}
	};

	template <class Ring>
	class RReconstructionBase {
	public:
		Ring _intRing;
		mutable OpCounter C;
		typedef typename Ring::Element Element;

		RReconstructionBase(const Ring& Z) :
			_intRing(Z)
		{}
		RReconstructionBase(const RReconstructionBase<Ring>& RR) :
			_intRing(RR._intRing)
		{}

		virtual bool reconstructRational(Element& a, Element& b, const Element& x, const Element& m) const =0;

		virtual bool reconstructRational(Element& a, Element& b, const Element& x, const Element& m, const Element& a_bound) const =0;

		virtual ~RReconstructionBase() {}

		void write(std::ostream& is) const
		{
			C.write(is);
		}
	};

	/*
	 * This is the default RReconstruction, using PID_Integer and ClassicRationalReconstruction of Wang
	 */
	template <>
	class RReconstructionBase<PID_integer> {
	public:
		typedef PID_integer Ring;
		Ring _intRing;
		mutable OpCounter C;
		typedef Ring::Element Element;

		RReconstructionBase(const Ring& Z) :
			_intRing(Z)
		{}
		RReconstructionBase(const RReconstructionBase<Ring>& RR) :
			_intRing(RR._intRing)
		{}

		bool reconstructRational(Element& a, Element& b, const Element& x, const Element& m)
		{
			Element a_bound; _intRing.sqrt(a_bound,m/2);
			return _intRing.reconstructRational(a,b,x,m,a_bound,a_bound);
		}

		bool reconstructRational(Element& a, Element& b, const Element& x, const Element& m, const Element& a_bound)
		{
			_intRing.reconstructRational(a,b,x,m,a_bound);
			return true;
		}

		~RReconstructionBase() {}
#if 0
		const void write(std::ostream& is) {
			C.write(is);
		}
#endif
	};

} //namespace LinBox

#undef DEF_RR_THRESH
#endif



// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,:0,t0,+0,=s
// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:

