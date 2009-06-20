PHP_ARG_WITH(lua,for LUA support,
[  --with-lua[=DIR]     Include LUA support])

if test "$PHP_LUA" != "no"; then
	PHP_NEW_EXTENSION(lua, lua.c, $ext_shared)
	if test "$PHP_LUA" != "yes"; then
	    LUA_SEARCH_DIRS=$PHP_LUA
	else
	    LUA_SEARCH_DIRS="/usr/local /usr"
	fi
	for i in $LUA_SEARCH_DIRS; do
		if test -f $i/include/lua/lua.h; then
			LUA_DIR=$i
			LUA_INCDIR=$i/include/lua
		elif test -f $i/include/lua.h; then
			LUA_DIR=$i
			LUA_INCDIR=$i/include
		fi
	done

	if test -z "$LUA_DIR"; then
		AC_MSG_ERROR(Cannot find lua)
	fi

	LUA_LIBDIR=$LUA_DIR/$PHP_LIBDIR

	PHP_ADD_LIBRARY_WITH_PATH(lua, $LUA_LIBDIR, LUA_SHARED_LIBADD)

	PHP_CHECK_LIBRARY(lua,lua_new_state,
	[
		PHP_ADD_LIBRARY_WITH_PATH(lua, $LUA_LIBDIR, LUA_SHARED_LIBADD)
	], [], [
		-L$LUA_LIBDIR
	])

	PHP_ADD_INCLUDE($LUA_INCDIR)
	PHP_SUBST(LUA_SHARED_LIBADD)
	AC_DEFINE(HAVE_LUA,1,[ ])
fi
