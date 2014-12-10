/* Copyright (C) 2010 LinBox
 * Written by Zhendong Wan
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

#ifndef __LINBOX_randiter_ntl_zz_H
#define __LINBOX_randiter_ntl_zz_H

#include <ctime>
#include <vector>

namespace LinBox
{

	class NTL_ZZ;

	class NTL_ZZRandIter {
	public:
		typedef NTL::ZZ  Element;

		NTL_ZZRandIter (const NTL_ZZ& F,
				const integer& size = 0,
				const integer& seed = 0
			       ) {

			_size = NTL::to_ZZ(std::string(size).data());

			if (seed == integer(0)) NTL::SetSeed (NTL::to_ZZ(time(NULL)));

			else NTL::SetSeed(NTL::to_ZZ(std::string(seed).data()));
		}

		Element& random (Element& x) const
		{

			if (_size == 0)
				NTL::RandomLen (x, 30);
			else
				NTL::RandomBnd (x, _size);

			return x;
		}

	private:
		Element _size;

	};

} // namespace LinBox

#endif //  __LINBOX_randiter_ntl_zz_H


// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,:0,t0,+0,=s
// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:

