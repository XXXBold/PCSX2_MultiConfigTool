#ifndef INIFILE_H_INCLUDED
  #define INIFILE_H_INCLUDED

#ifdef __cplusplus
  extern "C" {
#endif

#include <limits.h>
#include "datatypes.h"

typedef struct TagIniFile*    Inifile;
typedef struct TagIniSection* IniSection;
typedef struct TagIniKey*     IniKey;

/**
 * Available Options to use for the current inifile
 */
enum Options
{
  /**
   * Section/Key names are handled case-sensitive (default: Non-case sensitive)
   */
  INI_OPT_CASE_SENSITIVE         =0x1,
  /**
   * Will ignore malformed lines when reading a .ini-File.
   */
  INI_OPT_IGNORE_MALFORMED_LINES =0x2,
  /**
   * Must be set, in order to allow double entries (Sections, and keys in the same section) in a file.
   * For Sections: If the same sectionname is defined multiple times, they're merged together.
   * For Keys: Will keep the value of the last key with the same name in a section.
   */
  INI_OPT_ALLOW_DOUBLE_ENTRIES   =0x4,
  /**
   * Allows Multiline values for keys. For this, a line must end with "... \".
   * This option is experimental and should be used with care for now.
   */
  INI_OPT_ALLOW_MULTILINE        =0x8,
};

/**
 * These are possible error codes, returned from public Inifile_ Functions. <br>
 * To get an appropriate Errortext, use IniFile_GetErrorText(). <br>
 * For extended information, make sure INIFILE_PRINT_ERRORS is defined, to print errors within this library.
 */
enum ErrorCodes
{
  /**
   * No Error, success
   */
  INI_ERR_NONE=0,
  /**
   * Parameter passed by the user was invalid (e.g. datatype mismatch, buffer too small, ...)
   */
  INI_ERR_PARAM,
  /**
   * Occurs if data is requested, but the Key is empty.
   */
  INI_ERR_DATA_EMPTY,
  /**
   * Conversion to specified datatype failed
   */
  INI_ERR_CONVERSION,
  /**
   * File Read error: <br>
   * The file contains malformed lines (Use option INI_OPT_IGNORE_MALFORMED_LINES to ignore invalid lines).
   */
  INI_ERR_MALFORMED,
  /**
   * File read or write error: <br>
   * I/O Error occured while reading or writing file (e.g. fopen failed)
   */
  INI_ERR_IO,
  /**
   * Internal Error occured, such as memory allocation failure etc.
   * This error should not occur and might be reported.
   */
  INI_ERR_INTERNAL,
  /**
   * Used when searching for section/keys: <br>
   * Nor section nor key was found.
   */
  INI_ERR_FIND_NONE,
  /**
   * Used when searching for section/keys: <br>
   * Only sectionname was found, but not the key.
   */
  INI_ERR_FIND_SECTION,
};

/**
 * Predefined Numbers, used in the Iterator-Functions.
 */
enum Iterator
{
  /**
   * The first Element (Key or Section)
   */
  ITERATOR_FIRST = 0,
  /**
   * The last Element (Key or Section)
   */
  ITERATOR_LAST  = UINT_MAX
};

/**
 * Define this to print errors to stderr
 */
#define INIFILE_PRINT_ERRORS

/**
 * Creates a new, empty Inifile object.
 *
 * @param newFile _OUT_ Pointer to Inifile, will be initialized.
 * @param path    _IN_ Path where to load/store the inifile to. Can be changed using IniFile_SetPath(). For Maxiumum length of the path, see INI_MAX_PATHLEN value in .c-File.
 * @param options _IN_ Options for the new inifile object, see enum Options above.
 *
 * @return INI_ERR_NONE on success, on Error: <br>
 *         INI_ERR_PARAM         <br>
 *         INI_ERR_INTERNAL
 */
extern int IniFile_New(Inifile *newFile,
                       const char *path,
                       unsigned int options);

/**
 * Delete the Inifile Object (Not the file itself). <br>
 * Don't use the Inifile Object anymore after that.
 *
 * @param file   _IN_ The Inifile Object
 */
extern void IniFile_Dispose(Inifile file);


/**
 * Changes the path for the inifile.
 *
 * @param file   _IN_ The Inifile Object
 * @param path   _IN_ New Path to set
 *
 * @return INI_ERR_NONE on success, on Error: <br>
 *         INI_ERR_PARAM         <br>
 */
extern int Inifile_SetPath(Inifile file,
                           const char *path);
/**
 * Reads the file from specified path. If file already contains data, they'll be discarded before reading.
 * On read error, all contents of file (the memory object) will be reset to "zero" state.
 *
 * @param file   _IN_ The Inifile Object
 *
 * @return INI_ERR_NONE on success, on Error: <br>
 *         INI_ERR_PARAM         <br>
 *         INI_ERR_IO            <br>
 *         INI_ERR_INTERNAL      <br>
 *         INI_ERR_MALFORMED
 */
extern int IniFile_Read(Inifile file);

/**
 * Writes the file to the specified path.
 *
 * @param file   _IN_ The Inifile Object
 *
 * @return INI_ERR_NONE on success, on Error: <br>
 *         INI_ERR_PARAM         <br>
 *         INI_ERR_IO            <br>
 *         INI_ERR_INTERNAL
 */
extern int IniFile_Write(Inifile file);

/**
 * Deletes all Keys and Sections from the Inifile Object. <br>
 * For deleting single entries, refer to functions below. <br>
 * This won't modify the current file on disk, to delete it, refer to IniFile_Remove().
 *
 * @param file   _IN_ The Inifile Object
 */
extern void IniFile_Clean(Inifile file);

/**
 * Removes the inifile from disk (if existing). <br>
 * This won't modify the current data from the Inifile object, to delete these, refer to IniFile_Clean().
 *
 * @param file   _IN_ The Inifile Object
 *
 * @return INI_ERR_NONE If file was deleted successfully or if it didn't exist yet, on Error: <br>
 *         INI_ERR_IO
 */
extern int IniFile_Remove(Inifile file);

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
 *         INI_ERR_PARAM         <br>
 *         INI_ERR_FIND_NONE     <br>
 *         INI_ERR_FIND_SECTION  <br>
 *         INI_ERR_DATA_EMPTY    <br>
 *         INI_ERR_CONVERSION    <br>
 *         INI_ERR_INTERNAL
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
 * @return INI_ERR_NONE If key was created/ value set successful, on Error: <br>
 *         INI_ERR_PARAM         <br>
 *         INI_ERR_INTERNAL      <br>
 */
extern int IniFile_CreateEntry_SetValue(Inifile file,
                                        const char *section,
                                        const char *key,
                                        const TagData *ptagData);

/**
 * Compare two Strings (usually Section or Keyname) according to the current Case Sensitive setting for the inifile.
 * This functions works as the strcmp() functions.
 *
 * @param file   _IN_ The Inifile Object
 * @param str1   _IN_ 1st String to compare
 * @param str2   _IN_ 2nd String to compare
 *
 * @return As strcmp(), 0 if Strings are equal, < 0 = Str1 is less than Str2, > 0 Str2 is less then Str1.
 */
extern int IniFile_StringCompare(const Inifile file,
                                 const char *str1,
                                 const char *str2);

/**
 * Iterator function, used to manually walk through the inifile data. <br>
 * Finds the first section, should usually return an empty string ("").
 *
 * @param file     _IN_ The Inifile Object
 * @param position _IN_ Position to set the Iterator to, zero-based. Use ITERATOR_FIRST to go to the first Section, <br>
 *                 and ITERATOR_LAST to select the last Section, or a custom index value.
 *
 * @return Name of first Section ("" empty string, usually).
 */
extern const char *IniFile_Iterator_SetSectionIndex(Inifile file,
                                                    unsigned int position);

/**
 * Iterator function, used to manually walk through the inifile data. <br>
 * Find next section, if present.
 *
 * @param file   _IN_ The Inifile Object
 *
 * @return Name of the next Section, NULL if they're no more sections.
 */
extern const char *IniFile_Iterator_NextSection(Inifile file);

/**
 * Iterator function, used to manually walk through the inifile data. <br>
 * Finds the First key, within the current section, if present.
 *
 * @param file     _IN_ The Inifile Object
 * @param position _IN_ Position to set the Iterator to, zero-based. Use ITERATOR_FIRST to go to the first Key, <br>
 *                 and ITERATOR_LAST to select the last Key, or a custom index value.
 *
 * @return Name of first Key, NULL if the Section doesn't contain keys yet.
 */
extern const char *IniFile_Iterator_SetKeyIndex(Inifile file,
                                                unsigned int position);

/**
 * Iterator function, used to manually walk through the inifile data. <br>
 * Finds the next key within the current Section, if present.
 *
 * @param file   _IN_ The Inifile Object
 *
 * @return Name of next Key, NULL if they're no more keys.
 */
extern const char *IniFile_Iterator_NextKey(Inifile file);

/**
 * Iterator function, used to manually walk through the inifile data. <br>
 * Tries to find the specified section and sets the iterator on it if found.
 *
 * @param file    _IN_ The Inifile Object
 * @param section _IN_ The section to search for
 *
 * @return INI_ERR_NONE If section was found, on Error: <br>
 *         INI_ERR_PARAM         <br>
 *         INI_ERR_FIND_NONE
 */
extern int IniFile_Iterator_FindSection(Inifile file,
                                        const char *section);

/**
 * Iterator function, used to manually walk through the inifile data. <br>
 * Tries to find the specified key in the current section, sets iterator on it if found.
 *
 * @param file   _IN_ The Inifile Object
 * @param key    _IN_ The keyname to search for
 *
 * @return INI_ERR_NONE If key was found, on Error: <br>
 *         INI_ERR_PARAM         <br>
 *         INI_ERR_FIND_SECTION
 */
extern int IniFile_Iterator_FindKey(Inifile file,
                                    const char *key);

/**
 * Iterator function, used to manually walk through the inifile data. <br>
 * Sets the specified value to the current key. Pass NULL to set an empty value.
 *
 * @param file     _IN_ The Inifile Object
 * @param ptagData _IN_ Data to set for the current key, or NULL to set an empty value.
 *
 * @return INI_ERR_NONE If the vlaue could be set, on Error: <br>
 *         INI_ERR_PARAM       <br>
 *         INI_ERR_INTERNAL
 */
extern int IniFile_Iterator_KeySetValue(Inifile file,
                                        const TagData *ptagData);

/**
 * Iterator function, used to manually walk through the inifile data. <br>
 * Gets the Value from the current key.
 *
 * @param file     _IN_ The Inifile Object
 * @param ptagData _OUT_ Data retreived from the current key
 *
 * @return INI_ERR_NONE If the value was retreived succesful, on Error: <br>
 *         INI_ERR_PARAM       <br>
 *         INI_ERR_DATA_EMPTY  <br>
 *         INI_ERR_CONVERSION  <br>
 *         INI_ERR_INTERNAL
 */
extern int IniFile_Iterator_KeyGetValue(const Inifile file,
                                        TagData *ptagData);

/**
 * Iterator function, used to manually walk through the inifile data. <br>
 * Creates a new Section, which is added after the current iterator. <br>
 * After successful creation, iterator is placed on the new section.
 *
 * @param file    _IN_ The Inifile Object
 * @param section _IN_ Name for new section to create
 *
 * @return INI_ERR_NONE If section was created succesful, on Error: <br>
 *         INI_ERR_PARAM       <br>
 *         INI_ERR_INTERNAL
 */
extern int IniFile_Iterator_CreateSection(Inifile file,
                                          const char *section);

/**
 * Iterator function, used to manually walk through the inifile data. <br>
 * Creates a new key, which is added after the current iterator. <br>
 * After successful creation, iterator is placed on the new section.
 *
 * @param file   _IN_ The Inifile Object
 * @param key    _IN_ Name for new Key to create.
 *
 * @return INI_ERR_NONE If the key was created successful, on Error: <br>
 *         INI_ERR_PARAM       <br>
 *         INI_ERR_INTERNAL
 */
extern int IniFile_Iterator_CreateKey(Inifile file,
                                      const char *key);

/**
 * Iterator function, used to manually walk through the inifile data. <br>
 * Delete the current section (And all keys it contains). <br>
 * If the current section is the first, just all keys will be deleted.
 *
 * @param file   _IN_ The Inifile Object
 */
extern void IniFile_Iterator_DeleteSection(Inifile file);

/**
 * Iterator function, used to manually walk through the inifile data. <br>
 * Deletes the key at the current iterator.
 *
 * @param file   _IN_ The Inifile Object
 */
extern void IniFile_Iterator_DeleteKey(Inifile file);

/**
 * Delete the specified key and its value from the Inifile object.
 * On successful deletion, the internal iterator is set to the previous Key of the deleted one. <br>
 * On Error, iterator state is undefined.
 *
 * @param file    _IN_ The Inifile Object
 * @param section _IN_ Section where the key belongs to
 * @param key     _IN_ The key Name to delete
 *
 * @return INI_ERR_NONE if Key was found/deleted successful, on Error: <br>
 *         INI_ERR_PARAM         <br>
 *         INI_ERR_FIND_NONE     <br>
 *         INI_ERR_FIND_SECTION
 */
extern int IniFile_DeleteEntry_Key(Inifile file,
                                   const char *section,
                                   const char *key);

/**
 * Delete the specified section (And all containing keys). <br>
 * On successful deletion, the internal Iterator is set to the previous Section of the deleted one. <br>
 * If the first Section should be deleted, pass NULL. <br>
 * The First section won't be really deleted, because it's always used, <br>
 * just all keys it contains will be removed. Iterator will be set on first Section aswell.
 *
 * @param file    _IN_ The Inifile Object
 * @param section _IN_ The section name to delete
 *
 * @return INI_ERR_NONE if Section was found/deleted successful, on Error: <br>
 *         INI_ERR_PARAM         <br>
 *         INI_ERR_FIND_NONE
 */
extern int IniFile_DeleteEntry_Section(Inifile file,
                                       const char *section);


/**
 * Returns an Error text describing the Errorcode.
 *
 * @param error  Errorcode, received as a return value from an IniFile Function.
 *
 * @return Error text
 */
extern const char *IniFile_GetErrorText(int error);

/**
 * Prints all contents of the object to stdout,
 * used for testing purposes.
 *
 * @param file   _IN_ The Inifile Object
 * @param fp     _IN_ Stream where to print the infile dump (e.g. stdout, stderr)
 */
extern void IniFile_DumpContent(const Inifile file,
                                FILE *fp);


#ifdef __cplusplus
  }
#endif

#endif /* INIFILE_H_INCLUDED */
