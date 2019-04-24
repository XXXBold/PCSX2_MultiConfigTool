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
  #define STRCMP_CIS(str1,str2) _stricmp(str1,str2)
#else
  #include <unistd.h>
  #if defined (_POSIX_C_SOURCE) && (_POSIX_C_SOURCE>=200112L)
    #include <strings.h>
    #define STRCMP_CIS(str1,str2) strcasecmp(str1,str2)
  #else
    #define STRCMP_CIS(str1,str2) Ini_stricmp(str1,str2)
  #endif
#endif /* _WIN32 */
#define STRCMP_CS(str1,str2)  strcmp(str1,str2)
#define IS_OCT_DIGIT(ch) ((isdigit(ch)) && (ch<'8'))

#define KEYS_REALLOC(ptr,newCount)     realloc((ptr),(newCount)*sizeof(struct TagIniKey))
#define SECTIONS_REALLOC(ptr,newCount) realloc((ptr),(newCount)*sizeof(struct TagIniSection))
#define SECTIONS_NEED_REALLOC(file)    (((file)->uiSectionsCount<(file)->uiSectionsMax)?0:1)
#define KEYS_NEED_REALLOC(section)     (((section)->uiKeysCount<(section)->uiKeysMax)?0:1)

#define SECTION_CALC_INDEX(file) (int)(((file)->ptagCurrSection-(file)->ptagSections)/(unsigned int)sizeof(struct TagIniSection*))
#define KEY_CALC_INDEX(file)     (int)(((file)->ptagCurrSection->ptagCurrKey-(file)->ptagCurrSection->ptagKeys)/(unsigned int)sizeof(struct TagIniKey *))

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
  MAX_PATH_LENGTH                   = 260,
  /* Internal used return values */
  INI_CONTINUE_ON_NEXT_LINE         = 100,
};

enum chars
{
  SECTION_NAME_BEGIN        = '[',
  SECTION_NAME_END          = ']',
  COMMENT_INDICATOR_1       = ';',
  COMMENT_INDICATOR_2       = '#',

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
//ESC_SEQ_CHAR_O            = '\o',
//ESC_SEQ_CHAR_X            = '\x',
  ESC_SEQ_CHAR_0            = '\0',
};

struct TagIniKey
{
  char caKeyName[MAX_KEYNAME_LENGTH+1];
  char caData[MAX_KEYVAL_LENGTH];
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

INLINE_PROT int Ini_UnEscapeString_m(unsigned char *pucStr,
                                     unsigned int *puiDataSize);

INLINE_PROT int Ini_EscapeString_m(const unsigned char *pucInText,
                                   char *pcOutText,
                                   unsigned int *puiOutBufSize);

INLINE_PROT int Ini_CheckNameValid_m(const char *pcName,
                                     unsigned int uiMaxLen);
INLINE_PROT int Ini_CheckKeyValueValid_m(const char *pcValue);

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
INLINE_PROT int Ini_Read_Key_m(char *pcLine,
                               char **ppcKeyVal,
                               unsigned int *puiValLength);
INLINE_PROT int Ini_Read_Comment_m(char *pcLine);

INLINE_PROT int Ini_ConvertValue_m(const char *pcStr,
                                   TagData *ptagData);


static const char *pcaBooleanStringsTrue_m[]={"true","enabled","1","y","t"};
static const char *pcaBooleanStringsFalse_m[]={"false","disabled","0","n","f"};


int IniFile_New(Inifile *newFile,
                unsigned int options)
{
  if(Ini_OptionsValid_m(options))
    return(INI_ERR_PARAM_INVALID);

  if(!(*newFile=malloc(sizeof(struct TagIniFile))))
    return(INI_ERR_MEMORY_ALLOC);

  (*newFile)->uiBufSize=(MAX_KEYNAME_LENGTH+MAX_KEYVAL_LENGTH)*5;
  if(!((*newFile)->caTmpBuffer=malloc((*newFile)->uiBufSize)))
  {
    free(*newFile);
    return(INI_ERR_MEMORY_ALLOC);
  }

  if(!((*newFile)->ptagSections=SECTIONS_REALLOC(NULL,INITIAL_SECTION_COUNT)))
  {
    free((*newFile)->caTmpBuffer);
    free(*newFile);
    return(INI_ERR_MEMORY_ALLOC);
  }
  if(!((*newFile)->ptagSections[0].ptagKeys=KEYS_REALLOC(NULL,INITIAL_KEYS_PER_SECTION_COUNT)))
  {
    free((*newFile)->ptagSections);
    free((*newFile)->caTmpBuffer);
    free(*newFile);
    return(INI_ERR_MEMORY_ALLOC);
  }
  (*newFile)->uiOptions=options;
  (*newFile)->uiSectionsCount=1;
  (*newFile)->uiSectionsMax=INITIAL_SECTION_COUNT;
  (*newFile)->ptagCurrSection=(*newFile)->ptagSections;
  (*newFile)->ptagSections[0].uiKeysCount=0;
  (*newFile)->ptagSections[0].uiKeysMax=INITIAL_KEYS_PER_SECTION_COUNT;
  (*newFile)->ptagSections[0].ptagCurrKey=(*newFile)->ptagSections[0].ptagKeys;
  return(INI_ERR_NONE);
}

int IniFile_Read(Inifile file,
                 const char *path)
{
  TagData tagData;
  FILE *fp;
  char *pcNextLine;
  char *pcLine;
  char *pcTmp;
  int iRc=INI_ERR_NONE;
  unsigned int uiLength;

  if((!file) || (!path)) /* Validate params for not being NULL */
    return(INI_ERR_PARAM_INVALID);

  /* Open files as binary, to stay compatible with all platforms */
  if(!(fp=fopen(path,"rb")))
    return(INI_ERR_FOPEN);

  fseek(fp,0,SEEK_END);
  uiLength=ftell(fp)+1;
  rewind(fp);
  if(uiLength>file->uiBufSize)
  {
    if(!(pcTmp=realloc(file->caTmpBuffer,uiLength)))
    {
      fclose(fp);
      return(INI_ERR_MEMORY_ALLOC);
    }
    file->uiBufSize=uiLength;
    file->caTmpBuffer=pcTmp;
  }
  /* Read whole file at once */
  uiLength=fread(file->caTmpBuffer,1,uiLength,fp)+1;
  if(!feof(fp)) /* If EOF not reached, I/O Error occured */
  {
    fclose(fp);
    return(INI_ERR_READ_IO);
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
        if((iRc=Ini_Read_Key_m(pcLine,&pcTmp,&uiLength)) != INI_ERR_NONE)
          break;

        if((iRc=Ini_CreateNewKey_m(file,pcLine)) != INI_ERR_NONE)
          break;
        /* Check for escape sequences, and if key value continues on next line */
        if((iRc=Ini_UnEscapeString_m((unsigned char*)pcTmp,&uiLength)) == INI_CONTINUE_ON_NEXT_LINE)
        {
          if(OPTION_CHK(file,INI_OPT_ALLOW_MULTILINE))
          {
            unsigned int uiLenTmp;
            do
            {
              iRc=INI_ERR_NONE;
              if(!pcNextLine) /* If no more data, exit loop */
                break;
              pcLine=pcNextLine;
              pcNextLine=Ini_FindEndOfLine_m(&pcLine,&uiLenTmp);
              iRc=Ini_UnEscapeString_m((unsigned char*)pcLine,&uiLenTmp);
              memmove(pcTmp+uiLength-1,pcLine,uiLenTmp);
              uiLength+=uiLenTmp-1; /* Only 1 termination needed */
            }while(iRc == INI_CONTINUE_ON_NEXT_LINE);
          }
          else /* If continuation on next line is disabled, just go on */
            iRc=INI_ERR_NONE;
        }
        else if(iRc != INI_ERR_NONE) /* Some error occured */
          break;
        tagData.dataSize=uiLength;
        tagData.eType=eDataType_String;
        tagData.data.pcVal=pcTmp;
        iRc=Ini_KeySetData_m(file,&tagData);
        break;
    }
    switch(iRc)
    {
      case INI_ERR_NONE: /* All okay, continue reading */
      case INI_CONTINUE_ON_NEXT_LINE:
        iRc=INI_ERR_NONE;
        break;
      case INI_ERR_READ_MALFORMED:
      case INI_ERR_READ_QUOTES_INVALID:
      case INI_ERR_ESC_SEQ_INVALID:
      case INI_ERR_SECTIONNAME_INVALID:
      case INI_ERR_KEYNAME_INVALID:
      case INI_ERR_KEYVAL_INVALID:
        if(!OPTION_CHK(file,INI_OPT_IGNORE_MALFORMED_LINES))
        {
          pcNextLine=NULL; /* Will quit loop below */
          Ini_ResetContent_m(file);
        }
        break;
      default: /* Fatal error, can't be ignored */
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

int IniFile_Write(Inifile file,
                  const char *path)
{
  FILE *fp;
  unsigned int uiSize;
  unsigned int uiIndexSection;
  unsigned int uiIndexKey;

  if((!file) || (!path)) /* Validate params for not being NULL */
    return(INI_ERR_PARAM_INVALID);

  if(!(fp=fopen(path,"w")))
    return(INI_ERR_FOPEN);

  for(uiIndexSection=0;uiIndexSection < file->uiSectionsCount;++uiIndexSection)
  {
    if(uiIndexSection) /* First section is unnamed */
    {
      /* Check return val for possible I/O Error */
      if(fprintf(fp,"[%s]\n",file->ptagSections[uiIndexSection].caSectionName) < 1)
      {
        fclose(fp);
        return(INI_ERR_WRITE_IO);
      }
    }
    for(uiIndexKey=0;uiIndexKey < file->ptagSections[uiIndexSection].uiKeysCount;++uiIndexKey)
    {
      uiSize=file->uiBufSize;
      if(Ini_EscapeString_m((unsigned char *)file->ptagSections[uiIndexSection].ptagKeys[uiIndexKey].caData,
                            file->caTmpBuffer,
                            &uiSize) != INI_ERR_NONE)
      {
        ERR_PRINT("Ini_EscapeString_m() failed, buffer too small, quit writing");
        fclose(fp);
        return(INI_ERR_BUFFER_TOO_SMALL);
      }
      /* Check return val for possible I/O Error */
      if(fprintf(fp,
                 "%s=%s\n",
                 file->ptagSections[uiIndexSection].ptagKeys[uiIndexKey].caKeyName,
                 file->caTmpBuffer) < 1)
      {
        fclose(fp);
        return(INI_ERR_WRITE_IO);
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

int IniFile_FindEntry_GetValue(Inifile file,
                               const char *section,
                               const char *key,
                               TagData *ptagData)
{
  if((!file) || (!key) || (!ptagData)) /* Validate params for not being NULL */
    return(INI_ERR_PARAM_INVALID);

  if(!Ini_FindSection_m(file,section))
    return(INI_ERR_FIND_NONE);

  if(!Ini_FindKey_m(file,key))
    return(INI_ERR_FIND_SECTION);

  return(Ini_ConvertValue_m(file->ptagCurrSection->ptagCurrKey->caData,ptagData));
}

int IniFile_CreateEntry_SetValue(Inifile file,
                                 const char *section,
                                 const char *key,
                                 const TagData *ptagData)
{
  int iRc;

  if((!file) || (!key) || (!ptagData)) /* Validate params for not being NULL */
    return(INI_ERR_PARAM_INVALID);

  /* Empty section: Always exists as first */
  if((!section) || (section[0]=='\0'))
    file->ptagCurrSection=&file->ptagSections[0];
  else
  {
    /* Check if section exist or create new if not */
    switch((iRc=Ini_CreateNewSection_m(file,section)))
    {
      case INI_ERR_NONE:
      case INI_ERR_SECTIONNAME_DUPLICATE: /* Section already exists */
        break;
      default: /* Other error occured */
        return(iRc);
    }
  }
  /* Check if key exists, create if not */
  switch((iRc=Ini_CreateNewKey_m(file,key)))
  {
    case INI_ERR_NONE:
    case INI_ERR_KEYNAME_DUPLICATE: /* Key already exists */
      break;
    default:
      return(iRc);
  }
  return(Ini_KeySetData_m(file,ptagData));
}

int IniFile_DeleteEntry_Key(Inifile file,
                            const char *section,
                            const char *key)
{
  unsigned int uiKeyIndex;

  if((!file) || (!key)) /* Validate params for not being NULL */
    return(INI_ERR_PARAM_INVALID);

  if(!Ini_FindSection_m(file,section))
    return(INI_ERR_FIND_NONE);

  if(!Ini_FindKey_m(file,key))
    return(INI_ERR_FIND_SECTION);
  /* Delete Key+value */

  uiKeyIndex=KEY_CALC_INDEX(file);
  --file->ptagCurrSection->uiKeysCount;
  if(uiKeyIndex < file->ptagCurrSection->uiKeysCount)
  {
    memmove(file->ptagCurrSection->ptagCurrKey,
            file->ptagCurrSection->ptagCurrKey+1,
            (file->ptagCurrSection->uiKeysCount-uiKeyIndex)*sizeof(struct TagIniKey));
  }
  return(INI_ERR_NONE);
}

int IniFile_DeleteEntry_Section(Inifile file,
                                const char *section)
{
  unsigned int uiSectionIndex;

  if((!file) || (!section)) /* Validate params for not being NULL */
    return(INI_ERR_PARAM_INVALID);

  if(!Ini_FindSection_m(file,section))
    return(INI_ERR_FIND_NONE);

  /* Can't delete first section */
  if(file->ptagCurrSection == file->ptagSections)
    return(INI_ERR_PARAM_INVALID);

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
  return(INI_ERR_NONE);
}

void IniFile_DumpContent(const Inifile file)
{
  unsigned int uiSize;
  unsigned int uiIndexSection;
  unsigned int uiIndexKey;

  puts("***Start Dumping Contents of Inifile***");
  for(uiIndexSection=0;uiIndexSection < file->uiSectionsCount;++uiIndexSection)
  {
    if(uiIndexSection)
      printf("0x%p [%s]\n",
             file->ptagSections[uiIndexSection].caSectionName,
             file->ptagSections[uiIndexSection].caSectionName);
    for(uiIndexKey=0;uiIndexKey < file->ptagSections[uiIndexSection].uiKeysCount;++uiIndexKey)
    {
      uiSize=file->uiBufSize;
      if(Ini_EscapeString_m((unsigned char*)file->ptagSections[uiIndexSection].ptagKeys[uiIndexKey].caData,
                            file->caTmpBuffer,
                            &uiSize) != INI_ERR_NONE)
        return;
      printf("0x%p=0x%p %s=%s\n",
             file->ptagSections[uiIndexSection].ptagKeys[uiIndexKey].caKeyName,
             file->ptagSections[uiIndexSection].ptagKeys[uiIndexKey].caData,
             file->ptagSections[uiIndexSection].ptagKeys[uiIndexKey].caKeyName,
             file->caTmpBuffer);
    }
  }
  puts("***End Dumping Contents of Inifile***");
}

INLINE_FCT int Ini_UnEscapeString_m(unsigned char *pucStr,
                                    unsigned int *puiDataSize)
{
  unsigned int uiIndex;
  unsigned int uiIndexB;
  char caTmp[4];

  for(uiIndex=0;pucStr[uiIndex] != '\0';++uiIndex)
  {
    if(pucStr[uiIndex] != CHAR_BS)
      continue;
    /* Evaluate Escape sequence character */
    switch(pucStr[uiIndex+1])
    {
      case CHAR_A: /* 'a', Bell (alert) */
        pucStr[uiIndex]=ESC_SEQ_CHAR_A;
        break;
      case CHAR_B: /* 'b', Backspace */
        pucStr[uiIndex]=ESC_SEQ_CHAR_B;
        break;
      case CHAR_F: /* 'f', Form feed */
        pucStr[uiIndex]=ESC_SEQ_CHAR_F;
        break;
      case CHAR_N: /* 'n', New line */
        pucStr[uiIndex]=ESC_SEQ_CHAR_N;
        break;
      case CHAR_R: /* 'r', Carriage return */
        pucStr[uiIndex]=ESC_SEQ_CHAR_R;
        break;
      case CHAR_T: /* 't', Horizontal Tab */
        pucStr[uiIndex]=ESC_SEQ_CHAR_T;
        break;
      case CHAR_V: /* 'v', Vertical Tab */
        pucStr[uiIndex]=ESC_SEQ_CHAR_V;
        break;
      case CHAR_SQ: /* '\'', Single quotation mark */
        pucStr[uiIndex]=ESC_SEQ_CHAR_SQ;
        break;
      case CHAR_DQ: /* '\"', Double quotation mark */
        pucStr[uiIndex]=ESC_SEQ_CHAR_DQ;
        break;
      case CHAR_BS: /* '\\', Backslash */
        pucStr[uiIndex]=ESC_SEQ_CHAR_BS;
        break;
      case CHAR_QUES: /* '?', Question mark  */
        pucStr[uiIndex]=ESC_SEQ_CHAR_QUES;
        break;
      case CHAR_O: /* 'o', ASCII octal notation */
        uiIndexB=uiIndex+2;
        if(!IS_OCT_DIGIT(pucStr[uiIndexB])) /* If 1st char isn't octal, escape sequence is invalid */
          return(INI_ERR_ESC_SEQ_INVALID);

        caTmp[3]='\0';
        while(1)
        {
          if(!IS_OCT_DIGIT(pucStr[uiIndexB])) /* Octal sequence ended */
            break;
          /* First octal char can max. be 3 (otherwise it won't fit into 1 byte */
          if(pucStr[uiIndexB] > '3')
            return(INI_ERR_ESC_SEQ_INVALID);

          /* Next 2 bytes also must be a valid octal number */
          if((!IS_OCT_DIGIT(pucStr[uiIndexB+1])) ||
             (!IS_OCT_DIGIT(pucStr[uiIndexB+2])))
            return(INI_ERR_ESC_SEQ_INVALID);

          caTmp[0]=pucStr[uiIndexB++];
          caTmp[1]=pucStr[uiIndexB++];
          caTmp[2]=pucStr[uiIndexB++];
          pucStr[uiIndex++]=strtol(caTmp,NULL,8);
        }
        uiIndexB-=uiIndex;
        /* If we're at the end of the buffer, just terminate with '\0', no need to memmove */
        if(uiIndexB+uiIndex == (*puiDataSize)-1)
          pucStr[uiIndex]='\0';
        else
          memmove(&pucStr[uiIndex],
                  &pucStr[uiIndex+uiIndexB],
                  (*puiDataSize)-(uiIndex+uiIndexB));
        (*puiDataSize)-=uiIndexB;
        continue; /* Continue, special escape character handling */
      case CHAR_X: /* 'x', ASCII Hexadecimal notation */
        uiIndexB=uiIndex+2;
        if(!isxdigit(pucStr[uiIndexB])) /* If 1st char isn't hexadecimal, escape sequence is invalid */
          return(INI_ERR_ESC_SEQ_INVALID);

        caTmp[2]='\0';
        while(1)
        {
          if(!isxdigit(pucStr[uiIndexB])) /* Hex sequence ended */
            break;
          /* Next byte also must be a valid hex number */
          if(!isxdigit(pucStr[uiIndexB+1]))
            return(INI_ERR_ESC_SEQ_INVALID);

          caTmp[0]=pucStr[uiIndexB++];
          caTmp[1]=pucStr[uiIndexB++];
          pucStr[uiIndex++]=strtol(caTmp,NULL,16);
        }
        /* If we're at the end of the buffer, just terminate with '\0', no need to memmove */
        uiIndexB-=uiIndex;
        if(uiIndexB+uiIndex == (*puiDataSize)-1)
          pucStr[uiIndex]='\0';
        else
          memmove(&pucStr[uiIndex],
                  &pucStr[uiIndex+uiIndexB],
                  (*puiDataSize)-(uiIndex+uiIndexB));
        (*puiDataSize)-=uiIndexB;
        continue; /* Continue, special escape character handling */
      case CHAR_0: /* '\0' continue on next line, return directly (line ended) */
        pucStr[uiIndex]='\0';
        (*puiDataSize)--;
        return(INI_CONTINUE_ON_NEXT_LINE);
      default:
        return(INI_ERR_ESC_SEQ_INVALID);
    }
    memmove(&pucStr[uiIndex+1],&pucStr[uiIndex+2],(*puiDataSize)-(uiIndex+2));
    (*puiDataSize)--;
  }
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

INLINE_FCT int Ini_EscapeString_m(const unsigned char *pucInText,
                                  char *pcOutText,
                                  unsigned int *puiOutBufSize)
{
  unsigned int uiIndex;
  unsigned int uiIndexOut=0;
  int iLastDataBin=0;

  for(uiIndex=0;pucInText[uiIndex] != '\0';++uiIndex)
  {
    if(uiIndexOut+2 > *puiOutBufSize) /* Buffer too small */
      return(INI_ERR_BUFFER_TOO_SMALL);

    switch(pucInText[uiIndex])
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
        /* If ASCII and printable, skip */
        if((isascii(pucInText[uiIndex])) && (isprint(pucInText[uiIndex])))
        {
          pcOutText[uiIndexOut++]=pucInText[uiIndex];
          break;
        }
        /* Binary data, escape to HEX */
        if(!iLastDataBin)
        {
          pcOutText[uiIndexOut++]='\\';
          pcOutText[uiIndexOut++]=CHAR_X;
        }
        if(uiIndexOut+3 > *puiOutBufSize)
          return(INI_ERR_BUFFER_TOO_SMALL);

        pcOutText[uiIndexOut++]=(pucInText[uiIndex]>>4)+(((pucInText[uiIndex]>>4) > 9)?'A'-10:'0');
        pcOutText[uiIndexOut++]=(pucInText[uiIndex] & 0xF)+(((pucInText[uiIndex] & 0xF) > 9)?'A'-10:'0');
        iLastDataBin=1;
        continue;
    }
    iLastDataBin=0;
  }
  if(uiIndexOut+1 > *puiOutBufSize)
    return(INI_ERR_BUFFER_TOO_SMALL);
  pcOutText[uiIndexOut++]='\0';
  *puiOutBufSize=uiIndexOut;
  return(INI_ERR_NONE);
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
    file->ptagSections=pvTmp;

  file->ptagCurrSection=&file->ptagSections[file->uiSectionsCount];
  /* Copy Sectionname */
  strcpy(file->ptagCurrSection->caSectionName,pcSection);

  /* Init new Keys table... */
  if(!(file->ptagCurrSection->ptagKeys=KEYS_REALLOC(NULL,INITIAL_KEYS_PER_SECTION_COUNT)))
    return(INI_ERR_MEMORY_ALLOC);
  file->ptagCurrSection->uiKeysCount=0;
  file->ptagCurrSection->uiKeysMax=INITIAL_KEYS_PER_SECTION_COUNT;

  ++file->uiSectionsCount;
  //KEY_FIRST_SET_EMPTY(file->ptagSections[GET_SECTION_INDEX(file)]);
  return(INI_ERR_NONE);
}

INLINE_FCT int Ini_CreateNewKey_m(Inifile file,
                                  const char *pcKey)
{
  void *pvTmp=NULL;

  if(!CHECK_KEYNAME_VALID(pcKey))
    return(INI_ERR_KEYNAME_INVALID);

  if(Ini_FindKey_m(file,pcKey))
  {
    if(OPTION_CHK(file,INI_OPT_ALLOW_DOUBLE_ENTRIES))
      return(INI_ERR_NONE);
    return(INI_ERR_KEYNAME_DUPLICATE);
  }

  if((KEYS_NEED_REALLOC(file->ptagCurrSection))
     && (!(pvTmp=KEYS_REALLOC(file->ptagCurrSection->ptagKeys,file->ptagCurrSection->uiKeysMax*=2))))
    return(INI_ERR_MEMORY_ALLOC);

  if(pvTmp)
    file->ptagCurrSection->ptagKeys=pvTmp;

  file->ptagCurrSection->ptagCurrKey=&file->ptagCurrSection->ptagKeys[file->ptagCurrSection->uiKeysCount];

  strcpy(file->ptagCurrSection->ptagCurrKey->caKeyName,pcKey);

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

  if(uiIndex > uiMaxLen)
    return(0);

  do /* Validate each character */
  {
    if((!isprint(pcName[--uiIndex])) || (isspace(pcName[uiIndex])))
      return(0);
  }
  while(uiIndex);

  return(1);
}

INLINE_FCT int Ini_CheckKeyValueValid_m(const char *pcValue)
{
  if(!pcValue)
    return(INI_ERR_SECTIONNAME_INVALID);
  if(strlen(pcValue) < MAX_KEYVAL_LENGTH)
    return(INI_ERR_NONE);
  return(INI_ERR_SECTIONNAME_INVALID);
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
      return(1);
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
      return(1);
    }
  }
  return(0);
}

INLINE_FCT void Ini_ResetContent_m(Inifile file)
{
  unsigned int uiIndexSection;

  file->ptagSections[0].uiKeysCount=0;
  file->ptagSections[0].ptagCurrKey=file->ptagSections[0].ptagKeys;
  /* Keep the first (empty) section as is, it's always used */
  for(uiIndexSection=1;uiIndexSection < file->uiSectionsCount;++uiIndexSection)
  {
    free(file->ptagSections[uiIndexSection].ptagKeys);
    file->ptagSections[uiIndexSection].uiKeysCount=0;
  }
  file->ptagCurrSection=file->ptagSections;
  file->ptagSections[0].ptagCurrKey=file->ptagSections[0].ptagKeys;
  file->uiSectionsCount=1;
}

INLINE_FCT int Ini_OptionsValid_m(unsigned int uiOptions)
{
  uiOptions&=~(INI_OPT_CASE_SENSITIVE | INI_OPT_IGNORE_MALFORMED_LINES | INI_OPT_ALLOW_DOUBLE_ENTRIES | INI_OPT_ALLOW_MULTILINE);
  return((uiOptions)?-1:0);
}

INLINE_FCT int Ini_KeySetData_m(Inifile file,
                                const TagData *ptagData)
{
  switch(ptagData->eType)
  {
    case eDataType_Int:
      sprintf(file->ptagCurrSection->ptagCurrKey->caData,"%d",ptagData->data.iVal);
      break;
    case eDataType_Uint:
      sprintf(file->ptagCurrSection->ptagCurrKey->caData,"%u",ptagData->data.uiVal);
      break;
    case eDataType_Double:
      sprintf(file->ptagCurrSection->ptagCurrKey->caData,"%f",ptagData->data.dVal);
      break;
    case eDataType_Boolean:
      strcpy(file->ptagCurrSection->ptagCurrKey->caData,(ptagData->data.bVal)?pcaBooleanStringsTrue_m[0]:pcaBooleanStringsFalse_m[0]);
      break;
    case eDataType_String:
      if(Ini_CheckKeyValueValid_m(ptagData->data.pcVal))
        return(INI_ERR_KEYVAL_INVALID);
      memcpy(file->ptagCurrSection->ptagCurrKey->caData,ptagData->data.pcVal,ptagData->dataSize);
      break;
    default:
      return(INI_ERR_DATATYPE_UNKNOWN);
  }
  return(INI_ERR_NONE);
}

INLINE_FCT int Ini_Read_Section_m(char *pcLine,
                                  unsigned int uiLength)
{
  /* Check if last char is correct */
  if(pcLine[uiLength-2] != SECTION_NAME_END)
    return(INI_ERR_READ_MALFORMED);

  pcLine[uiLength-2]='\0';
  return(INI_ERR_NONE);
}

INLINE_FCT int Ini_Read_Key_m(char *pcLine,
                              char **ppcKeyVal,
                              unsigned int *puiValLength)
{
  if(!((*ppcKeyVal)=strchr(pcLine,'=')))
    return(INI_ERR_READ_MALFORMED);

  **ppcKeyVal='\0'; /* Overwrite '=' with '\0' */
  Ini_RemoveFollowingWhitespaces_m(pcLine,(*ppcKeyVal)-pcLine);
  ++(*ppcKeyVal);
  Ini_RemoveLeadingWhitespaces_m(ppcKeyVal);

  *puiValLength=pcLine+(*puiValLength)-(*ppcKeyVal); /* Calculate length of ppcKeyVal */
  /* Check for Quotes at beginning and at the end, and remove them */
  if(**ppcKeyVal=='\"')
  {
    (*puiValLength)-=2;
    if((*ppcKeyVal)[*puiValLength] == '\"')
    {
      (*ppcKeyVal)[*puiValLength]='\0';
      ++(*ppcKeyVal);
    }
    else
      return(INI_ERR_READ_QUOTES_INVALID);
  }
  if(**ppcKeyVal=='\'')
  {
    (*puiValLength)-=2;
    if((*ppcKeyVal)[*puiValLength] == '\'')
    {
      (*ppcKeyVal)[*puiValLength]='\0';
      ++(*ppcKeyVal);
    }
    else
      return(INI_ERR_READ_QUOTES_INVALID);
  }
  return(INI_ERR_NONE);
}

INLINE_FCT int Ini_Read_Comment_m(char *pcLine)
{
//printf("Ignoring Comment: %s\n",pcLine);   /* TODO: add option to read comments? */
  return(INI_ERR_NONE);
}

INLINE_FCT int Ini_ConvertValue_m(const char *pcStr,
                                  TagData *ptagData)
{
  char *pcTmp;
  long lTmp;
  unsigned long ulTmp;
  double dTmp;
  errno=0;
  switch(ptagData->eType)
  {
    case eDataType_Int:
      lTmp=strtol(pcStr,&pcTmp,10);
      /* Check for conversion error / Int Over- or Underflow */
      if(((lTmp > INT_MAX) || ((lTmp == LONG_MAX) && (errno == ERANGE))) /* Check for overflow */
         || ((lTmp < INT_MIN) || ((lTmp == LONG_MIN) && (errno == ERANGE))) /* Check for underflow */
         || (*pcTmp != '\0')) /* Check for invalid string */
        return(INI_ERR_CONVERSION);
      ptagData->data.iVal=(int)lTmp;
      break;
    case eDataType_Uint:
      ulTmp=strtoul(pcStr,&pcTmp,10);
      /* Check for conversion error / Uint Overflow */
      if(((ulTmp > UINT_MAX) || ((ulTmp == ULONG_MAX) && (errno == ERANGE))) /* Check for overflow */
         || (*pcTmp != '\0')) /* Check for invalid string */
        return(INI_ERR_CONVERSION);
      ptagData->data.uiVal=(unsigned int)ulTmp;
      break;
    case eDataType_Double:
      dTmp=strtod(pcStr,&pcTmp);
      /* Check for conversion error / Over- or Underflow */
      if(((dTmp == HUGE_VAL) && (errno == ERANGE)) /* Check for Overflow */
         || ((dTmp == -HUGE_VAL) && (errno == ERANGE))
         || ((dTmp == 0.0) && (errno == ERANGE)) /* Check for Underflow */
         || (*pcTmp != '\0')) /* Check for invalid string */
        return(INI_ERR_CONVERSION);
      ptagData->data.dVal=dTmp;
      break;
    case eDataType_Boolean:
      for(ulTmp=0;ulTmp < sizeof(pcaBooleanStringsFalse_m)/sizeof(char *);++ulTmp)
      {
        /* Check for positive boolean value */
        if(STRCMP_CIS(pcStr,pcaBooleanStringsFalse_m[ulTmp]) == 0)
        {
          ptagData->data.bVal=0;
          return(INI_ERR_NONE);
        }
        /* Check for negative boolean value */
        if(STRCMP_CIS(pcStr,pcaBooleanStringsTrue_m[ulTmp]) == 0)
        {
          ptagData->data.bVal=1;
          return(INI_ERR_NONE);
        }
      }
      return(INI_ERR_CONVERSION);
    case eDataType_String:
      if((ulTmp=strlen(pcStr)+1) > ptagData->dataSize)
        return(INI_ERR_BUFFER_TOO_SMALL); /* Buffer too small */
      memcpy(ptagData->data.pcVal,pcStr,ulTmp);
      break;
    default: /* Unknown datatype */
      return(INI_ERR_DATATYPE_UNKNOWN);
  }
  return(INI_ERR_NONE);
}
