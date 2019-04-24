#ifndef APPCONFIG_H_INCLUDED
  #define APPCONFIG_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "datatypes.h"

/**
 * Structure to define diffrent entries for the config. <br>
 * define an array of this and initialise with default values <br>
 * before calling appConfig_Load. Make sure this is valid the whole time <br>
 * while this library is in use. <br>
 * An Example to show the (global) definition of an array containg some config: <br>
 *
 * char gCharArray[100]="Test initial String";
 *
 * AppConfigEntry taEntrys_m[]={{.keyName="TestInt",     .groupName=NULL},
 *                              {.keyName="TestUInt",    .groupName=NULL},
 *                              {.keyName="TestDouble",  .groupName=NULL},
 *                              {.keyName="TestBoolean", .groupName=NULL},
 *                              {.keyName="TestString",  .groupName=NULL}};
 *
 * Optional, use the macros in datatypes.h to perform further initialisation.
 *
 * DATA_SET_INT(taEntrys_m[0].tagData,10);
 * DATA_SET_INT(taEntrys_m[1].tagData,1010);
 * DATA_SET_INT(taEntrys_m[2].tagData,123.456);
 * ...
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

enum AppCfg_ReturnCode_T
{
  APPCONFIG_LOAD_EXIST =0,
  APPCONFIG_LOAD_NEW   =1,
  APPCONFIG_LOAD_ERROR =-1
};

typedef struct TagAppConfig_T* AppConfig;

/* Define this, to print errors to stderr */
#define APPCONFIG_PRINT_ERRORS


/**
 * Loads a config, if exist, or creates a new one.
 *
 * @param config   Pointer to AppConfig. Will be initialized on success.
 * @param appName  _IN_ The name of the app, can't be NULL.
 * @param entries  _IN_ Pointer to AppConfigEntry struct defined by the user.
 *                 Only a reference to this will be stored internal, <br>
 *                 make sure it's contents are valid until appConfig_Close(), <br>
 *                 so best would be define this on heap. Should be initialized before Loading the config.
 * @param entriesCount
 *                 _IN_ Count of param entries.
 * @param fileName _IN_OPT_ Filename for the config file, NULL for default name.
 * @param location _IN_OPT_ Overwrite the default Location for configfiles by specify this parameter. <br>
 *                 Pass NULL to use the default location of your Platform. <br>
 *                 Example: Default on Windows is: "C:User\<user>\AppData".
 *
 * @return APPCONFIG_LOAD_NEW   : New Config was created (didn't exist before)
 *         APPCONFIG_LOAD_EXIST : Loaded existing config successfully.
 *         APPCONFIG_LOAD_ERROR : Failed to load the config, don't use the config opject.
 */
extern int appConfig_Load(AppConfig *config,
                          const char *appName,
                          AppConfigEntry *entries,
                          size_t entriesCount,
                          const char *fileName,
                          const char *location);

/**
 * Saves the configuration to disk.
 *
 * @param config _IN_ The config, created by calling appConfig_Load.
 *
 * @return 0 on success, nonzero on failure.
 */
extern int appConfig_Save(const AppConfig config);

/**
 * Returns the current configuration path
 *
 * @param config The config
 *
 * @return The Configuration path which is currently set.
 */
extern const char *appConfig_GetPath(const AppConfig config);

/**
 * Clears up the internal data and frees all memory used for the config.
 * Don't use the config afther that anymore.
 *
 * @param config _IN_ The config object to clean up
 */
extern void appConfig_Close(AppConfig config);


#ifdef __cplusplus
}
#endif

#endif /* APPCONFIG_H_INCLUDED */
