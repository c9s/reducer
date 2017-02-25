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

    REGISTER_LONG_CONSTANT("REDUCER_TYPE_LONG"   , REDUCER_TYPE_LONG   , CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("REDUCER_TYPE_STRING"   , REDUCER_TYPE_STRING   , CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("REDUCER_TYPE_DOUBLE"   , REDUCER_TYPE_DOUBLE   , CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("REDUCER_TYPE_BOOLEAN"   , REDUCER_TYPE_BOOLEAN   , CONST_CS | CONST_PERSISTENT);

    REGISTER_LONG_CONSTANT("REDUCER_AGGR_SUM"   , REDUCER_AGGR_SUM   , CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("REDUCER_AGGR_AVG"   , REDUCER_AGGR_AVG   , CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("REDUCER_AGGR_COUNT" , REDUCER_AGGR_COUNT , CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("REDUCER_AGGR_CNT"   , REDUCER_AGGR_COUNT , CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("REDUCER_AGGR_FIRST" , REDUCER_AGGR_FIRST , CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("REDUCER_AGGR_LAST"  , REDUCER_AGGR_LAST , CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("REDUCER_AGGR_MIN"  , REDUCER_AGGR_MIN , CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("REDUCER_AGGR_MAX"  , REDUCER_AGGR_MAX , CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("REDUCER_AGGR_GROUP"  , REDUCER_AGGR_GROUP , CONST_CS | CONST_PERSISTENT);
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


zend_always_inline static uint isa_cast(compiled_agt *agt, zval *val)
{
    if ((Z_TYPE_P(val) == IS_LONG && Z_TYPE_P(val) == IS_DOUBLE)) {
      return 1;
    }
    switch (agt->isa) {
      case REDUCER_TYPE_LONG:
      convert_to_long(val);
      return 1;

      case REDUCER_TYPE_DOUBLE:
      convert_to_double(val);
      return 1;

      case REDUCER_TYPE_BOOLEAN:
      convert_to_boolean(val);
      return 1;

      default:
      return 0;
    }
    return 1;
}

zend_always_inline static void compile_aggregator_selector_default(compiled_agt *agt, zend_long num_alias, zend_string * alias)
{
    if (alias) {
        agt->alias = alias;
        agt->selector = alias;
    } else {
        agt->alias = NULL;
        agt->selector = NULL;
        agt->num_alias = num_alias;
        agt->num_selector = num_alias;
    }
}

zend_always_inline static void compile_aggregator_selector(compiled_agt *agt, zval * agt_def, zend_long num_alias, zend_string * alias)
{
    zval * sel = zend_hash_str_find(Z_ARRVAL_P(agt_def), "selector", sizeof("selector") -1);
    if (sel) {
        ZVAL_DEREF(sel);

        if (Z_TYPE_P(sel) == IS_STRING) { 

            agt->selector = Z_STR_P(sel);

        } else if (Z_TYPE_P(sel) == IS_LONG) {

            agt->num_selector = Z_LVAL_P(sel);

        } else {

            php_error_docref(NULL, E_USER_ERROR, "Invalid selector type.");

        }

        if (alias) {

          agt->alias = alias;

        } else {

          agt->num_alias = num_alias;

        }


    } else {
      if (alias) {

          agt->selector = alias;
          agt->alias = alias;

      } else {

          agt->selector = NULL;
          agt->num_selector = num_alias;

          agt->alias = NULL;
          agt->num_alias = num_alias;

      }
    }
}

static zend_always_inline void compile_aggregator_isa(compiled_agt *agt, zval * agt_def)
{
  zval * isa = zend_hash_str_find(Z_ARRVAL_P(agt_def), "isa", sizeof("isa")-1);
  if (!isa) {
    agt->isa = REDUCER_TYPE_LONG;
    return;
  }

  switch (Z_TYPE_P(isa)) {
    case IS_LONG:
      agt->isa = Z_LVAL_P(isa);
      break;

    case IS_STRING:
      if (strncasecmp(Z_STRVAL_P(isa), "int", sizeof("int")-1) == 0) {
        agt->isa = REDUCER_TYPE_LONG;
      } else if (strncasecmp(Z_STRVAL_P(isa), "double", sizeof("double")-1) == 0) {
        agt->isa = REDUCER_TYPE_DOUBLE;
      } else if (strncasecmp(Z_STRVAL_P(isa), "boolean", sizeof("boolean")-1) == 0) {
        agt->isa = REDUCER_TYPE_BOOLEAN;
      } else {
        php_error_docref(NULL, E_USER_ERROR, "Unsupported aggregator typename.");
      }
      break;

    default:
      php_error_docref(NULL, E_USER_ERROR, "Unsupported aggregator data type.");
      break;
  }
}



static zend_always_inline void compile_aggregator_function_constant(compiled_agt *agt, zval *type)
{
  agt->is_callable = 0;
  agt->type = type;
}

static zend_always_inline void compile_aggregator_function_callable(compiled_agt *agt, zval *type)
{
    agt->is_callable = 1;
    agt->type = type;

    // fetch fci
    agt->fci = empty_fcall_info;
    agt->fci_cache = empty_fcall_info_cache;

    char *errstr = NULL;
    if (zend_fcall_info_init(type, 0, &agt->fci, &agt->fci_cache, NULL, &errstr) == FAILURE) {
        php_error_docref(NULL, E_USER_ERROR, "Error setting converter callback: %s", errstr);
    }
    if (errstr) {
        efree(errstr);
    }

    // shared fci configuration
    agt->fci.param_count = 2;
    agt->fci.no_separation = 0;
}


static zend_always_inline void compile_aggregator_function(compiled_agt *agt, zval *agt_def)
{
    zval *fun;
    fun = zend_hash_str_find(Z_ARRVAL_P(agt_def), "aggregator", sizeof("aggregator") - 1);
    if (fun == NULL) {
        php_error_docref(NULL, E_USER_ERROR, "Aggregator is not defined.");
    }
    ZVAL_DEREF(fun);

    if (Z_TYPE_P(fun) == IS_LONG) {

        compile_aggregator_function_constant(agt, fun);

    } else if (zend_is_callable(fun, IS_CALLABLE_CHECK_NO_ACCESS, NULL)) {

        compile_aggregator_function_callable(agt, fun);

    } else {

        php_error_docref(NULL, E_USER_ERROR, "Invalid aggregator function.");

    }
}

static zend_always_inline void compile_aggregator(compiled_agt *agt, zval *agt_def, ulong num_alias, zend_string *alias)
{
    if (Z_TYPE_P(agt_def) == IS_LONG) {

        compile_aggregator_function_constant(agt, agt_def);

        agt->isa = REDUCER_TYPE_LONG;

        compile_aggregator_selector_default(agt, num_alias, alias);

    } else if (zend_is_callable(agt_def, IS_CALLABLE_CHECK_NO_ACCESS, NULL)) {

        compile_aggregator_function_callable(agt, agt_def);

        agt->isa = REDUCER_TYPE_LONG;

        compile_aggregator_selector_default(agt, num_alias, alias);

    } else if (Z_TYPE_P(agt_def) == IS_ARRAY) {


          compile_aggregator_function(agt, agt_def);
          compile_aggregator_selector(agt, agt_def, num_alias, alias);
          compile_aggregator_isa(agt, agt_def);

    } else {

        php_error_docref(NULL, E_USER_ERROR, "Unsupported aggregator");
    }
}

void compile_aggregators(compiled_agt *agts, zval *aggregators)
{
  compiled_agt *agt;
  uint agt_idx = 0;
  ulong num_alias;
  zend_string *alias;
  zval *arg, *tmp;

  ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(aggregators), num_alias, alias, arg) {

      compile_aggregator(&agts[agt_idx++], arg, num_alias, alias);

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

          if (Z_TYPE_P(field) == IS_STRING) {
              if ((carry_val = zend_hash_find(Z_ARRVAL_P(first), Z_STR_P(field))) != NULL) {
                  zend_hash_add_new(result_ht, Z_STR_P(field), carry_val);
              }
          } else if (Z_TYPE_P(field) == IS_LONG) {
              if ((carry_val = zend_hash_index_find(Z_ARRVAL_P(first), Z_LVAL_P(field))) != NULL) {
                  zend_hash_index_add_new(result_ht, Z_LVAL_P(field), carry_val);
              }
          } else {
              php_error_docref(NULL, E_USER_ERROR, "Unsupported field value type.");
          }

      } ZEND_HASH_FOREACH_END();

      // TODO: Aggregate value from the first row.
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

                  if (isa_cast(current_agt, current_val) == 0) {
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
                  if (isa_cast(current_agt, current_val) == 0) {
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
              case REDUCER_AGGR_GROUP:
                  if (carry_val == NULL) {
                      zval group;
                      array_init(&group);
                      add_next_index_zval(&group, current_val);
                      carry_val = REDUCER_HASH_ADD_NEW(result_ht, current_agt->num_alias, current_agt->alias, &group);
                  } else {
                      add_next_index_zval(carry_val, current_val);
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
    zval *row, *category_key, *group, groups_array;
    HashTable * groups_ht;

    // Allocate the group array for the current aggregation
    array_init(&groups_array);
    groups_ht = Z_ARRVAL(groups_array);

    ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(rows), row) {
        if ((category_key = zend_hash_find(Z_ARRVAL_P(row), field)) != NULL) {
            if (Z_TYPE_P(category_key) == IS_STRING) {
                if ((group = zend_hash_find(groups_ht, Z_STR_P(category_key))) == NULL) {
                    // Allocate a new group array
                    zval new_group;
                    array_init_size(&new_group, GROUP_INIT_SIZE);

                    // Set the group array into the groups array.
                    group = zend_hash_update(groups_ht, Z_STR_P(category_key), &new_group);
                }
            } else if (Z_TYPE_P(category_key) == IS_LONG) {
                if ((group = zend_hash_index_find(groups_ht, Z_LVAL_P(category_key))) == NULL) {
                    // Allocate a new group array
                    zval new_group;
                    array_init_size(&new_group, GROUP_INIT_SIZE);

                    // Set the group array into the groups array.
                    group = zend_hash_index_update(groups_ht, Z_LVAL_P(category_key), &new_group);
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
