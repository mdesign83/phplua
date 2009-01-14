dnl $Id: config.m4,v 1.1 2007/07/12 13:01:37 johannes Exp $
dnl config.m4 for extension lua

PHP_ARG_WITH(lua, for lua support,
[  --with-lua             Include lua support])
  
if test "$PHP_LUA" != "no"; then
  AC_MSG_CHECKING(for pkg-config)
  if test ! -f "$PKG_CONFIG"; then
    PKG_CONFIG=`which pkg-config`
  fi
  FOUND=0
  if test -f "$PKG_CONFIG"; then
    AC_MSG_RESULT(found)
    AC_MSG_CHECKING(for lua via pkg-config)
    CHECK_PC="lua lua-5.2 lua5.2 lua-5.1 lua5.1 lua-5.0 lua5.0"
    for PROBE in $CHECK_PC
    do
      if $PKG_CONFIG --exists $PROBE;then
        LUAVERSION=`pkg-config --modversion $PROBE`
        AC_MSG_RESULT(found. version $LUAVERSION)
        LDFLAGS="$LDFLAGS `$PKG_CONFIG --libs $PROBE`"
        CFLAGS="$CFLAGS `$PKG_CONFIG --cflags $PROBE`"
        FOUND=1
        break
      fi
    done
    if test $FOUND != 1; then
      AC_MSG_RESULT(not found)
      AC_MSG_ERROR(lua couldn't be found. Check PKG_CONFIG_PATH)
    fi
  else
    AC_MSG_RESULT(not found)
    AC_MSG_ERROR(In order to compile cairo you will need a working pkg-config
Use PKG_CONFIG to specify pkg-config in a non-standard location.)
  fi
  PHP_NEW_EXTENSION(lua, lua.c, $ext_shared)
fi
