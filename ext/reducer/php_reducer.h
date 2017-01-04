#ifndef PHP_REDUCER_H
#define PHP_REDUCER_H

#include "SAPI.h"
#include "zend_API.h"
#include "php.h"
#include "php_ini.h"
#include "zend_types.h"
#include "php_variables.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"
#include "ext/standard/html.h"

extern zend_module_entry reducer_module_entry;

#define GROUP_INIT_SIZE 16

#define phpext_reducer_ptr &reducer_module_entry

#ifdef ZTS
#include "TSRM.h"
#endif

#define PHP_REDUCER_VERSION "0.9.3"

#define REDUCER_AGGR_COUNT             2               
#define REDUCER_AGGR_FIRST             3
#define REDUCER_AGGR_LAST              4
#define REDUCER_AGGR_AVG               5
#define REDUCER_AGGR_SUM               6
#define REDUCER_AGGR_MIN               7
#define REDUCER_AGGR_MAX               8
#define REDUCER_AGGR_GROUP             9

PHP_MINIT_FUNCTION(reducer);
PHP_MSHUTDOWN_FUNCTION(reducer);
PHP_RINIT_FUNCTION(reducer);
PHP_RSHUTDOWN_FUNCTION(reducer);
PHP_MINFO_FUNCTION(reducer);

PHP_FUNCTION(group_by);
PHP_FUNCTION(aggregate);

ZEND_BEGIN_MODULE_GLOBALS(reducer)

ZEND_END_MODULE_GLOBALS(reducer)

#if defined(COMPILE_DL_REDUCER) && defined(ZTS)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

// #define IF_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(reducer, v)

typedef struct _aggregator {

  zend_string *alias;
  zend_string *selector;
  ulong num_alias;
  ulong num_selector;

  zend_bool is_callable;
  zval *type; // could be a constant or a function call.
  zend_fcall_info fci;
  zend_fcall_info_cache fci_cache;

} compiled_agt;


#endif
