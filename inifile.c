#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#if defined (__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) /* >= C99 */
  #define INLINE_FCT inline
  #define INLINE_PROT static inline
#else /* No inline available from C Standard */
  #define INLINE_FCT
  #define INLINE_PROT
#endif /* __STDC_VERSION__ >= C99 */

#include "inifile.h"

enum
{
  INITIAL_SECTION_COUNT             = 10,
  INITIAL_KEYS_PER_SECTION_COUNT    = 10,
  MAX_SECTION_LENGTH                = 80,
  MAX_KEYNAME_LENGTH                = MAX_SECTION_LENGTH,
  MAX_KEYVAL_LENGTH                 = 300,
  MAX_PATH_LENGTH                   = 260
};

enum
{
  INI_FOUND_NONE,
  INI_FOUND_SECTION,
  INI_FOUND_SECTION_AND_KEY
};

#define INI_SECTION_NAME_BEGIN  '['
#define INI_SECTION_NAME_END    ']'

#define INI_COMMENT_INDICATOR_1 ';'
#define INI_COMMENT_INDICATOR_2 '#'

#define KEYS_REALLOC(ptr,newCount)     realloc((ptr),(newCount)*sizeof(struct TagIniKey))
#define SECTIONS_REALLOC(ptr,newCount) realloc((ptr),(newCount)*sizeof(struct TagIniSection))
#define SECTIONS_NEED_REALLOC(inifile) (((inifile)->iSectionsIndex<(inifile)->iSectionsMax)?0:1)
#define KEYS_NEED_REALLOC(section)     (((section)->iKeysIndex<(section)->iKeysMax)?0:1)

#define GET_SECTION_INDEX(file) ((file)->iSectionsIndex)

#define KEY_FIRST_SET_EMPTY(section)  ((section).ptagKeys[0].caKeyName[0]='\0')
#define KEY_FIRST_IS_EMPTY(section)   ((section)->ptagKeys[0].caKeyName[0]=='\0')
#define SECTION_FIRST_SET_EMPTY(file) ((file)->ptagSections[0].caSectionName[0]='\0')
#define SECTION_FIRST_IS_EMPTY(file)  ((file)->ptagSections[0].caSectionName[0]=='\0')

struct TagIniKey
{
  char caKeyName[MAX_KEYNAME_LENGTH+1];
  char caData[MAX_KEYVAL_LENGTH];
};

struct TagIniSection
{
  int iKeysIndex;
  int iKeysMax;
  char caSectionName[MAX_SECTION_LENGTH+1];
  struct TagIniKey *ptagKeys;
};

struct TagIniFile
{
  int iSectionsIndex;
  int iSectionsMax;
  struct TagIniSection *ptagSections;
};

INLINE_PROT int Ini_InitNewFile_m(Inifile *ptagFile);

INLINE_PROT int strIsEqual_NoneCaseSensitive_m(const char *pcStr1,
                                               const char *pcStr2);
INLINE_PROT char *Ini_RemoveLeadingWhitespaces_m(char *pcString);
INLINE_PROT void Ini_RemoveFollowingWhitespaces_m(char *pcString);

INLINE_PROT int Ini_CheckSectionNameValid_m(const char *pcSection);
INLINE_PROT int Ini_CheckKeyNameValid_m(const char *pcKey);
INLINE_PROT int Ini_CheckKeyValueValid_m(const char *pcValue);

INLINE_PROT int Ini_CreateNewSection_m(Inifile tagfile,
                                       const char *pcSection);

INLINE_PROT int Ini_CreateNewKey_m(IniSection tagSection,
                                   const char *pcKey,
                                   EIniDataType type,
                                   const void *buffer);

INLINE_PROT int Ini_KeySetData_m(IniKey tagKey,
                                 const char *pcName,
                                 EIniDataType type,
                                 const void *buffer);

int Ini_FindEntry_m(Inifile file,
                    const char *pcSection,
                    const char *pcKey,
                    IniSection *ptagSection,
                    IniKey *ptagKey);

INLINE_PROT int Ini_Read_Section_m(char **ppcLine);
INLINE_PROT int Ini_Read_Key_m(char *pcLine,
                               char **ppcKeyVal);
INLINE_PROT int Ini_Read_Comment_m(char *pcLine);

INLINE_PROT int Ini_ParseLine_m(Inifile file,
                                char *pcLine);

INLINE_PROT int Ini_ConvertValue_m(const char *pcStr,
                                   EIniDataType type,
                                   void *buffer,
                                   size_t szBufSize);

static const char *pcaBooleanStringsTrue_m[]= {"True", "Enabled", "1"};
static const char *pcaBooleanStringsFalse_m[]={"False","Disabled","0"};

Inifile IniFile_New(void)
{
  Inifile newFile;

  if(Ini_InitNewFile_m(&newFile))
    return(NULL);
  return(newFile);
}

Inifile IniFile_Read(const char *path)
{
  FILE *fp;
  Inifile file;
  char caLineBuffer[MAX_KEYNAME_LENGTH+MAX_KEYVAL_LENGTH+5];

  if(!(fp=fopen(path,"r")))
    return(NULL);

  if(Ini_InitNewFile_m(&file))
  {
    fclose(fp);
    return(NULL);
  }

  while(fgets(caLineBuffer,sizeof(caLineBuffer),fp))
  {
    if(Ini_ParseLine_m(file,caLineBuffer))
    {
      fprintf(stderr,"Inifile \"%s\" malformed Line: %s\n",path,caLineBuffer);
      fclose(fp);
      IniFile_Delete(file);
      return(NULL);
    }
  }
  /* If EOF was reached, the whole file was read successfully */
  if(feof(fp))
  {
    fclose(fp);
    return(file);
  }
  fclose(fp);
  IniFile_Delete(file);
  return(NULL);
}

int IniFile_Write(Inifile file,
                  const char *path)
{
  FILE *fp;
  int iIndexSection;
  int iIndexKey;

  if(!(fp=fopen(path,"w")))
    return(-1);

  for(iIndexSection=0;iIndexSection<=file->iSectionsIndex;++iIndexSection)
  {
    if((iIndexSection) || (!SECTION_FIRST_IS_EMPTY(file)))
      fprintf(fp,"[%s]\n",file->ptagSections[iIndexSection].caSectionName);
    for(iIndexKey=0;iIndexKey<file->ptagSections[iIndexSection].iKeysIndex;++iIndexKey)
    {
      fprintf(fp,
              "%s=%s\n",
              file->ptagSections[iIndexSection].ptagKeys[iIndexKey].caKeyName,
              file->ptagSections[iIndexSection].ptagKeys[iIndexKey].caData);
    }
  }
  fclose(fp);
  return(0);
}

void IniFile_Delete(Inifile file)
{
  int iIndexSection;
  for(iIndexSection=0;iIndexSection<=file->iSectionsIndex;++iIndexSection)
    free(file->ptagSections[iIndexSection].ptagKeys);
  free(file->ptagSections);
  free(file);
}

int IniFile_FindEntry_GetValue(Inifile file,
                               const char *section,
                               const char *key,
                               EIniDataType type,
                               void *buffer,
                               size_t bufSize)
{
  IniKey ptagKey;
  switch(Ini_FindEntry_m(file,section,key,NULL,&ptagKey))
  {
    case INI_FOUND_SECTION_AND_KEY:
      break;
    default:
      return(-1);
  }
  return(Ini_ConvertValue_m(ptagKey->caData,type,buffer,bufSize));
}

int IniFile_CreateEntry_SetValue(Inifile file,
                                 const char *section,
                                 const char *key,
                                 EIniDataType type,
                                 const void *buffer)
{
  IniKey ptagKey;
  IniSection ptagSection;

  switch(Ini_FindEntry_m(file,section,key,&ptagSection,&ptagKey))
  {
    case INI_FOUND_NONE: /* Neither Section nor key found, create new section */
      if(Ini_CreateNewSection_m(file,(section)?section:""))
        return(-1);
      ptagSection=&file->ptagSections[GET_SECTION_INDEX(file)];
      /* Fallthrough */
    case INI_FOUND_SECTION: /* Creating new Key */
      if(Ini_CreateNewKey_m(ptagSection,key,type,buffer))
        return(-1);
      break;
    case INI_FOUND_SECTION_AND_KEY: /* Overwrite existing value */
      if(Ini_KeySetData_m(ptagKey,NULL,type,buffer))
        return(-1);
      break;
  }
  return(0);
}

INLINE_FCT int Ini_CreateNewSection_m(Inifile tagfile,
                                      const char *pcSection)
{
  void *pvTmp=NULL;

  if(Ini_CheckSectionNameValid_m(pcSection))
    return(-1);

  /* Realloc if needed */
  if((SECTIONS_NEED_REALLOC(tagfile)) && (!(pvTmp=SECTIONS_REALLOC(tagfile->ptagSections,tagfile->iSectionsMax*=2))))
    return(-1);
  if(pvTmp)
    tagfile->ptagSections=pvTmp;

 if((!tagfile->iSectionsIndex) && (!tagfile->ptagSections[0].iKeysIndex)) /* Exception for first Section */
    --tagfile->iSectionsIndex;

  strcpy(tagfile->ptagSections[++tagfile->iSectionsIndex].caSectionName,pcSection);

  if((!tagfile->iSectionsIndex) && (!tagfile->ptagSections[0].iKeysIndex)) /* If first section, key space is already allocated */
    return(0);
  /* Init new Keys table... */
  if(!(tagfile->ptagSections[GET_SECTION_INDEX(tagfile)].ptagKeys=KEYS_REALLOC(NULL,INITIAL_KEYS_PER_SECTION_COUNT)))
    return(-1);
  tagfile->ptagSections[GET_SECTION_INDEX(tagfile)].iKeysIndex=0;
  tagfile->ptagSections[GET_SECTION_INDEX(tagfile)].iKeysMax=INITIAL_KEYS_PER_SECTION_COUNT;
  KEY_FIRST_SET_EMPTY(tagfile->ptagSections[GET_SECTION_INDEX(tagfile)]);
  return(0);
}

INLINE_FCT int Ini_CreateNewKey_m(IniSection tagSection,
                                  const char *pcKey,
                                  EIniDataType type,
                                  const void *buffer)
{
  void *pvTmp=NULL;
  if((KEYS_NEED_REALLOC(tagSection))
  && (!(pvTmp=KEYS_REALLOC(tagSection->ptagKeys,tagSection->iKeysMax*=2))))
    return(-1);

  if(pvTmp)
    tagSection->ptagKeys=pvTmp;

  if(Ini_KeySetData_m(&tagSection->ptagKeys[tagSection->iKeysIndex],pcKey,type,buffer))
    return(-1);

  if(!KEY_FIRST_IS_EMPTY(tagSection))
    ++tagSection->iKeysIndex;
  return(0);
}

void IniFile_DumpContent(Inifile file)
{
  int iIndexSection;
  int iIndexKey;

  puts("***Start Dumping Contents of Inifile***");
  for(iIndexSection=0;iIndexSection<=file->iSectionsIndex;++iIndexSection)
  {
    printf("[%s]\n",file->ptagSections[iIndexSection].caSectionName);
    for(iIndexKey=0;iIndexKey<file->ptagSections[iIndexSection].iKeysIndex;++iIndexKey)
    {
      printf("%s=%s\n",
             file->ptagSections[iIndexSection].ptagKeys[iIndexKey].caKeyName,
             file->ptagSections[iIndexSection].ptagKeys[iIndexKey].caData);
    }
  }
  puts("***End Dumping Contents of Inifile***");
}

INLINE_FCT char *Ini_RemoveLeadingWhitespaces_m(char *pcString)
{
  int iIndex;
  for(iIndex=0;isspace(pcString[iIndex]);++iIndex);
  return(&pcString[iIndex]);
}

INLINE_FCT void Ini_RemoveFollowingWhitespaces_m(char *pcString)
{
  int iIndex;
  for(iIndex=strlen(pcString)-1;isspace(pcString[iIndex]);--iIndex);
  pcString[iIndex+1]='\0';
}

INLINE_FCT int strIsEqual_NoneCaseSensitive_m(const char *pcStr1,
                                              const char *pcStr2)
{
  unsigned int uiIndex;
  if((uiIndex=strlen(pcStr1))!=strlen(pcStr2))
    return(0);
  while((uiIndex) && (tolower(pcStr1[uiIndex])==tolower(pcStr2[uiIndex])))
  {
    --uiIndex;
  }
  return((uiIndex)?0:1);
}

INLINE_FCT int Ini_CheckSectionNameValid_m(const char *pcSection)
{
  if(!pcSection)
    return(-1);
  if(strlen(pcSection)<MAX_SECTION_LENGTH)
    return(0);
  return(-1);
}

INLINE_FCT int Ini_CheckKeyNameValid_m(const char *pcKey)
{
  if(!pcKey)
    return(-1);
  if(strlen(pcKey)<MAX_KEYNAME_LENGTH)
    return(0);
  return(-1);
}

INLINE_FCT int Ini_CheckKeyValueValid_m(const char *pcValue)
{
  if(!pcValue)
    return(-1);
  if(strlen(pcValue)<MAX_KEYVAL_LENGTH)
    return(0);
  return(-1);
}

INLINE_FCT int Ini_InitNewFile_m(Inifile *ptagFile)
{
  if(!(*ptagFile=malloc(sizeof(struct TagIniFile))))
    return(-1);

  if(!((*ptagFile)->ptagSections=SECTIONS_REALLOC(NULL,INITIAL_SECTION_COUNT)))
  {
    free(*ptagFile);
    return(-1);
  }
  SECTION_FIRST_SET_EMPTY(*ptagFile);
  if(!((*ptagFile)->ptagSections[0].ptagKeys=KEYS_REALLOC(NULL,INITIAL_KEYS_PER_SECTION_COUNT)))
  {
    free((*ptagFile)->ptagSections);
    free(*ptagFile);
    return(-1);
  }
  (*ptagFile)->iSectionsIndex=0;
  (*ptagFile)->iSectionsMax=INITIAL_SECTION_COUNT;
  (*ptagFile)->ptagSections[0].iKeysIndex=0;
  (*ptagFile)->ptagSections[0].iKeysMax=INITIAL_KEYS_PER_SECTION_COUNT;
  KEY_FIRST_SET_EMPTY((*ptagFile)->ptagSections[0]);
  return(0);
}

INLINE_FCT int Ini_KeySetData_m(IniKey tagKey,
                                const char *pcName,
                                EIniDataType type,
                                const void *buffer)
{
  if(pcName)
  {
    if(Ini_CheckKeyNameValid_m(pcName))
      return(-1);
    strcpy(tagKey->caKeyName,pcName);
  }
  switch(type)
  {
    case eIniDataType_Int:
      sprintf(tagKey->caData,"%d",*((const int*)buffer));
      break;
    case eIniDataType_Uint:
      sprintf(tagKey->caData,"%u",*((const unsigned int*)buffer));
      break;
    case eIniDataType_Double:
      sprintf(tagKey->caData,"%f",*((const double*)buffer));
      break;
    case eIniDataType_Boolean:
      strcpy(tagKey->caData,(*((const int*)buffer))?pcaBooleanStringsTrue_m[0]:pcaBooleanStringsFalse_m[0]);
      break;
    case eIniDataType_String:
      if(Ini_CheckKeyValueValid_m(buffer))
        return(-1);
      strcpy(tagKey->caData,buffer);
      break;
    default:
      return(-1);
  }
  return(0);
}

INLINE_FCT int Ini_Read_Section_m(char **ppcLine)
{
  int iIndex;
  ++(*ppcLine); /* Skip '[' */
  for(iIndex=0;((*ppcLine)[iIndex]!='\0') && (iIndex<MAX_SECTION_LENGTH);++iIndex)
  {
    if(((*ppcLine)[iIndex]==INI_SECTION_NAME_END) && ((*ppcLine)[iIndex+1]=='\0'))
    {
      if(iIndex<1) /* Empty sections not allowed */
        return(-1);
      (*ppcLine)[iIndex]='\0';
      return(0);
    }
  }
  return(-1);
}

INLINE_FCT int Ini_Read_Key_m(char *pcLine,
                              char **ppcKeyVal)
{
  if(!((*ppcKeyVal)=strchr(pcLine,'=')))
    return(-1);

  **ppcKeyVal='\0'; /* Overwrite '=' with '\0' */
  ++(*ppcKeyVal);
  *ppcKeyVal=Ini_RemoveLeadingWhitespaces_m(*ppcKeyVal);
  Ini_RemoveFollowingWhitespaces_m(pcLine);
  return(0);
}

INLINE_FCT int Ini_Read_Comment_m(char *pcLine)
{
  printf("Ignoring Comment: %s\n",pcLine);   /* TODO: add option to read comments? */
  return(0);
}

INLINE_FCT int Ini_ParseLine_m(Inifile file,
                               char *pcLine)
{
  char *pcTmp;
  pcLine=Ini_RemoveLeadingWhitespaces_m(pcLine);
  if(pcLine[strlen(pcLine)-1]=='\n')
    pcLine[strlen(pcLine)-1]='\0'; /* Remove '\n'  (usually there from fgets()) */
  if(pcLine[0]=='\0') /* Skip newlines in inifile */
    return(0);

  switch(pcLine[0]) /* Check first character */
  {
    case INI_SECTION_NAME_BEGIN: /* Is a new section */
      if(Ini_Read_Section_m(&pcLine))
        return(-1);
      if(Ini_CreateNewSection_m(file,pcLine))
        return(-1);
      break;
    case INI_COMMENT_INDICATOR_1: /* Is a comment, ignore for now... */
    case INI_COMMENT_INDICATOR_2:
      /* Fallthrough */
      if(Ini_Read_Comment_m(pcLine))
        return(-1);
      break;
    default:
      if(Ini_Read_Key_m(pcLine,&pcTmp))
        return(-1);
      if(Ini_CreateNewKey_m(&file->ptagSections[GET_SECTION_INDEX(file)],pcLine,eIniDataType_String,pcTmp))
        return(-1);
      break;
  }
  return(0);
}

int Ini_FindEntry_m(Inifile file,
                    const char *pcSection,
                    const char *pcKey,
                    IniSection *ptagSection,
                    IniKey *ptagKey)
{
  int iIndexSection=0;
  int iIndexKey;

  if(!pcKey)
    return(INI_FOUND_NONE);

  if(pcSection)
  {
    /* Search for section */
    for(iIndexSection=0;iIndexSection<=file->iSectionsIndex;++iIndexSection)
    {
      if(strIsEqual_NoneCaseSensitive_m(pcSection,file->ptagSections[iIndexSection].caSectionName))
      {
        if(ptagSection)
          *ptagSection=&file->ptagSections[iIndexSection];
        break;
      }
    }
    if(iIndexSection>file->iSectionsIndex) /* Section not found */
      return(INI_FOUND_NONE);
  }
  else if(SECTION_FIRST_IS_EMPTY(file))
  {
    if(ptagSection)
      *ptagSection=&file->ptagSections[0];
  }
  else
    return(INI_FOUND_NONE);

  if(ptagKey)
  {
    for(iIndexKey=0;iIndexKey<file->ptagSections[iIndexSection].iKeysIndex;++iIndexKey)
    {
      if(strIsEqual_NoneCaseSensitive_m(pcKey,file->ptagSections[iIndexSection].ptagKeys[iIndexKey].caKeyName))
      {
        *ptagKey=&file->ptagSections[iIndexSection].ptagKeys[iIndexKey];
        return(INI_FOUND_SECTION_AND_KEY); /* Found both */
      }
    }
  }
  return(INI_FOUND_SECTION); /* Only Section found */
}

INLINE_FCT int Ini_ConvertValue_m(const char *pcStr,
                                  EIniDataType type,
                                  void *buffer,
                                  size_t szBufSize)
{
  char *pcTmp;
  long lTmp;
  unsigned long ulTmp;
  double dTmp;
  errno=0;
  switch(type)
  {
    case eIniDataType_Int:
      lTmp=strtol(pcStr,&pcTmp,10);
      /* Check for conversion error / Int Over- or Underflow */
      if(((lTmp > INT_MAX) || ((lTmp==LONG_MAX) && (errno==ERANGE))) /* Check for overflow */
      || ((lTmp < INT_MIN) || ((lTmp==LONG_MIN) && (errno==ERANGE))) /* Check for underflow */
      || (*pcTmp!='\0')) /* Check for invalid string */
        return(-1);
      *((int*)buffer)=lTmp;
      break;
    case eIniDataType_Uint:
      ulTmp=strtoul(pcStr,&pcTmp,10);
      /* Check for conversion error / Uint Overflow */
      if(((ulTmp > UINT_MAX) || ((ulTmp==ULONG_MAX) && (errno==ERANGE))) /* Check for overflow */
      || (*pcTmp!='\0')) /* Check for invalid string */
        return(-1);
      *((unsigned int*)buffer)=ulTmp;
      break;
    case eIniDataType_Double:
      dTmp=strtod(pcStr,&pcTmp);
      /* Check for conversion error / Over- or Underflow */
      if(((dTmp==HUGE_VAL)  && (errno==ERANGE)) /* Check for Overflow */
      || ((dTmp==-HUGE_VAL) && (errno==ERANGE))
      || ((dTmp==0.0)       && (errno==ERANGE)) /* Check for Underflow */
      || (*pcTmp!='\0')) /* Check for invalid string */
        return(-1);
      *((double*)buffer)=dTmp;
      break;
    case eIniDataType_Boolean:
      for(ulTmp=0;ulTmp<sizeof(pcaBooleanStringsFalse_m)/sizeof(char*);++ulTmp)
      {
        /* Check for positive boolean value */
        if(strIsEqual_NoneCaseSensitive_m(pcStr,pcaBooleanStringsFalse_m[ulTmp]))
        {
          *((int*)buffer)=0;
          return(0);
        }
        /* Check for negative boolean value */
        if(strIsEqual_NoneCaseSensitive_m(pcStr,pcaBooleanStringsTrue_m[ulTmp]))
        {
          *((int*)buffer)=1;
          return(0);
        }
      }
      return(-1);
      break;
    case eIniDataType_String:
      if((ulTmp=strlen(pcStr)+1)<szBufSize)
      {
        memcpy(buffer,pcStr,ulTmp);
        break;
      }
      return(-1);
    default: /* Unknown datatype */
      return(-1);
  }
  return(0);
}
