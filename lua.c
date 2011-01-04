/*
   +----------------------------------------------------------------------+
   | LUA extension for PHP                                                |
   +----------------------------------------------------------------------+
   | Copyright (c) 2007 Johannes Schlueter                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Johannes Schlueter  <johannes@php.net>                       |
   |         Marcelo  Araujo     <msaraujo@php.net>                       |
   |         Andreas Streichardt <andreas.streichardt@globalpark.com      |
   +----------------------------------------------------------------------+
   */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_lua.h"

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#define EXT_LUA_VERSION PHP_LUA_VERSION

#ifdef LUA_STACK_DEBUG
#define LUA_STACK_START(L) int __lua_old_top = lua_gettop(L)
#define LUA_STACK_END(L) \
  if (__lua_old_top != lua_gettop(L)) {								\
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "wrong stack size (%i before operation, %i now)", __lua_old_top, lua_gettop(L)); \
  }
#else
#define LUA_STACK_START(L)
#define LUA_STACK_END(L)
#endif

#define LUA_POP_ZVAL(L, z) \
  php_lua_get_zval_from_stack(L, -1, z); \
lua_pop(L, 1)

ZEND_DECLARE_MODULE_GLOBALS(lua)

#define getLuaZ(var) (((php_lua_object*)zend_object_store_get_object(var TSRMLS_CC))->L)
#define getLuaY() ((php_lua_object*)zend_object_store_get_object(getThis() TSRMLS_CC))
#define getLua()     getLuaZ(getThis())

static zend_object_handlers lua_handlers;
static zend_class_entry *lua_ce;

typedef struct _php_lua_object {
  zend_object std;
  lua_State *L;
  zval* callbacks;
  long callback_count;
} php_lua_object;

static const luaL_Reg php_lualibs[] = {
  {"base", luaopen_base},
  {LUA_LOADLIBNAME, luaopen_package},
  {LUA_TABLIBNAME, luaopen_table},
  {LUA_IOLIBNAME, luaopen_io},
  {LUA_OSLIBNAME, luaopen_os},
  {LUA_STRLIBNAME, luaopen_string},
  {LUA_MATHLIBNAME, luaopen_math},
  {LUA_DBLIBNAME, luaopen_debug},
  {NULL, NULL}
};

static void php_lua_push_zval(lua_State *L, zval *val_p TSRMLS_DC);
static void php_lua_write_real_property(lua_State *L,int index,zval *prop, zval *value TSRMLS_DC);

static void *php_lua_alloc (void *ud, void *ptr, size_t osize, size_t nsize) { /* {{{ */
  if (nsize == 0) {
    if (ptr) {
      efree(ptr);
    }
    return NULL;
  } else {
    if (ptr) {
      return erealloc(ptr, nsize);
    } else {
      return emalloc(nsize);
    }
  }
} /* }}} */


static void php_lua_push_array(lua_State *L, zval *array TSRMLS_DC) /* {{{ */
{
  zval **data;
  HashTable *arr_hash;
  HashPosition pointer;
  int array_count;
  char *key;
  int key_len;
  long index;
  int hash_key;
  char *key_copy;

  zval *zkey;

  arr_hash = Z_ARRVAL_P(array);
  array_count = zend_hash_num_elements(arr_hash);

  lua_newtable(L);

  for(zend_hash_internal_pointer_reset_ex(arr_hash, &pointer); zend_hash_get_current_data_ex(arr_hash, (void**) &data, &pointer) == SUCCESS; 	       zend_hash_move_forward_ex(arr_hash, &pointer))
  {
    hash_key=zend_hash_get_current_key_ex(arr_hash, &key, &key_len, &index, 0, &pointer);

    if (hash_key == HASH_KEY_IS_LONG && index == 0) {
      php_error_docref(NULL TSRMLS_CC, E_WARNING, "Trying to push array index %ld to lua which is unsupported in lua. Element has been discarded",index);
    }
    else {
      ALLOC_INIT_ZVAL(zkey);
      if (hash_key == HASH_KEY_IS_STRING) {
        key_copy=estrndup(key,key_len-1);
        Z_TYPE_P(zkey)=IS_STRING;
        Z_STRVAL_P(zkey)=key_copy;
        Z_STRLEN_P(zkey)=key_len-1;
      }
      else {
        Z_TYPE_P(zkey)=IS_LONG;
        Z_LVAL_P(zkey)=index;
      }

      php_lua_write_real_property(L,-3,zkey,*data TSRMLS_CC);
      zval_ptr_dtor(&zkey);
    }
  }
} /* }}} */




static void php_lua_push_zval(lua_State *L, zval *val_p TSRMLS_DC) /* {{{ */
{
  /* TODO: Use proper type for lua and separate only when needed */

  /* push into lua stack properly */
  switch (Z_TYPE_P(val_p)) {
    case IS_STRING:
      lua_pushlstring(L, Z_STRVAL_P(val_p), Z_STRLEN_P(val_p));
      break;

    case IS_NULL:	   
      lua_pushnil(L);	   
      break;

    case IS_DOUBLE:
      lua_pushnumber(L, Z_DVAL_P(val_p));
      break;

    case IS_LONG:
      lua_pushnumber(L, Z_LVAL_P(val_p));
      break;

    case IS_BOOL:
      lua_pushboolean(L, Z_BVAL_P(val_p));
      break;

    case IS_ARRAY:
      php_lua_push_array(L,val_p TSRMLS_CC);
      break;

    default:
      php_error_docref(NULL TSRMLS_CC, E_WARNING, "invalid type `%s' supported in lua", zend_zval_type_name(val_p));
      lua_pushnil(L);
      break;
  }

} /* }}} */

static int php_lua_push_apply_func(void *pDest, void *argument TSRMLS_DC) /* {{{ */
{
  php_lua_push_zval((lua_State*)argument, *(zval**)pDest TSRMLS_CC);
  return ZEND_HASH_APPLY_KEEP;
} /* }}} */

static void php_lua_get_zval_from_stack(lua_State *L, int index, zval *ret TSRMLS_DC) /* {{{ */
{
  const char *value;
  size_t value_len;

  switch (lua_type(L, index)) {
    case LUA_TBOOLEAN:
      ZVAL_BOOL(ret, lua_toboolean(L, index));
      break;

    case LUA_TNUMBER:
      ZVAL_DOUBLE(ret, lua_tonumber(L, index));
      break;

    case LUA_TSTRING:
      value = lua_tolstring(L, index, &value_len);
      ZVAL_STRINGL(ret, (char*)value, value_len, 1);
      break;

    case LUA_TTABLE:
      array_init(ret);
      /* notify lua to traverse the table */
      lua_pushnil(L);

      zval *akey,*aval;

      /* table has been moved by one because of the pushnil */
      /* this will ONLY work with negative indices! */
      while (lua_next(L, index-1) != 0)
      {
        ALLOC_INIT_ZVAL(akey);
        ALLOC_INIT_ZVAL(aval);

        /* `key' is at index -2 and `value' at index -1 */
        php_lua_get_zval_from_stack(L,-2,akey);
        php_lua_get_zval_from_stack(L,-1,aval);

        switch(Z_TYPE_P(akey))
        {
          /* lua can't use (at least when i tried ;) ) floats as array keys so that should be safe */
          case IS_DOUBLE:
            add_index_zval(ret,(long)Z_DVAL_P(akey),aval);
            break;
          default:
            add_assoc_zval(ret,Z_STRVAL_P(akey),aval);
            break;
        }
        lua_pop(L, 1);  /* removes `value'; keeps `key' for next iteration */
        zval_ptr_dtor(&akey);
      }
      break;


    case LUA_TFUNCTION:
      /* TODO: Make this a PHP Function */      

    default:
      php_error_docref(NULL TSRMLS_CC, E_WARNING, "invalid type `%i' passed from lua.", lua_type(L, index));
      /* fall-through */

    case LUA_TNIL:
      ZVAL_NULL(ret);
      break;
  }

} /* }}} */

static int php_lua_print(lua_State *L) /* {{{ */
{
  int i;
  int n = lua_gettop(L); 

  for (i = 1; i <= n; i++) {
    php_printf("%s", lua_tostring(L, i));
  }

  return 0;
} /* }}} */

static void php_lua_object_dtor(void *object, zend_object_handle handle TSRMLS_DC) /* {{{ */
{
  php_lua_object *intern = (php_lua_object*)object;
  zend_object_std_dtor(&(intern->std) TSRMLS_CC);

  long i;
  for (i=0;i<intern->callback_count;i++)
    zval_dtor(&intern->callbacks[i]);
  efree(intern->callbacks);
  if (intern->L) {
    lua_close(intern->L);
  }

  efree(intern);
} /* }}} */

static zval *php_lua_read_property(zval *obj, zval *prop, int type TSRMLS_DC) /* {{{ */
{
  zval *retval;
  lua_State *L = getLuaZ(obj);

  LUA_STACK_START(L);

  if (type != BP_VAR_R) {
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "No write access using read_property.");
    ALLOC_INIT_ZVAL(retval);
    return retval;
  }

  if (Z_TYPE_P(prop) == IS_STRING) {
    MAKE_STD_ZVAL(retval);
    lua_getfield(L, LUA_GLOBALSINDEX, Z_STRVAL_P(prop) TSRMLS_CC);
    php_lua_get_zval_from_stack(L, -1, retval TSRMLS_CC);
    Z_DELREF_P(retval);
  } else {
    ALLOC_INIT_ZVAL(retval);
  }

  lua_pop(L, 1);

  LUA_STACK_END(L);

  return retval;
} /* }}} */


static void php_lua_write_real_property(lua_State *L,int index,zval *prop, zval *value TSRMLS_DC) /* {{{ */
{
  LUA_STACK_START(L);
  php_lua_push_zval(L, prop TSRMLS_CC);
  php_lua_push_zval(L, value TSRMLS_CC);

  lua_settable(L, index);
  LUA_STACK_END(L);
} /* }}} */



static void php_lua_write_property(zval *obj, zval *prop, zval *value TSRMLS_DC) /* {{{ */
{
  /* TODO: Use proper type for lua and separate only when needed */
  lua_State *L = getLuaZ(obj);

  php_lua_write_real_property(L,LUA_GLOBALSINDEX,prop,value TSRMLS_CC);
} /* }}} */

static int php_lua_atpanic(lua_State *L) { /* {{{ */
  TSRMLS_FETCH();
  php_error_docref(NULL TSRMLS_CC, E_WARNING, "lua panic (%s)", lua_tostring(L, 1));
  lua_pop(L, 1);
  zend_bailout();
  return 0;
} /* }}} */

static zend_object_value php_lua_create_object(zend_class_entry *ce TSRMLS_DC) /* {{{ */
{
  zval tmp;
  zend_object_value retval;
  php_lua_object *intern;
  lua_State *L;

  L = lua_newstate(php_lua_alloc, NULL);
  lua_atpanic(L, php_lua_atpanic);

  intern = ecalloc(1, sizeof(php_lua_object));
  intern->L = L;
  intern->callback_count=0;
  intern->callbacks=emalloc(sizeof(zval*));
  zend_object_std_init(&(intern->std), ce TSRMLS_CC);
  zend_hash_copy(intern->std.properties,
      &ce->default_properties, (copy_ctor_func_t) zval_add_ref,
      (void *) &tmp, sizeof(zval *));

  retval.handle = zend_objects_store_put(intern, php_lua_object_dtor, NULL, NULL TSRMLS_CC);
  retval.handlers = &lua_handlers;
  return retval;
} /* }}} */

static int php_lua_callback(lua_State *L) /* {{{ */
{
  php_lua_object *object=(php_lua_object*)lua_topointer(L, lua_upvalueindex(1));
  long selected_callback_index=(long)lua_tonumber(L, lua_upvalueindex(2));
  zval *return_value;

  ALLOC_INIT_ZVAL(return_value);

  if (!zend_is_callable(&object->callbacks[selected_callback_index],0,NULL))
    return;

  zval **params;
  int n = lua_gettop(L);    /* number of arguments */

  params=emalloc(n*sizeof(zval));

  int i;
  for (i = 1; i <= n; i++) {
    ALLOC_INIT_ZVAL(params[i-1]);
    /* php_lua_get_zval_from_stack won't work with positive indices */
    php_lua_get_zval_from_stack(L,-n-1+i,params[i-1] TSRMLS_CC);
  }

  /* XXX no check - do we need one? :S */
  /* johannes said i should use zend_call_method but this only allows up to 2 parameters?! */
  call_user_function(EG(function_table),NULL,&object->callbacks[selected_callback_index],return_value,n,params TSRMLS_CC);

  /* hmm...what if the result is NULL? should php return a return value (NULL) then or just return 0? :S */
  php_lua_push_zval(L,return_value TSRMLS_CC);

  for (i = 0; i < n; i++) {
    zval_ptr_dtor(&params[i]);
  }
  efree(params);

  zval_ptr_dtor(&return_value);

  /* PHP doesn't support multiple return values so this will always be 1 */
  return 1;
} /* }}} */

static void php_lua_call_table_with_arguments(lua_State *L,int level,int table_index,char *function,int propagate_self,zval *args,zval *return_value) /* {{{ */
{
  lua_getfield(L,table_index,function);
  if (lua_type(L,lua_gettop(L)) != LUA_TFUNCTION) {
    lua_pop(L, lua_gettop(L) - level);
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "invalid lua callback. '%s' is not a defined function",function);
    return;
  }

  /* push the table on the stack as the first argument */
  if (propagate_self)
    lua_pushvalue(L,table_index + lua_gettop(L));

  zend_hash_apply_with_argument(Z_ARRVAL_P(args), php_lua_push_apply_func, (void *)L  TSRMLS_CC);

  if (lua_pcall(L, zend_hash_num_elements(Z_ARRVAL_P(args)) + propagate_self, LUA_MULTRET, 0) != 0) { 
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "error running lua function `%s': %s", function, lua_tostring(L, -1));
    lua_pop(L, lua_gettop(L) - level);
    return;
  }

  /* always return an array. otherwise we couldn't distinguish between a table return or a multi return */
  array_init(return_value);
  int retcount = lua_gettop(L) - level;
  int i;
  zval *val;

  for (i = -retcount; i < 0; i++)
  {
    MAKE_STD_ZVAL(val);
    php_lua_get_zval_from_stack(L, i, val TSRMLS_CC);
    add_next_index_zval(return_value, val);
  }
  lua_pop(L, retcount);

  return;
} /* }}} */

static void php_lua_call_table_function(INTERNAL_FUNCTION_PARAMETERS,int propagate_self) /* {{{ */
{
  zval *callback,*args;
  zval **lua_table,**lua_function;
  int array_count,level;
  lua_State *L;

  L=getLua();
  LUA_STACK_START(L);

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "za", &callback, &args) == FAILURE) {
    return;
  }

  array_count=zend_hash_num_elements(Z_ARRVAL_P(callback));
  if (array_count!=2) {
    LUA_STACK_END(L);
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "invalid callback supplied. must contain exactly 2 elements");
    return;
  }
  if (zend_hash_index_find(Z_ARRVAL_P(callback), 0, (void**)&lua_table) == FAILURE) {
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "callback index 0 is empty");
    return;
  }
  if (zend_hash_index_find(Z_ARRVAL_P(callback), 1, (void**)&lua_function) == FAILURE) {
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "callback index 1 is empty");
    return;
  }
  lua_getglobal(L,Z_STRVAL_PP(lua_table));
  if (lua_type(L,lua_gettop(L)) != LUA_TTABLE) {
    lua_pop(L, -1);
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "trying to call an invalid table '%s'",Z_STRVAL_PP(lua_table));
    return;
  }
  level=lua_gettop(L);

  php_lua_call_table_with_arguments(L,level,-1,Z_STRVAL_PP(lua_function),propagate_self,args,return_value);

  /* remove the table which is still on top */
  lua_pop(L,-1);

  LUA_STACK_END(L);
} /* }}} */

/* {{{ */
static int php_lua_binary_zval_writer(lua_State* L, const void* p,size_t size, void* u)
{
  Z_STRVAL_P((zval*)u)=erealloc(Z_STRVAL_P((zval*)u),(Z_STRLEN_P((zval*)u)+1+size)*sizeof(char));
  memcpy(&Z_STRVAL_P((zval*)u)[Z_STRLEN_P((zval*)u)],p,size);
  Z_STRLEN_P((zval*)u)=Z_STRLEN_P((zval*)u)+size;
  Z_STRVAL_P((zval*)u)[Z_STRLEN_P((zval*)u)]='\0';
  return 0;
}
/* }}} */

#define RETURN_STR "return %s"


/* {{{ lua::__construct()
   Create new LUA instance */ 
PHP_METHOD(lua, __construct)
{
  lua_State *L = getLua();
  // mop: open standard libs if desired
  if (lua_globals.load_standard_libs)
    luaL_openlibs(L);

  lua_register(L, "print", php_lua_print);
}
/* }}} */

/* {{{ lua::__call(string method, array args [, int nresults])
   Call a lua function from within PHP */
PHP_METHOD(lua, __call)
{
  int level, retcount;
  char *function, *func;
  int function_len;
  zval *args;
  long nresults = LUA_MULTRET;
  lua_State *L = getLua();

  LUA_STACK_START(L);

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sa|l", &function, &function_len, &args, &nresults) == FAILURE) {
    return;
  }

  level = lua_gettop(L);

  spprintf(&func, sizeof(RETURN_STR)-2-1+function_len, RETURN_STR, function);
  if (luaL_dostring(L, func)) {
    efree(func);
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "error looking up lua function `%s': %s", function, lua_tostring(L, -1));
    lua_pop(L, lua_gettop(L) - level);
    LUA_STACK_END(L);
    return;
  }
  efree(func);

  zend_hash_apply_with_argument(Z_ARRVAL_P(args), php_lua_push_apply_func, (void *)L  TSRMLS_CC);

  if (lua_pcall(L, zend_hash_num_elements(Z_ARRVAL_P(args)), nresults, 0) != 0) { 
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "error running lua function `%s': %s", function, lua_tostring(L, -1));
    lua_pop(L, lua_gettop(L) - level);
    LUA_STACK_END(L);
    return;
  }

  retcount = lua_gettop(L) - level;

  if (retcount == 0) {
    /* No vlaue returned -> return null */
  } else {
    /* multiple return values -> build an array */
    int i;
    zval *val;

    array_init(return_value);
    for (i = -retcount; i < 0; i++)
    {
      MAKE_STD_ZVAL(val);
      php_lua_get_zval_from_stack(L, i, val TSRMLS_CC);
      add_next_index_zval(return_value, val);
    }
    lua_pop(L, retcount);
  }

  LUA_STACK_END(L);
  return;
}

/* {{{ lua::call_function(string function, array args)
   Call a lua function from within PHP */
PHP_METHOD(lua, call_function)
{
  char *function;
  int function_len;
  zval *args;
  lua_State *L;
  int level;

  L=getLua();
  LUA_STACK_START(L);

  level=lua_gettop(L);

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sa", &function, &function_len, &args) == FAILURE) {
    return;
  }

  php_lua_call_table_with_arguments(L,level,LUA_GLOBALSINDEX,function,0,args,return_value);

  LUA_STACK_END(L);
}

/* {{{ lua::call_table(array lua_callback, array args)
   Call a lua table from within PHP (lua_table.lua_function()) */
PHP_METHOD(lua, call_table)
{
  php_lua_call_table_function(INTERNAL_FUNCTION_PARAM_PASSTHRU,0);
}

/* {{{ lua::call_table_self(array lua_callback, array args)
   Call a lua table from within PHP and propagate self (lua_table:lua_function()) */
PHP_METHOD(lua, call_table_self)
{
  php_lua_call_table_function(INTERNAL_FUNCTION_PARAM_PASSTHRU,1);
}

/* {{{ void lua::evaluate(string code)
   Evaluates code with lua */
PHP_METHOD(lua, evaluate)
{
  int error;
  int code_len;
  char *code;
  lua_State *L = getLua();

  LUA_STACK_START(L);

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &code, &code_len) == FAILURE) {
    return;
  }

  error = luaL_loadbuffer(L, code, code_len, "line") || lua_pcall(L, 0, LUA_MULTRET, 0);
  if (error) {
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "lua error: %s", lua_tostring(L, -1));
    lua_pop(L, 1);  /* pop error message from the stack */
  }

  /* always return an array. otherwise we couldn't distinguish between a table return or a multi return */
  array_init(return_value);
  int retcount = lua_gettop(L);
  int i;
  zval *val;

  for (i = -retcount; i < 0; i++)
  {
    MAKE_STD_ZVAL(val);
    php_lua_get_zval_from_stack(L, i, val TSRMLS_CC);
    add_next_index_zval(return_value, val);
  }
  lua_pop(L, retcount);

  LUA_STACK_END(L);
}
/* }}} */

/* {{{ void lua::evaluatefile(string file)
   Evaluates a lua script */
PHP_METHOD(lua, evaluatefile) 
{
  int error;
  int file_len;
  char *file;
  lua_State *L = getLua();

  LUA_STACK_START(L);

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &file, &file_len) == FAILURE) {
    return;	 
  }

  if (php_check_open_basedir(file TSRMLS_CC) || (PG(safe_mode) && !php_checkuid(file, "rb+", CHECKUID_CHECK_MODE_PARAM))) {
    LUA_STACK_END(L);
    RETURN_FALSE;
  }

  error = luaL_dofile(L, file);

  if (error) {
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "lua error: %s", lua_tostring(L, -1)); 
    lua_pop(L, 1);
  } 
  
  array_init(return_value);
  int retcount = lua_gettop(L);
  int i;
  zval *val;

  for (i = -retcount; i < 0; i++)
  {
    MAKE_STD_ZVAL(val);
    php_lua_get_zval_from_stack(L, i, val TSRMLS_CC);
    add_next_index_zval(return_value, val);
  }
  lua_pop(L, retcount);

  LUA_STACK_END(L);
}
/* }}} */

/* {{{ string lua::getVersion() 
   Return Lua's version      */
PHP_METHOD(lua, getversion) 
{
  RETURN_STRING(LUA_RELEASE, 1);
}
/* }}} */

/* {{{ */
PHP_METHOD(lua,expose_function)
{
  zval *callback;
  char *lua_name;
  int len;
  
  php_lua_object *object=getLuaY();
  lua_State *L = object->L;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"sz",&lua_name,&len,&callback) == FAILURE)
    return;

  if (zend_is_callable(callback,0,NULL)) {
    lua_pushlightuserdata(L,object);
    lua_pushnumber(L,object->callback_count);
    lua_pushcclosure(L, php_lua_callback,2);
    lua_setglobal(L, lua_name);
  }
  /* hmm...out of memory check? */
  object->callbacks=erealloc(object->callbacks,sizeof(zval)*(object->callback_count+1));
  object->callbacks[object->callback_count] = *callback;
  zval_copy_ctor(&object->callbacks[object->callback_count]);
  object->callback_count++;
}
/* }}} */

/* {{{ */
PHP_METHOD(lua,compile)
{
  char *chunk;
  int len;
  int error;

  lua_State *L = lua_newstate(php_lua_alloc, NULL);
  lua_atpanic(L, php_lua_atpanic);

  LUA_STACK_START(L);

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&chunk,&len) == FAILURE)
  {
    lua_close(L);
    return;
  }

  error = luaL_loadbuffer(L, chunk, len, "line");
  if (error) {
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "lua error: %s", lua_tostring(L, -1));
    lua_pop(L, 1);  /* pop error message from the stack */
    lua_close(L);
    return;
  }
  
  RETVAL_STRING("",1);

  error=lua_dump(L,php_lua_binary_zval_writer,return_value);
  lua_pop(L, 1);  /* clean stack */
  LUA_STACK_END(L);
  lua_close(L);
  if (error) {
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "Couldn't dump code");
    RETURN_NULL();
  }
}

/* {{{ lua_functions[] */
zend_function_entry lua_functions[] = {
  {NULL, NULL, NULL}
};
/* }}} */

/* {{{ ARG_INFO */

ZEND_BEGIN_ARG_INFO_EX(arginfo_lua_call, 0, 0, 2)
	ZEND_ARG_INFO(0, method)
	ZEND_ARG_INFO(0, args)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_lua_call_table, 0, 0, 2)
  ZEND_ARG_INFO(0, callback)
  ZEND_ARG_INFO(0, args)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_lua_call_function, 0, 0, 2)
  ZEND_ARG_INFO(0, function)
  ZEND_ARG_INFO(0, args)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_lua_evaluate, 0, 0, 1)
  ZEND_ARG_INFO(0, code)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_lua_evaluatefile, 0, 0, 1)
  ZEND_ARG_INFO(0, file)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_lua_expose_function, 0, 0, 1)
  ZEND_ARG_INFO(0, callback)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_lua_compile, 0, 0, 1)
  ZEND_ARG_INFO(0, chunk)
ZEND_END_ARG_INFO()

/* }}} */

/* {{{ lua_class_functions[] */
zend_function_entry lua_class_functions[] = {
  PHP_ME(lua, __construct,                NULL,                                   ZEND_ACC_PUBLIC)
  PHP_ME(lua, __call,                     arginfo_lua_call,                       ZEND_ACC_PUBLIC)
  PHP_ME(lua, call_function,              arginfo_lua_call_function,              ZEND_ACC_PUBLIC)
  PHP_ME(lua, call_table,                 arginfo_lua_call_table,                 ZEND_ACC_PUBLIC)
  PHP_ME(lua, call_table_self,            arginfo_lua_call_table,                 ZEND_ACC_PUBLIC)
  PHP_ME(lua, evaluate,                   arginfo_lua_evaluate,                   ZEND_ACC_PUBLIC)
  PHP_ME(lua, evaluatefile,               arginfo_lua_evaluatefile,               ZEND_ACC_PUBLIC)
  PHP_ME(lua, getversion,                 NULL,                                   ZEND_ACC_PUBLIC)
  PHP_ME(lua, expose_function,            arginfo_lua_expose_function,            ZEND_ACC_PUBLIC)
  PHP_ME(lua, compile,                    arginfo_lua_compile,                    ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
  {NULL, NULL, NULL}
};
/* }}} */

#ifdef COMPILE_DL_LUA
ZEND_GET_MODULE(lua)
#endif

  /* {{{ PHP_INI
  */
PHP_INI_BEGIN()
  STD_PHP_INI_BOOLEAN("lua.load_standard_libs","1", PHP_INI_ALL, OnUpdateBool, load_standard_libs, zend_lua_globals, lua_globals)
PHP_INI_END()
  /* }}} */

  /* {{{ php_lua_init_globals
  */
static void php_lua_init_globals(zend_lua_globals *lua_globals)
{
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
*/
PHP_MINIT_FUNCTION(lua)
{
  REGISTER_INI_ENTRIES();

  zend_class_entry ce;
  INIT_CLASS_ENTRY(ce, "lua", lua_class_functions);
  lua_ce = zend_register_internal_class(&ce TSRMLS_CC);
  lua_ce->create_object = php_lua_create_object;
  memcpy(&lua_handlers, zend_get_std_object_handlers(),
      sizeof(zend_object_handlers));
  lua_handlers.write_property = php_lua_write_property;
  lua_handlers.read_property  = php_lua_read_property;
  lua_ce->ce_flags |= ZEND_ACC_FINAL;

  zend_declare_class_constant_long(lua_ce, "MULTRET", sizeof("MULTRET")-1, LUA_MULTRET TSRMLS_CC);

  ZEND_INIT_MODULE_GLOBALS(lua, php_lua_init_globals, NULL);

  return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
*/
PHP_MSHUTDOWN_FUNCTION(lua)
{
  UNREGISTER_INI_ENTRIES();
  return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION
*/
PHP_RINIT_FUNCTION(lua)
{
  return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION
*/
PHP_RSHUTDOWN_FUNCTION(lua)
{
  return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
*/
PHP_MINFO_FUNCTION(lua)
{
  php_info_print_table_start();
  php_info_print_table_row(2, "lua support", "enabled");
  php_info_print_table_row(2, "lua extension version", EXT_LUA_VERSION);
  php_info_print_table_row(2, "lua release", LUA_RELEASE);
  php_info_print_table_row(2, "lua copyright", LUA_COPYRIGHT);
  php_info_print_table_row(2, "lua authors", LUA_AUTHORS);
  php_info_print_table_end();

  DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ lua_module_entry
*/
zend_module_entry lua_module_entry = {
  STANDARD_MODULE_HEADER,
  "lua",
  lua_functions,
  PHP_MINIT(lua),
  PHP_MSHUTDOWN(lua),
  PHP_RINIT(lua),
  PHP_RSHUTDOWN(lua),
  PHP_MINFO(lua),
  EXT_LUA_VERSION,
  STANDARD_MODULE_PROPERTIES
};
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * vim: set noet sw=4 ts=4:
 * vim600: noet sw=4 ts=4 fdm=marker:
 * End:
 */
