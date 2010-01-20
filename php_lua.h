/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2006 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id: php_lua.h,v 1.2 2008/03/31 09:50:39 sfox Exp $ */

#ifndef PHP_LUA_H
#define PHP_LUA_H

extern zend_module_entry lua_module_entry;
#define phpext_lua_ptr &lua_module_entry

#define PHP_LUA_VERSION "0.2.0-dev"

#ifdef PHP_WIN32
#define PHP_LUA_API __declspec(dllexport)
#else
#define PHP_LUA_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(lua);
PHP_MSHUTDOWN_FUNCTION(lua);
PHP_RINIT_FUNCTION(lua);
PHP_RSHUTDOWN_FUNCTION(lua);
PHP_MINFO_FUNCTION(lua);

PHP_FUNCTION(confirm_lua_compiled);	/* For testing, remove later. */

ZEND_BEGIN_MODULE_GLOBALS(lua)
        zend_bool load_standard_libs;
ZEND_END_MODULE_GLOBALS(lua)

/* In every utility function you add that needs to use variables 
   in php_lua_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as LUA_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define LUA_G(v) TSRMG(lua_globals_id, zend_lua_globals *, v)
#else
#define LUA_G(v) (lua_globals.v)
#endif

#endif	/* PHP_LUA_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
