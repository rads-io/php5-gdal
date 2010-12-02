

#include <ogrsf_frmts.h>
#include "php.h"
#include "php_gdal.h"
#include "ogrdatasource.h"
#include "ogrsfdriver.h"
#include "ogrlayer.h"

zend_class_entry *gdal_ogrdatasource_ce;
zend_object_handlers ogrdatasource_object_handlers;

//
// PHP stuff
//

void ogrdatasource_free_storage(void *object TSRMLS_DC)
{
  php_ogrdatasource_object *obj = (php_ogrdatasource_object *)object;
  //delete obj->layer; // TODO
  zend_hash_destroy(obj->std.properties);
  FREE_HASHTABLE(obj->std.properties);
  efree(obj);
}

zend_object_value ogrdatasource_create_handler(zend_class_entry *type TSRMLS_DC)
{
  zval *tmp;
  zend_object_value retval;

  php_ogrdatasource_object *obj =
    (php_ogrdatasource_object *)emalloc(sizeof(php_ogrdatasource_object));
  memset(obj, 0, sizeof(php_ogrdatasource_object));
  obj->std.ce = type;

  ALLOC_HASHTABLE(obj->std.properties);
  zend_hash_init(obj->std.properties, 0, NULL, ZVAL_PTR_DTOR, 0);
  zend_hash_copy(obj->std.properties, &type->default_properties,
                 (copy_ctor_func_t)zval_add_ref,
                 (void *)&tmp, sizeof(zval *));

  retval.handle =
    zend_objects_store_put(obj, NULL,
                           ogrdatasource_free_storage, NULL TSRMLS_CC);
  retval.handlers = &ogrdatasource_object_handlers;

  return retval;

  //pdo_stmt_construct(stmt, return_value, dbstmt_ce, ctor_args TSRMLS_CC);

}

//
// CLASS METHODS
//

PHP_METHOD(OGRDataSource, GetName)
{
  OGRDataSource *datasource;
  php_ogrdatasource_object *obj;

  if (ZEND_NUM_ARGS() != 0) {
    WRONG_PARAM_COUNT;
  }

  obj = (php_ogrdatasource_object *)
    zend_object_store_get_object(getThis() TSRMLS_CC);
  datasource = obj->datasource;
  const char *name = datasource->GetName();
  if (name) {
    RETURN_STRING((char *)name, 1);
  } else {
    RETURN_NULL();
  }
}

PHP_METHOD(OGRDataSource, GetLayerCount)
{
  OGRDataSource *datasource;
  php_ogrdatasource_object *obj;

  if (ZEND_NUM_ARGS() != 0) {
    WRONG_PARAM_COUNT;
  }

  obj = (php_ogrdatasource_object *)
    zend_object_store_get_object(getThis() TSRMLS_CC);
  datasource = obj->datasource;
  RETURN_LONG(datasource->GetLayerCount());
}

PHP_METHOD(OGRDataSource, GetLayer)
{
  OGRDataSource *datasource;
  php_ogrdatasource_object *obj;
  OGRLayer *layer;
  php_ogrlayer_object *layer_obj;
  long index;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, (char*)"l",
                            &index) == FAILURE) {
    WRONG_PARAM_COUNT;
  }
  obj = (php_ogrdatasource_object *)
    zend_object_store_get_object(getThis() TSRMLS_CC);
  datasource = obj->datasource;

  layer = datasource->GetLayer(index);
  if (!layer) {
    RETURN_NULL();
  }
  if (object_init_ex(return_value, gdal_ogrlayer_ce) != SUCCESS) {
    RETURN_NULL();
  }
  layer_obj = (php_ogrlayer_object*)
    zend_object_store_get_object
    //zend_objects_get_address
    (return_value TSRMLS_CC);
  layer_obj->layer = layer;
}

PHP_METHOD(OGRDataSource, TestCapability)
{
  // Available capabilities as of 1.7.3 (ogr_core.h):
  //    "CreateLayer", "DeleteLayer"
  OGRDataSource *datasource;
  php_ogrdatasource_object *obj;
  char *strcapability = NULL;
  int strcapability_len;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, (char*)"s",
                            &strcapability, &strcapability_len) == FAILURE) {
    WRONG_PARAM_COUNT;
  }

  obj = (php_ogrdatasource_object *)
    zend_object_store_get_object(getThis() TSRMLS_CC);
  datasource = obj->datasource;
  RETURN_BOOL(datasource->TestCapability(strcapability));
}

PHP_METHOD(OGRDataSource, GetDriver)
{
  OGRDataSource *datasource;
  OGRSFDriver *driver;
  php_ogrdatasource_object *obj;
  driver_object *driver_obj;

  if (ZEND_NUM_ARGS() != 0) {
    WRONG_PARAM_COUNT;
  }

  obj = (php_ogrdatasource_object *)
    zend_object_store_get_object(getThis() TSRMLS_CC);
  datasource = obj->datasource;
  driver = datasource->GetDriver();
  if (!driver) {
    RETURN_NULL();
  }
  if (object_init_ex(return_value, gdal_ogrsfdriver_ce) != SUCCESS) {
    RETURN_NULL();
  }
  driver_obj = (driver_object*)
    zend_object_store_get_object(return_value TSRMLS_CC);
  driver_obj->driver = driver;
}


//
// PHP stuff
//

function_entry ogrdatasource_methods[] = {
  PHP_ME(OGRDataSource, GetName, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(OGRDataSource, GetLayerCount, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(OGRDataSource, GetLayer, NULL, ZEND_ACC_PUBLIC)
  // PHP_ME(OGRDataSource, GetLayerByName, NULL, ZEND_ACC_PUBLIC)
  // PHP_ME(OGRDataSource, DeleteLayer, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(OGRDataSource, TestCapability, NULL, ZEND_ACC_PUBLIC)
  // PHP_ME(OGRDataSource, CreateLayer, NULL, ZEND_ACC_PUBLIC)
  // PHP_ME(OGRDataSource, CopyLayer, NULL, ZEND_ACC_PUBLIC)
  // PHP_ME(OGRDataSource, GetStyleTable, NULL, ZEND_ACC_PUBLIC)
  // PHP_ME(OGRDataSource, SetStyleTableDirectly, NULL, ZEND_ACC_PUBLIC)
  // PHP_ME(OGRDataSource, SetStyleTable, NULL, ZEND_ACC_PUBLIC)
  // PHP_ME(OGRDataSource, ExecuteSQL, NULL, ZEND_ACC_PUBLIC)
  // PHP_ME(OGRDataSource, ReleaseResultSet, NULL, ZEND_ACC_PUBLIC)
  // PHP_ME(OGRDataSource, SyncToDisk, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(OGRDataSource, GetDriver, NULL, ZEND_ACC_PUBLIC)
  {NULL, NULL, NULL}
};

void php_gdal_ogrdatasource_startup(INIT_FUNC_ARGS)
{
  zend_class_entry ce;
  INIT_CLASS_ENTRY(ce, "OGRDataSource", ogrdatasource_methods);
  gdal_ogrdatasource_ce = zend_register_internal_class(&ce TSRMLS_CC);
  gdal_ogrdatasource_ce->create_object = ogrdatasource_create_handler;
  memcpy(&ogrdatasource_object_handlers,
         zend_get_std_object_handlers(), sizeof(zend_object_handlers));
  ogrdatasource_object_handlers.clone_obj = NULL;
}
