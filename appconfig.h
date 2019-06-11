#ifndef APPCONFIG_H_INCLUDED
  #define APPCONFIG_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "datatypes.h"

/**
 * Structure to define different entries for the config. <br>
 * define an array of this and initialize with default values <br>
 * before calling appConfig_Load. Make sure this is valid the whole time <br>
 * while this library is in use. <br>
 * An Example to show the (global) definition of an array containg some data: <br>
 *
 * #define INIT_STRING "Test initial String" <br>
 * char charArray_m[100]; <br>
 *
 * AppConfigEntry taEntrys_m[]={{.keyName="TestInt",     .groupName=NULL}, <br>
 *                              {.keyName="TestUInt",    .groupName=NULL}, <br>
 *                              {.keyName="TestDouble",  .groupName=NULL}, <br>
 *                              {.keyName="TestBoolean", .groupName=NULL}, <br>
 *                              {.keyName="TestString",  .groupName=NULL}}; <br>
 *
 * Use the functions in datatypes.h to initialise the TagData Structure, e.g.: <br>
 *
 * dataType_Set_Int(&taEntrys_m[0].tagData,1234,eDataRepresentation_Int_Default); <br>
 * dataType_Set_Boolean(&taEntrys_m[3].tagData,1); <br>
 * dataType_Set_String(taEntrys_m[4].tagData,charArray_m,sizeof(charArray_m),INIT_STRING,sizeof(INIT_STRING));
 *
 *
 */
typedef struct
{
  /**
   * Used to group the config. Can be NULL if not needed. <br>
   */
  const char *groupName;
  /**
   * The keyName to identify the entry. Must be uniqe per group.
   */
  const char *keyName;
  /**
   * Contains information about data itself, @see TagData doc for details.
   */
  TagData tagData;
}AppConfigEntry;

/**
 * Return codes used from appConfig_Load().
 */
enum AppCfg_RetCode
{
  /**
   * No Error, function was successful
   */
  APPCFG_ERR_NONE,
  /**
   * Successfully loaded existing Configuration
   */
  APPCFG_LOAD_EXISTING,
  /**
   * Successfully created a new config (all default values are used)
   */
  APPCFG_LOAD_NEW,
  /**
   * Invalid Parameter / Error in parameter passed by the user
   */
  APPCFG_ERR_PARAM,
  /**
   * Data Storage is malformed and can't be read
   */
  APPCFG_ERR_DATA_MALFORMED,
  /**
   * I/O Error occurred while accessing the data storage
   */
  APPCFG_ERR_IO,
  /**
   * Internal Error occurred, e.g. malloc() failure
   */
  APPCFG_ERR_INTERNAL,
};

enum AppCfg_Option
{
  /**
   * If reading data from the storage, invalid or broken values are silently ignored. <br>
   * To get some information when reading invalid values, define APPCONFIG_PRINT_ERRORS, <br>
   * which will print some information inside this library then.
   */
  APPCFG_OPT_IGNORE_INVALID_ENTRIES=0x1,
};

typedef struct TagAppConfig_T* AppConfig;

/**
 * Define this, to print errors to stderr
 */
#define APPCONFIG_PRINT_ERRORS

/**
 * Opens or creates a new config datastructure object.
 *
 * @param config   _OUT_ Pointer to AppConfig. Will be initialized on success.
 * @param entries  _IN_ Pointer to AppConfigEntry struct defined by the user.
 *                 Only a reference to this will be stored internal, <br>
 *                 make sure it's contents are valid until appConfig_Close(), <br>
 *                 so best would be define this on heap. Should be initialized before Loading the config.
 * @param options  _IN_ Available options to use, see enum AppCfg_Option (combine via OR (|)).
 * @param entriesCount
 *                 _IN_ Count of param entries.
 * @param appName  _IN_ The name of the app, can't be NULL.
 * @param fileName _IN_OPT_ Filename for the config file, NULL for default name.
 * @param location _IN_OPT_ Overwrite the default Location for configfiles by specify this parameter. <br>
 *                 Pass NULL to use the default location of your Platform. <br>
 *                 Example: Default on Windows is: "C:User\<user>\AppData".
 *
 * @return APPCONFIG_ERR_NONE on success, on error: <br>
 *         APPCONFIG_LOAD_ERR_PARAM           <br>
 *         APPCONFIG_LOAD_ERR_IO              <br>
 *         APPCONFIG_LOAD_ERR_INTERNAL
 */
extern int appConfig_Open(AppConfig *config,
                          AppConfigEntry *entries,
                          size_t entriesCount,
                          unsigned int options,
                          const char *appName,
                          const char *fileName,
                          const char *location);

/**
 * Clears up the internal data and frees all memory used for the config.
 * Don't use the config after that anymore.
 *
 * @param config _IN_ The config, created by calling appConfig_Open()
 */
extern void appConfig_Close(AppConfig config);

/**
 * Load the config data from storage, if present. <br>
 * If there is no data present in storage, the default value will be kept.
 *
 * @param config _IN_ The config, created by calling appConfig_Open()
 *
 * @return APPCONFIG_LOAD_NEW or APPCONFIG_LOAD_EXIST on success, on error: <br>
 *         APPCONFIG_LOAD_ERR_PARAM           <br>
 *         APPCONFIG_LOAD_ERR_DATA_MALFORMED  <br>
 *         APPCONFIG_LOAD_ERR_IO              <br>
 *         APPCONFIG_LOAD_ERR_INTERNAL
 */
extern int appConfig_DataLoad(AppConfig config);

/**
 * Saves the config object's content to the storage.
 *
 * @param config _IN_ The config, created by calling appConfig_Open()
 *
 * @return APPCONFIG_ERR_NONE on success, on error: <br>
 *         APPCONFIG_LOAD_ERR_PARAM           <br>
 *         APPCONFIG_LOAD_ERR_IO              <br>
 *         APPCONFIG_LOAD_ERR_INTERNAL
 */
extern int appConfig_DataSave(AppConfig config);

/**
 * Deletes the stored data. This might be used, if the data is corrupted etc. <br>
 * It will not affect the current data in the config object.
 *
 * @param config _IN_ The config, created by calling appConfig_Open()
 *
 * @return APPCONFIG_ERR_NONE on success, on error: <br>
 *         APPCONFIG_LOAD_ERR_PARAM           <br>
 *         APPCONFIG_LOAD_ERR_IO
 */
extern int appConfig_DataDelete(AppConfig config);

/**
 * Prints out the contents of the current entries.
 *
 * @param config _IN_ The config, created by calling appConfig_Open()
 * @param fp     _IN_ The destination stream (e.g. stdout, stderr...)
 */
extern void appConfig_DumpContents(const AppConfig config,
                                   FILE *fp);

/**
 * Returns an error string with a short description to the specified error.
 *
 * @param error  _IN_ Errorcode
 *
 * @return Error String
 */
extern const char *appConfig_GetErrorString(int error);

/**
 * Returns the current configuration path
 *
 * @param config _IN_ The config, created by calling appConfig_Open()
 *
 * @return The Configuration path which is currently set.
 */
extern const char *appConfig_GetPath(const AppConfig config);


#ifdef __cplusplus
}
#endif

#endif /* APPCONFIG_H_INCLUDED */
