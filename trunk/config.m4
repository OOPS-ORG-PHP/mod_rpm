dnl Id: config.m4,v 1.21 2004/07/27 08:51:49 oops Exp $
dnl config.m4 for extension korean

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(rpm, for RPM support,
[  --with-rpm[=PATH]         Include rpm support])

if test "$PHP_RPM" != "no"; then
  AC_DEFINE(HAVE_RPM,1,[ ])

  RPM_PARAMETER=$CFLAGS
  PHP_SUBST(CPPFLAGS)
  PHP_SUBST(LDFLAGS)

  SEARCH_PATH="/usr /usr/local $PHP_RPM"

  dnl header search
  HEADLIST="rpmcli.h rpmbuild.h header.h"
  RPM_HEADER_DIR=$CPPFLAGS
  for j in $HEADLIST; do
    AC_MSG_CHECKING(for $j header file)

    TMP_RPM_HEADER_DIR=
    for i in $CPPFLAGS $SEARCH_PATH; do
      if test -f "$i/$j" ; then
        TMP_RPM_HEADER_DIR="$i"
        break
      elif test -f "$i/include/$j" ; then
        TMP_RPM_HEADER_DIR="$i/include"
        break
      elif test -f "$i/include/rpm/$j" ; then
        TMP_RPM_HEADER_DIR="$i/include/rpm"
        break
      elif test -f "$i/rpm/include/$j" ; then
        TMP_RPM_HEADER_DIR="$i/rpm/include"
        break
      fi
    done

    if test -z "$TMP_RPM_HEADER_DIR"; then
      AC_MSG_ERROR([$j not found anywhere under $CPPFLAGS $SEARCH_PATH])
    fi

    AC_MSG_RESULT(yes)
    INCCHK=`echo $RPM_HEADER_DIR | grep "$TMP_RPM_HEADER_DIR\( \|$\)" 2> /dev/null`
    if test -z "$INCCHK"; then
      RPM_HEADER_DIR="$TMP_RPM_HEADER_DIR $RPM_HEADER_DIR"
    fi
  done

  dnl
  dnl library search
  dnl

  NEW_LDFLAGS=
  LIBLIST="rpm rpmdb rpmio rpmbuild popt z bz2"

  for j in $LIBLIST; do
    AC_MSG_CHECKING(for the location of lib$j)
    if test "$j" = "rpm" -o "$j" = "rpmdb" -o "$j" = "rpmio"; then
      fixdir=rpm
    elif test "$j" = "popt"; then
      fixdir=popt
    elif test "$j" = "z"; then
      fixdir=zlib
    elif test "$j" = "bz2"; then
      fixdir=bzip2
    fi

    RPM_LIB_DIR=
    for i in $LDFLAGS $SEARCH_PATH; do
      if test -f $i/lib$j.$SHLIB_SUFFIX_NAME -o -f $i/lib$j.a ; then
        RPM_LIB_DIR=$i
        break
      elif test -f $i/lib/lib$j.$SHLIB_SUFFIX_NAME -o -f $i/lib/lib$j.a ; then
        RPM_LIB_DIR=$i/lib
        break
      elif test -f $i/lib/$fixdir/lib$j.$SHLIB_SUFFIX_NAME -o -f $i/lib/$fixdir/lib$j.a ; then
        RPM_LIB_DIR=$i/lib/$fixdir
        break
      elif test -f $i/$fixdir/lib/lib$j.$SHLIB_SUFFIX_NAME -o -f $i/$fixdir/lib/lib$j.a ; then
        RPM_LIB_DIR=$i/$fixdir/lib
        break
      fi
    done

    if test -z "$RPM_LIB_DIR"; then
      AC_MSG_ERROR([lib$j.(a|so) not found anywhere under $LDFLAGS $SEARCH_PATH])
    fi

    AC_MSG_RESULT($RPM_LIB_DIR)
    LDCHK=`echo $NEW_LDFLAGS | grep "\-Wl,-rpath=$RPM_LIB_DIR\( \|$\)" 2> /dev/null`
    if test -z "$LDCHK"; then
      NEW_LDFLAGS="-Wl,-rpath=$RPM_LIB_DIR $NEW_LDFLAGS"
    fi

    PHP_ADD_LIBRARY_WITH_PATH($j, $RPM_LIBDIR, RPM_SHARED_LIBADD)
  done

  LDFLAGS=$NEW_LDFLAGS
  CPPFLAGS="$CPPFLAGS -DIAM_RPM"
  RPM_HEADER_DIR="$RPM_HEADER_DIR `pwd`/librpm"
  extra_src="parse.c librpm/rpm.c librpm/rpmqv.c librpm/lastmsg.c
             librpm/query.c librpm/rpmchecksig.c librpm/verify.c
             librpm/rpminstall.c librpm/problems.c"

  for i in $RPM_HEADER_DIR; do
    PHP_EXPAND_PATH($i, PHP_RPM_HEADER_DIR)
    PHP_ADD_INCLUDE($PHP_RPM_HEADER_DIR)
  done
  PHP_SUBST(RPM_PARAMTER)
  PHP_SUBST(RPM_SHARED_LIBADD)

  PHP_NEW_EXTENSION(rpm, php_rpm.c $extra_src, $ext_shared,, \\$(RPM_PARAMETER))
fi
