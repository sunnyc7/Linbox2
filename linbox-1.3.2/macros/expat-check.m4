dnl Check for expat
dnl Copyright (c) the LinBox group
dnl This file is part of LinBox

 dnl ========LICENCE========
 dnl This file is part of the library LinBox.
 dnl 
 dnl LinBox is free software: you can redistribute it and/or modify
 dnl it under the terms of the  GNU Lesser General Public
 dnl License as published by the Free Software Foundation; either
 dnl version 2.1 of the License, or (at your option) any later version.
 dnl 
 dnl This library is distributed in the hope that it will be useful,
 dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
 dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 dnl Lesser General Public License for more details.
 dnl 
 dnl You should have received a copy of the GNU Lesser General Public
 dnl License along with this library; if not, write to the Free Software
 dnl Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 dnl ========LICENCE========
 dnl
dnl Rich Seagraves
dnl stolen from Pascal Giorgi, 2001-12-10
dnl Inspired by gnome-bonobo-check.m4 by Miguel de Icaza, 99-04-12
dnl Stolen from Chris Lahey       99-2-5
dnl stolen from Manish Singh again
dnl stolen back from Frank Belew
dnl stolen from Manish Singh
dnl Shamelessly stolen from Owen Taylor

dnl LB_CHECK_EXPAT ([MINIMUM-VERSION [, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl
dnl Test for expat Library and define EXPAT_CFLAGS and EXPAT_LIBS
AC_DEFUN([LB_CHECK_EXPAT],
[

AC_ARG_WITH(expat,
[AC_HELP_STRING([--with-expat=<path>|yes|no], [Use Expat library. If argument is no, you do not
                             have the library installed on your machine (set
			     as default). If argument is yes or <empty> that
			     means the library is reachable with the standard
			     search path (/usr or /usr/local). Otherwise you
			     give the <path> to the directory which contain the
			     library.
	     ])],
	     [if test "$withval" = yes ; then
			EXPAT_HOME_PATH="${DEFAULT_CHECKING_PATH}"
	      elif test "$withval" != no ; then
			EXPAT_HOME_PATH="$withval"
	     fi],
	     [])

min_expat_version=ifelse([$1], ,1.95,$1)


dnl Check for existence

BACKUP_CXXFLAGS=${CXXFLAGS}
BACKUP_LIBS=${LIBS}

if test -n "$EXPAT_HOME_PATH"; then
AC_MSG_CHECKING(for EXPAT >= $min_expat_version)
fi

for EXPAT_HOME in ${EXPAT_HOME_PATH}
 do
if test -r "$EXPAT_HOME/include/expat.h"; then

	if test "x$EXPAT_HOME" != "x/usr" -a "x$EXPAT_HOME" != "x/usr/local"; then
		EXPAT_CFLAGS="-I${EXPAT_HOME}/include"
		EXPAT_LIBS="-L${EXPAT_HOME}/lib -lexpat"
	else
		EXPAT_CFLAGS=
		EXPAT_LIBS="-lexpat"
	fi

	CXXFLAGS="${BACKUP_CXXFLAGS} ${EXPAT_CFLAGS} ${GMP_CFLAGS}"
	LIBS="${BACKUP_LIBS} ${EXPAT_LIBS} ${GMP_LIBS}"
	AC_TRY_LINK(
	[#include <expat.h>],
	[XML_Content_Type a;],
	[
	AC_TRY_RUN(
	[#include <expat.h>
	 int main () {  if(XML_MAJOR_VERSION < 1  || (XML_MAJOR_VERSION == 1 && XML_MINOR_VERSION < 95)) return -1;  else return 0; }
	],[
	expat_found="yes"
	break
	],[
	expat_problem="$problem $EXPAT_HOME"
	unset EXPAT_CFLAGS
	unset EXPAT_LIBS
	],[
	expat_found="yes"
	expat_cross="yes"
	break
	])
	],
	[
	expat_found="no"
	expat_checked="$checked $EXPAT_HOME"
	unset EXPAT_CFLAGS
	unset EXPAT_LIBS
	])
else
	expat_found="no"
fi
done

if test "x$expat_found" = "xyes" ; then
	AC_SUBST(EXPAT_CFLAGS)
	AC_SUBST(EXPAT_LIBS)
	AC_DEFINE(XMLENABLED,1,[Define if Expat is installed])
	HAVE_EXPAT=yes
	if test "x$expat_cross" != "xyes"; then
		AC_MSG_RESULT(found)
	else
		AC_MSG_RESULT(unknown)
		echo "WARNING: You appear to be cross compiling, so there is no way to determine"
		echo "whether your EXPAT version is new enough. I am assuming it is."
	fi
	ifelse([$2], , :, [$2])
elif test -n "$expat_problem"; then
	AC_MSG_RESULT(problem)
	echo "Sorry, your EXPAT version is too old. Disabling."
	ifelse([$3], , :, [$3])
elif test "x$expat_found" = "xno" ; then
	AC_MSG_RESULT(not found)
	ifelse([$3], , :, [$3])
fi

AM_CONDITIONAL(LINBOX_HAVE_EXPAT, test "x$HAVE_EXPAT" = "xyes")

CXXFLAGS=${BACKUP_CXXFLAGS}
LIBS=${BACKUP_LIBS}
#unset LD_LIBRARY_PATH

])

