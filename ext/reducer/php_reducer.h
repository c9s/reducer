#ifndef PHP_REDUCER_H
#define PHP_REDUCER_H

#include "SAPI.h"
#include "zend_API.h"
#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"
#include "ext/standard/html.h"
#include "php_variables.h"

extern zend_module_entry reducer_module_entry;

#define phpext_reducer_ptr &filter_module_entry

#ifdef ZTS
#include "TSRM.h"
#endif


// XXX: replace the version
#define PHP_REDUCER_VERSION PHP_VERSION

#define REDUCER_COUNT             2               
#define REDUCER_FIRST             3
#define REDUCER_LAST              4
#define REDUCER_AVG               5
#define REDUCER_SUM               6

// #define REDUCER_ASC 2

PHP_MINIT_FUNCTION(reducer);
PHP_MSHUTDOWN_FUNCTION(reducer);
PHP_RINIT_FUNCTION(reducer);
PHP_RSHUTDOWN_FUNCTION(reducer);
PHP_MINFO_FUNCTION(reducer);

PHP_FUNCTION(group_by);
PHP_FUNCTION(fold_rows);

ZEND_BEGIN_MODULE_GLOBALS(reducer)

ZEND_END_MODULE_GLOBALS(reducer)

#if defined(COMPILE_DL_REDUCER) && defined(ZTS)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

// #define IF_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(reducer, v)


#endif
