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

static const zend_function_entry reducer_functions[] = {
	PHP_FE(group_by,		arginfo_group_by)
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
    // php_info_print_table_row( 2, "Input Validation and Filtering", "enabled" );
    // php_info_print_table_row( 2, "Revision", "$Id: 9cc4bdc40f4013d538d685f912be2e9698c649cc $");
    php_info_print_table_end();
    DISPLAY_INI_ENTRIES();
}


zval fold_group(zval* rows, zend_string* field, zval* aggregators)
{
  zval result;
  array_init(&result);

  zval *row;
  zval *result_val = NULL;
  zval *current;

  ZEND_HASH_FOREACH_VAL(HASH_OF(rows), row) {


    result_val = zend_hash_find(Z_ARRVAL(result), field);
    if (result_val == NULL) {
      zval *tmp;
      if ((tmp = zend_hash_find(Z_ARRVAL_P(row), field)) != NULL) {
        result_val = zend_hash_add_new(Z_ARRVAL(result), field, tmp);
        zval_add_ref(result_val);
      }
    }

    zval *aggregator;
    ulong num_key;
    zend_string *key;

    ZEND_HASH_FOREACH_KEY_VAL(HASH_OF(aggregators), num_key, key, aggregator) {
        if (Z_TYPE_P(aggregator) != IS_LONG) {
            continue;
        }
        if (key == NULL) {
            continue;
        }

        current = zend_hash_find(HASH_OF(row), key);

        // get the carried value, and then use aggregator to reduce the values.
        result_val = zend_hash_find(Z_ARRVAL(result), key);

        switch (Z_LVAL_P(aggregator)) {
          case REDUCER_SUM:
            if (current == NULL) {
                continue;
            }
            if (result_val == NULL) {
                // result_val = zend_hash_update(Z_ARRVAL(result), key, current);
                result_val = zend_hash_add_new(Z_ARRVAL(result), key, current);
                zval_add_ref(result_val);
            } else {
                Z_LVAL_P(result_val) = Z_LVAL_P(result_val) + Z_LVAL_P(current);
            }
            break;
          case REDUCER_COUNT:
            if (result_val == NULL) {
                zval tmp;
                ZVAL_LONG(&tmp, 0);
                result_val = zend_hash_add_new(Z_ARRVAL(result), key, &tmp);
                zval_add_ref(result_val);
            } else {
                Z_LVAL_P(result_val)++;
            }
            break;
          case REDUCER_AVG:
            break;
          case REDUCER_LAST:
            break;
          case REDUCER_FIRST:
            break;
        }

    } ZEND_HASH_FOREACH_END();

  } ZEND_HASH_FOREACH_END();
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
            add_next_index_zval(group, row);
            zval_addref_p(row);
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
                add_next_index_zval(&tmp_collection, tmp_group);
                zval_add_ref(tmp_group);
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
    add_next_index_zval(&groups, rows);
    return group_groups(&groups, fields TSRMLS_CC);
}


PHP_FUNCTION(group_by)
{
    zval *rows;
    zval *fields;
    zval *aggregators;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "aaa", &rows, &fields, &aggregators) == FAILURE) {
        return;
    }

    zval groups;
    zval* group;
    groups = group_rows(rows, fields TSRMLS_CC);

    zval* field;
    zend_hash_internal_pointer_end(HASH_OF(fields));
    field = zend_hash_get_current_data(HASH_OF(fields));

    // push folded result into return_value array.
    array_init(return_value);
    ZEND_HASH_FOREACH_VAL(Z_ARRVAL(groups), group) {
      zval res = fold_group(group, Z_STR_P(field), aggregators);
      add_next_index_zval(return_value, &res);
    } ZEND_HASH_FOREACH_END();
    zval_dtor(&groups);
}
