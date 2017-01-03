#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php_reducer.h"
#include "ext/standard/php_var.h"

ZEND_DECLARE_MODULE_GLOBALS(reducer)

ZEND_BEGIN_ARG_INFO_EX(arginfo_group_by, 0, 0, 1)
  ZEND_ARG_INFO(0, array)
  ZEND_ARG_INFO(0, fields)
  ZEND_ARG_INFO(0, aggregators)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fold_rows, 0, 0, 1)
  ZEND_ARG_INFO(0, array)
  ZEND_ARG_INFO(0, fields)
  ZEND_ARG_INFO(0, aggregators)
ZEND_END_ARG_INFO()

static const zend_function_entry reducer_functions[] = {
  PHP_FE(group_by,  arginfo_group_by)
  PHP_FE(fold_rows,  arginfo_fold_rows)
  PHP_FE_END
};

zend_module_entry reducer_module_entry = {
	STANDARD_MODULE_HEADER,
	"reducer",
	reducer_functions,
	PHP_MINIT(reducer),
	PHP_MSHUTDOWN(reducer),
	NULL,
	PHP_RSHUTDOWN(reducer),
	PHP_MINFO(reducer),
	PHP_REDUCER_VERSION,
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_REDUCER
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(reducer)
#endif

#define REDUCER_HASH_ADD_NEW(ht, num_idx, str_idx, val) \
      (str_idx) \
      ? zend_hash_add_new(ht, str_idx, val) \
      : zend_hash_index_add_new(ht, num_idx, val)

#define REDUCER_HASH_FIND(ht, num_idx, str_idx) \
      (str_idx) \
      ? zend_hash_find(ht, str_idx) \
      : zend_hash_index_find(ht, num_idx)

PHP_INI_BEGIN()
    // STD_PHP_INI_ENTRY("reducer.default",   "unsafe_raw", PHP_INI_SYSTEM|PHP_INI_PERDIR, UpdateDefaultFilter, default_filter, zend_filter_globals, filter_globals)
    // PHP_INI_ENTRY("reducer.default_flags", NULL,     PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateFlags)
PHP_INI_END()

PHP_MINIT_FUNCTION(reducer)
{
    // ZEND_INIT_MODULE_GLOBALS(reducer, php_reducer_init_globals, NULL);
    REGISTER_INI_ENTRIES();
    REGISTER_LONG_CONSTANT("REDUCER_SUM"   , REDUCER_SUM   , CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("REDUCER_AVG"   , REDUCER_AVG   , CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("REDUCER_COUNT" , REDUCER_COUNT , CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("REDUCER_FIRST" , REDUCER_FIRST , CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("REDUCER_LAST"  , REDUCER_LAST , CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("REDUCER_MIN"  , REDUCER_MIN , CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("REDUCER_MAX"  , REDUCER_MAX , CONST_CS | CONST_PERSISTENT);
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(reducer)
{
    UNREGISTER_INI_ENTRIES();
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(reducer)
{
    return SUCCESS;
}

PHP_MINFO_FUNCTION(reducer)
{
    php_info_print_table_start();
    php_info_print_table_row(2, "Version", PHP_REDUCER_VERSION);
    // php_info_print_table_row( 2, "Revision", "$Id: 9cc4bdc40f4013d538d685f912be2e9698c649cc $");
    php_info_print_table_end();
    DISPLAY_INI_ENTRIES();
}


zval fold_rows(zval* rows, zval* fields, zval* aggregators)
{
  zval result, *row, *result_val, *current, *first, *field, *tmp;

  array_init(&result);

  HashTable *ht = Z_ARRVAL_P(rows);
  HashTable *result_ht = Z_ARRVAL(result);
  zend_long cnt = zend_array_count(ht);

  zend_hash_internal_pointer_reset(ht);

  if ((first = zend_hash_get_current_data(ht)) != NULL) {
      ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(fields), field) {
          if ((tmp = zend_hash_find(Z_ARRVAL_P(first), Z_STR_P(field))) != NULL) {
              result_val = zend_hash_add_new(result_ht, Z_STR_P(field), tmp); 
          }
      } ZEND_HASH_FOREACH_END();
  }
  
  zval *aggregator, *agg_type, compiled_aggregators;
  ulong num_key, num_selector;
  zend_string *selector, *alias;

  array_init(&compiled_aggregators);

  ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(aggregators), num_key, alias, aggregator) {

      zval compiled;
      array_init(&compiled);

      if (Z_TYPE_P(aggregator) == IS_LONG) {

          zend_hash_index_add_new(Z_ARRVAL(compiled), 0, aggregator);

          zval zval_alias;
          if (alias) {
              ZVAL_STR(&zval_alias, alias);
              zend_hash_index_add_new(Z_ARRVAL(compiled), 1, &zval_alias); // selector
          } else {
              ZVAL_LONG(&zval_alias, num_key);
              zend_hash_index_add_new(Z_ARRVAL(compiled), 1, &zval_alias); // selector
          }
      } 
      else if (zend_is_callable(aggregator, IS_CALLABLE_CHECK_NO_ACCESS, NULL)) {

          if (Z_REFCOUNTED_P(aggregator)) {
              Z_ADDREF_P(aggregator);
          }
          zend_hash_index_add_new(Z_ARRVAL(compiled), 0, aggregator);

          zval zval_alias;
          if (alias) {
              ZVAL_STR(&zval_alias, alias);
              zend_hash_index_add_new(Z_ARRVAL(compiled), 1, &zval_alias); // selector
          } else {
              ZVAL_LONG(&zval_alias, num_key);
              zend_hash_index_add_new(Z_ARRVAL(compiled), 1, &zval_alias); // selector
          }

      } else if (Z_TYPE_P(aggregator) == IS_ARRAY) {

          tmp = zend_hash_str_find(Z_ARRVAL_P(aggregator), "aggregator", sizeof("aggregator") - 1);
          if (tmp == NULL) {
              php_error_docref(NULL, E_USER_ERROR, "Aggregator is not defined.");
          }
          ZVAL_DEREF(tmp);
          if (Z_REFCOUNTED_P(tmp)) {
              Z_ADDREF_P(tmp);
          }
          tmp = zend_hash_index_add_new(Z_ARRVAL(compiled), 0, tmp);

          tmp = zend_hash_str_find(Z_ARRVAL_P(aggregator), "selector", sizeof("selector") -1);
          if (tmp) { 
              ZVAL_DEREF(tmp);
              zend_hash_index_add_new(Z_ARRVAL(compiled), 1, tmp);
          } else {
              zval zval_alias;
              if (alias) {
                  ZVAL_STR(&zval_alias, alias);
                  zend_hash_index_add_new(Z_ARRVAL(compiled), 1, &zval_alias); // selector
              } else {
                  ZVAL_LONG(&zval_alias, num_key);
                  zend_hash_index_add_new(Z_ARRVAL(compiled), 1, &zval_alias); // selector
              }
          }
      } else {
          zval_dtor(&compiled);
          php_error_docref(NULL, E_USER_ERROR, "Unsupported aggregator");
      }

      REDUCER_HASH_ADD_NEW(Z_ARRVAL(compiled_aggregators), num_key, alias, &compiled);

  } ZEND_HASH_FOREACH_END();

  ZEND_HASH_FOREACH_VAL(ht, row) {
    if (Z_TYPE_P(row) != IS_ARRAY) {
        php_error_docref(NULL, E_USER_ERROR, "input row is not an array.");
    }
    ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(compiled_aggregators), num_key, alias, aggregator) {

        // alias might be null
        agg_type = zend_hash_index_find(Z_ARRVAL_P(aggregator), 0);

        zend_bool is_callable = zend_is_callable(agg_type, IS_CALLABLE_CHECK_NO_ACCESS, NULL);

        tmp = zend_hash_index_find(Z_ARRVAL_P(aggregator), 1);

        if (Z_TYPE_P(tmp) == IS_STRING) {
            selector = Z_STR_P(tmp);
            current = zend_hash_find(HASH_OF(row), selector);
        } else if (Z_TYPE_P(tmp) == IS_LONG) {
            num_selector = Z_LVAL_P(tmp);
            selector = NULL;
            current = zend_hash_index_find(HASH_OF(row), num_selector);
        } else {
            php_error_docref(NULL, E_USER_ERROR, "Unsupported selector.");
        }

        // get the carried value, and then use aggregator to reduce the values.
        result_val = REDUCER_HASH_FIND(result_ht, num_key, alias);


        if (is_callable) {

            zend_fcall_info fci = empty_fcall_info;
            zend_fcall_info_cache fci_cache = empty_fcall_info_cache;
            char *errstr = NULL;
            if (zend_fcall_info_init(agg_type, 0, &fci, &fci_cache, NULL, &errstr) == FAILURE) {
                php_error_docref(NULL, E_USER_ERROR, "Error setting converter callback: %s", errstr);
            }
            if (errstr) {
                efree(errstr);
            }

            zval retval;
            zval args[2];

            if (result_val == NULL) {
                zval tmp;
                ZVAL_LONG(&tmp, 0);
                result_val = REDUCER_HASH_ADD_NEW(result_ht, num_key, alias, &tmp);
            }

            ZVAL_COPY(&args[0], result_val);
            ZVAL_COPY(&args[1], current);

            fci.retval = &retval;
            fci.param_count = 2;
            fci.params = args;
            fci.no_separation = 0;
            zend_call_function(&fci, &fci_cache); // == SUCCESS;

            ZVAL_COPY(result_val, &retval);
            zval_ptr_dtor(&retval);

            zval_ptr_dtor(&args[0]);
            zval_ptr_dtor(&args[1]);

        } else if (Z_TYPE_P(agg_type) == IS_LONG) {
            switch (Z_LVAL_P(agg_type)) {
              case REDUCER_MIN:
                  if (current == NULL || (Z_TYPE_P(current) != IS_LONG && Z_TYPE_P(current) != IS_DOUBLE)) {
                      continue;
                  }
                  if (result_val == NULL) {
                      zval tmp;
                      ZVAL_DEREF(current);
                      ZVAL_COPY(&tmp, current);
                      result_val = REDUCER_HASH_ADD_NEW(result_ht, num_key, alias, &tmp);

                  } else {
                      switch (Z_TYPE_P(result_val) ) {
                          case IS_LONG:
                              if (Z_LVAL_P(current) < Z_LVAL_P(result_val)) {
                                  Z_LVAL_P(result_val) = Z_LVAL_P(current);
                              }
                          break;
                          case IS_DOUBLE:
                              if (Z_DVAL_P(current) < Z_DVAL_P(result_val)) {
                                  Z_DVAL_P(result_val) = Z_DVAL_P(current);
                              }
                          break;
                      }
                  }
                  break;
              case REDUCER_MAX:
                  if (current == NULL || (Z_TYPE_P(current) != IS_LONG && Z_TYPE_P(current) != IS_DOUBLE)) {
                      continue;
                  }
                  if (result_val == NULL) {
                      zval tmp;
                      ZVAL_DEREF(current);
                      ZVAL_COPY(&tmp, current);
                      result_val = REDUCER_HASH_ADD_NEW(result_ht, num_key, alias, &tmp);
                  } else {
                      switch (Z_TYPE_P(result_val) ) {
                          case IS_LONG:
                              if (Z_LVAL_P(current) > Z_LVAL_P(result_val)) {
                                  Z_LVAL_P(result_val) = Z_LVAL_P(current);
                              }
                          break;
                          case IS_DOUBLE:
                              if (Z_DVAL_P(current) > Z_DVAL_P(result_val)) {
                                  Z_DVAL_P(result_val) = Z_DVAL_P(current);
                              }
                          break;
                      }
                  }
                  break;
              case REDUCER_AVG:
              case REDUCER_SUM:
                  if (current == NULL || (Z_TYPE_P(current) != IS_LONG && Z_TYPE_P(current) != IS_DOUBLE)) {
                      continue;
                  }
                  if (result_val == NULL) {
                      zval tmp;
                      ZVAL_DEREF(current);
                      ZVAL_COPY(&tmp, current);
                      result_val = REDUCER_HASH_ADD_NEW(result_ht, num_key, alias, &tmp);
                  } else {
                      switch (Z_TYPE_P(result_val) ) {
                          case IS_LONG:
                              Z_LVAL_P(result_val) += Z_LVAL_P(current);
                          break;
                          case IS_DOUBLE:
                              Z_DVAL_P(result_val) += Z_DVAL_P(current);
                          break;
                      }
                  }
                  break;
              case REDUCER_COUNT:
                  if (result_val == NULL) {
                      zval tmp;
                      ZVAL_LONG(&tmp, 0);
                      result_val = REDUCER_HASH_ADD_NEW(result_ht, num_key, alias, &tmp);
                  } else {
                      Z_LVAL_P(result_val)++;
                  }
                  break;
              case REDUCER_LAST:
                  break;
              case REDUCER_FIRST:
                  break;
            }
        }


    } ZEND_HASH_FOREACH_END();

  } ZEND_HASH_FOREACH_END();


  ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(compiled_aggregators), num_key, alias, aggregator) {
      agg_type = zend_hash_index_find(Z_ARRVAL_P(aggregator), 0);
      if (Z_TYPE_P(agg_type) != IS_LONG) {
          continue;
      }

      result_val = REDUCER_HASH_FIND(result_ht, num_key, alias);
      if (result_val != NULL) {
          switch (Z_LVAL_P(agg_type)) {
              case REDUCER_AVG:
                  convert_to_double(result_val);
                  Z_DVAL_P(result_val) /= cnt;
              break;
          }
      }
  } ZEND_HASH_FOREACH_END();

  zval_dtor(&compiled_aggregators);
  return result;
}


zval group_items(zval* rows, zend_string* field)
{
    zval *row, *row_key, *group, groups_array;
    HashTable * groups_ht;

    // Allocate the group array for the current aggregation
    array_init(&groups_array);
    groups_ht = Z_ARRVAL(groups_array);

    ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(rows), row) {
        if ((row_key = zend_hash_find(Z_ARRVAL_P(row), field)) != NULL) {
            if (Z_TYPE_P(row_key) == IS_STRING) {
                if ((group = zend_hash_find(groups_ht, Z_STR_P(row_key))) == NULL) {
                    // Allocate a new group array
                    zval new_group;
                    array_init(&new_group);

                    // Set the group array into the groups array.
                    group = zend_hash_update(groups_ht, Z_STR_P(row_key), &new_group);
                }
            } else if (Z_TYPE_P(row_key) == IS_LONG) {
                if ((group = zend_hash_index_find(groups_ht, Z_LVAL_P(row_key))) == NULL) {
                    // Allocate a new group array
                    zval new_group;
                    array_init(&new_group);

                    // Set the group array into the groups array.
                    group = zend_hash_index_update(groups_ht, Z_LVAL_P(row_key), &new_group);
                }
            }

            // Append the row into the group array
            if (Z_REFCOUNTED_P(row)) {
                Z_ADDREF_P(row);
            }
            add_next_index_zval(group, row);
        }
    } ZEND_HASH_FOREACH_END();
    return groups_array;
}

zval group_groups(zval* groups, zval* fields TSRMLS_DC) {

    zval *field, *tmp_group, *group;

    ZEND_HASH_FOREACH_VAL(HASH_OF(fields), field) {

        zval tmp_collection;
        array_init(&tmp_collection);

        ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(groups), group) {

            zval tmp_groups = group_items(group, Z_STR_P(field));
            ZEND_HASH_FOREACH_VAL(Z_ARRVAL(tmp_groups), tmp_group) {
                zval_add_ref(tmp_group);
                add_next_index_zval(&tmp_collection, tmp_group);
            } ZEND_HASH_FOREACH_END();
            zval_dtor(&tmp_groups);

        } ZEND_HASH_FOREACH_END();

        zval_dtor(groups); // destruct the previous groups array.
        ZVAL_COPY(groups, &tmp_collection);

        // destruct tmp_collection after copying the array.
        zval_dtor(&tmp_collection);

    } ZEND_HASH_FOREACH_END();

    return *groups;
}

/**
 * group input rows into group arrays.
 */
zval group_rows(zval* rows, zval* fields TSRMLS_DC) {
    zval groups;
    array_init(&groups);

    Z_ADDREF_P(rows);
    add_next_index_zval(&groups, rows);
    return group_groups(&groups, fields TSRMLS_CC);
}

PHP_FUNCTION(fold_rows)
{
    zval *rows, *fields, *aggregators;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "aaa", &rows, &fields, &aggregators) == FAILURE) {
        return;
    }
    zval result = fold_rows(rows, fields, aggregators);
    ZVAL_COPY(return_value, &result);
    zval_dtor(&result);
}

PHP_FUNCTION(group_by)
{
    zval *rows, *fields, *aggregators, groups, *group;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "aaa", &rows, &fields, &aggregators) == FAILURE) {
        return;
    }

    groups = group_rows(rows, fields TSRMLS_CC);

    // push folded result into return_value array.
    array_init(return_value);
    ZEND_HASH_FOREACH_VAL(Z_ARRVAL(groups), group) {
      zval res = fold_rows(group, fields, aggregators);
      add_next_index_zval(return_value, &res);
    } ZEND_HASH_FOREACH_END();
    zval_dtor(&groups);
}
