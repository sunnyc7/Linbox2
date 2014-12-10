/* lb-blackbox.h
 * Copyright (C) 2005 Pascal Giorgi
 *
 * Written by Pascal Giorgi <pgiorgi@uwaterloo.ca>
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

#ifndef __LINBOX_lb_blackbox_H
#define __LINBOX_lb_blackbox_H

#include <lb-domain-collection.h>
#include <lb-blackbox-collection.h>

/***************************
 * Initializer of Blackbox *
 ***************************/
void UpdateBlackbox();


/*******************************************************
 * API to contruct a m x n zero blackbox over a domain *
 *******************************************************/
const BlackboxKey& createBlackbox(const DomainKey &k, size_t m, size_t n, const char* name=NULL);


/**********************************************************
 * API to contruct a blackbox over a domain from a stream *
 **********************************************************/
const BlackboxKey& createBlackbox(const DomainKey &k, std::istream &is, const char *name=NULL);


/************************************
 * API to copy an existing blackbox *
 ************************************/
const BlackboxKey& copyBlackbox(const BlackboxKey &k);


/********************************************
 * API to get the dimensions of  a blackbox *
 ********************************************/
typedef std::pair<size_t, size_t> BlackboxDimension;
BlackboxDimension getBlackboxDimension(const BlackboxKey &key);


/*******************************************
 * API to write a blackbox over an ostream *
 *******************************************/
void writeBlackbox (const BlackboxKey &key,  std::ostream &os);


/*******************************************
 * API to set a blackbox with random value *
 *******************************************/
void setBlackboxAtRandom(const BlackboxKey &k);


/**********************************************
 * API to rebind a blackbox over a new domain *
 **********************************************/
void rebindBlackbox(const BlackboxKey &Vkey, const DomainKey &Dkey);


/*******************************************
 * API to modify the current blackbox type *
 *******************************************/
void setBlackbox(const char* t);


/************************************
 * API to write info on a blackbox  *
 ************************************/
void writeBlackboxInfo(const BlackboxKey &k, std::ostream& os);

#endif

// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,:0,t0,+0,=s
// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:

