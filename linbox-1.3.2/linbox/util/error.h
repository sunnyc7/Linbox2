/* linbox/util/error.h
 * Copyright (C) 1994-1997 Givaro Team
 *
 * Written by T. Gautier
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

#ifndef __LINBOX_util_error_H
#define __LINBOX_util_error_H

#include <cstring>
#include <iostream>

namespace LinBox
{

	// ------------------------------- LinboxError
	/** base class for execption handling in LinBox
	  \ingroup util
	  */
	class LinboxError {
		static const size_t max_error_string = 256;
	public:
		LinboxError (const char* msg = '\0') {
			std::strncpy(strg, msg, max_error_string);
			strg[max_error_string-1] = 0;
		};


		// -- virtual print of the error message
		virtual std::ostream &print (std::ostream &o) const
		{ return o << strg<<std::endl ; }

		// -- non virtual output operator
		friend std::ostream &operator << (std::ostream &o, const LinboxError &E);

		// - useful to setup a break point on it
		static void throw_error (const LinboxError &err)
		{ throw err; }

		virtual ~LinboxError() {}

	protected:
		char strg[max_error_string];
	};

	class LinboxMathError : public LinboxError {
	public:
		LinboxMathError (const char* msg) :
			LinboxError (msg)
		{};
	};

	class LinboxMathDivZero : public LinboxMathError {
	public:
		LinboxMathDivZero (const char* msg) :
			LinboxMathError (msg)
		{};
	};

	class LinboxMathInconsistentSystem : public LinboxMathError {
	public:
		LinboxMathInconsistentSystem (const char* msg) :
			LinboxMathError (msg)
		{};
	};

	// -- Exception thrown in input of data structure
	class LinboxBadFormat : public LinboxError {
	public:
		LinboxBadFormat (const char* msg) :
			LinboxError (msg)
		{};
	};

}

#ifdef LinBoxSrcOnly       // for all-source compilation
#include "linbox/util/error.C"
#endif

#endif // __LINBOX_util_error_H


// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,:0,t0,+0,=s
// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:

