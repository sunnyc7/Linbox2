dnl Check for IML
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
dnl Boyer Brice 07/04/11
dnl Bradford Hovinen, 2001-06-13
dnl Modified by Pascal Giorgi, 2003-12-03
dnl Inspired by gnome-bonobo-check.m4 by Miguel de Icaza, 99-04-12
dnl Stolen from Chris Lahey       99-2-5
dnl stolen from Manish Singh again
dnl stolen back from Frank Belew
dnl stolen from Manish Singh
dnl Shamelessly stolen from Owen Taylor

dnl LB_CHECK_IML ([MINIMUM-VERSION [, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl
dnl Test for IML and define IML_CFLAGS and IML_LIBS

AC_DEFUN([LB_CHECK_IML],
[

AC_ARG_WITH(iml,
[AC_HELP_STRING([--with-iml=<path>|yes], [Use IML library. This library is (not yet) mandatory for
    LinBox compilation. If argument is yes or <empty> or <bad> :)
    that means the library is reachable with the standard
    search path (/usr or /usr/local). Otherwise you give
    the <path> to the directory which contains the
    library.
])],
    [if test "$withval" = yes ; then
        IML_HOME_PATH="${DEFAULT_CHECKING_PATH}"
        elif test "$withval" != no ; then
        IML_HOME_PATH="$withval ${DEFAULT_CHECKING_PATH}"
        fi],
    [IML_HOME_PATH="${DEFAULT_CHECKING_PATH}"])

dnl  min_iml_version=ifelse([$1], ,1.0.3,$1)


dnl Check for existence
BACKUP_CXXFLAGS=${CXXFLAGS}
BACKUP_LIBS=${LIBS}

AC_MSG_CHECKING(for IML)

for IML_HOME in ${IML_HOME_PATH}
  do
    if test -r "$IML_HOME/include/iml.h"; then

       if test "x$IML_HOME" != "x/usr" -a "x$IML_HOME" != "x/usr/local"; then
           IML_CFLAGS="-I${IML_HOME}/include"
           IML_LIBS="-L${IML_HOME}/lib -liml"
       else
           IML_CFLAGS=
           IML_LIBS="-liml"
       fi

       CXXFLAGS="${BACKUP_CXXFLAGS} ${IML_CFLAGS} ${GMP_CFLAGS}"
       LIBS="${BACKUP_LIBS} ${IML_LIBS} ${GMP_LIBS}"

       AC_TRY_LINK(
       [#include <gmp.h>
	   extern "C" {
	   #include <iml.h>
	   }],
       [Double a;],
       [
	   AC_TRY_RUN(
	   [
	   int main () { return 0; /* not possible to check version */ }
	   ],[
	   iml_found="yes"
	   break
	   ],[
	   iml_problem="$problem $IML_HOME"
	   unset IML_CFLAGS
	   unset IML_LIBS
	   ],[
	   iml_found="yes"
	   iml_cross="yes"
	   break
	   ])
	   ],
       [
       iml_found="no"
       iml_checked="$checked $IML_HOME"
       unset IML_CFLAGS
       unset IML_LIBS
       ])
	   dnl  AC_MSG_RESULT(found in $iml_checked ? $iml_found)
    else
       iml_found="no"
	   dnl  AC_MSG_RESULT(not found at all $IML_HOME : $iml_found)
    fi
done

if test "x$iml_found" = "xyes" ; then
    AC_SUBST(IML_CFLAGS)
    AC_SUBST(IML_LIBS)
    AC_DEFINE(HAVE_IML,1,[Define if IML is installed])
    HAVE_IML=yes
    if test "x$iml_cross" != "xyes"; then
        AC_MSG_RESULT(found)
    else
        AC_MSG_RESULT(unknown)
        echo "WARNING: You appear to be cross compiling, so there is no way to determine"
        echo "whether your IML version is new enough. I am assuming it is."
    fi
    ifelse([$2], , :, [$2])
elif test -n "$iml_problem"; then
    AC_MSG_RESULT(problem)
    echo "Sorry, your IML version is too old. Disabling."
    ifelse([$3], , :, [$3])
elif test "x$iml_found" = "xno" ; then
    AC_MSG_RESULT(not found)
    ifelse([$3], , :, [$3])
fi

AM_CONDITIONAL(LINBOX_HAVE_IML, test "x$HAVE_IML" = "xyes")

CXXFLAGS=${BACKUP_CXXFLAGS}
LIBS=${BACKUP_LIBS}
#unset LD_LIBRARY_PATH

])
