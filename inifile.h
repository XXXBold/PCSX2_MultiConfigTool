#ifndef INIFILE_H_INCLUDED
  #define INIFILE_H_INCLUDED

#ifdef __cplusplus
  extern "C" {
#endif

typedef enum
{
  eIniDataType_Int,
  eIniDataType_Uint,
  eIniDataType_Double,
  eIniDataType_Boolean,
  eIniDataType_String
}EIniDataType;

typedef struct TagIniFile*    Inifile;
typedef struct TagIniSection* IniSection;
typedef struct TagIniKey*     IniKey;


/**
 * Reads the file from specified path and returns a new Inifile Object with its content.
 *
 * @param path Path to file
 *
 * @return New created Inifile or NULL on error
 */
extern Inifile IniFile_Read(const char *path);

/**
 * Creates a new, empty Inifile object.
 *
 * @return New created Inifile or NULL on error
 */
extern Inifile IniFile_New(void);

/**
 * Reads the file from specified path and returns a new Inifile Object with its content.
 *
 * @param file The Inifile Object
 * @param path Destination path
 *
 * @return 0 on success, negative value on error.
 */
extern int IniFile_Write(Inifile file,
                         const char *path);

/**
 * Delete the Inifile and its contents.
 *
 * @param file The Inifile Object
 */
extern void IniFile_Delete(Inifile file);

/**
 * Finds a specific entry using the section and the key,
 * returns the containing data. <br>
 * It tries to convert data to the specified type.
 *
 * @param file The Inifile Object
 * @param section The Section name to search for
 * @param key The Value's key name
 * @param type The destination type
 * @param buffer Buffer for filling the data in, must match the type.
 * @param bufSize Size of buffer, only used for eIniDataType_String, for other types pass 0.
 *
 * @return 0 on success, negative value on error.
 */
extern int IniFile_FindEntry_GetValue(Inifile file,
                                      const char *section,
                                      const char *key,
                                      EIniDataType type,
                                      void *buffer,
                                      size_t bufSize);

/**
 * Sets the value for the specified entry.
 * If not existing, it automatically creates the section and/or key,
 * if existsing, it overwrites the value.
 *
 * @param file The Inifile Object
 * @param section The destination Section name
 * @param key The Value's key name
 * @param type The passed datatype
 * @param buffer Buffer containing the data
 *
 * @return 0 on success, negative value on error.
 */
extern int IniFile_CreateEntry_SetValue(Inifile file,
                                        const char *section,
                                        const char *key,
                                        EIniDataType type,
                                        const void *buffer);

/**
 * Prints all contents of the object to stdout,
 * used for testing purposes.
 *
 * @param file The Inifile Object
 */
extern void IniFile_DumpContent(Inifile file);


#ifdef __cplusplus
  }
#endif

#endif /* INIFILE_H_INCLUDED */
