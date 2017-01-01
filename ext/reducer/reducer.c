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
    php_info_print_table_row( 2, "Input Validation and Filtering", "enabled" );
    php_info_print_table_row( 2, "Revision", "$Id: 9cc4bdc40f4013d538d685f912be2e9698c649cc $");
    php_info_print_table_end();
    DISPLAY_INI_ENTRIES();
}


zval fold_group(zval* rows, zval* aggregators)
{
  zval first;
  array_init(&first);

  HashTable* ht = Z_ARRVAL_P(rows);
  // HashTable * target_hash = HASH_OF(rows);

  HashPosition pos;
  uint32_t ht_iter;

  // reset the pointer and update pos (HashPosition)
  zend_hash_internal_pointer_reset_ex(ht, &pos);

  // create iterator from pos
  // ht_iter = zend_hash_iterator_add(ht, pos);

  zval *entry;
  if ((entry = zend_hash_get_current_data(ht)) == NULL) {
    return first;
  }
  return first;

  zend_hash_move_forward_ex(ht, &pos);

  zval key;
  zval *zv;

  do {
    zv = zend_hash_get_current_data_ex(ht, &pos);
    if (zv == NULL) {
      break;
    }
    
    // Ensure the value is a reference. Otherwise the location of the value may be freed.
    ZVAL_MAKE_REF(zv);

    // get key
    zend_hash_get_current_key_zval_ex(ht, &key, &pos);
    zend_hash_move_forward_ex(ht, &pos);

  } while (!EG(exception));
  // zend_hash_iterator_del(ht_iter);
    
  /*
		ZVAL_DEREF(entry);
		ZVAL_COPY(return_value, entry);
    */
  // zend_hash_move_forward(ht);
	// zend_hash_get_current_key_zval(array, return_value);
  
  /*
  do {
		zv = zend_hash_get_current_data_ex(target_hash, &pos);
				zend_hash_move_forward_ex(target_hash, &pos);
		// Ensure the value is a reference. Otherwise the location of the value may be freed.
		ZVAL_MAKE_REF(zv);
		zend_hash_get_current_key_zval_ex(target_hash, &args[1], &pos);

		zend_hash_move_forward_ex(target_hash, &pos);
  }
  */
  return first;
}


zval group_items(zval* rows, zend_string* field)
{
    // Allocate the group array for the current aggregation
    zval groups_array;
    array_init(&groups_array);
    // ZVAL_NEW_ARR(&groups_array);

    HashTable * groups_ht = Z_ARRVAL(groups_array);

    zend_string *key;
    ulong num_key;
    zval *row;

    ZEND_HASH_FOREACH_KEY_VAL(HASH_OF(rows), num_key, key, row) {
      // if (key) { }
      // find the key in the row array.
      zval* row_key;
      if ((row_key = zend_hash_find(Z_ARRVAL_P(row), field)) != NULL) {
        zval* group;
        if ((group = zend_hash_find(groups_ht, Z_STR_P(row_key))) == NULL) {
          // Allocate a new group array
          zval new_group;
          array_init(&new_group);

          // Set the group array into the groups array.
          if ((group = zend_hash_update(groups_ht, Z_STR_P(row_key), &new_group)) == NULL) {
            // XXX: failed
          }
        }

        // Append the row into the group array
        Z_ADDREF_P(row);
        add_next_index_zval(group, row);
      }

    } ZEND_HASH_FOREACH_END();
    return groups_array;
}


PHP_FUNCTION(group_by)
{
    zval *rows;
    zval *fields;
    zval *aggregators;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "aaa", &rows, &fields, &aggregators) == FAILURE) {
        return;
    }


    array_init(return_value);

    zval *field;
    ZEND_HASH_FOREACH_VAL(HASH_OF(fields), field) {

      zval groups = group_items(rows, Z_STR_P(field));

      zval* group;
      ZEND_HASH_FOREACH_VAL(Z_ARRVAL(groups), group) {
        zval fold = fold_group(group, aggregators);
        zval_ptr_dtor(&fold);
      } ZEND_HASH_FOREACH_END();
      add_next_index_zval(return_value, &groups);



    } ZEND_HASH_FOREACH_END();

    // (zv, copy, dtor)
    // RETURN_ZVAL( );
    // RETURN_FALSE;
}
