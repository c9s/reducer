dnl $Id$
dnl config.m4 for extension reducer

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(reducer, for reducer support,
dnl Make sure that the comment is aligned:
dnl [  --with-reducer             Include reducer support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(reducer, whether to enable reducer support,
dnl Make sure that the comment is aligned:
[  --enable-reducer           Enable reducer support])

if test "$PHP_REDUCER" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-reducer -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/reducer.h"  # you most likely want to change this
  dnl if test -r $PHP_REDUCER/$SEARCH_FOR; then # path given as parameter
  dnl   REDUCER_DIR=$PHP_REDUCER
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for reducer files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       REDUCER_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$REDUCER_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the reducer distribution])
  dnl fi

  dnl # --with-reducer -> add include path
  dnl PHP_ADD_INCLUDE($REDUCER_DIR/include)

  dnl # --with-reducer -> check for lib and symbol presence
  dnl LIBNAME=reducer # you may want to change this
  dnl LIBSYMBOL=reducer # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $REDUCER_DIR/$PHP_LIBDIR, REDUCER_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_REDUCERLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong reducer lib version or lib not found])
  dnl ],[
  dnl   -L$REDUCER_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(REDUCER_SHARED_LIBADD)

  PHP_NEW_EXTENSION(reducer, reducer.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
