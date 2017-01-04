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

ZEND_BEGIN_ARG_INFO_EX(arginfo_aggregate, 0, 0, 1)
  ZEND_ARG_INFO(0, array)
  ZEND_ARG_INFO(0, fields)
  ZEND_ARG_INFO(0, aggregators)
ZEND_END_ARG_INFO()

static const zend_function_entry reducer_functions[] = {
  PHP_FE(group_by,  arginfo_group_by)
  PHP_FE(aggregate,  arginfo_aggregate)
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

#define REDUCER_HASH_UPDATE(ht, num_idx, str_idx, val) \
      (str_idx) \
      ? zend_hash_update(ht, str_idx, val) \
      : zend_hash_index_update(ht, num_idx, val)

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
    REGISTER_LONG_CONSTANT("REDUCER_AGGR_SUM"   , REDUCER_AGGR_SUM   , CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("REDUCER_AGGR_AVG"   , REDUCER_AGGR_AVG   , CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("REDUCER_AGGR_COUNT" , REDUCER_AGGR_COUNT , CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("REDUCER_AGGR_FIRST" , REDUCER_AGGR_FIRST , CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("REDUCER_AGGR_LAST"  , REDUCER_AGGR_LAST , CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("REDUCER_AGGR_MIN"  , REDUCER_AGGR_MIN , CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("REDUCER_AGGR_MAX"  , REDUCER_AGGR_MAX , CONST_CS | CONST_PERSISTENT);
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

void compile_aggregator(compiled_agt *current_agt, zval *type) {
    if (Z_TYPE_P(type) == IS_LONG) {
        current_agt->is_callable = 0;
        current_agt->type = type;
    } else if (zend_is_callable(type, IS_CALLABLE_CHECK_NO_ACCESS, NULL)) {
        current_agt->is_callable = 1;
        current_agt->type = type;

        // fetch fci
        current_agt->fci = empty_fcall_info;
        current_agt->fci_cache = empty_fcall_info_cache;

        char *errstr = NULL;
        if (zend_fcall_info_init(type, 0, &current_agt->fci, &current_agt->fci_cache, NULL, &errstr) == FAILURE) {
            php_error_docref(NULL, E_USER_ERROR, "Error setting converter callback: %s", errstr);
        }
        if (errstr) {
            efree(errstr);
        }

        // shared fci configuration
        current_agt->fci.param_count = 2;
        current_agt->fci.no_separation = 0;
    }
}

void compile_aggregators(compiled_agt *agts, zval *aggregators) {
  compiled_agt *current_agt;
  uint agt_idx = 0;
  ulong num_alias;
  zend_string *alias;
  zval *aggregator, *tmp;

  ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(aggregators), num_alias, alias, aggregator) {
      current_agt = &agts[agt_idx++];

      if (Z_TYPE_P(aggregator) == IS_LONG) {

          compile_aggregator(current_agt, aggregator);

          if (alias) {
              current_agt->alias = alias;
              current_agt->selector = alias;
          } else {
              current_agt->alias = NULL;
              current_agt->selector = NULL;
              current_agt->num_alias = num_alias;
              current_agt->num_selector = num_alias;
          }
      } 
      else if (zend_is_callable(aggregator, IS_CALLABLE_CHECK_NO_ACCESS, NULL)) {

          compile_aggregator(current_agt, aggregator);

          if (alias) {
              current_agt->alias = alias;
              current_agt->selector = alias;
          } else {
              current_agt->alias = NULL;
              current_agt->selector = NULL;
              current_agt->num_alias = num_alias;
              current_agt->num_selector = num_alias;
          }

      } else if (Z_TYPE_P(aggregator) == IS_ARRAY) {

          tmp = zend_hash_str_find(Z_ARRVAL_P(aggregator), "aggregator", sizeof("aggregator") - 1);
          if (tmp == NULL) {
              php_error_docref(NULL, E_USER_ERROR, "Aggregator is not defined.");
          }
          ZVAL_DEREF(tmp);
          compile_aggregator(current_agt, tmp);

          if (alias) {
              current_agt->alias = alias;
          } else {
              current_agt->alias = NULL;
              current_agt->num_alias = num_alias;
          }

          tmp = zend_hash_str_find(Z_ARRVAL_P(aggregator), "selector", sizeof("selector") -1);
          if (tmp) {
              ZVAL_DEREF(tmp);
              if (Z_TYPE_P(tmp) == IS_STRING) { 
                  current_agt->selector = Z_STR_P(tmp);
              } else if (Z_TYPE_P(tmp) == IS_LONG) {
                  current_agt->num_selector = Z_LVAL_P(tmp);
              }
          } else {

            if (alias) {
                current_agt->selector = alias;
            } else {
                current_agt->selector = NULL;
                current_agt->num_selector = num_alias;
            }

          }
      } else {
          php_error_docref(NULL, E_USER_ERROR, "Unsupported aggregator");
      }

  } ZEND_HASH_FOREACH_END();
}


zval aggregate(zval* rows, zval* fields, compiled_agt* agts, uint agts_cnt)
{
  zval result, *row, *carry_val, *current_val, *first, *field, *tmp;
  ulong num_alias, num_selector;
  zend_string *selector, *alias;

  uint agt_idx;
  compiled_agt *current_agt;

  array_init(&result);

  HashTable *ht = Z_ARRVAL_P(rows);
  HashTable *result_ht = Z_ARRVAL(result);
  zend_long cnt = zend_array_count(ht);

  zend_hash_internal_pointer_reset(ht);

  if ((first = zend_hash_get_current_data(ht)) != NULL) {
      ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(fields), field) {
          if ((carry_val = zend_hash_find(Z_ARRVAL_P(first), Z_STR_P(field))) != NULL) {
              zend_hash_add_new(result_ht, Z_STR_P(field), carry_val);
          }
      } ZEND_HASH_FOREACH_END();


      for (agt_idx = 0; agt_idx < agts_cnt ; agt_idx++) {
          current_agt = &agts[agt_idx];
          if (Z_TYPE_P(current_agt->type) == IS_LONG) {
              carry_val = REDUCER_HASH_FIND(result_ht, current_agt->num_alias, current_agt->alias);
          }
      }

  }




  // Iterate the rows and aggregate the result.
  ZEND_HASH_FOREACH_VAL(ht, row) {
    if (Z_TYPE_P(row) != IS_ARRAY) {
        php_error_docref(NULL, E_USER_ERROR, "input row is not an array.");
    }

    for (agt_idx = 0; agt_idx < agts_cnt ; agt_idx++) {
        current_agt = &agts[agt_idx];

        // get the carried value, and then use aggregator to reduce the values.
        current_val = REDUCER_HASH_FIND(Z_ARRVAL_P(row), current_agt->num_selector, current_agt->selector);

        if (current_val == NULL) {
            continue;
        }

        carry_val = REDUCER_HASH_FIND(result_ht, current_agt->num_alias, current_agt->alias);

        if (current_agt->is_callable) {
            zval retval;
            zval args[2];

            if (carry_val == NULL) {
                zval tmp;
                ZVAL_LONG(&tmp, 0); // set initial value = zero
                carry_val = REDUCER_HASH_ADD_NEW(result_ht, current_agt->num_alias, current_agt->alias, &tmp);
            }

            ZVAL_COPY(&args[0], carry_val);
            ZVAL_COPY(&args[1], current_val);

            current_agt->fci.retval = &retval;
            current_agt->fci.params = args;

            zend_call_function(&current_agt->fci, &current_agt->fci_cache); // == SUCCESS;
            ZVAL_COPY(carry_val, &retval);

            zval_ptr_dtor(&retval);
            zval_ptr_dtor(&args[0]);
            zval_ptr_dtor(&args[1]);

        } else if (Z_TYPE_P(current_agt->type) == IS_LONG) {

            switch (Z_LVAL_P(current_agt->type)) {
              case REDUCER_AGGR_MIN:
                  if ((Z_TYPE_P(current_val) != IS_LONG && Z_TYPE_P(current_val) != IS_DOUBLE)) {
                      continue;
                  }
                  if (carry_val == NULL) {
                      zval tmp;
                      ZVAL_DEREF(current_val);
                      ZVAL_COPY(&tmp, current_val);
                      carry_val = REDUCER_HASH_ADD_NEW(result_ht, current_agt->num_alias, current_agt->alias, &tmp);

                  } else {
                      switch (Z_TYPE_P(carry_val) ) {
                          case IS_LONG:
                              if (Z_LVAL_P(current_val) < Z_LVAL_P(carry_val)) {
                                  Z_LVAL_P(carry_val) = Z_LVAL_P(current_val);
                              }
                          break;
                          case IS_DOUBLE:
                              if (Z_DVAL_P(current_val) < Z_DVAL_P(carry_val)) {
                                  Z_DVAL_P(carry_val) = Z_DVAL_P(current_val);
                              }
                          break;
                      }
                  }
                  break;
              case REDUCER_AGGR_MAX:
                  if ((Z_TYPE_P(current_val) != IS_LONG && Z_TYPE_P(current_val) != IS_DOUBLE)) {
                      continue;
                  }
                  if (carry_val == NULL) {
                      zval tmp;
                      ZVAL_DEREF(current_val);
                      ZVAL_COPY(&tmp, current_val);
                      carry_val = REDUCER_HASH_ADD_NEW(result_ht, current_agt->num_alias, current_agt->alias, &tmp);
                  } else {
                      switch (Z_TYPE_P(carry_val) ) {
                          case IS_LONG:
                              if (Z_LVAL_P(current_val) > Z_LVAL_P(carry_val)) {
                                  Z_LVAL_P(carry_val) = Z_LVAL_P(current_val);
                              }
                          break;
                          case IS_DOUBLE:
                              if (Z_DVAL_P(current_val) > Z_DVAL_P(carry_val)) {
                                  Z_DVAL_P(carry_val) = Z_DVAL_P(current_val);
                              }
                          break;
                      }
                  }
                  break;
              case REDUCER_AGGR_AVG:
              case REDUCER_AGGR_SUM:
                  if ((Z_TYPE_P(current_val) != IS_LONG && Z_TYPE_P(current_val) != IS_DOUBLE)) {
                      continue;
                  }
                  if (carry_val == NULL) {
                      zval tmp;
                      ZVAL_DEREF(current_val);
                      ZVAL_COPY(&tmp, current_val);
                      carry_val = REDUCER_HASH_ADD_NEW(result_ht, current_agt->num_alias, current_agt->alias, &tmp);
                  } else {
                      switch (Z_TYPE_P(carry_val)) {
                          case IS_LONG:
                              Z_LVAL_P(carry_val) += Z_LVAL_P(current_val);
                          break;
                          case IS_DOUBLE:
                              Z_DVAL_P(carry_val) += Z_DVAL_P(current_val);
                          break;
                      }
                  }
                  break;
              case REDUCER_AGGR_COUNT:
                  if (carry_val == NULL) {
                      zval tmp;
                      ZVAL_LONG(&tmp, 0);
                      carry_val = REDUCER_HASH_ADD_NEW(result_ht, current_agt->num_alias, current_agt->alias, &tmp);
                  } else {
                      Z_LVAL_P(carry_val)++;
                  }
                  break;
              case REDUCER_AGGR_LAST:
                  carry_val = REDUCER_HASH_UPDATE(result_ht, current_agt->num_alias, current_agt->alias, current_val);
                  break;
              case REDUCER_AGGR_FIRST:
                  if (carry_val == NULL) {
                      carry_val = REDUCER_HASH_ADD_NEW(result_ht, current_agt->num_alias, current_agt->alias, current_val);
                  }
                  break;
            }
        }
    }

  } ZEND_HASH_FOREACH_END();


  for (agt_idx = 0; agt_idx < agts_cnt ; agt_idx++) {
      current_agt = &agts[agt_idx];
      if (Z_TYPE_P(current_agt->type) == IS_LONG) {
          carry_val = REDUCER_HASH_FIND(result_ht, current_agt->num_alias, current_agt->alias);
          if (carry_val != NULL) {
              switch (Z_LVAL_P(current_agt->type)) {
                  case REDUCER_AGGR_AVG:
                      convert_to_double(carry_val);
                      Z_DVAL_P(carry_val) /= cnt;
                  break;
              }
          }
      }
  }
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
                    array_init_size(&new_group, GROUP_INIT_SIZE);

                    // Set the group array into the groups array.
                    group = zend_hash_update(groups_ht, Z_STR_P(row_key), &new_group);
                }
            } else if (Z_TYPE_P(row_key) == IS_LONG) {
                if ((group = zend_hash_index_find(groups_ht, Z_LVAL_P(row_key))) == NULL) {
                    // Allocate a new group array
                    zval new_group;
                    array_init_size(&new_group, GROUP_INIT_SIZE);

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
                Z_ADDREF_P(tmp_group);
                add_next_index_zval(&tmp_collection, tmp_group);
            } ZEND_HASH_FOREACH_END();
            zval_ptr_dtor(&tmp_groups);

        } ZEND_HASH_FOREACH_END();

        zval_ptr_dtor(groups); // destruct the previous groups array.

        ZVAL_COPY(groups, &tmp_collection);


        // destruct tmp_collection after copying the array.
        zval_ptr_dtor(&tmp_collection);

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

PHP_FUNCTION(aggregate)
{
    zval *rows, *fields, *aggregators;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "aaa", &rows, &fields, &aggregators) == FAILURE) {
        return;
    }

    zend_long agts_cnt = zend_array_count(Z_ARRVAL_P(aggregators));
    compiled_agt agts[agts_cnt];
    compile_aggregators(agts, aggregators);

    zval result = aggregate(rows, fields, agts, agts_cnt);
    RETVAL_ZVAL(&result, 1, 1);
}

PHP_FUNCTION(group_by)
{
    zval *rows, *fields, *aggregators, groups, *group;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "aaa", &rows, &fields, &aggregators) == FAILURE) {
        return;
    }

    groups = group_rows(rows, fields TSRMLS_CC);

    zend_long agts_cnt = zend_array_count(Z_ARRVAL_P(aggregators));
    compiled_agt agts[agts_cnt];
    compile_aggregators(agts, aggregators);

    // push folded result into return_value array.
    array_init(return_value);
    ZEND_HASH_FOREACH_VAL(Z_ARRVAL(groups), group) {
      zval res = aggregate(group, fields, agts, agts_cnt);
      add_next_index_zval(return_value, &res);
    } ZEND_HASH_FOREACH_END();
    zval_ptr_dtor(&groups);
}
