#ifndef INIFILE_H_INCLUDED
  #define INIFILE_H_INCLUDED

#ifdef __cplusplus
  extern "C" {
#endif

#include "datatypes.h"

typedef struct TagIniFile*    Inifile;
typedef struct TagIniSection* IniSection;
typedef struct TagIniKey*     IniKey;

enum Options
{
  INI_OPT_CASE_SENSITIVE         =0x1,
  INI_OPT_IGNORE_MALFORMED_LINES =0x2,
  INI_OPT_ALLOW_DOUBLE_ENTRIES   =0x4,
  INI_OPT_ALLOW_MULTILINE        =0x8,
};

enum ErrorCodes
{
  INI_ERR_NONE=0,
  INI_ERR_PARAM_INVALID,
  INI_ERR_MEMORY_ALLOC,
  INI_ERR_FOPEN,
  INI_ERR_READ_MALFORMED,
  INI_ERR_READ_QUOTES_INVALID,
  INI_ERR_READ_IO,
  INI_ERR_WRITE_IO,
  INI_ERR_ESC_SEQ_INVALID,
  INI_ERR_FIND_NONE,
  INI_ERR_FIND_SECTION,
  INI_ERR_SECTIONNAME_DUPLICATE,
  INI_ERR_SECTIONNAME_INVALID,
  INI_ERR_KEYNAME_DUPLICATE,
  INI_ERR_KEYNAME_INVALID,
  INI_ERR_KEYVAL_INVALID,
  INI_ERR_DATATYPE_UNKNOWN,
  INI_ERR_BUFFER_TOO_SMALL,
  INI_ERR_CONVERSION,
};

/* Define this to print errors to stderr */
#define INIFILE_PRINT_ERRORS

/**
 * Creates a new, empty Inifile object.
 *
 * @param newFile _IN_ Pointer to Inifile, will get initialised.
 *
 * @return INI_ERR_NONE on success, on Error: <br>
 *         INI_ERR_PARAM_INVALID  <br>
 *         INI_ERR_MEMORY_ALLOC
 */
extern int IniFile_New(Inifile *newFile,
                       unsigned int options);

/**
 * Reads the file from specified path. If file already contains data, they'll be discarded before reading.
 * On read error, all contents of file (the memory object) will be reset to "zero" state.
 *
 * @param file   _IN_ The Inifile Object
 * @param path   _IN_ Path to file
 *
 * @return INI_ERR_NONE on success, on Error: <br>
 *         INI_ERR_PARAM_INVALID       <br>
 *         INI_ERR_MEMORY_ALLOC        <br>
 *         INI_ERR_FOPEN               <br>
 *         INI_ERR_READ_MALFORMED      <br>
 *         INI_ERR_READ_QUOTES_INVALID <br>
 *         INI_ERR_READ_IO             <br>
 *         INI_ERR_SECTIONNAME_INVALID <br>
 *         INI_ERR_KEYNAME_INVALID     <br>
 *         INI_ERR_KEYVAL_INVALID
 */
extern int IniFile_Read(Inifile file,
                        const char *path);

/**
 * Reads the file from specified path and returns a new Inifile Object with its content.
 *
 * @param file   _IN_ The Inifile Object
 * @param path   _IN_ Destination path
 *
 * @return INI_ERR_NONE on success, on Error: <br>
 *         INI_ERR_PARAM_INVALID  <br>
 *         INI_ERR_FOPEN          <br>
 *         INI_ERR_WRITE_IO
 */
extern int IniFile_Write(Inifile file,
                         const char *path);

/**
 * Deletes all Keys/sections from the Inifile.
 * For deleting single entries, refer to IniFile_DeleteEntry Functions.
 *
 * @param file   _IN_ The Inifile Object
 */
extern void IniFile_Clean(Inifile file);

/**
 * Delete the Inifile Object (Not the file itself). <br>
 * Don't use the Inifile Object anymore after that.
 *
 * @param file   _IN_ The Inifile Object
 */
extern void IniFile_Dispose(Inifile file);

/**
 * Finds a specific entry using the section and the key, returns the containing data. <br>
 * It tries to convert data to the specified type.
 *
 * @param file     _IN_     The Inifile Object
 * @param section  _IN_OPT_ The Section name to search for, NULL to look in unnamed section.
 * @param key      _IN_     The Value's key name
 * @param ptagData _OUT_    Pointer to TagData-Struct, Datatype and size must be specified!
 *
 * @return INI_ERR_NONE If key was found and conversion was successful, on Error: <br>
 *         INI_ERR_PARAM_INVALID    <br>
 *         INI_ERR_FIND_NONE        <br>
 *         INI_ERR_FIND_SECTION     <br>
 *         INI_ERR_DATATYPE_UNKNOWN <br>
 *         INI_ERR_CONVERSION       <br>
 *         INI_ERR_BUFFER_TOO_SMALL
 */
extern int IniFile_FindEntry_GetValue(Inifile file,
                                      const char *section,
                                      const char *key,
                                      TagData *ptagData);

/**
 * Sets the value for the specified entry. <br>
 * If not existing, it automatically creates the section and/or key, <br>
 * if existsing, it overwrites the value.
 *
 * @param file     _IN_     The Inifile Object
 * @param section  _IN_OPT_ The destination Section name, NULL to use the unnamed section
 * @param key      _IN_     The Value's key name
 * @param ptagData _IN_     Pointer to Filled TagData-Struct
 *
 * @return INI_ERR_NONE If key was created/set successful, on Error: <br>
 *         INI_ERR_PARAM_INVALID       <br>
 *         INI_ERR_MEMORY_ALLOC        <br>
 *         INI_ERR_SECTIONNAME_INVALID <br>
 *         INI_ERR_KEYNAME_INVALID     <br>
 *         INI_ERR_KEYVAL_INVALID      <br>
 *         INI_ERR_DATATYPE_UNKNOWN
 */
extern int IniFile_CreateEntry_SetValue(Inifile file,
                                        const char *section,
                                        const char *key,
                                        const TagData *ptagData);


/**
 * Delete the specified key and its value from the Inifile object.
 *
 * @param file    _IN_ The Inifile Object
 * @param section _IN_ Section where the key belongs to
 * @param key     _IN_ The key Name to delete
 *
 * @return INI_ERR_NONE if Key was found/deleted successful, on Error: <br>
 *         INI_ERR_PARAM_INVALID <br>
 *         INI_ERR_FIND_NONE     <br>
 *         INI_ERR_FIND_SECTION
 */
extern int IniFile_DeleteEntry_Key(Inifile file,
                                   const char *section,
                                   const char *key);

/**
 * Delete the specified section (And all containing keys).
 *
 * @param file    _IN_ The Inifile Object
 * @param section _IN_ The section name to delete
 *
 * @return INI_ERR_NONE if Section was found/deleted successful, on Error: <br>
 *         INI_ERR_PARAM_INVALID <br>
 *         INI_ERR_FIND_NONE
 */
extern int IniFile_DeleteEntry_Section(Inifile file,
                                       const char *section);

/**
 * Prints all contents of the object to stdout,
 * used for testing purposes.
 *
 * @param file   _IN_ The Inifile Object
 */
extern void IniFile_DumpContent(const Inifile file);


#ifdef __cplusplus
  }
#endif

#endif /* INIFILE_H_INCLUDED */
