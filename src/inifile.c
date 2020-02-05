#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <limits.h>

#include "inifile.h"

#if defined (__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) /* >= C99 */
  #define INLINE_FCT inline
  #define INLINE_PROT static inline
#else /* No inline available from C Standard */
  #define INLINE_FCT  static
  #define INLINE_PROT static
#endif /* __STDC_VERSION__ >= C99 */

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h> /* For DeleteFile() function */
  #define STRCMP_CIS(str1,str2) _stricmp(str1,str2)
#elif __linux__
  #include <unistd.h> /* For unlink() function */
  #if defined (_POSIX_C_SOURCE) && (_POSIX_C_SOURCE>=200112L)
    #include <strings.h>
    #define STRCMP_CIS(str1,str2) strcasecmp(str1,str2)
  #else
    #define STRCMP_CIS(str1,str2) Ini_stricmp(str1,str2)
  #endif
#else
 #error Platform not implemented
#endif /* _WIN32 */
#define STRCMP_CS(str1,str2)  strcmp(str1,str2)
#define IS_OCT_DIGIT(ch) ((isdigit(ch)) && (ch<'8'))

#define KEYS_REALLOC(ptr,newCount)     realloc((ptr),(newCount)*sizeof(struct TagIniKey))
#define SECTIONS_REALLOC(ptr,newCount) realloc((ptr),(newCount)*sizeof(struct TagIniSection))
#define SECTIONS_NEED_REALLOC(file)    (((file)->uiSectionsCount<(file)->uiSectionsMax)?0:1)
#define KEYS_NEED_REALLOC(section)     (((section)->uiKeysCount<(section)->uiKeysMax)?0:1)

#define SECTION_CALC_INDEX(file) (((file)->ptagCurrSection-(file)->ptagSections)/sizeof(struct TagIniSection*))
#define KEY_CALC_INDEX(file)     (((file)->ptagCurrSection->ptagCurrKey-(file)->ptagCurrSection->ptagKeys)/sizeof(struct TagIniKey*))

#define CHECK_SECTIONNAME_VALID(str) Ini_CheckNameValid_m(str,MAX_SECTION_LENGTH-1)
#define CHECK_KEYNAME_VALID(str)     Ini_CheckNameValid_m(str,MAX_KEYNAME_LENGTH-1)

#define OPTION_CHK(ini,opt) ((ini)->uiOptions&opt)
#define INI_STRCMP(ini,str1,str2) (OPTION_CHK(ini,INI_OPT_CASE_SENSITIVE)?STRCMP_CS(str1,str2):STRCMP_CIS(str1,str2))

#ifdef INIFILE_PRINT_ERRORS
  #define ERR_PRINT(str)           fputs("INI Error: " str "\n",stderr)
  #define ERR_PRINT_VARGS(str,...) fprintf(stderr,"INI Error: " str "\n",__VA_ARGS__)
#else
  #define ERR_PRINT(str)
  #define ERR_PRINT_VARGS(str,...)
#endif /* INIFILE_PRINT_ERRORS */

enum
{
  INITIAL_SECTION_COUNT             = 10,
  INITIAL_KEYS_PER_SECTION_COUNT    = 10,
  MAX_SECTION_LENGTH                = 80,
  MAX_KEYNAME_LENGTH                = MAX_SECTION_LENGTH,
  MAX_KEYVAL_LENGTH                 = 300,

  INI_MAX_PATHLEN                   = 260,

  /* Internal used return values */
  INI_CONTINUE_ON_NEXT_LINE         = 100,
  INI_ERR_MEMORY_ALLOC,
  INI_ERR_FOPEN,
  INI_ERR_READ_QUOTES_INVALID,
  INI_ERR_ESC_SEQ_INVALID,
  INI_ERR_SECTIONNAME_DUPLICATE,
  INI_ERR_SECTIONNAME_INVALID,
  INI_ERR_KEYNAME_DUPLICATE,
  INI_ERR_KEYNAME_INVALID,
  INI_ERR_KEYVAL_INVALID,
  INI_ERR_DATATYPE_UNKNOWN,
  INI_ERR_BUFFER_TOO_SMALL,
};

enum chars
{
  SECTION_NAME_BEGIN        = '[',
  SECTION_NAME_END          = ']',
  COMMENT_INDICATOR_1       = ';',
  COMMENT_INDICATOR_2       = '#',

  CHAR_EQUAL1               = '=',
  CHAR_EQUAL2               = ':',

  CHAR_NEWLINE_1            = '\n',
  CHAR_NEWLINE_2            = '\r',

  CHAR_A                    = 'a',  /* 'a',  Bell (alert) */
  CHAR_B                    = 'b',  /* 'b',  backspace */
  CHAR_F                    = 'f',  /* 'f',  Form feed */
  CHAR_N                    = 'n',  /* 'n',  new line */
  CHAR_R                    = 'r',  /* 'r',  carriage return */
  CHAR_T                    = 't',  /* 't',  horizontal tab */
  CHAR_V                    = 'v',  /* 'v',  vertical tab */
  CHAR_SQ                   = '\'', /* '\'', Single quotation mark */
  CHAR_DQ                   = '\"', /* '\"', Double quotation mark */
  CHAR_BS                   = '\\', /* '\\', Backslash */
  CHAR_QUES                 = '?',  /* '?',  Question mark  */
  CHAR_O                    = 'o',  /* 'o',  ASCII octal notation */
  CHAR_X                    = 'x',  /* 'x',  ASCII Hexadecimal notation */
  CHAR_0                    = '\0',  /* '\0'  continue on next line */

  ESC_SEQ_CHAR_A            = '\a',
  ESC_SEQ_CHAR_B            = '\b',
  ESC_SEQ_CHAR_F            = '\f',
  ESC_SEQ_CHAR_N            = '\n',
  ESC_SEQ_CHAR_R            = '\r',
  ESC_SEQ_CHAR_T            = '\t',
  ESC_SEQ_CHAR_V            = '\v',
  ESC_SEQ_CHAR_SQ           = '\'',
  ESC_SEQ_CHAR_DQ           = '\"',
  ESC_SEQ_CHAR_BS           = '\\',
  ESC_SEQ_CHAR_QUES         = '\?',
  ESC_SEQ_CHAR_0            = '\0',
};

struct TagIniKey
{
  char caKeyName[MAX_KEYNAME_LENGTH+1];
  unsigned char ucaData[MAX_KEYVAL_LENGTH];
  unsigned int uiDataSize;
  EDataType type;
};

struct TagIniSection
{
  unsigned int uiKeysCount;
  unsigned int uiKeysMax;
  char caSectionName[MAX_SECTION_LENGTH+1];
  struct TagIniKey *ptagKeys;
  struct TagIniKey *ptagCurrKey;
};

struct TagIniFile
{
  char caPath[INI_MAX_PATHLEN+1];
  char *caTmpBuffer;
  unsigned int uiBufSize;
  unsigned int uiOptions;
  unsigned int uiSectionsCount;
  unsigned int uiSectionsMax;
  struct TagIniSection *ptagSections;
  struct TagIniSection *ptagCurrSection;
};


INLINE_PROT void Ini_ResetContent_m(Inifile file);
INLINE_PROT int Ini_OptionsValid_m(unsigned int uiOptions);

INLINE_PROT int Ini_stricmp(const char *pcStr1,
                            const char *pcStr2);

INLINE_PROT void Ini_RemoveLeadingWhitespaces_m(char **ppcString);
INLINE_PROT unsigned int Ini_RemoveFollowingWhitespaces_m(char *pcString,
                                                          unsigned int uiLength);

INLINE_PROT char *Ini_FindEndOfLine_m(char **ppcLine,
                                      unsigned int *puiLength);

INLINE_PROT int Ini_UnEscapeString_m(const char *pcStrIn,
                                     unsigned char *pucStrOut,
                                     unsigned int *puiDataSize,
                                     EDataType *pDataType);

INLINE_PROT int Ini_EscapeData_m(const unsigned char *pucDataIn,
                                 unsigned int uiDataInSize,
                                 char *pcOutText,
                                 unsigned int *puiOutBufSize,
                                 EDataType dataType);

INLINE_PROT int Ini_CheckNameValid_m(const char *pcName,
                                     unsigned int uiMaxLen);

INLINE_PROT int Ini_CreateNewSection_m(Inifile tagfile,
                                       const char *pcSection);
INLINE_PROT int Ini_CreateNewKey_m(Inifile file,
                                   const char *pcKey);

INLINE_PROT int Ini_KeySetData_m(Inifile file,
                                 const TagData *ptagData);

INLINE_PROT int Ini_FindSection_m(Inifile file,
                                  const char *pcSection);
INLINE_PROT int Ini_FindKey_m(Inifile file,
                              const char *pcKey);

INLINE_PROT int Ini_Read_Section_m(char *pcLine,
                                   unsigned int uiLength);
INLINE_PROT int Ini_Read_Key_m(Inifile tagfile,
                               char **ppcNextLine,
                               char *pcLine,
                               unsigned int uiLength);
INLINE_PROT int Ini_Read_Comment_m(char *pcLine);

INLINE_PROT int Ini_ConvertValue_m(IniKey ptagKey,
                                   TagData *ptagData);

static const char *pcaBooleanStringsTrue_m[]= {"true", "enabled", "1","y","t"};
static const char *pcaBooleanStringsFalse_m[]={"false","disabled","0","n","f"};


int IniFile_New(Inifile *newFile,
                const char *path,
                unsigned int options)
{
  unsigned int uiLen=strlen(path)+1;
  if((Ini_OptionsValid_m(options)) || (uiLen > INI_MAX_PATHLEN))
    return(INI_ERR_PARAM);

  if(!(*newFile=malloc(sizeof(struct TagIniFile))))
    return(INI_ERR_INTERNAL);

  (*newFile)->uiBufSize=(MAX_KEYNAME_LENGTH+MAX_KEYVAL_LENGTH)*5;
  if(!((*newFile)->caTmpBuffer=malloc((*newFile)->uiBufSize)))
  {
    free(*newFile);
    return(INI_ERR_INTERNAL);
  }

  if(!((*newFile)->ptagSections=SECTIONS_REALLOC(NULL,INITIAL_SECTION_COUNT)))
  {
    free((*newFile)->caTmpBuffer);
    free(*newFile);
    return(INI_ERR_INTERNAL);
  }
  if(!((*newFile)->ptagSections[0].ptagKeys=KEYS_REALLOC(NULL,INITIAL_KEYS_PER_SECTION_COUNT)))
  {
    free((*newFile)->ptagSections);
    free((*newFile)->caTmpBuffer);
    free(*newFile);
    return(INI_ERR_INTERNAL);
  }
  (*newFile)->uiOptions=options;
  (*newFile)->uiSectionsCount=1;
  (*newFile)->uiSectionsMax=INITIAL_SECTION_COUNT;
  (*newFile)->ptagCurrSection=(*newFile)->ptagSections;
  (*newFile)->ptagSections[0].caSectionName[0]='\0';
  (*newFile)->ptagSections[0].uiKeysCount=0;
  (*newFile)->ptagSections[0].uiKeysMax=INITIAL_KEYS_PER_SECTION_COUNT;
  (*newFile)->ptagSections[0].ptagCurrKey=(*newFile)->ptagSections[0].ptagKeys;
  memcpy((*newFile)->caPath,path,uiLen);
  return(INI_ERR_NONE);
}

void IniFile_Dispose(Inifile file)
{
  unsigned int uiIndexSection;

  if(!file) /* Validate param for not being NULL */
    return;

  for(uiIndexSection=0;uiIndexSection < file->uiSectionsCount;++uiIndexSection)
    free(file->ptagSections[uiIndexSection].ptagKeys);
  free(file->ptagSections);
  free(file->caTmpBuffer);
  free(file);
}

int Inifile_SetPath(Inifile file,
                    const char *path)
{
  unsigned int uiLen=strlen(path)+1;
  if((!file) || (uiLen > INI_MAX_PATHLEN))
    return(INI_ERR_PARAM);

  memcpy(file->caPath,path,uiLen);
  return(INI_ERR_NONE);
}

int IniFile_Read(Inifile file)
{
  FILE *fp;
  char *pcNextLine;
  char *pcLine;
  char *pcTmp;
  int iRc=INI_ERR_NONE;
  unsigned int uiLength;

  if(!file) /* Validate params for not being NULL */
    return(INI_ERR_PARAM);

  errno=0;
  /* Open files as binary, to stay compatible with all platforms */
  if(!(fp=fopen(file->caPath,"rb")))
  {
    ERR_PRINT_VARGS("fopen() failed for file \"%s\" (%d): %s",file->caPath,errno,strerror(errno));
    return(INI_ERR_IO);
  }

  fseek(fp,0,SEEK_END);
  uiLength=ftell(fp)+1;
  rewind(fp);
  if(uiLength < 2) /* Empty file, okay, don't read it then */
  {
    fclose(fp);
    return(INI_ERR_NONE);
  }

  if(uiLength>file->uiBufSize)
  {
    if(!(pcTmp=realloc(file->caTmpBuffer,uiLength)))
    {
      fclose(fp);
      return(INI_ERR_INTERNAL);
    }
    file->uiBufSize=uiLength;
    file->caTmpBuffer=pcTmp;
  }
  /* Read whole file at once */
  uiLength=fread(file->caTmpBuffer,1,uiLength,fp)+1;
  if(!feof(fp)) /* If EOF not reached, I/O Error occured */
  {
    fclose(fp);
    return(INI_ERR_IO);
  }
  file->caTmpBuffer[uiLength-1]='\0';
  fclose(fp);

  pcLine=file->caTmpBuffer;
  while(1)
  {
    pcNextLine=Ini_FindEndOfLine_m(&pcLine,&uiLength);
    switch(pcLine[0])
    {
      case SECTION_NAME_BEGIN:
        if((iRc=Ini_Read_Section_m(++pcLine,--uiLength)) != INI_ERR_NONE)
          break;

        iRc=Ini_CreateNewSection_m(file,pcLine);
        break;
      case COMMENT_INDICATOR_1:
      case COMMENT_INDICATOR_2:
        iRc=Ini_Read_Comment_m(pcLine);
        break;
      default:
        if((iRc=Ini_Read_Key_m(file,&pcNextLine,pcLine,uiLength)) != INI_ERR_NONE)
          break;
    }
    switch(iRc)
    {
      case INI_ERR_NONE: /* All okay, continue reading */
      case INI_CONTINUE_ON_NEXT_LINE:
        iRc=INI_ERR_NONE;
        break;
      case INI_ERR_MALFORMED:
      case INI_ERR_READ_QUOTES_INVALID:
      case INI_ERR_ESC_SEQ_INVALID:
      case INI_ERR_SECTIONNAME_INVALID:
      case INI_ERR_KEYNAME_INVALID:
      case INI_ERR_KEYVAL_INVALID:
        if(!OPTION_CHK(file,INI_OPT_IGNORE_MALFORMED_LINES))
        {
          iRc=INI_ERR_MALFORMED;
          pcNextLine=NULL; /* Will quit loop below */
          Ini_ResetContent_m(file);
        }
        else
          iRc=INI_ERR_NONE;
        break;
      default: /* Fatal error, can't be ignored */
        iRc=INI_ERR_INTERNAL;
        Ini_ResetContent_m(file);
        pcNextLine=NULL;
        break;
    }
    if(!pcNextLine) /* No more data */
      break;
    pcLine=pcNextLine;
  }
  return(iRc);
}

int IniFile_Write(Inifile file)
{
  FILE *fp;
  unsigned int uiSize;
  unsigned int uiIndexSection;
  unsigned int uiIndexKey;

  if(!file) /* Validate params for not being NULL */
    return(INI_ERR_PARAM);

  errno=0;
  if(!(fp=fopen(file->caPath,"w")))
  {
    ERR_PRINT_VARGS("fopen() failed for file \"%s\" (%d): %s",file->caPath,errno,strerror(errno));
    return(INI_ERR_IO);
  }

  for(uiIndexSection=0;uiIndexSection < file->uiSectionsCount;++uiIndexSection)
  {
    if(uiIndexSection) /* First section is unnamed */
    {
      /* Check return val for possible I/O Error */
      if(fprintf(fp,"[%s]\n",file->ptagSections[uiIndexSection].caSectionName) < 1)
      {
        fclose(fp);
        return(INI_ERR_IO);
      }
    }
    for(uiIndexKey=0;uiIndexKey < file->ptagSections[uiIndexSection].uiKeysCount;++uiIndexKey)
    {
      uiSize=file->uiBufSize;
      if(!file->ptagSections[uiIndexSection].ptagKeys[uiIndexKey].uiDataSize)
        file->caTmpBuffer[0]='\0';
      else if(Ini_EscapeData_m(file->ptagSections[uiIndexSection].ptagKeys[uiIndexKey].ucaData,
                               file->ptagSections[uiIndexSection].ptagKeys[uiIndexKey].uiDataSize,
                               file->caTmpBuffer,
                               &uiSize,
                               file->ptagSections[uiIndexSection].ptagKeys[uiIndexKey].type) != INI_ERR_NONE)
      {
        ERR_PRINT("Ini_EscapeData_m() failed, buffer too small, quit writing");
        fclose(fp);
        return(INI_ERR_INTERNAL);
      }
      /* Check return val for possible I/O Error */
      if(fprintf(fp,
                 "%s=%s\n",
                 file->ptagSections[uiIndexSection].ptagKeys[uiIndexKey].caKeyName,
                 file->caTmpBuffer) < 1)
      {
        fclose(fp);
        return(INI_ERR_IO);
      }
    }
  }
  fclose(fp);
  return(INI_ERR_NONE);
}

void IniFile_Clean(Inifile file)
{
  Ini_ResetContent_m(file);
}

int IniFile_Remove(Inifile file)
{
  errno=0;
  /* If file was removed successfully or doesn't exist, return no error */
#ifdef _WIN32
  if((DeleteFile(file->caPath)) || ((errno=GetLastError()) == ERROR_FILE_NOT_FOUND))
    return(INI_ERR_NONE);
  ERR_PRINT_VARGS("DeleteFile(\"%s\") failed (%d): %s",file->caPath,errno,strerror(errno));
#elif __linux__
  if((unlink(file->caPath) == 0) || (errno == ENOENT))
    return(INI_ERR_NONE);
  ERR_PRINT_VARGS("unlink(\"%s\") failed (%d): %s",file->caPath,errno,strerror(errno));
#endif
  return(INI_ERR_IO);
}

int IniFile_FindEntry_GetValue(Inifile file,
                               const char *section,
                               const char *key,
                               TagData *ptagData)
{
  if((!file) || (!key) || (!ptagData)) /* Validate params for not being NULL */
    return(INI_ERR_PARAM);

  if(!Ini_FindSection_m(file,section))
    return(INI_ERR_FIND_NONE);

  if(!Ini_FindKey_m(file,key))
    return(INI_ERR_FIND_SECTION);

  return(IniFile_Iterator_KeyGetValue(file,ptagData));
}

int IniFile_CreateEntry_SetValue(Inifile file,
                                 const char *section,
                                 const char *key,
                                 const TagData *ptagData)
{
  int iRc;

  if(!file) /* Validate params for not being NULL */
    return(INI_ERR_PARAM);

  /* Empty section: Always exists as first */
  if((!section) || (section[0]=='\0'))
    file->ptagCurrSection=&file->ptagSections[0];
  else
  {
    /* Check if section exist or create new if not */
    if((iRc=IniFile_Iterator_CreateSection(file,section)) != INI_ERR_NONE)
      return(iRc);
  }
  /* Check if key exists, create if not */
  if((iRc=IniFile_Iterator_CreateKey(file,key)) != INI_ERR_NONE)
    return(iRc);

  return(IniFile_Iterator_KeySetValue(file,ptagData));
}

int IniFile_StringCompare(const Inifile file,
                          const char *str1,
                          const char *str2)
{
  return(INI_STRCMP(file,str1,str2));
}

const char *IniFile_Iterator_SetSectionIndex(Inifile file,
                                             unsigned int position)
{
  file->ptagCurrSection=file->ptagSections +
                        ((position < file->uiSectionsCount)?position:file->uiSectionsCount-1);
  return(file->ptagCurrSection->caSectionName);
}

const char *IniFile_Iterator_NextSection(Inifile file)
{
  if(file->ptagCurrSection < file->ptagSections+file->uiSectionsCount-1)
  {
    ++file->ptagCurrSection;
    return(file->ptagCurrSection->caSectionName);
  }
  return(NULL);
}

const char *IniFile_Iterator_SetKeyIndex(Inifile file,
                                         unsigned int position)
{
  file->ptagCurrSection->ptagCurrKey=file->ptagCurrSection->ptagKeys +
                                     ((position < file->ptagCurrSection->uiKeysCount)?position:file->ptagCurrSection->uiKeysCount-1);

  return((file->ptagCurrSection->uiKeysCount)?file->ptagCurrSection->ptagCurrKey->caKeyName:NULL);
}

const char *IniFile_Iterator_NextKey(Inifile file)
{
  if(file->ptagCurrSection->ptagCurrKey < file->ptagCurrSection->ptagKeys+file->ptagCurrSection->uiKeysCount-1)
  {
    ++file->ptagCurrSection->ptagCurrKey;
    return(file->ptagCurrSection->ptagCurrKey->caKeyName);
  }
  return(NULL);
}

int IniFile_Iterator_FindSection(Inifile file,
                                 const char *section)
{
  if(!file)
    return(INI_ERR_PARAM);

  return((Ini_FindSection_m(file,section))?INI_ERR_NONE:INI_ERR_FIND_NONE);
}

int IniFile_Iterator_FindKey(Inifile file,
                             const char *key)
{
  if((!file) || (!key))
    return(INI_ERR_PARAM);

  return((Ini_FindKey_m(file,key))?INI_ERR_NONE:INI_ERR_FIND_SECTION);
}

int IniFile_Iterator_KeySetValue(Inifile file,
                                 const TagData *ptagData)
{
  int iRc;

  if(!file) /* Validate parameter, data may be NULL */
    return(INI_ERR_PARAM);

  switch((iRc=Ini_KeySetData_m(file,ptagData)))
  {
    case INI_ERR_NONE:
      break;
    case INI_ERR_CONVERSION:
    case INI_ERR_BUFFER_TOO_SMALL:
    case INI_ERR_DATATYPE_UNKNOWN:
      ERR_PRINT_VARGS("Ini_KeySetData_m() failed (%d): %s",iRc,IniFile_GetErrorText(iRc));
      iRc=INI_ERR_PARAM;
      break;
    default:
      ERR_PRINT_VARGS("Ini_KeySetData_m() failed (%d): %s",iRc,IniFile_GetErrorText(iRc));
      return(INI_ERR_INTERNAL);
  }
  return(iRc);
}

int IniFile_Iterator_KeyGetValue(const Inifile file,
                                 TagData *ptagData)
{
  int iRc;

  if((!file) || (!ptagData))
    return(INI_ERR_PARAM);

  if(((iRc=Ini_ConvertValue_m(file->ptagCurrSection->ptagCurrKey,ptagData)) != INI_ERR_NONE) &&
     (iRc != INI_ERR_DATA_EMPTY))
  {
    ERR_PRINT_VARGS("Ini_ConvertValue_m() failed (%d): %s",iRc,IniFile_GetErrorText(iRc));
    switch(iRc)
    {
      case INI_ERR_NONE:
      case INI_ERR_DATA_EMPTY:
      case INI_ERR_CONVERSION:
        break;
      case INI_ERR_BUFFER_TOO_SMALL:
      case INI_ERR_DATATYPE_UNKNOWN:
        iRc=INI_ERR_PARAM;
        break;
      default:
        iRc=INI_ERR_INTERNAL;
    }
  }
  return(iRc);
}

int IniFile_Iterator_CreateSection(Inifile file,
                                   const char *section)
{
  int iRc;
  if((!file) || (!section))
    return(INI_ERR_PARAM);

  switch((iRc=Ini_CreateNewSection_m(file,section)))
  {
    case INI_ERR_NONE:
    case INI_ERR_SECTIONNAME_DUPLICATE: /* Section already exists, ok */
      break;
    case INI_ERR_SECTIONNAME_INVALID:
      ERR_PRINT_VARGS("Ini_CreateNewSection_m() failed (%d): %s",iRc,IniFile_GetErrorText(iRc));
      return(INI_ERR_PARAM);
    case INI_ERR_MEMORY_ALLOC:
    default: /* Other error occured */
      ERR_PRINT_VARGS("Ini_CreateNewSection_m() failed (%d): %s",iRc,IniFile_GetErrorText(iRc));
      return(INI_ERR_INTERNAL);
  }
  return(INI_ERR_NONE);
}

int IniFile_Iterator_CreateKey(Inifile file,
                               const char *key)
{
  int iRc;
  if((!file) || (!key))
    return(INI_ERR_PARAM);

  switch((iRc=Ini_CreateNewKey_m(file,key)))
  {
    case INI_ERR_NONE:
    case INI_ERR_KEYNAME_DUPLICATE: /* Key already exists, ok */
      break;
    case INI_ERR_KEYNAME_INVALID:
      ERR_PRINT_VARGS("Ini_CreateNewKey_m() failed (%d): %s",iRc,IniFile_GetErrorText(iRc));
      return(INI_ERR_PARAM);
    case INI_ERR_MEMORY_ALLOC:
    default:
      ERR_PRINT_VARGS("Ini_CreateNewKey_m() failed (%d): %s",iRc,IniFile_GetErrorText(iRc));
      return(INI_ERR_INTERNAL);
  }
  return(INI_ERR_NONE);
}

void IniFile_Iterator_DeleteSection(Inifile file)
{
  unsigned int uiSectionIndex;
  /* Check if first section */
  if(file->ptagCurrSection == file->ptagSections)
  {
    file->ptagCurrSection->uiKeysCount=0;
    return;
  }

  uiSectionIndex=SECTION_CALC_INDEX(file);
  /* free Keys for the section */
  free(file->ptagCurrSection->ptagKeys);
  --file->uiSectionsCount;
  if(uiSectionIndex < file->uiSectionsCount-1)
  {
    memmove(file->ptagCurrSection,
            file->ptagCurrSection+1,
            (file->uiSectionsCount-uiSectionIndex)*sizeof(struct TagIniSection));
  }
  --file->ptagCurrSection;
}

void IniFile_Iterator_DeleteKey(Inifile file)
{
  unsigned int uiKeyIndex;
  /* Delete Key+value */
  uiKeyIndex=KEY_CALC_INDEX(file);
  --file->ptagCurrSection->uiKeysCount;
  if(uiKeyIndex < file->ptagCurrSection->uiKeysCount)
  {
    memmove(file->ptagCurrSection->ptagCurrKey,
            file->ptagCurrSection->ptagCurrKey+1,
            (file->ptagCurrSection->uiKeysCount-uiKeyIndex)*sizeof(struct TagIniKey));
  }
  if(file->ptagCurrSection->ptagCurrKey != file->ptagCurrSection->ptagKeys)
    --file->ptagCurrSection->ptagCurrKey;
}

int IniFile_DeleteEntry_Key(Inifile file,
                            const char *section,
                            const char *key)
{
  if((!file) || (!key)) /* Validate params for not being NULL */
    return(INI_ERR_PARAM);

  if(!Ini_FindSection_m(file,section))
    return(INI_ERR_FIND_NONE);

  if(!Ini_FindKey_m(file,key))
    return(INI_ERR_FIND_SECTION);

  IniFile_Iterator_DeleteKey(file);
  return(INI_ERR_NONE);
}

int IniFile_DeleteEntry_Section(Inifile file,
                                const char *section)
{
  if(!file) /* Validate params for not being NULL */
    return(INI_ERR_PARAM);

  if(!Ini_FindSection_m(file,section))
    return(INI_ERR_FIND_NONE);

  IniFile_Iterator_DeleteSection(file);
  return(INI_ERR_NONE);
}

const char *IniFile_GetErrorText(int error)
{
  switch(error)
  {
    /* External available error codes */
    case INI_ERR_NONE:
      return("No Error");
    case INI_ERR_PARAM:
      return("Invalid Parameter");
    case INI_ERR_DATA_EMPTY:
      return("Can't get Data, Key is empty");
    case INI_ERR_MALFORMED:
      return("File is malformed, can't read");
    case INI_ERR_IO:
      return("I/O Error occured");
    case INI_ERR_INTERNAL:
      return("Internal Error occured");
    case INI_ERR_FIND_NONE:
      return("Nor Section nor Keyname found");
    case INI_ERR_FIND_SECTION:
      return("Only Section found, but not Keyname");

    /* Internal error codes */
    case INI_ERR_MEMORY_ALLOC:
      return("Failed to allocate requested memory");
    case INI_ERR_FOPEN:
      return("Failed to open file");
    case INI_ERR_READ_QUOTES_INVALID:
      return("Invalid quotes in line");
    case INI_ERR_ESC_SEQ_INVALID:
      return("Invalid Escape sequence in line");
    case INI_ERR_SECTIONNAME_DUPLICATE:
      return("Duplicate Section, already exists");
    case INI_ERR_SECTIONNAME_INVALID:
      return("Invalid Sectionname");
    case INI_ERR_KEYNAME_DUPLICATE:
      return("Duplicate Keyname, already exists");
    case INI_ERR_KEYNAME_INVALID:
      return("Invalid Keyname");
    case INI_ERR_KEYVAL_INVALID:
      return("Invalid Keyvalue");
    case INI_ERR_DATATYPE_UNKNOWN:
      return("Unknown Datatype specified");
    case INI_ERR_BUFFER_TOO_SMALL:
      return("Buffer is too small");
    case INI_ERR_CONVERSION:
      return("Conversion to specified type failed");
    default:
      return("Unknown Error");
  }
}

void IniFile_DumpContent(const Inifile file,
                         FILE *fp)
{
  unsigned int uiSize;
  unsigned int uiIndexSection;
  unsigned int uiIndexKey;

  fputs("***Start Dumping Contents of Inifile***\n",fp);
  for(uiIndexSection=0;uiIndexSection < file->uiSectionsCount;++uiIndexSection)
  {
    if(uiIndexSection)
      fprintf(fp,
              "0x%p [%s]\n",
              file->ptagSections[uiIndexSection].caSectionName,
              file->ptagSections[uiIndexSection].caSectionName);
    for(uiIndexKey=0;uiIndexKey < file->ptagSections[uiIndexSection].uiKeysCount;++uiIndexKey)
    {
      uiSize=file->uiBufSize;
      /* Check if key is empty */
      if(!file->ptagSections[uiIndexSection].ptagKeys[uiIndexKey].uiDataSize)
        strcpy(file->caTmpBuffer,"(empty)");
      else if(Ini_EscapeData_m(file->ptagSections[uiIndexSection].ptagKeys[uiIndexKey].ucaData,
                               file->ptagSections[uiIndexSection].ptagKeys[uiIndexKey].uiDataSize,
                               file->caTmpBuffer,
                               &uiSize,
                               file->ptagSections[uiIndexSection].ptagKeys[uiIndexKey].type) != INI_ERR_NONE)
        return;
      fprintf(fp,
              "0x%p=0x%p %s=%s (types: 0x%.4X)\n",
              file->ptagSections[uiIndexSection].ptagKeys[uiIndexKey].caKeyName,
              file->ptagSections[uiIndexSection].ptagKeys[uiIndexKey].ucaData,
              file->ptagSections[uiIndexSection].ptagKeys[uiIndexKey].caKeyName,
              file->caTmpBuffer,
              file->ptagSections[uiIndexSection].ptagKeys[uiIndexKey].type);
    }
  }
  fputs("***End Dumping Contents of Inifile***\n",fp);
}

INLINE_FCT int Ini_UnEscapeString_m(const char *pcStrIn,
                                    unsigned char *pucStrOut,
                                    unsigned int *puiDataSize,
                                    EDataType *pType)
{
  unsigned int uiIndexOut=0;
  char caTmp[4];

  if(!*puiDataSize)
    return((pcStrIn[0] == '\0')?INI_ERR_NONE:INI_ERR_BUFFER_TOO_SMALL);

  *pType |= eDataType_Binary;
  while(*pcStrIn != '\0')
  {
    if(uiIndexOut > (*puiDataSize)-2)
        return(INI_ERR_BUFFER_TOO_SMALL);

    if(*pcStrIn != CHAR_BS)
    {
      *pType &= ~(eDataType_Binary);
      pucStrOut[uiIndexOut++]=*pcStrIn;
      ++pcStrIn;
      continue;
    }
    ++pcStrIn;
    /* Evaluate Escape sequence character */
    switch(*pcStrIn)
    {
      case CHAR_A: /* 'a', Bell (alert) */
        pucStrOut[uiIndexOut++]=ESC_SEQ_CHAR_A;
        break;
      case CHAR_B: /* 'b', Backspace */
        pucStrOut[uiIndexOut++]=ESC_SEQ_CHAR_B;
        break;
      case CHAR_F: /* 'f', Form feed */
        pucStrOut[uiIndexOut++]=ESC_SEQ_CHAR_F;
        break;
      case CHAR_N: /* 'n', New line */
        pucStrOut[uiIndexOut++]=ESC_SEQ_CHAR_N;
        break;
      case CHAR_R: /* 'r', Carriage return */
        pucStrOut[uiIndexOut++]=ESC_SEQ_CHAR_R;
        break;
      case CHAR_T: /* 't', Horizontal Tab */
        pucStrOut[uiIndexOut++]=ESC_SEQ_CHAR_T;
        break;
      case CHAR_V: /* 'v', Vertical Tab */
        pucStrOut[uiIndexOut++]=ESC_SEQ_CHAR_V;
        break;
      case CHAR_SQ: /* '\'', Single quotation mark */
        pucStrOut[uiIndexOut++]=ESC_SEQ_CHAR_SQ;
        break;
      case CHAR_DQ: /* '\"', Double quotation mark */
        pucStrOut[uiIndexOut++]=ESC_SEQ_CHAR_DQ;
        break;
      case CHAR_BS: /* '\\', Backslash */
        pucStrOut[uiIndexOut++]=ESC_SEQ_CHAR_BS;
        break;
      case CHAR_QUES: /* '?', Question mark  */
        pucStrOut[uiIndexOut++]=ESC_SEQ_CHAR_QUES;
        break;
      case CHAR_O: /* 'o', ASCII octal notation */
        ++pcStrIn;
        if(!IS_OCT_DIGIT(*pcStrIn)) /* If 1st char isn't octal, escape sequence is invalid */
          return(INI_ERR_ESC_SEQ_INVALID);

        caTmp[3]='\0';
        if(*pcStrIn > '3')
          return(INI_ERR_ESC_SEQ_INVALID);
        /* Next 2 bytes also must be a valid octal number */
        if((!IS_OCT_DIGIT(*(pcStrIn+1))) ||
           (!IS_OCT_DIGIT(*(pcStrIn+2))))
          return(INI_ERR_ESC_SEQ_INVALID);

        caTmp[0]=*pcStrIn++;
        caTmp[1]=*pcStrIn++;
        caTmp[2]=*pcStrIn++;
        if(uiIndexOut > (*puiDataSize)-2)
          return(INI_ERR_BUFFER_TOO_SMALL);
        pucStrOut[uiIndexOut++]=strtol(caTmp,NULL,8);
        continue; /* Continue, to keep the binary datatype flag */
      case CHAR_X: /* 'x', ASCII Hexadecimal notation */
        ++pcStrIn;
        if(!isxdigit(*pcStrIn)) /* If 1st char isn't hexadecimal, escape sequence is invalid */
          return(INI_ERR_ESC_SEQ_INVALID);

        caTmp[2]='\0';
        /* Next byte also must be a valid hexadecimal number */
        if(!isxdigit(*(pcStrIn+1)))
          return(INI_ERR_ESC_SEQ_INVALID);

        caTmp[0]=*pcStrIn++;
        caTmp[1]=*pcStrIn++;
        if(uiIndexOut > (*puiDataSize)-2)
          return(INI_ERR_BUFFER_TOO_SMALL);
        pucStrOut[uiIndexOut++]=strtol(caTmp,NULL,16);
        continue; /* Continue, to keep the binary datatype flag */
      case CHAR_0: /* '\0' continue on next line, return directly (line ended) */
        pucStrOut[uiIndexOut]='\0';
        *puiDataSize=uiIndexOut+1;
        return(INI_CONTINUE_ON_NEXT_LINE);
      default: /* Escape sequence unknown */
        return(INI_ERR_ESC_SEQ_INVALID);
    }
    *pType &= ~(eDataType_Binary);
    ++pcStrIn;
  }
  if(*pType != eDataType_Binary) /* If nonbinary data, termination is added */
    pucStrOut[uiIndexOut++]='\0';
  *puiDataSize=uiIndexOut;
  return(INI_ERR_NONE);
}

INLINE_FCT int Ini_EscapeData_m(const unsigned char *pucDataIn,
                                unsigned int uiDataInSize,
                                char *pcOutText,
                                unsigned int *puiOutBufSize,
                                EDataType dataType)
{
  unsigned int uiIndex;
  unsigned int uiIndexOut=0;

  /* Empty data, this is valid, just return empty string then */
  if(!uiDataInSize)
  {
    pcOutText[0]='\0';
    *puiOutBufSize=0;
    return(INI_ERR_NONE);
  }

  /* If type is string, escape with quotes */
  if((dataType & eDataType_String) &&
     ((dataType & (eDataType_Int | eDataType_Uint | eDataType_Double | eDataType_Boolean)) == 0))
    pcOutText[uiIndexOut++]='\"';

  if(dataType == eDataType_Binary)
  {/* If type is just binary, escape all bytes as binary */
    for(uiIndex=0;uiIndex < uiDataInSize;++uiIndex)
    {
      if(uiIndexOut > (*puiOutBufSize)-5)
        return(INI_ERR_BUFFER_TOO_SMALL);
      pcOutText[uiIndexOut++]=CHAR_BS;
      pcOutText[uiIndexOut++]=CHAR_X;
      pcOutText[uiIndexOut++]=(pucDataIn[uiIndex]>>4)+(((pucDataIn[uiIndex]>>4) > 9)?'A'-10:'0');
      pcOutText[uiIndexOut++]=(pucDataIn[uiIndex] & 0xF)+(((pucDataIn[uiIndex] & 0xF) > 9)?'A'-10:'0');
    }
    pcOutText[uiIndexOut++]='\0';
    *puiOutBufSize=uiIndexOut;
    return(INI_ERR_NONE);
  }
  for(uiIndex=0;uiIndex < uiDataInSize-1;++uiIndex)
  {
    if(uiIndexOut+2 > *puiOutBufSize) /* Buffer too small */
      return(INI_ERR_BUFFER_TOO_SMALL);

    switch(pucDataIn[uiIndex])
    {
      case ESC_SEQ_CHAR_A:
        pcOutText[uiIndexOut++]='\\';
        pcOutText[uiIndexOut++]=CHAR_A;
        break;
      case ESC_SEQ_CHAR_B:
        pcOutText[uiIndexOut++]='\\';
        pcOutText[uiIndexOut++]=CHAR_B;
        break;
      case ESC_SEQ_CHAR_F:
        pcOutText[uiIndexOut++]='\\';
        pcOutText[uiIndexOut++]=CHAR_F;
        break;
      case ESC_SEQ_CHAR_N:
        pcOutText[uiIndexOut++]='\\';
        pcOutText[uiIndexOut++]=CHAR_N;
        break;
      case ESC_SEQ_CHAR_R:
        pcOutText[uiIndexOut++]='\\';
        pcOutText[uiIndexOut++]=CHAR_R;
        break;
      case ESC_SEQ_CHAR_T:
        pcOutText[uiIndexOut++]='\\';
        pcOutText[uiIndexOut++]=CHAR_T;
        break;
      case ESC_SEQ_CHAR_V:
        pcOutText[uiIndexOut++]='\\';
        pcOutText[uiIndexOut++]=CHAR_V;
        break;
      case ESC_SEQ_CHAR_QUES:
        pcOutText[uiIndexOut++]='\\';
        pcOutText[uiIndexOut++]=CHAR_QUES;
        break;
      case ESC_SEQ_CHAR_SQ:
        pcOutText[uiIndexOut++]='\\';
        pcOutText[uiIndexOut++]=CHAR_SQ;
        break;
      case ESC_SEQ_CHAR_DQ:
        pcOutText[uiIndexOut++]='\\';
        pcOutText[uiIndexOut++]=CHAR_DQ;
        break;
      case ESC_SEQ_CHAR_BS:
        pcOutText[uiIndexOut++]='\\';
        pcOutText[uiIndexOut++]=CHAR_BS;
        break;
      default:
        /* If ASCII and printable, just copy */
        if((isascii(pucDataIn[uiIndex])) && (isprint(pucDataIn[uiIndex])))
        {
          pcOutText[uiIndexOut++]=pucDataIn[uiIndex];
          break;
        }
        /* Binary data, escape to HEX */
        pcOutText[uiIndexOut++]=CHAR_BS;
        pcOutText[uiIndexOut++]=CHAR_X;
        if(uiIndexOut > (*puiOutBufSize)-3)
          return(INI_ERR_BUFFER_TOO_SMALL);

        pcOutText[uiIndexOut++]=(pucDataIn[uiIndex]>>4)+(((pucDataIn[uiIndex]>>4) > 9)?'A'-10:'0');
        pcOutText[uiIndexOut++]=(pucDataIn[uiIndex] & 0xF)+(((pucDataIn[uiIndex] & 0xF) > 9)?'A'-10:'0');
        continue;
    }
  }
  if(uiIndexOut > (*puiOutBufSize)-2)
    return(INI_ERR_BUFFER_TOO_SMALL);
  if((dataType & eDataType_String) &&
     ((dataType & (eDataType_Int | eDataType_Uint | eDataType_Double | eDataType_Boolean)) == 0))
    pcOutText[uiIndexOut++]='\"';
  pcOutText[uiIndexOut++]='\0';
  *puiOutBufSize=uiIndexOut;
  return(INI_ERR_NONE);
}

INLINE_FCT char *Ini_FindEndOfLine_m(char **ppcLine,
                                     unsigned int *puiLength)
{
  char *pcNextLineStart;
  char *pcLastChar=NULL;
  /* Skip leading whitespaces */
  while(isspace(**ppcLine))
  {
    ++(*ppcLine);
  }
  pcNextLineStart=*ppcLine;
  while(((*pcNextLineStart) != CHAR_NEWLINE_1) && ((*pcNextLineStart) != CHAR_NEWLINE_2))
  {
    /* Check if '\0' is reached, end of data then */
    if(*pcNextLineStart++ == '\0')
    {
      *pcLastChar='\0';
      *puiLength=pcLastChar-(*ppcLine)+1;
      return(NULL);
    }
    if(!isspace(*(pcNextLineStart-1)))
      pcLastChar=pcNextLineStart;
  }
  *pcLastChar='\0'; /* Terminate line */
  *puiLength=pcLastChar-(*ppcLine)+1;
  /* If there are multiple newlines, skip them all */
  while(((*++pcNextLineStart) == CHAR_NEWLINE_1) || ((*pcNextLineStart) == CHAR_NEWLINE_2));

  if(*pcNextLineStart == '\0') /* End after newline */
    pcNextLineStart=NULL;
  return(pcNextLineStart);
}

INLINE_FCT int Ini_CreateNewSection_m(Inifile file,
                                      const char *pcSection)
{
  void *pvTmp=NULL;

  if(!CHECK_SECTIONNAME_VALID(pcSection))
    return(INI_ERR_SECTIONNAME_INVALID);

  /* Check if section already exists */
  if(Ini_FindSection_m(file,pcSection))
  {
    if(OPTION_CHK(file,INI_OPT_ALLOW_DOUBLE_ENTRIES))
      return(INI_ERR_NONE);

    return(INI_ERR_SECTIONNAME_DUPLICATE);
  }

  /* Realloc if needed */
  if((SECTIONS_NEED_REALLOC(file)) && (!(pvTmp=SECTIONS_REALLOC(file->ptagSections,file->uiSectionsMax*=2))))
    return(INI_ERR_MEMORY_ALLOC);
  if(pvTmp)
  {
    file->ptagCurrSection=((struct TagIniSection*)pvTmp)+file->uiSectionsCount-1;
    file->ptagSections=pvTmp;
  }

  /* Check if iterator is on the last key */
  if(file->ptagCurrSection < file->ptagSections+file->uiSectionsCount-1)
  {
    memmove(file->ptagCurrSection+2,
            file->ptagCurrSection+1,
            sizeof(struct TagIniSection)*(file->uiSectionsCount-(file->ptagCurrSection-file->ptagSections)-1));
  }

  ++file->ptagCurrSection;

  /* Copy Sectionname */
  strcpy(file->ptagCurrSection->caSectionName,pcSection);

  /* Init new Keys table... */
  if(!(file->ptagCurrSection->ptagKeys=KEYS_REALLOC(NULL,INITIAL_KEYS_PER_SECTION_COUNT)))
    return(INI_ERR_MEMORY_ALLOC);
  file->ptagCurrSection->ptagCurrKey=file->ptagCurrSection->ptagKeys;
  file->ptagCurrSection->uiKeysCount=0;
  file->ptagCurrSection->uiKeysMax=INITIAL_KEYS_PER_SECTION_COUNT;
  ++file->uiSectionsCount;
  return(INI_ERR_NONE);
}

INLINE_FCT int Ini_CreateNewKey_m(Inifile file,
                                  const char *pcKey)
{
  void *pvTmp=NULL;

  if(!CHECK_KEYNAME_VALID(pcKey))
  {
    ERR_PRINT_VARGS("Ini_CreateNewKey_m() failed (%d): %s",INI_ERR_KEYNAME_INVALID,IniFile_GetErrorText(INI_ERR_KEYNAME_INVALID));
    return(INI_ERR_KEYNAME_INVALID);
  }
  if(Ini_FindKey_m(file,pcKey))
  {
    if(OPTION_CHK(file,INI_OPT_ALLOW_DOUBLE_ENTRIES))
      return(INI_ERR_NONE);

    return(INI_ERR_KEYNAME_DUPLICATE);
  }
  if((KEYS_NEED_REALLOC(file->ptagCurrSection))
     && (!(pvTmp=KEYS_REALLOC(file->ptagCurrSection->ptagKeys,file->ptagCurrSection->uiKeysMax*=2))))
  {
    ERR_PRINT_VARGS("Ini_CreateNewKey_m() failed (%d): %s",INI_ERR_MEMORY_ALLOC,IniFile_GetErrorText(INI_ERR_MEMORY_ALLOC));
    return(INI_ERR_MEMORY_ALLOC);
  }
  if(pvTmp)
  {
    file->ptagCurrSection->ptagCurrKey=((struct TagIniKey*)pvTmp)+file->ptagCurrSection->uiKeysCount-1;
    file->ptagCurrSection->ptagKeys=pvTmp;
  }

  /* Increment current to next if there are already keys */
  if(file->ptagCurrSection->uiKeysCount)
    ++file->ptagCurrSection->ptagCurrKey;
  /* Check if last key */
  if(file->ptagCurrSection->ptagCurrKey < file->ptagCurrSection->ptagKeys+file->ptagCurrSection->uiKeysCount-1)
  {
    memmove(file->ptagCurrSection->ptagCurrKey+1,
            file->ptagCurrSection->ptagCurrKey,
            sizeof(struct TagIniKey)*(file->ptagCurrSection->uiKeysCount-(file->ptagCurrSection->ptagCurrKey-file->ptagCurrSection->ptagKeys)));
  }
  strcpy(file->ptagCurrSection->ptagCurrKey->caKeyName,pcKey);
  file->ptagCurrSection->ptagCurrKey->uiDataSize=0;
  file->ptagCurrSection->ptagCurrKey->type=0;
  ++file->ptagCurrSection->uiKeysCount;
  return(INI_ERR_NONE);
}

INLINE_FCT void Ini_RemoveLeadingWhitespaces_m(char **ppcString)
{
  while(isspace(**ppcString))
    ++(*ppcString);
}

INLINE_FCT unsigned int Ini_RemoveFollowingWhitespaces_m(char *pcString,
                                                         unsigned int uiLength)
{
  if(!uiLength)
    return(0);

  while(isspace(pcString[--uiLength]));
  pcString[++uiLength]='\0';
  return(uiLength);
}

INLINE_FCT int Ini_stricmp(const char *pcStr1,
                           const char *pcStr2)
{
  unsigned uiIndex=0;
  int iRc;

  while((iRc=tolower(pcStr1[uiIndex]-tolower(pcStr2[uiIndex]))) == 0)
  {
    if(pcStr1[uiIndex++] == '\0')
      break;
  }
  return(iRc);
}

INLINE_FCT int Ini_CheckNameValid_m(const char *pcName,
                                    unsigned int uiMaxLen)
{
  unsigned int uiIndex;

  if(!pcName)
    return(0);
  uiIndex=strlen(pcName);

  if((!uiIndex) || (uiIndex > uiMaxLen))
    return(0);

  do /* Validate each character */
  {
    if((!isprint(pcName[--uiIndex])) || (isspace(pcName[uiIndex])))
      return(0);
  }
  while(uiIndex);

  return(1);
}

INLINE_FCT int Ini_FindSection_m(Inifile file,
                                 const char *pcSection)
{
  unsigned int uiIndex;

  /* If not specified (empty), use first section (Always allocated) */
  if((!pcSection) || (pcSection[0] == '\0'))
  {
    file->ptagCurrSection=file->ptagSections;
    return(1);
  }
  for(uiIndex=1;uiIndex < file->uiSectionsCount;++uiIndex)
  {
    if(INI_STRCMP(file,pcSection,file->ptagSections[uiIndex].caSectionName) == 0)
    {
      file->ptagCurrSection=&file->ptagSections[uiIndex];
      return(1); /* Found Section, return */
    }
  }
  return(0);
}

INLINE_FCT int Ini_FindKey_m(Inifile file,
                             const char *pcKey)
{
  unsigned int uiIndex;

  for(uiIndex=0;uiIndex < file->ptagCurrSection->uiKeysCount;++uiIndex)
  {
    if(INI_STRCMP(file,pcKey,file->ptagCurrSection->ptagKeys[uiIndex].caKeyName) == 0)
    {
      file->ptagCurrSection->ptagCurrKey=&file->ptagCurrSection->ptagKeys[uiIndex];
      return(1); /* Found key, return */
    }
  }
  return(0);
}

INLINE_FCT void Ini_ResetContent_m(Inifile file)
{
  file->ptagSections[0].uiKeysCount=0;
  file->ptagSections[0].ptagCurrKey=file->ptagSections[0].ptagKeys;
  /* Keep the first (empty) section as is, it's always used */
  while(file->uiSectionsCount > 1)
  {
    --file->uiSectionsCount;
    free(file->ptagSections[file->uiSectionsCount].ptagKeys);
    file->ptagSections[file->uiSectionsCount].uiKeysCount=0;

  }
  file->ptagCurrSection=file->ptagSections;
  file->ptagSections[0].ptagCurrKey=file->ptagSections[0].ptagKeys;
}

INLINE_FCT int Ini_OptionsValid_m(unsigned int uiOptions)
{
  uiOptions&=~(INI_OPT_CASE_SENSITIVE | INI_OPT_IGNORE_MALFORMED_LINES | INI_OPT_ALLOW_DOUBLE_ENTRIES | INI_OPT_ALLOW_MULTILINE);
  return((uiOptions)?-1:0);
}

INLINE_FCT int Ini_KeySetData_m(Inifile file,
                                const TagData *ptagData)
{
  int iRc;
  if(!ptagData) /* Remove data from key */
  {
    file->ptagCurrSection->ptagCurrKey->uiDataSize=0;
    return(INI_ERR_NONE);
  }
  switch(ptagData->uiType&0xFF)
  {
    case eDataType_Int:
    case eDataType_Uint:
      switch(ptagData->uiType&0xFF00)
      {
        case eRepr_Int_Binary:
          /* Skip leading zeros */
          for(iRc=0;(unsigned)iRc < sizeof(int)*CHAR_BIT+1;++iRc)
          {
            if(ptagData->data.uiVal & (1<<(sizeof(int)*CHAR_BIT-iRc-1)))
              break;
          }
          file->ptagCurrSection->ptagCurrKey->uiDataSize=iRc++;
          while((unsigned)iRc < sizeof(int)*CHAR_BIT+1)
          {
            file->ptagCurrSection->ptagCurrKey->ucaData[iRc-file->ptagCurrSection->ptagCurrKey->uiDataSize-1]=
              (ptagData->data.uiVal & (1<<(sizeof(int)*CHAR_BIT-iRc)))?'1':'0';
            ++iRc;
          }
          iRc-=file->ptagCurrSection->ptagCurrKey->uiDataSize;
          file->ptagCurrSection->ptagCurrKey->ucaData[iRc-1]='\0';
          //printf("KEY SET DATA: %s len: %d\n",(char*)file->ptagCurrSection->ptagCurrKey->ucaData,iRc);
          break;
        case eRepr_Int_Octal:
          if((ptagData->uiType & 0xFF) == eDataType_Int)
            iRc=sprintf((char*)file->ptagCurrSection->ptagCurrKey->ucaData,"0%o",ptagData->data.iVal);
          else
            iRc=sprintf((char*)file->ptagCurrSection->ptagCurrKey->ucaData,"0%o",ptagData->data.uiVal);
          ++iRc;
          break;
        case eRepr_Int_Decimal:
          if((ptagData->uiType & 0xFF) == eDataType_Int)
            iRc=sprintf((char*)file->ptagCurrSection->ptagCurrKey->ucaData,"%d",ptagData->data.iVal);
          else
            iRc=sprintf((char*)file->ptagCurrSection->ptagCurrKey->ucaData,"%u",ptagData->data.uiVal);
          ++iRc;
          break;
        case eRepr_Int_Hexadecimal:
          if((ptagData->uiType & 0xFF) == eDataType_Int)
            iRc=sprintf((char*)file->ptagCurrSection->ptagCurrKey->ucaData,"0x%X",ptagData->data.iVal);
          else
            iRc=sprintf((char*)file->ptagCurrSection->ptagCurrKey->ucaData,"0x%X",ptagData->data.uiVal);
          ++iRc;
          break;
        default:
          ERR_PRINT_VARGS("Ini_KeySetData_m(): Invalid eDataRepresentation (0x%X)",ptagData->uiType);
          return(INI_ERR_DATATYPE_UNKNOWN);
      }
      break;
    case eDataType_Double:
      iRc=sprintf((char*)file->ptagCurrSection->ptagCurrKey->ucaData,"%f",ptagData->data.dVal);
      break;
    case eDataType_Boolean:
      iRc=strlen(((ptagData->data.bVal)?pcaBooleanStringsTrue_m[0]:pcaBooleanStringsFalse_m[0]))+1;
      memcpy(file->ptagCurrSection->ptagCurrKey->ucaData,
             (ptagData->data.bVal)?pcaBooleanStringsTrue_m[0]:pcaBooleanStringsFalse_m[0],
             iRc);
      break;
    case eDataType_String:
      if(ptagData->uiDataSizeUsed > MAX_KEYVAL_LENGTH)
        return(INI_ERR_BUFFER_TOO_SMALL);
      iRc=ptagData->uiDataSizeUsed;
      memcpy(file->ptagCurrSection->ptagCurrKey->ucaData,
             ptagData->data.pcVal,
             iRc);
      break;
    case eDataType_Binary:
      if(ptagData->uiDataSizeUsed > MAX_KEYVAL_LENGTH)
        return(INI_ERR_BUFFER_TOO_SMALL);
      iRc=ptagData->uiDataSizeUsed;
      memcpy(file->ptagCurrSection->ptagCurrKey->ucaData,
             ptagData->data.pucVal,
             iRc);
      break;
    default:
      return(INI_ERR_DATATYPE_UNKNOWN);
  }
  if(iRc < 1)
  {
    ERR_PRINT_VARGS("Ini_KeySetData_m() failed (%d): %s",INI_ERR_CONVERSION,IniFile_GetErrorText(INI_ERR_CONVERSION));
    return(INI_ERR_CONVERSION);
  }
  file->ptagCurrSection->ptagCurrKey->type=ptagData->uiType;
  file->ptagCurrSection->ptagCurrKey->uiDataSize=iRc;
  return(INI_ERR_NONE);
}

INLINE_FCT int Ini_Read_Section_m(char *pcLine,
                                  unsigned int uiLength)
{
  /* Check if last char is correct */
  if(pcLine[uiLength-2] != SECTION_NAME_END)
  {
    ERR_PRINT_VARGS("Ini_Read_Section_m() failed (%d): %s",INI_ERR_MALFORMED,IniFile_GetErrorText(INI_ERR_MALFORMED));
    return(INI_ERR_MALFORMED);
  }
  pcLine[uiLength-2]='\0';
  return(INI_ERR_NONE);
}

INLINE_FCT int Ini_Read_Key_m(Inifile tagFile,
                              char **ppcNextLine,
                              char *pcLine,
                              unsigned int uiLength)
{
  char *pcTmp=pcLine;
  int iRc;
  while((*pcTmp != '\0'))
  {
    if((*pcTmp == CHAR_EQUAL1) || (*pcTmp == CHAR_EQUAL2))
      break;
    ++pcTmp;
  }
  if(*pcTmp == '\0')
  {
    ERR_PRINT_VARGS("Ini_Read_Key_m() failed (%u): %s (No Equal sign found)",INI_ERR_MALFORMED,IniFile_GetErrorText(INI_ERR_MALFORMED));
    return(INI_ERR_MALFORMED);
  }

  *pcTmp='\0'; /* Overwrite '=' or ':' with '\0' */
  Ini_RemoveFollowingWhitespaces_m(pcLine,pcTmp-pcLine); /* Remove whitespaces between keyname and '=' */
  if((iRc=Ini_CreateNewKey_m(tagFile,pcLine)) != INI_ERR_NONE)
  {
    ERR_PRINT_VARGS("Ini_Read_Key_m() failed (%u): %s",iRc,IniFile_GetErrorText(iRc));
    return(iRc);
  }

  ++pcTmp;
  Ini_RemoveLeadingWhitespaces_m(&pcTmp);

  uiLength=pcLine+uiLength-pcTmp;

  tagFile->ptagCurrSection->ptagCurrKey->type=0;
  /* Check for Quotes at beginning and at the end, and remove them */
  if((pcTmp[0] == '\"') || (pcTmp[0] == '\''))
  {
    uiLength-=2; /* Real size is 2 shorter then */
    if(pcTmp[0] != pcTmp[uiLength])
    {
      --tagFile->ptagCurrSection->uiKeysCount; /* Discard key on failure */
      ERR_PRINT_VARGS("Ini_Read_Key_m() failed (%d): %s",INI_ERR_READ_QUOTES_INVALID,IniFile_GetErrorText(INI_ERR_READ_QUOTES_INVALID));
      return(INI_ERR_READ_QUOTES_INVALID);
    }
    pcTmp[uiLength]='\0';
    ++pcTmp;
    tagFile->ptagCurrSection->ptagCurrKey->type|=eDataType_String;
  }
  /* Check if key is empty */
  if(pcTmp[0] == '\0')
  {
    tagFile->ptagCurrSection->ptagCurrKey->type=0;
    tagFile->ptagCurrSection->ptagCurrKey->uiDataSize=0;
    return(INI_ERR_NONE);
  }
//printf("Length: %u, ln: %s\n",uiLength,pcTmp);
  uiLength=MAX_KEYVAL_LENGTH;
  if((iRc=Ini_UnEscapeString_m(pcTmp,
                               tagFile->ptagCurrSection->ptagCurrKey->ucaData,
                               &uiLength,
                               &tagFile->ptagCurrSection->ptagCurrKey->type)) == INI_CONTINUE_ON_NEXT_LINE)
  {
    if(OPTION_CHK(tagFile,INI_OPT_ALLOW_MULTILINE))
    {
      unsigned int uiTmp;
      do
      {
        iRc=INI_ERR_NONE;
        if(!(*ppcNextLine)) /* If no more data, exit loop */
          break;
        pcLine=*ppcNextLine;
        *ppcNextLine=Ini_FindEndOfLine_m(&pcLine,&uiTmp);
        uiTmp=MAX_KEYVAL_LENGTH-uiLength;
//      printf("\"%s\" size now: %u, left: %u\n",tagFile->ptagCurrSection->ptagCurrKey->ucaData,uiLength,uiTmp);
        iRc=Ini_UnEscapeString_m(pcLine,
                                 &tagFile->ptagCurrSection->ptagCurrKey->ucaData[uiLength-1],
                                 &uiTmp,
                                 &tagFile->ptagCurrSection->ptagCurrKey->type);
        uiLength+=uiTmp-1; /* Only 1 termination needed */
      }while(iRc == INI_CONTINUE_ON_NEXT_LINE);
    }
    else /* If continuation on next line is disabled, just go on */
      iRc=INI_ERR_NONE;
  }
//printf("LINE: %u len, \"%s\"\n",uiLength,tagFile->ptagCurrSection->ptagCurrKey->ucaData);
  tagFile->ptagCurrSection->ptagCurrKey->uiDataSize=uiLength;
  if(iRc!=INI_ERR_NONE)
  {
    ERR_PRINT_VARGS("Ini_Read_Key_m() failed (%d): %s",iRc,IniFile_GetErrorText(iRc));
    --tagFile->ptagCurrSection->uiKeysCount; /* Discard key on failure */
  }
  return(iRc);
}

INLINE_FCT int Ini_Read_Comment_m(char *pcLine)
{
//printf("Ignoring Comment: %s\n",pcLine);   /* TODO: add option to read comments? */
  return(INI_ERR_NONE);
}

INLINE_FCT int Ini_ConvertValue_m(IniKey ptagKey,
                                  TagData *ptagData)
{
  char *pcTmp;
  long lTmp;
  unsigned long ulTmp;
  double dTmp;

  if(!ptagKey->uiDataSize)
  {
    return(INI_ERR_DATA_EMPTY);
  }

  /* If data contains binary Bytes, already return on incompatible types */
  if(ptagKey->type & eDataType_Binary)
  {
    if(((ptagData->uiType & 0xFF) == eDataType_Int)    ||
       ((ptagData->uiType & 0xFF) == eDataType_Uint)   ||
       ((ptagData->uiType & 0xFF) == eDataType_Double) ||
       ((ptagData->uiType & 0xFF) == eDataType_Boolean))
    {
      return(INI_ERR_CONVERSION);
    }
  }
  errno=0; /* Reset error indicator */
  switch(ptagData->uiType & 0xFF)
  {
    case eDataType_Int:
      switch(ptagData->uiType & 0xFF00)
      {
        case eRepr_Int_Binary:
          ulTmp=strtoul((char*)ptagKey->ucaData,&pcTmp,2);
          lTmp=(int)ulTmp; /* Convert unsigned -> signed */
          if(ulTmp > UINT_MAX) /* Check if value overflow int, if so set pcTmp to indicate Error */
            pcTmp=(char*)ptagKey->ucaData;
          break;
        case eRepr_Int_Octal:
          lTmp=strtol((char*)ptagKey->ucaData,&pcTmp,8);
          break;
        case eRepr_Int_Decimal:
          lTmp=strtol((char*)ptagKey->ucaData,&pcTmp,10);
          break;
        case eRepr_Int_Hexadecimal:
          lTmp=strtol((char*)ptagKey->ucaData,&pcTmp,16);
          break;
        default:
          return(INI_ERR_DATATYPE_UNKNOWN);
      }
      /* Check for conversion error / Int Over- or Underflow */
      if((errno == ERANGE) || /* Check for over- /underflow */
         (*pcTmp != '\0')) /* Check for invalid string */
      {
        return(INI_ERR_CONVERSION);
      }
      ptagData->data.iVal=(int)lTmp;
      break;
    case eDataType_Uint:
      switch(ptagData->uiType & 0xFF00)
      {
        case eRepr_Int_Binary:
          ulTmp=strtoul((char*)ptagKey->ucaData,&pcTmp,2);
          break;
        case eRepr_Int_Octal:
          ulTmp=strtoul((char*)ptagKey->ucaData,&pcTmp,8);
          break;
        case eRepr_Int_Decimal:
          ulTmp=strtoul((char*)ptagKey->ucaData,&pcTmp,10);
          break;
        case eRepr_Int_Hexadecimal:
          ulTmp=strtoul((char*)ptagKey->ucaData,&pcTmp,16);
          break;
        default:
          return(INI_ERR_DATATYPE_UNKNOWN);
      }
      /* Check for conversion error / Uint Overflow */
      if((errno == ERANGE) || /* Check for overflow */
         (*pcTmp != '\0')) /* Check for invalid string */
        return(INI_ERR_CONVERSION);
      ptagData->data.uiVal=(unsigned int)ulTmp;
      break;
    case eDataType_Double:
      dTmp=strtod((char*)ptagKey->ucaData,&pcTmp);
      /* Check for conversion error / Over- or Underflow */
      if(((dTmp == HUGE_VAL) && (errno == ERANGE))  || /* Check for Overflow */
         ((dTmp == -HUGE_VAL) && (errno == ERANGE)) || /* Check for Underflow */
         ((dTmp == 0.0) && (errno == ERANGE))       || /* Check other conversion error */
         (*pcTmp != '\0'))                             /* Check for invalid string */
        return(INI_ERR_CONVERSION);
      ptagData->data.dVal=dTmp;
      break;
    case eDataType_Boolean:
      for(ulTmp=0;ulTmp < sizeof(pcaBooleanStringsFalse_m)/sizeof(char *);++ulTmp)
      {
        /* Check for positive boolean value */
        if(STRCMP_CIS((char*)ptagKey->ucaData,pcaBooleanStringsFalse_m[ulTmp]) == 0)
        {
          ptagData->data.bVal=0;
          break;
        }
        /* Check for negative boolean value */
        if(STRCMP_CIS((char*)ptagKey->ucaData,pcaBooleanStringsTrue_m[ulTmp]) == 0)
        {
          ptagData->data.bVal=1;
          break;
        }
      }
      /* Check if value found */
      if(ulTmp == sizeof(pcaBooleanStringsFalse_m)/sizeof(char *))
        return(INI_ERR_CONVERSION);
      break;
    case eDataType_String:
      if(ptagKey->uiDataSize > ptagData->uiDataSizeMax)
        return(INI_ERR_BUFFER_TOO_SMALL); /* Buffer too small */
      ptagData->uiDataSizeUsed=ptagKey->uiDataSize;
      memcpy(ptagData->data.pcVal,ptagKey->ucaData,ptagKey->uiDataSize);
      break;
    case eDataType_Binary:
      if(ptagKey->uiDataSize > ptagData->uiDataSizeMax)
        return(INI_ERR_BUFFER_TOO_SMALL); /* Buffer too small */
      ptagData->uiDataSizeUsed=ptagKey->uiDataSize;
      memcpy(ptagData->data.pucVal,ptagKey->ucaData,ptagKey->uiDataSize);
      break;
    default: /* Unknown datatype */
      return(INI_ERR_DATATYPE_UNKNOWN);
  }
  ptagKey->type|=ptagData->uiType;
  return(INI_ERR_NONE);
}
