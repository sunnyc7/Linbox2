# Copyright (c) 2010 the LinBox group
# ========LICENCE========
# This file is part of the library LinBox.
#
# LinBox is free software: you can redistribute it and/or modify
# it under the terms of the  GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
# ========LICENCE========



pkgincludesubdir=$(pkgincludedir)/blackbox

BASIC_HDRS =			\
	archetype.h               \
	factory.h                 \
	butterfly.h               \
	hilbert.h                 \
	compose.h                 \
	permutation.h             \
	squarize.h                \
	sparse1.h                 \
	sparse.h                  \
	sparse.inl                \
	scalar-matrix.h           \
	submatrix.h               \
	inverse.h                 \
	transpose.h               \
	sum.h                     \
	diagonal.h                \
	diagonal-gf2.h            \
	moore-penrose.h           \
	direct-sum.h              \
	null-matrix.h             \
	matrix-blackbox.h         \
	dif.h                     \
	companion.h               \
	frobenius.h               \
	zero-one.h                \
	zero-one.inl              \
	zo.h         zo.inl       \
	zo-gf2.h     zo-gf2.inl   \
	quad-matrix.h             \
	triplesbb.h               \
	apply.h                   \
	submatrix-traits.h        \
	random-matrix-traits.h    \
	scompose.h                \
	blackbox-interface.h      \
	random-matrix.h           \
	lambda-sparse.h           \
	polynomial.h              \
	block-hankel.h            \
	block-toeplitz.h          \
	block-hankel-inverse.h    \
	jit-matrix.h              \
	rational-matrix-factory.h

NTL_HDRS =			\
	toeplitz.h ntl-hankel.h ntl-sylvester.h \
	toeplitz.inl ntl-hankel.inl ntl-sylvester.inl

GIVARO_HDRS =

LIDIA_HDRS =

if LINBOX_HAVE_NTL
USE_NTL_HDRS = $(NTL_HDRS)
endif

if LINBOX_HAVE_GIVARO
USE_GIVARO_HDRS = $(GIVARO_HDRS)
endif

if LINBOX_HAVE_LIDIA
USE_LIDIA_HDRS = $(LIDIA_HDRS)
endif

pkgincludesub_HEADERS =		\
	$(BASIC_HDRS)		\
	$(USE_NTL_HDRS)		\
	$(USE_GIVARO_HDRS)	\
	$(USE_LIDIA_HDRS)

EXTRA_DIST = 			\
	$(NTL_HDRS)		\
	$(GIVARO_HDRS)		\
	$(LIDIA_HDRS)		\
	blackbox.doxy

